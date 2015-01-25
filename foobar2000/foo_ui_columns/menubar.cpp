#include "foo_ui_columns.h"

//#define _WIN32_WINNT 0x500

cfg_int cfg_fullsizemenu(create_guid(0xe880f267,0x73de,0x7952,0x5b,0x79,0xb5,0xda,0x77,0x28,0x6d,0xb6),0);


#define WM_CHANGEUISTATE                0x0127
#define WM_UPDATEUISTATE                0x0128
#define WM_QUERYUISTATE                 0x0129

/*
 * LOWORD(wParam) values in WM_*UISTATE*
 */
#define UIS_SET                         1
#define UIS_CLEAR                       2
#define UIS_INITIALIZE                  3

/*
 * HIWORD(wParam) values in WM_*UISTATE*
 */
#define UISF_HIDEFOCUS                  0x1
#define UISF_HIDEACCEL                  0x2
//#if(_WIN32_WINNT >= 0x0501)
#define UISF_ACTIVE                     0x4

#include <windows.h>
#include <COMMCTRL.H>
#include <WinUser.h>
#include <windowsx.h>
#include <commctrl.h>
#include <strsafe.h>

#define ID_MENU  2001
enum {MSG_HIDE_MENUACC = WM_USER + 1, MSG_SHOW_MENUACC = WM_USER + 2, MSG_CREATE_MENU = WM_USER + 3, MSG_SIZE_LIMIT_CHANGE};

#if 0

struct main_menu_group
{
	const char * name;
	const TCHAR * name_accel;
	const GUID * guid;
	main_menu_group(const char * p_name, const TCHAR * p_name_accel, const GUID * p_guid)
		:name(p_name),name_accel(p_name_accel),guid(p_guid)
	{};
};

namespace menu_groups
{

	main_menu_group file("File",_T("&File"),&mainmenu_groups::file);
	main_menu_group edit("Edit",_T("&Edit"),&mainmenu_groups::edit);
	main_menu_group view("View",_T("&View"),&mainmenu_groups::view);
	main_menu_group playback("Playback",_T("&Playback"),&mainmenu_groups::playback);
	main_menu_group library("Library",_T("&Library"),&mainmenu_groups::library);
	main_menu_group help("Help",_T("&Help"),&mainmenu_groups::help);
};

main_menu_group * g_menu_groups[] = 
{
	&menu_groups::file, &menu_groups::edit, &menu_groups::view, &menu_groups::playback, &menu_groups::library,
	&menu_groups::help
};

#endif

//menu_manager.cpp
class mnemonic_manager
{
	pfc::string8_fast_aggressive used;
	bool is_used(unsigned c)
	{
		char temp[8];
		temp[pfc::utf8_encode_char(uCharLower(c),temp)]=0;
		return !!strstr(used,temp);
	}

	static bool is_alphanumeric(char c)
	{
		return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9');
	}




	void insert(const char * src,unsigned idx,pfc::string_base & out)
	{
		out.reset();
		out.add_string(src,idx);
		out.add_string("&");
		out.add_string(src+idx);
		used.add_char(uCharLower(src[idx]));
	}
public:
	bool check_string(const char * src)
	{//check for existing mnemonics
		const char * ptr = src;
		while(ptr = strchr(ptr,'&'))
		{
			if (ptr[1]=='&') ptr+=2;
			else
			{
				unsigned c = 0;
				if (pfc::utf8_decode_char(ptr+1,c)>0)
				{
					if (!is_used(c)) used.add_char(uCharLower(c));
				}
				return true;
			}
		}
		return false;
	}
	bool process_string(const char * src,pfc::string_base & out)//returns if changed
	{
		if (check_string(src)) {out=src;return false;}
		unsigned idx=0;
		while(src[idx]==' ') idx++;
		while(src[idx])
		{
			if (is_alphanumeric(src[idx]) && !is_used(src[idx]))
			{
				insert(src,idx,out);
				return true;
			}

			while(src[idx] && src[idx]!=' ' && src[idx]!='\t') idx++;
			if (src[idx]=='\t') break;
			while(src[idx]==' ') idx++;
		}

		//no success picking first letter of one of words
		idx=0;
		while(src[idx])
		{
			if (src[idx]=='\t') break;
			if (is_alphanumeric(src[idx]) && !is_used(src[idx]))
			{
				insert(src,idx,out);
				return true;
			}
			idx++;
		}

		//giving up
		out = src;
		return false;
	}
};


class mainmenu_root_group
{
public:
	pfc::string8 m_name;
	pfc::array_t<TCHAR> m_name_with_accelerators;
	types::t_guid m_guid;
	t_uint32 m_sort_priority;

	mainmenu_root_group()
		: m_sort_priority(NULL)
	{};

	static t_size g_compare(const mainmenu_root_group & p_item1, const mainmenu_root_group&p_item2)
	{
		return pfc::compare_t(p_item1.m_sort_priority,p_item2.m_sort_priority);
	}
	static void g_get_root_items(pfc::list_base_t<mainmenu_root_group> & p_out)
	{
		p_out.remove_all();
		mnemonic_manager mnemonics;

		service_enum_t<mainmenu_group> e;
		service_ptr_t<mainmenu_group> ptr;
		service_ptr_t<mainmenu_group_popup> ptrp;

		while(e.next(ptr))
		{
			if (ptr->get_parent() == pfc::guid_null)
			{
				if (ptr->service_query_t(ptrp))
				{
					mainmenu_root_group item;
					pfc::string8 name;
					ptrp->get_display_string(name);
					item.m_guid = ptrp->get_guid();
					item.m_sort_priority = ptrp->get_sort_priority();
					uFixAmpersandChars_v2(name, item.m_name);
					name.reset();
					mnemonics.process_string(item.m_name, name);
					pfc::stringcvt::string_os_from_utf8 os(name);
					item.m_name_with_accelerators.append_fromptr(os.get_ptr(), os.length());
					item.m_name_with_accelerators.append_single(0);
					p_out.add_item(item);
				}
			}
		}
		p_out.sort_t(g_compare);
	}
};



class menu_extension : public ui_extension::container_menu_ui_extension, message_hook_manager::message_hook
{
	static const TCHAR * class_name;

	WNDPROC menuproc;
	bool initialised;
public:
//	static pfc::ptr_list_t<playlists_list_window> list_wnd;
	//static HHOOK msghook;
	static bool hooked;
	//static menu_extension * p_hooked_menu;

	bool redrop;
	bool is_submenu;
	bool full_hieght;
	int active_item;
	int actual_active;
	int sub_menu_ref_count;
	service_ptr_t<mainmenu_manager> p_manager;
	service_ptr_t<ui_status_text_override> m_status_override;

	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data_child_ex(class_name, true, false);
	}

	HWND wnd_menu;
	HWND wnd_prev_focus;
	pfc::list_t<mainmenu_root_group> m_buttons;

	LRESULT WINAPI hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static LRESULT WINAPI main_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	virtual bool on_hooked_message(message_hook_manager::t_message_hook_type p_type, int code, WPARAM wp, LPARAM lp);

	void make_menu(unsigned idx);

	inline void destroy_menu()
	{
		uSendMessage(get_wnd(), WM_CANCELMODE, 0, 0);
	}

	inline void update_menu_acc()
	{
		uPostMessage(get_wnd(), MSG_HIDE_MENUACC, 0, 0);
	}

	inline void show_menu_acc()
	{
		uPostMessage(get_wnd(), MSG_SHOW_MENUACC, 0, 0);
	}

	static const GUID extension_guid;

	const GUID & get_extension_guid() const
	{
		return extension_guid;
	}

	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;

	virtual void set_focus();
	virtual void show_accelerators();
	virtual void hide_accelerators();

	virtual bool on_menuchar(unsigned short chr);

	virtual unsigned get_type () const {return ui_extension::type_toolbar;};

	virtual bool is_menu_focused()const; 

	menu_extension();
	~menu_extension();
};

bool menu_extension::hooked = false;

menu_extension::menu_extension() : wnd_menu(0), menuproc(0), 
	redrop(true), is_submenu(false), active_item(0), sub_menu_ref_count(-1), /*hooked(false), */
	actual_active(0), initialised(false), p_manager(0), wnd_prev_focus(0)
{
};

menu_extension::~menu_extension()
{
}

const TCHAR * menu_extension::class_name = _T("{76E6DB50-0DE3-4f30-A7E4-93FD628B1401}");

bool menu_extension::is_menu_focused()const
{
	return GetFocus() == wnd_menu;
}


LRESULT WINAPI menu_extension::main_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	menu_extension * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<menu_extension*>(uGetWindowLong(wnd,GWL_USERDATA));
	
	rv = p_this ? p_this->hook(wnd,msg,wp,lp) : uDefWindowProc(wnd, msg, wp, lp);;
	
	return rv;
}

LRESULT menu_extension::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	
	if(msg == WM_CREATE)
	{
		initialised = true;

		mainmenu_root_group::g_get_root_items(m_buttons);
		t_size button_count = m_buttons.get_count();

		pfc::array_t<TBBUTTON> tbb;
		tbb.set_size(button_count);
		memset(tbb.get_ptr(), 0, tbb.get_size()*sizeof(TBBUTTON));
		
		wnd_menu = CreateWindowEx(/*TBSTYLE_EX_MIXEDBUTTONS|*/WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL, 
			WS_CHILD | WS_VISIBLE|WS_CLIPSIBLINGS | WS_CLIPCHILDREN |TBSTYLE_FLAT | TBSTYLE_TRANSPARENT |TBSTYLE_LIST | CCS_NORESIZE| CCS_NOPARENTALIGN| CCS_NODIVIDER, 
			0, 0, 0, 25, wnd, (HMENU) ID_MENU, core_api::get_my_instance(), NULL); 
		
		if (wnd_menu)
		{
			SetWindowLongPtr(wnd_menu,GWL_USERDATA,(LPARAM)(this));
			
			SendMessage(wnd_menu, TB_SETBITMAPSIZE, (WPARAM) 0, MAKELONG(0,0));
			SendMessage(wnd_menu, TB_SETBUTTONSIZE, (WPARAM) 0, MAKELONG(0,/*GetSystemMetrics(SM_CYMENUSIZE)*/0));
			
			SendMessage(wnd_menu, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 

			unsigned n, count = tbb.get_size();
			for (n=0; n<count; n++)
			{
				tbb[n].iBitmap = I_IMAGECALLBACK; 
				tbb[n].idCommand = n+1; 
				tbb[n].fsState = TBSTATE_ENABLED; 
				tbb[n].fsStyle = BTNS_DROPDOWN|BTNS_AUTOSIZE ; 
				tbb[n].dwData = 0; 
				tbb[n].iString = (int)m_buttons[n].m_name_with_accelerators.get_ptr(); 
			}

			SendMessage(wnd_menu, TB_ADDBUTTONS, (WPARAM) tbb.get_size(), (LPARAM) (LPTBBUTTON) tbb.get_ptr());
			
//			uSendMessage(wnd_menu, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);
			
//			uSendMessage(wnd_menu, TB_AUTOSIZE, 0, 0); 
			
			//if (is_win2k_or_newer())
			{
				BOOL a = true;
				SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
				uSendMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM(a ? UIS_CLEAR : UIS_SET , UISF_HIDEACCEL), 0);
			}
			
//			uSendMessage(wnd_menu, TB_SETPARENT, (WPARAM) (HWND)wnd_host, 0);
			menuproc = (WNDPROC)SetWindowLongPtr(wnd_menu,GWL_WNDPROC,(LPARAM)main_hook);
		}
		
		
		
	}
	else if (msg == WM_WINDOWPOSCHANGED)
	{
		LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
		if (!(lpwp->flags & SWP_NOSIZE))
		{
			//SIZE sz = {0,0};
			//SendMessage(wnd_menu, TB_GETMAXSIZE, NULL, (LPARAM)&sz);

			RECT rc = {0,0,0,0};
			t_size count = m_buttons.get_count();
			int cx = lpwp->cx;
			int cy = lpwp->cy;
			int extra = 0;
			if (count && (BOOL)SendMessage(wnd_menu, TB_GETITEMRECT, count - 1, (LPARAM)(&rc)))
			{
				cx = min(cx, rc.right);
				cy = min(cy, rc.bottom);
				extra = (lpwp->cy - rc.bottom)/2;
			}
			SetWindowPos(wnd_menu, 0, 0, extra, cx, cy, SWP_NOZORDER);
			RedrawWindow(wnd, 0, 0, RDW_ERASE|RDW_INVALIDATE);
		}
	}
	else if (msg==WM_SIZE)
	{
	}
	else if (msg== WM_NOTIFY && ((LPNMHDR)lp)->idFrom == ID_MENU)
	{
		switch (((LPNMHDR)lp)->code)
		{
		case TBN_HOTITEMCHANGE:
			{
				if (!(((LPNMTBHOTITEM)lp)->dwFlags & HICF_LEAVING)  && (((LPNMTBHOTITEM)lp)->dwFlags & HICF_MOUSE || ((LPNMTBHOTITEM)lp)->dwFlags & HICF_LMOUSE))
					redrop = true;
			}
			break;
		case TBN_DROPDOWN:
			{
				if (redrop)
				{
					PostMessage(wnd, MSG_CREATE_MENU, ((LPNMTOOLBAR)lp)->iItem, 0);
				}
				else redrop = true;
				
				return TBDDRET_DEFAULT;
			}
		}
	}
	else if (msg == MSG_HIDE_MENUACC)
	{
		//if (is_win2k_or_newer())
		{
			BOOL a = true;
			SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
			if ((uSendMessage(wnd_menu, WM_QUERYUISTATE,0,0) & UISF_HIDEACCEL) != !a)
			SendMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM(a ?  UIS_CLEAR  : UIS_SET , UISF_HIDEACCEL), 0);
		}
	}
	else if (msg == MSG_SHOW_MENUACC)
	{
		//if (is_win2k_or_newer())
		{
			SendMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);
		}
	}
	
	else if (msg == MSG_CREATE_MENU)
	{
		if (lp) SetFocus(wnd_menu);
		active_item = wp;
		
		make_menu(wp);
	}
	else if (msg == WM_MENUSELECT)
	{
		if (HIWORD(wp) & MF_POPUP)
		{
			is_submenu = true;
			m_status_override.release();
		}
		else 
		{
			is_submenu = false;
			if (p_manager.is_valid())
			{
				unsigned id = LOWORD(wp);
				
				bool set = false;

				pfc::string8 blah;

				set = p_manager->get_description(id-1, blah);
				
				service_ptr_t<ui_status_text_override> p_status_override;

				if (set)
				{
					get_host()->override_status_text_create(p_status_override);

					if (p_status_override.is_valid())
					{
						p_status_override->override_text(blah);
					}
				}
				m_status_override = p_status_override;
			}
		}
	}
	else if (msg == WM_INITMENUPOPUP)
	{
		sub_menu_ref_count++;
	}
	else if (msg == WM_UNINITMENUPOPUP)
	{
		sub_menu_ref_count--;
	}
	else if (msg == WM_GETMINMAXINFO)
	{
		LPMINMAXINFO mmi = LPMINMAXINFO(lp);

		RECT rc = {0,0,0,0};
		SendMessage(wnd_menu,  TB_GETITEMRECT, m_buttons.get_count()-1, (LPARAM)(&rc));

		//SIZE sz = {0,0};
		//SendMessage(wnd_menu, TB_GETMAXSIZE, NULL, (LPARAM)&sz);
		//console::formatter() << sz.cx << sz.cy;

		mmi->ptMinTrackSize.x = rc.right;
		mmi->ptMinTrackSize.y = rc.bottom;
		mmi->ptMaxTrackSize.y = rc.bottom;
		return 0;
	}
	else if (msg == WM_SETTINGCHANGE)
	{
		if (wp == SPI_SETNONCLIENTMETRICS)
		{
			PostMessage(wnd, MSG_SIZE_LIMIT_CHANGE, 0,0);
		}
	}
	else if (wp == SPI_GETKEYBOARDCUES)
	{
		BOOL a = TRUE;
		SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
		SendMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM((a || GetFocus() == wnd_menu) ? UIS_CLEAR : UIS_SET , UISF_HIDEACCEL), 0);
	}
	else if (msg == MSG_SIZE_LIMIT_CHANGE)
	{
		get_host()->on_size_limit_change(wnd, uie::size_limit_minimum_height|uie::size_limit_maximum_height|uie::size_limit_minimum_width);
	}
	else if (msg == WM_DESTROY)
	{
		DestroyWindow(wnd_menu);
		wnd_menu=NULL;
		m_buttons.remove_all();
		initialised = false;
	}
	return DefWindowProc(wnd, msg, wp, lp);
}


LRESULT WINAPI menu_extension::hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{

	switch(msg)
	{
	case WM_KILLFOCUS:
		{
			update_menu_acc();
		}
		break;
	case WM_SETFOCUS:
		{
			show_menu_acc();
			wnd_prev_focus=(HWND)wp;
		}
		break;
	case WM_CHAR:
		if (wp == ' ') return 0;
		break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			win32_keyboard_lparam & lpkeyb = get_keyboard_lparam(lp);
			if ( (wp == VK_ESCAPE || wp==VK_MENU) && !lpkeyb.previous_key_state) 
			{
				update_menu_acc();
				if (wp == VK_ESCAPE) SetFocus(wnd_prev_focus);
				else PostMessage(wnd, TB_SETHOTITEM, -1, 0);
				return 0;
			}
			else if ( (wp == VK_SPACE) && !lpkeyb.previous_key_state) 
			{
				LPARAM newlp = lp;
				((win32_keyboard_lparam&)newlp).context_code = 1;
				HWND wndparent = uFindParentPopup(wnd);
				if (wndparent) PostMessage(wndparent, WM_SYSKEYDOWN, wp, lp | (1<<29) ); 
				return 0;
			}
		}
		break;
	case WM_SYSKEYUP:
		{
			if ( wp==VK_MENU) 
			{
				SetFocus(wnd_prev_focus);
				return 0;
			}
		}
		break;
	}
	return uCallWindowProc(menuproc,wnd,msg,wp,lp);
}

void menu_extension::make_menu(unsigned idx)
{
	if (idx == actual_active || hooked || idx < 1 || idx > m_buttons.get_count()) return;

	service_ptr_t<menu_extension> dummy = this; //menu command may delete us
	
	actual_active = idx;
	
	RECT rc;
	POINT pt;
	
	uSendMessage(wnd_menu, TB_GETRECT,	idx, (LPARAM)&rc);
	
	MapWindowPoints(wnd_menu, HWND_DESKTOP, (LPPOINT)&rc, 2);  
	
	pt.x = rc.left;
	pt.y = rc.bottom;

	HMENU menu = CreatePopupMenu();

	hooked = true;
//	p_hooked_menu = this;
	message_hook_manager::register_hook(message_hook_manager::type_message_filter, this);
	//msghook = uSetWindowsHookEx(WH_MSGFILTER, menu_hook_t_proc, 0, GetCurrentThreadId());
	service_ptr_t<mainmenu_manager> p_menu = standard_api_create_t<mainmenu_manager>();

	bool b_keyb = standard_config_objects::query_show_keyboard_shortcuts_in_menus();


	if (p_menu.is_valid())
	{
		p_menu->instantiate(m_buttons[idx-1].m_guid);
		p_menu->generate_menu_win32(menu,1,-1,b_keyb ? mainmenu_manager::flag_show_shortcuts : 0);
		menu_helpers::win32_auto_mnemonics(menu);
	}

	SendMessage(wnd_menu, TB_PRESSBUTTON, idx, TRUE);
	SendMessage(wnd_menu, TB_SETHOTITEM, idx-1, 0);
	is_submenu = false;

	sub_menu_ref_count = -1; //we get a notificationwhen the inital menu is created

//	RECT rc_desktop;
//	if (GetWindowRect(GetDesktopWindow(), &rc_desktop))

//	if (pt.x < rc_desktop.left) pt.x = rc_desktop.left;
//	else if (pt.x > rc_desktop.right) pt.x = rc_desktop.right;

	p_manager = p_menu;

	TPMPARAMS tpmp;
	memset(&tpmp, 0, sizeof(TPMPARAMS));
	tpmp.cbSize = sizeof(tpmp);

	GetWindowRect(wnd_menu, &tpmp.rcExclude);

	HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

	if (mon)
	{
		MONITORINFO mi;
		memset(&mi, 0, sizeof(MONITORINFO));
		mi.cbSize = sizeof(MONITORINFO);
		if (uGetMonitorInfo(mon, &mi))
		{
			if (pt.x < mi.rcMonitor.left) pt.x = mi.rcMonitor.left;
			tpmp.rcExclude.left = mi.rcMonitor.left;
			tpmp.rcExclude.right = mi.rcMonitor.right;
		}
	}

//	menu_info::MENU_A_BASE = 1;
//	menu_info::MENU_B_BASE = -1;

	if (GetFocus() != wnd_menu) 
		wnd_prev_focus = GetFocus();

	int cmd = TrackPopupMenuEx(menu,TPM_LEFTBUTTON|TPM_RETURNCMD,pt.x,pt.y,get_wnd(),&tpmp);
//	int cmd = TrackPopupMenuEx(menu,TPM_LEFTBUTTON|TPM_RETURNCMD,pt.x,pt.y,g_main_window,0);
	
	m_status_override.release();

	//SendMessage(wnd_menu, TB_PRESSBUTTON, idx, FALSE);

	PostMessage(wnd_menu, TB_PRESSBUTTON, idx, FALSE);
	if (GetFocus() != wnd_menu) 
	{
		PostMessage(wnd_menu, TB_SETHOTITEM, -1, 0);
	}



	DestroyMenu(menu);

	if (cmd>0 && p_menu.is_valid())
	{
//		uSendMessage(wnd_menu, TB_SETHOTITEM, -1, 0);
		if (IsWindow(wnd_prev_focus))
			SetFocus(wnd_prev_focus);
		p_menu->execute_command(cmd - 1);
	}

//	if (p_menu.is_valid()) 
	{
		p_manager.release();
		p_menu.release();
	}

	//UnhookWindowsHookEx(msghook); // hook may not be freed instantly, so dont make msghook = 0
	message_hook_manager::deregister_hook(message_hook_manager::type_message_filter, this);
	hooked = false;
	//p_hooked_menu=0;

	actual_active = 0;
}

bool menu_extension::on_hooked_message(message_hook_manager::t_message_hook_type p_type, int code, WPARAM wp, LPARAM lp)
{
	static POINT last_pt;

	if (code == MSGF_MENU)
	{
		switch(((MSG*)lp)->message)
		{

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			{
				uSendMessage(wnd_menu, TB_SETHOTITEM, -1, 0);
				if (((MSG*)lp)->message == WM_LBUTTONDOWN)
				{
					POINT pt; RECT toolbar;
					GetClientRect(get_wnd(), &toolbar);
					pt.y = GET_Y_LPARAM(((MSG*)lp)->lParam);
					pt.x = GET_X_LPARAM(((MSG*)lp)->lParam);


					if (ScreenToClient(wnd_menu, &pt) && PtInRect(&toolbar, pt))
					{
						t_size idx = SendMessage(wnd_menu, TB_HITTEST, 0, (long)&pt);

						if (idx >= 0 && idx < m_buttons.get_count())
							redrop = false;
					}
				}
			}
			break;
		case WM_KEYDOWN:
			{
				switch ( ((MSG*)lp)->wParam )
				{
				case VK_LEFT:
					if (!sub_menu_ref_count) 
					{
						destroy_menu();
						if (active_item==1) active_item = m_buttons.get_count(); else active_item--;
						uPostMessage(get_wnd(), MSG_CREATE_MENU, active_item, 0);
					}

					break;
				case VK_RIGHT:
					if (!is_submenu) 
					{
						destroy_menu();
						if (active_item==m_buttons.get_count()) active_item = 1; else active_item++;
						uPostMessage(get_wnd(), MSG_CREATE_MENU, active_item, 0);
					}
					break;
				case VK_ESCAPE:
					if (!sub_menu_ref_count) 
					{
						SetFocus(wnd_menu);
						uSendMessage(wnd_menu, TB_SETHOTITEM, active_item-1, 0);
						destroy_menu();
					}
					break;
				}
			}
			break;

		case WM_MOUSEMOVE:
			{
				POINT px;
				px.y = GET_Y_LPARAM(((MSG*)lp)->lParam);
				px.x = GET_X_LPARAM(((MSG*)lp)->lParam);
				if (px.x != last_pt.x || px.y != last_pt.y)
				{
					HWND wnd_hit = WindowFromPoint(px);
					if (wnd_hit == wnd_menu)
					{
						POINT pt = px;
						int hot_item = uSendMessage(wnd_menu, TB_GETHOTITEM, 0, 0);
						if (ScreenToClient(wnd_menu, &pt))
						{

							t_size idx = SendMessage(wnd_menu, TB_HITTEST, 0, (long)&pt);

							if (idx >= 0 && idx < m_buttons.get_count() && (active_item-1) != idx)
							{
								destroy_menu();
								active_item = idx+1; 
								uPostMessage(get_wnd(), MSG_CREATE_MENU, active_item, 0);
							} 
						}
					}
				}
				last_pt = px;
			}
			break;

		}
	}
	return false;
}



void menu_extension::get_name(pfc::string_base & out)const
{
	out.set_string("Menu");
}
void menu_extension::get_category(pfc::string_base & out)const
{
	out.set_string("Toolbars");
}

void menu_extension::set_focus()
{
	{
		SetFocus(wnd_menu);
	}
}
void menu_extension::show_accelerators()
{
	{
		show_menu_acc();
	}
}
void menu_extension::hide_accelerators()
{
	{
		if (GetFocus() != wnd_menu) update_menu_acc();
	}
}

bool menu_extension::on_menuchar(unsigned short chr)
{
	{
		UINT id;
		if (uSendMessage(wnd_menu,TB_MAPACCELERATOR,chr,(LPARAM)&id))
		{
			uPostMessage(get_wnd(),MSG_CREATE_MENU,id,TRUE);
			return true;
		}
	}
	return false;
}



// {76E6DB50-0DE3-4f30-A7E4-93FD628B1401}
const GUID menu_extension::extension_guid = 
{ 0x76e6db50, 0xde3, 0x4f30, { 0xa7, 0xe4, 0x93, 0xfd, 0x62, 0x8b, 0x14, 0x1 } };

ui_extension::window_factory<menu_extension> blah;





