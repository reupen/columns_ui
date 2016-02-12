#include "stdafx.h"

bool command_picker_data::__populate_mainmenu_dynamic_recur(command_data & data, const mainmenu_node::ptr & ptr_node, pfc::string_base & full, bool b_root)
{
	if (ptr_node.is_valid())
	{
		switch (ptr_node->get_type())
		{
		case mainmenu_node::type_command:
		{
			pfc::string8 subfull = full, subname;
			t_uint32 flags;
			ptr_node->get_display(subname, flags);

			if (subfull.length() && subfull.get_ptr()[subfull.length() - 1] != '/')
				subfull.add_byte('/');
			subfull.add_string(subname);

			command_data * p_data = new command_data(data);
			p_data->m_subcommand = ptr_node->get_guid();
			ptr_node->get_description(p_data->m_desc);

			m_data.add_item(p_data);

			unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, subfull);
			SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
		}
		return true;
		case mainmenu_node::type_group:
		{

			pfc::string8 name, subfull = full;
			if (!b_root)
			{
				t_uint32 flags;
				ptr_node->get_display(name, flags);
				if (subfull.length() && subfull.get_ptr()[subfull.length() - 1] != '/')
					subfull.add_byte('/');
				subfull.add_string(name);
			}

			mainmenu_node::ptr ptr_child;
			for (t_size i = 0, count = ptr_node->get_children_count(); i<count; i++)
			{
				ptr_child = ptr_node->get_child(i);
				__populate_mainmenu_dynamic_recur(data, ptr_child, subfull, false);
			}
		}
		return true;
		default:
			return false;
		};
	}
	return false;
}
bool command_picker_data::__populate_commands_recur(command_data & data, pfc::string_base & full, contextmenu_item_node * p_node, bool b_root)
{
	if (p_node)
	{
		if (p_node->get_type() == contextmenu_item_node::TYPE_POPUP)
		{
			pfc::string8 name, subfull = full;
			unsigned dummy;
			p_node->get_display_data(name, dummy, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list);
			if (subfull.length() && subfull.get_ptr()[subfull.length() - 1] != '/')
				subfull.add_byte('/');
			subfull.add_string(name);

			unsigned child, child_count = p_node->get_children_count();
			for (child = 0; child<child_count; child++)
			{
				contextmenu_item_node * p_child = p_node->get_child(child);
				__populate_commands_recur(data, subfull, p_child, false);
			}
			return true;
		}
		else if (p_node->get_type() == contextmenu_item_node::TYPE_COMMAND && !b_root)
		{
			pfc::string8 subfull = full, subname;
			unsigned dummy;
			p_node->get_display_data(subname, dummy, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list);
			if (subfull.length() && subfull.get_ptr()[subfull.length() - 1] != '/')
				subfull.add_byte('/');
			subfull.add_string(subname);

			command_data * p_data = new command_data(data);
			p_data->m_subcommand = p_node->get_guid();
			p_node->get_description(p_data->m_desc);

			m_data.add_item(p_data);

			unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, subfull);
			SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
			return true;
		}
	}
	return false;
}
void command_picker_data::populate_commands()
{
	SendMessage(wnd_command, LB_RESETCONTENT, 0, 0);
	m_data.delete_all();
	SendMessage(wnd_command, WM_SETREDRAW, FALSE, 0);
	if (m_group == 2)
	{
		service_enum_t<contextmenu_item> e;
		service_ptr_t<contextmenu_item> ptr;

		unsigned p_item_index = 0, p_service_item_index;//,n=0;
		while (e.next(ptr))
		{
			{
				unsigned p_service_item_count = ptr->get_num_items();
				for (p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++)
				{
					pfc::ptrholder_t<contextmenu_item_node_root> p_node(ptr->instantiate_item(p_service_item_index, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list));

					command_data data;
					data.m_guid = ptr->get_item_guid(p_service_item_index);

					pfc::string8 name, full;
					ptr->get_item_default_path(p_service_item_index, full);

					if (p_node.is_valid() && __populate_commands_recur(data, full, p_node.get_ptr(), true))
					{
					}
					else
					{
						ptr->get_item_name(p_service_item_index, name);
						if (full.length() && full[full.length() - 1] != '/')
							full.add_byte('/');
						full.add_string(name);

						command_data * p_data = new command_data(data);
						ptr->get_item_description(p_service_item_index, p_data->m_desc);
						m_data.add_item(p_data);

						unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, full);
						SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
						//							n++;
					}
				}
			}
		}
	}
	else if (m_group == 3)
	{
		service_enum_t<mainmenu_commands> e;
		service_ptr_t<mainmenu_commands> ptr;

		unsigned p_item_index = 0, p_service_item_index;//,n=0;
		while (e.next(ptr))
		{
			service_ptr_t<mainmenu_commands_v2> ptr_v2;
			ptr->service_query_t(ptr_v2);
			{
				unsigned p_service_item_count = ptr->get_command_count();
				for (p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++)
				{
					command_data data;
					data.m_guid = ptr->get_command(p_service_item_index);
					pfc::string8 name, full;
					ptr->get_name(p_service_item_index, name);
					{
						pfc::list_t<pfc::string8> levels;
						GUID parent = ptr->get_parent();
						while (parent != pfc::guid_null)
						{
							pfc::string8 parentname;
							if (menu_helpers::maingroupname_from_guid(GUID(parent), parentname, parent))
								levels.insert_item(parentname, 0);
						}
						unsigned i, count = levels.get_count();
						for (i = 0; i<count; i++)
						{
							full.add_string(levels[i]);
							full.add_byte('/');

						}
					}
					full.add_string(name);

					if (ptr_v2.is_valid() && ptr_v2->is_command_dynamic(p_service_item_index))
					{
						mainmenu_node::ptr ptr_node = ptr_v2->dynamic_instantiate(p_service_item_index);
						__populate_mainmenu_dynamic_recur(data, ptr_node, full, true);
					}
					else
					{
						command_data * p_data = new command_data(data);
						ptr->get_description(p_service_item_index, p_data->m_desc);
						m_data.add_item(p_data);
						unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, full);
						SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
					}
				}
			}
		}
	}
	else if (m_group == 1)
	{
		service_enum_t<uie::button> e;
		service_ptr_t<uie::button> ptr;
		while (e.next(ptr))
		{
			service_ptr_t<uie::custom_button> p_button;
			if (ptr->get_guid_type() == uie::BUTTON_GUID_BUTTON && ptr->service_query_t(p_button))
			{
				command_data * p_data = new command_data;
				p_data->m_guid = ptr->get_item_guid();
				p_button->get_description(p_data->m_desc);
				m_data.add_item(p_data);
				pfc::string8 temp;
				p_button->get_name(temp);
				unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, temp);
				SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
			}
		}
	}
	unsigned n, count = SendMessage(wnd_command, LB_GETCOUNT, 0, 0);
	for (n = 0; n<count; n++)
	{
		LRESULT ret = SendMessage(wnd_command, LB_GETITEMDATA, n, 0);
		command_data* p_data = ((command_data*)ret);

		if (ret != LB_ERR && p_data->m_guid == m_guid && p_data->m_subcommand == m_subcommand)
		{
			SendMessage(wnd_command, LB_SETCURSEL, n, 0);
			update_description();
			break;
		}
	}
	SendMessage(wnd_command, WM_SETREDRAW, TRUE, 0);
}
void command_picker_data::update_description()
{
	LRESULT p_command = SendMessage(wnd_command, LB_GETCURSEL, 0, 0);
	if (p_command != LB_ERR)
	{
		LRESULT p_data = SendMessage(wnd_command, LB_GETITEMDATA, p_command, 0);
		if (p_data != LB_ERR)
			uSendDlgItemMessageText(m_wnd, IDC_DESC, WM_SETTEXT, 0, ((command_data*)p_data)->m_desc);
		else
			uSendDlgItemMessageText(m_wnd, IDC_DESC, WM_SETTEXT, 0, "");
	}
	else
		uSendDlgItemMessageText(m_wnd, IDC_DESC, WM_SETTEXT, 0, "");
}

void command_picker_data::set_data(const command_picker_param & p_data)
{
	m_group = p_data.m_group;
	m_guid = p_data.m_guid;
	m_subcommand = p_data.m_subcommand;
	m_filter = p_data.m_filter;
}
void command_picker_data::get_data(command_picker_param & p_data) const
{
	p_data.m_group = m_group;
	p_data.m_guid = m_guid;
	p_data.m_subcommand = m_subcommand;
	p_data.m_filter = m_filter;
}
void command_picker_data::initialise(HWND wnd)
{
	m_wnd = wnd;
	wnd_group = GetDlgItem(wnd, IDC_GROUP);
	wnd_filter = GetDlgItem(wnd, IDC_ITEM);
	wnd_command = GetDlgItem(wnd, IDC_COMMAND);

	SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM)_T("Separator"));
	SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM)_T("Buttons"));
	SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM)_T("Shortcut menu items"));
	SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM)_T("Main menu items"));

	SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM)_T("None"));
	SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM)_T("Now playing item"));
	SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM)_T("Current playlist selection"));
	SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM)_T("Active selection"));

	SendMessage(wnd_group, LB_SETCURSEL, m_group, 0);

}
void command_picker_data::deinitialise(HWND wnd)
{
	SendMessage(wnd_group, LB_RESETCONTENT, 0, 0);
	SendMessage(wnd_filter, LB_RESETCONTENT, 0, 0);
	SendMessage(wnd_command, LB_RESETCONTENT, 0, 0);
	m_data.delete_all();
}
BOOL command_picker_data::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		m_scope.initialize(FindOwningPopup(wnd));
		initialise(wnd);
		populate_commands();
		SendMessage(wnd_filter, LB_SETCURSEL, (WPARAM)m_filter, 0);
	}
	return TRUE;
	case WM_DESTROY:
		deinitialise(wnd);
		return TRUE;
	case WM_ERASEBKGND:
		SetWindowLongPtr(wnd, DWLP_MSGRESULT, TRUE);
		return TRUE;
	case WM_PAINT:
		uih::HandleModernBackgroundPaint(wnd, GetDlgItem(wnd, IDOK));
		return TRUE;
	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
		SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
		return (BOOL)GetSysColorBrush(COLOR_WINDOW);
	case WM_COMMAND:
		switch (wp)
		{
		case IDC_GROUP | (LBN_SELCHANGE << 16) :
			m_group = SendMessage(wnd_group, LB_GETCURSEL, 0, 0);
			m_guid.reset();
			m_subcommand.reset();
			populate_commands();
			return TRUE;
		case IDC_ITEM | (LBN_SELCHANGE << 16) :
		{
			LRESULT p_filter = SendMessage(wnd_filter, LB_GETCURSEL, 0, 0);
			if (p_filter != LB_ERR)
				m_filter = p_filter;
		}
											  return TRUE;
		case IDC_COMMAND | (LBN_SELCHANGE << 16) :
		{
			m_guid.reset();
			m_subcommand.reset();

			LRESULT p_command = SendMessage(wnd_command, LB_GETCURSEL, 0, 0);
			if (p_command != LB_ERR)
			{
				LRESULT ret = SendMessage(wnd_command, LB_GETITEMDATA, p_command, 0);
				command_data* p_data = (command_data*)ret;
				if (ret != LB_ERR)
				{
					m_guid = p_data->m_guid;
					m_subcommand = p_data->m_subcommand;
				}
			}
			update_description();
		}
												 return TRUE;
		case IDCANCEL:
		{
			EndDialog(wnd, 0);
		}
		return TRUE;
		case IDOK:
		{
			EndDialog(wnd, 1);
		}
		return TRUE;
		}
		break;
	}
	return FALSE;
}

