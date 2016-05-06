#include "stdafx.h"
#include "buttons.h"

void toolbar_extension::config_param::export_to_stream(stream_writer * p_file, bool b_paths, abort_callback & p_abort)
{
	p_file->write_lendian_t(g_guid_fcb, p_abort);
	p_file->write_lendian_t(VERSION_CURRENT, p_abort);

	unsigned n, count = m_buttons.get_count();

	p_file->write_lendian_t(I_TEXT_BELOW, p_abort);
	p_file->write_lendian_t(sizeof(m_text_below), p_abort);
	p_file->write_lendian_t(m_text_below, p_abort);

	p_file->write_lendian_t(I_APPEARANCE, p_abort);
	p_file->write_lendian_t(sizeof(m_appearance), p_abort);
	p_file->write_lendian_t(m_appearance, p_abort);

	p_file->write_lendian_t(I_BUTTONS, p_abort);

	stream_writer_memblock p_write;
	//FIX

	p_file->write_lendian_t(p_write.m_data.get_size() + sizeof(count), p_abort);
	p_file->write_lendian_t(count, p_abort);

	for (n = 0; n<count; n++)
	{
		m_buttons[n].write_to_file(p_write, b_paths, p_abort);
		p_file->write_lendian_t(p_write.m_data.get_size(), p_abort);
		p_file->write(p_write.m_data.get_ptr(), p_write.m_data.get_size(), p_abort);
		p_write.m_data.set_size(0);
	}
}

void toolbar_extension::config_param::export_to_file(const char * p_path, bool b_paths)
{

	try {
		abort_callback_impl p_abort;
		service_ptr_t<file> p_file;
		filesystem::g_open(p_file, p_path, filesystem::open_mode_write_new, p_abort);
		export_to_stream(p_file.get_ptr(), b_paths, p_abort);
	}
	catch (const pfc::exception & p_error)
	{
		popup_message::g_show_ex(p_error.what(), pfc_infinite, "Error writing FCB file", pfc_infinite);
		abort_callback_dummy abortCallback;
		filesystem::g_remove(p_path, abortCallback);
	}
}

void toolbar_extension::config_param::import_from_stream(stream_reader * p_file, bool add, abort_callback & p_abort)
{
	const char * profilepath = core_api::get_profile_path();
	if (!profilepath) throw pfc::exception_bug_check("NULL profile path");
	if (!stricmp_utf8_max(profilepath, "file://", 7))
		profilepath += 7;
	pfc::string8 str_base = profilepath;
	t_size blen = str_base.get_length();
	if (blen && str_base[blen - 1] == '\\')
		str_base.truncate(blen - 1);
	//uGetModuleFileName(NULL, str_base);
	//unsigned pos = str_base.find_last('\\');
	//if (pos != -1)
	//	str_base.truncate(pos);
	if (!add)
		m_buttons.remove_all();

	{
		GUID temp;
		p_file->read_lendian_t(temp, p_abort);
		if (temp != g_guid_fcb)
			throw exception_io_data();
		t_config_version vers;
		p_file->read_lendian_t(vers, p_abort);
		if (vers > VERSION_CURRENT)
			throw "Fcb version is newer than component";
		while (1) //!p_file.is_eof(p_abort)
		{
			t_identifier id;
			try {
				p_file->read_lendian_t(id, p_abort);
			}
			catch (exception_io_data_truncation const &) { break; }
			unsigned size;
			p_file->read_lendian_t(size, p_abort);
			//if (size > p_file->get_size(p_abort) - p_file->get_position(p_abort))
			//	throw exception_io_data();
			switch (id)
			{
			case I_TEXT_BELOW:
				p_file->read_lendian_t(m_text_below, p_abort);
				break;
			case I_APPEARANCE:
				p_file->read_lendian_t(m_appearance, p_abort);
				break;
			case I_BUTTONS:
			{
				service_ptr_t<genrand_service> genrand = genrand_service::g_create();
				genrand->seed(GetTickCount());
				t_uint32 dirname = genrand->genrand(pfc_infinite);

				unsigned count, n;
				p_file->read_lendian_t(count, p_abort);
				for (n = 0; n<count; n++)
				{
					button temp = g_button_null;
					unsigned size_button;
					p_file->read_lendian_t(size_button, p_abort);
					pfc::string_formatter formatter;
					temp.read_from_file(vers, str_base, formatter << dirname, p_file, size_button, p_abort);
					//						assert(n < 7);
					m_buttons.add_item(temp);
				}

			}
			break;
			}
		}
	}
}
void toolbar_extension::config_param::import_from_file(const char * p_path, bool add)
{
	try {
		abort_callback_impl p_abort;
		service_ptr_t<file> p_file;
		filesystem::g_open(p_file, p_path, filesystem::open_mode_read, p_abort);
		import_from_stream(p_file.get_ptr(), add, p_abort);
	}
	catch (const pfc::exception & p_error)
	{
		popup_message::g_show_ex(p_error.what(), pfc_infinite, "Error reading FCB file", pfc_infinite);
	}
	catch (const char * p_error)
	{
		popup_message::g_show_ex(p_error, pfc_infinite, "Error reading FCB file", pfc_infinite);;
	}
}

void toolbar_extension::config_param::on_selection_change(t_size index)
{
	m_selection = index != pfc_infinite && index < m_buttons.get_count() ? &m_buttons[index] : NULL;
	m_image = m_selection ? (m_active ? &m_selection->m_custom_hot_image : &m_selection->m_custom_image) : 0;
	pfc::string8 temp;
	if (m_selection)
	{
		m_selection->get_name(temp);
	}
	uSendDlgItemMessageText(m_wnd, IDC_COMMAND_DESC, WM_SETTEXT, 0, temp);
	SendDlgItemMessage(m_wnd, IDC_SHOW, CB_SETCURSEL, m_selection ? m_selection->m_show : -1, 0);

	bool b_enable = index != pfc_infinite && m_selection && m_selection->m_type != TYPE_SEPARATOR;
	Button_SetCheck(GetDlgItem(m_wnd, IDC_USE_CUSTOM_TEXT), b_enable ? m_selection->m_use_custom_text : FALSE);
	SendDlgItemMessage(m_wnd, IDC_TEXT, WM_SETTEXT, 0, b_enable ? (LPARAM)pfc::stringcvt::string_os_from_utf8(m_selection->m_text).get_ptr() : (LPARAM)_T(""));
	EnableWindow(GetDlgItem(m_wnd, IDC_PICK), index != pfc_infinite);
	EnableWindow(GetDlgItem(m_wnd, IDC_SHOW), b_enable);
	EnableWindow(GetDlgItem(m_wnd, IDC_USE_CUSTOM_TEXT), b_enable);
	EnableWindow(GetDlgItem(m_wnd, IDC_TEXT), b_enable && m_selection->m_use_custom_text);
	SendMessage(m_child, MSG_BUTTON_CHANGE, 0, 0);

}

void toolbar_extension::config_param::populate_buttons_list()
{
	unsigned n, count = m_buttons.get_count();

	pfc::string8_fast_aggressive name;
	pfc::array_staticsize_t<t_list_view::t_item_insert> items(count);
	for (n = 0; n<count; n++)
	{
		m_buttons[n].get_name_name(name);
		items[n].m_subitems.add_item(name);
		m_buttons[n].get_name_type(name);
		items[n].m_subitems.add_item(name);
	}
	m_button_list.insert_items(0, count, items.get_ptr());
}

void toolbar_extension::config_param::refresh_buttons_list_items(t_size index, t_size count, bool b_update_display)
{
	unsigned n, real_count = m_buttons.get_count();

	if (index + count > real_count) count = real_count - index;

	pfc::string8_fast_aggressive name;
	pfc::list_t<t_list_view::t_item_insert> items;
	items.set_count(count);
	for (n = index; n<index + count; n++)
	{
		m_buttons[n].get_name_name(name);
		items[n - index].m_subitems.add_item(name);
		m_buttons[n].get_name_type(name);
		items[n - index].m_subitems.add_item(name);
	}
	m_button_list.replace_items(index, items, b_update_display);
}

BOOL CALLBACK toolbar_extension::config_param::g_ConfigPopupProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	config_param * ptr = NULL;
	switch (msg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(wnd, DWLP_USER, lp);
		ptr = reinterpret_cast<config_param*>(lp);
		break;
	default:
		ptr = reinterpret_cast<config_param*>(GetWindowLongPtr(wnd, DWLP_USER));
		break;
	};
	return ptr ? ptr->ConfigPopupProc(wnd, msg, wp, lp) : FALSE;
}


BOOL toolbar_extension::config_param::ConfigPopupProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		m_wnd = wnd;
		m_scope.initialize(FindOwningPopup(wnd));

		HWND wnd_show = GetDlgItem(wnd, IDC_SHOW);

		SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM)_T("Image"));
		SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM)_T("Image and text"));
		SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM)_T("Text"));

		HWND wnd_text = GetDlgItem(wnd, IDC_TEXT_LOCATION);

		SendMessage(wnd_text, CB_ADDSTRING, 0, (LPARAM)_T("Right"));
		SendMessage(wnd_text, CB_ADDSTRING, 0, (LPARAM)_T("Below"));

		HWND wnd_app = GetDlgItem(wnd, IDC_APPEARANCE);

		SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM)_T("Normal"));
		SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM)_T("Flat"));
		SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM)_T("No edges"));

		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB);
		uTabCtrl_InsertItemText(wnd_tab, 0, "Normal image");
		uTabCtrl_InsertItemText(wnd_tab, 1, "Hot image");

		RECT tab;

		GetWindowRect(wnd_tab, &tab);
		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);

		TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

		m_child = CreateDialogParam(core_api::get_my_instance(), MAKEINTRESOURCE(IDD_IMAGE), wnd, ConfigChildProc, (LPARAM)this);

		SetWindowPos(m_child, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		if (m_child)
		{
			{
				EnableThemeDialogTexture(m_child, ETDT_ENABLETAB);
			}
		}

		SetWindowPos(m_child, 0, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
		//SetWindowPos(wnd_tab,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		ShowWindow(m_child, SW_NORMAL);
		UpdateWindow(m_child);

		SendMessage(wnd_text, CB_SETCURSEL, m_text_below ? 1 : 0, 0);
		SendMessage(wnd_app, CB_SETCURSEL, m_appearance, 0);

		HWND wnd_button_list = m_button_list.create_in_dialog_units(wnd, ui_helpers::window_position_t(14, 16, 310, 106));
		populate_buttons_list();
		ShowWindow(wnd_button_list, SW_SHOWNORMAL);

	}
	return TRUE;
	case WM_DESTROY:
		m_button_list.destroy();
		break;
#if 0
	case WM_PAINT:
		uih::HandleModernBackgroundPaint(wnd, GetDlgItem(wnd, IDOK));
		return TRUE;
	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
		SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
		return (BOOL)GetSysColorBrush(COLOR_WINDOW);
#endif
		//case WM_ERASEBKGND:
		//	SetWindowLongPtr(wnd, DWLP_MSGRESULT, FALSE);
		//	return TRUE;
		//case WM_PAINT:
		//	uih::HandleModernBackgroundPaint(wnd, GetDlgItem(wnd, IDOK));
		//	return TRUE;
		//case WM_CTLCOLORBTN:
		//case WM_CTLCOLORDLG:
		//case WM_CTLCOLORSTATIC:
		/*SetBkColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
		SetDCBrushColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
		SetDCPenColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
		SetBkMode((HDC)wp, TRANSPARENT);
		//SetROP2((HDC)wp, R2_BLACK);
		SelectBrush((HDC)wp, GetSysColorBrush(COLOR_3DDKSHADOW));
		return (BOOL)GetSysColorBrush(COLOR_3DDKSHADOW);*/
		//return FALSE;
	case WM_COMMAND:
		switch (wp)
		{
		case IDCANCEL:
		{
			EndDialog(wnd, 0);
		}
		return TRUE;
		case (CBN_SELCHANGE << 16) | IDC_SHOW:
		{
			if (m_selection)
			{
				m_selection->m_show = (t_show)SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
			}
		}
		break;
		case (CBN_SELCHANGE << 16) | IDC_TEXT_LOCATION:
		{
			m_text_below = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0) != 0;
		}
		break;
		case (CBN_SELCHANGE << 16) | IDC_APPEARANCE:
		{
			m_appearance = (t_appearance)SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
		}
		break;
		case IDC_ADD:
		{
			command_picker_data p_temp;
			command_picker_param p_data(g_button_null.m_guid, g_button_null.m_subcommand,
				g_button_null.m_type, g_button_null.m_filter);
			p_temp.set_data(p_data);
			if (uDialogBox(IDD_COMMAND, wnd, ConfigCommandProc, reinterpret_cast<LPARAM>(&p_temp)))
			{
				t_size index = m_buttons.add_item(g_button_null);

				p_temp.get_data(p_data);
				m_buttons[index].m_type = (t_type)p_data.m_group;
				m_buttons[index].m_guid = p_data.m_guid;
				m_buttons[index].m_subcommand = p_data.m_subcommand;
				m_buttons[index].m_filter = (t_filter)p_data.m_filter;

				pfc::string8_fast_aggressive name;
				//m_buttons[index].get_name(name);
				//unsigned idx = uSendDlgItemMessageText(wnd, IDC_BUTTON_LIST, LB_ADDSTRING, 0, name);

				t_list_view::t_item_insert item;
				m_buttons[index].get_name_name(name);
				item.m_subitems.add_item(name);
				m_buttons[index].get_name_type(name);
				item.m_subitems.add_item(name);
				t_size index_list = m_button_list.get_item_count();
				m_button_list.insert_items(index_list, 1, &item);
				m_button_list.set_item_selected_single(index_list);
				m_button_list.ensure_visible(index_list);

				//SendDlgItemMessage(wnd, IDC_BUTTON_LIST, LB_SETCURSEL, idx, 0);
				//SendMessage(wnd, WM_COMMAND, (LBN_SELCHANGE<<16)|IDC_BUTTON_LIST, (LPARAM)GetDlgItem(wnd,IDC_BUTTON_LIST));
			}
		}
		break;
		case IDC_RESET:
		{
			if (win32_helpers::message_box(wnd, _T("This will reset all your buttons to the default buttons. Continue?"), _T("Reset buttons"), MB_YESNO) == IDYES)
			{
				m_button_list.remove_items(bit_array_true(), false);
				toolbar_extension::reset_buttons(m_buttons);
				populate_buttons_list();
			}
		}
		break;
		case IDC_REMOVE:
		{
			t_size index = m_button_list.get_selected_item_single();
			if (index != pfc_infinite)
			{
				m_button_list.remove_item(index);
				m_buttons.remove_by_idx(index);
				if (index < m_button_list.get_item_count())
					m_button_list.set_item_selected_single(index);
				else if (index)
					m_button_list.set_item_selected_single(index - 1);
			}
		}
		break;
#if 0
		case IDC_UP:
		{
			t_size index = m_button_list.get_selected_item_single();

			if (index != pfc_infinite && index < m_buttons.get_count() && index)
			{
				m_buttons.swap_items(index, index - 1);

				//blaarrgg, designed in the dark ages
				m_selection = &m_buttons[index - 1];

				refresh_buttons_list_items(index - 1, 2);
				m_button_list.set_item_selected_single(index - 1);

			}
		}
		break;
		case IDC_DOWN:
		{
			t_size index = m_button_list.get_selected_item_single();
			if (index != pfc_infinite && index + 1 < m_buttons.get_count())
			{
				m_buttons.swap_items(index, index + 1);

				//blaarrgg, designed in the dark ages
				m_selection = &m_buttons[index + 1];

				refresh_buttons_list_items(index, 2);
				m_button_list.set_item_selected_single(index + 1);
			}
		}
		break;
#endif
		case IDC_TOOLS:
		{
			RECT rc;
			GetWindowRect(HWND(lp), &rc);
			HMENU menu = CreatePopupMenu();
			enum { IDM_SET_MASK = 1, IDM_EXPORT, IDM_SAVE, IDM_LOAD, IDM_ADD };

			//AppendMenu(menu,MF_SEPARATOR,0,0);
			AppendMenu(menu, MF_STRING, IDM_LOAD, _T("Load from file..."));
			AppendMenu(menu, MF_STRING, IDM_ADD, _T("Add from file..."));
			AppendMenu(menu, MF_STRING, IDM_EXPORT, _T("Save to file (embed images)..."));
			AppendMenu(menu, MF_STRING, IDM_SAVE, _T("Save to file (store image paths)..."));

			int cmd = TrackPopupMenu(menu, TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rc.left, rc.bottom, 0, wnd, 0);
			DestroyMenu(menu);
			if (cmd == IDM_SAVE)
			{
				pfc::string8 path;
				if (uGetOpenFileName(wnd, "fcb files (*.fcb)|*.fcb|All files (*.*)|*.*", 0, "fcb", "Save as", NULL, path, TRUE))
				{
					export_to_file(path, true);
				}
			}
			else if (cmd == IDM_EXPORT)
			{
				pfc::string8 path;
				if (uGetOpenFileName(wnd, "fcb files (*.fcb)|*.fcb|All files (*.*)|*.*", 0, "fcb", "Save as", NULL, path, TRUE))
				{
					export_to_file(path);
				}
			}
			else if (cmd == IDM_LOAD || cmd == IDM_ADD)
			{
				pfc::string8 path;
				if (uGetOpenFileName(wnd, "fcb Files (*.fcb)|*.fcb|All Files (*.*)|*.*", 0, "fcb", "Open file", NULL, path, FALSE))
				{
					m_button_list.remove_items(bit_array_true(), false);

					HWND wnd_show = GetDlgItem(wnd, IDC_SHOW);
					HWND wnd_text = GetDlgItem(wnd, IDC_TEXT_LOCATION);
					HWND wnd_app = GetDlgItem(wnd, IDC_APPEARANCE);

					import_from_file(path, cmd == IDM_ADD);
					SendMessage(wnd_text, CB_SETCURSEL, m_text_below ? 1 : 0, 0);
					SendMessage(wnd_app, CB_SETCURSEL, m_appearance, 0);
					populate_buttons_list();
				}
			}
		}
		break;
		case IDC_USE_CUSTOM_TEXT:
		{
			if (m_selection)
			{
				m_selection->m_use_custom_text = Button_GetCheck(HWND(lp)) != 0;
				bool b_enable = m_selection->m_type != TYPE_SEPARATOR;
				EnableWindow(GetDlgItem(wnd, IDC_TEXT), !b_enable || m_selection->m_use_custom_text);
			}
		}
		break;
		case IDC_TEXT | (EN_CHANGE << 16) :
		{
			if (m_selection)
			{
				m_selection->m_text = string_utf8_from_window(HWND(lp));
			}
		}
										  break;
		case IDC_PICK:
		{
			if (m_selection)
			{
				command_picker_data p_temp;
				command_picker_param p_data(m_selection->m_guid, m_selection->m_subcommand,
					m_selection->m_type, m_selection->m_filter);
				p_temp.set_data(p_data);
				if (uDialogBox(IDD_COMMAND, wnd, ConfigCommandProc, reinterpret_cast<LPARAM>(&p_temp)))
				{
					p_temp.get_data(p_data);
					m_selection->m_type = (t_type)p_data.m_group;
					m_selection->m_guid = p_data.m_guid;
					m_selection->m_subcommand = p_data.m_subcommand;
					m_selection->m_filter = (t_filter)p_data.m_filter;

					unsigned idx = m_button_list.get_selected_item_single();
					if (idx != pfc_infinite)
					{
						refresh_buttons_list_items(idx, 1);

						pfc::string8 name;

						m_buttons[idx].get_name(name);

						uSendDlgItemMessageText(wnd, IDC_COMMAND_DESC, WM_SETTEXT, 0, name);
					}
					bool b_enable = m_selection->m_type != TYPE_SEPARATOR;
					EnableWindow(GetDlgItem(wnd, IDC_SHOW), m_selection->m_type != TYPE_SEPARATOR);
					EnableWindow(GetDlgItem(wnd, IDC_USE_CUSTOM_TEXT), b_enable);
					EnableWindow(GetDlgItem(wnd, IDC_TEXT), !b_enable || m_selection->m_use_custom_text);
					SendMessage(m_child, MSG_COMMAND_CHANGE, 0, 0);
				}
			}
		}
		break;
		case IDOK:
		{
			EndDialog(wnd, 1);
		}
		return TRUE;
		default:
			return FALSE;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lp)->idFrom)
		{
		case IDC_TAB:
			switch (((LPNMHDR)lp)->code)
			{
			case TCN_SELCHANGE:
			{
				m_active = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB));
				m_image = m_selection ? (m_active ? &m_selection->m_custom_hot_image : &m_selection->m_custom_image) : 0;
				SendMessage(m_child, MSG_BUTTON_CHANGE, 0, 0);
			}
			break;
			}
			break;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
}





