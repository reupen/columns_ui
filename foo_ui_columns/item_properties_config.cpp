#include "stdafx.h"
#include "item_properties.h"

BOOL CALLBACK selection_properties_config_t::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		pfc::vartoggle_t<bool> init(m_initialising, true);

		HWND wnd_fields = m_field_list.create_in_dialog_units(wnd, ui_helpers::window_position_t(21, 17, 226, 150));
		SetWindowPos(wnd_fields, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		ShowWindow(wnd_fields, SW_SHOWNORMAL);

		HWND wnd_lv = GetDlgItem(wnd, IDC_INFOSECTIONS);
		ListView_SetExtendedListViewStyleEx(wnd_lv, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
		uih::SetListViewWindowExplorerTheme(wnd_lv);

		RECT rc;
		GetClientRect(wnd_lv, &rc);
		uih::ListView_InsertColumnText(wnd_lv, 0, L"", RECT_CX(rc));

		t_size i, count = tabsize(g_info_sections);
		for (i = 0; i < count; i++)
		{
			uih::ListView_InsertItemText(wnd_lv, i, 0, g_info_sections[i].name);
			ListView_SetCheckState(wnd_lv, i, (m_info_sections_mask & (1 << g_info_sections[i].id)) ? TRUE : FALSE);
		}

		HWND wnd_combo = GetDlgItem(wnd, IDC_EDGESTYLE);
		ComboBox_AddString(wnd_combo, L"None");
		ComboBox_AddString(wnd_combo, L"Sunken");
		ComboBox_AddString(wnd_combo, L"Grey");
		ComboBox_SetCurSel(wnd_combo, m_edge_style);

		Button_SetCheck(GetDlgItem(wnd, IDC_SHOWCOLUMNS), m_show_columns ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(GetDlgItem(wnd, IDC_SHOWGROUPS), m_show_groups ? BST_CHECKED : BST_UNCHECKED);
	}
	break;
	case WM_DESTROY:
	{
		m_field_list.destroy();
	}
	break;
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
	case WM_NOTIFY:
	{
		LPNMHDR lpnm = (LPNMHDR)lp;
		switch (lpnm->idFrom)
		{
		case IDC_INFOSECTIONS:
			switch (lpnm->code)
			{
			case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lp;
				if (!m_initialising && lpnmlv->iItem < tabsize(g_info_sections) && (lpnmlv->uChanged & LVIF_STATE))
				{
					m_info_sections_mask = m_info_sections_mask & ~(1 << g_info_sections[lpnmlv->iItem].id);

					//if (((((UINT)(lpnmlv->uNewState & LVIS_STATEIMAGEMASK )) >> 12) -1))
					if (ListView_GetCheckState(lpnm->hwndFrom, lpnmlv->iItem))
						m_info_sections_mask = m_info_sections_mask | (1 << g_info_sections[lpnmlv->iItem].id);

				}
			}
			break;
			};
			break;
		};
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wp))
		{
		case IDOK:
			EndDialog(wnd, 1);
			break;
		case IDCANCEL:
			EndDialog(wnd, 0);
			break;
		case IDC_SHOWCOLUMNS:
			m_show_columns = Button_GetCheck((HWND)lp) != 0;
			break;
		case IDC_SHOWGROUPS:
			m_show_groups = Button_GetCheck((HWND)lp) != 0;
			break;
		case IDC_EDGESTYLE:
			switch (HIWORD(wp))
			{
			case CBN_SELCHANGE:
				m_edge_style = ComboBox_GetCurSel((HWND)lp);
				break;
			}
			break;
		case IDC_UP:
		{
			if (m_field_list.get_selection_count(2) == 1)
			{
				t_size index = 0, count = m_field_list.get_item_count();
				while (!m_field_list.get_item_selected(index) && index < count) index++;
				if (index && m_fields.get_count())
				{
					m_fields.swap_items(index, index - 1);

					pfc::list_t<t_list_view::t_item_insert> items;
					m_field_list.get_insert_items(index - 1, 2, items);
					m_field_list.replace_items(index - 1, items);
					m_field_list.set_item_selected_single(index - 1);
				}
			}
		}
		break;
		case IDC_DOWN:
		{
			if (m_field_list.get_selection_count(2) == 1)
			{
				t_size index = 0, count = m_field_list.get_item_count();
				while (!m_field_list.get_item_selected(index) && index < count) index++;
				if (index + 1 < count && index + 1 < m_fields.get_count())
				{
					m_fields.swap_items(index, index + 1);

					pfc::list_t<t_list_view::t_item_insert> items;
					m_field_list.get_insert_items(index, 2, items);
					m_field_list.replace_items(index, items);
					m_field_list.set_item_selected_single(index + 1);
				}
			}
		}
		break;
		case IDC_NEW:
		{
			field_t temp;
			temp.m_name_friendly = "<enter name here>";
			temp.m_name = "<ENTER FIELD HERE>";
			t_size index = m_fields.add_item(temp);

			pfc::list_t<t_list_view::t_item_insert> items;
			m_field_list.get_insert_items(index, 1, items);
			m_field_list.insert_items(index, 1, items.get_ptr());
			m_field_list.set_item_selected_single(index);
			SetFocus(m_field_list.get_wnd());
			m_field_list.activate_inline_editing();

		}
		break;
		case IDC_REMOVE:
		{
			if (m_field_list.get_selection_count(2) == 1)
			{
				bit_array_bittable mask(m_field_list.get_item_count());
				m_field_list.get_selection_state(mask);
				//bool b_found = false;
				t_size index = 0, count = m_field_list.get_item_count();
				while (index < count)
				{
					if (mask[index]) break;
					index++;
				}
				if (index < count && index < m_fields.get_count())
				{
					m_fields.remove_by_idx(index);
					m_field_list.remove_item(index);
					t_size new_count = m_field_list.get_item_count();
					if (new_count)
					{
						if (index < new_count)
							m_field_list.set_item_selected_single(index);
						else if (index)
							m_field_list.set_item_selected_single(index - 1);
					}
				}
			}
		}
		break;
		}
		break;
	}
	return FALSE;
}

bool selection_properties_config_t::run_modal(HWND wnd)
{
	return uDialogBox(IDD_SELECTIONCONFIG, wnd, g_DialogProc, (LPARAM)this) != 0;
}

selection_properties_config_t::selection_properties_config_t(pfc::list_t<field_t>  p_fields, t_size edge_style, t_uint32 info_sections_mask, bool b_show_columns, bool b_show_groups) : m_fields(std::move(p_fields)), m_edge_style(edge_style), m_info_sections_mask(info_sections_mask), m_show_columns(b_show_columns),
m_show_groups(b_show_groups), m_initialising(false), m_field_list(m_fields)
{

}

BOOL CALLBACK selection_properties_config_t::g_DialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	selection_properties_config_t * p_data = nullptr;
	if (msg == WM_INITDIALOG)
	{
		p_data = reinterpret_cast<selection_properties_config_t*>(lp);
		SetWindowLongPtr(wnd, DWLP_USER, lp);
	}
	else
		p_data = reinterpret_cast<selection_properties_config_t*>(GetWindowLongPtr(wnd, DWLP_USER));
	return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
}
