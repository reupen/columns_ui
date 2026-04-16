#include "pch.h"

#include "menu_mnemonics.h"

namespace cui::toolbars::menu {

constexpr auto ID_MENU = 2001u;
constexpr auto MSG_HIDE_MENUACC = WM_USER + 3u;
constexpr auto MSG_SHOW_MENUACC = WM_USER + 4u;
constexpr auto MSG_CREATE_MENU = WM_USER + 5u;
constexpr auto MSG_SIZE_LIMIT_CHANGE = WM_USER + 6u;

class MainMenuRootGroup {
public:
    pfc::string8 m_name;
    pfc::array_t<TCHAR> m_name_with_accelerators;
    GUID m_guid{};
    uint32_t m_sort_priority{NULL};

    MainMenuRootGroup() = default;

    static auto g_compare(const MainMenuRootGroup& p_item1, const MainMenuRootGroup& p_item2)
    {
        return pfc::compare_t(p_item1.m_sort_priority, p_item2.m_sort_priority);
    }

    static void g_get_root_items(pfc::list_base_t<MainMenuRootGroup>& p_out)
    {
        p_out.remove_all();
        MnemonicManager mnemonics;

        service_enum_t<mainmenu_group> e;
        service_ptr_t<mainmenu_group> ptr;
        service_ptr_t<mainmenu_group_popup> ptrp;

        while (e.next(ptr)) {
            if (ptr->get_parent() == pfc::guid_null) {
                if (ptr->service_query_t(ptrp)) {
                    MainMenuRootGroup item;
                    pfc::string8 name;
                    ptrp->get_display_string(name);
                    item.m_guid = ptrp->get_guid();
                    item.m_sort_priority = ptrp->get_sort_priority();
                    uFixAmpersandChars_v2(name, item.m_name);
                    name.reset();
                    mnemonics.process_string(item.m_name, name);
                    pfc::stringcvt::string_os_from_utf8 os(name);
                    item.m_name_with_accelerators.append_fromptr(os.get_ptr(), os.length());
                    item.m_name_with_accelerators.append_single(0);
                    p_out.add_item(item);
                }
            }
        }
        p_out.sort_t(g_compare);
    }
};

class MenuToolbar
    : public uie::container_uie_window_v3_t<uie::menu_window_v2>
    , uih::MessageHook {
public:
    static constexpr GUID extension_guid{0x76e6db50, 0xde3, 0x4f30, {0xa7, 0xe4, 0x93, 0xfd, 0x62, 0x8b, 0x14, 0x1}};

    MenuToolbar();

    uie::container_window_v3_config get_window_config() override
    {
        uie::container_window_v3_config config(L"columns_ui_menu_toolbar_DnWbOvEVLmo");
        config.invalidate_children_on_move_or_resize = true;
        return config;
    }

    LRESULT WINAPI handle_toolbar_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT WINAPI s_handle_toolbar_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) noexcept;
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    bool on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp) override;
    void make_menu(int idx);
    void destroy_menu() const { SendMessage(get_wnd(), WM_CANCELMODE, 0, 0); }
    void update_menu_acc() const { PostMessage(get_wnd(), MSG_HIDE_MENUACC, 0, 0); }
    void show_menu_acc() const { PostMessage(get_wnd(), MSG_SHOW_MENUACC, 0, 0); }

    const GUID& get_extension_guid() const override { return extension_guid; }

    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;

    void set_focus() override;
    void show_accelerators() override;
    void hide_accelerators() override;

    bool on_menuchar(unsigned short chr) override;

    unsigned get_type() const override { return ui_extension::type_toolbar; }

    bool is_menu_focused() const override;
    HWND get_previous_focus_window() const override;

private:
    void set_window_theme();

    inline static bool s_hooked{};

    bool m_redrop_menu{true};
    bool m_is_submenu{false};
    int m_wanted_active_item{0};
    int m_actual_active_item{0};
    int m_num_menus_open{-1};
    service_ptr_t<mainmenu_manager> m_manager;
    service_ptr_t<ui_status_text_override> m_status_override;

    HWND m_toolbar_wnd{nullptr};
    HWND m_previous_focus_wnd{nullptr};
    pfc::list_t<MainMenuRootGroup> m_buttons;

    WNDPROC m_menu_proc{nullptr};
    bool m_is_initialised{false};
    bool m_menu_key_pressed{false};
    std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;
};

MenuToolbar::MenuToolbar() : m_manager(nullptr) {}

bool MenuToolbar::is_menu_focused() const
{
    return GetFocus() == m_toolbar_wnd;
}

HWND MenuToolbar::get_previous_focus_window() const
{
    return m_previous_focus_wnd;
}

void MenuToolbar::set_window_theme()
{
    SetWindowTheme(m_toolbar_wnd, cui::colours::is_dark_mode_active() ? L"DarkMode" : nullptr, nullptr);
}

LRESULT WINAPI MenuToolbar::s_handle_toolbar_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) noexcept
{
    auto self = reinterpret_cast<MenuToolbar*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
    return self ? self->handle_toolbar_message(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
}

LRESULT MenuToolbar::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        m_is_initialised = true;

        MainMenuRootGroup::g_get_root_items(m_buttons);
        size_t button_count = m_buttons.get_count();

        std::vector<TBBUTTON> tb_buttons(button_count);

        m_toolbar_wnd = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, nullptr,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT
                | TBSTYLE_LIST | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER,
            0, 0, 0, 25, wnd, reinterpret_cast<HMENU>(static_cast<size_t>(ID_MENU)), core_api::get_my_instance(),
            nullptr);

        if (!m_toolbar_wnd)
            break;

        SetWindowLongPtr(m_toolbar_wnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));

        set_window_theme();
        m_dark_mode_notifier
            = std::make_unique<cui::colours::dark_mode_notifier>([this, self = ptr{this}] { set_window_theme(); });

        SendMessage(m_toolbar_wnd, TB_SETBITMAPSIZE, 0, MAKELONG(0, 0));
        SendMessage(m_toolbar_wnd, TB_SETBUTTONSIZE, 0, MAKELONG(0, 0));

        SendMessage(m_toolbar_wnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

        for (auto&& [n, tb_button] : ranges::views::enumerate(tb_buttons)) {
            tb_button.iBitmap = I_IMAGECALLBACK;
            tb_button.idCommand = gsl::narrow<int>(n + 1);
            tb_button.fsState = TBSTATE_ENABLED;
            tb_button.fsStyle = BTNS_DROPDOWN | BTNS_AUTOSIZE;
            tb_button.dwData = 0;
            tb_button.iString = reinterpret_cast<INT_PTR>(m_buttons[n].m_name_with_accelerators.get_ptr());
        }

        SendMessage(m_toolbar_wnd, TB_ADDBUTTONS, tb_buttons.size(), reinterpret_cast<LPARAM>(tb_buttons.data()));

        BOOL a = true;
        SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
        SendMessage(m_toolbar_wnd, WM_UPDATEUISTATE, MAKEWPARAM(a ? UIS_CLEAR : UIS_SET, UISF_HIDEACCEL), 0);

        m_menu_proc = (WNDPROC)SetWindowLongPtr(m_toolbar_wnd, GWLP_WNDPROC, (LPARAM)s_handle_toolbar_message);

        break;
    }
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            RECT rc = {0, 0, 0, 0};
            size_t count = m_buttons.get_count();
            int cx = lpwp->cx;
            int cy = lpwp->cy;
            int extra = 0;
            if (count && (BOOL)SendMessage(m_toolbar_wnd, TB_GETITEMRECT, count - 1, (LPARAM)(&rc))) {
                cx = std::min(cx, gsl::narrow_cast<int>(rc.right));
                cy = std::min(cy, gsl::narrow_cast<int>(rc.bottom));
                extra = (lpwp->cy - rc.bottom) / 2;
            }
            SetWindowPos(m_toolbar_wnd, nullptr, 0, extra, cx, cy, SWP_NOZORDER);
            RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
        }
        break;
    }

    case WM_NOTIFY: {
        if (((LPNMHDR)lp)->idFrom == ID_MENU) {
            switch (((LPNMHDR)lp)->code) {
            case TBN_HOTITEMCHANGE: {
                if (!(((LPNMTBHOTITEM)lp)->dwFlags & HICF_LEAVING)
                    && (((LPNMTBHOTITEM)lp)->dwFlags & HICF_MOUSE || ((LPNMTBHOTITEM)lp)->dwFlags & HICF_LMOUSE))
                    m_redrop_menu = true;
                break;
            }
            case TBN_DROPDOWN: {
                if (m_redrop_menu)
                    PostMessage(wnd, MSG_CREATE_MENU, ((LPNMTOOLBAR)lp)->iItem, 0);
                else
                    m_redrop_menu = true;

                return TBDDRET_DEFAULT;
            }
            }
        }
        break;
    }
    case MSG_HIDE_MENUACC: {
        BOOL a = true;
        SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
        if ((SendMessage(m_toolbar_wnd, WM_QUERYUISTATE, 0, 0) & UISF_HIDEACCEL) != !a)
            SendMessage(m_toolbar_wnd, WM_UPDATEUISTATE, MAKEWPARAM(a ? UIS_CLEAR : UIS_SET, UISF_HIDEACCEL), 0);
        break;
    }
    case MSG_SHOW_MENUACC: {
        SendMessage(m_toolbar_wnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);
        break;
    }
    case MSG_CREATE_MENU: {
        if (lp)
            SetFocus(m_toolbar_wnd);
        m_wanted_active_item = gsl::narrow_cast<int>(wp);

        make_menu(m_wanted_active_item);
        break;
    }
    case WM_MENUSELECT: {
        if (HIWORD(wp) & MF_POPUP) {
            m_is_submenu = true;
            m_status_override.release();
        } else {
            m_is_submenu = false;
            if (m_manager.is_valid()) {
                unsigned id = LOWORD(wp);

                bool set = false;

                pfc::string8 blah;

                set = m_manager->get_description(id - 1, blah);

                service_ptr_t<ui_status_text_override> p_status_override;

                if (set) {
                    get_host()->override_status_text_create(p_status_override);

                    if (p_status_override.is_valid()) {
                        p_status_override->override_text(blah);
                    }
                }
                m_status_override = p_status_override;
            }
        }
        break;
    }
    case WM_INITMENUPOPUP: {
        m_num_menus_open++;
        break;
    }
    case WM_UNINITMENUPOPUP: {
        m_num_menus_open--;
        break;
    }
    case WM_GETMINMAXINFO: {
        const auto mmi = reinterpret_cast<LPMINMAXINFO>(lp);

        RECT rc = {0, 0, 0, 0};
        SendMessage(m_toolbar_wnd, TB_GETITEMRECT, m_buttons.get_count() - 1, (LPARAM)(&rc));

        mmi->ptMinTrackSize.x = rc.right;
        mmi->ptMinTrackSize.y = rc.bottom;
        mmi->ptMaxTrackSize.y = rc.bottom;
        return 0;
    }
    case WM_SETTINGCHANGE: {
        if (wp == SPI_SETNONCLIENTMETRICS) {
            PostMessage(wnd, MSG_SIZE_LIMIT_CHANGE, 0, 0);
        }
        break;
    }
    case SPI_GETKEYBOARDCUES: {
        BOOL a = TRUE;
        SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
        SendMessage(m_toolbar_wnd, WM_UPDATEUISTATE,
            MAKEWPARAM((a || GetFocus() == m_toolbar_wnd) ? UIS_CLEAR : UIS_SET, UISF_HIDEACCEL), 0);
        break;
    }
    case MSG_SIZE_LIMIT_CHANGE: {
        get_host()->on_size_limit_change(
            wnd, uie::size_limit_minimum_height | uie::size_limit_maximum_height | uie::size_limit_minimum_width);
        break;
    }
    case WM_DESTROY: {
        m_dark_mode_notifier.reset();
        DestroyWindow(m_toolbar_wnd);
        m_toolbar_wnd = nullptr;
        m_buttons.remove_all();
        m_is_initialised = false;
        break;
    }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

LRESULT WINAPI MenuToolbar::handle_toolbar_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_KILLFOCUS: {
        m_menu_key_pressed = false;
        update_menu_acc();
        m_previous_focus_wnd = nullptr;
    } break;
    case WM_SETFOCUS: {
        m_menu_key_pressed = false;
        show_menu_acc();
        m_previous_focus_wnd = (HWND)wp;
    } break;
    case WM_CHAR:
        if (wp == ' ')
            return 0;
        break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN: {
        auto lpkeyb = uih::GetKeyboardLParam(lp);
        if ((wp == VK_ESCAPE || (wp == VK_F10 && !HIBYTE(GetKeyState(VK_SHIFT))) || wp == VK_MENU)
            && !lpkeyb.previous_key_state) {
            update_menu_acc();
            if (wp == VK_ESCAPE) {
                if (m_previous_focus_wnd && IsWindow(m_previous_focus_wnd))
                    SetFocus(m_previous_focus_wnd);
            } else {
                m_menu_key_pressed = true;
                PostMessage(wnd, TB_SETHOTITEM, -1, 0);
            }
            return 0;
        }
        if ((wp == VK_SPACE) && !lpkeyb.previous_key_state) {
            HWND wndparent = uFindParentPopup(wnd);
            if (wndparent)
                PostMessage(wndparent, WM_SYSKEYDOWN, wp, lp | (1 << 29));
            return 0;
        }
        break;
    }
    case WM_SYSKEYUP: {
        if (m_menu_key_pressed && (wp == VK_MENU || wp == VK_F10)) {
            if (m_previous_focus_wnd && IsWindow(m_previous_focus_wnd))
                SetFocus(m_previous_focus_wnd);
            m_menu_key_pressed = false;
            return 0;
        }
        break;
    }
    }
    return CallWindowProc(m_menu_proc, wnd, msg, wp, lp);
}

void MenuToolbar::make_menu(int idx)
{
    if (idx == m_actual_active_item || s_hooked || idx < 1 || idx > gsl::narrow<int>(m_buttons.get_count()))
        return;

    // The menu command may cause the menu toolbar to be destroyed
    ptr self{this};

    m_actual_active_item = idx;

    RECT rc;
    POINT pt;

    SendMessage(m_toolbar_wnd, TB_GETRECT, idx, (LPARAM)&rc);

    MapWindowPoints(m_toolbar_wnd, HWND_DESKTOP, (LPPOINT)&rc, 2);

    pt.x = rc.left;
    pt.y = rc.bottom;

    HMENU menu = CreatePopupMenu();

    s_hooked = true;
    register_message_hook(uih::MessageHookType::type_message_filter, this);
    service_ptr_t<mainmenu_manager> p_menu = standard_api_create_t<mainmenu_manager>();

    bool b_keyb = standard_config_objects::query_show_keyboard_shortcuts_in_menus();

    if (p_menu.is_valid()) {
        p_menu->instantiate(m_buttons[idx - 1].m_guid);
        p_menu->generate_menu_win32(menu, 1, -1, b_keyb ? mainmenu_manager::flag_show_shortcuts : 0);
        menu_helpers::win32_auto_mnemonics(menu);
    }

    SendMessage(m_toolbar_wnd, TB_PRESSBUTTON, idx, TRUE);
    SendMessage(m_toolbar_wnd, TB_SETHOTITEM, idx - 1, 0);
    m_is_submenu = false;

    m_num_menus_open = -1;

    m_manager = p_menu;

    TPMPARAMS tpmp{};
    tpmp.cbSize = sizeof(tpmp);

    GetWindowRect(m_toolbar_wnd, &tpmp.rcExclude);

    HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    if (monitor) {
        MONITORINFO mi{};
        mi.cbSize = sizeof(MONITORINFO);
        if (uGetMonitorInfo(monitor, &mi)) {
            if (pt.x < mi.rcMonitor.left)
                pt.x = mi.rcMonitor.left;
            tpmp.rcExclude.left = mi.rcMonitor.left;
            tpmp.rcExclude.right = mi.rcMonitor.right;
        }
    }

    if (GetFocus() != m_toolbar_wnd)
        m_previous_focus_wnd = GetFocus();

    int cmd = TrackPopupMenuEx(menu, TPM_LEFTBUTTON | TPM_RETURNCMD, pt.x, pt.y, get_wnd(), &tpmp);
    m_status_override.release();

    PostMessage(m_toolbar_wnd, TB_PRESSBUTTON, idx, FALSE);

    if (GetFocus() != m_toolbar_wnd)
        PostMessage(m_toolbar_wnd, TB_SETHOTITEM, -1, 0);

    DestroyMenu(menu);

    if (cmd > 0 && p_menu.is_valid()) {
        if (IsWindow(m_previous_focus_wnd))
            SetFocus(m_previous_focus_wnd);
        p_menu->execute_command(cmd - 1);
    }

    m_manager.release();
    p_menu.release();

    deregister_message_hook(uih::MessageHookType::type_message_filter, this);
    s_hooked = false;
    m_actual_active_item = 0;
}

bool MenuToolbar::on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp)
{
    static POINT last_pt;

    if (code == MSGF_MENU) {
        switch (((MSG*)lp)->message) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            SendMessage(m_toolbar_wnd, TB_SETHOTITEM, -1, 0);
            if (((MSG*)lp)->message == WM_LBUTTONDOWN) {
                POINT pt;
                RECT toolbar;
                GetClientRect(get_wnd(), &toolbar);
                pt.y = GET_Y_LPARAM(((MSG*)lp)->lParam);
                pt.x = GET_X_LPARAM(((MSG*)lp)->lParam);

                if (ScreenToClient(m_toolbar_wnd, &pt) && PtInRect(&toolbar, pt)) {
                    size_t idx = SendMessage(m_toolbar_wnd, TB_HITTEST, 0, reinterpret_cast<LPARAM>(&pt));

                    if (idx >= 0 && idx < m_buttons.get_count())
                        m_redrop_menu = false;
                }
            }
            break;
        }
        case WM_KEYDOWN: {
            switch (((MSG*)lp)->wParam) {
            case VK_LEFT:
                if (!m_num_menus_open) {
                    destroy_menu();
                    if (m_wanted_active_item == 1)
                        m_wanted_active_item = gsl::narrow<int>(m_buttons.get_count());
                    else
                        --m_wanted_active_item;
                    PostMessage(get_wnd(), MSG_CREATE_MENU, m_wanted_active_item, 0);
                }

                break;
            case VK_RIGHT:
                if (!m_is_submenu) {
                    destroy_menu();
                    if (m_wanted_active_item == gsl::narrow<int>(m_buttons.get_count()))
                        m_wanted_active_item = 1;
                    else
                        ++m_wanted_active_item;
                    PostMessage(get_wnd(), MSG_CREATE_MENU, m_wanted_active_item, 0);
                }
                break;
            case VK_ESCAPE:
                if (!m_num_menus_open) {
                    SetFocus(m_toolbar_wnd);
                    SendMessage(m_toolbar_wnd, TB_SETHOTITEM, m_wanted_active_item - 1, 0);
                    destroy_menu();
                }
                break;
            }
            break;
        }

        case WM_MOUSEMOVE: {
            POINT px;
            px.y = GET_Y_LPARAM(((MSG*)lp)->lParam);
            px.x = GET_X_LPARAM(((MSG*)lp)->lParam);
            if (px.x != last_pt.x || px.y != last_pt.y) {
                HWND wnd_hit = WindowFromPoint(px);
                if (wnd_hit == m_toolbar_wnd) {
                    POINT pt = px;

                    if (ScreenToClient(m_toolbar_wnd, &pt)) {
                        const int idx = static_cast<int>(
                            SendMessage(m_toolbar_wnd, TB_HITTEST, 0, reinterpret_cast<LPARAM>(&pt)));

                        if (idx >= 0 && idx < gsl::narrow<int>(m_buttons.get_count())
                            && (m_wanted_active_item - 1) != idx) {
                            destroy_menu();
                            m_wanted_active_item = idx + 1;
                            PostMessage(get_wnd(), MSG_CREATE_MENU, m_wanted_active_item, 0);
                        }
                    }
                }
            }
            last_pt = px;
            break;
        }
        }
    }
    return false;
}

void MenuToolbar::get_name(pfc::string_base& out) const
{
    out.set_string("Menu");
}

void MenuToolbar::get_category(pfc::string_base& out) const
{
    out.set_string("Toolbars");
}

void MenuToolbar::set_focus()
{
    SetFocus(m_toolbar_wnd);
}

void MenuToolbar::show_accelerators()
{
    show_menu_acc();
}

void MenuToolbar::hide_accelerators()
{
    if (GetFocus() != m_toolbar_wnd)
        update_menu_acc();
}

bool MenuToolbar::on_menuchar(unsigned short chr)
{
    UINT id{};

    if (SendMessage(m_toolbar_wnd, TB_MAPACCELERATOR, chr, (LPARAM)&id)) {
        PostMessage(get_wnd(), MSG_CREATE_MENU, id, TRUE);
        return true;
    }

    return false;
}

namespace {

ui_extension::window_factory<MenuToolbar> _menu_toolbar_factory;

}

} // namespace cui::toolbars::menu
