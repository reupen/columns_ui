#include "foo_ui_columns.h"

#if 0
//#define _WIN32_WINNT 0x500


#define ID_SEEK 2005
#define SEEK_TIMER_ID  669

ptr_list_t<seekbar_extension> seekbar_extension::windows;

extern bool g_playing;

/*static class order_modify_callback : public config_var_callback_autoreg<config_var_string>
{
	mem_block_list_t<HWND> combos;
	virtual void on_changed(const config_var_string * ptr)
	{
		unsigned n, count = combos.get_count();

		for (n = 0; n< count; n++)
		{
			string8 name;
			ptr->get_value(name);
		 	uComboBox_SelectString(combos[n], name);
		}
	}
public:
	void register_callback(HWND combo, bool calloninit)
	{
		combos.add_item(combo);
		if (calloninit)
		{
			string8 temp;
			playback_order_helper::get_config_by_name(temp);
			uComboBox_SelectString(combo, temp);
		}
	}
	void deregister_callback(HWND combo)
	{
		combos.remove_item(combo);
	}
public:
	order_modify_callback() : config_var_callback_autoreg<config_var_string>("CORE/Playback flow control",false) {}
} g_order_modify_callback;*/

void seekbar_extension::update_seekbars(bool positions_only)
{
	unsigned n, count = windows.get_count();
	if (positions_only)
	{
		for (n=0; n<count; n++)
			if (windows[n]->wnd_host) windows[n]->update_seek_pos();
	}
	else
	{
		for (n=0; n<count; n++)
			if (windows[n]->wnd_host) windows[n]->update_seek();
	}

}

unsigned seekbar_extension::g_seek_timer = 0;

void seekbar_extension::update_seek_timer()
{
	if (windows.get_count() && g_playing)
	{
		if (!g_seek_timer) 
		{
			g_seek_timer = SetTimer(0, SEEK_TIMER_ID, 150, (TIMERPROC)SeekTimerProc);
	//		seekbar_extension::update_seekbars();
		}
	}
	else if (g_seek_timer)
	{


		KillTimer(0, g_seek_timer);
		g_seek_timer=0;
	//	seekbar_extension::update_seekbars();
	}
//		console::info(string_printf("%u %u",windows.get_count(),g_playing?1:0));
}

void seekbar_extension::update_seek_pos()
{
	if (wnd_seekbar==0) return;

	static_api_ptr_t<play_control> play_api;
	
	if (play_api->is_playing() && play_api->playback_get_length()/* && play_api->playback_can_seek()*/)
	{
		double position = 0, length = 0;
		position = play_api->playback_get_position();
		length = play_api->playback_get_length();

		if (position<0) position=0;
		if (position>length) position=length;
				
		int pos_display = (int) (10.0*position);
		if (!is_seeking) uSendMessage(wnd_seekbar,TBM_SETPOS,1,pos_display);
	}
}


VOID CALLBACK seekbar_extension::SeekTimerProc(HWND wnd, UINT msg, UINT event, DWORD time)
{
	if (windows.get_count() && g_playing) update_seekbars(true);
}

LRESULT seekbar_extension::create_tooltip(const char * text, POINT pt)
{
	destroy_tooltip();

	wnd_tooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_NOPREFIX ,		
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, wnd_host, 0, core_api::get_my_instance(), NULL);
	
    SetWindowPos(wnd_tooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	
	uTOOLINFO ti;

	memset(&ti,0,sizeof(ti));
	
	ti.cbSize = sizeof(uTOOLINFO);
	ti.uFlags = TTF_SUBCLASS|TTF_TRANSPARENT|TTF_TRACK|TTF_ABSOLUTE;
	ti.hwnd = wnd_host;
	ti.hinst = core_api::get_my_instance();
	ti.lpszText = const_cast<char *>(text);
	
	uToolTip_AddTool(wnd_tooltip, &ti);
	
	uSendMessage(wnd_tooltip, TTM_TRACKPOSITION, 0, MAKELONG(pt.x, pt.y+21));
	uSendMessage(wnd_tooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)  &ti);	
	
	return TRUE;
}

void seekbar_extension::destroy_tooltip()
{
	if (wnd_tooltip) {DestroyWindow(wnd_tooltip); wnd_tooltip=0;}
}


LRESULT seekbar_extension::update_tooltip(const char * text)
{
	if (!wnd_tooltip) return FALSE;

	uTOOLINFO ti;
	memset(&ti,0,sizeof(ti));
	
	ti.cbSize = sizeof(uTOOLINFO);
	ti.uFlags = TTF_SUBCLASS|TTF_TRANSPARENT|TTF_TRACK|TTF_ABSOLUTE;
	ti.hwnd = wnd_host;
	ti.hinst = core_api::get_my_instance();
	ti.lpszText = const_cast<char *>(text);

	uToolTip_AddTool(wnd_tooltip, &ti, true);

	return TRUE;
}

void seekbar_extension::disable_seek()
{
	uSendMessage(wnd_seekbar,TBM_SETPOS,1,0);
	EnableWindow(wnd_seekbar,FALSE);
}

void seekbar_extension::update_seek()
{
//	console::info("tet");
	if (wnd_seekbar==0) return;

	static_api_ptr_t<play_control> play_api;
	
	if (play_api->is_playing() && play_api->playback_get_length()/* && play_api->playback_can_seek()*/)
	{
		double position = 0, length = 0;
		position = play_api->playback_get_position();
			length = play_api->playback_get_length();
		
		if (length<0) length = 0;
		if (position<0) position=0;
		if (position>length) position=length;
		
		uSendMessage(wnd_seekbar,TBM_SETRANGEMIN,0,0);
		uSendMessage(wnd_seekbar,TBM_SETRANGEMAX,0,(long)(10*length));
		
		int pos_display = (int) (10.0*position);
		if (!is_seeking) uSendMessage(wnd_seekbar,TBM_SETPOS,1,pos_display);

/*
		bool b_can_seek = play_api->playback_can_seek();
		bool b_enabled = IsWindowEnabled(wnd_seekbar) != 0;

		if (b_can_seek != b_enabled)
			EnableWindow(wnd_seekbar,b_can_seek);
*/

		if (play_api->playback_can_seek()/* && play_api->playback_get_length()*/)
			EnableWindow(wnd_seekbar,TRUE);
		
	}
	else
	{
		disable_seek();
	}
}


seekbar_extension::seekbar_extension() : wnd_seekbar(0), seekproc(0), is_seeking(false), last_seek_msg(0), last_seek_lp(0), last_seek_wp(0), wnd_prev_focus(0), initialised(0)
{
};

seekbar_extension::~seekbar_extension()
{
	if (initialised)
	{
		windows.remove_item(this);
		initialised=false;
	}
}

const char * seekbar_extension::class_name = "{678FE380-ABBB-4c72-A0B3-72E769671125}";

LRESULT WINAPI seekbar_extension::main_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	seekbar_extension * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<seekbar_extension*>(uGetWindowLong(wnd,GWL_USERDATA));
	
	rv = p_this ? p_this->hook(wnd,msg,wp,lp) : uDefWindowProc(wnd, msg, wp, lp);
	
	return rv;
}

LRESULT WINAPI seekbar_extension::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	
	if(msg == WM_CREATE)
	{
		windows.add_item(this);

		initialised = true;

		wnd_seekbar = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_VISIBLE|WS_CHILD|TBS_NOTICKS ,
			0,0,100,20, wnd, (HMENU)ID_SEEK, core_api::get_my_instance(), NULL);

		if (wnd_seekbar)
		{
			uSetWindowLong(wnd_seekbar,GWL_USERDATA,(LPARAM)(this));

	//		uSendMessage(p_this->wnd_seekbar,WM_SETFONT,(WPARAM)g_def_font,MAKELPARAM(1,0));
			update_seek();
				
			seekproc = (WNDPROC)uSetWindowLong(wnd_seekbar,GWL_WNDPROC,(LPARAM)main_hook);
			update_seek_timer();
			

	//		g_order_modify_callback.register_callback(p_this->wnd_seekbar, true);


		}

		seekbar_extension::update_seek_timer();
		
	}
	else if (msg==WM_SIZE)
	{
		SetWindowPos(wnd_seekbar, 0, 0, 0, LOWORD(lp), 20, SWP_NOZORDER);
	}
/*	if (msg == WM_NOTIFY && ((LPNMHDR)lp)->idFrom == ID_SEEK && ((LPNMHDR)lp)->code ==NM_CUSTOMDRAW)
	{
		LPNMCUSTOMDRAW lpnmcd = (LPNMCUSTOMDRAW) lp;

		console::info(string_printf("%x",lpnmcd->dwDrawStage));

		switch (lpnmcd->dwDrawStage)
		{
		case CDDS_PREPAINT :
			{
				HDC dc = lpnmcd->hdc;

				HBRUSH br = CreateSolidBrush(0xff0000);

				FillRect(dc, &lpnmcd->rc, br);

				DeleteObject(br);
			}
			return  CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
		case CDDS_ITEMPREPAINT :
		case CDDS_PREERASE :
		//	if (lpnmcd->dwItemSpec == TBCD_TICS)
			{
				console::info("nnn");
				HDC dc = lpnmcd->hdc;

				HBRUSH br = CreateSolidBrush(0xff0000);

				FillRect(dc, &lpnmcd->rc, br);

				DeleteObject(br);

				if (dc)
				{
					POINT pt = {0, 0};
					MapWindowPoints(p_this->wnd_seekbar, wnd, &pt, 1);
					OffsetWindowOrgEx(dc, pt.x, pt.y, 0);
					uSendMessage(wnd, WM_ERASEBKGND,wp, 0);
					SetWindowOrgEx(dc, pt.x, pt.y, 0);
				}
				return CDRF_SKIPDEFAULT;
			}
			return CDRF_DODEFAULT;
		}

	}*/
/*	else if (msg == WM_ERASEBKGND)
	{
		HDC dc = (HDC)wp;

		if (dc)
		{
			HWND wnd_parent = GetParent(wnd);
			POINT pt = {0, 0};
			MapWindowPoints(wnd, wnd_parent, &pt, 1);
			OffsetWindowOrgEx(dc, pt.x, pt.y, 0);
			uSendMessage(wnd_parent, WM_ERASEBKGND,wp, 0);
			SetWindowOrgEx(dc, pt.x, pt.y, 0);
		}
		return TRUE;
	}*/
	else if (msg == WM_GETMINMAXINFO)
	{
		LPMINMAXINFO mmi = LPMINMAXINFO(lp);

		if (wnd_seekbar)
		{
			RECT rc;
			GetWindowRect(wnd_seekbar, &rc);
			mmi->ptMinTrackSize.y = rc.bottom-rc.top;
			mmi->ptMaxTrackSize.y = rc.bottom-rc.top;
		}

		return 0;
	}

	else if (msg == WM_SETFOCUS )
	{
		wnd_prev_focus = (HWND)wp;
	}
	else if (msg == WM_KEYDOWN && wp == VK_ESCAPE)
	{
		destroy_tooltip();
		if (GetCapture() == wnd_seekbar) ReleaseCapture();
		SetFocus(IsWindow(wnd_prev_focus) ? wnd_prev_focus : core_api::get_main_window());
		is_seeking = false;
		return 0;
	}
	else if (msg == WM_DESTROY)
	{
		if (initialised)
		{
			windows.remove_item(this);
			initialised=false;
		}
	}
	return uDefWindowProc(wnd, msg, wp, lp);
}


LRESULT WINAPI seekbar_extension::hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_DESTROY:
	/*		if (g_seekbar_timer) 
	{
	KillTimer(0, g_seekbar_timer);
	g_seekbar_timer=0;
	}*/
		wnd_seekbar=0;
		break;
	case WM_MOUSEWHEEL:
		return uDefWindowProc(wnd, msg, wp, lp);
	case WM_MOUSEMOVE:
		if (is_seeking && (last_seek_msg != msg ||  last_seek_wp != wp || last_seek_lp != lp))
		{
			
			POINT pt; RECT rect; RECT client; RECT thumb;
			pt.x = GET_X_LPARAM(lp);
			pt.y = GET_Y_LPARAM(lp);
			GetClientRect(wnd_seekbar, &client);
			uSendMessage(wnd_seekbar,TBM_GETCHANNELRECT,0,(long)&rect);
			uSendMessage(wnd_seekbar,TBM_GETTHUMBRECT,0,(long)&thumb);
			rect.top=client.top;
			rect.bottom=client.bottom;
			rect.left += MulDiv(thumb.right-thumb.left,1,2);
			rect.right -= MulDiv(thumb.right-thumb.left,1,2);
			
			if (IsWindowEnabled(wnd_seekbar)) 
			{
				POINT px = pt;
				ClientToScreen(wnd_seekbar, &px);
				
				if (pt.x>rect.right) pt.x = rect.right;
				else if (pt.x<rect.left) pt.x = rect.left;
				uSendMessage(wnd_tooltip, TTM_TRACKPOSITION, 0, MAKELONG(px.x, px.y+21/*GetSystemMetrics(SM_CYCURSOR)*/));
				int pos = MulDiv(pt.x - rect.left, uSendMessage(wnd_seekbar,TBM_GETRANGEMAX,0,0), rect.right-rect.left);
				
				//				double pos_d = (double(pt.x - rect.left)) * (static_api_ptr_t<play_control>()->get_playback_time()) / (double(right-rect.left));
				uSendMessage(wnd_seekbar,TBM_SETPOS,1,pos);
				//				console::info(string_printf("%i %i %i %i",pt.x - rect.left, uSendMessage(wnd_seekbar,TBM_GETRANGEMAX,0,0), rect.right-rect.left,sizeof(int)));
				if (wp & MK_SHIFT) static_api_ptr_t<play_control>()->playback_seek(pos/10.0);
				update_tooltip(string_print_time_ex(pos/10.0,1));
				last_seek_wp = wp;
				last_seek_lp = lp;
				last_seek_msg = msg;
			}
			return 0;
		}
		break;
	case WM_LBUTTONUP:
		{
			if (is_seeking)
			{
				int pos = uSendMessage(wnd_seekbar,TBM_GETPOS,0,0);
				if (!(wp & MK_SHIFT)) static_api_ptr_t<play_control>()->playback_seek(pos/10.0);
				destroy_tooltip();
				
				if (GetCapture() == wnd) ReleaseCapture();
				is_seeking = false;
				last_seek_msg = msg;
				SetFocus(IsWindow(wnd_prev_focus) ? wnd_prev_focus : core_api::get_main_window());
			}
		}
		return 0;
	case WM_RBUTTONUP:
		{
			if (is_seeking)
			{
				destroy_tooltip();
				if (GetCapture() == wnd) ReleaseCapture();
				SetFocus(IsWindow(wnd_prev_focus) ? wnd_prev_focus : core_api::get_main_window());
				is_seeking = false;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			
			last_seek_msg = msg;
			if (IsWindowEnabled(wnd_seekbar)) 
			{
				POINT pt; 
				
				pt.x = GET_X_LPARAM(lp);
				pt.y = GET_Y_LPARAM(lp);
				RECT rect; RECT client; RECT thumb;
				GetClientRect(wnd_seekbar, &client);
				
				if (PtInRect(&client, pt))
				{
					uSendMessage(wnd_seekbar,TBM_GETCHANNELRECT,0,(long)&rect);
					uSendMessage(wnd_seekbar,TBM_GETTHUMBRECT,0,(long)&thumb);
					rect.top=client.top;
					rect.bottom=client.bottom;
					
					is_seeking = true;
					SetCapture(wnd);
					
					SetFocus(wnd_host);
					int pos = MulDiv(pt.x - rect.left, uSendMessage(wnd_seekbar,TBM_GETRANGEMAX,0,0)+1, rect.right-rect.left-1);
					uSendMessage(wnd_seekbar,TBM_SETPOS,1,pos);
					ClientToScreen(wnd_seekbar, &pt);
					create_tooltip(string_print_time_ex(pos/10.0,1), pt);
				}
			}
			return 0;
		}
		return 0;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		return 0;
	}
	return uCallWindowProc(seekproc,wnd,msg,wp,lp);
}





const GUID & seekbar_extension::get_extension_guid() const
{
	return extension_guid;
}

void seekbar_extension::get_name(string_base & out)const
{
	out.set_string("Seekbar");
}
void seekbar_extension::get_category(string_base & out)const
{
	out.set_string("Toolbars");
}

unsigned seekbar_extension::get_type  () const{return ui_extension::TYPE_TOOLBAR;};

// {678FE380-ABBB-4c72-A0B3-72E769671125}
const GUID seekbar_extension::extension_guid = 
{ 0x678fe380, 0xabbb, 0x4c72, { 0xa0, 0xb3, 0x72, 0xe7, 0x69, 0x67, 0x11, 0x25 } };

ui_extension::window_factory<seekbar_extension> blah;



#endif