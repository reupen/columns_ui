#include "pch.h"
#include "buttons.h"

#include "dark_mode.h"
#include "dark_mode_dialog.h"
#include "menu_items.h"

#define ID_BUTTONS 2001

namespace cui::toolbars::buttons {

namespace {

void update_button_state(HWND toolbar_wnd, int button_id, bool is_enabled, bool is_pressed)
{
    auto tb_state = LOWORD(SendMessage(toolbar_wnd, TB_GETSTATE, button_id, 0));

    if (is_enabled)
        tb_state |= TBSTATE_ENABLED;
    else
        tb_state &= ~TBSTATE_ENABLED;

    if (is_pressed)
        tb_state |= TBSTATE_PRESSED;
    else
        tb_state &= ~TBSTATE_PRESSED;

    SendMessage(toolbar_wnd, TB_SETSTATE, button_id, MAKELONG(tb_state, 0));
}

} // namespace

void ButtonsToolbar::ButtonStateCallback::on_button_state_change(unsigned p_new_state)
{
    const auto is_enabled = (p_new_state & uie::BUTTON_STATE_ENABLED) != 0;
    const auto is_pressed = (p_new_state & uie::BUTTON_STATE_PRESSED) != 0;

    update_button_state(m_toolbar_wnd, m_button_id, is_enabled, is_pressed);
}

void ButtonsToolbar::MainMenuStateCallback::menu_state_changed(const GUID& main, const GUID& sub)
{
    pfc::string8 _;
    uint32_t flags{};
    m_mainmenu_commands_v3->get_display(m_command_index, _, flags);

    const auto is_enabled = (flags & mainmenu_commands::flag_disabled) == 0;
    const auto is_pressed = (flags & mainmenu_commands::flag_checked) != 0;

    update_button_state(m_toolbar_wnd, m_button_id, is_enabled, is_pressed);
}

const GUID& ButtonsToolbar::get_extension_guid() const
{
    return extension_guid;
}

ButtonsToolbar::ConfigParam::ConfigParam() : m_button_list(*this) {}

void ButtonsToolbar::get_menu_items(ui_extension::menu_hook_t& p_hook)
{
    ui_extension::menu_node_ptr p_node(new uie::menu_node_configure(this, "Buttons options"));
    p_hook.add_node(p_node);
}

unsigned ButtonsToolbar::get_type() const
{
    return ui_extension::type_toolbar;
}

void ButtonsToolbar::import_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    ConfigParam param;
    param.m_selection = nullptr;
    param.m_active = 0;
    param.m_text_below = m_text_below;
    param.m_appearance = m_appearance;
    param.m_icon_size = m_icon_size;
    param.m_width = m_width;
    param.m_height = m_height;
    param.import_from_stream(p_reader, false, p_abort);

    configure(
        param.m_buttons, param.m_text_below, param.m_appearance, param.m_icon_size, param.m_width, param.m_height);
}

void ButtonsToolbar::export_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    ConfigParam param;
    param.m_selection = nullptr;
    param.m_buttons = m_buttons;
    param.m_active = 0;
    param.m_text_below = m_text_below;
    param.m_appearance = m_appearance;
    param.m_icon_size = m_icon_size;
    param.m_width = m_width;
    param.m_height = m_height;
    param.export_to_stream(p_writer, false, p_abort);
}

void ButtonsToolbar::reset_buttons(std::vector<Button>& p_buttons)
{
    const std::initializer_list<std::tuple<GUID, Type, Show, const char*>> default_buttons{
        {standard_commands::guid_main_stop, TYPE_MENU_ITEM_MAIN, SHOW_IMAGE, nullptr},
        {standard_commands::guid_main_pause, TYPE_MENU_ITEM_MAIN, SHOW_IMAGE, nullptr},
        {standard_commands::guid_main_play, TYPE_MENU_ITEM_MAIN, SHOW_IMAGE, nullptr},
        {standard_commands::guid_main_previous, TYPE_MENU_ITEM_MAIN, SHOW_IMAGE, nullptr},
        {standard_commands::guid_main_next, TYPE_MENU_ITEM_MAIN, SHOW_IMAGE, nullptr},
        {standard_commands::guid_main_random, TYPE_MENU_ITEM_MAIN, SHOW_IMAGE, nullptr},
        {{}, TYPE_SEPARATOR, SHOW_IMAGE, nullptr},
        {standard_commands::guid_main_open, TYPE_MENU_ITEM_MAIN, SHOW_IMAGE, nullptr},
        {{}, TYPE_SEPARATOR, SHOW_IMAGE, nullptr},
        {main_menu::commands::toggle_live_editing_id, TYPE_MENU_ITEM_MAIN, SHOW_TEXT, "Live layout editing"},
    };

    p_buttons.clear();

    for (auto&& default_button : default_buttons) {
        const auto& [guid, type, show, text] = default_button;
        Button temp{};
        temp.m_type = type;
        temp.m_show = show;
        temp.m_guid = guid;
        if (text) {
            temp.m_use_custom_text = true;
            temp.m_text = text;
        }
        p_buttons.emplace_back(std::move(temp));
    }
}

ButtonsToolbar::ButtonsToolbar()
{
    reset_buttons(m_buttons);
}

ButtonsToolbar::~ButtonsToolbar() = default;

const TCHAR* ButtonsToolbar::class_name = _T("{D75D4E2D-603B-4699-9C49-64DDFFE56A16}");

void ButtonsToolbar::create_toolbar()
{
    const auto is_dark = colours::is_dark_mode_active();

    std::vector<TBBUTTON> tbbuttons(m_buttons.size());
    std::vector<ButtonImage> images(m_buttons.size());
    std::vector<ButtonImage> images_hot(m_buttons.size());

    RECT rc;
    GetClientRect(wnd_host, &rc);

    wnd_toolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, nullptr,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT
            | (!m_text_below && m_appearance != APPEARANCE_NOEDGE ? TBSTYLE_LIST : 0) | TBSTYLE_TRANSPARENT
            | TBSTYLE_TOOLTIPS | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER,
        0, 0, rc.right, rc.bottom, wnd_host, (HMENU)ID_BUTTONS, core_api::get_my_instance(), nullptr);

    COLORREF colour_btntext = dark::get_system_colour(COLOR_BTNTEXT, colours::is_dark_mode_active());

    if (wnd_toolbar) {
        set_window_theme();

        bool b_need_hot = false;

        for (auto&& button : m_buttons) {
            if (button.m_use_custom_hot) {
                b_need_hot = true;
                break;
            }
        }

        const bool is_custom_size = m_icon_size == IconSize::Custom;
        int button_width{};
        int button_height{};
        size_t image_count{};

        for (auto&& [n, button] : ranges::views::enumerate(m_buttons)) {
            if (button.m_type == TYPE_SEPARATOR)
                continue;

            for (const auto& instance : uie::button::enumerate()) {
                if (instance->get_item_guid() == button.m_guid) {
                    button.m_interface = instance;
                    break;
                }
            }

            if (button.m_type == TYPE_MENU_ITEM_MAIN && button.m_subcommand == GUID{}) {
                mainmenu_commands::ptr mainmenu_commands_instance;
                uint32_t command_index{};
                if (menu_item_resolver::g_resolve_main_command(button.m_guid, mainmenu_commands_instance, command_index)
                    && mainmenu_commands_instance->service_query_t(button.m_mainmenu_commands_v3)) {
                    button.m_mainmenu_commands_index = command_index;
                }
            }

            if (button.m_show != SHOW_IMAGE && button.m_show != SHOW_IMAGE_TEXT)
                continue;

            ++image_count;

            if (button.m_use_custom_hot)
                images_hot[n].preload(button.m_custom_hot_image);

            if (button.m_use_custom) {
                images[n].preload(button.m_custom_image);

                if (!is_custom_size) {
                    const auto [image_width, image_height] = images[n].get_size().value_or(std::make_tuple(0, 0));
                    button_width = std::max(button_width, image_width);
                    button_height = std::max(button_height, image_height);
                }
            }
        }

        if (button_width == 0 && button_height == 0) {
            button_width = is_custom_size ? m_width.get_scaled_value() : GetSystemMetrics(SM_CXSMICON);
            button_height = is_custom_size ? m_height.get_scaled_value() : GetSystemMetrics(SM_CYSMICON);
        }

        bool any_images_resized{};
        bool any_hot_images_resized{};

        for (auto&& [n, button] : ranges::views::enumerate(m_buttons)) {
            const auto resized = images[n].load(
                button.m_use_custom ? std::make_optional(std::ref(button.m_custom_image)) : std::nullopt,
                button.m_interface, colour_btntext, button_width, button_height);

            any_images_resized = any_images_resized || resized;

            if (button.m_use_custom_hot) {
                const auto resized_hot = images_hot[n].load(
                    button.m_custom_hot_image, nullptr, colour_btntext, button_width, button_height);

                any_hot_images_resized = any_hot_images_resized || resized_hot;
            }
        }

        if (any_images_resized)
            console::print(fmt::format(
                "Buttons toolbar – resized some custom non-hover icons to {} x {}px", button_width, button_height)
                    .c_str());

        if (any_hot_images_resized)
            console::print(fmt::format(
                "Buttons toolbar – resized some custom hover icons to {} x {}px", button_width, button_height)
                    .c_str());

        m_standard_images.reset(
            ImageList_Create(button_width, button_height, ILC_COLOR32 | ILC_MASK, gsl::narrow<int>(image_count), 0));
        if (b_need_hot)
            m_hot_images.reset(ImageList_Create(
                button_width, button_height, ILC_COLOR32 | ILC_MASK, gsl::narrow<int>(image_count), 0));

        for (auto&& [index, tuple] :
            ranges::views::enumerate(ranges::views::zip(tbbuttons, m_buttons, images, images_hot))) {
            auto&& [tbbutton, button, image, hot_image] = tuple;
            tbbutton.iString = -1; //"It works"

            if (button.m_type == TYPE_SEPARATOR) {
                tbbutton.idCommand = gsl::narrow<int>(index);
                tbbutton.fsStyle = is_dark ? BTNS_BUTTON | BTNS_SHOWTEXT : BTNS_SEP;
                tbbutton.iBitmap = I_IMAGENONE;
            } else {
                if (button.m_show == SHOW_IMAGE || button.m_show == SHOW_IMAGE_TEXT) {
                    tbbutton.iBitmap = image.add_to_imagelist(m_standard_images.get());
                    if (!button.m_use_custom_hot || !hot_image.is_valid())
                        image.add_to_imagelist(m_hot_images.get());
                    else
                        hot_image.add_to_imagelist(m_hot_images.get());
                } else {
                    tbbutton.iBitmap = I_IMAGENONE;
                }

                tbbutton.idCommand = gsl::narrow<int>(index);
                tbbutton.fsState = 0;
                tbbutton.fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;

                if (!m_text_below && m_appearance != APPEARANCE_NOEDGE
                    && (button.m_show == SHOW_TEXT || button.m_show == SHOW_IMAGE_TEXT))
                    tbbutton.fsStyle |= BTNS_SHOWTEXT;

                if (button.m_show == SHOW_TEXT || button.m_show == SHOW_IMAGE_TEXT) {
                    const auto display_text = button.get_display_text();
                    pfc::stringcvt::string_os_from_utf8 str_conv(display_text.c_str());
                    pfc::array_t<TCHAR, pfc::alloc_fast_aggressive> name;
                    name.prealloc(str_conv.length() + 4);
                    name.append_fromptr(str_conv.get_ptr(), str_conv.length());
                    name.append_single(0);
                    name.append_single(0);
                    tbbutton.iString
                        = SendMessage(wnd_toolbar, TB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(name.get_ptr()));
                }

                bool is_enabled{true};
                bool is_pressed{};

                if (button.m_interface.is_valid()) {
                    unsigned state = button.m_interface->get_button_state();

                    if (button.m_interface->get_button_type() == uie::BUTTON_TYPE_DROPDOWN_ARROW)
                        tbbutton.fsStyle |= BTNS_DROPDOWN;

                    is_enabled = (state & uie::BUTTON_STATE_ENABLED) != 0;
                    is_pressed = (state & uie::BUTTON_STATE_PRESSED) != 0;
                } else if (button.m_mainmenu_commands_v3.is_valid()) {
                    pfc::string8 _;
                    uint32_t flags{};
                    button.m_mainmenu_commands_v3->get_display(*button.m_mainmenu_commands_index, _, flags);

                    is_enabled = (flags & mainmenu_commands::flag_disabled) == 0;
                    is_pressed = (flags & mainmenu_commands::flag_checked) != 0;
                }

                if (is_enabled)
                    tbbutton.fsState |= TBSTATE_ENABLED;

                if (is_pressed)
                    tbbutton.fsState |= TBSTATE_PRESSED;
            }
        }

        const auto ex_style = SendMessage(wnd_toolbar, TB_GETEXTENDEDSTYLE, 0, 0);
        SendMessage(wnd_toolbar, TB_SETEXTENDEDSTYLE, 0,
            ex_style | TBSTYLE_EX_DRAWDDARROWS | (!m_text_below ? TBSTYLE_EX_MIXEDBUTTONS : 0));

        SendMessage(wnd_toolbar, TB_SETBITMAPSIZE, 0, MAKELONG(button_width, button_height));

        // todo: custom padding
        const auto padding = SendMessage(wnd_toolbar, TB_GETPADDING, 0, 0);

        if (m_appearance == APPEARANCE_NOEDGE) {
            SendMessage(wnd_toolbar, TB_SETPADDING, 0, MAKELPARAM(0, 0));
            /*
            HTHEME thm;
            thm = p_uxtheme->OpenThemeData(wnd_toolbar, L"Toolbar");
            MARGINS mg;
            p_uxtheme->GetThemeMargins(thm, NULL, 1, 1, 3602, NULL, &mg);
            p_uxtheme->CloseThemeData(thm);
            TBMETRICS temp;
            memset(&temp, 0, sizeof(temp));
            temp.cbSize = sizeof(temp);
            temp.dwMask = TBMF_BUTTONSPACING;
            temp.cxButtonSpacing =-4;
            temp.cyButtonSpacing=0;
            //SendMessage(wnd_toolbar, TB_SETMETRICS, 0, (LPARAM)&temp);
            temp.dwMask = TBMF_BUTTONSPACING|TBMF_BARPAD|TBMF_PAD;*/
        } else if (m_appearance == APPEARANCE_FLAT)
            SendMessage(wnd_toolbar, TB_SETPADDING, 0, MAKELPARAM(5, HIWORD(padding)));

        if (m_standard_images)
            SendMessage(wnd_toolbar, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(m_standard_images.get()));
        if (m_hot_images)
            SendMessage(wnd_toolbar, TB_SETHOTIMAGELIST, 0, reinterpret_cast<LPARAM>(m_hot_images.get()));

        SendMessage(wnd_toolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

        SendMessage(wnd_toolbar, TB_ADDBUTTONS, tbbuttons.size(), reinterpret_cast<LPARAM>(tbbuttons.data()));

        if (is_dark) {
            for (auto&& [n, button] : ranges::views::enumerate(m_buttons)) {
                if (button.m_type != TYPE_SEPARATOR)
                    continue;

                TBBUTTONINFO tbbi{};
                tbbi.cbSize = sizeof(tbbi);
                tbbi.cx = 5_spx;
                tbbi.dwMask = TBIF_BYINDEX | TBIF_SIZE;
                SendMessage(wnd_toolbar, TB_SETBUTTONINFO, n, reinterpret_cast<LPARAM>(&tbbi));
            }
        }

        for (auto&& [index, button] : ranges::views::enumerate(m_buttons)) {
            if (button.m_interface.is_valid())
                button.m_button_state_callback
                    = std::make_shared<ButtonStateCallback>(button.m_interface, wnd_toolbar, gsl::narrow<int>(index));
            else if (button.m_mainmenu_commands_v3.is_valid())
                button.m_mainmenu_state_callback
                    = std::make_shared<MainMenuStateCallback>(button.m_mainmenu_commands_v3,
                        *button.m_mainmenu_commands_index, wnd_toolbar, gsl::narrow<int>(index));
        }

        ShowWindow(wnd_toolbar, SW_SHOWNORMAL);

        const auto all_buttons_without_text
            = ranges::all_of(m_buttons, [](auto&& button) { return button.m_show == SHOW_IMAGE; });

        if (all_buttons_without_text && m_appearance == APPEARANCE_NOEDGE) {
            SendMessage(wnd_toolbar, TB_SETBUTTONSIZE, 0, MAKELONG(button_width, button_height));
        }

        SendMessage(wnd_toolbar, TB_AUTOSIZE, 0, 0);
    }
}

void ButtonsToolbar::destroy_toolbar()
{
    for (auto& button : m_buttons)
        button.reset_state();

    DestroyWindow(wnd_toolbar);
    wnd_toolbar = nullptr;
    m_standard_images.reset();
    m_hot_images.reset();
}

void ButtonsToolbar::set_window_theme() const
{
    SetWindowTheme(wnd_toolbar, colours::is_dark_mode_active() ? L"DarkMode" : nullptr, nullptr);
}

LRESULT ButtonsToolbar::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        wnd_host = wnd;
        initialised = true;
        create_toolbar();
        m_dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>([this, self = ptr{this}] {
            destroy_toolbar();
            create_toolbar();
        });
        break;
    }
    case WM_DESTROY:
        m_dark_mode_notifier.reset();
        destroy_toolbar();
        wnd_host = nullptr;
        initialised = false;
        break;
    case WM_WINDOWPOSCHANGED: {
        const auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);

        if (lpwp->flags & SWP_NOSIZE)
            break;

        RECT rc = {0, 0, 0, 0};
        const size_t count = m_buttons.size();
        int cx = lpwp->cx;
        int cy = lpwp->cy;
        int extra = 0;
        if (count && SendMessage(wnd_toolbar, TB_GETITEMRECT, count - 1, reinterpret_cast<LPARAM>(&rc))) {
            cx = std::min(cx, gsl::narrow_cast<int>(rc.right));
            cy = std::min(cy, gsl::narrow_cast<int>(rc.bottom));
            extra = (lpwp->cy - rc.bottom) / 2;
        }
        SetWindowPos(wnd_toolbar, nullptr, 0, extra, cx, cy, SWP_NOZORDER);
        break;
    }
    case WM_GETMINMAXINFO: {
        if (m_buttons.empty())
            break;

        const auto mmi = reinterpret_cast<LPMINMAXINFO>(lp);

        RECT rc = {0, 0, 0, 0};

        if (SendMessage(wnd_toolbar, TB_GETITEMRECT, m_buttons.size() - 1, reinterpret_cast<LPARAM>(&rc))) {
            mmi->ptMinTrackSize.x = rc.right;
            mmi->ptMinTrackSize.y = rc.bottom;
            mmi->ptMaxTrackSize.y = rc.bottom;
            return 0;
        }
        break;
    }
    case WM_USER + 2: {
        if (!wnd_toolbar || wp >= m_buttons.size() || !m_buttons[wp].m_interface.is_valid())
            break;

        const unsigned state = m_buttons[wp].m_interface->get_button_state();
        if (state & uie::BUTTON_STATE_PRESSED) {
            PostMessage(wnd_toolbar, TB_PRESSBUTTON, wp, MAKELONG(TRUE, 0));
        }
        break;
    }
    case WM_NOTIFY: {
        if (reinterpret_cast<LPNMHDR>(lp)->idFrom != ID_BUTTONS)
            break;

        switch (reinterpret_cast<LPNMHDR>(lp)->code) {
        case TBN_ENDDRAG: {
            const auto lpnmtb = reinterpret_cast<LPNMTOOLBARW>(lp);
            PostMessage(wnd, WM_USER + 2, lpnmtb->iItem, NULL);
            break;
        }
        case TBN_GETINFOTIP: {
            const auto lpnmtbgit = reinterpret_cast<LPNMTBGETINFOTIPW>(lp);
            if (!m_buttons[lpnmtbgit->iItem].m_interface.is_valid()
                || (m_buttons[lpnmtbgit->iItem].m_interface->get_button_state() & uie::BUTTON_STATE_SHOW_TOOLTIP)) {
                const auto text = m_buttons[lpnmtbgit->iItem].get_name(true);
                StringCchCopy(
                    lpnmtbgit->pszText, lpnmtbgit->cchTextMax, pfc::stringcvt::string_wide_from_utf8(text.c_str()));
            }
            break;
        }
        case TBN_DROPDOWN: {
            const auto lpnmtb = reinterpret_cast<LPNMTOOLBARW>(lp);
            const pfc::refcounted_object_ptr_t menu_items = new ui_extension::menu_hook_impl;

            m_buttons[lpnmtb->iItem].m_interface->get_menu_items(*menu_items.get_ptr());
            const HMENU menu = CreatePopupMenu();
            menu_items->win32_build_menu(menu, 1, pfc_infinite);
            POINT pt = {lpnmtb->rcButton.left, lpnmtb->rcButton.bottom};
            MapWindowPoints(lpnmtb->hdr.hwndFrom, HWND_DESKTOP, &pt, 1);
            const int cmd = TrackPopupMenuEx(menu, TPM_LEFTBUTTON | TPM_RETURNCMD, pt.x, pt.y, wnd, nullptr);
            if (cmd)
                menu_items->execute_by_id(cmd);
            DestroyMenu(menu);

            return TBDDRET_DEFAULT;
        }
        case NM_CUSTOMDRAW: {
            const auto lptbcd = reinterpret_cast<LPNMTBCUSTOMDRAW>(lp);
            switch ((lptbcd)->nmcd.dwDrawStage) {
            case CDDS_PREPAINT:
                return CDRF_NOTIFYITEMDRAW;
            case CDDS_ITEMPREPAINT: {
                const auto is_dark = colours::is_dark_mode_active();
                const auto index = lptbcd->nmcd.dwItemSpec;

                if (is_dark && index < m_buttons.size() && m_buttons[index].m_type == TYPE_SEPARATOR) {
                    const auto divider_brush = get_colour_brush(dark::ColourID::ToolbarDivider, true);
                    const auto divider_width = uih::scale_dpi_value(1, USER_DEFAULT_SCREEN_DPI * 2);
                    const auto& item_rect = lptbcd->nmcd.rc;
                    RECT line_rect{};
                    line_rect.top = item_rect.top + 1_spx;
                    line_rect.bottom = item_rect.bottom - 1_spx;
                    line_rect.left = item_rect.left + (wil::rect_width(item_rect) - divider_width) / 2;
                    line_rect.right = line_rect.left + divider_width;
                    FillRect(lptbcd->nmcd.hdc, &line_rect, divider_brush.get());
                }

                if (m_appearance != APPEARANCE_NOEDGE && !m_text_below && index < m_buttons.size()
                    && m_buttons[index].m_show == SHOW_TEXT) {
                    // Workaround for commctrl6
                    lptbcd->rcText.left -= LOWORD(SendMessage(wnd_toolbar, TB_GETPADDING, 0, 0)) + 2;
                }

                if (m_appearance == APPEARANCE_FLAT) {
                    LRESULT ret = TBCDRF_NOEDGES | TBCDRF_NOOFFSET | TBCDRF_HILITEHOTTRACK;

                    if (lptbcd->nmcd.uItemState & CDIS_HOT) {
                        lptbcd->clrText = get_colour(dark::ColourID::ToolbarFlatHotText, is_dark);
                        ret |= TBCDRF_USECDCOLORS;
                    }

                    lptbcd->clrHighlightHotTrack = get_colour(dark::ColourID::ToolbarFlatHotBackground, is_dark);
                    return ret;
                }

                if (m_appearance == APPEARANCE_NOEDGE) {
                    return TBCDRF_NOEDGES | TBCDRF_NOBACKGROUND;
                }
                break;
            }
            }
            break;
        }
        case NM_TOOLTIPSCREATED: {
            const auto lpnmttc = reinterpret_cast<LPNMTOOLTIPSCREATED>(lp);
            SetWindowTheme(
                lpnmttc->hwndToolTips, colours::is_dark_mode_active() ? L"DarkMode_Explorer" : nullptr, nullptr);
            break;
        }
        }
        break;
    }
    case WM_COMMAND: {
        if (wp >= m_buttons.size()) {
            console::print("buttons toolbar: error index out of range!");
            break;
        }

        const auto& button = m_buttons[wp];
        GUID caller = pfc::guid_null;
        metadb_handle_list_t<pfc::alloc_fast_aggressive> data;

        switch (button.m_filter) {
        case FILTER_PLAYLIST: {
            const auto api = playlist_manager::get();
            data.prealloc(api->activeplaylist_get_selection_count(pfc_infinite));
            api->activeplaylist_get_selected_items(data);
            caller = contextmenu_item::caller_active_playlist_selection;
            break;
        }
        case FILTER_ACTIVE_SELECTION: {
            const auto api = ui_selection_manager_v2::get();
            api->get_selection(data, ui_selection_manager_v2::flag_no_now_playing);
            caller = contextmenu_item::caller_undefined;
            break;
        }
        case FILTER_PLAYING: {
            metadb_handle_ptr hdle;
            if (play_control::get()->get_now_playing(hdle))
                data.add_item(hdle);
            caller = contextmenu_item::caller_now_playing;
            break;
        }
        case FILTER_NONE:
            break;
        }

        switch (button.m_type) {
        case TYPE_MENU_ITEM_CONTEXT: {
            if (button.m_filter != FILTER_NONE && data.size() == 0)
                break;

            service_ptr_t<contextmenu_item> item;
            uint32_t index{};

            if (menu_item_resolver::g_resolve_context_command(button.m_guid, item, index))
                item->item_execute_simple(index, button.m_subcommand, data, caller);

            break;
        }
        case TYPE_MENU_ITEM_MAIN:
            if (button.m_subcommand != pfc::guid_null)
                mainmenu_commands::g_execute_dynamic(button.m_guid, button.m_subcommand);
            else
                mainmenu_commands::g_execute(button.m_guid);
            break;
        case TYPE_BUTTON: {
            service_ptr_t<uie::custom_button> p_button;
            if (button.m_interface.is_valid() && button.m_interface->service_query_t(p_button))
                p_button->execute(data);
        } break;
        case TYPE_SEPARATOR:
            break;
        }
        break;
    }
    case WM_CONTEXTMENU: {
        if (reinterpret_cast<HWND>(wp) != wnd_toolbar || lp != -1)
            break;

        POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        POINT pts = pt;
        ScreenToClient(wnd_toolbar, &pt);
        const auto hittest_index = SendMessage(wnd_toolbar, TB_HITTEST, 0, reinterpret_cast<LPARAM>(&pt));

        if (hittest_index < 0 || hittest_index >= std::ssize(m_buttons)
            || !m_buttons[hittest_index].m_interface.is_valid())
            break;

        pfc::refcounted_object_ptr_t menu_items = new ui_extension::menu_hook_impl;

        m_buttons[hittest_index].m_interface->get_menu_items(*menu_items.get_ptr());

        if (!menu_items->get_children_count())
            break;

        const HMENU menu = CreatePopupMenu();
        menu_items->win32_build_menu(menu, 1, pfc_infinite);
        const int cmd = TrackPopupMenuEx(menu, TPM_LEFTBUTTON | TPM_RETURNCMD, pts.x, pts.y, wnd, nullptr);
        if (cmd)
            menu_items->execute_by_id(cmd);
        DestroyMenu(menu);
        return 0;
    }
    }

    return DefWindowProc(wnd, msg, wp, lp);
}

void ButtonsToolbar::get_name(pfc::string_base& out) const
{
    out.set_string("Buttons");
}
void ButtonsToolbar::get_category(pfc::string_base& out) const
{
    out.set_string("Toolbars");
}

void ButtonsToolbar::get_config(stream_writer* out, abort_callback& p_abort) const
{
    const auto count = gsl::narrow<uint32_t>(m_buttons.size());
    out->write_lendian_t(WI_EnumValue(ConfigVersion::VERSION_CURRENT), p_abort);
    out->write_lendian_t(m_text_below, p_abort);
    out->write_lendian_t(m_appearance, p_abort);
    out->write_lendian_t(count, p_abort);
    for (unsigned n = 0; n < count; n++) {
        m_buttons[n].write(out, p_abort);
    }
    out->write_lendian_t(WI_EnumValue(m_icon_size), p_abort);
    out->write_lendian_t(m_width.value, p_abort);
    out->write_lendian_t(m_width.dpi, p_abort);
    out->write_lendian_t(m_height.value, p_abort);
    out->write_lendian_t(m_height.dpi, p_abort);
}

void ButtonsToolbar::set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    if (!p_size)
        return;

    if (get_wnd())
        throw pfc::exception_bug_check(
            "uie::window::set_config() cannot be called once the window has been initialised.");

    ConfigVersion p_version;
    p_reader->read_lendian_t(p_version, p_abort);
    if (p_version <= ConfigVersion::VERSION_CURRENT) {
        p_reader->read_lendian_t(m_text_below, p_abort);
        p_reader->read_lendian_t(m_appearance, p_abort);
        const auto count = p_reader->read_lendian_t<uint32_t>(p_abort);
        m_buttons.clear();
        for (unsigned n = 0; n < count; n++) {
            Button temp;
            temp.read(p_version, p_reader, p_abort);
            m_buttons.emplace_back(std::move(temp));
        }

        try {
            m_icon_size = static_cast<IconSize>(p_reader->read_lendian_t<int32_t>(p_abort));

            const auto width = p_reader->read_lendian_t<int32_t>(p_abort);
            const auto width_dpi = p_reader->read_lendian_t<int32_t>(p_abort);
            m_width.set(width, width_dpi);

            const auto height = p_reader->read_lendian_t<int32_t>(p_abort);
            const auto height_dpi = p_reader->read_lendian_t<int32_t>(p_abort);
            m_height.set(height, height_dpi);
        } catch (const exception_io_data_truncation&) {
        }
    }
}

bool ButtonsToolbar::show_config_popup(HWND wnd_parent)
{
    ConfigParam param;
    param.m_selection = nullptr;
    param.m_buttons = m_buttons;

    for (auto& button : param.m_buttons)
        button.reset_state();

    param.m_active = 0;
    param.m_text_below = m_text_below;
    param.m_appearance = m_appearance;
    param.m_icon_size = m_icon_size;
    param.m_width = m_width;
    param.m_height = m_height;

    dark::DialogDarkModeConfig dark_mode_config{.button_ids
        = {IDC_PICK, IDC_ADD, IDC_REMOVE, IDC_RESET, IDC_BROWSE_ICON, IDC_BROWSE_HOVER_ICON, IDC_TOOLS, IDOK, IDCANCEL},
        .checkbox_ids = {IDC_USE_CUSTOM_TEXT, IDC_USE_CUSTOM_ICON, IDC_USE_CUSTOM_HOVER_ICON},
        .combo_box_ids = {IDC_SHOW, IDC_TEXT_LOCATION, IDC_APPEARANCE, IDC_ICON_SIZE},
        .edit_ids = {IDC_TEXT, IDC_ICON_PATH, IDC_HOVER_ICON_PATH, IDC_WIDTH, IDC_HEIGHT},
        .spin_ids = {IDC_WIDTH_SPIN, IDC_HEIGHT_SPIN}};

    const auto dialog_result = modal_dialog_box(IDD_BUTTONS_OPTIONS, dark_mode_config, wnd_parent,
        [&param](auto&&... args) { return param.on_dialog_message(std::forward<decltype(args)>(args)...); });

    if (dialog_result > 0) {
        configure(
            param.m_buttons, param.m_text_below, param.m_appearance, param.m_icon_size, param.m_width, param.m_height);
        return true;
    }
    return false;
}

// {D8E65660-64ED-42e7-850B-31D828C25294}
const GUID ButtonsToolbar::extension_guid
    = {0xd8e65660, 0x64ed, 0x42e7, {0x85, 0xb, 0x31, 0xd8, 0x28, 0xc2, 0x52, 0x94}};

ui_extension::window_factory<ButtonsToolbar> blah;

} // namespace cui::toolbars::buttons
