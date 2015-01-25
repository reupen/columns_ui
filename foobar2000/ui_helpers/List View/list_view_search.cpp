#include "stdafx.h"

bool t_list_view::is_search_box_open() {return m_search_editbox != NULL;}
void t_list_view::focus_search_box() {if (m_search_editbox) SetFocus(m_search_editbox);}

void t_list_view::show_search_box(const char * label, bool b_focus)
{
	if (!m_search_editbox)
	{
		m_search_editbox = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"" /*pfc::stringcvt::string_os_from_utf8("").get_ptr()*/, WS_CHILD|WS_CLIPSIBLINGS|ES_LEFT|
			WS_VISIBLE|WS_CLIPCHILDREN|ES_AUTOHSCROLL|WS_TABSTOP, 0, 
			0, 0, 0, get_wnd(), HMENU(668), core_api::get_my_instance(), 0);

		m_search_label = label;

		SetWindowLongPtr(m_search_editbox,GWL_USERDATA,(LPARAM)(this));
		m_proc_search_edit = (WNDPROC)SetWindowLongPtr(m_search_editbox,GWL_WNDPROC,(LPARAM)(g_on_search_edit_message));
		//SetWindowPos(m_wnd_inline_edit,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		SendMessage(m_search_editbox, WM_SETFONT, (WPARAM)m_font.get(), MAKELONG(TRUE,0));
		SetWindowTheme(m_search_editbox, L"SearchBoxEdit", NULL);
		
		//m_search_box_theme = OpenThemeData(m_search_editbox, L"Edit");
	/*	COLORREF cr = NULL;
		GetThemeColor(m_theme, NULL, NULL, TMT_EDGEHIGHLIGHTCOLOR, &cr);
		m_search_box_hot_brush = CreateSolidBrush(cr);
		BYTE b = LOBYTE(HIWORD(cr)), g = HIBYTE(LOWORD(cr)), r = LOBYTE(LOWORD(cr));
		r-=r/20;
		g-=g/20;
		b-=b/20;
		//r = pfc::rint32(r*0.9);
		//g = pfc::rint32(g*0.9);
		//b = pfc::rint32(b*0.9);
		cr = RGB(r, g, b);
		m_search_box_nofocus_brush = CreateSolidBrush(cr);*/
		//SendMessage(m_search_editbox, EM_SETMARGINS, EC_LEFTMARGIN, 0);
		Edit_SetCueBannerText(m_search_editbox, uT(label));
		if (b_focus)
			SetFocus(m_search_editbox);
		on_size();

#if 0
		HTHEME thm = OpenThemeData(m_search_editbox, L"Edit");
		t_size i;
		for (i=TMT_RESERVEDLOW; i<TMT_RESERVEDHIGH; i++)
		{
			COLORREF cr = 0;
			if (SUCCEEDED(GetThemeColor(thm, EP_BACKGROUND, EBS_NORMAL, i, &cr)) && cr)
				console::formatter() << i << " " << (unsigned)GetRValue(cr) << " " << (unsigned)GetGValue(cr) << " " << (unsigned)GetBValue(cr);
			cr = 0;
			if (SUCCEEDED(GetThemeColor(thm, EP_BACKGROUND, EBS_HOT, i, &cr)) && cr)
				console::formatter() << i << " " << (unsigned)GetRValue(cr) << " " << (unsigned)GetGValue(cr) << " " << (unsigned)GetBValue(cr);
			cr = 0;
			if (SUCCEEDED(GetThemeColor(thm, EP_BACKGROUND, EBS_FOCUSED, i, &cr)) && cr)
				console::formatter() << i << " " << (unsigned)GetRValue(cr) << " " << (unsigned)GetGValue(cr) << " " << (unsigned)GetBValue(cr);
		}
		CloseThemeData(thm);
#endif
	}
}
void t_list_view::close_search_box(bool b_notify)
{
	if (m_search_editbox)
	{
		DestroyWindow(m_search_editbox);
		m_search_editbox = NULL;
	}
	/*if (m_search_box_theme)
	{
		CloseThemeData(m_search_box_theme);
		m_search_box_theme = NULL;
	}*/
	m_search_label.reset();
	on_size();
	if (b_notify)
		notify_on_search_box_close();
}

void t_list_view::get_search_box_rect(LPRECT rc)
{
	if (m_search_editbox)
	{
		GetRelativeRect(m_search_editbox, get_wnd(), rc);
		//rc->top -= 2;
		//rc->bottom += 2;
	}
	else
	{
		GetClientRect(get_wnd(), rc);
		rc->top = rc->bottom;
	}
}
unsigned t_list_view::get_search_box_height()
{
	unsigned ret = 0;
	if (m_search_editbox)
	{
		RECT rc;
		get_search_box_rect(&rc);
		ret = RECT_CY(rc);
	}
	return ret;
}

LRESULT WINAPI t_list_view::g_on_search_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	t_list_view * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<t_list_view*>(GetWindowLongPtr(wnd,GWL_USERDATA));
	
	rv = p_this ? p_this->on_search_edit_message(wnd,msg,wp,lp) : DefWindowProc(wnd, msg, wp, lp);;
	
	return rv;
}

LRESULT t_list_view::on_search_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_KILLFOCUS:
		break;
	case WM_SETFOCUS:
		break;
	case WM_GETDLGCODE:
		//return CallWindowProc(m_proc_search_edit,wnd,msg,wp,lp)|DLGC_WANTALLKEYS;
		break;
	case WM_KEYDOWN:
		switch (wp)
		{
		case VK_TAB:
			{
				uie::window::g_on_tab(wnd);
			}
			//return 0;
			break;
		case VK_ESCAPE:
			close_search_box();
			//notify_on_search_box_close();
			return 0;
		case VK_RETURN:
			return 0;
		}
		break;
	case WM_MOUSEMOVE:
		{
			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			__search_box_update_hot_status(pt);
		}
		break;
#if 0
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(wnd, &ps);
			RECT rc;
			GetClientRect(m_search_editbox, &rc);
			HTHEME thm = OpenThemeData(m_search_editbox, L"SearchBox");//SearchBox
			t_size step = rc.right/10;
			t_size i;
			for (i=0; i<10; i++)
			{
				rc.left = i*step;
				rc.right = (i+1)*step;
				FillRect(dc, &rc, GetSysColorBrush(COLOR_3DLIGHT));
				DrawThemeBackground(thm, (HDC)dc, 1, i, &rc, NULL);
			}
			CloseThemeData(thm);
			EndPaint(wnd, &ps);
		}
		return 0;
#endif
	}
	return CallWindowProc(m_proc_search_edit,wnd,msg,wp,lp);
}

void t_list_view::__search_box_update_hot_status(const POINT & pt)
{
	POINT pts = pt;
	MapWindowPoints(get_wnd(), HWND_DESKTOP, &pts, 1);

	bool b_in_wnd = WindowFromPoint(pts) == m_search_editbox;
	bool b_focused = GetFocus() == m_search_editbox;
	bool b_new_hot = b_in_wnd && !b_focused;

	if (m_search_box_hot != b_new_hot)
	{
		m_search_box_hot = b_new_hot;
		if (m_search_box_hot)
			SetCapture(m_search_editbox);
		else if (GetCapture() == m_search_editbox)
			ReleaseCapture();
		RedrawWindow(m_search_editbox, NULL, NULL, RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW|RDW_ERASENOW);
	}
}
