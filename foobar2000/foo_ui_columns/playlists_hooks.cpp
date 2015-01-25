#include "foo_ui_columns.h"

#if 0
bool g_tab_dragging = false;
unsigned g_tab_idx = 0;
RECT g_drag_rect;
/*
static RECT pl_tooltip;
		static last_idx = -1;*/

LRESULT WINAPI TabHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		if (wp == VK_LEFT || wp == VK_RIGHT) break;
		else if (process_keydown(msg, lp, wp)) return 0;
		break;
	case WM_KEYUP:
		if (process_keydown(msg, lp, wp)) return 0;
		break;
	case WM_SYSKEYUP:
		if (process_keydown(msg, lp, wp)) return 0;
		break;
	case WM_SYSKEYDOWN:
		if (wp == VK_LEFT || wp == VK_RIGHT) break;
		else if (process_keydown(msg, lp, wp)) return 0;
		break;
	case WM_LBUTTONDOWN:
		{
			{
				TCHITTESTINFO hittest;
				hittest.pt.x = GET_X_LPARAM(lp);
				hittest.pt.y = GET_Y_LPARAM(lp);
				int idx = TabCtrl_HitTest(g_tab, &hittest);
				if (idx>=0)
				{
					static_api_ptr_t<playlist_manager> playlist_api;
					if (cfg_playlists_shift_lmb && (wp & MK_SHIFT)) playlist_api->remove_playlist_switch(idx);
					else if (cfg_drag_pl)
					{
						SetCapture(wnd);
						g_tab_dragging = true;
						g_tab_idx = idx;
						TabCtrl_GetItemRect(wnd, idx, &g_drag_rect);
					}
				}
			}
		}
		break;
	case WM_MOUSEMOVE:
		if (g_tab_dragging && (wp & MK_LBUTTON))
		{
			TCHITTESTINFO hittest;
			hittest.pt.x = GET_X_LPARAM(lp);
			hittest.pt.y = GET_Y_LPARAM(lp);
			int idx = TabCtrl_HitTest(g_tab, &hittest);
			if (idx>=0 && !PtInRect(&g_drag_rect, hittest.pt))
			{
				int cur_idx = g_tab_idx;
				static_api_ptr_t<playlist_manager> playlist_api;
				int count = playlist_api->get_playlist_count();
				
				int n = cur_idx;
				order_helper order(count);
				if (n < idx)
				{
					while (n<idx && n < count)
					{
						order.swap(n,n+1);
						n++;
					}
				}
				else if (n > idx)
				{
					while (n>idx && n > 0)
					{
						order.swap(n,n-1);
						n--;
					}
				}
				if (n != cur_idx) 
				{
					TabCtrl_GetItemRect(wnd, n, &g_drag_rect);
					playlist_api->reorder(order.get_ptr(),count);
					g_tab_idx = n;
				}
			}
		}
		else g_tab_dragging = false;

		break;
	case WM_LBUTTONUP:
		if (g_tab_dragging)
		{
			g_tab_dragging = false;
			ReleaseCapture();
		}
		break;
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONUP:
		{
			if (cfg_mclick || 1 || cfg_plm_rename) //in actuality we dont get messages when mouse not on tab here.
			{
				TCHITTESTINFO hittest;
				hittest.pt.x = GET_X_LPARAM(lp);
				hittest.pt.y = GET_Y_LPARAM(lp);
				int idx = TabCtrl_HitTest(g_tab, &hittest);
				static_api_ptr_t<playlist_manager> playlist_api;
				if (idx >= 0) 
				{
					if (cfg_mclick && msg == WM_MBUTTONUP) {playlist_api->remove_playlist_switch(idx);}
					if (cfg_plm_rename && msg == WM_LBUTTONDBLCLK) {rename_playlist(idx);}
				}
				else if (1) 
				{

						unsigned new_idx = playlist_api->create_playlist(string8("New playlist"),infinite,playlist_api->get_playlist_count());
						playlist_api->set_active_playlist(new_idx);
				}
			}
		}
		break;
	}
	return uCallWindowProc(tabproc,wnd,msg,wp,lp);
}
#endif