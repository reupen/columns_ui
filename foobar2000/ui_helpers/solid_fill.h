#ifndef _COLUMNS_SOLID_FILL_H_
#define _COLUMNS_SOLID_FILL_H_

class window_solid_fill : public ui_helpers::container_window
{
public:
	void set_fill_colour(COLORREF value)
	{
		m_fill_colour = value;
		if (get_wnd())
			redraw();
	}

	window_solid_fill()
		: m_fill_colour(NULL)
	{};

private:
	void redraw()
	{
		RedrawWindow(get_wnd(), 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
	}
	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data_ex(_T("columns_ui_solid_fill"), _T(""), false, 0, WS_CHILD|WS_CLIPCHILDREN, WS_EX_CONTROLPARENT|WS_EX_STATICEDGE, 0);
	}
	virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch (msg)
		{
		case WM_ERASEBKGND:
			return FALSE;
		case WM_PAINT:
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(wnd, &ps);
			gdi_object_t<HBRUSH>::ptr_t p_brush = CreateSolidBrush(m_fill_colour);
			FillRect(dc, &ps.rcPaint, p_brush);
			p_brush.release();
			EndPaint(wnd, &ps);
			return 0;
		}
		return DefWindowProc(wnd, msg, wp, lp);
	};

	COLORREF m_fill_colour;
};

class window_fill : public ui_helpers::container_window
{
public:
	enum t_mode
	{
		mode_solid_fill,
		mode_theme_fill,
		mode_theme_solid_fill,
	};

	void set_fill_colour(COLORREF value)
	{
		m_mode = mode_solid_fill;
		m_fill_colour = value;
		if (get_wnd())
			redraw();
	}
	void set_fill_themed(const WCHAR * pclass, int part, int state, COLORREF fallback)
	{
		m_mode = mode_theme_fill;
		m_theme_state = state;
		m_theme_part = part;
		m_fill_colour = fallback;
		m_theme_class = pclass;
		if (m_theme)
			CloseThemeData(m_theme);
		m_theme=NULL;
		if (get_wnd())
		{
			m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(get_wnd(), m_theme_class) : NULL;
			redraw();
		}
	}

	void set_fill_themed_colour(const WCHAR * pclass, int index)
	{
		m_mode = mode_theme_solid_fill;
		m_theme_class = pclass;
		m_theme_colour_index = index;
		if (m_theme)
			CloseThemeData(m_theme);
		m_theme=NULL;
		if (get_wnd())
		{
			m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(get_wnd(), m_theme_class) : NULL;
			redraw();
		}
	}

	window_fill()
		: m_fill_colour(NULL), m_mode(mode_solid_fill), m_theme(NULL), m_theme_state(NULL), m_theme_part(NULL), m_theme_prop(NULL),
		m_theme_colour_index(NULL)
	{};

private:
	void redraw()
	{
		RedrawWindow(get_wnd(), 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
		//RedrawWindow(get_wnd(), 0, 0, RDW_INVALIDATE|RDW_ERASE);
	}
	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data_ex(_T("columns_ui_fill"), _T(""), false, 0, WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE, WS_EX_CONTROLPARENT|WS_EX_STATICEDGE, 0);
	}
	virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch (msg)
		{
		case WM_CREATE:
			{
				if (m_mode == mode_theme_fill || m_mode == mode_theme_solid_fill)
					m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, m_theme_class) : NULL;
				SetWindowTheme(wnd, L"Explorer", NULL);
			}
			break;
		case WM_DESTROY:
			{
				if (m_theme) CloseThemeData(m_theme);
				m_theme = NULL;
			}
			break;
		case WM_THEMECHANGED:
			{
				if (m_theme) CloseThemeData(m_theme);
				m_theme=NULL;
				if (m_mode == mode_theme_fill || m_mode == mode_theme_solid_fill)
					m_theme = (IsThemeActive() && IsAppThemed()) ? OpenThemeData(wnd, m_theme_class) : 0;
				redraw();
			}
			break;
		case WM_ENABLE:
			redraw();
			break;
		case WM_ERASEBKGND:
			return FALSE;
		case WM_PAINT:
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(wnd, &ps);
			gdi_object_t<HBRUSH>::ptr_t p_brush;
			RECT rc;
			GetClientRect(wnd, &rc);

			if (!IsWindowEnabled(wnd))
			{
				FillRect(dc, &ps.rcPaint, GetSysColorBrush(COLOR_BTNFACE));
			}
			else if (m_mode == mode_theme_fill && m_theme && IsThemePartDefined(m_theme, m_theme_part, m_theme_state))
			{
				if (IsThemeBackgroundPartiallyTransparent(m_theme, m_theme_part, m_theme_state))
					DrawThemeParentBackground(wnd, dc, &rc);
				if (FAILED(DrawThemeBackground(m_theme, dc, m_theme_part, m_theme_state, &rc, NULL)))
				{
					p_brush = CreateSolidBrush(m_fill_colour);
					FillRect(dc, &ps.rcPaint, p_brush);
				}
			}
			else if (m_mode == mode_theme_solid_fill && m_theme)
			{
				COLORREF cr_fill = GetThemeSysColor(m_theme, m_theme_colour_index);
				p_brush = CreateSolidBrush(cr_fill);
				FillRect(dc, &ps.rcPaint, p_brush);
			}
			else
			{
				p_brush = CreateSolidBrush(m_fill_colour);
				FillRect(dc, &ps.rcPaint, p_brush);
			}
			p_brush.release();
			EndPaint(wnd, &ps);
			return 0;
		}
		return DefWindowProc(wnd, msg, wp, lp);
	};

	t_mode m_mode;
	pfc::string_simple_t<WCHAR> m_theme_class;
	COLORREF m_fill_colour;
	HTHEME m_theme;
	int m_theme_part, m_theme_state, m_theme_prop, m_theme_colour_index;
};

class window_transparent_fill : public ui_helpers::container_window
{
public:
	void set_fill_colour(COLORREF value)
	{
		m_fill_colour = value;
		if (get_wnd())
			redraw();
	}

	window_transparent_fill()
		: m_fill_colour(NULL)
	{};

private:
	void redraw()
	{
		RedrawWindow(get_wnd(), 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
	}
	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data_ex(_T("columns_ui_transparent_fill"), _T(""), false, 0, WS_POPUP|WS_CLIPSIBLINGS| WS_CLIPCHILDREN, /*WS_EX_TOPMOST|*/WS_EX_LAYERED, 0);
	}
	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	COLORREF m_fill_colour;
};

#endif //_COLUMNS_SOLID_FILL_H_