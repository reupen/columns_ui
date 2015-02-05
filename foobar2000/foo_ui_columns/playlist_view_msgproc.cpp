#include "foo_ui_columns.h"


LRESULT playlist_view::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_NCCREATE:
		wnd_playlist = wnd;
		initialised = true;
		list_playlist.add_item(this);
		g_playlist_message_window.add_ref();
		break;
	case WM_CREATE:
	{
		pfc::com_ptr_t<IDropTarget_playlist> IDT_playlist = new IDropTarget_playlist(this);
		RegisterDragDrop(wnd, IDT_playlist.get_ptr());
		if (true)
		{
			m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"ListView") : NULL;
			SetWindowTheme(wnd, L"Explorer", NULL);
		}
		m_always_show_focus = config_object::g_get_data_bool_simple(standard_config_objects::bool_playback_follows_cursor, false);
		on_playlist_font_change();
		create_header(true);
		drawing_enabled = true;
		m_cache.initialise();
	}
	return 0;
	case WM_DESTROY:
		m_edit_save = false;
		exit_inline_edit();
		m_cache.deinitialise();
		RevokeDragDrop(wnd);
		SendMessage(wnd, WM_SETFONT, 0, 0);
		SendMessage(wnd_header, WM_SETFONT, 0, 0);
		{
			if (m_theme) CloseThemeData(m_theme);
			m_theme = NULL;
		}
		m_selection_holder.release();
		break;
	case WM_NCDESTROY:
		g_playlist_message_window.release();
		wnd_playlist = 0;
		initialised = false;
		list_playlist.remove_item(this);
		m_shown = false;
		//		if (!list_playlist.get_count())
		//		{
		//			g_playlist_entries.rebuild_all();
		//		}
		break;
	case WM_THEMECHANGED:
	{
		if (m_theme) CloseThemeData(m_theme);
		m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"ListView") : 0;
	}
	break;
	case WM_SHOWWINDOW:
		if (wp == TRUE && lp == 0 && !m_shown)
		{
			static_api_ptr_t<playlist_manager> playlist_api;
			ensure_visible(playlist_api->activeplaylist_get_focus_item());
			m_shown = true;
		}
		break;
	case WM_WINDOWPOSCHANGED:
	{
		LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
		if (!(lpwp->flags & SWP_NOSIZE))
		{
			on_size(lpwp->cx, lpwp->cy);
		}
	}
	break;
	case WM_ERASEBKGND:
		return TRUE;
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc_paint = BeginPaint(wnd, &ps);

		RECT rc_update, rc_playlist;
		get_playlist_rect(&rc_playlist);


		rc_update = ps.rcPaint;
		if (rc_update.top<rc_playlist.top) rc_update.top = rc_playlist.top;
		if (rc_update.bottom >= rc_update.top)
		{

			int item_height = get_item_height();

			int start_item = (rc_update.top - rc_playlist.top) / item_height;
			int end_item = (rc_update.bottom - rc_playlist.top) / item_height;

			if (((end_item - start_item) + 1)*item_height < rc_update.bottom - rc_update.top) end_item++;
			{
				draw_items(dc_paint, start_item, 1 + (end_item - start_item));
			}
		}
		EndPaint(wnd, &ps);
	}
	return 0;
	case WM_SETREDRAW:
		drawing_enabled = (wp != 0);
		return 0;
	case WM_MOUSEACTIVATE:
		if (GetFocus() != wnd)
			m_no_next_edit = true;
		return MA_ACTIVATE;
	case WM_UPDATEUISTATE:
		RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE);
		break;
	case WM_KILLFOCUS:
		RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		m_selection_holder.release();
		break;
	case WM_SETFOCUS:
		//if (msg == WM_SETFOCUS && (HWND)wp != wnd)
		//m_no_next_edit = true;
		RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
		m_selection_holder->set_playlist_selection_tracking();
		break;
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
	{
		static_api_ptr_t<playlist_manager> playlist_api;
		uie::window_ptr p_this = this;
		//DWORD vk_slash = VkKeyScan('/');
		if (wp == VK_CONTROL) g_drag_lmb = true;
		if (m_prevent_wm_char_processing = process_keydown(msg, lp, wp, true)) return 0;
		else
		{
			SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
			if (wp == VK_HOME || wp == VK_DOWN || wp == VK_END || wp == VK_PRIOR || wp == VK_NEXT || wp == VK_UP)
			{
				int focus = playlist_api->activeplaylist_get_focus_item();
				int total = playlist_api->activeplaylist_get_item_count();

				if ((wp == VK_HOME || wp == VK_PRIOR || wp == VK_UP))
				{
					//	if (focus == 0) return 0;
				}
				if ((wp == VK_END || wp == VK_NEXT || wp == VK_DOWN))
				{
					//	if (focus == total - 1) return 0;
				}

				SCROLLINFO si;
				memset(&si, 0, sizeof(si));
				si.cbSize = sizeof(si);

				si.fMask = SIF_PAGE | SIF_POS;
				GetScrollInfo(wnd_playlist, SB_VERT, &si);

				int offset = 0;
				int scroll = scroll_item_offset;

				if (wp == VK_HOME)
					scroll = 0;
				else if (wp == VK_PRIOR && focus == scroll_item_offset)
					scroll -= si.nPage;
				else if (wp == VK_UP)
				{
					if (focus <= scroll_item_offset)
						scroll = focus - 1;
					else if (focus > si.nPos + si.nPage - 1)
						scroll = focus - 1 - si.nPage + 1;
				}
				else if (wp == VK_DOWN)
				{
					if (focus < scroll_item_offset)
						scroll = focus + 1;
					else if (focus >= si.nPos + si.nPage - 1)
						scroll = focus + 1 - si.nPage + 1;
				}
				else if (wp == VK_END)
					scroll = total - 1;
				else if (wp == VK_NEXT && focus == si.nPos + si.nPage - 1)
					scroll += si.nPage;

				drawing_enabled = false;

				si.nPos = scroll;
				si.fMask = SIF_POS;
				scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &si, true);

				if (wp == VK_HOME)
					offset = 0 - focus;
				else if (wp == VK_PRIOR)
					offset = scroll_item_offset - focus;
				else if (wp == VK_END)
					offset = total - focus - 1;
				else if (wp == VK_NEXT)
					offset = get_last_viewable_item() - focus;
				else if (wp == VK_DOWN)
					offset = 1;
				else if (wp == VK_UP)
					offset = -1;


				//if (offset) 
				process_keydown(offset, ((HIWORD(lp) & KF_ALTDOWN) != 0), drawing_enabled, (HIWORD(lp) & KF_REPEAT) != 0);
				drawing_enabled = true;

				RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

				return 0;
			}
			else if (wp == VK_SPACE)
			{
				int focus = playlist_api->activeplaylist_get_focus_item();
				set_sel_single(focus, true, false, false);
				return 0;
			}
			else if (wp == VK_RETURN)
			{
				bool ctrl_down = 0 != (GetKeyState(VK_CONTROL) & KF_UP);
				int focus = playlist_api->activeplaylist_get_focus_item();
				unsigned active = playlist_api->get_active_playlist();
				if (ctrl_down)
				{
					if (active != -1 && focus != -1)
						playlist_api->queue_add_item_playlist(active, focus);
				}
				else
				{
					//					playlist_api->set_playing_playlist(active);
					unsigned focus = playlist_api->activeplaylist_get_focus_item();
					//unsigned active = playlist_api->get_active_playlist();
					//playlist_api->playlist_set_playback_cursor(active, focus);
					playlist_api->activeplaylist_execute_default_action(focus);
					//static_api_ptr_t<play_control>()->play_start(play_control::track_command_settrack);
				}
				return 0;
			}
			else if (wp == VK_SHIFT)
			{
				if (!(HIWORD(lp) & KF_REPEAT)) g_shift_item_start = playlist_api->activeplaylist_get_focus_item();
			}
			else if (wp == VK_F2)
			{
				unsigned count = g_get_cache().active_column_get_active_count();
				if (count)
				{
					unsigned focus = playlist_api->activeplaylist_get_focus_item();
					if (focus != pfc_infinite)
					{
						t_size i, pcount = playlist_api->activeplaylist_get_item_count();
						bit_array_bittable sel(pcount);
						playlist_api->activeplaylist_get_selection_mask(sel);

						pfc::list_t<t_size> indices;
						indices.prealloc(32);
						for (i = 0; i<pcount; i++)
							if (sel[i]) indices.add_item(i);

						/*t_size start = focus, end = focus;

						if (sel[start] && pcount)
						{
						while (start>0 && sel[start-1]) start--;
						while (end<pcount-1 && sel[end+1]) end++;
						}*/

						unsigned count = g_get_cache().active_column_get_active_count();
						unsigned column;
						for (column = 0; column<count; column++)
						{
							if (!g_get_columns()[g_get_cache().active_column_active_to_actual(column)]->edit_field.is_empty())
							{
								//create_inline_edit_v2(start, end-start+1, column);
								create_inline_edit_v2(indices, column);
								break;
							}
						}
					}
				}
			}
			else if (wp == VK_DELETE)
			{
				playlist_api->activeplaylist_undo_backup();
				playlist_api->activeplaylist_remove_selection();
			}
			else if (wp == VK_F3)
			{
				standard_commands::main_playlist_search();
			}
			/*else if (vk_slash != -1 && wp == LOWORD(vk_slash))
			{
			HWND wnd_search = m_searcher.create(wnd);
			on_size();
			ShowWindow(wnd_search, SW_SHOWNORMAL);
			;
			}*/
		}
	}
	break;
	case WM_CHAR:
		if (!m_prevent_wm_char_processing)
		{
			//if (!(HIWORD(lp) & KF_REPEAT))
			{
				if ((GetKeyState(VK_CONTROL) & KF_UP))
				{
					static_api_ptr_t<playlist_manager> playlist_api;
					if (wp == 1) //Ctrl-A
					{
						playlist_api->activeplaylist_set_selection(bit_array_true(), bit_array_true());
						return 0;
					}
					else if (wp == 26) //Ctrl-Z
					{
						playlist_api->activeplaylist_undo_restore();
						return 0;
					}
					else if (wp == 25) //Ctrl-Y
					{
						playlist_api->activeplaylist_redo_restore();
						return 0;
					}
					else if (wp == 24) //Ctrl-X
					{
						playlist_utils::cut();
						return 0;
					}
					else if (wp == 3) //Ctrl-C
					{
						playlist_utils::copy();
						return 0;
					}
					else if (wp == 6) //Ctrl-F
					{
						standard_commands::main_playlist_search();
						return 0;
					}
					else if (wp == 22) //Ctrl-V
					{
						playlist_utils::paste(wnd);
						return 0;
					}
				}
			}
		}
		break;
	case WM_KEYUP:
		if (process_keydown(msg, lp, wp, true)) return 0;
		break;
	case WM_SYSKEYUP:
		if (process_keydown(msg, lp, wp, true)) return 0;
		break;
	case WM_SYSKEYDOWN:
	{
		uie::window_ptr p_this = this;
		if (m_prevent_wm_char_processing = process_keydown(msg, lp, wp, true)) return 0;
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (0 && g_tooltip)
		{
			MSG message;
			memset(&message, 0, sizeof(MSG));
			message.hwnd = wnd;
			message.message = msg;
			message.wParam = wp;
			message.lParam = lp;

			uSendMessage(g_tooltip, TTM_RELAYEVENT, 0, (LPARAM)&message);
		}
		bool b_was_focused = GetFocus() == wnd;
		if (!b_was_focused)
			m_no_next_edit = true;
		//#ifdef INLINE_EDIT
		exit_inline_edit();
		//			g_no_next_edit = false;
		//#endif
		dragged = false;
		SetFocus(wnd);
		SetCapture(wnd);

		static_api_ptr_t<playlist_manager> playlist_api;
		g_drag_lmb = true;
		int focus = playlist_api->activeplaylist_get_focus_item();

		drag_start_lmb.x = GET_X_LPARAM(lp);
		drag_start_lmb.y = GET_Y_LPARAM(lp);

		int item_height = get_item_height();
		int idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
		//		int idx = ((GET_Y_LPARAM(lp) -get_header_height()) / item_height) + scroll_item_offset;
		//		if( idx >= 0 && idx <playlist_api->activeplaylist_get_item_count()  && GET_X_LPARAM(lp) < g_playlist_entries.get_total_width_actual())

		if (idx >= 0)
		{

			//		playlist_oper * playlist_api = playlist_api;
			//				playlist_api->set_playback_cursor(idx);
			//#ifdef INLINE_EDIT
			m_prev_sel = (playlist_api->activeplaylist_is_item_selected(idx) && !m_wnd_edit && (playlist_api->activeplaylist_get_selection_count(2) == 1));
			//#endif

			if (!is_visible(idx)) SendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);

			if (wp & MK_CONTROL && wp & MK_SHIFT)
			{
				playlist_api->activeplaylist_move_selection(idx - focus);
				dragged = true;
				drag_type = 0;
			}
			else if (wp & MK_SHIFT)
			{
				drag_type = 2; dragitem = idx, dragstartitem = idx;

				int n = (cfg_alternative_sel ? focus : g_shift_item_start), t = idx;
				bool focus_sel = playlist_api->activeplaylist_is_item_selected(focus);


				set_sel_range(n, t, (cfg_alternative_sel != 0), (cfg_alternative_sel ? !focus_sel : false));
				playlist_api->activeplaylist_set_focus_item(idx);

				dragged = true;

			}
			else if (wp & MK_CONTROL)
			{
				/*			drag_type = 2; dragitem = idx,dragstartitem=idx;

				set_sel_single(idx, false, true, false);

				dragged = true;*/

			}
			else if (playlist_api->activeplaylist_is_item_selected(idx))
			{
				drag_type = 1; dragitem = idx, dragstartitem = idx;
				playlist_api->activeplaylist_undo_backup();
				playlist_api->activeplaylist_set_focus_item(idx);
				dragged = false;
			}
			else
			{
				drag_type = 2; dragitem = idx, dragstartitem = idx;//item irrelevant actually;

				set_sel_single(idx, false, true, true);

				/*			bit_array_bittable mask(playlist_api->activeplaylist_get_item_count());
				//		playlist_api->activeplaylist_is_item_selected_mask(mask);
				int n, t = playlist_api->activeplaylist_get_item_count();
				for (n = 0;n <t;n++) { if (n==idx) mask.set(n, true); else mask.set(n, false); }

				console::info("crap");
				playlist_api->set_sel_mask(mask);
				playlist_api->activeplaylist_set_focus_item(idx);*/

				dragged = false;
			}
		}
		else
		{
			//			console::info("wow");
			//				bit_array_bittable mask(playlist_api->activeplaylist_get_item_count());
			playlist_api->activeplaylist_set_selection(bit_array_true(), bit_array_false());
			dragged = true;
			drag_type = 0;
		}
	}

	break;
	case WM_RBUTTONUP:
		m_no_next_edit = false;
		break;
	case WM_MBUTTONUP:
	{
		m_no_next_edit = false;
		unsigned idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
		playlist_mclick_actions::run(cfg_playlist_middle_action, idx != -1, idx);
	}
	break;

	case WM_LBUTTONUP:
	{
		if (0 && g_tooltip)
		{
			MSG message;
			memset(&message, 0, sizeof(MSG));
			message.hwnd = wnd;
			message.message = msg;
			message.wParam = wp;
			message.lParam = lp;

			uSendMessage(g_tooltip, TTM_RELAYEVENT, 0, (LPARAM)&message);
		}
		ReleaseCapture();
		g_drag_lmb = false;
		int idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), true);   //((GET_Y_LPARAM(lp) -get_header_height()) / get_item_height()) + scroll_item_offset;
		static_api_ptr_t<playlist_manager> playlist_api;
		if (!dragged)
		{
			if (wp & MK_CONTROL)
			{
				//			int idx_down = hittest_item(drag_start_lmb.x, drag_start_lmb.y);
				if (idx >= 0) set_sel_single(idx, true, true, false);
			}
			else
			{

				//				int item_height = get_item_height();

				//			int idx = ((GET_Y_LPARAM(lp) - get_header_height()) / item_height) + scroll_item_offset;
				if (idx >= 0 /*&& idx < playlist_api->activeplaylist_get_item_count() && (GET_X_LPARAM(lp) < g_playlist_entries.get_total_width_actual())*/)
				{



					if (!m_no_next_edit && cfg_inline_edit && playlist_api->activeplaylist_is_item_selected(idx) && m_prev_sel /*&& !dragged*/)
					{
						//if (m_no_next_edit && GetCapture() == wnd) ReleaseCapture();

						{
							exit_inline_edit();
							if (main_window::config_get_inline_metafield_edit_mode() != main_window::mode_disabled)
							{
								m_edit_index = idx;
								long width;
								m_edit_column = hittest_column(GET_X_LPARAM(lp), width);
								if (m_edit_column >= 0 && !g_get_columns()[g_get_cache().active_column_active_to_actual(m_edit_column)]->edit_field.is_empty())
								{
									m_edit_timer = (SetTimer(wnd, EDIT_TIMER_ID, GetDoubleClickTime(), 0) != 0);
								}
							}
						}

					}

					int focus = playlist_api->activeplaylist_get_focus_item();
					set_sel_single(focus, false, false, true);
				}


			}
		}
		dragged = true;
		drag_type = 0;
		dragstartitem = 0;
		dragitem = 0;
		//#ifdef INLINE_EDIT
		m_no_next_edit = false;
		//#endif
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (0 && g_tooltip)
		{
			MSG message;
			memset(&message, 0, sizeof(MSG));
			message.hwnd = wnd;
			message.message = msg;
			message.wParam = wp;
			message.lParam = lp;

			uSendMessage(g_tooltip, TTM_RELAYEVENT, 0, (LPARAM)&message);
		}
		const unsigned cx_drag = (unsigned)abs(GetSystemMetrics(SM_CXDRAG));
		const unsigned cy_drag = (unsigned)abs(GetSystemMetrics(SM_CYDRAG));
		if (!g_dragging && ((g_dragging1 && wp & MK_RBUTTON && (abs(drag_start.x - GET_X_LPARAM(lp)) > cx_drag || abs(drag_start.y - GET_Y_LPARAM(lp)) > cy_drag)) || (g_drag_lmb && (wp & MK_LBUTTON) && (wp & MK_CONTROL) && (abs(drag_start_lmb.x - GET_X_LPARAM(lp)) > 3 || abs(drag_start_lmb.y - GET_Y_LPARAM(lp)) > 3))))
		{
			static_api_ptr_t<playlist_manager> playlist_api;
			metadb_handle_list data;
			playlist_api->activeplaylist_get_selected_items(data);
			if (data.get_count() > 0)
			{
				static_api_ptr_t<playlist_incoming_item_filter> incoming_api;
				IDataObject * pDataObject = incoming_api->create_dataobject(data);
				if (pDataObject)
				{
					//RegisterClipboardFormat(_T("foo_ui_columns");

					if (g_tooltip) { DestroyWindow(g_tooltip); g_tooltip = 0; last_idx = -1; last_column = -1; }
					DWORD blah;
					{
						pfc::com_ptr_t<IDropSource_playlist> p_IDropSource_playlist = new IDropSource_playlist(this);
						DoDragDrop(pDataObject, p_IDropSource_playlist.get_ptr(), DROPEFFECT_COPY, &blah);
					}
					pDataObject->Release();
				}
			}
			data.remove_all();
			g_dragging = false;
			g_dragging1 = false;
			g_drag_lmb = false;
			if (wp & MK_LBUTTON)
			{
				dragged = true;
				drag_type = 0;
				dragstartitem = 0;
				dragitem = 0;
			}
		}




		if (cfg_tooltip && (GET_Y_LPARAM(lp) > get_header_height()))
		{
			int item_height = get_item_height();
			int idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
			long cx;
			int column = hittest_column(GET_X_LPARAM(lp), cx);
			//			unsigned act_col = g_cache.active_column_active_to_actual(column);

			if (column >= 0 && idx >= 0)
			{
				if (last_idx != (idx) || last_column != column)
				{
					if (!cfg_tooltips_clipped || is_item_clipped(idx, column))
					{
						pfc::string8 src;
						g_cache.active_get_display_name(idx, column, src);
						pfc::string8 temp;
						titleformat_compiler::remove_color_marks(src, temp);
						temp.replace_char(9, 0x20);
						CreateToolTip(temp);
					}
					else { DestroyWindow(g_tooltip); g_tooltip = 0; last_idx = -1; last_column = -1; }

					POINT a;
					a.x = cx + 3;
					a.y = (idx - scroll_item_offset) * item_height + get_header_height();
					ClientToScreen(wnd_playlist, &a);

					tooltip.top = a.y;
					tooltip.bottom = a.y + item_height;
					tooltip.left = a.x;
					tooltip.right = a.x + get_column_width(column);

				}
				last_idx = idx;
				last_column = column;
			}
			else { DestroyWindow(g_tooltip); g_tooltip = 0; last_idx = -1; last_column = -1; }
		}


		if (drag_type && (wp & MK_LBUTTON) && !(GetKeyState(VK_SHIFT) & KF_UP) && !(GetKeyState(VK_CONTROL) & KF_UP))
		{
			RECT rc;
			get_playlist_rect(&rc);
			static_api_ptr_t<playlist_manager> playlist_api;

			int total = playlist_api->activeplaylist_get_item_count();

			int item_height = get_item_height();
			int valid_idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), false);
			int idx = hittest_item_no_scroll(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), false);
			//    (GET_Y_LPARAM(lp) - get_header_height()) / (item_height);

			int items_count = ((rc.bottom - rc.top) / item_height) + 1;


			if ((idx + scroll_item_offset) != dragitem || GET_Y_LPARAM(lp) < get_header_height()) //(idx + scroll_item_offset) < playlist_api->activeplaylist_get_item_count()
			{
				if (idx >= items_count - 1)
				{

					bool need_redrawing = false;

					int focus = playlist_api->activeplaylist_get_focus_item();

					SCROLLINFO si;
					memset(&si, 0, sizeof(si));
					si.cbSize = sizeof(si);
					si.fMask = SIF_POS;
					GetScrollInfo(wnd_playlist, SB_VERT, &si);

					int old_offset = si.nPos;
					si.nPos += 3;

					scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &si, true);

					if (old_offset != scroll_item_offset) need_redrawing = true;

					int t = scroll_item_offset + items_count - 2; //n=dragitem,

					if (t > total) t = total - 1;


					if (t != dragitem)
					{

						drawing_enabled = false;
						if (drag_type == 1)
							playlist_api->activeplaylist_move_selection((rc.bottom - rc.top) / item_height + scroll_item_offset - focus - 1);
						else if (drag_type == 2)
						{

							set_sel_range(dragstartitem, t, false);
							playlist_api->activeplaylist_set_focus_item(t);
						}

						dragitem = t;
						drawing_enabled = true;
						need_redrawing = true;

					}
					if (need_redrawing) RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);


				}
				else if (idx < 0 || GET_Y_LPARAM(lp) < get_header_height() || GET_Y_LPARAM(lp) < 0)
				{


					int focus = playlist_api->activeplaylist_get_focus_item();

					bool need_redrawing = false;

					SCROLLINFO si;
					memset(&si, 0, sizeof(si));
					si.cbSize = sizeof(si);
					si.fMask = SIF_POS;
					GetScrollInfo(wnd_playlist, SB_VERT, &si);
					int old_offset = si.nPos;
					si.nPos -= 3;
					scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &si, true);

					if (old_offset != scroll_item_offset) need_redrawing = true;

					if (dragitem != scroll_item_offset)
					{
						drawing_enabled = false;
						if (drag_type == 1)
							playlist_api->activeplaylist_move_selection(scroll_item_offset - focus);
						else if (drag_type == 2)
						{

							set_sel_range(dragstartitem, scroll_item_offset, false);
							playlist_api->activeplaylist_set_focus_item(scroll_item_offset);
							RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
						}

						dragitem = scroll_item_offset;
						drawing_enabled = true;
						need_redrawing = true;
					}

					if (need_redrawing) RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);


				}
				else
				{
					int focus = playlist_api->activeplaylist_get_focus_item();

					if (drag_type == 1)
						playlist_api->activeplaylist_move_selection(idx + scroll_item_offset - focus);
					else if (drag_type == 2)
					{
						if (valid_idx >= 0)
						{
							drawing_enabled = false;
							set_sel_range(dragstartitem, valid_idx, false);
							playlist_api->activeplaylist_set_focus_item(valid_idx);
							drawing_enabled = true;
							RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
						}

					}

					dragitem = valid_idx;
					dragged = true;
				}
			}

		}
		else if (!(wp & MK_LBUTTON)) drag_type = 0;
	}
	break;
	case WM_LBUTTONDBLCLK:
	{
		int idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), true);

		if (idx >= 0)
		{
			//#ifdef INLINE_EDIT
			exit_inline_edit();
			m_no_next_edit = true;
			//#endif
			//if (!is_visible(idx)) uSendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0),0);

#if 0
			// DEATH's code
	case WM_LBUTTONDBLCLK:
	{
		int idx = item_from_point((short)HIWORD(lp));
		if (idx >= 0 && idx<(int)m_api->activeplaylist_get_item_count())
		{
			m_api->activeplaylist_set_focus_item(idx);
			static_api_ptr_t<play_control>()->play_start(play_control::TRACK_COMMAND_SETTRACK);
		}
	}
	return 0;
#endif
	static_api_ptr_t<playlist_manager> playlist_api;
	//unsigned active = playlist_api->get_active_playlist();
	//				playlist_api->set_playing_playlist(active);
	//playlist_api->playlist_set_playback_cursor(active, idx);
	//playlist_api->queue_flush();
	unsigned focus = playlist_api->activeplaylist_get_focus_item();
	playlist_api->activeplaylist_execute_default_action(focus);

		}
		else if (cfg_playlist_double.get_value().m_command != pfc::guid_null)
		{
			mainmenu_commands::g_execute(cfg_playlist_double.get_value().m_command);
		}

		dragged = true;
	}

	break;
	case WM_RBUTTONDOWN:
	{
		if (wnd_playlist) SetFocus(wnd_playlist);

		g_dragging1 = true;

		drag_start.x = GET_X_LPARAM(lp);
		drag_start.y = GET_Y_LPARAM(lp);

		static_api_ptr_t<playlist_manager> playlist_api;


		//		int item_height = get_item_height();
		//		int idx = ((GET_Y_LPARAM(lp) - get_header_height()) / item_height) + scroll_item_offset;
		int idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), true);
		if (idx != -1 && !is_visible(idx))
			SendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);

		if (idx >= 0 /*&& idx < playlist_api->activeplaylist_get_item_count() && (GET_X_LPARAM(lp) < g_playlist_entries.get_total_width_actual())*/)
		{

			if (!playlist_api->activeplaylist_is_item_selected(idx) && !(GetKeyState(VK_CONTROL) & KF_UP))
			{
				set_sel_single(idx, false, false, true);
			}
			playlist_api->activeplaylist_set_focus_item(idx);

		}


	}

	break;
	case WM_MOUSEWHEEL:
	{//GET_WHEEL_DELTA_WPARAM
		exit_inline_edit();
		if (1 || (wp & MK_CONTROL))
		{

			LONG_PTR style = GetWindowLongPtr(wnd_playlist, GWL_STYLE);
			if (!(style & WS_VSCROLL) || ((wp & MK_CONTROL) && (style & WS_HSCROLL)))
			{
				if ((style & WS_HSCROLL))
				{
					SCROLLINFO si;
					memset(&si, 0, sizeof(SCROLLINFO));
					si.fMask = SIF_PAGE;
					si.cbSize = sizeof(SCROLLINFO);
					GetScrollInfo(wnd, SB_HORZ, &si);

					int new_pos = horizontal_offset;
					int old_pos = horizontal_offset;

					unsigned scroll_lines = GetNumScrollLines();

					int zDelta = short(HIWORD(wp));

					if (scroll_lines == -1)
					{
						scroll_lines = si.nPage > 1 ? si.nPage - 1 : 1;
					}
					else scroll_lines *= 3;

					int delta = MulDiv(zDelta, scroll_lines, 120);

					if (!si.nPage) si.nPage++;

					if (delta < 0 && delta*-1 > si.nPage)
					{
						delta = si.nPage*-1;
						if (delta >1) delta--;
					}
					else if (delta > 0 && delta > si.nPage)
					{
						delta = si.nPage;
						if (delta >1) delta--;
					}

					scroll(scroll_horizontally, scroll_position_delta, -delta);

				}
				return 1;
			}
		}

		SCROLLINFO si;
		memset(&si, 0, sizeof(SCROLLINFO));
		si.fMask = SIF_PAGE;
		si.cbSize = sizeof(SCROLLINFO);
		GetScrollInfo(wnd, SB_VERT, &si);

		int new_pos = scroll_item_offset;
		int old_pos = scroll_item_offset;
		unsigned scroll_lines = GetNumScrollLines();

		int zDelta = short(HIWORD(wp));

		if (scroll_lines == -1)
		{
			scroll_lines = si.nPage > 1 ? si.nPage - 1 : 1;
		}

		int delta = MulDiv(zDelta, scroll_lines, 120);

		if (!si.nPage) si.nPage++;

		if (delta < 0 && delta*-1 > si.nPage)
		{
			delta = si.nPage*-1;
			if (delta >1) delta--;
		}
		else if (delta > 0 && delta > si.nPage)
		{
			delta = si.nPage;
			if (delta >1) delta--;
		}

		scroll(scroll_vertically, scroll_position_delta, -delta);
	}
	return 1;
	case WM_VSCROLL:
	{
		exit_inline_edit();
		scroll(scroll_vertically, scroll_sb, LOWORD(wp));
	}
	return 0;
	case WM_HSCROLL:
	{
		exit_inline_edit();
		scroll(scroll_horizontally, scroll_sb, LOWORD(wp));
	}
	return 0;
	case WM_MENUSELECT:
	{
		if (HIWORD(wp) & MF_POPUP)
		{
			m_status_override.release();
		}
		else
		{
			if (g_main_menu_a.is_valid() || g_main_menu_b.is_valid())
			{
				unsigned id = LOWORD(wp);

				bool set = false;

				pfc::string8 desc;

				if (g_main_menu_a.is_valid() && id < MENU_B_BASE)
				{
					set = g_main_menu_a->get_description(id - MENU_A_BASE, desc);
				}
				else if (g_main_menu_b.is_valid())
				{
					contextmenu_node * node = g_main_menu_b->find_by_id(id - MENU_B_BASE);
					if (node) set = node->get_description(desc);
				}

				service_ptr_t<ui_status_text_override> p_status_override;

				if (set)
				{
					get_host()->override_status_text_create(p_status_override);

					if (p_status_override.is_valid())
					{
						p_status_override->override_text(desc);
					}
				}
				m_status_override = p_status_override;
			}
		}
	}
	break;
	case WM_CONTEXTMENU:
	{
		uie::window_ptr p_this_temp = this;
		if ((HWND)wp == wnd_header)
		{
			POINT pt = { (short)LOWORD(lp), (short)HIWORD(lp) };
			POINT temp;
			temp.x = pt.x;
			temp.y = pt.y;
			ScreenToClient(wnd_header, &temp);
			HDHITTESTINFO hittest;
			hittest.pt.x = temp.x;
			hittest.pt.y = temp.y;


			uSendMessage(wnd_header, HDM_HITTEST, 0, (LPARAM)&hittest);

			enum { IDM_ASC = 1, IDM_DES = 2, IDM_SEL_ASC, IDM_SEL_DES, IDM_AUTOSIZE, IDM_PREFS, IDM_EDIT_COLUMN, IDM_CUSTOM_BASE };

			HMENU menu = CreatePopupMenu();
			HMENU selection_menu = CreatePopupMenu();
			if (!(hittest.flags & HHT_NOWHERE))
			{
				uAppendMenu(menu, (MF_STRING), IDM_ASC, "&Sort ascending");
				uAppendMenu(menu, (MF_STRING), IDM_DES, "Sort &descending");
				uAppendMenu(selection_menu, (MF_STRING), IDM_SEL_ASC, "Sort a&scending");
				uAppendMenu(selection_menu, (MF_STRING), IDM_SEL_DES, "Sort d&escending");
				uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)selection_menu, "Se&lection");
				uAppendMenu(menu, (MF_SEPARATOR), 0, "");
				uAppendMenu(menu, (MF_STRING), IDM_EDIT_COLUMN, "&Edit this column");
				uAppendMenu(menu, (MF_SEPARATOR), 0, "");
				uAppendMenu(menu, (MF_STRING | (cfg_nohscroll ? MF_CHECKED : MF_UNCHECKED)), IDM_AUTOSIZE, "&Auto-sizing columns");
				uAppendMenu(menu, (MF_STRING), IDM_PREFS, "&Preferences");
				uAppendMenu(menu, (MF_SEPARATOR), 0, "");

				pfc::string8 playlist_name;
				static_api_ptr_t<playlist_manager> playlist_api;
				playlist_api->activeplaylist_get_name(playlist_name);

				pfc::string8_fast_aggressive filter, name;

				int s, e = columns.get_count();
				for (s = 0; s<e; s++)
				{
					bool add = false;
					switch (columns[s]->filter_type)
					{
					case FILTER_NONE:
					{
						add = true;
						break;
					}
					case FILTER_SHOW:
					{
						if (wildcard_helper::test(playlist_name, columns[s]->filter, true))
						{
							add = true;
							/*				g_columns.get_string(s, name, STRING_NAME);
							uAppendMenu(menu,MF_STRING|MF_CHECKED,IDM_CUSTOM_BASE+s,name);*/
						}
					}
					break;
					case FILTER_HIDE:
					{
						if (!wildcard_helper::test(playlist_name, columns[s]->filter, true))
						{
							add = true;
							/*						g_columns.get_string(s, name, STRING_NAME);
							uAppendMenu(menu,MF_STRING|MF_CHECKED,IDM_CUSTOM_BASE+s,name);*/
						}
					}
					break;
					}
					if (add)
					{
						uAppendMenu(menu, MF_STRING | (columns[s]->show ? MF_CHECKED : MF_UNCHECKED), IDM_CUSTOM_BASE + s, columns[s]->name);
					}
				}


			}
			else
			{
				uAppendMenu(menu, (MF_STRING | (cfg_nohscroll ? MF_CHECKED : MF_UNCHECKED)), IDM_AUTOSIZE, "&Auto-sizing columns");
				uAppendMenu(menu, (MF_STRING), IDM_PREFS, "&Preferences");
			}


			menu_helpers::win32_auto_mnemonics(menu);

			int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, 0);
			DestroyMenu(menu);

			if (cmd == IDM_ASC)
			{

				g_set_sort(hittest.iItem, false);
			}
			else if (cmd == IDM_DES)
			{
				g_set_sort(hittest.iItem, true);
			}
			else if (cmd == IDM_SEL_ASC)
			{
				g_set_sort(hittest.iItem, false, true);
			}
			else if (cmd == IDM_SEL_DES)
			{
				g_set_sort(hittest.iItem, true, true);
			}
			else if (cmd == IDM_EDIT_COLUMN)
			{
				g_set_tab("Columns");
				cfg_cur_prefs_col = g_cache.active_column_active_to_actual(hittest.iItem); //get_idx
				static_api_ptr_t<ui_control>()->show_preferences(columns::config_get_playlist_view_guid());
			}
			else if (cmd == IDM_AUTOSIZE)
			{
				cfg_nohscroll = cfg_nohscroll == 0;
				update_all_windows();
				pvt::ng_playlist_view_t::g_on_autosize_change();
			}
			else if (cmd == IDM_PREFS)
			{
				static_api_ptr_t<ui_control>()->show_preferences(columns::config_get_main_guid());
			}
			else if (cmd >= IDM_CUSTOM_BASE)
			{
				if (t_size(cmd - IDM_CUSTOM_BASE) < columns.get_count())
				{
					columns[cmd - IDM_CUSTOM_BASE]->show = !columns[cmd - IDM_CUSTOM_BASE]->show; //g_columns
					//if (!cfg_nohscroll) 
					g_save_columns();
					//g_cache.flush_all();
					g_reset_columns();
					update_all_windows();
					pvt::ng_playlist_view_t::g_on_columns_change();
				}

			}
			return 0;
		}
		else if ((HWND)wp == wnd)
		{
			//DWORD mp = GetMessagePos();
			POINT px, pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
			static_api_ptr_t<playlist_manager> playlist_api;
			if (playlist_api->activeplaylist_get_selection_count(1) > 0 && 1)
			{
				if (pt.x == -1 && pt.y == -1)
				{
					int focus = playlist_api->activeplaylist_get_focus_item();
					unsigned last = get_last_viewable_item();
					if (focus == -1 || focus < scroll_item_offset || focus > last)
					{
						px.x = 0;
						px.y = 0;
					}
					else
					{
						RECT rc;
						get_playlist_rect(&rc);
						px.x = 0;
						unsigned item_height = get_item_height();
						px.y = (focus - scroll_item_offset)*(item_height)+item_height / 2 + rc.top;
					}
					pt = px;
					MapWindowPoints(wnd, HWND_DESKTOP, &pt, 1);
				}
				else
				{
					px = pt;
					ScreenToClient(wnd, &px);
					//int idx = hittest_item(px.x, px.y);
					//if (!is_visible(idx))
					//	SendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0),0);

				}
				//			int idx = hittest_item(px.x, px.y);

				enum { ID_PLAY = 1, ID_CUT, ID_COPY, ID_PASTE, ID_SELECTION, ID_CUSTOM_BASE = 0x8000 };
				HMENU menu = CreatePopupMenu();//LoadMenu(core_api::get_my_instance(),MAKEINTRESOURCE(IDR_TREEPOPUP));

				service_ptr_t<mainmenu_manager> p_manager_selection;
				service_ptr_t<contextmenu_manager> p_manager_context;
				p_manager_selection = standard_api_create_t<mainmenu_manager>();
				contextmenu_manager::g_create(p_manager_context);
				if (p_manager_selection.is_valid())
				{
					p_manager_selection->instantiate(mainmenu_groups::edit_part2_selection);
					p_manager_selection->generate_menu_win32(menu, ID_SELECTION, ID_CUSTOM_BASE - ID_SELECTION, standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0);
					if (GetMenuItemCount(menu) > 0) uAppendMenu(menu, MF_SEPARATOR, 0, "");
				}

				AppendMenu(menu, MF_STRING, ID_CUT, L"Cut");
				AppendMenu(menu, MF_STRING, ID_COPY, L"Copy");
				if (playlist_utils::check_clipboard())
					AppendMenu(menu, MF_STRING, ID_PASTE, L"Paste");
				AppendMenu(menu, MF_SEPARATOR, 0, NULL);
				if (p_manager_context.is_valid())
				{
					const keyboard_shortcut_manager::shortcut_type shortcuts[] = { keyboard_shortcut_manager::TYPE_CONTEXT_PLAYLIST, keyboard_shortcut_manager::TYPE_CONTEXT };
					p_manager_context->set_shortcut_preference(shortcuts, tabsize(shortcuts));
					p_manager_context->init_context_playlist(standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0);

					p_manager_context->win32_build_menu(menu, ID_CUSTOM_BASE, -1);
				}
				menu_helpers::win32_auto_mnemonics(menu);
				MENU_A_BASE = ID_SELECTION;
				MENU_B_BASE = ID_CUSTOM_BASE;

				g_main_menu_a = p_manager_selection;
				g_main_menu_b = p_manager_context;

				int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, 0);
				if (m_status_override.is_valid())
				{
					m_status_override.release();
				}

				DestroyMenu(menu);
				if (cmd)
				{
					if (cmd == ID_CUT)
					{
						playlist_utils::cut();
					}
					else if (cmd == ID_COPY)
					{
						playlist_utils::copy();
					}
					else if (cmd == ID_PASTE)
					{
						playlist_utils::paste(wnd);
					}
					else if (cmd >= ID_SELECTION && cmd<ID_CUSTOM_BASE)
					{
						if (p_manager_selection.is_valid())
						{
							p_manager_selection->execute_command(cmd - ID_SELECTION);
						}
					}
					else if (cmd >= ID_CUSTOM_BASE)
					{
						if (p_manager_context.is_valid())
						{
							p_manager_context->execute_by_id(cmd - ID_CUSTOM_BASE);
						}
					}
				}
				g_main_menu_a.release();
				g_main_menu_b.release();
			}


			//	contextmenu_manager::win32_run_menu_context_playlist(wnd, 0, config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, true) ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0);
		}
	}
	return 0;

	//#ifdef INLINE_EDIT
	case WM_PARENTNOTIFY:
	{
		if (wp == WM_DESTROY)
		{
			if (m_wnd_edit && (HWND)lp == m_wnd_edit) m_wnd_edit = 0;
		}
	}
	break;
	case MSG_KILL_INLINE_EDIT:
		exit_inline_edit();
		return 0;

#if 1
	case WM_COMMAND:
		switch (wp)
		{
		case (EN_CHANGE << 16) | 667:
		{
			m_edit_changed = true;
		}
		break;
		}
		break;
#endif

	case WM_TIMER:
	{
		if (wp == EDIT_TIMER_ID)
		{
			create_inline_edit_v2(m_edit_index, m_edit_column);
			if (m_edit_timer)
			{
				KillTimer(wnd_playlist, EDIT_TIMER_ID);
				m_edit_timer = false;
			}
			return 0;
		}

	}
	break;

	//#endif
	case WM_NOTIFY:
		switch (((LPNMHDR)lp)->idFrom)
		{
		case ID_PLAYLIST_TOOLTIP:
			switch (((LPNMHDR)lp)->code)
			{
			case TTN_SHOW:

				RECT rc, rc_tt;

				rc = tooltip;
				GetWindowRect(g_tooltip, &rc_tt);

				int offset = MulDiv(get_item_height() - rc_tt.bottom + rc_tt.top, 1, 2);


				rc.top += offset;




				SetWindowPos(g_tooltip,
					NULL,
					rc.left, rc.top,
					0, 0,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				return TRUE;
			}
			break;
		case 5001:
			switch (((LPNMHDR)lp)->code)
			{
			case HDN_BEGINTRACKA:
			case HDN_BEGINTRACKW:
			{
				return (cfg_nohscroll ? TRUE : FALSE);
			}
			case HDN_ENDDRAG:
			{
				if (((LPNMHEADERA)lp)->iButton == 0)
				{

					if (((LPNMHEADERA)lp)->pitem && (((LPNMHEADERA)lp)->pitem->mask & HDI_ORDER))
					{

						int from = ((LPNMHEADERA)lp)->iItem;
						int to = ((LPNMHEADERA)lp)->pitem->iOrder;
						if (to >= 0 && from != to)
						{
							int act_from = g_cache.active_column_active_to_actual(from), act_to = g_cache.active_column_active_to_actual(to);

							columns.move(act_from, act_to);
							//if (!cfg_nohscroll) 
							g_save_columns();
							g_reset_columns();
							update_all_windows();
							pvt::ng_playlist_view_t::g_on_columns_change();
						}
					}
					else
					{
					}
				}
				return (TRUE);
			}
			case HDN_DIVIDERDBLCLICK:
				if (!cfg_nohscroll)
				{
					static_api_ptr_t<playlist_manager> playlist_api;
					HDC hdc;
					hdc = GetDC(wnd_playlist);
					int size;
					pfc::string8 text;

					SelectObject(hdc, g_font);


					int w = 0, n, t = playlist_api->activeplaylist_get_item_count();

					for (n = 0; n<t; n++)
					{
						//	playlist_api->format_title(n, text, g_playlist_entries.get_display_spec(((LPNMHEADER)lp)->iItem), NULL);
						g_cache.active_get_display_name(n, ((LPNMHEADER)lp)->iItem, text);
						size = ui_helpers::get_text_width_color(hdc, text, text.length());
						if (size > w) w = size;
					}

					//	g_playlist_entries.get_column(((LPNMHEADER)lp)->iItem)->_set_width(w+5);
					columns[g_cache.active_column_active_to_actual(((LPNMHEADER)lp)->iItem)]->width = w + 15;

					ReleaseDC(wnd_playlist, hdc);
					update_all_windows();
					g_save_columns();
					pvt::ng_playlist_view_t::g_on_column_widths_change();
				}

				return 0;
			case HDN_ITEMCLICK:
			{
				bool des = false;

				static_api_ptr_t<playlist_manager> playlist_api;

				unsigned col;
				bool descending;
				bool sorted = g_cache.active_get_playlist_sort(col, &descending);

				if (sorted && col == ((LPNMHEADER)lp)->iItem)
					des = !descending;

				g_set_sort(((LPNMHEADER)lp)->iItem, des /*, playlist_api->activeplaylist_get_selection_count(1) && cfg_sortsel != 0*/);

			}
			break;
			case HDN_ITEMCHANGED:
			{
				if (!cfg_nohscroll)
				{
					if (((LPNMHEADER)lp)->pitem->mask & HDI_WIDTH)
						columns[g_cache.active_column_active_to_actual(((LPNMHEADER)lp)->iItem)]->width = ((LPNMHEADER)lp)->pitem->cxy;
					update_all_windows(wnd_header);
					g_save_columns();
					pvt::ng_playlist_view_t::g_on_column_widths_change();
				}
			}
			break;
			}
			break;
		}

	}
	return uDefWindowProc(wnd, msg, wp, lp);
}

