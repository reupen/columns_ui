#include "foo_ui_columns.h"

/* no range checks here !*/
void playlist_view::process_keydown(int offset, bool alt_down, bool prevent_redrawing, bool repeat)
{
	static_api_ptr_t<playlist_manager> playlist_api;

	int focus = playlist_api->activeplaylist_get_focus_item();
	int count = playlist_api->activeplaylist_get_item_count();

	//	if (focus < 0) focus =0;
	//	if (focus >= count) focus = count-1;

	//	int alt_offset = offset;

	if ((focus + offset) < 0) offset -= (focus + offset);
	if ((focus + offset) >= count) offset = (count - 1 - focus);

	bool focus_sel = playlist_api->activeplaylist_is_item_selected(focus);
	if (prevent_redrawing)
		uSendMessage(wnd_playlist, WM_SETREDRAW, FALSE, 0);

	if ((GetKeyState(VK_SHIFT) & KF_UP) && (GetKeyState(VK_CONTROL) & KF_UP))
	{
		if (!repeat) playlist_api->activeplaylist_undo_backup();
		playlist_api->activeplaylist_move_selection(offset);
	}
	else if ((GetKeyState(VK_CONTROL) & KF_UP))
		playlist_api->activeplaylist_set_focus_item(focus + offset);
	else if (GetKeyState(VK_SHIFT) & KF_UP)
	{
		set_sel_range(cfg_alternative_sel ? focus : g_shift_item_start, focus + offset, (cfg_alternative_sel != 0), (cfg_alternative_sel ? !focus_sel : false));
		playlist_api->activeplaylist_set_focus_item(focus + offset);
	}
	else
	{
		//		console::info(pfc::string_printf("%i",focus+offset));
		set_sel_single(focus + offset, false, true, (GetKeyState(VK_SHIFT) & KF_UP) ? false : true);
	}

	if (prevent_redrawing)
	{
		uSendMessage(wnd_playlist, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

bool playlist_view::process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist, bool keyb)
{
	static_api_ptr_t<keyboard_shortcut_manager> keyboard_api;

	if (msg == WM_SYSKEYDOWN)
	{
		if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp))
		{
			return true;
		}
	}
	else if (msg == WM_KEYDOWN)
	{
		if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp))
		{
			return true;
		}
		if (wp == VK_TAB)
		{
			uie::window::g_on_tab(GetFocus());
#if 0
			HWND wnd_focus = GetFocus();
			HWND wnd_temp = GetParent(wnd_focus);

			while (GetWindowLong(wnd_temp, GWL_EXSTYLE) & WS_EX_CONTROLPARENT)
			{
				wnd_temp = GetParent(wnd_temp);
			}

			HWND wnd_next = GetNextDlgTabItem(wnd_temp, wnd_focus, (GetAsyncKeyState(VK_SHIFT) & KF_UP) ? TRUE : FALSE);

			if (wnd_next && wnd_next != wnd_focus) SetFocus(wnd_next);
#endif

		}
	}
	return false;
}
