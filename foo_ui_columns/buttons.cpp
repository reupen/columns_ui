#include "stdafx.h"
#include "buttons.h"
#include "menu_items.h"

#define ID_BUTTONS 2001

ButtonsToolbar::class_data& ButtonsToolbar::get_class_data() const
{
    __implement_get_class_data_child_ex(class_name, true, false);
}

const GUID& ButtonsToolbar::get_extension_guid() const
{
    return extension_guid;
}

ButtonsToolbar::ConfigParam::ConfigParam()
    : m_button_list(*this)
{
}

void ButtonsToolbar::get_menu_items(ui_extension::menu_hook_t& p_hook)
{
    ui_extension::menu_node_ptr p_node(new uie::menu_node_configure(this, "Buttons options"));
    p_hook.add_node(p_node);
}

unsigned ButtonsToolbar::get_type() const
{
    return ui_extension::type_toolbar;
}

void ButtonsToolbar::import_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort)
{
    ButtonsToolbar::ConfigParam param;
    param.m_selection = nullptr;
    param.m_child = nullptr;
    param.m_active = 0;
    param.m_image = nullptr;
    param.m_text_below = m_text_below;
    param.m_appearance = m_appearance;
    param.import_from_stream(p_reader, false, p_abort);

    configure(param.m_buttons, param.m_text_below, param.m_appearance);
};

void ButtonsToolbar::export_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    ConfigParam param;
    param.m_selection = nullptr;
    param.m_buttons = m_buttons;
    param.m_child = nullptr;
    param.m_active = 0;
    param.m_image = nullptr;
    param.m_text_below = m_text_below;
    param.m_appearance = m_appearance;
    param.export_to_stream(p_writer, false, p_abort);
}

// {AFD89390-8E1F-434c-B9C5-A4C1261BB792}
const GUID ButtonsToolbar::g_guid_fcb
    = {0xafd89390, 0x8e1f, 0x434c, {0xb9, 0xc5, 0xa4, 0xc1, 0x26, 0x1b, 0xb7, 0x92}};

void ButtonsToolbar::reset_buttons(pfc::list_base_t<Button>& p_buttons)
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
        {cui::main_menu::commands::toggle_live_editing_id, TYPE_MENU_ITEM_MAIN, SHOW_TEXT, "Live layout editing"},
    };

    p_buttons.remove_all();

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
        p_buttons.add_item(temp);
    }
}

ButtonsToolbar::ButtonsToolbar()
{
    reset_buttons(m_buttons);
};

ButtonsToolbar::~ButtonsToolbar() = default;

const TCHAR* ButtonsToolbar::class_name = _T("{D75D4E2D-603B-4699-9C49-64DDFFE56A16}");

void ButtonsToolbar::create_toolbar()
{
    pfc::array_t<TBBUTTON> tbb;
    tbb.set_size(m_buttons.get_count());

    std::vector<ButtonImage> images(m_buttons.get_count());
    std::vector<ButtonImage> images_hot(m_buttons.get_count());

    memset(tbb.get_ptr(), 0, tbb.get_size() * sizeof(*tbb.get_ptr()));

    RECT rc;
    GetClientRect(wnd_host, &rc);

    wnd_toolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, nullptr,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT
            | (!m_text_below && m_appearance != APPEARANCE_NOEDGE ? TBSTYLE_LIST : 0) | TBSTYLE_TRANSPARENT
            | TBSTYLE_TOOLTIPS | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER,
        0, 0, rc.right, rc.bottom, wnd_host, (HMENU)ID_BUTTONS, core_api::get_my_instance(), nullptr);

    COLORREF colour_3dface = GetSysColor(COLOR_3DFACE);
    COLORREF colour_btntext = GetSysColor(COLOR_BTNTEXT);

    if (wnd_toolbar) {
        //            SetWindowLongPtr(p_this->wnd_toolbar,GWLP_USERDATA,(LPARAM)(p_this));

        HIMAGELIST il = nullptr;
        HIMAGELIST iml_hot = nullptr;

        // libpng_handle::g_create(p_libpng);

        bool b_need_hot = false;

        unsigned n;
        unsigned count = tbb.get_size();
        for (n = 0; n < count; n++) {
            if (m_buttons[n].m_use_custom_hot) {
                b_need_hot = true;
                break;
            }
        }

        SIZE sz = {0, 0};

        pfc::bit_array_bittable mask(count);

        for (n = 0; n < count; n++) {
            if (m_buttons[n].m_type != TYPE_SEPARATOR) {
                m_buttons[n].m_callback.set_wnd(this);
                m_buttons[n].m_callback.set_id(n);
                service_enum_t<ui_extension::button> e;
                service_ptr_t<ui_extension::button> ptr;
                while (e.next(ptr)) {
                    if (ptr->get_item_guid() == m_buttons[n].m_guid) {
                        m_buttons[n].m_interface = ptr;
                        break;
                    }
                }

                if (m_buttons[n].m_show == SHOW_IMAGE || m_buttons[n].m_show == SHOW_IMAGE_TEXT) {
                    if (m_buttons[n].m_use_custom_hot)
                        images_hot[n].load(m_buttons[n].m_custom_hot_image);
                    if (!m_buttons[n].m_use_custom) {
                        if (m_buttons[n].m_interface.is_valid()) {
                            mask.set(n, true);
                            // images[n].load(m_buttons[n].m_interface, colour_btntext);
                        }
                    } else {
                        images[n].load(m_buttons[n].m_custom_image);

                        SIZE szt;
                        images[n].get_size(szt);
                        sz.cx = max(sz.cx, szt.cx);
                        sz.cy = max(sz.cy, szt.cy);
                    }
                }
            }
        }
        if (sz.cx == 0 && sz.cy == 0) {
            sz.cx = GetSystemMetrics(SM_CXSMICON);
            sz.cy = GetSystemMetrics(SM_CYSMICON);
        }
        for (n = 0; n < count; n++) {
            if (mask[n])
                images[n].load(m_buttons[n].m_interface, colour_btntext, sz.cx, sz.cy);
        }

        width = sz.cx;
        height = sz.cy;
        il = ImageList_Create(width, height, ILC_COLOR32 | ILC_MASK, 7, 0);
        if (b_need_hot)
            iml_hot = ImageList_Create(width, height, ILC_COLOR32 | ILC_MASK, 7, 0);

        // SendMessage(wnd_toolbar, TB_ADDSTRING,  NULL, (LPARAM)_T("\0")); //Add a empty string at index 0

        for (n = 0; n < count; n++) {
            tbb[n].iString = -1; //"It works"

            if (m_buttons[n].m_type == TYPE_SEPARATOR) {
                tbb[n].idCommand = n;
                tbb[n].fsState = TBSTATE_ENABLED;
                tbb[n].fsStyle = BTNS_SEP;
            } else {
                m_buttons[n].m_callback.set_wnd(this);
                m_buttons[n].m_callback.set_id(n);
                if (m_buttons[n].m_show == SHOW_IMAGE || m_buttons[n].m_show == SHOW_IMAGE_TEXT) {
                    tbb[n].iBitmap = images[n].add_to_imagelist(il);
                    if (!m_buttons[n].m_use_custom_hot || !images_hot[n].is_valid())
                        images[n].add_to_imagelist(iml_hot);
                    else
                        images_hot[n].add_to_imagelist(iml_hot);
                } else
                    tbb[n].iBitmap = I_IMAGENONE;

                tbb[n].idCommand = n;
                tbb[n].fsState = 0;
                tbb[n].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
                if (!m_text_below && m_appearance != APPEARANCE_NOEDGE
                    && (m_buttons[n].m_show == SHOW_TEXT || m_buttons[n].m_show == SHOW_IMAGE_TEXT))
                    tbb[n].fsStyle |= BTNS_SHOWTEXT;

                if (/*m_text_below || (tbb[n].fsStyle & BTNS_SHOWTEXT) */ m_buttons[n].m_show == SHOW_TEXT
                    || m_buttons[n].m_show == SHOW_IMAGE_TEXT) {
                    const auto display_text = m_buttons[n].get_display_text();
                    pfc::stringcvt::string_os_from_utf8 str_conv(display_text.c_str());
                    pfc::array_t<TCHAR, pfc::alloc_fast_aggressive> name;
                    name.prealloc(str_conv.length() + 4);
                    name.append_fromptr(str_conv.get_ptr(), str_conv.length());
                    name.append_single(0);
                    name.append_single(0);
                    tbb[n].iString = SendMessage(wnd_toolbar, TB_ADDSTRING, NULL, (LPARAM)name.get_ptr());
                }

                if (m_buttons[n].m_interface.is_valid()) {
                    unsigned state = m_buttons[n].m_interface->get_button_state();
                    if (m_buttons[n].m_interface->get_button_type() == uie::BUTTON_TYPE_DROPDOWN_ARROW)
                        tbb[n].fsStyle |= BTNS_DROPDOWN;
                    if (state & uie::BUTTON_STATE_ENABLED)
                        tbb[n].fsState |= TBSTATE_ENABLED;
                    if (state & uie::BUTTON_STATE_PRESSED)
                        tbb[n].fsState |= TBSTATE_PRESSED;
                    // m_buttons[n].m_interface->register_callback(m_buttons[n].m_callback);
                } else {
                    tbb[n].fsState |= TBSTATE_ENABLED;
                }
            }
        }
        // p_libpng.release();

        unsigned ex_style = SendMessage(wnd_toolbar, TB_GETEXTENDEDSTYLE, 0, 0);
        SendMessage(wnd_toolbar, TB_SETEXTENDEDSTYLE, 0,
            ex_style | TBSTYLE_EX_DRAWDDARROWS | (!m_text_below ? TBSTYLE_EX_MIXEDBUTTONS : 0));

        SendMessage(wnd_toolbar, TB_SETBITMAPSIZE, (WPARAM)0, MAKELONG(width, height));
        // SendMessage(wnd_toolbar, TB_SETBUTTONSIZE, (WPARAM) 0, MAKELONG(width,height));

        // todo: custom padding
        unsigned padding = SendMessage(wnd_toolbar, TB_GETPADDING, (WPARAM)0, 0);
        if (m_appearance == APPEARANCE_NOEDGE) {
            SendMessage(wnd_toolbar, TB_SETPADDING, (WPARAM)0, MAKELPARAM(0, 0));
            DLLVERSIONINFO2 dvi;
            HRESULT hr = uih::get_comctl32_version(dvi);
            if (SUCCEEDED(hr) && dvi.info1.dwMajorVersion >= 6) {
                /*
                HTHEME thm;
                uxtheme_api_ptr p_uxtheme;
                uxtheme_handle::g_create(p_uxtheme);
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
            }
        } else if (m_appearance == APPEARANCE_FLAT)
            SendMessage(wnd_toolbar, TB_SETPADDING, (WPARAM)0, MAKELPARAM(5, HIWORD(padding)));

        if (il)
            SendMessage(wnd_toolbar, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)il);
        if (iml_hot)
            SendMessage(wnd_toolbar, TB_SETHOTIMAGELIST, (WPARAM)0, (LPARAM)iml_hot);

        SendMessage(wnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

        SendMessage(wnd_toolbar, TB_ADDBUTTONS, (WPARAM)tbb.get_size(), (LPARAM)tbb.get_ptr());

        for (n = 0; n < count; n++)
            if (m_buttons[n].m_interface.is_valid())
                m_buttons[n].m_interface->register_callback(m_buttons[n].m_callback);

        ShowWindow(wnd_toolbar, SW_SHOWNORMAL);
        SendMessage(wnd_toolbar, TB_AUTOSIZE, 0, 0);
    }
}

void ButtonsToolbar::destroy_toolbar()
{
    t_size count = m_buttons.get_count();
    for (t_size i = 0; i < count; i++)
        if (m_buttons[i].m_interface.is_valid())
            m_buttons[i].m_interface->deregister_callback(m_buttons[i].m_callback);
    auto iml = (HIMAGELIST)SendMessage(wnd_toolbar, TB_GETIMAGELIST, (WPARAM)0, (LPARAM)0);
    auto iml_hot = (HIMAGELIST)SendMessage(wnd_toolbar, TB_GETHOTIMAGELIST, (WPARAM)0, (LPARAM)0);
    DestroyWindow(wnd_toolbar);
    wnd_toolbar = nullptr;
    if (iml)
        ImageList_Destroy(iml);
    if (iml_hot)
        ImageList_Destroy(iml_hot);
    m_buttons.remove_all();
}

LRESULT ButtonsToolbar::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE) {
        wnd_host = wnd;
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        m_gdiplus_initialised
            = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplus_instance, &gdiplusStartupInput, nullptr));
        initialised = true;
        create_toolbar();

    } else if (msg == WM_DESTROY) {
        destroy_toolbar();
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
            t_size count = m_buttons.get_count();
            int cx = lpwp->cx;
            int cy = lpwp->cy;
            int extra = 0;
            if (count && (BOOL)SendMessage(wnd_toolbar, TB_GETITEMRECT, count - 1, (LPARAM)(&rc))) {
                cx = min(cx, rc.right);
                cy = min(cy, rc.bottom);
                extra = (lpwp->cy - rc.bottom) / 2;
            }
            SetWindowPos(wnd_toolbar, nullptr, 0, extra, cx, cy, SWP_NOZORDER);
        }
    } else if (msg == WM_SIZE) {
    } else if (msg == WM_GETMINMAXINFO) {
        if (m_buttons.get_count()) {
            auto mmi = LPMINMAXINFO(lp);

            RECT rc = {0, 0, 0, 0};

            if (SendMessage(wnd_toolbar, TB_GETITEMRECT, m_buttons.get_count() - 1, (LPARAM)(&rc))) {
                mmi->ptMinTrackSize.x = rc.right;
                mmi->ptMinTrackSize.y = rc.bottom;
                mmi->ptMaxTrackSize.y = rc.bottom;
                return 0;
            }
        }
    }

    else if (msg == WM_USER + 2) {
        if (wnd_toolbar && wp < m_buttons.get_count() && m_buttons[wp].m_interface.is_valid()) {
            unsigned state = m_buttons[wp].m_interface->get_button_state();
            unsigned tbstate = 0;
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
            auto lptbcd = (LPNMTBCUSTOMDRAW)lp;
            switch ((lptbcd)->nmcd.dwDrawStage) {
            case CDDS_PREPAINT:
                return (CDRF_NOTIFYITEMDRAW);
            case CDDS_ITEMPREPAINT: {
                if (m_appearance != APPEARANCE_NOEDGE && !m_text_below && lptbcd->nmcd.dwItemSpec >= 0
                    && lptbcd->nmcd.dwItemSpec < m_buttons.get_count()
                    && m_buttons[lptbcd->nmcd.dwItemSpec].m_show == SHOW_TEXT) {
                    DLLVERSIONINFO2 dvi;
                    HRESULT hr = uih::get_comctl32_version(dvi);
                    if (SUCCEEDED(hr) && dvi.info1.dwMajorVersion >= 6)
                        lptbcd->rcText.left
                            -= LOWORD(SendMessage(wnd_toolbar, TB_GETPADDING, (WPARAM)0, 0)) + 2; // Hack for commctrl6
                }
                if (m_appearance == APPEARANCE_FLAT) {
                    LRESULT rv = TBCDRF_NOEDGES | TBCDRF_NOOFFSET;
                    if (lptbcd->nmcd.uItemState & CDIS_HOT) {
                        lptbcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                    } else {
                    }
                    lptbcd->clrHighlightHotTrack = GetSysColor(COLOR_HIGHLIGHT);
                    rv |= TBCDRF_HILITEHOTTRACK;
                    return rv;
                }
                if (m_appearance == APPEARANCE_NOEDGE) {
                    return TBCDRF_NOEDGES | TBCDRF_NOBACKGROUND;
                }
            } break;
            }
        } break;
        }
    } else if (msg == WM_COMMAND) {
        if (wp >= 0 && wp < m_buttons.get_count()) {
            GUID caller = pfc::guid_null;
            metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
            switch (m_buttons[wp].m_filter) {
            case FILTER_PLAYLIST: {
                static_api_ptr_t<playlist_manager> api;
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
                if (static_api_ptr_t<play_control>()->get_now_playing(hdle))
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
                int lresult = SendMessage(wnd_toolbar, TB_HITTEST, 0, (LPARAM)&pt);
                if (lresult >= 0 && // not a separator
                    (unsigned)lresult < m_buttons.get_count() && // safety
                    m_buttons[lresult].m_interface.is_valid())

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
    unsigned count = m_buttons.get_count();
    out->write_lendian_t(VERSION_CURRENT, p_abort);
    out->write_lendian_t(m_text_below, p_abort);
    out->write_lendian_t(m_appearance, p_abort);
    out->write_lendian_t(count, p_abort);
    for (unsigned n = 0; n < count; n++) {
        m_buttons[n].write(out, p_abort);
    }
}

void ButtonsToolbar::set_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort)
{
    if (!p_size)
        return;

    if (get_wnd())
        throw pfc::exception_bug_check(
            "uie::window::set_config() cannot be called once the window has been initialised.");

    ConfigVersion p_version;
    unsigned count = m_buttons.get_count();
    p_reader->read_lendian_t(p_version, p_abort);
    if (p_version <= VERSION_CURRENT) {
        p_reader->read_lendian_t(m_text_below, p_abort);
        p_reader->read_lendian_t(m_appearance, p_abort);
        p_reader->read_lendian_t(count, p_abort);
        m_buttons.remove_all();
        for (unsigned n = 0; n < count; n++) {
            Button temp;
            temp.read(p_version, p_reader, p_abort);
            m_buttons.add_item(temp);
        }
    }
}

BOOL CALLBACK ButtonsToolbar::ConfigCommandProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(wnd, DWLP_USER, lp);
        auto* ptr = reinterpret_cast<CommandPickerData*>(lp);
        return ptr->on_message(wnd, msg, wp, lp);
    }
    default: {
        auto* ptr = reinterpret_cast<CommandPickerData*>(GetWindowLongPtr(wnd, DWLP_USER));
        return ptr->on_message(wnd, msg, wp, lp);
    }
    }
}

BOOL CALLBACK ButtonsToolbar::ConfigChildProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(wnd, DWLP_USER, lp);
        {
            auto* ptr = reinterpret_cast<ConfigParam*>(lp);

            uSendDlgItemMessageText(wnd, IDC_IMAGE_TYPE, CB_ADDSTRING, 0, "Default");
            uSendDlgItemMessageText(wnd, IDC_IMAGE_TYPE, CB_ADDSTRING, 0, "Custom");

            SHAutoComplete(GetDlgItem(wnd, IDC_IMAGE_PATH), SHACF_FILESYSTEM);
        }
        return TRUE;
    case MSG_COMMAND_CHANGE: {
        auto* ptr = reinterpret_cast<ConfigParam*>(GetWindowLongPtr(wnd, DWLP_USER));
        if (ptr->m_selection) {
            bool& b_custom = (ptr->m_active ? ptr->m_selection->m_use_custom_hot : ptr->m_selection->m_use_custom);
            bool b_enable = ptr->m_selection && ptr->m_selection->m_type != TYPE_SEPARATOR;
            EnableWindow(GetDlgItem(wnd, IDC_IMAGE_PATH), b_enable && b_custom);
            EnableWindow(GetDlgItem(wnd, IDC_IMAGE_TYPE), b_enable);
            EnableWindow(GetDlgItem(wnd, IDC_BROWSE), b_enable && b_custom);
        }
    } break;
    case MSG_BUTTON_CHANGE: {
        auto* ptr = reinterpret_cast<ConfigParam*>(GetWindowLongPtr(wnd, DWLP_USER));
        bool b_custom = ptr->m_selection
            ? (ptr->m_active ? ptr->m_selection->m_use_custom_hot : ptr->m_selection->m_use_custom)
            : false;

        SendDlgItemMessage(wnd, IDC_IMAGE_TYPE, CB_SETCURSEL, ptr->m_selection && b_custom ? 1 : 0, 0);
        uSendDlgItemMessageText(
            wnd, IDC_IMAGE_PATH, WM_SETTEXT, 0, (ptr->m_selection && b_custom) ? ptr->m_image->m_path.get_ptr() : "");
        bool b_enable = ptr->m_selection && ptr->m_selection->m_type != TYPE_SEPARATOR;
        EnableWindow(GetDlgItem(wnd, IDC_IMAGE_PATH), b_enable && b_custom);
        EnableWindow(GetDlgItem(wnd, IDC_IMAGE_TYPE), b_enable);
        EnableWindow(GetDlgItem(wnd, IDC_BROWSE), b_enable && b_custom);
        if (!b_enable) {
            // SendDlgItemMessage(wnd, IDC_IMAGE_TYPE, CB_SETCURSEL,
        }
    } break;
    case WM_COMMAND:
        switch (wp) {
        case (CBN_SELCHANGE << 16) | IDC_IMAGE_TYPE: {
            auto* ptr = reinterpret_cast<ConfigParam*>(GetWindowLongPtr(wnd, DWLP_USER));
            if (ptr->m_selection && ptr->m_image) {
                unsigned idx = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
                if (idx != CB_ERR && ptr->m_selection) {
                    bool& b_custom
                        = (ptr->m_active ? ptr->m_selection->m_use_custom_hot : ptr->m_selection->m_use_custom);
                    b_custom = idx == 1;
                    EnableWindow(GetDlgItem(wnd, IDC_IMAGE_PATH), b_custom);
                    EnableWindow(GetDlgItem(wnd, IDC_BROWSE), b_custom);
                    uSendDlgItemMessageText(wnd, IDC_IMAGE_PATH, WM_SETTEXT, 0,
                        (ptr->m_selection && b_custom) ? ptr->m_image->m_path.get_ptr() : "");
                }
            }
        } break;
        case (EN_CHANGE << 16) | IDC_IMAGE_PATH: {
            auto* ptr = reinterpret_cast<ConfigParam*>(GetWindowLongPtr(wnd, DWLP_USER));
            if (ptr->m_image) {
                ptr->m_image->m_path = string_utf8_from_window((HWND)lp);
            }
        } break;
        case IDC_BROWSE: {
            auto* ptr = reinterpret_cast<ConfigParam*>(GetWindowLongPtr(wnd, DWLP_USER));
            bool b_custom = ptr->m_selection
                ? (ptr->m_active ? ptr->m_selection->m_use_custom_hot : ptr->m_selection->m_use_custom)
                : false;
            if (ptr->m_image && b_custom) {
                pfc::string8 temp;
                if (!uGetFullPathName(ptr->m_selection->m_custom_image.m_path, temp)
                    || (uGetFileAttributes(temp) & FILE_ATTRIBUTE_DIRECTORY))
                    temp.reset();

                constexpr auto extension_mask
                    = "Image Files (*.bmp;*.gif;*.ico;*.png;*.tiff;*.webp)|*.bmp;*.gif;*.ico;*.png;*.tiff;*.webp|All "
                      "Files (*.*)|*.*";

                if (uGetOpenFileName(wnd, extension_mask, 0, "png", "Choose image", nullptr, temp, FALSE)) {
                    ptr->m_image->m_path = temp;
                    uSendDlgItemMessageText(
                        wnd, IDC_IMAGE_PATH, WM_SETTEXT, 0, (true) ? ptr->m_image->m_path.get_ptr() : "");
                }
            }
        } break;
        default:
            return FALSE;
        }
    default:
        return FALSE;
    }
    return FALSE;
}

bool ButtonsToolbar::show_config_popup(HWND wnd_parent)
{
    ConfigParam param;
    param.m_selection = nullptr;
    param.m_buttons = m_buttons;
    param.m_child = nullptr;
    param.m_active = 0;
    param.m_image = nullptr;
    param.m_text_below = m_text_below;
    param.m_appearance = m_appearance;

    const auto dialog_result = DialogBoxParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_BUTTONS_OPTIONS),
        wnd_parent, ConfigParam::g_ConfigPopupProc, reinterpret_cast<LPARAM>(&param));

    if (dialog_result > 0) {
        configure(param.m_buttons, param.m_text_below, param.m_appearance);
        return true;
    }
    return false;
}

// {D8E65660-64ED-42e7-850B-31D828C25294}
const GUID ButtonsToolbar::extension_guid
    = {0xd8e65660, 0x64ed, 0x42e7, {0x85, 0xb, 0x31, 0xd8, 0x28, 0xc2, 0x52, 0x94}};

ui_extension::window_factory<ButtonsToolbar> blah;
