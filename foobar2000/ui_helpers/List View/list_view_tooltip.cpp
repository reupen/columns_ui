#include "stdafx.h"

void t_list_view::set_show_tooltips (bool b_val) 
{
	m_show_tooltips = b_val;
}

void t_list_view::set_limit_tooltips_to_clipped_items (bool b_val)
{
	m_limit_tooltips_to_clipped_items = b_val;
}

void t_list_view::create_tooltip(/*t_size index, t_size column, */const char * str)
{
	destroy_tooltip();

	bool b_comctl_6 = true;

	m_wnd_tooltip = CreateWindowEx(b_comctl_6?WS_EX_TRANSPARENT:0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_NOPREFIX ,		
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, get_wnd(), 0, core_api::get_my_instance(), NULL);

	SendMessage(m_wnd_tooltip, WM_SETFONT, (WPARAM)m_font.get(), MAKELPARAM(FALSE,0));

	RECT rect;
	GetClientRect (get_wnd(), &rect);

	uTOOLINFO ti;
	memset(&ti,0,sizeof(ti));

	ti.cbSize = sizeof(uTOOLINFO);
	ti.uFlags = TTF_TRANSPARENT|TTF_SUBCLASS;
	ti.hwnd = get_wnd();
	ti.hinst = core_api::get_my_instance();
	ti.uId = IDC_TOOLTIP;
	ti.lpszText = const_cast<char *>(str);
	ti.rect = rect;

	uToolTip_AddTool(m_wnd_tooltip, &ti);
}
void t_list_view::destroy_tooltip()
{
	if (m_wnd_tooltip)
	{
		DestroyWindow(m_wnd_tooltip);
		m_wnd_tooltip = NULL;
	}
	m_tooltip_last_index = -1;
	m_tooltip_last_column = -1;
}
