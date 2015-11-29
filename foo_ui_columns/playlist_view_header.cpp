#include "stdafx.h"


void playlist_view::rebuild_header(bool rebuild)
{

	if (rebuild)
	{
		int n, t = Header_GetItemCount(wnd_header);
		{
			for (n = 0; n < t; n++) Header_DeleteItem(wnd_header, 0);
		}
	}

	uHDITEM hdi;
	memset(&hdi, 0, sizeof(HDITEM));

	hdi.mask = (rebuild ? HDI_TEXT | HDI_FORMAT : 0) | HDI_WIDTH;
	hdi.fmt = HDF_LEFT | HDF_STRING;

	pfc::string8 name;

	{
		pfc::array_t<int, pfc::alloc_fast_aggressive> widths;
		get_column_widths(widths);

		const bit_array & p_mask = g_cache.active_get_columns_mask();

		int n, t = columns.get_count(), i = 0;//,tw=g_playlist_entries.get_total_width();
		for (n = 0; n < t; n++)
		{
			if (p_mask[n])
			{
				if (rebuild)
				{
					alignment align = columns[n]->align;
					hdi.fmt = HDF_STRING | (align == ALIGN_CENTRE ? HDF_CENTER : (align == ALIGN_RIGHT ? HDF_RIGHT : HDF_LEFT));
					name = columns[n]->name;
					hdi.cchTextMax = name.length();
					hdi.pszText = const_cast<char*>(name.get_ptr());
				}

				hdi.cxy = widths[i];

				uHeader_InsertItem(wnd_header, i++, &hdi, rebuild);
			}
		}
	}
}

void playlist_view::move_header(bool redraw, bool update)
{

	RECT rc_playlist, rc_header;
	GetClientRect(wnd_playlist, &rc_playlist);
	GetRelativeRect(wnd_header, wnd_playlist, &rc_header);
	int header_height = calculate_header_height();

	if (rc_header.left != 0 - horizontal_offset ||
		rc_header.top != 0 ||
		rc_header.right - rc_header.left != rc_playlist.right - rc_playlist.left + horizontal_offset ||
		rc_header.bottom - rc_header.top != header_height)
	{
		uSendMessage(wnd_header, WM_SETREDRAW, FALSE, 0);
		if (rc_header.bottom - rc_header.top != header_height)
		{
			RECT playlist, redraw;
			get_playlist_rect(&playlist);
			ScrollWindowEx(wnd_playlist, 0, (header_height - rc_header.bottom), &playlist, &playlist, 0, &redraw, 0);
			//			RedrawWindow(wnd_playlist,&redraw,0,RDW_INVALIDATE|RDW_UPDATENOW);
		}
		SetWindowPos(wnd_header, 0, 0 - horizontal_offset, 0, rc_playlist.right - rc_playlist.left + horizontal_offset, header_height, SWP_NOZORDER);
		if (cfg_nohscroll && update) rebuild_header(false);
		uSendMessage(wnd_header, WM_SETREDRAW, TRUE, 0);
		if (redraw) RedrawWindow(wnd_header, 0, 0, RDW_UPDATENOW | RDW_INVALIDATE);
	}

}

void playlist_view::create_header(bool visible)
{
	if (wnd_header) { DestroyWindow(wnd_header); wnd_header = 0; }
	if (cfg_header)
	{
		wnd_header = CreateWindowEx(0, WC_HEADER, _T("Playlist display column titles"),
			WS_CHILD | (visible ? WS_VISIBLE : 0) | HDS_HOTTRACK | HDS_DRAGDROP | HDS_HORZ | (/*nohscroll ? 0 : */HDS_FULLDRAG) | (cfg_header_hottrack ? HDS_BUTTONS : 0),
			0, 0, 0, 0, wnd_playlist, HMENU(5001), core_api::get_my_instance(), NULL);

		on_header_font_change();
		rebuild_header();
	}
	if (visible) RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
}
