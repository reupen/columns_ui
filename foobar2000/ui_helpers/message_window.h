#ifndef _CUI_MESSAGE_H_
#define _CUI_MESSAGE_H_

#if 1
class message_window_t : public ui_helpers::container_window_v2
#else
class message_window_t : public ui_helpers::container_window_autorelease_t
#endif
{
public:
	message_window_t()
		: m_wnd_edit(NULL), m_flags(NULL), m_wnd_button(NULL),m_wnd_static(NULL) {};
	class callback_t : public main_thread_callback
	{
	public:
		callback_t(HWND wnd, const char * p_title, const char * p_text, INT oem_icon = OIC_INFORMATION)
			: m_wnd(wnd),m_title(p_title),m_text(p_text), m_oem_icon(oem_icon)
		{};
	private:
		virtual void callback_run()
		{
			g_run(core_api::get_main_window(), m_title, m_text, m_oem_icon);
		}
		pfc::string8 m_title, m_text;
		HWND m_wnd;
		INT m_oem_icon;
	};
	static void g_run(HWND wnd_parent, const char * p_title, const char * p_text, INT icon = OIC_INFORMATION)
	{
		message_window_t * p_test = new message_window_t;
		RECT rc;
		GetWindowRect(wnd_parent, &rc);
		int cx = 400;
		int cy = 150;
		HWND wnd;
		if (wnd = p_test->create(wnd_parent,0,ui_helpers::window_position_t((rc.left+(RECT_CX(rc)-cx)/2), (rc.top+(RECT_CY(rc)-cy)/2), cx, cy)))
		{
			p_test->initialise(p_title, p_text, icon);
			cy = min (p_test->calc_height(), max (RECT_CY(rc),150));;
			int y = (rc.top+(RECT_CY(rc)-cy)/2);
			if (y < rc.top) y = rc.top;			
			SetWindowPos(wnd, NULL, (rc.left+(RECT_CX(rc)-cx)/2), y, cx, cy, SWP_NOZORDER);
			ShowWindow(wnd, SW_SHOWNORMAL);
		}
		else
			delete p_test;
	}
	static void g_run_threadsafe(HWND wnd, const char * p_title, const char * p_text, INT oem_icon = OIC_INFORMATION)
	{
		service_ptr_t<main_thread_callback> cb = new service_impl_t<callback_t>(wnd, p_title, p_text, oem_icon);
		static_api_ptr_t<main_thread_callback_manager>()->add_callback(cb);
	}
	static void g_run_threadsafe(const char * p_title, const char * p_text, INT oem_icon = OIC_INFORMATION)
	{
		service_ptr_t<main_thread_callback> cb = new service_impl_t<callback_t>((HWND)NULL, p_title, p_text, oem_icon);
		static_api_ptr_t<main_thread_callback_manager>()->add_callback(cb);
	}
	int calc_height()
	{
		RECT rc, rcw, rcwc;
		GetWindowRect(m_wnd_button, &rc);
		GetWindowRect(get_wnd(), &rcw);
		GetClientRect(get_wnd(), &rcwc);
		return 11+11+11+11+1+RECT_CY(rc)+(RECT_CY(rcw)-RECT_CY(rcwc)) + max((t_size)get_text_height(),(t_size)get_icon_height());
	}
	int get_text_height()
	{
		return SendMessage(m_wnd_edit, EM_GETLINECOUNT, 0, 0)*uGetFontHeight(m_font);
	}
	int get_icon_height()
	{
		RECT rc_icon;
		GetWindowRect(m_wnd_static, &rc_icon);
		return RECT_CY(rc_icon);
	}
private:
#if 1
	virtual t_uint32 get_styles() const {return style_popup_default;}
	virtual t_uint32 get_ex_styles() const {return ex_style_popup_default;}
	virtual const GUID & get_class_guid() 
	{
		// {679AB57E-F613-4109-95D2-8C06D7D1DEBB}
		static const GUID g_guid = 
		{ 0x679ab57e, 0xf613, 0x4109, { 0x95, 0xd2, 0x8c, 0x6, 0xd7, 0xd1, 0xde, 0xbb } };
		return g_guid;
	};

#else
	class_data & get_class_data() const 
	{
		__implement_get_class_data_ex(_T("Dop_Message_Box"), _T("Message"), false, 0, WS_SYSMENU | WS_POPUP | WS_CLIPSIBLINGS| WS_CLIPCHILDREN  | WS_CAPTION | WS_THICKFRAME, WS_EX_DLGMODALFRAME, 0/*CS_VREDRAW*/);
	}
#endif
	void initialise(const char * p_title, const char * p_text, INT oem_icon = OIC_INFORMATION)
	{
		uSetWindowText(get_wnd(), p_title);
		pfc::string8 buffer;
		const char * ptr = p_text, *start = ptr;

		while (*ptr)
		{
			start = ptr;
			while (*ptr && *ptr != '\r' && *ptr!='\n') ptr++;
			if (ptr>start)
				buffer.add_string(start, ptr-start);
			if (*ptr)
			{
				buffer.add_byte('\r');
				buffer.add_byte('\n');
			}
			if (*ptr == '\r') ptr++;
			if (*ptr == '\n') ptr++;
		}
		
		uSetWindowText(m_wnd_edit, buffer);
		HICON icon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(oem_icon), IMAGE_ICON, 0,0, LR_DEFAULTSIZE|LR_SHARED);
		SendMessage(m_wnd_static, STM_SETIMAGE, IMAGE_ICON, LPARAM(icon));
		
	}
	virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch (msg)
		{
		case WM_NCCREATE:
			modeless_dialog_manager::g_add(wnd);
			break;
		case WM_SIZE:
			{
				RedrawWindow(wnd, 0, 0, RDW_INVALIDATE);
				RECT rc, rcicon;
				GetWindowRect(m_wnd_button, &rc);
				GetWindowRect(m_wnd_static, &rcicon);
				int cy_button = RECT_CY(rc);
				HDWP dwp = BeginDeferWindowPos(3);
				dwp = DeferWindowPos(dwp, m_wnd_edit, NULL, 11+11+7+RECT_CX(rcicon), 11, LOWORD(lp)-11*4-7-RECT_CX(rcicon), HIWORD(lp)-11*2-cy_button-11-11, SWP_NOZORDER);
				dwp = DeferWindowPos(dwp, m_wnd_button, NULL, LOWORD(lp)-11-73-11, HIWORD(lp)-11-cy_button, 73, cy_button, SWP_NOZORDER);
				dwp = DeferWindowPos(dwp, m_wnd_static, NULL, 11+11, 11, RECT_CX(rcicon), RECT_CY(rcicon), SWP_NOZORDER);
				EndDeferWindowPos(dwp);
				RedrawWindow(wnd, 0, 0, RDW_UPDATENOW);
			}
			return 0;
		case WM_GETMINMAXINFO:
			{
				RECT rc, rcicon, rcw, rcwc;
				GetWindowRect(m_wnd_button, &rc);
				GetWindowRect(m_wnd_static, &rcicon);
				GetWindowRect(wnd, &rcw);
				GetClientRect(wnd, &rcwc);
				LPMINMAXINFO lpmmi = (LPMINMAXINFO)lp;
				lpmmi->ptMinTrackSize.x = 11+11+11+11+7+RECT_CX(rcicon)+50+(RECT_CX(rcw)-RECT_CX(rcwc));
				lpmmi->ptMinTrackSize.y = 11+11+11+11+RECT_CY(rcicon)+RECT_CY(rc)+(RECT_CY(rcw)-RECT_CY(rcwc));
			}
			return 0;
		case WM_CREATE:
			{
				//SetWindowLongPtr(wnd, GWL_STYLE, GetWindowLongPtr(wnd, GWL_STYLE)|WS_SYSMENU);
				//MessageBox(wnd, L"No Ipod Found", L"Error", MB_ICONQUESTION);
				{
					m_font = uCreateIconFont();
				}
				RECT rc;
				GetClientRect(wnd, &rc);
				m_wnd_edit = CreateWindowEx(0, WC_EDIT, L"", WS_CHILD|WS_VISIBLE|WS_GROUP|ES_READONLY|ES_CENTER|ES_MULTILINE|ES_AUTOVSCROLL, 11, 11, RECT_CX(rc)-11-11, RECT_CY(rc)-11-11, wnd, (HMENU)1001, core_api::get_my_instance(), NULL);
				SendMessage(m_wnd_edit, WM_SETFONT, (WPARAM)m_font.get(), MAKELPARAM(FALSE,0));
				int cy_button = uGetFontHeight(m_font)+10;
				m_wnd_button = CreateWindowEx(0, WC_BUTTON, L"Close", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON|WS_GROUP, RECT_CX(rc)-11*2-73, RECT_CY(rc)-11-cy_button, 73, cy_button, wnd, (HMENU)IDCANCEL, core_api::get_my_instance(), NULL);
				SendMessage(m_wnd_button, WM_SETFONT, (WPARAM)m_font.get(), MAKELPARAM(FALSE,0));
				m_wnd_static = CreateWindowEx(0, WC_STATIC, L"", WS_CHILD|WS_VISIBLE|WS_GROUP|SS_ICON, 0, 0, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), wnd, (HMENU)1002, core_api::get_my_instance(), 0);
			}
			return 0;
		case WM_SHOWWINDOW:
			if (wp == TRUE && lp == 0 && m_wnd_button)
				SetFocus(m_wnd_button);
			break;
		case DM_GETDEFID:
			return IDCANCEL|(DC_HASDEFID<<16);
		case WM_DESTROY:
			m_font.release();
			return 0;
		case WM_CTLCOLORSTATIC:
			SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
			SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		case WM_ERASEBKGND:
			return FALSE;
		/*case WM_NOTIFY:
			{
				LPNMHDR lpnm = (LPNMHDR)lp;
				switch (lpnm->idFrom)
				{
				case 1001:
					{
						switch (lpnm->code)
						{
						case NM_CUSTOMDRAW:
							break;
						};
					}
				};
			}
			break;*/
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC dc = BeginPaint(wnd, &ps);
				if (dc)
				{
					RECT rc_client, rc_button;
					GetClientRect(wnd, &rc_client);
					RECT rc_fill = rc_client;
					if (m_wnd_button)
					{
						GetWindowRect(m_wnd_button, &rc_button);
						rc_fill.bottom -= RECT_CY(rc_button)+11;
						rc_fill.bottom -= 11;
					}

					FillRect(dc, &rc_fill, GetSysColorBrush(COLOR_WINDOW));

					if (m_wnd_button)
					{
						rc_fill.top=rc_fill.bottom;
						rc_fill.bottom+=1;
						FillRect(dc, &rc_fill, GetSysColorBrush(COLOR_3DLIGHT));
					}
					rc_fill.top = rc_fill.bottom;
					rc_fill.bottom = rc_client.bottom;
					FillRect(dc, &rc_fill, GetSysColorBrush(COLOR_3DFACE));
					EndPaint(wnd, &ps);
				}
			}
			return 0;
		case WM_NCDESTROY:
			modeless_dialog_manager::g_remove(wnd);
			delete this;
			break;
		case WM_COMMAND:
			switch (wp)
			{
			case IDCANCEL:
				SendMessage(wnd, WM_CLOSE, NULL, NULL);
				return 0;
			}
			break;
		case WM_CLOSE:
			destroy();
			return 0;
		}
		return DefWindowProc(wnd, msg, wp, lp);
	}
	//HWND m_wnd_caption;
	HWND m_wnd_edit, m_wnd_button, m_wnd_static;
	gdi_object_t<HFONT>::ptr_t m_font;
	t_size m_flags;
};


#endif