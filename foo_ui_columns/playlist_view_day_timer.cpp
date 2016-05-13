#include "stdafx.h"
#include "playlist_view.h"

static unsigned g_day_timer;

VOID CALLBACK on_day_change()
{
	//	static_api_ptr_t<playlist_manager> playlist_api;
	//	unsigned count=playlist_api->get_playlist_count(),n;
	//	for (n=0;n<count;n++)
	playlist_view::g_get_cache().flush_all(false);

	unsigned m, pcount = playlist_view::list_playlist.get_count();
	for (m = 0; m < pcount; m++)
	{
		playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
		if (p_playlist->wnd_playlist)
			RedrawWindow(p_playlist->wnd_playlist, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	set_day_timer();
}


void kill_day_timer()
{
	HWND wnd = g_playlist_message_window.get_wnd();
	if (g_day_timer && wnd) { KillTimer(wnd, DAY_TIMER_ID); g_day_timer = 0; }
}

void set_day_timer()
{
	HWND wnd = g_playlist_message_window.get_wnd();

	if (g_day_timer) { KillTimer(wnd, DAY_TIMER_ID); g_day_timer = 0; }

	if (cfg_playlist_date && g_playlist_message_window.get_wnd())
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		//	unsigned ms=st.wMilliseconds + st.wMinute*60*1000 + st.wSecond*1000 + st.wHour*60*60*1000;
		unsigned ms =/*24**/60 * 60 * 1000 - (st.wMilliseconds + ((/*st.wHour*60 + */st.wMinute) * 60 + st.wSecond) * 1000);

		SetTimer(wnd, DAY_TIMER_ID, ms, nullptr);
	}
}
