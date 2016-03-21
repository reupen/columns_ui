#include "stdafx.h"

LRESULT playlist_message_window::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		playlist_view::g_load_columns();
		playlist_view::g_get_cache().enable();
		set_day_timer();
		break;
	case WM_TIMECHANGE:
		if (cfg_playlist_date) on_day_change();
		break;
	case WM_TIMER:
		on_day_change();
		break;
	case WM_SYSCOLORCHANGE:
		if (!cfg_pv_use_custom_colours)
		{
			playlist_view::g_reset_columns();
			//playlist_view::update_all_windows();
		}
		break;
	case WM_THEMECHANGED:
		break;
	case WM_DESTROY:
		if (g_font) DeleteObject(g_font); g_font = 0;
		if (g_header_font) DeleteObject(g_header_font); g_header_font = 0;
		g_to_global.release();
		g_to_global_colour.release();
		kill_day_timer();
		playlist_view::g_get_cache().disable();
		playlist_view::g_kill_columns();
		break;

	}
	return DefWindowProc(wnd, msg, wp, lp);
}

void playlist_message_window::add_ref()
{
	if (!ref_count++)
	{
		create(0);
	}
}

void playlist_message_window::release()
{
	if (!--ref_count)
	{
		destroy();
	}
}

playlist_message_window::class_data & playlist_message_window::get_class_data() const
{
	__implement_get_class_data_ex(_T("{13EFE4B7-A679-4e5c-8B98-F24A77667F78}"), _T(""), false, 0, 0, 0, 0);
}

playlist_message_window g_playlist_message_window;
