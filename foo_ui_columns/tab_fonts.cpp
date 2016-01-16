#include "stdafx.h"

bool tab_appearance_fonts::is_active()
{
	return m_wnd != 0;
}

bool tab_appearance_fonts::get_help_url(pfc::string_base & p_out)
{
	p_out = "http://yuo.be/wiki/columns_ui:config:colours_and_fonts:fonts";
	return true;
}

const char * tab_appearance_fonts::get_name()
{
	return "Fonts";
}

HWND tab_appearance_fonts::create(HWND wnd)
{
	return uCreateDialog(IDD_FONTS_GLOBAL, wnd, g_on_message, (LPARAM)this);
}

void tab_appearance_fonts::apply()
{

}

BOOL CALLBACK tab_appearance_fonts::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		m_wnd = wnd;
		m_wnd_colours_mode = GetDlgItem(wnd, IDC_COLOURS_MODE);
		m_wnd_colours_element = GetDlgItem(wnd, IDC_COLOURS_ELEMENT);

		m_fonts_client_list.g_get_list(m_fonts_client_list);

		ComboBox_AddString(m_wnd_colours_element, L"Common (list items)");
		ComboBox_AddString(m_wnd_colours_element, L"Common (labels)");

		t_size i, count = m_fonts_client_list.get_count();
		for (i = 0; i < count; i++)
			ComboBox_AddString(m_wnd_colours_element, pfc::stringcvt::string_os_from_utf8(m_fonts_client_list[i].m_name));

		ComboBox_SetCurSel(m_wnd_colours_element, 0);
		m_element_ptr = g_fonts_manager_data.m_common_items_entry;

		update_mode_combobox();
		update_font_desc();
		update_change();

		refresh_me(wnd);
	}
	break;
	case WM_DESTROY:
	{
		m_wnd = NULL;
		m_fonts_client_list.remove_all();
		m_element_ptr.release();
		m_element_api.release();
	}
	break;
	case WM_COMMAND:
		switch (wp)
		{
		case IDC_CHANGE_FONT:
		{
			LOGFONT lf;
			get_font(lf);
			if (font_picker(lf, wnd))
			{
				m_element_ptr->font = lf;
				update_font_desc();
				on_font_changed();
			}
		}
		break;
		case IDC_COLOURS_MODE | (CBN_SELCHANGE << 16) :
		{
			int idx = ComboBox_GetCurSel((HWND)lp);
			m_element_ptr->font_mode = (cui::fonts::font_mode_t)ComboBox_GetItemData((HWND)lp, idx);;
			update_font_desc();
			update_change();
			on_font_changed();
		}
													  break;
		case IDC_COLOURS_ELEMENT | (CBN_SELCHANGE << 16) :
		{
			int idx = ComboBox_GetCurSel((HWND)lp);
			m_element_api.release();
			if (idx != -1)
			{
				if (idx == 0)
					m_element_ptr = g_fonts_manager_data.m_common_items_entry;
				else if (idx == 1)
					m_element_ptr = g_fonts_manager_data.m_common_labels_entry;
				else if (idx >= 2)
				{
					m_element_api = m_fonts_client_list[idx - 2].m_ptr;
					g_fonts_manager_data.find_by_guid(m_fonts_client_list[idx - 2].m_guid, m_element_ptr);
				}
			}
			update_mode_combobox();
			update_font_desc();
			update_change();
		}
														 return 0;
														 /*case IDC_IMPORT:
														 g_import_fonts_to_unified();
														 break;*/
		}
		break;
	}
	return 0;
}

void tab_appearance_fonts::on_font_changed()
{
	if (m_element_api.is_valid())
		m_element_api->on_font_changed();
	else
	{
		t_size index_element = ComboBox_GetCurSel(m_wnd_colours_element);
		if (index_element <= 1)
		{
			g_fonts_manager_data.g_on_common_font_changed(1 << index_element);
			t_size i, count = m_fonts_client_list.get_count();
			for (i = 0; i < count; i++)
			{
				fonts_manager_data::entry_ptr_t p_data;
				g_fonts_manager_data.find_by_guid(m_fonts_client_list[i].m_guid, p_data);
				if (index_element == 0 && p_data->font_mode == cui::fonts::font_mode_common_items)
				{
					m_fonts_client_list[i].m_ptr->on_font_changed();
				}
				else if (index_element == 1 && p_data->font_mode == cui::fonts::font_mode_common_labels)
					m_fonts_client_list[i].m_ptr->on_font_changed();
			}
		}
	}
}

void tab_appearance_fonts::update_font_desc()
{
	LOGFONT lf;
	get_font(lf);
	uSetWindowText(GetDlgItem(m_wnd, IDC_FONT_DESC), string_font_desc(lf));
}

void tab_appearance_fonts::update_change()
{
	EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_FONT), m_element_ptr->font_mode == cui::fonts::font_mode_custom);
}

void tab_appearance_fonts::get_font(LOGFONT & lf)
{
	t_size index_element = ComboBox_GetCurSel(m_wnd_colours_element);
	if (index_element <= 1)
		static_api_ptr_t<cui::fonts::manager>()->get_font(cui::fonts::font_type_t(index_element), lf);
	else
		static_api_ptr_t<cui::fonts::manager>()->get_font(m_element_api->get_client_guid(), lf);
}

void tab_appearance_fonts::update_mode_combobox()
{
	ComboBox_ResetContent(m_wnd_colours_mode);
	t_size index;
	t_size index_element = ComboBox_GetCurSel(m_wnd_colours_element);
	if (index_element <= 1)
	{
		index = ComboBox_AddString(m_wnd_colours_mode, L"System");
		ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_system);
	}
	else
	{
		index = ComboBox_AddString(m_wnd_colours_mode, L"Common (list items)");
		ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_common_items);
		index = ComboBox_AddString(m_wnd_colours_mode, L"Common (labels)");
		ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_common_labels);
	}
	index = ComboBox_AddString(m_wnd_colours_mode, L"Custom");
	ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_custom);

	ComboBox_SetCurSel(m_wnd_colours_mode, win32_helpers::combobox_find_item_by_data(m_wnd_colours_mode, m_element_ptr->font_mode));
}

BOOL CALLBACK tab_appearance_fonts::g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	tab_appearance_fonts * p_data = NULL;
	if (msg == WM_INITDIALOG)
	{
		p_data = reinterpret_cast<tab_appearance_fonts*>(lp);
		SetWindowLongPtr(wnd, DWLP_USER, lp);
	}
	else
		p_data = reinterpret_cast<tab_appearance_fonts*>(GetWindowLongPtr(wnd, DWLP_USER));
	return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
}

void tab_appearance_fonts::refresh_me(HWND wnd)
{
	initialising = true;
	initialising = false;
}

tab_appearance_fonts::tab_appearance_fonts() : initialising(false), m_wnd_colours_mode(NULL), m_wnd_colours_element(NULL), m_wnd(NULL)
{

}
