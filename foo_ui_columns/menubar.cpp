#include "stdafx.h"

#include "menu_mnemonics.h"

cfg_int cfg_fullsizemenu(GUID{0xe880f267, 0x73de, 0x7952, {0x5b, 0x79, 0xb5, 0xda, 0x77, 0x28, 0x6d, 0xb6}}, 0);

#define ID_MENU 2001

enum {
    MSG_HIDE_MENUACC = WM_USER + 1,
    MSG_SHOW_MENUACC = WM_USER + 2,
    MSG_CREATE_MENU = WM_USER + 3,
    MSG_SIZE_LIMIT_CHANGE
};

class MainMenuRootGroup {
public:
    pfc::string8 m_name;
    pfc::array_t<TCHAR> m_name_with_accelerators;
    GUID m_guid{};
    t_uint32 m_sort_priority{NULL};

    MainMenuRootGroup() = default;

    static t_size g_compare(const MainMenuRootGroup& p_item1, const MainMenuRootGroup& p_item2)
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
    : public ui_extension::container_uie_window_t<uie::menu_window_v2>
    , uih::MessageHook {
public:
    //    static pfc::ptr_list_t<playlists_list_window> list_wnd;
    // static HHOOK msghook;
    static bool hooked;
    // static menu_extension * p_hooked_menu;

    bool redrop{true};
    bool is_submenu{false};
    int active_item{0};
    int actual_active{0};
    int sub_menu_ref_count{-1};
    service_ptr_t<mainmenu_manager> p_manager;
    service_ptr_t<ui_status_text_override> m_status_override;

    class_data& get_class_data() const override { __implement_get_class_data_child_ex(class_name, true, false); }

    HWND wnd_menu{nullptr};
    HWND wnd_prev_focus{nullptr};
    pfc::list_t<MainMenuRootGroup> m_buttons;

    LRESULT WINAPI hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT WINAPI main_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    bool on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp) override;

    void make_menu(unsigned idx);

    void destroy_menu() const { SendMessage(get_wnd(), WM_CANCELMODE, 0, 0); }

    void update_menu_acc() const { uPostMessage(get_wnd(), MSG_HIDE_MENUACC, 0, 0); }

    void show_menu_acc() const { uPostMessage(get_wnd(), MSG_SHOW_MENUACC, 0, 0); }

    static const GUID extension_guid;

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

    MenuToolbar();
    ~MenuToolbar();

private:
    void set_window_theme();

    static const TCHAR* class_name;

    WNDPROC menuproc{nullptr};
    bool initialised{false};
    bool m_menu_key_pressed{false};
    std::unique_ptr<cui::colours::dark_mode_notifier> m_dark_mode_notifier;
};

bool MenuToolbar::hooked = false;

MenuToolbar::MenuToolbar() : p_manager(nullptr) {}

MenuToolbar::~MenuToolbar() = default;

const TCHAR* MenuToolbar::class_name = _T("{76E6DB50-0DE3-4f30-A7E4-93FD628B1401}");

bool MenuToolbar::is_menu_focused() const
{
    return GetFocus() == wnd_menu;
}

HWND MenuToolbar::get_previous_focus_window() const
{
    return wnd_prev_focus;
}

void MenuToolbar::set_window_theme()
{
    SetWindowTheme(wnd_menu, cui::colours::is_dark_mode_active() ? L"DarkMode" : nullptr, nullptr);
}

LRESULT WINAPI MenuToolbar::main_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto p_this = reinterpret_cast<MenuToolbar*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
    return p_this ? p_this->hook(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
}

LRESULT MenuToolbar::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        initialised = true;

        MainMenuRootGroup::g_get_root_items(m_buttons);
        t_size button_count = m_buttons.get_count();

        pfc::array_t<TBBUTTON> tbb;
        tbb.set_size(button_count);
        memset(tbb.get_ptr(), 0, tbb.get_size() * sizeof(TBBUTTON));

        wnd_menu = CreateWindowEx(/*TBSTYLE_EX_MIXEDBUTTONS|*/ WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, nullptr,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT
                | TBSTYLE_LIST | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER,
            0, 0, 0, 25, wnd, (HMENU)ID_MENU, core_api::get_my_instance(), nullptr);

        if (wnd_menu) {
            SetWindowLongPtr(wnd_menu, GWLP_USERDATA, (LPARAM)(this));

            set_window_theme();
            m_dark_mode_notifier
                = std::make_unique<cui::colours::dark_mode_notifier>([this, self = ptr{this}] { set_window_theme(); });

            SendMessage(wnd_menu, TB_SETBITMAPSIZE, (WPARAM)0, MAKELONG(0, 0));
            SendMessage(wnd_menu, TB_SETBUTTONSIZE, (WPARAM)0, MAKELONG(0, /*GetSystemMetrics(SM_CYMENUSIZE)*/ 0));

            SendMessage(wnd_menu, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

            unsigned count = tbb.get_size();
            for (unsigned n = 0; n < count; n++) {
                tbb[n].iBitmap = I_IMAGECALLBACK;
                tbb[n].idCommand = n + 1;
                tbb[n].fsState = TBSTATE_ENABLED;
                tbb[n].fsStyle = BTNS_DROPDOWN | BTNS_AUTOSIZE;
                tbb[n].dwData = 0;
                tbb[n].iString = reinterpret_cast<INT_PTR>(m_buttons[n].m_name_with_accelerators.get_ptr());
            }

            SendMessage(wnd_menu, TB_ADDBUTTONS, (WPARAM)tbb.get_size(), (LPARAM)(LPTBBUTTON)tbb.get_ptr());

            //            SendMessage(wnd_menu, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);

            //            SendMessage(wnd_menu, TB_AUTOSIZE, 0, 0);

            BOOL a = true;
            SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
            SendMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM(a ? UIS_CLEAR : UIS_SET, UISF_HIDEACCEL), 0);

            //            SendMessage(wnd_menu, TB_SETPARENT, (WPARAM) (HWND)wnd_host, 0);
            menuproc = (WNDPROC)SetWindowLongPtr(wnd_menu, GWLP_WNDPROC, (LPARAM)main_hook);
        }

        break;
    }
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            // SIZE sz = {0,0};
            // SendMessage(wnd_menu, TB_GETMAXSIZE, NULL, (LPARAM)&sz);

            RECT rc = {0, 0, 0, 0};
            t_size count = m_buttons.get_count();
            int cx = lpwp->cx;
            int cy = lpwp->cy;
            int extra = 0;
            if (count && (BOOL)SendMessage(wnd_menu, TB_GETITEMRECT, count - 1, (LPARAM)(&rc))) {
                cx = std::min(cx, gsl::narrow_cast<int>(rc.right));
                cy = std::min(cy, gsl::narrow_cast<int>(rc.bottom));
                extra = (lpwp->cy - rc.bottom) / 2;
            }
            SetWindowPos(wnd_menu, nullptr, 0, extra, cx, cy, SWP_NOZORDER);
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
                    redrop = true;
                break;
            }
            case TBN_DROPDOWN: {
                if (redrop)
                    PostMessage(wnd, MSG_CREATE_MENU, ((LPNMTOOLBAR)lp)->iItem, 0);
                else
                    redrop = true;

                return TBDDRET_DEFAULT;
            }
            }
        }
        break;
    }
    case MSG_HIDE_MENUACC: {
        BOOL a = true;
        SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
        if ((SendMessage(wnd_menu, WM_QUERYUISTATE, 0, 0) & UISF_HIDEACCEL) != !a)
            SendMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM(a ? UIS_CLEAR : UIS_SET, UISF_HIDEACCEL), 0);
        break;
    }
    case MSG_SHOW_MENUACC: {
        SendMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);
        break;
    }
    case MSG_CREATE_MENU: {
        if (lp)
            SetFocus(wnd_menu);
        active_item = wp;

        make_menu(wp);
        break;
    }
    case WM_MENUSELECT: {
        if (HIWORD(wp) & MF_POPUP) {
            is_submenu = true;
            m_status_override.release();
        } else {
            is_submenu = false;
            if (p_manager.is_valid()) {
                unsigned id = LOWORD(wp);

                bool set = false;

                pfc::string8 blah;

                set = p_manager->get_description(id - 1, blah);

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
        sub_menu_ref_count++;
        break;
    }
    case WM_UNINITMENUPOPUP: {
        sub_menu_ref_count--;
        break;
    }
    case WM_GETMINMAXINFO: {
        auto mmi = LPMINMAXINFO(lp);

        RECT rc = {0, 0, 0, 0};
        SendMessage(wnd_menu, TB_GETITEMRECT, m_buttons.get_count() - 1, (LPARAM)(&rc));

        // SIZE sz = {0,0};
        // SendMessage(wnd_menu, TB_GETMAXSIZE, NULL, (LPARAM)&sz);
        // console::formatter() << sz.cx << sz.cy;

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
        SendMessage(wnd_menu, WM_UPDATEUISTATE,
            MAKEWPARAM((a || GetFocus() == wnd_menu) ? UIS_CLEAR : UIS_SET, UISF_HIDEACCEL), 0);
        break;
    }
    case MSG_SIZE_LIMIT_CHANGE: {
        get_host()->on_size_limit_change(
            wnd, uie::size_limit_minimum_height | uie::size_limit_maximum_height | uie::size_limit_minimum_width);
        break;
    }
    case WM_DESTROY: {
        m_dark_mode_notifier.reset();
        DestroyWindow(wnd_menu);
        wnd_menu = nullptr;
        m_buttons.remove_all();
        initialised = false;
        break;
    }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

LRESULT WINAPI MenuToolbar::hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_KILLFOCUS: {
        m_menu_key_pressed = false;
        update_menu_acc();
        wnd_prev_focus = nullptr;
    } break;
    case WM_SETFOCUS: {
        m_menu_key_pressed = false;
        show_menu_acc();
        wnd_prev_focus = (HWND)wp;
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
                if (wnd_prev_focus && IsWindow(wnd_prev_focus))
                    SetFocus(wnd_prev_focus);
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
    } break;
    case WM_SYSKEYUP: {
        if (m_menu_key_pressed && (wp == VK_MENU || wp == VK_F10)) {
            if (wnd_prev_focus && IsWindow(wnd_prev_focus))
                SetFocus(wnd_prev_focus);
            m_menu_key_pressed = false;
            return 0;
        }
    } break;
    }
    return CallWindowProc(menuproc, wnd, msg, wp, lp);
}

void MenuToolbar::make_menu(unsigned idx)
{
    if (idx == actual_active || hooked || idx < 1 || idx > m_buttons.get_count())
        return;

    service_ptr_t<MenuToolbar> dummy = this; // menu command may delete us

    actual_active = idx;

    RECT rc;
    POINT pt;

    SendMessage(wnd_menu, TB_GETRECT, idx, (LPARAM)&rc);

    MapWindowPoints(wnd_menu, HWND_DESKTOP, (LPPOINT)&rc, 2);

    pt.x = rc.left;
    pt.y = rc.bottom;

    HMENU menu = CreatePopupMenu();

    hooked = true;
    //    p_hooked_menu = this;
    register_message_hook(uih::MessageHookType::type_message_filter, this);
    // msghook = uSetWindowsHookEx(WH_MSGFILTER, menu_hook_t_proc, 0, GetCurrentThreadId());
    service_ptr_t<mainmenu_manager> p_menu = standard_api_create_t<mainmenu_manager>();

    bool b_keyb = standard_config_objects::query_show_keyboard_shortcuts_in_menus();

    if (p_menu.is_valid()) {
        p_menu->instantiate(m_buttons[idx - 1].m_guid);
        p_menu->generate_menu_win32(menu, 1, -1, b_keyb ? mainmenu_manager::flag_show_shortcuts : 0);
        menu_helpers::win32_auto_mnemonics(menu);
    }

    SendMessage(wnd_menu, TB_PRESSBUTTON, idx, TRUE);
    SendMessage(wnd_menu, TB_SETHOTITEM, idx - 1, 0);
    is_submenu = false;

    sub_menu_ref_count = -1; // we get a notificationwhen the inital menu is created

    //    RECT rc_desktop;
    //    if (GetWindowRect(GetDesktopWindow(), &rc_desktop))

    //    if (pt.x < rc_desktop.left) pt.x = rc_desktop.left;
    //    else if (pt.x > rc_desktop.right) pt.x = rc_desktop.right;

    p_manager = p_menu;

    TPMPARAMS tpmp{};
    tpmp.cbSize = sizeof(tpmp);

    GetWindowRect(wnd_menu, &tpmp.rcExclude);

    HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    if (mon) {
        MONITORINFO mi{};
        mi.cbSize = sizeof(MONITORINFO);
        if (uGetMonitorInfo(mon, &mi)) {
            if (pt.x < mi.rcMonitor.left)
                pt.x = mi.rcMonitor.left;
            tpmp.rcExclude.left = mi.rcMonitor.left;
            tpmp.rcExclude.right = mi.rcMonitor.right;
        }
    }

    //    menu_info::MENU_A_BASE = 1;
    //    menu_info::MENU_B_BASE = -1;

    if (GetFocus() != wnd_menu)
        wnd_prev_focus = GetFocus();

    int cmd = TrackPopupMenuEx(menu, TPM_LEFTBUTTON | TPM_RETURNCMD, pt.x, pt.y, get_wnd(), &tpmp);
    //    int cmd = TrackPopupMenuEx(menu,TPM_LEFTBUTTON|TPM_RETURNCMD,pt.x,pt.y,g_main_window,0);

    m_status_override.release();

    // SendMessage(wnd_menu, TB_PRESSBUTTON, idx, FALSE);

    PostMessage(wnd_menu, TB_PRESSBUTTON, idx, FALSE);
    if (GetFocus() != wnd_menu) {
        PostMessage(wnd_menu, TB_SETHOTITEM, -1, 0);
    }

    DestroyMenu(menu);

    if (cmd > 0 && p_menu.is_valid()) {
        //        SendMessage(wnd_menu, TB_SETHOTITEM, -1, 0);
        if (IsWindow(wnd_prev_focus))
            SetFocus(wnd_prev_focus);
        p_menu->execute_command(cmd - 1);
    }

    //    if (p_menu.is_valid())
    {
        p_manager.release();
        p_menu.release();
    }

    // UnhookWindowsHookEx(msghook); // hook may not be freed instantly, so dont make msghook = 0
    deregister_message_hook(uih::MessageHookType::type_message_filter, this);
    hooked = false;
    // p_hooked_menu=0;

    actual_active = 0;
}

bool MenuToolbar::on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp)
{
    static POINT last_pt;

    if (code == MSGF_MENU) {
        switch (((MSG*)lp)->message) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            SendMessage(wnd_menu, TB_SETHOTITEM, -1, 0);
            if (((MSG*)lp)->message == WM_LBUTTONDOWN) {
                POINT pt;
                RECT toolbar;
                GetClientRect(get_wnd(), &toolbar);
                pt.y = GET_Y_LPARAM(((MSG*)lp)->lParam);
                pt.x = GET_X_LPARAM(((MSG*)lp)->lParam);

                if (ScreenToClient(wnd_menu, &pt) && PtInRect(&toolbar, pt)) {
                    t_size idx = SendMessage(wnd_menu, TB_HITTEST, 0, reinterpret_cast<LPARAM>(&pt));

                    if (idx >= 0 && idx < m_buttons.get_count())
                        redrop = false;
                }
            }
        } break;
        case WM_KEYDOWN: {
            switch (((MSG*)lp)->wParam) {
            case VK_LEFT:
                if (!sub_menu_ref_count) {
                    destroy_menu();
                    if (active_item == 1)
                        active_item = m_buttons.get_count();
                    else
                        active_item--;
                    uPostMessage(get_wnd(), MSG_CREATE_MENU, active_item, 0);
                }

                break;
            case VK_RIGHT:
                if (!is_submenu) {
                    destroy_menu();
                    if (active_item == m_buttons.get_count())
                        active_item = 1;
                    else
                        active_item++;
                    uPostMessage(get_wnd(), MSG_CREATE_MENU, active_item, 0);
                }
                break;
            case VK_ESCAPE:
                if (!sub_menu_ref_count) {
                    SetFocus(wnd_menu);
                    SendMessage(wnd_menu, TB_SETHOTITEM, active_item - 1, 0);
                    destroy_menu();
                }
                break;
            }
        } break;

        case WM_MOUSEMOVE: {
            POINT px;
            px.y = GET_Y_LPARAM(((MSG*)lp)->lParam);
            px.x = GET_X_LPARAM(((MSG*)lp)->lParam);
            if (px.x != last_pt.x || px.y != last_pt.y) {
                HWND wnd_hit = WindowFromPoint(px);
                if (wnd_hit == wnd_menu) {
                    POINT pt = px;
                    int hot_item = SendMessage(wnd_menu, TB_GETHOTITEM, 0, 0);
                    if (ScreenToClient(wnd_menu, &pt)) {
                        t_size idx = SendMessage(wnd_menu, TB_HITTEST, 0, reinterpret_cast<LPARAM>(&pt));

                        if (idx >= 0 && idx < m_buttons.get_count() && (active_item - 1) != idx) {
                            destroy_menu();
                            active_item = idx + 1;
                            uPostMessage(get_wnd(), MSG_CREATE_MENU, active_item, 0);
                        }
                    }
                }
            }
            last_pt = px;
        } break;
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
    {
        SetFocus(wnd_menu);
    }
}
void MenuToolbar::show_accelerators()
{
    {
        show_menu_acc();
    }
}
void MenuToolbar::hide_accelerators()
{
    {
        if (GetFocus() != wnd_menu)
            update_menu_acc();
    }
}

bool MenuToolbar::on_menuchar(unsigned short chr)
{
    {
        UINT id;
        if (SendMessage(wnd_menu, TB_MAPACCELERATOR, chr, (LPARAM)&id)) {
            uPostMessage(get_wnd(), MSG_CREATE_MENU, id, TRUE);
            return true;
        }
    }
    return false;
}

// {76E6DB50-0DE3-4f30-A7E4-93FD628B1401}
const GUID MenuToolbar::extension_guid = {0x76e6db50, 0xde3, 0x4f30, {0xa7, 0xe4, 0x93, 0xfd, 0x62, 0x8b, 0x14, 0x1}};

ui_extension::window_factory<MenuToolbar> blah;
