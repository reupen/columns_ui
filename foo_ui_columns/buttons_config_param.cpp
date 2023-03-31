#include "pch.h"
#include "buttons.h"

namespace cui::toolbars::buttons {

namespace {

constexpr GUID fcb_header_v1 = {0xafd89390, 0x8e1f, 0x434c, {0xb9, 0xc5, 0xa4, 0xc1, 0x26, 0x1b, 0xb7, 0x92}};
constexpr GUID fcb_header_v2 = {0xa7d77fe1, 0xcee2, 0x4fee, {0xa0, 0x7, 0xe9, 0xfc, 0x97, 0xdd, 0x2b, 0xc7}};

} // namespace

void ButtonsToolbar::ConfigParam::export_to_stream(stream_writer* p_file, bool b_paths, abort_callback& p_abort)
{
    p_file->write_lendian_t(fcb_header_v2, p_abort);
    p_file->write_lendian_t(WI_EnumValue(FCBVersion::VERSION_CURRENT), p_abort);

    const auto count = gsl::narrow<uint32_t>(m_buttons.size());

    p_file->write_lendian_t(I_TEXT_BELOW, p_abort);
    p_file->write_lendian_t(mmh::sizeof_t<uint32_t>(m_text_below), p_abort);
    p_file->write_lendian_t(m_text_below, p_abort);

    p_file->write_lendian_t(I_APPEARANCE, p_abort);
    p_file->write_lendian_t(mmh::sizeof_t<uint32_t>(m_appearance), p_abort);
    p_file->write_lendian_t(m_appearance, p_abort);

    p_file->write_lendian_t(I_ICON_SIZE, p_abort);
    p_file->write_lendian_t(mmh::sizeof_t<uint32_t>(m_icon_size), p_abort);
    p_file->write_lendian_t(WI_EnumValue(m_icon_size), p_abort);

    p_file->write_lendian_t(I_WIDTH, p_abort);
    p_file->write_lendian_t(gsl::narrow<uint32_t>(sizeof(int32_t) * 2), p_abort);
    p_file->write_lendian_t(m_width.value, p_abort);
    p_file->write_lendian_t(m_width.dpi, p_abort);

    p_file->write_lendian_t(I_HEIGHT, p_abort);
    p_file->write_lendian_t(gsl::narrow<uint32_t>(sizeof(int32_t) * 2), p_abort);
    p_file->write_lendian_t(m_height.value, p_abort);
    p_file->write_lendian_t(m_height.dpi, p_abort);

    p_file->write_lendian_t(I_BUTTONS, p_abort);

    stream_writer_memblock p_write;
    // FIX

    p_file->write_lendian_t(gsl::narrow<uint32_t>(p_write.m_data.get_size() + sizeof(count)), p_abort);
    p_file->write_lendian_t(count, p_abort);

    for (unsigned n = 0; n < count; n++) {
        m_buttons[n].write_to_file(p_write, b_paths, p_abort);
        p_file->write_lendian_t(gsl::narrow<uint32_t>(p_write.m_data.get_size()), p_abort);
        p_file->write(p_write.m_data.get_ptr(), p_write.m_data.get_size(), p_abort);
        p_write.m_data.set_size(0);
    }
}

void ButtonsToolbar::ConfigParam::export_to_file(const char* p_path, bool b_paths)
{
    try {
        abort_callback_impl p_abort;
        service_ptr_t<file> p_file;
        filesystem::g_open(p_file, p_path, filesystem::open_mode_write_new, p_abort);
        export_to_stream(p_file.get_ptr(), b_paths, p_abort);
    } catch (const pfc::exception& p_error) {
        popup_message::g_show_ex(p_error.what(), pfc_infinite, "Error writing FCB file", pfc_infinite);
        abort_callback_dummy abortCallback;
        filesystem::g_remove(p_path, abortCallback);
    }
}

void ButtonsToolbar::ConfigParam::import_from_stream(stream_reader* p_file, bool add, abort_callback& p_abort)
{
    const char* profilepath = core_api::get_profile_path();
    if (!profilepath)
        throw pfc::exception_bug_check("NULL profile path");
    if (!stricmp_utf8_max(profilepath, "file://", 7))
        profilepath += 7;
    pfc::string8 str_base = profilepath;
    size_t blen = str_base.get_length();
    if (blen && str_base[blen - 1] == '\\')
        str_base.truncate(blen - 1);
    // uGetModuleFileName(NULL, str_base);
    // unsigned pos = str_base.find_last('\\');
    // if (pos != -1)
    //    str_base.truncate(pos);

    GUID header_id = p_file->read_lendian_t<GUID>(p_abort);

    if (header_id != fcb_header_v1 && header_id != fcb_header_v2)
        throw exception_io_data();

    const auto vers = static_cast<FCBVersion>(p_file->read_lendian_t<int32_t>(p_abort));

    if (vers > FCBVersion::VERSION_CURRENT)
        throw "This buttons toolbar configuration requires a newer version of Columns UI.";

    if (!add)
        m_buttons.clear();

    while (true) //! p_file.is_eof(p_abort)
    {
        Identifier id;
        try {
            p_file->read_lendian_t(id, p_abort);
        } catch (exception_io_data_truncation const&) {
            break;
        }
        unsigned size;
        p_file->read_lendian_t(size, p_abort);
        // if (size > p_file->get_size(p_abort) - p_file->get_position(p_abort))
        //    throw exception_io_data();
        switch (id) {
        case I_TEXT_BELOW:
            p_file->read_lendian_t(m_text_below, p_abort);
            break;
        case I_APPEARANCE:
            p_file->read_lendian_t(m_appearance, p_abort);
            break;
        case I_ICON_SIZE:
            m_icon_size = static_cast<IconSize>(p_file->read_lendian_t<int32_t>(p_abort));
            break;
        case I_WIDTH: {
            const auto value = p_file->read_lendian_t<int32_t>(p_abort);
            const auto dpi = p_file->read_lendian_t<int32_t>(p_abort);
            m_width.set(value, dpi);
            break;
        }
        case I_HEIGHT: {
            const auto value = p_file->read_lendian_t<int32_t>(p_abort);
            const auto dpi = p_file->read_lendian_t<int32_t>(p_abort);
            m_height.set(value, dpi);
            break;
        }
        case I_BUTTONS: {
            service_ptr_t<genrand_service> genrand = genrand_service::g_create();
            genrand->seed(GetTickCount());
            uint32_t dirname = genrand->genrand(pfc_infinite);

            unsigned count;
            p_file->read_lendian_t(count, p_abort);
            for (unsigned n = 0; n < count; n++) {
                Button temp{};
                unsigned size_button;
                p_file->read_lendian_t(size_button, p_abort);
                pfc::string_formatter formatter;
                temp.read_from_file(vers, str_base, formatter << dirname, p_file, size_button, p_abort);
                //                        assert(n < 7);
                m_buttons.emplace_back(std::move(temp));
            }
            break;
        }
        default:
            p_file->skip(size, p_abort);
            break;
        }
    }
}
void ButtonsToolbar::ConfigParam::import_from_file(const char* p_path, bool add)
{
    try {
        abort_callback_impl p_abort;
        service_ptr_t<file> p_file;
        filesystem::g_open(p_file, p_path, filesystem::open_mode_read, p_abort);
        import_from_stream(p_file.get_ptr(), add, p_abort);
    } catch (const pfc::exception& p_error) {
        popup_message::g_show_ex(p_error.what(), pfc_infinite, "Error reading FCB file", pfc_infinite);
    } catch (const char* p_error) {
        popup_message::g_show_ex(p_error, pfc_infinite, "Error reading FCB file", pfc_infinite);
    }
}

void ButtonsToolbar::ConfigParam::on_selection_change(size_t index)
{
    m_selection = index != pfc_infinite && index < m_buttons.size() ? &m_buttons[index] : nullptr;
    m_image = m_selection ? (m_active ? &m_selection->m_custom_hot_image : &m_selection->m_custom_image) : nullptr;
    SendDlgItemMessage(m_wnd, IDC_SHOW, CB_SETCURSEL, m_selection ? m_selection->m_show : -1, 0);

    bool b_enable = index != pfc_infinite && m_selection && m_selection->m_type != TYPE_SEPARATOR;
    Button_SetCheck(GetDlgItem(m_wnd, IDC_USE_CUSTOM_TEXT), b_enable ? m_selection->m_use_custom_text : FALSE);
    SendDlgItemMessage(m_wnd, IDC_TEXT, WM_SETTEXT, 0,
        b_enable ? (LPARAM)pfc::stringcvt::string_os_from_utf8(m_selection->m_text).get_ptr() : (LPARAM) _T(""));
    EnableWindow(GetDlgItem(m_wnd, IDC_PICK), index != pfc_infinite);
    EnableWindow(GetDlgItem(m_wnd, IDC_SHOW), b_enable);
    EnableWindow(GetDlgItem(m_wnd, IDC_USE_CUSTOM_TEXT), b_enable);
    EnableWindow(GetDlgItem(m_wnd, IDC_TEXT), b_enable && m_selection->m_use_custom_text);
    SendMessage(m_child, MSG_BUTTON_CHANGE, 0, 0);
}

void ButtonsToolbar::ConfigParam::populate_buttons_list()
{
    const auto count = m_buttons.size();

    pfc::array_staticsize_t<uih::ListView::InsertItem> items(count);
    for (size_t n = 0; n < count; n++) {
        items[n].m_subitems.resize(2);
        items[n].m_subitems[0] = m_buttons[n].get_name().c_str();
        items[n].m_subitems[1] = m_buttons[n].get_type_desc().c_str();
    }
    m_button_list.insert_items(0, count, items.get_ptr());
}

void ButtonsToolbar::ConfigParam::refresh_buttons_list_items(size_t index, size_t count, bool b_update_display)
{
    const auto real_count = m_buttons.size();

    if (index + count > real_count)
        count = real_count - index;

    pfc::list_t<uih::ListView::InsertItem> items;
    items.set_count(count);
    for (size_t n = index; n < index + count; n++) {
        items[n - index].m_subitems.resize(2);
        items[n - index].m_subitems[0] = m_buttons[n].get_name().c_str();
        items[n - index].m_subitems[1] = m_buttons[n].get_type_desc().c_str();
    }
    m_button_list.replace_items(index, items);
}

INT_PTR CALLBACK ButtonsToolbar::ConfigParam::g_ConfigPopupProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ConfigParam* ptr = nullptr;
    switch (msg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(wnd, DWLP_USER, lp);
        ptr = reinterpret_cast<ConfigParam*>(lp);
        break;
    default:
        ptr = reinterpret_cast<ConfigParam*>(GetWindowLongPtr(wnd, DWLP_USER));
        break;
    }
    return ptr ? ptr->ConfigPopupProc(wnd, msg, wp, lp) : FALSE;
}

BOOL ButtonsToolbar::ConfigParam::ConfigPopupProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        const auto _ = pfc::vartoggle_t(m_initialising, true);
        m_wnd = wnd;
        m_scope.initialize(FindOwningPopup(wnd));

        HWND icon_size_wnd = GetDlgItem(wnd, IDC_ICON_SIZE);
        ComboBox_AddString(icon_size_wnd, L"Automatic");
        ComboBox_AddString(icon_size_wnd, L"Custom:");

        HWND width_spin_wnd = GetDlgItem(wnd, IDC_WIDTH_SPIN);
        SendMessage(width_spin_wnd, UDM_SETRANGE32, 1, 999);

        HWND height_spin_wnd = GetDlgItem(wnd, IDC_HEIGHT_SPIN);
        SendMessage(height_spin_wnd, UDM_SETRANGE32, 1, 999);

        HWND wnd_show = GetDlgItem(wnd, IDC_SHOW);

        SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM) _T("Image"));
        SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM) _T("Image and text"));
        SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM) _T("Text"));

        HWND wnd_text = GetDlgItem(wnd, IDC_TEXT_LOCATION);

        SendMessage(wnd_text, CB_ADDSTRING, 0, (LPARAM) _T("Right"));
        SendMessage(wnd_text, CB_ADDSTRING, 0, (LPARAM) _T("Below"));

        HWND wnd_app = GetDlgItem(wnd, IDC_APPEARANCE);

        SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM) _T("Normal"));
        SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM) _T("Flat"));
        SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM) _T("No edges"));

        HWND wnd_tab = GetDlgItem(wnd, IDC_TAB);
        uTabCtrl_InsertItemText(wnd_tab, 0, "Normal icon");
        uTabCtrl_InsertItemText(wnd_tab, 1, "Hover icon");

        RECT tab;

        GetWindowRect(wnd_tab, &tab);
        MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);

        TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

        m_child = CreateDialogParam(
            core_api::get_my_instance(), MAKEINTRESOURCE(IDD_BUTTON_IMAGE), wnd, ConfigChildProc, (LPARAM)this);

        SetWindowPos(m_child, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        if (m_child) {
            {
                EnableThemeDialogTexture(m_child, ETDT_ENABLETAB);
            }
        }

        SetWindowPos(m_child, nullptr, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
        // SetWindowPos(wnd_tab,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

        ShowWindow(m_child, SW_NORMAL);
        UpdateWindow(m_child);

        SendMessage(wnd_text, CB_SETCURSEL, m_text_below ? 1 : 0, 0);
        SendMessage(wnd_app, CB_SETCURSEL, m_appearance, 0);
        ComboBox_SetCurSel(icon_size_wnd, WI_EnumValue(m_icon_size));
        update_size_field_status();

        HWND wnd_button_list = m_button_list.create(wnd, uih::WindowPosition(14, 16, 310, 106), true);
        populate_buttons_list();
        ShowWindow(wnd_button_list, SW_SHOWNORMAL);
        return TRUE;
    }
    case WM_DESTROY:
        m_button_list.destroy();
        break;
#if 0
    case WM_PAINT:
        uih::handle_modern_background_paint(wnd, GetDlgItem(wnd, IDOK));
        return TRUE;
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
        SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
        return (BOOL)GetSysColorBrush(COLOR_WINDOW);
#endif
        // case WM_ERASEBKGND:
        //    SetWindowLongPtr(wnd, DWLP_MSGRESULT, FALSE);
        //    return TRUE;
        // case WM_PAINT:
        //    uih::handle_modern_background_paint(wnd, GetDlgItem(wnd, IDOK));
        //    return TRUE;
        // case WM_CTLCOLORBTN:
        // case WM_CTLCOLORDLG:
        // case WM_CTLCOLORSTATIC:
        /*SetBkColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
        SetDCBrushColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
        SetDCPenColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
        SetBkMode((HDC)wp, TRANSPARENT);
        //SetROP2((HDC)wp, R2_BLACK);
        SelectBrush((HDC)wp, GetSysColorBrush(COLOR_3DDKSHADOW));
        return (BOOL)GetSysColorBrush(COLOR_3DDKSHADOW);*/
        // return FALSE;
    case WM_COMMAND:
        switch (wp) {
        case IDCANCEL: {
            EndDialog(wnd, 0);
            return TRUE;
        }
        case IDC_SHOW | CBN_SELCHANGE << 16:
            if (m_selection) {
                m_selection->m_show = (Show)SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
            }
            break;
        case IDC_TEXT_LOCATION | CBN_SELCHANGE << 16:
            m_text_below = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0) != 0;
            break;
        case IDC_APPEARANCE | CBN_SELCHANGE << 16:
            m_appearance = (Appearance)SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
            break;
        case IDC_ICON_SIZE | CBN_SELCHANGE << 16:
            if (auto index = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp)); index != CB_ERR) {
                m_icon_size = static_cast<IconSize>(index);
                update_size_field_status();
            }
            break;
        case IDC_WIDTH | EN_CHANGE << 16:
            if (!m_initialising)
                m_width = static_cast<int32_t>(SendMessage(GetDlgItem(wnd, IDC_WIDTH_SPIN), UDM_GETPOS32, 0, 0));
            break;
        case IDC_HEIGHT | EN_CHANGE << 16:
            if (!m_initialising)
                m_height = static_cast<int32_t>(SendMessage(GetDlgItem(wnd, IDC_HEIGHT_SPIN), UDM_GETPOS32, 0, 0));
            break;
        case IDC_ADD: {
            CommandPickerDialog command_picker_dialog;

            if (const auto [succeeded, data] = command_picker_dialog.open_modal(wnd); succeeded) {
                auto& button = m_buttons.emplace_back(Button{});
                button.m_type = static_cast<Type>(data.group);
                button.m_guid = data.guid;
                button.m_subcommand = data.subcommand;
                button.m_filter = static_cast<Filter>(data.filter);

                uih::ListView::InsertItem item;
                item.m_subitems.resize(2);
                item.m_subitems[0] = button.get_name().c_str();
                item.m_subitems[1] = button.get_type_desc().c_str();
                size_t index_list = m_button_list.get_item_count();
                m_button_list.insert_items(index_list, 1, &item);
                m_button_list.set_item_selected_single(index_list);
                m_button_list.ensure_visible(index_list);
            }
            break;
        }
        case IDC_RESET: {
            if (win32_helpers::message_box(wnd,
                    _T("This will reset all your buttons to the default buttons. Continue?"), _T("Reset buttons"),
                    MB_YESNO)
                == IDYES) {
                m_button_list.remove_items(bit_array_true());
                reset_buttons(m_buttons);
                populate_buttons_list();
            }
            break;
        }
        case IDC_REMOVE: {
            size_t index = m_button_list.get_selected_item_single();
            if (index != pfc_infinite) {
                m_button_list.remove_item(index);
                m_buttons.erase(m_buttons.begin() + index);
                if (index < m_button_list.get_item_count())
                    m_button_list.set_item_selected_single(index);
                else if (index)
                    m_button_list.set_item_selected_single(index - 1);
            }
            break;
        }
        case IDC_TOOLS: {
            RECT rc;
            GetWindowRect(HWND(lp), &rc);
            HMENU menu = CreatePopupMenu();
            enum { IDM_SET_MASK = 1, IDM_EXPORT, IDM_SAVE, IDM_LOAD, IDM_ADD };

            // AppendMenu(menu,MF_SEPARATOR,0,0);
            AppendMenu(menu, MF_STRING, IDM_LOAD, _T("Load from file..."));
            AppendMenu(menu, MF_STRING, IDM_ADD, _T("Add from file..."));
            AppendMenu(menu, MF_STRING, IDM_EXPORT, _T("Save to file (embed images)..."));
            AppendMenu(menu, MF_STRING, IDM_SAVE, _T("Save to file (store image paths)..."));

            int cmd = TrackPopupMenu(
                menu, TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rc.left, rc.bottom, 0, wnd, nullptr);
            DestroyMenu(menu);
            if (cmd == IDM_SAVE) {
                pfc::string8 path;
                if (uGetOpenFileName(
                        wnd, "fcb files (*.fcb)|*.fcb|All files (*.*)|*.*", 0, "fcb", "Save as", nullptr, path, TRUE)) {
                    export_to_file(path, true);
                }
            } else if (cmd == IDM_EXPORT) {
                pfc::string8 path;
                if (uGetOpenFileName(
                        wnd, "fcb files (*.fcb)|*.fcb|All files (*.*)|*.*", 0, "fcb", "Save as", nullptr, path, TRUE)) {
                    export_to_file(path);
                }
            } else if (cmd == IDM_LOAD || cmd == IDM_ADD) {
                pfc::string8 path;
                if (uGetOpenFileName(wnd, "fcb Files (*.fcb)|*.fcb|All Files (*.*)|*.*", 0, "fcb", "Open file", nullptr,
                        path, FALSE)) {
                    m_button_list.remove_items(bit_array_true());

                    HWND wnd_text = GetDlgItem(wnd, IDC_TEXT_LOCATION);
                    HWND wnd_app = GetDlgItem(wnd, IDC_APPEARANCE);

                    import_from_file(path, cmd == IDM_ADD);
                    SendMessage(wnd_text, CB_SETCURSEL, m_text_below ? 1 : 0, 0);
                    SendMessage(wnd_app, CB_SETCURSEL, m_appearance, 0);
                    ComboBox_SetCurSel(GetDlgItem(wnd, IDC_ICON_SIZE), WI_EnumValue(m_icon_size));
                    update_size_field_status();
                    populate_buttons_list();
                }
            }
            break;
        }
        case IDC_USE_CUSTOM_TEXT: {
            if (m_selection) {
                m_selection->m_use_custom_text = Button_GetCheck(HWND(lp)) != 0;
                bool b_enable = m_selection->m_type != TYPE_SEPARATOR;
                EnableWindow(GetDlgItem(wnd, IDC_TEXT), !b_enable || m_selection->m_use_custom_text);
            }
            break;
        }
        case IDC_TEXT | (EN_CHANGE << 16): {
            if (m_selection) {
                m_selection->m_text = uGetWindowText(HWND(lp));
            }
            break;
        }
        case IDC_PICK: {
            if (m_selection) {
                CommandPickerDialog command_picker_dialog(
                    {m_selection->m_guid, m_selection->m_subcommand, m_selection->m_type, m_selection->m_filter});

                if (const auto [succeeded, data] = command_picker_dialog.open_modal(wnd); succeeded) {
                    m_selection->m_type = static_cast<Type>(data.group);
                    m_selection->m_guid = data.guid;
                    m_selection->m_subcommand = data.subcommand;
                    m_selection->m_filter = static_cast<Filter>(data.filter);
                    m_selection->m_interface.release();

                    const auto idx = m_button_list.get_selected_item_single();
                    if (idx != pfc_infinite) {
                        refresh_buttons_list_items(idx, 1);
                    }
                    bool b_enable = m_selection->m_type != TYPE_SEPARATOR;
                    EnableWindow(GetDlgItem(wnd, IDC_SHOW), m_selection->m_type != TYPE_SEPARATOR);
                    EnableWindow(GetDlgItem(wnd, IDC_USE_CUSTOM_TEXT), b_enable);
                    EnableWindow(GetDlgItem(wnd, IDC_TEXT), !b_enable || m_selection->m_use_custom_text);
                    SendMessage(m_child, MSG_COMMAND_CHANGE, 0, 0);
                }
            }
            break;
        }
        case IDOK: {
            EndDialog(wnd, 1);
        }
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lp)->idFrom) {
        case IDC_TAB:
            switch (((LPNMHDR)lp)->code) {
            case TCN_SELCHANGE: {
                m_active = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB));
                m_image = m_selection ? (m_active ? &m_selection->m_custom_hot_image : &m_selection->m_custom_image)
                                      : nullptr;
                SendMessage(m_child, MSG_BUTTON_CHANGE, 0, 0);
            } break;
            }
            break;
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

void ButtonsToolbar::ConfigParam::update_size_field_status()
{
    const auto _ = pfc::vartoggle_t(m_initialising, true);

    const auto is_custom_size = m_icon_size == IconSize::Custom;
    EnableWindow(GetDlgItem(m_wnd, IDC_WIDTH), is_custom_size);
    EnableWindow(GetDlgItem(m_wnd, IDC_HEIGHT), is_custom_size);

    uSetDlgItemText(m_wnd, IDC_WIDTH, is_custom_size ? std::to_string(m_width.get_scaled_value()).c_str() : "");
    uSetDlgItemText(m_wnd, IDC_HEIGHT, is_custom_size ? std::to_string(m_height.get_scaled_value()).c_str() : "");
}

} // namespace cui::toolbars::buttons
