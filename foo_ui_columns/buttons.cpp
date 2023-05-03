#include "pch.h"
#include "buttons.h"

#include "dark_mode.h"
#include "dark_mode_dialog.h"
#include "menu_items.h"

#define ID_BUTTONS 2001

namespace cui::toolbars::buttons {

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

            button.m_callback.set_wnd(this);
            button.m_callback.set_id(gsl::narrow<int>(n));

            for (auto enumerator = uie::button::enumerate(); !enumerator.finished(); ++enumerator) {
                if ((*enumerator)->get_item_guid() == button.m_guid) {
                    button.m_interface = *enumerator;
                    break;
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
            console::print(reinterpret_cast<const char*>(fmt::format(
                u8"Buttons toolbar – resized some custom non-hover icons to {} x {}px", button_width, button_height)
                                                             .c_str()));

        if (any_hot_images_resized)
            console::print(reinterpret_cast<const char*>(fmt::format(
                u8"Buttons toolbar – resized some custom hover icons to {} x {}px", button_width, button_height)
                                                             .c_str()));

        m_standard_images.reset(
            ImageList_Create(button_width, button_height, ILC_COLOR32 | ILC_MASK, gsl::narrow<int>(image_count), 0));
        if (b_need_hot)
            m_hot_images.reset(ImageList_Create(
                button_width, button_height, ILC_COLOR32 | ILC_MASK, gsl::narrow<int>(image_count), 0));

        for (auto&& [n, tbbutton] : ranges::views::enumerate(tbbuttons)) {
            tbbutton.iString = -1; //"It works"

            if (m_buttons[n].m_type == TYPE_SEPARATOR) {
                tbbutton.idCommand = gsl::narrow<int>(n);
                tbbutton.fsStyle = is_dark ? BTNS_BUTTON | BTNS_SHOWTEXT : BTNS_SEP;
                tbbutton.iBitmap = I_IMAGENONE;
            } else {
                m_buttons[n].m_callback.set_wnd(this);
                m_buttons[n].m_callback.set_id(gsl::narrow<int>(n));
                if (m_buttons[n].m_show == SHOW_IMAGE || m_buttons[n].m_show == SHOW_IMAGE_TEXT) {
                    tbbutton.iBitmap = images[n].add_to_imagelist(m_standard_images.get());
                    if (!m_buttons[n].m_use_custom_hot || !images_hot[n].is_valid())
                        images[n].add_to_imagelist(m_hot_images.get());
                    else
                        images_hot[n].add_to_imagelist(m_hot_images.get());
                } else
                    tbbutton.iBitmap = I_IMAGENONE;

                tbbutton.idCommand = gsl::narrow<int>(n);
                tbbutton.fsState = 0;
                tbbutton.fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
                if (!m_text_below && m_appearance != APPEARANCE_NOEDGE
                    && (m_buttons[n].m_show == SHOW_TEXT || m_buttons[n].m_show == SHOW_IMAGE_TEXT))
                    tbbutton.fsStyle |= BTNS_SHOWTEXT;

                if (/*m_text_below || (tbb_item.fsStyle & BTNS_SHOWTEXT) */ m_buttons[n].m_show == SHOW_TEXT
                    || m_buttons[n].m_show == SHOW_IMAGE_TEXT) {
                    const auto display_text = m_buttons[n].get_display_text();
                    pfc::stringcvt::string_os_from_utf8 str_conv(display_text.c_str());
                    pfc::array_t<TCHAR, pfc::alloc_fast_aggressive> name;
                    name.prealloc(str_conv.length() + 4);
                    name.append_fromptr(str_conv.get_ptr(), str_conv.length());
                    name.append_single(0);
                    name.append_single(0);
                    tbbutton.iString
                        = SendMessage(wnd_toolbar, TB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(name.get_ptr()));
                }

                if (m_buttons[n].m_interface.is_valid()) {
                    unsigned state = m_buttons[n].m_interface->get_button_state();
                    if (m_buttons[n].m_interface->get_button_type() == uie::BUTTON_TYPE_DROPDOWN_ARROW)
                        tbbutton.fsStyle |= BTNS_DROPDOWN;
                    if (state & uie::BUTTON_STATE_ENABLED)
                        tbbutton.fsState |= TBSTATE_ENABLED;
                    if (state & uie::BUTTON_STATE_PRESSED)
                        tbbutton.fsState |= TBSTATE_PRESSED;
                    // m_buttons[n].m_interface->register_callback(m_buttons[n].m_callback);
                } else {
                    tbbutton.fsState |= TBSTATE_ENABLED;
                }
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

        for (auto&& button : m_buttons) {
            if (button.m_interface.is_valid())
                button.m_interface->register_callback(button.m_callback);
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
    size_t count = m_buttons.size();
    for (size_t i = 0; i < count; i++)
        if (m_buttons[i].m_interface.is_valid())
            m_buttons[i].m_interface->deregister_callback(m_buttons[i].m_callback);
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
    if (msg == WM_CREATE) {
        wnd_host = wnd;
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        m_gdiplus_initialised = (Gdiplus::Ok == GdiplusStartup(&m_gdiplus_instance, &gdiplusStartupInput, nullptr));
        initialised = true;
        create_toolbar();
        m_dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>([this, self = ptr{this}] {
            destroy_toolbar();
            create_toolbar();
        });
    } else if (msg == WM_DESTROY) {
        m_dark_mode_notifier.reset();
        destroy_toolbar();
        m_buttons.clear();
        wnd_host = nullptr;
        initialised = false;
        if (m_gdiplus_initialised) {
            Gdiplus::GdiplusShutdown(m_gdiplus_instance);
            m_gdiplus_initialised = false;
        }
    } else if (msg == WM_WINDOWPOSCHANGED) {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            // SIZE sz = {0,0};
            // SendMessage(wnd_menu, TB_GETMAXSIZE, NULL, (LPARAM)&sz);

            RECT rc = {0, 0, 0, 0};
            size_t count = m_buttons.size();
            int cx = lpwp->cx;
            int cy = lpwp->cy;
            int extra = 0;
            if (count && (BOOL)SendMessage(wnd_toolbar, TB_GETITEMRECT, count - 1, (LPARAM)(&rc))) {
                cx = std::min(cx, gsl::narrow_cast<int>(rc.right));
                cy = std::min(cy, gsl::narrow_cast<int>(rc.bottom));
                extra = (lpwp->cy - rc.bottom) / 2;
            }
            SetWindowPos(wnd_toolbar, nullptr, 0, extra, cx, cy, SWP_NOZORDER);
        }
    } else if (msg == WM_SIZE) {
    } else if (msg == WM_GETMINMAXINFO) {
        if (m_buttons.size()) {
            auto mmi = LPMINMAXINFO(lp);

            RECT rc = {0, 0, 0, 0};

            if (SendMessage(wnd_toolbar, TB_GETITEMRECT, m_buttons.size() - 1, (LPARAM)(&rc))) {
                mmi->ptMinTrackSize.x = rc.right;
                mmi->ptMinTrackSize.y = rc.bottom;
                mmi->ptMaxTrackSize.y = rc.bottom;
                return 0;
            }
        }
    }

    else if (msg == WM_USER + 2) {
        if (wnd_toolbar && wp < m_buttons.size() && m_buttons[wp].m_interface.is_valid()) {
            unsigned state = m_buttons[wp].m_interface->get_button_state();
            if (state & uie::BUTTON_STATE_PRESSED) {
                PostMessage(wnd_toolbar, TB_PRESSBUTTON, wp, MAKELONG(TRUE, 0));
            }
        }
    }

    else if (msg == WM_NOTIFY && ((LPNMHDR)lp)->idFrom == ID_BUTTONS) {
        switch (((LPNMHDR)lp)->code) {
        case TBN_ENDDRAG: {
            auto lpnmtb = (LPNMTOOLBAR)lp;
            PostMessage(wnd, WM_USER + 2, lpnmtb->iItem, NULL);
        } break;
        case TBN_GETINFOTIP: {
            auto lpnmtbgit = (LPNMTBGETINFOTIP)lp;
            if (!m_buttons[lpnmtbgit->iItem].m_interface.is_valid()
                || (m_buttons[lpnmtbgit->iItem].m_interface->get_button_state() & uie::BUTTON_STATE_SHOW_TOOLTIP)) {
                const auto text = m_buttons[lpnmtbgit->iItem].get_name(true);
                StringCchCopy(
                    lpnmtbgit->pszText, lpnmtbgit->cchTextMax, pfc::stringcvt::string_wide_from_utf8(text.c_str()));
            }
        } break;
        case TBN_DROPDOWN: {
            auto lpnmtb = (LPNMTOOLBAR)lp;
            pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> menu_items = new ui_extension::menu_hook_impl;

            m_buttons[lpnmtb->iItem].m_interface->get_menu_items(*menu_items.get_ptr());
            HMENU menu = CreatePopupMenu();
            menu_items->win32_build_menu(menu, 1, pfc_infinite);
            POINT pt = {lpnmtb->rcButton.left, lpnmtb->rcButton.bottom};
            MapWindowPoints(lpnmtb->hdr.hwndFrom, HWND_DESKTOP, &pt, 1);
            int cmd = TrackPopupMenuEx(menu, TPM_LEFTBUTTON | TPM_RETURNCMD, pt.x, pt.y, wnd, nullptr);
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
                    line_rect.left = item_rect.left + (RECT_CX(item_rect) - divider_width) / 2;
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
            } break;
            }
        } break;
        }
    } else if (msg == WM_COMMAND) {
        if (wp >= 0 && wp < m_buttons.size()) {
            GUID caller = pfc::guid_null;
            metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
            switch (m_buttons[wp].m_filter) {
            case FILTER_PLAYLIST: {
                const auto api = playlist_manager::get();
                data.prealloc(api->activeplaylist_get_selection_count(pfc_infinite));
                api->activeplaylist_get_selected_items(data);
                caller = contextmenu_item::caller_active_playlist_selection;
            } break;
            case FILTER_ACTIVE_SELECTION: {
                auto api = ui_selection_manager_v2::get();
                api->get_selection(data, ui_selection_manager_v2::flag_no_now_playing);
                caller = contextmenu_item::caller_undefined;
            } break;
            case FILTER_PLAYING: {
                metadb_handle_ptr hdle;
                if (play_control::get()->get_now_playing(hdle))
                    data.add_item(hdle);
                caller = contextmenu_item::caller_now_playing;
            } break;
            }

            switch (m_buttons[wp].m_type) {
            case TYPE_MENU_ITEM_CONTEXT:
                menu_helpers::run_command_context_ex(m_buttons[wp].m_guid, m_buttons[wp].m_subcommand, data, caller);
                break;
            case TYPE_MENU_ITEM_MAIN:
                if (m_buttons[wp].m_subcommand != pfc::guid_null)
                    mainmenu_commands::g_execute_dynamic(m_buttons[wp].m_guid, m_buttons[wp].m_subcommand);
                else
                    mainmenu_commands::g_execute(m_buttons[wp].m_guid);
                break;
            case TYPE_BUTTON: {
                service_ptr_t<uie::custom_button> p_button;
                if (m_buttons[wp].m_interface.is_valid() && m_buttons[wp].m_interface->service_query_t(p_button))
                    p_button->execute(data);
            } break;
            }
        } else
            console::print("buttons toolbar: error index out of range!");
    } else if (msg == WM_CONTEXTMENU) {
        if (HWND(wp) == wnd_toolbar) {
            if (lp != -1) {
                POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
                POINT pts = pt;
                ScreenToClient(wnd_toolbar, &pt);
                const auto lresult = SendMessage(wnd_toolbar, TB_HITTEST, 0, (LPARAM)&pt);
                if (lresult >= 0 && // not a separator
                    lresult < std::ssize(m_buttons) && m_buttons[lresult].m_interface.is_valid())

                {
                    pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> menu_items
                        = new ui_extension::menu_hook_impl;

                    m_buttons[lresult].m_interface->get_menu_items(*menu_items.get_ptr());
                    if (menu_items->get_children_count()) {
                        HMENU menu = CreatePopupMenu();
                        menu_items->win32_build_menu(menu, 1, pfc_infinite);
                        int cmd = TrackPopupMenuEx(menu, TPM_LEFTBUTTON | TPM_RETURNCMD, pts.x, pts.y, wnd, nullptr);
                        if (cmd)
                            menu_items->execute_by_id(cmd);
                        DestroyMenu(menu);
                        return 0;
                    }
                }
            }
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
