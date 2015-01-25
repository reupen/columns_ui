#include "stdafx.h"

bool t_list_view::on_wm_keydown(WPARAM wp, LPARAM lp, LRESULT & ret, bool & b_processed)
{
	b_processed = false;
	ret = 0;
	if (notify_on_keyboard_keydown_filter(WM_KEYDOWN, wp, lp, b_processed))
		return true;

	if (!b_processed)
	{
		SendMessage(get_wnd(), WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
		if (wp == VK_TAB)
		{
			ui_extension::window::g_on_tab(get_wnd());
			b_processed = true;
		}
		else if (wp == VK_HOME ||wp == VK_DOWN || wp == VK_END || wp == VK_PRIOR || wp == VK_NEXT ||  wp == VK_UP) 
		{
			t_size focus = get_focus_item();
			t_size total = m_items.get_count();

			if (total)
			{
				if (focus == pfc_infinite)
					focus = 0;
				else if (focus >= total)
					focus = total-1;

				SCROLLINFO si;
				memset(&si, 0, sizeof(si));
				si.cbSize = sizeof(si);

				si.fMask = SIF_RANGE|SIF_PAGE|SIF_POS;
				GetScrollInfo(get_wnd(), SB_VERT, &si);

				int old_scroll_position = si.nPos;

				int offset=0;
				int scroll = m_scroll_position;
				t_size last_visible = get_last_viewable_item(); 

				if (wp == VK_HOME)
					scroll = 0;
				else if (wp == VK_PRIOR && (focus == get_next_item(m_scroll_position)))
					scroll -= si.nPage;
				else if (wp == VK_UP)
				{
					if (focus < total) // != pfc_infinite
					{
						if (focus <= get_next_item(m_scroll_position)) 
							scroll = focus ? get_item_position(focus-1) : 0;
						else if (focus > last_visible) 
							scroll = get_item_position(focus) + si.nPage - m_item_height;
					}
				}
				else if (wp == VK_DOWN)
				{
					if (focus < total) // != pfc_infinite
					{
						if (focus + 1 < total)
						{
							if (focus < get_previous_item(m_scroll_position)) 
								scroll = get_item_position(focus + 1)  + m_item_height;
							else if (focus >= last_visible) 
								scroll = get_item_position(focus +1) - si.nPage + m_item_height;
						}
						else
							scroll = si.nMax;
					}
				}
				else if (wp == VK_END) 
					scroll = si.nMax;
				else if (wp == VK_NEXT && last_visible == focus) 
					scroll += si.nPage;

				si.nPos = scroll;
				si.fMask = SIF_POS;
				m_scroll_position = SetScrollInfo(get_wnd(), SB_VERT, &si, true);

#if 0
				{
					RECT playlist;
					get_items_rect(&playlist);
					int dx = 0;
					int dy = 0;
					dy = (old_scroll_position - m_scroll_position);
					ScrollWindowEx(get_wnd(), dx, dy, &playlist, &playlist, 0, 0, SW_INVALIDATE);
				}
#endif

				if (wp == VK_HOME)
					offset = 0-focus;
				else if (wp == VK_PRIOR) 
					offset = get_next_item(m_scroll_position)-focus;
				else if (wp == VK_END) 
					offset = total-focus-1;
				else if (wp == VK_NEXT)
					offset = get_last_viewable_item()-focus;
				else if (wp == VK_DOWN)
					offset = 1;
				else if (wp == VK_UP)
					offset = -1;

				bool b_redraw = disable_redrawing();

				//if (offset) 
					process_keydown(offset,((HIWORD(lp) & KF_ALTDOWN) != 0), (HIWORD(lp) & KF_REPEAT) != 0);

				if (b_redraw)
					enable_redrawing();
			}

			b_processed =  true;
		}
		else if (wp == VK_SPACE)
		{
			bool ctrl_down = 0!=(GetKeyState(VK_CONTROL) & KF_UP);
			if (ctrl_down)
			{
				t_size focus = get_focus_item();
				if (focus!=pfc_infinite)
					set_item_selected(focus, !get_item_selected(focus));
				b_processed = true;
			}
		}
		else if (wp == VK_RETURN)
		{
			bool ctrl_down = 0!=(GetKeyState(VK_CONTROL) & KF_UP);
			t_size focus = get_focus_item();
			if (focus!=pfc_infinite)
				execute_default_action(focus, pfc_infinite, true, ctrl_down);
			b_processed =  true;
		}
		else if (wp == VK_SHIFT)
		{
			if (!(HIWORD(lp) & KF_REPEAT))
			{
				t_size focus = get_focus_item();
				m_shift_start = focus != pfc_infinite ? focus : 0;
			}
		}
		else if (wp == VK_F2)
		{
			activate_inline_editing();
			b_processed = true;
		}
		else if (wp == VK_DELETE)
		{
			b_processed = notify_on_keyboard_keydown_remove();
		}
		else if (wp == VK_F3)
		{
			b_processed = notify_on_keyboard_keydown_search();
		}
	}
	return b_processed;
}
