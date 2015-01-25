#include "stdafx.h"

void t_list_view::ensure_visible(t_size index)
{
	if (index < m_items.get_count() && !is_visible(index))
	{
		RECT rc;
		get_items_rect(&rc);
		scroll(false, get_item_position(index) - (RECT_CY(rc)/2) + (get_item_height(index)/2));
	}
}
void t_list_view::scroll(bool b_sb, int val, bool b_horizontal)
{
	INT sb = b_horizontal ? SB_HORZ : SB_VERT;
	int & p_scroll_position = b_horizontal ? m_horizontal_scroll_position : m_scroll_position;

	SCROLLINFO si;
	memset(&si, 0, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	GetScrollInfo(get_wnd(), sb, &si);

	int pos = p_scroll_position;
	SCROLLINFO scroll2;
	memset(&scroll2, 0, sizeof(SCROLLINFO));
	scroll2.cbSize = sizeof(SCROLLINFO);
	scroll2.fMask = SIF_POS;
	if (b_sb)
	{

		if (val == SB_LINEDOWN && p_scroll_position < si.nMax)
			pos = p_scroll_position + m_item_height;
		else if (val == SB_LINEUP && p_scroll_position > si.nMin)
			pos = p_scroll_position - m_item_height;
		else if (val == SB_PAGEUP) 
			pos = p_scroll_position - si.nPage;
		else if (val == SB_PAGEDOWN) 
			pos = p_scroll_position + si.nPage;
		else if (val == SB_THUMBTRACK) 
			pos = si.nTrackPos;
		else if (val == SB_BOTTOM) 
			pos = si.nMax;
		else if (val == SB_TOP) 
			pos = si.nMin;
	}
	else
		pos = val;
	scroll2.nPos = pos;

	if (si.nPos != scroll2.nPos)
	{
		destroy_tooltip();
		//exit_inline_edit();
		p_scroll_position = SetScrollInfo(get_wnd(), sb, &scroll2, true);
		{
			RECT playlist;
			get_items_rect(&playlist);
			int dx = 0;
			int dy = 0;
			(b_horizontal ? dx : dy) = (si.nPos - p_scroll_position);
			ScrollWindowEx(get_wnd(), dx, dy, &playlist, &playlist, 0, 0, SW_INVALIDATE);
			RedrawWindow(get_wnd(),0,0,RDW_UPDATENOW);
			if (b_horizontal)
				reposition_header();
		}
		//RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
	}
}

void t_list_view::_update_scroll_info_vertical()
{
		RECT rc;
		get_items_rect(&rc);

		t_size old_scroll_position = m_scroll_position;
		SCROLLINFO scroll;
		memset(&scroll, 0, sizeof(SCROLLINFO));
		scroll.cbSize = sizeof(SCROLLINFO);
		scroll.fMask = SIF_RANGE|SIF_PAGE|SIF_POS;
		scroll.nMin = 0;
		t_size count = m_items.get_count();
		scroll.nMax = count ? get_item_group_bottom(count-1): 0;
		scroll.nPage = RECT_CY(rc);
		scroll.nPos = m_scroll_position;
		bool b_old_show = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_VSCROLL) != 0;;
		m_scroll_position = SetScrollInfo(get_wnd(), SB_VERT, &scroll, true);
		GetScrollInfo(get_wnd(), SB_VERT, &scroll);
		bool b_show = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_VSCROLL) != 0;//scroll.nPage < (UINT)scroll.nMax;
		//if (b_old_show != b_show)
			//BOOL ret = ShowScrollBar(get_wnd(), SB_VERT, b_show);
		if (m_scroll_position != old_scroll_position/* || b_old_show != b_show*/)
			invalidate_all(false);
}

void t_list_view::_update_scroll_info_horizontal()
{
		RECT rc;
		get_items_rect(&rc);

		t_size old_scroll_position = m_horizontal_scroll_position;
		t_size cx = get_columns_display_width();

		SCROLLINFO scroll;
		memset(&scroll, 0, sizeof(SCROLLINFO));
		scroll.cbSize = sizeof(SCROLLINFO);
		scroll.fMask = SIF_RANGE|SIF_PAGE;
		scroll.nMin = 0;
		scroll.nMax = m_autosize ? 0 : (cx?cx-1:0);
		scroll.nPage = m_autosize ? 0 : RECT_CX(rc);
		bool b_old_show = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_HSCROLL) != 0;;
		m_horizontal_scroll_position = SetScrollInfo(get_wnd(), SB_HORZ, &scroll, true);
		GetScrollInfo(get_wnd(), SB_HORZ, &scroll);
		bool b_show = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_HSCROLL) != 0;//scroll.nPage < (UINT)scroll.nMax;
		//if (b_old_show != b_show)
		{
			//RECT rc1, rc2;
			//GetClientRect(get_wnd(), &rc1);
			//BOOL ret = ShowScrollBar(get_wnd(), SB_HORZ, b_show);
			//GetClientRect(get_wnd(), &rc2);
		}
		if (m_horizontal_scroll_position != old_scroll_position/* || b_old_show != b_show*/)
			invalidate_all(false);
		if (b_old_show != b_show)
		{
			get_items_rect(&rc);
			memset(&scroll, 0, sizeof(SCROLLINFO));
			scroll.cbSize = sizeof(SCROLLINFO);
			scroll.fMask = SIF_PAGE;
			scroll.nPage = RECT_CY(rc);
			m_scroll_position = SetScrollInfo(get_wnd(), SB_VERT, &scroll, true);
		}
}

void t_list_view::update_scroll_info(bool b_update, bool b_vertical, bool b_horizontal)
{
	//god this is a bit complicated when showing h scrollbar causes need for v scrollbar (and vv)

	//bool b_scroll_shown = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_VSCROLL) != 0;
	if (b_vertical)
	{
		_update_scroll_info_vertical();
	}
	if (b_horizontal)
	{
		_update_scroll_info_horizontal();
	}
	if (b_update)
		UpdateWindow(get_wnd());
}
void t_list_view::create_timer_scroll_up()
{
	if (!m_timer_scroll_up)
	{
		SetTimer(get_wnd(), TIMER_SCROLL_UP, 10, NULL);
		m_timer_scroll_up = true;
	}
}
void t_list_view::create_timer_scroll_down()
{
	if (!m_timer_scroll_down)
	{
		SetTimer(get_wnd(), TIMER_SCROLL_DOWN, 10, NULL);
		m_timer_scroll_down = true;
	}
}
void t_list_view::destroy_timer_scroll_up()
{
	if (m_timer_scroll_up)
	{
		KillTimer(get_wnd(), TIMER_SCROLL_UP);
		m_timer_scroll_up = false;
	}
}
void t_list_view::destroy_timer_scroll_down()
{
	if (m_timer_scroll_down)
	{
		KillTimer(get_wnd(), TIMER_SCROLL_DOWN);
		m_timer_scroll_down = false;
	}
}
