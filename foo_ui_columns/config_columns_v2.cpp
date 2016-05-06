#include "stdafx.h"
#include "NG Playlist/ng_playlist.h"

extern cfg_int g_cur_tab;
extern cfg_int g_last_colour;

enum {MSG_COLUMN_NAME_CHANGED = WM_USER+2,MSG_SELECTION_CHANGED};
struct column_times
{
	service_ptr_t<titleformat_object> to_display;
	service_ptr_t<titleformat_object> to_colour;
	double time_display_compile;
	double time_colour_compile;
	double time_display;
	double time_colour;
};

class column_tab : public pfc::refcounted_object_root
{
public:
	typedef column_tab self_t;
	typedef pfc::refcounted_object_ptr_t<self_t> ptr;
	virtual HWND create(HWND wnd)=0;
	//virtual void destroy(HWND wnd)=0;
	//virtual const char * get_name()=0;
	virtual void set_column(const column_t::ptr & column)=0;
	virtual void get_column(column_t::ptr & p_out)=0;

};

class edit_column_window_options : public column_tab
{
public:
	virtual void get_column(column_t::ptr & p_out){p_out=m_column;};
	typedef edit_column_window_options self_t;
	virtual HWND create(HWND wnd)
	{
		return uCreateDialog(IDD_COLUMN_OPTIONS, wnd, g_on_message, (LPARAM)this);
	}
	//virtual const char * get_name()=0;
	edit_column_window_options(const column_t::ptr & column) : initialising(false), editproc(NULL), m_column(column),m_wnd(NULL) {};

	bool initialising;
	WNDPROC editproc;
	HWND m_wnd;

	column_t::ptr m_column;

	void set_detail_enabled(HWND wnd, BOOL show)
	{
		if (show == FALSE)
		{
			pfc::vartoggle_t<bool>(initialising, true);
			
			uSendDlgItemMessageText(wnd, IDC_NAME, WM_SETTEXT, 0, "");
			uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_STRING, WM_SETTEXT, 0, "");
			uSendDlgItemMessageText(wnd, IDC_EDITFIELD, WM_SETTEXT, 0, "");
			SetDlgItemInt(wnd, IDC_WIDTH, 0, false);
			SetDlgItemInt(wnd, IDC_PARTS, 0, false);
			SendDlgItemMessage(wnd, IDC_SHOW_COLUMN, BM_SETCHECK, 0, 0);
			SendDlgItemMessage(wnd, IDC_ALIGNMENT, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_SETCURSEL, 0, 0);
		}

		EnableWindow(GetDlgItem(wnd, IDC_STRING), show);
		EnableWindow(GetDlgItem(wnd, IDC_NAME), show);
		EnableWindow(GetDlgItem(wnd, IDC_WIDTH), show);
		EnableWindow(GetDlgItem(wnd, IDC_PARTS), show);
		EnableWindow(GetDlgItem(wnd, IDC_SHOW_COLUMN), show);
		EnableWindow(GetDlgItem(wnd, IDC_ALIGNMENT), show);
		EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), show && m_column.is_valid() && m_column->filter_type != FILTER_NONE);
		EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_TYPE), show);
		EnableWindow(GetDlgItem(wnd, IDC_EDITFIELD), show);
	}	

	void refresh_me(HWND wnd, bool init = false)
	{
		initialising = true;	

		if (m_column.is_valid())
		{

			uSendDlgItemMessageText(wnd, IDC_NAME, WM_SETTEXT, 0, m_column->name);
			uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_STRING, WM_SETTEXT, 0, m_column->filter);
			uSendDlgItemMessageText(wnd, IDC_EDITFIELD, WM_SETTEXT, 0, m_column->edit_field);

			SendDlgItemMessage(wnd,IDC_SHOW_COLUMN,BM_SETCHECK,m_column->show,0);
			SendDlgItemMessage(wnd,IDC_ALIGNMENT,CB_SETCURSEL,(t_size)m_column->align,0);
			SendDlgItemMessage(wnd,IDC_PLAYLIST_FILTER_TYPE,CB_SETCURSEL,(t_size)m_column->filter_type,0);

			SetDlgItemInt(wnd, IDC_WIDTH, m_column->width, false);
			SetDlgItemInt(wnd, IDC_PARTS, m_column->parts, false);
		}

		initialising = false;

		set_detail_enabled(wnd, m_column.is_valid());


	}

	virtual void set_column(const column_t::ptr & column)
	{
		if (m_column.get_ptr() != column.get_ptr())
		{
			m_column = column;
			refresh_me(m_wnd);
		}
	}
	static BOOL CALLBACK g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		self_t * p_data = NULL;
		if (msg == WM_INITDIALOG)
		{
			p_data = reinterpret_cast<self_t*>(lp);
			SetWindowLongPtr(wnd, DWLP_USER, lp);
		}
		else
			p_data = reinterpret_cast<self_t*>(GetWindowLongPtr(wnd, DWLP_USER));
		return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
	}

	BOOL CALLBACK on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				m_wnd =wnd;
				uTCITEM tabs;
				memset(&tabs, 0, sizeof(tabs));

				uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Left");
				uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Centre");
				uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Right");

				uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show on all playlists");
				uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show only on playlists:");
				uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Hide on playlists:");

				SendDlgItemMessage(wnd, IDC_STRING, EM_LIMITTEXT, 0, 0);

				refresh_me(wnd, true);
			}

			break;
		case WM_DESTROY:
			m_wnd = NULL;
			break;


		case WM_COMMAND:
			switch(wp)
			{

			case (CBN_SELCHANGE<<16)|IDC_ALIGNMENT:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->align= ((alignment)SendMessage((HWND)lp,CB_GETCURSEL,0,0));
					}
				}
				break;
			case (CBN_SELCHANGE<<16)|IDC_PLAYLIST_FILTER_TYPE:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->filter_type = ((playlist_filter_type)SendMessage((HWND)lp,CB_GETCURSEL,0,0));
						EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), m_column->filter_type != FILTER_NONE);
					}
				}
				break;
			case IDC_SHOW_COLUMN:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->show = ((SendMessage((HWND)lp,BM_GETCHECK,0,0) !=0 ));
					}
				}
				break;
			case (EN_CHANGE<<16)|IDC_SORT:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->sort_spec = (string_utf8_from_window((HWND)lp));
					}
				}
				break;
			case IDC_PICK_COLOUR:
				colour_code_gen(wnd, IDC_COLOUR, true, false);
				break;			
			case (EN_CHANGE<<16)|IDC_WIDTH:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->width =(GetDlgItemInt(wnd, IDC_WIDTH, 0, false));
					}
				}
				break;
			case (EN_CHANGE<<16)|IDC_PARTS:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->parts = (GetDlgItemInt(wnd, IDC_PARTS, 0, false));
					}
				}
				break;
			case (EN_CHANGE<<16)|IDC_PLAYLIST_FILTER_STRING:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->filter = (string_utf8_from_window((HWND)lp));
					}
				}
				break;
			case (EN_CHANGE<<16)|IDC_EDITFIELD:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->edit_field = (string_utf8_from_window((HWND)lp));
					}
				}
				break;
			case (EN_CHANGE<<16)|IDC_NAME:
				{
					if (!initialising && m_column.is_valid()) 
					{
						pfc::string8 name = string_utf8_from_window((HWND)lp);
						m_column->name = (name);
						SendMessage(GetAncestor(wnd, GA_PARENT), MSG_COLUMN_NAME_CHANGED, NULL, NULL);
					}
				}
				break;
			}
		}
		return 0;
	}

};

class edit_column_window_scripts : public column_tab
{
public:
	typedef edit_column_window_scripts self_t;
	virtual void get_column(column_t::ptr & p_out){p_out=m_column;};
	virtual HWND create(HWND wnd)
	{
		return uCreateDialog(IDD_COLUMN_SCRIPTS, wnd, g_on_message, (LPARAM)this);
	}
	//virtual const char * get_name()=0;
	edit_column_window_scripts(const column_t::ptr & col) : initialising(false), editproc(NULL), m_column(col), m_wnd(NULL) {};

	HWND m_wnd;

	bool initialising;
	WNDPROC editproc;

	column_t::ptr m_column;

	void set_detail_enabled(HWND wnd, BOOL show)
	{
		if (show == FALSE)
		{
			pfc::vartoggle_t<bool>(initialising, true);
			
			uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, "");
			SendDlgItemMessage(wnd, IDC_CUSTOM_SORT, BM_SETCHECK, 0, 0);
			SendDlgItemMessage(wnd, IDC_CUSTOM_COLOUR, BM_SETCHECK, 0, 0);
		}

		EnableWindow(GetDlgItem(wnd, IDC_STRING), show);
		EnableWindow(GetDlgItem(wnd, IDC_CUSTOM_SORT), show);
		EnableWindow(GetDlgItem(wnd, IDC_CUSTOM_COLOUR), show);
	}	
	void refresh_me(HWND wnd, bool init = false)
	{
		initialising = true;	

		if (m_column.is_valid())
		{
			update_string(wnd);
			SendDlgItemMessage(wnd,IDC_CUSTOM_SORT,BM_SETCHECK,m_column->use_custom_sort,0);
			SendDlgItemMessage(wnd,IDC_CUSTOM_COLOUR,BM_SETCHECK,m_column->use_custom_colour,0);
		}

		initialising = false;

		set_detail_enabled(wnd, m_column.is_valid());

	}

	virtual void set_column(const column_t::ptr & column)
	{
		if (m_column.get_ptr() != column.get_ptr())
		{
			m_column = column;
			refresh_me(m_wnd);
		}
	}

	void update_string(HWND wnd, int id = -1)
	{
		initialising = true;	
		if (id == -1) id = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));

		pfc::string8 temp;

		if (id >=0 && id <= 2)
		{

			if (m_column.is_valid())
			{
				if (id == 0) temp = m_column->spec;
				else if (id == 1) temp = m_column->colour_spec;
				else if (id == 2) temp = m_column->sort_spec;
			}
		}
		uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, temp);
		initialising = false;
	}

	void save_string(HWND wnd, LPARAM lp)
	{
		int id = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
		if (id >=0 && id <=2)
		{

			if (!initialising && m_column.is_valid()) 
			{
				if (id == 0) m_column->spec = string_utf8_from_window((HWND)lp);
				else if (id == 1) m_column->colour_spec = string_utf8_from_window((HWND)lp);
				else if (id == 2) m_column->sort_spec = string_utf8_from_window((HWND)lp);
			}
		}


	}

	static LRESULT WINAPI g_EditHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		self_t * p_data = NULL;
		p_data = reinterpret_cast<self_t*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
		return p_data ? p_data->EditHook(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
	}

	LRESULT WINAPI EditHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_CHAR:
			if (!(HIWORD(lp) & KF_REPEAT) && (wp == 1) &&  (GetKeyState(VK_CONTROL) & KF_UP))
			{
				SendMessage(wnd, EM_SETSEL, 0, -1);
				return 0;
			}	
			break;
		}
		return uCallWindowProc(editproc,wnd,msg,wp,lp);
	}

	static BOOL CALLBACK g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		self_t * p_data = NULL;
		if (msg == WM_INITDIALOG)
		{
			p_data = reinterpret_cast<self_t*>(lp);
			SetWindowLongPtr(wnd, DWLP_USER, lp);
		}
		else
			p_data = reinterpret_cast<self_t*>(GetWindowLongPtr(wnd, DWLP_USER));
		return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
	}

	BOOL CALLBACK on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				m_wnd = wnd;
				uTCITEM tabs;
				memset(&tabs, 0, sizeof(tabs));

				HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

				tabs.mask = TCIF_TEXT;
				tabs.pszText = "Display";
				uTabCtrl_InsertItem(wnd_tab, 0, &tabs);
				tabs.pszText = "Style";
				uTabCtrl_InsertItem(wnd_tab, 1, &tabs);
				tabs.pszText = "Sort";
				uTabCtrl_InsertItem(wnd_tab, 2, &tabs);

				TabCtrl_SetCurSel(wnd_tab, g_cur_tab);

				colour_code_gen(wnd, IDC_COLOUR, true, true); 

				SendDlgItemMessage(wnd, IDC_STRING, EM_LIMITTEXT, 0, 0);

				refresh_me(wnd, true);

				SetWindowLongPtr(GetDlgItem(wnd, IDC_STRING),GWLP_USERDATA,(LPARAM)this);
				editproc = (WNDPROC)SetWindowLongPtr(GetDlgItem(wnd, IDC_STRING),GWLP_WNDPROC,(LPARAM)g_EditHook);

				g_editor_font_notify.set(GetDlgItem(wnd, IDC_STRING));
			}

			break;

		case WM_DESTROY:
			{
				m_wnd = NULL;
				g_editor_font_notify.release();
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
						int id = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
						g_cur_tab = id;
						update_string(wnd,id);
					}
					break;
				}
				break;
			}
			break;

		case WM_COMMAND:
			switch(wp)
			{

			case IDC_CUSTOM_SORT:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->use_custom_sort = ((SendMessage((HWND)lp,BM_GETCHECK,0,0) !=0 ));
					}
				}
				break;
			case IDC_CUSTOM_COLOUR:
				{
					if (!initialising && m_column.is_valid()) 
					{
						m_column->use_custom_colour = ((SendMessage((HWND)lp,BM_GETCHECK,0,0) !=0 ));
					}
				}
				break;
			case IDC_TFHELP:
				{
					RECT rc;
					GetWindowRect(GetDlgItem(wnd, IDC_TFHELP), &rc);
					//		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)(&rc), 2);
					HMENU menu = CreatePopupMenu();

					enum {IDM_TFHELP=1, IDM_SHELP=2, IDM_SPEEDTEST, IDM_PREVIEW,  IDM_EDITORFONT};

					uAppendMenu(menu,(MF_STRING),IDM_TFHELP,"Titleformatting &help");
					uAppendMenu(menu,(MF_STRING),IDM_SHELP,"&String help");
					uAppendMenu(menu,(MF_SEPARATOR),0,"");
					uAppendMenu(menu,(MF_STRING),IDM_SPEEDTEST,"Sp&eed test");
					uAppendMenu(menu,(MF_STRING),IDM_PREVIEW,"&Preview script");
					uAppendMenu(menu,(MF_SEPARATOR),0,"");
					uAppendMenu(menu,(MF_STRING),IDM_EDITORFONT,"Change editor &font");


					int cmd = TrackPopupMenu(menu,TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,rc.left,rc.bottom,0,wnd,0);
					DestroyMenu(menu);
					if (cmd == IDM_TFHELP)
					{
						standard_commands::main_titleformat_help();
					}
					else if (cmd == IDM_SHELP)
					{
						uMessageBox(wnd, COLOUR_HELP, "Style string help", 0);
					}
					else if (cmd == IDM_SPEEDTEST)
					{
						speedtest(g_columns, cfg_global != 0, cfg_oldglobal != 0, cfg_playlist_date != 0);
					}
					else if (cmd == IDM_PREVIEW)
					{
						preview_to_console(string_utf8_from_window(wnd, IDC_STRING), cfg_global != 0);
					}
					else if (cmd == IDM_EDITORFONT)
					{
						if (font_picker(wnd, cfg_editor_font))
							g_editor_font_notify.on_change();
					}
				}
				break;
			case IDC_PICK_COLOUR:
				colour_code_gen(wnd, IDC_COLOUR, true, false);
				break;			
			case (EN_CHANGE<<16)|IDC_STRING:
				{
					if (!initialising && m_column.is_valid()) 
						save_string(wnd, lp);
				}
				break;
			}
		}
		return 0;
	}

};

// {0A7A2845-06A4-4c15-B09F-A6EBEE86335D}
const GUID g_guid_cfg_child_column = 
{ 0xa7a2845, 0x6a4, 0x4c15, { 0xb0, 0x9f, 0xa6, 0xeb, 0xee, 0x86, 0x33, 0x5d } };

cfg_uint cfg_child_column(g_guid_cfg_child_column, 0);

static class tab_columns_v3 : public preferences_tab
{
private:
	HWND m_wnd_child;
	HWND m_wnd;
	column_tab::ptr m_child;
	//edit_column_window_options m_tab_options;
	//edit_column_window_scripts m_tab_scripts;
public:
	tab_columns_v3() : initialising(false), m_wnd_child(nullptr), m_wnd(nullptr) {};

	void make_child()
	{
		//HWND wnd_destroy = child;
		if (m_wnd_child)
		{
			ShowWindow(m_wnd_child, SW_HIDE);
			DestroyWindow(m_wnd_child);
			m_wnd_child=NULL;
			m_child.release();
		}

		HWND wnd_tab = GetDlgItem(m_wnd, IDC_TAB1);
		
		RECT tab;
		
		GetWindowRect(wnd_tab,&tab);
		MapWindowPoints(HWND_DESKTOP, m_wnd, (LPPOINT)&tab, 2);
		
		TabCtrl_AdjustRect(wnd_tab,FALSE,&tab);
		
		unsigned count = 2;
		if (cfg_child_column >= count) cfg_child_column = 0;

		if (cfg_child_column < count && cfg_child_column >= 0)
		{
			int item = ListView_GetNextItem(GetDlgItem(m_wnd, IDC_COLUMNS), -1, LVNI_SELECTED);

			column_t::ptr column;
			if (item != -1)
				column = m_columns[item];

			if (cfg_child_column == 0)
				m_child = new edit_column_window_options(column);
			else
				m_child = new edit_column_window_scripts(column);
			m_wnd_child = m_child->create(m_wnd);
		}

		if (m_wnd_child) 
		{
			EnableThemeDialogTexture(m_wnd_child, ETDT_ENABLETAB);
			SetWindowPos(m_wnd_child, HWND_TOP, tab.left, tab.top, tab.right-tab.left, tab.bottom-tab.top, NULL);
			ShowWindow(m_wnd_child, SW_SHOWNORMAL);
		}

		//SetWindowPos(wnd_tab,GetDlgItem(m_wnd, IDC_GROUPBOX),0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
	}

	void refresh_me(HWND wnd, bool init = false)
	{
		initialising = true;	
		HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);

		SendDlgItemMessage(wnd,IDC_COLUMNS,WM_SETREDRAW,false,0);
		int idx =(init ?  cfg_cur_prefs_col : ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED));
		ListView_DeleteAllItems(wnd_lv);

		pfc::string8 temp;

		int i, t = m_columns.get_count();
		for (i = 0; i < t; i++)
		{
			uih::ListView_InsertItemText(wnd_lv, i, 0 ,m_columns[i]->name);
		}

		SendDlgItemMessage(wnd,IDC_COLUMNS,WM_SETREDRAW,true,0);
		initialising = false;

		if (idx>=0 && idx < (int)m_columns.get_count())
		{
			ListView_SetItemState(wnd_lv, idx, LVIS_SELECTED, LVIS_SELECTED);
		}

		RECT rc_lv;
		GetClientRect(wnd_lv, &rc_lv);
		ListView_SetColumnWidth(wnd_lv, 0, RECT_CX(rc_lv));
	}


	static BOOL CALLBACK g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		tab_columns_v3 * p_data = NULL;
		if (msg == WM_INITDIALOG)
		{
			p_data = reinterpret_cast<tab_columns_v3*>(lp);
			SetWindowLongPtr(wnd, DWLP_USER, lp);
		}
		else
			p_data = reinterpret_cast<tab_columns_v3*>(GetWindowLongPtr(wnd, DWLP_USER));
		return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
	}

	BOOL CALLBACK on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{

		switch(msg)
		{
		case WM_INITDIALOG:
			{
				m_wnd = wnd;
				//if (g_main_window && !cfg_nohscroll ) playlist_view::g_save_columns();
				HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
				uih::SetListViewWindowExplorerTheme(wnd_lv);
				ListView_SetExtendedListViewStyleEx(wnd_lv, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
				uih::ListView_InsertColumnText(wnd_lv, 0, L"Column", 50);

				m_columns.set_entries_copy(g_columns);

				refresh_me(wnd, true);

				HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
				uTabCtrl_InsertItemText(wnd_tab, 0, "Options");
				uTabCtrl_InsertItemText(wnd_tab, 1, "Scripts");
				
				TabCtrl_SetCurSel(wnd_tab, cfg_child_column);
				
				make_child();

			}

			break;
		case WM_CONTEXTMENU:
			if (HWND(wp) == GetDlgItem(wnd, IDC_COLUMNS))
			{
				enum {ID_REMOVE=1, ID_UP, ID_DOWN, ID_NEW};
				POINT pt = {GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
				int item = ListView_GetNextItem(GetDlgItem(m_wnd, IDC_COLUMNS), -1, LVNI_SELECTED);
				//if (item != -1 && item >= 0)
				{
					HMENU menu = CreatePopupMenu();
					AppendMenu(menu, MF_STRING, ID_NEW, L"&New");
					if (item != -1)
						AppendMenu(menu, MF_STRING, ID_REMOVE, L"&Remove");
					if (item != -1 && m_columns.get_count()>1)
						AppendMenu(menu, MF_SEPARATOR, NULL, NULL);
					if (item>0)
						AppendMenu(menu, MF_STRING, ID_UP, L"Move &up");
					if (item >=0 && (t_size(item+1)) < m_columns.get_count())
						AppendMenu(menu, MF_STRING, ID_DOWN, L"Move &down");
					int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);
					DestroyMenu(menu);
					if (cmd)
					{
						int & idx = item;
						HWND wnd_lv = HWND(wp);
						if (cmd == ID_NEW)
						{
							column_t::ptr temp = new column_t;
							temp->name = "New Column";
							t_size insert = m_columns.insert_item(temp, idx>=0 && (t_size)idx < m_columns.get_count() ? idx : m_columns.get_count());
							uih::ListView_InsertItemText(wnd_lv, insert, 0, "New Column");
							ListView_SetItemState(wnd_lv, insert, LVIS_SELECTED, LVIS_SELECTED);
							ListView_EnsureVisible(wnd_lv, insert, FALSE);
						}
						else if (idx >= 0 && (t_size)idx < m_columns.get_count()) 
						{
							if (cmd == ID_REMOVE)
							{
								m_columns.remove_by_idx(idx);
								t_size new_count = m_columns.get_count();
								ListView_DeleteItem(wnd_lv, idx);

								if (idx > 0 && (t_size)idx == new_count) idx--;
								if (idx >=0 && (t_size)idx < new_count)
									ListView_SetItemState(wnd_lv, idx, LVIS_SELECTED, LVIS_SELECTED);
								if (new_count == 0)
									SendMessage(wnd, MSG_SELECTION_CHANGED, NULL, NULL);
							
							}
							else if (cmd == ID_UP)
							{
								if (idx > 0 && m_columns.move_up(idx)) 
								{
									uih::ListView_InsertItemText(wnd_lv, idx, 0, m_columns[idx]->name, true);
									uih::ListView_InsertItemText(wnd_lv, idx-1, 0, m_columns[idx-1]->name, true);
									ListView_SetItemState(wnd_lv, idx-1, LVIS_SELECTED, LVIS_SELECTED);
									ListView_EnsureVisible(wnd_lv, idx-1, FALSE);
								}
							}
							else if (cmd == ID_DOWN)
							{
								if ((t_size)(idx +1) < m_columns.get_count() && m_columns.move_down(idx)) 
								{
									uih::ListView_InsertItemText(wnd_lv, idx, 0, m_columns[idx]->name, true);
									uih::ListView_InsertItemText(wnd_lv, idx+1, 0, m_columns[idx+1]->name, true);
									ListView_SetItemState(wnd_lv, idx+1, LVIS_SELECTED, LVIS_SELECTED);
									ListView_EnsureVisible(wnd_lv, idx+1, FALSE);
								}
							}
						}
					}
				}

				return 0;
			}
			break;

		case WM_DESTROY:
			{
				HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
				int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
				if (idx>=0 && idx < (int)m_columns.get_count())
				{
					cfg_cur_prefs_col = idx;
				}

				apply();
				m_columns.remove_all();
				m_wnd = NULL;
				if (m_wnd_child)
				{
					DestroyWindow(m_wnd_child);
					m_wnd_child=NULL;
					m_child.release();
				}
			}
			break;
		case MSG_SELECTION_CHANGED:
			{
				int item = (ListView_GetNextItem(GetDlgItem(m_wnd, IDC_COLUMNS), -1, LVNI_SELECTED));
				m_child->set_column(item != -1 && item >=0 && (t_size)item < m_columns.get_count() ? m_columns[item] : column_t::ptr());
			}
			return 0;
		case MSG_COLUMN_NAME_CHANGED:
			{
				HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
				int item = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
				if (item != -1 && item < m_columns.get_count())
					uih::ListView_InsertItemText(wnd_lv, item, 0, m_columns[item]->name, true);
			}
			return 0;
		case WM_NOTIFY:
			{
				LPNMHDR lpnm = (LPNMHDR)lp;
				switch (lpnm->idFrom)
				{
				case IDC_COLUMNS:
					{
						switch (lpnm->code)
						{
						case LVN_ITEMCHANGED:
							{
								LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lp;
								if (m_child.is_valid())
								{
									//console::formatter() << (int)lpnmlv->iItem;
									if (lpnmlv->iItem != -1 && lpnmlv->iItem >=0 && (t_size)lpnmlv->iItem < m_columns.get_count())
									{
										/*if ((lpnmlv->uNewState & LVIS_SELECTED) && !(lpnmlv->uOldState & LVIS_SELECTED))
										{
											m_child->set_column(m_columns[lpnmlv->iItem]);
										}
										else if (!(lpnmlv->uNewState & LVIS_SELECTED) && (lpnmlv->uOldState & LVIS_SELECTED))
										{
											if (ListView_GetNextItem(GetDlgItem(m_wnd, IDC_COLUMNS), -1, LVNI_SELECTED) == -1)
												m_child->set_column(NULL);
										}*/
										if ((lpnmlv->uNewState & LVIS_SELECTED) != (lpnmlv->uOldState & LVIS_SELECTED))
											PostMessage(wnd, MSG_SELECTION_CHANGED, NULL, NULL);
									}
								}
							}
							return 0;
						};
					}
					break;
				case IDC_TAB1:
					switch (((LPNMHDR)lp)->code)
					{
					case TCN_SELCHANGE:
						{
							cfg_child_column = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
							make_child();
						}
						break;
					}
					break;
				}
				break;
			}
			break;
		case WM_PARENTNOTIFY:
			switch(wp)
			{
			case WM_DESTROY:
				{
					if (m_wnd_child && (HWND)lp == m_wnd_child)
					{
						m_wnd_child = NULL;
						//m_child.release();
					}
				}
				break;	
			}
			break;
		case WM_COMMAND:
			switch(wp)
			{
			case IDC_APPLY:
				apply();
				break;
			case IDC_UP:
				{
					HWND wnd_lv= GetDlgItem(wnd, IDC_COLUMNS);
					int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
					if (idx >= 0 && idx>0 && m_columns.move_up(idx)) 
					{
						uih::ListView_InsertItemText(wnd_lv, idx, 0, m_columns[idx]->name, true);
						uih::ListView_InsertItemText(wnd_lv, idx-1, 0, m_columns[idx-1]->name, true);
						ListView_SetItemState(wnd_lv, idx-1, LVIS_SELECTED, LVIS_SELECTED);
						ListView_EnsureVisible(wnd_lv, idx-1, FALSE);
					}
					//apply();
				}
				break;
			case IDC_DOWN:
				{
					HWND wnd_lv= GetDlgItem(wnd, IDC_COLUMNS);
					int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
					if (idx >= 0 && (t_size(idx+1)) < m_columns.get_count() && m_columns.move_down(idx)) 
					{
						uih::ListView_InsertItemText(wnd_lv, idx, 0, m_columns[idx]->name, true);
						uih::ListView_InsertItemText(wnd_lv, idx+1, 0, m_columns[idx+1]->name, true);
						ListView_SetItemState(wnd_lv, idx+1, LVIS_SELECTED, LVIS_SELECTED);
						ListView_EnsureVisible(wnd_lv, idx+1, FALSE);
					}
					//apply();
				}
				break;
			case IDC_NEW:
				{
					HWND wnd_lv= GetDlgItem(wnd, IDC_COLUMNS);
					int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
					//if (true) 
					{
						column_t::ptr temp = new column_t;
						temp->name = "New Column";
						t_size insert = m_columns.insert_item(temp, idx>=0 && (t_size)idx < m_columns.get_count() ? idx : m_columns.get_count());
						uih::ListView_InsertItemText(wnd_lv, insert, 0, "New Column");
						ListView_SetItemState(wnd_lv, insert, LVIS_SELECTED, LVIS_SELECTED);
						ListView_EnsureVisible(wnd_lv, insert, FALSE);
					}
					//apply();
				}
				break;
#if 0
			case IDC_REMOVE:
				{
					HWND wnd_lv= GetDlgItem(wnd, IDC_COLUMNS);
					int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
					if (idx >= 0) 
					{
						m_columns.remove_by_idx(idx);
						ListView_DeleteItem(wnd_lv, idx);

						if ((unsigned)idx == m_columns.get_count()) idx--;
						if (idx >=0)
							ListView_SetItemState(wnd_lv, idx, LVIS_SELECTED, LVIS_SELECTED);
					}
					apply();
				}
				break;
			case IDC_COPY:
				{
					HWND wnd_lv= GetDlgItem(wnd, IDC_COLUMNS);
					int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
					if (idx >= 0 && m_columns.copy_item(idx)) 
					{
						int new_idx = m_columns.get_count() -1;

						pfc::string8 temp;
						m_columns.get_string(new_idx, temp, STRING_NAME);

						if (uih::ListView_InsertItemText(wnd_lv, new_idx, 0, "New Column") == -1)
							m_columns.delete_item(new_idx);
						else
						{
							ListView_SetItemState(wnd_lv, new_idx, LVIS_SELECTED, LVIS_SELECTED);
							ListView_EnsureVisible(wnd_lv, new_idx, FALSE);
						}
					}
					apply();
				}
				break;
			case IDC_INSERT:
				{
					HWND wnd_lv= GetDlgItem(wnd, IDC_COLUMNS);
					int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
					if (idx >= 0 && m_columns.insert_item(idx, "New Column", "", false, "", false, "", 100, ALIGN_LEFT, FILTER_NONE, "", 100, true, ""))
					{
						if (uih::ListView_InsertItemText(wnd_lv, idx, 0, "New Column") == -1)
						{
							m_columns.delete_item(idx);
						}
					}
					apply();
				}
				break;
			case IDC_SAVE_NEW:
				{
					HWND wnd_lv= GetDlgItem(wnd, IDC_COLUMNS);
					column_info * temp = new column_info;
					temp->set_string(STRING_NAME, "New Column");
					uih::ListView_InsertItemText(wnd_lv, m_columns.get_count(), 0, "New Column");
					ListView_SetItemState(wnd_lv, m_columns.get_count(), LVIS_SELECTED, LVIS_SELECTED);
					ListView_EnsureVisible(wnd_lv, m_columns.get_count(), FALSE);
					m_columns.add_item(temp);
					apply();
				}
				break;
#endif
			}
		}
		return 0;
	}
	void apply()
	{
		g_columns.set_entries_copy(m_columns);
		refresh_all_playlist_views();
		pvt::ng_playlist_view_t::g_on_columns_change();
	}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_COLUMNS_V4,wnd,g_on_message, (LPARAM)this);}
	virtual const char * get_name() {return "Columns";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:playlist_view:columns";
		return true;
	}

private:
	column_list_t m_columns;
	bool initialising;
} g_tab_columns_v3;	

preferences_tab * g_get_tab_columns_v3()
{
	return &g_tab_columns_v3;
}
