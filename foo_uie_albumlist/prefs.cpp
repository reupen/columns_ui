#include "stdafx.h"

create_guid g_guid_preferences_album_list_panel(0x53c89e50, 0x685d, 0x8ed1, 0x43, 0x25, 0x6b, 0xe8, 0x0f, 0x1b, 0xe7, 0x1f);

struct edit_view_param
{
	unsigned idx;
	string8 name, value;
	bool b_new;
};

static BOOL CALLBACK EditViewProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		uSetWindowLong(wnd, DWL_USER, lp);
		{
			edit_view_param * ptr = reinterpret_cast<edit_view_param*>(lp);
			uSetDlgItemText(wnd, IDC_NAME, ptr->name);
			uSetDlgItemText(wnd, IDC_VALUE, ptr->value);
		}
		break;
	case WM_COMMAND:
		switch (wp)
		{
		case IDCANCEL:
			EndDialog(wnd, 0);
			break;
		case IDOK:
		{
			edit_view_param * ptr = reinterpret_cast<edit_view_param*>(uGetWindowLong(wnd, DWL_USER));
			{
				string8 temp;
				uGetDlgItemText(wnd, IDC_NAME, temp);
				if (temp.is_empty())
				{
					uMessageBox(wnd, "Please enter a valid name.", 0, 0);
					break;
				}
				unsigned idx_find = cfg_view_list.find_item(temp);
				if (idx_find != -1 && (ptr->b_new || ((idx_find != ptr->idx) && (idx_find != -1))))
				{
					uMessageBox(wnd, "View of this name already exists. Please enter another one.", 0, 0);
					break;
				}
				ptr->name = temp;
			}
			uGetDlgItemText(wnd, IDC_VALUE, ptr->value);
			EndDialog(wnd, 1);

		}
		break;
		}
		break;
	}
	return FALSE;
}

static bool run_edit_view(edit_view_param & param, HWND parent)
{
	return uDialogBox(IDD_EDIT_VIEW, parent, EditViewProc, reinterpret_cast<LPARAM>(&param)) != 0;
}

bool colour_picker(HWND wnd, cfg_int & out, COLORREF custom = 0)
{
	bool rv = false;
	COLORREF COLOR = out;
	COLORREF COLORS[16] = { custom,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (uChooseColor(&COLOR, wnd, &COLORS[0]))
	{
		out = COLOR;
		rv = true;
	}
	return rv;
}

cfg_int cfg_child(create_guid(0x637c25b6, 0x9166, 0xd8df, 0xae, 0x7a, 0x39, 0x75, 0x78, 0x08, 0xfa, 0xf0), 0);

BOOL font_picker(LOGFONT & p_font,HWND parent)
{
	CHOOSEFONT cf;
	memset(&cf,0,sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner=parent;
	cf.lpLogFont=&p_font;
	cf.Flags=CF_SCREENFONTS|CF_FORCEFONTEXIST|CF_INITTOLOGFONTSTRUCT;
	cf.nFontType=SCREEN_FONTTYPE;
	BOOL rv = ChooseFont(&cf);
	return rv;
}

class string_font_desc : public std::basic_string<TCHAR>
{
public:
	operator const TCHAR * () const {return data();}
	string_font_desc(const LOGFONT & lf)
	{
		reserve(64);
		HDC dc = GetDC(0);		
		unsigned pt = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
		ReleaseDC(0, dc);

		append(lf.lfFaceName, wcslen_max(lf.lfFaceName, tabsize(lf.lfFaceName)));
		append(_T(" "));
		append(pfc::stringcvt::string_os_from_utf8(pfc::format_int(pt)));
		append(_T("pt"));
		if (lf.lfWeight == FW_BOLD)
			append(_T(" Bold"));
		if (lf.lfItalic)
			append(_T(" Itallic"));
	}
};


tab_general g_config_general;

tab_advanced g_config_advanced;

static preferences_tab * g_tabs[] = 
{
	&g_config_general,
	&g_config_advanced,
};


HWND config_albumlist::child = 0;

static preferences_page_factory_t<config_albumlist> foo3;

bool tab_advanced::initialised = false;

void tab_general::refresh_views()
{
	{
		HWND list = uGetDlgItem(m_wnd, IDC_VIEWS);
		SendMessage(list, LB_RESETCONTENT, 0, 0);
		unsigned n, m = cfg_view_list.get_count();
		string8_fastalloc temp;
		for (n = 0; n<m; n++)
		{
			cfg_view_list.format_display(n, temp);
			uSendMessageText(list, LB_ADDSTRING, 0, temp);
		}
	}
}

BOOL tab_general::g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	tab_general * p_data = NULL;
	if (msg == WM_INITDIALOG)
	{
		p_data = reinterpret_cast<tab_general*>(lp);
		SetWindowLongPtr(wnd, DWL_USER, lp);
	}
	else
		p_data = reinterpret_cast<tab_general*>(GetWindowLongPtr(wnd, DWL_USER));
	return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
}

BOOL tab_general::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{

	case WM_INITDIALOG:
		m_wnd = wnd;

		refresh_views();

		//history_sort.add_item(cfg_sort_order);
		//history_sort.setup_dropdown(GetDlgItem(wnd,IDC_SORT_SPEC));

		//uSetDlgItemText(wnd,IDC_SORT_ORDER,cfg_sort_order);
		uSendDlgItemMessage(wnd, IDC_SHOW_NUMBERS, BM_SETCHECK, cfg_show_numbers, 0);
		uSendDlgItemMessage(wnd, IDC_SHOW_NUMBERS2, BM_SETCHECK, cfg_show_numbers2, 0);

		{
			HWND list = uGetDlgItem(wnd, IDC_DBLCLK);
			uSendMessageText(list, CB_ADDSTRING, 0, "Expand/collapse (default)");
			uSendMessageText(list, CB_ADDSTRING, 0, "Send to playlist");
			uSendMessageText(list, CB_ADDSTRING, 0, "Add to playlist");
			uSendMessageText(list, CB_ADDSTRING, 0, "Send to new playlist");
			uSendMessageText(list, CB_ADDSTRING, 0, "Send to autosend playlist");
			uSendMessage(list, CB_SETCURSEL, cfg_dblclk, 0);

			list = uGetDlgItem(wnd, IDC_MIDDLE);
			uSendMessageText(list, CB_ADDSTRING, 0, "None");
			uSendMessageText(list, CB_ADDSTRING, 0, "Send to playlist");
			uSendMessageText(list, CB_ADDSTRING, 0, "Add to playlist");
			uSendMessageText(list, CB_ADDSTRING, 0, "Send to new playlist");
			uSendMessageText(list, CB_ADDSTRING, 0, "Send to autosend playlist");
			uSendMessage(list, CB_SETCURSEL, cfg_middle, 0);
		}


		//			uSendDlgItemMessage(wnd,IDC_AUTOPLAY,BM_SETCHECK,cfg_autoplay,0);			
		//uSendDlgItemMessage(wnd,IDC_USE_CUSTOMSORT,BM_SETCHECK,cfg_sorttree,0);

		//EnableWindow(uGetDlgItem(wnd,IDC_SORT_ORDER),cfg_sorttree);

		//			uSetDlgItemText(wnd,IDC_RESULTS_SORT,cfg_results_sort_spec);
		//			uSetDlgItemText(wnd,IDC_HIERARCHYNEW,cfg_hierarchy);
		uSetDlgItemText(wnd, IDC_PLAYLIST_NAME, cfg_playlist_name);

		uSendDlgItemMessage(wnd, IDC_AUTO_SEND, BM_SETCHECK, cfg_autosend, 0);

		//SetDlgItemInt(wnd,IDC_HEIGHT,cfg_height, FALSE);
		//uSendDlgItemMessage(wnd,IDC_SPIN,UDM_SETRANGE32,0,999);
		m_initialised = true;

		break;
	case WM_COMMAND:
		switch (wp)
		{
			//			case IDC_USE_CUSTOMSORT:
			//				cfg_sorttree = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
			//				EnableWindow(uGetDlgItem(wnd,IDC_SORT_ORDER),cfg_sorttree);
			//				break;
		case IDC_MIDDLE | (CBN_SELCHANGE << 16) :
			cfg_middle = uSendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
			break;
		case IDC_DBLCLK | (CBN_SELCHANGE << 16) :
			cfg_dblclk = uSendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
			break;
		case IDC_AUTO_SEND:
			cfg_autosend = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			break;
			//			case (CBN_KILLFOCUS<<16)|IDC_SORT_ORDER:
			//				cfg_sort_order = string_utf8_from_window((HWND)lp);
			//				break;
			//			case IDC_TAGZHELP:
			//				standard_commands::main_titleformat_help();
			//				break;
		case (EN_KILLFOCUS << 16) | IDC_PLAYLIST_NAME:
			cfg_playlist_name = string_utf8_from_window((HWND)lp);
			break;
		case IDC_VIEWS | (LBN_DBLCLK << 16) :
		{
			HWND list = (HWND)lp;
			unsigned idx = uSendMessage(list, LB_GETCURSEL, 0, 0);
			if (idx != LB_ERR)
			{
				edit_view_param p;
				p.b_new = false;
				p.idx = idx;
				p.name = cfg_view_list.get_name(idx);
				p.value = cfg_view_list.get_value(idx);
				edit_view_param pbefore = p;
				if (run_edit_view(p, wnd))
				{
					string8 temp;
					if (idx < cfg_view_list.get_count()) //modal message loop
					{
						cfg_view_list.modify_item(idx, p.name, p.value);
						cfg_view_list.format_display(idx, temp);
						uSendMessage(list, LB_DELETESTRING, idx, 0);
						uSendMessageText(list, LB_INSERTSTRING, idx, temp);
						uSendMessageText(list, LB_SETCURSEL, idx, 0);
						album_list_window::g_on_view_script_change(pbefore.name, p.name);
					}
				}
			}
		}
											break;
		case IDC_VIEW_UP:
		{
			HWND list = uGetDlgItem(wnd, IDC_VIEWS);
			unsigned idx = uSendMessage(list, LB_GETCURSEL, 0, 0);
			if (idx != LB_ERR && idx>0)
			{
				uSendMessage(list, LB_DELETESTRING, idx, 0);
				cfg_view_list.swap(idx, idx - 1);
				string8 temp;
				cfg_view_list.format_display(idx - 1, temp);
				uSendMessageText(list, LB_INSERTSTRING, idx - 1, temp);
				uSendMessage(list, LB_SETCURSEL, idx - 1, 0);
			}
		}
		break;
		case IDC_VIEW_DOWN:
		{
			HWND list = uGetDlgItem(wnd, IDC_VIEWS);
			unsigned idx = uSendMessage(list, LB_GETCURSEL, 0, 0);
			if (idx != LB_ERR && idx + 1<cfg_view_list.get_count())
			{
				uSendMessage(list, LB_DELETESTRING, idx, 0);
				cfg_view_list.swap(idx, idx + 1);
				string8 temp;
				cfg_view_list.format_display(idx + 1, temp);
				uSendMessageText(list, LB_INSERTSTRING, idx + 1, temp);
				uSendMessage(list, LB_SETCURSEL, idx + 1, 0);
			}
		}
		break;
		case IDC_VIEW_DELETE:
		{
			HWND list = uGetDlgItem(wnd, IDC_VIEWS);
			unsigned idx = uSendMessage(list, LB_GETCURSEL, 0, 0);
			if (idx != LB_ERR)
			{
				cfg_view_list.remove_item(idx);
				uSendDlgItemMessage(wnd, IDC_VIEWS, LB_DELETESTRING, idx, 0);
			}
		}
		break;
		case IDC_VIEW_NEW:
		{
			edit_view_param p;
			p.b_new = true;
			p.idx = -1;
			if (run_edit_view(p, wnd))
			{
				HWND list = uGetDlgItem(wnd, IDC_VIEWS);
				unsigned n = cfg_view_list.add_item(p.name, p.value);
				string8 temp;
				cfg_view_list.format_display(n, temp);
				uSendMessageText(list, LB_ADDSTRING, 0, temp);
				uSendMessage(list, LB_SETCURSEL, n, 0);
			}
		}
		break;
		case IDC_VIEW_RESET:
		{
			cfg_view_list.reset();
			HWND list = uGetDlgItem(wnd, IDC_VIEWS);
			uSendMessage(list, LB_RESETCONTENT, 0, 0);
			unsigned n, m = cfg_view_list.get_count();
			string8_fastalloc temp;
			for (n = 0; n<m; n++)
			{
				cfg_view_list.format_display(n, temp);
				uSendMessageText(list, LB_ADDSTRING, 0, temp);
			}
		}
		break;
		}
		break;
	case WM_DESTROY:
		//history_sort.add_item(cfg_sort_order);
		m_initialised = false;
		m_wnd = NULL;
		break;
	}
	return 0;
}

class font_client_album_list : public cui::fonts::client
{
public:
	virtual const GUID & get_client_guid() const
	{
		return g_guid_album_list_font;
	}
	virtual void get_name(pfc::string_base & p_out) const
	{
		p_out = "Album List";
	}

	virtual cui::fonts::font_type_t get_default_font_type() const
	{
		return cui::fonts::font_type_items;
	}

	virtual void on_font_changed() const
	{
		album_list_window::g_update_all_fonts();

	}
};

font_client_album_list::factory<font_client_album_list> g_font_client_album_list;

class appearance_client_filter_impl : public cui::colours::client
{
public:
	virtual const GUID & get_client_guid() const { return g_guid_album_list_colours; };
	virtual void get_name(pfc::string_base & p_out) const { p_out = "Album List"; };

	virtual t_size get_supported_colours() const { return cui::colours::colour_flag_text | cui::colours::colour_flag_background | cui::colours::colour_flag_active_item_frame; }; //bit-mask
	virtual t_size get_supported_bools() const { return 0; }; //bit-mask
	virtual bool get_themes_supported() const { return true; };

	virtual void on_colour_changed(t_size mask) const
	{
		album_list_window::update_all_colours();
	};
	virtual void on_bool_changed(t_size mask) const {};
};

namespace {
	cui::colours::client::factory<appearance_client_filter_impl> g_appearance_client_impl;
};

BOOL tab_advanced::ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{

	case WM_INITDIALOG:

		uSendDlgItemMessage(wnd, IDC_SHOW_NUMBERS, BM_SETCHECK, cfg_show_numbers, 0);
		uSendDlgItemMessage(wnd, IDC_SHOW_NUMBERS2, BM_SETCHECK, cfg_show_numbers2, 0);

		{
			HWND list = uGetDlgItem(wnd, IDC_FRAME);

			uSendMessageText(list, CB_ADDSTRING, 0, "None");
			uSendMessageText(list, CB_ADDSTRING, 0, "Sunken");
			uSendMessageText(list, CB_ADDSTRING, 0, "Grey");

			uSendMessage(list, CB_SETCURSEL, cfg_frame, 0);
		}



		//uSendDlgItemMessage(wnd,IDC_USE_CUSTOM_COLOURS,BM_SETCHECK,cfg_use_custom_colours,0);
		uSendDlgItemMessage(wnd, IDC_KEYB, BM_SETCHECK, cfg_keyb, 0);
		uSendDlgItemMessage(wnd, IDC_POPULATE, BM_SETCHECK, cfg_populate, 0);
		uSendDlgItemMessage(wnd, IDC_HSCROLL, BM_SETCHECK, cfg_hscroll, 0);
		uSendDlgItemMessage(wnd, IDC_THEMED, BM_SETCHECK, cfg_themed, 0);
		uSendDlgItemMessage(wnd, IDC_SHOW_ROOT, BM_SETCHECK, cfg_show_root, 0);
		uSendDlgItemMessage(wnd, IDC_AUTOPLAY, BM_SETCHECK, cfg_autoplay, 0);
		uSendDlgItemMessage(wnd, IDC_ADD_ITEMS_USE_CORE_SORT, BM_SETCHECK, cfg_add_items_use_core_sort, 0);
		uSendDlgItemMessage(wnd, IDC_ADD_ITEMS_SELECT, BM_SETCHECK, cfg_add_items_select, 0);
		uSendDlgItemMessage(wnd, IDC_AUTOCOLLAPSE, BM_SETCHECK, cfg_picmixer, 0);

		uSendDlgItemMessage(wnd, IDC_USE_INDENT, BM_SETCHECK, cfg_use_custom_indent, 0);

		{
			HWND wnd_indent = GetDlgItem(wnd, IDC_INDENT);

			EnableWindow(wnd_indent, cfg_use_custom_indent);
			EnableWindow(GetDlgItem(wnd, IDC_INDENT_SPIN), cfg_use_custom_indent);
			if (cfg_use_custom_indent)
				SetDlgItemInt(wnd, IDC_INDENT, cfg_indent, TRUE);
			else
				uSendMessageText(wnd_indent, WM_SETTEXT, 0, "");

			uSendDlgItemMessage(wnd, IDC_INDENT_SPIN, UDM_SETRANGE32, 0, 999);
		}

		{
			uSendDlgItemMessage(wnd, IDC_USE_ITEM_HEIGHT, BM_SETCHECK, cfg_custom_item_height, 0);

			HWND wnd_indent = GetDlgItem(wnd, IDC_ITEM_HEIGHT);

			EnableWindow(wnd_indent, cfg_custom_item_height);
			EnableWindow(GetDlgItem(wnd, IDC_ITEM_HEIGHT_SPIN), cfg_custom_item_height);

			if (cfg_custom_item_height)
				SetDlgItemInt(wnd, IDC_ITEM_HEIGHT, cfg_item_height, TRUE);
			else
				uSendMessageText(wnd_indent, WM_SETTEXT, 0, "");

			uSendDlgItemMessage(wnd, IDC_ITEM_HEIGHT_SPIN, UDM_SETRANGE32, -99, 99);
		}

		//SetDlgItemInt(wnd,IDC_HEIGHT,cfg_height, FALSE);
		//uSendDlgItemMessage(wnd,IDC_SPIN,UDM_SETRANGE32,0,999);
		initialised = true;

		break;
	case WM_COMMAND:
		switch (wp)
		{
		case IDC_AUTOCOLLAPSE:
			cfg_picmixer = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			break;
		case IDC_ADD_ITEMS_USE_CORE_SORT:
			cfg_add_items_use_core_sort = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			break;
		case IDC_ADD_ITEMS_SELECT:
			cfg_add_items_select = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			break;
		case IDC_POPULATE:
			cfg_populate = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			break;
			//case IDC_USE_CUSTOMSORT:
			//cfg_sorttree = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
			//EnableWindow(uGetDlgItem(wnd,IDC_SORT_ORDER),cfg_sorttree);
			//break;
		case IDC_SHOW_NUMBERS:
			cfg_show_numbers = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			album_list_window::g_update_all_labels();
			break;
		case IDC_AUTO_SEND:
			cfg_autosend = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			break;
		case IDC_HSCROLL:
			cfg_hscroll = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			album_list_window::g_update_all_showhscroll();
			break;
		case IDC_THEMED:
			cfg_themed = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0;
			break;
		case IDC_SHOW_ROOT:
			cfg_show_root = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			album_list_window::g_refresh_all();
			break;
		case IDC_AUTOPLAY:
			cfg_autoplay = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			break;
		case IDC_KEYB:
			cfg_keyb = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			break;
		case IDC_SHOW_NUMBERS2:
			cfg_show_numbers2 = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			album_list_window::g_update_all_labels();
			break;
		case (CBN_SELCHANGE << 16) | IDC_FRAME:
		{
			cfg_frame = uSendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
			album_list_window::update_all_window_frames();
		}
		break;
		/*case (EN_CHANGE<<16)|IDC_HEIGHT:
		{
		if (initialised)
		{
		BOOL result;
		unsigned new_height = GetDlgItemInt(wnd, IDC_HEIGHT, &result, FALSE);
		if (result)
		{
		if (new_height > 999) new_height = 999;
		cfg_height = new_height;
		album_list_window::update_all_heights();
		}
		}
		}
		break;*/
		case (EN_CHANGE << 16) | IDC_ITEM_HEIGHT:
		{
			if (initialised && cfg_custom_item_height)
			{
				BOOL result;
				int new_height = GetDlgItemInt(wnd, IDC_ITEM_HEIGHT, &result, TRUE);
				if (result)
				{
					if (new_height > 99) new_height = 99;
					if (new_height < -99) new_height = -99;
					cfg_item_height = new_height;
					album_list_window::update_all_item_heights();
				}
			}
		}
		break;
		case IDC_USE_ITEM_HEIGHT:
		{
			cfg_custom_item_height = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			HWND wnd_indent = GetDlgItem(wnd, IDC_ITEM_HEIGHT);

			EnableWindow(wnd_indent, cfg_custom_item_height);
			EnableWindow(GetDlgItem(wnd, IDC_ITEM_HEIGHT_SPIN), cfg_custom_item_height);

			if (cfg_custom_item_height)
				SetDlgItemInt(wnd, IDC_ITEM_HEIGHT, cfg_item_height, TRUE);
			else
				uSendMessageText(wnd_indent, WM_SETTEXT, 0, "");

			album_list_window::update_all_item_heights();
		}
		break;
		case IDC_USE_INDENT:
		{
			cfg_use_custom_indent = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			HWND wnd_indent = GetDlgItem(wnd, IDC_INDENT);

			EnableWindow(wnd_indent, cfg_use_custom_indent);
			EnableWindow(GetDlgItem(wnd, IDC_INDENT_SPIN), cfg_use_custom_indent);
			if (cfg_use_custom_indent)
				SetDlgItemInt(wnd, IDC_INDENT, cfg_indent, TRUE);
			else
				uSendMessageText(wnd_indent, WM_SETTEXT, 0, "");

			album_list_window::update_all_indents();
		}
		break;
		case (EN_CHANGE << 16) | IDC_INDENT:
		{
			if (initialised && cfg_use_custom_indent)
			{
				BOOL result;
				unsigned new_height = GetDlgItemInt(wnd, IDC_INDENT, &result, TRUE);
				if (result)
				{
					if (new_height > 999) new_height = 999;
					cfg_indent = new_height;
					album_list_window::update_all_indents();
				}
			}
		}
		break;
		}
		break;
	case WM_DESTROY:
		initialised = false;
		break;
	}
	return 0;
}

void config_albumlist::make_child(HWND wnd)
{
	if (child)
	{
		ShowWindow(child, SW_HIDE);
		DestroyWindow(child);
		child = 0;
	}

	HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

	RECT tab;

	GetWindowRect(wnd_tab, &tab);
	MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);

	TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

	unsigned count = tabsize(g_tabs);
	if ((unsigned)cfg_child >= count) cfg_child = 0;

	if ((unsigned)cfg_child < count && cfg_child >= 0)
	{
		child = g_tabs[cfg_child]->create(wnd);
	}

	//SetWindowPos(child,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	if (child)
	{
		{
			EnableThemeDialogTexture(child, ETDT_ENABLETAB);
		}
	}

	SetWindowPos(child, 0, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
	SetWindowPos(wnd_tab, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	ShowWindow(child, SW_SHOWNORMAL);
	//UpdateWindow(child);
}

BOOL config_albumlist::ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
		unsigned n, count = tabsize(g_tabs);
		for (n = 0; n<count; n++)
		{
			uTabCtrl_InsertItemText(wnd_tab, n, g_tabs[n]->get_name());
		}
		TabCtrl_SetCurSel(wnd_tab, cfg_child);
		make_child(wnd);
	}
	break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lp)->idFrom)
		{
		case IDC_TAB1:
			switch (((LPNMHDR)lp)->code)
			{
			case TCN_SELCHANGE:
			{
				cfg_child = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
				make_child(wnd);
			}
			break;
			}
			break;
		}
		break;
	case WM_PARENTNOTIFY:
		switch (wp)
		{
		case WM_DESTROY:
		{
			if (child && (HWND)lp == child) child = 0;
		}
		break;
		}
		break;
	}
	return 0;
}
