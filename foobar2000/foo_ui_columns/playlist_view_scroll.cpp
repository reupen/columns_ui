#include "stdafx.h"

void playlist_view::update_scrollbar(bool redraw)
{
	LONG_PTR old_style = uGetWindowLong(wnd_playlist, GWL_STYLE);
	bool need_move = false;

	static_api_ptr_t<playlist_manager> playlist_api;

	RECT rect;
	get_playlist_rect(&rect);

	int item_height = get_item_height();

	int items = ((rect.bottom - rect.top) / item_height);
	int total = playlist_api->activeplaylist_get_item_count();

	SCROLLINFO info;
	memset(&info, 0, sizeof(SCROLLINFO));
	info.fMask = SIF_POS;
	info.cbSize = sizeof(SCROLLINFO);
	GetScrollInfo(wnd_playlist, SB_HORZ, &info);

	horizontal_offset = info.nPos;

	GetScrollInfo(wnd_playlist, SB_VERT, &info);
	scroll_item_offset = info.nPos;

	SCROLLINFO scroll;
	memset(&scroll, 0, sizeof(SCROLLINFO));
	scroll.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	scroll.nMin = 0;
	scroll.nPos = scroll_item_offset;
	scroll.nPage = items;
	scroll.nMax = total - 1;
	scroll.cbSize = sizeof(SCROLLINFO);
	scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &scroll, true);

	//	assert(0);

	bool redraw_playlist = info.nPos != scroll_item_offset;
	int old_vert_pos = info.nPos;

	bool show_vert = (items >= total ? FALSE : TRUE);

	if (((old_style & WS_VSCROLL) != 0) != show_vert)
	{
		ShowScrollBar(wnd_playlist, SB_VERT, show_vert);
		need_move = true;
		//maybe should call get_playlist_rect again ??
	}

	get_playlist_rect(&rect);

	bool show_horiz = FALSE;

	int old_horizontal_offset = horizontal_offset;

	if (cfg_nohscroll) horizontal_offset = 0;
	{
		unsigned totalh = get_columns_total_width();
		scroll.nMin = 0;
		scroll.nPos = horizontal_offset;
		scroll.nMax = cfg_nohscroll ? 0 : totalh - 1;
		scroll.nPage = rect.right - rect.left;
		if (scroll.nPage > scroll.nMax + 1)
			scroll.nPage = scroll.nMax + 1;
		horizontal_offset = SetScrollInfo(wnd_playlist, SB_HORZ, &scroll, true);//redraw
		if (!cfg_nohscroll)
			show_horiz = (totalh > (rect.right - rect.left) ? TRUE : FALSE);
	}

	if (((old_style & WS_HSCROLL) != 0) != show_horiz)
	{
		ShowScrollBar(wnd_playlist, SB_HORZ, show_horiz);
	}

	if (old_horizontal_offset != horizontal_offset) need_move = true;


	if (wnd_header && need_move)
		move_header();

	if (redraw_playlist)
	{

		{
			RECT playlist, rc_redraw;
			get_playlist_rect(&playlist);
			ScrollWindowEx(wnd_playlist, 0, (old_vert_pos - scroll_item_offset) * get_item_height(), &playlist, &playlist, 0, &rc_redraw, 0);
			RedrawWindow(wnd_playlist, &rc_redraw, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		}

	}
}


void playlist_view::scroll(t_scroll_direction p_direction, t_scroll_type p_type, int p_value)
{
	int nBar = p_direction == scroll_vertically ? SB_VERT : SB_HORZ;

	static_api_ptr_t<playlist_manager> playlist_api;

	int & position = p_direction == scroll_vertically ? scroll_item_offset : horizontal_offset;

	int new_pos = position;
	int old_pos = position;

	SCROLLINFO si;
	memset(&si, 0, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_TRACKPOS | SIF_PAGE | SIF_RANGE;
	GetScrollInfo(wnd_playlist, nBar, &si);
	if (p_type == scroll_sb)
	{

		if (p_value == SB_LINEDOWN && old_pos < si.nMax)
			new_pos = old_pos + 1;
		if (p_value == SB_LINEUP && old_pos > si.nMin)
			new_pos = old_pos - 1;
		if (p_value == SB_PAGEUP)
			new_pos = old_pos - si.nPage;
		if (p_value == SB_PAGEDOWN)
			new_pos = old_pos + si.nPage;
		if (p_value == SB_THUMBTRACK)
			new_pos = si.nTrackPos;
		if (p_value == SB_THUMBPOSITION)
			new_pos = si.nTrackPos;
		if (p_value == SB_BOTTOM)
			new_pos = si.nMax;
		if (p_value == SB_TOP)
			new_pos = si.nMin;
	}
	else
		new_pos = old_pos + p_value;

	if (new_pos < si.nMin)
		new_pos = si.nMin;
	if (new_pos > si.nMax)
		new_pos = si.nMax;

	if (new_pos != old_pos)
	{

		//evil mouse driver send WM_VSCROLL not WM_MOUSEWHEEL!!!!
		if (g_tooltip) { DestroyWindow(g_tooltip); g_tooltip = 0; last_idx = -1; last_column = -1; }

		SCROLLINFO scroll2;
		memset(&scroll2, 0, sizeof(SCROLLINFO));
		scroll2.fMask = SIF_POS;
		scroll2.nPos = new_pos;
		scroll2.cbSize = sizeof(SCROLLINFO);

		position = SetScrollInfo(wnd_playlist, nBar, &scroll2, true);

		if (p_direction == scroll_horizontally)
		{
			move_header(true, false);
		}

		if (drawing_enabled)
		{
			RECT playlist;
			get_playlist_rect(&playlist);
			int dx = (p_direction == scroll_horizontally) ? (old_pos - position) : 0;
			int dy = (p_direction == scroll_vertically) ? (old_pos - position) * get_item_height() : 0;
			ScrollWindowEx(wnd_playlist, dx, dy, &playlist, &playlist, 0, 0, SW_INVALIDATE);
			RedrawWindow(wnd_playlist, 0, 0, RDW_UPDATENOW);
		}
	}
}


