#include "stdafx.h"

LRESULT t_list_view::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
#if 0
	static UINT MSG_DI_GETDRAGIMAGE = RegisterWindowMessage(DI_GETDRAGIMAGE);
	
	if (0 && msg && msg == MSG_DI_GETDRAGIMAGE)
	{
		LPSHDRAGIMAGE lpsdi = (LPSHDRAGIMAGE)lp;

		t_size selection_count = 0, count = get_item_count();
		bit_array_bittable mask(count);
		get_selection_state(mask);
		pfc::list_t<t_size> indices;
		for (t_size i = 0; i<count; i++)
		{
			if (mask[i]) 
			{
				indices.add_item(i);
				if (++selection_count == 10) break;
			}
		}
		if (selection_count)
		{
			RECT rc_client;
			get_items_rect(&rc_client);

			HDC dc = GetDC(wnd);

			RECT rc = rc_client;
			rc.top = 0;
			rc.bottom = m_item_height * selection_count;
			//console::formatter() << m_item_height;
			rc.right = min (RECT_CX(rc), 256);
			rc.left = 0;

			//POINT pt = {0};
			//GetMessagePos(&pt);
			//ScreenToClient(wnd, &pt);
			//pt.x += m_horizontal_scroll_position;

			LOGFONT lf;
			if (m_lf_items_valid) lf = m_lf_items;
			else
				GetObject(m_font, sizeof(lf), &lf);
			lf.lfQuality = NONANTIALIASED_QUALITY;
			HFONT fnt = CreateFontIndirect(&lf);

			HDC dc_mem = CreateCompatibleDC(dc);
			HBITMAP bm_mem = CreateCompatibleBitmap(dc, RECT_CX(rc), RECT_CY(rc));

			HBITMAP bm_old = SelectBitmap(dc_mem, bm_mem);
			HFONT font_old = SelectFont(dc_mem, fnt);
			for (t_size i = 0; i<selection_count; i++)
			{
				RECT rc_item = rc; 
				rc_item.top = i*m_item_height;
				rc_item.bottom = (i+1)*m_item_height;
				render_item(dc_mem, indices[i], 0, true, true, false, false, &rc_item);
			}
			//FillRect(dc_mem, &rc, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(RGB(255,0,0))));
			SelectFont(dc_mem, font_old);
			SelectObject(dc_mem, bm_old);
			//DeleteObject(bm_mem);
			DeleteFont(fnt);
			DeleteDC(dc_mem);
			ReleaseDC(wnd, dc);



			lpsdi->sizeDragImage.cx = RECT_CX(rc);
			lpsdi->sizeDragImage.cy = RECT_CY(rc);
			lpsdi->ptOffset.x = RECT_CX(rc)/2;
			lpsdi->ptOffset.y = RECT_CY(rc) - m_item_height/3;
			lpsdi->hbmpDragImage = bm_mem;
			lpsdi->crColorKey = 0x00ffff;

			return TRUE;

		}
	}
#endif

	switch (msg)
	{
	case WM_CREATE:
		{
			m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"ListView") : NULL;
			SetWindowTheme(wnd, L"Explorer", NULL);
		}
		notify_on_initialisation();
		m_font = m_lf_items_valid ? CreateFontIndirect(&m_lf_items) : uCreateIconFont();
		m_group_font = m_lf_group_header_valid ? CreateFontIndirect(&m_lf_group_header) : (m_lf_items_valid ? CreateFontIndirect(&m_lf_items) : uCreateIconFont());
		m_item_height = get_default_item_height();
		m_group_height = get_default_group_height();
		if (m_show_header)
			create_header();
		m_initialised = true;
		notify_on_create();
		build_header();
		if (m_wnd_header)
			ShowWindow(m_wnd_header, SW_SHOWNORMAL);
		return 0;
	case WM_DESTROY:
		m_initialised = false;
		m_inline_edit_save = false;
		destroy_tooltip();
		exit_inline_edit();
		destroy_header();
		{
			if (m_theme) CloseThemeData(m_theme);
			m_theme = NULL;
		}
		m_font.release();
		notify_on_destroy();
		return 0;
	/*case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
			if (lpwp->flags & SWP_SHOWWINDOW) {
			   on_size();
			}
			if (lpwp->flags & SWP_HIDEWINDOW) {
			   //window_was_hidden();
			}
			if (!(lpwp->flags & SWP_NOMOVE)) {
			   //window_moved_to(pwp->x, pwp->y);
			}
			if (!(lpwp->flags & SWP_NOSIZE)) {
			   on_size(lpwp->cx, lpwp->cy);
			}

		}
		return 0;*/
	case WM_SIZE:
		on_size(LOWORD(lp), HIWORD(lp));
		break;
	/*case WM_STYLECHANGING:
		{
			LPSTYLESTRUCT plss = (LPSTYLESTRUCT)lp;
			if (wp == GWL_EXSTYLE)
				console::formatter() << "GWL_EXSTYLE changed";
		}
		break;*/
	case WM_THEMECHANGED:
		{
			if (m_theme) CloseThemeData(m_theme);
			m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"ListView") : 0;
		}
		break;
	case WM_TIMECHANGE:
		notify_on_time_change();
		break;
	case WM_MENUSELECT:
		notify_on_menu_select(wp, lp);
		break;
	case WM_PRINT:
		break;
	case WM_PRINTCLIENT:
		/*//if (lp == PRF_ERASEBKGND)
		{
			HDC dc = (HDC)wp;
			RECT rc;
			GetClientRect(wnd, &rc);
			render_background(dc, &rc);
			return 0;
		}*/
		break;
	case WM_ERASEBKGND:
		{
			/*HDC dc = (HDC)wp;
			//if (m_wnd_header)
			{
				RECT rc;
				GetClientRect(wnd, &rc);
				render_background(dc, &rc);
				return TRUE;
			}*/
		}
		return FALSE;
	case WM_PAINT:
		{
			//console::formatter() << "WM_PAINT";
			RECT rc_client;
			//GetClientRect(wnd, &rc_client);
			get_items_rect(&rc_client);
			//GetUpdateRect(wnd, &rc2, FALSE);

			PAINTSTRUCT ps;
			HDC dc = BeginPaint(wnd, &ps);

			RECT rc = /*rc_client;//*/ps.rcPaint;
			//RECT rc2 = {0, 0, RECT_CX(rc), RECT_CY(rc)};

			//console::formatter() << rc_client.left << " " << rc_client.top << " " << rc_client.right << " " << rc_client.bottom;

			HDC dc_mem = CreateCompatibleDC(dc);
			HBITMAP bm_mem = CreateCompatibleBitmap(dc, RECT_CX(rc), RECT_CY(rc));
			//if (!bm_mem) console::formatter() << "ONIJoj";
			HBITMAP bm_old = SelectBitmap(dc_mem, bm_mem);
			HFONT font_old = SelectFont(dc_mem, m_font);
			OffsetWindowOrgEx(dc_mem, rc.left, rc.top, NULL);
			//int item_height = get_default_item_height();
			render_items(dc_mem, rc, RECT_CX(rc_client));
			OffsetWindowOrgEx(dc_mem, -rc.left, -rc.top, NULL);
			BitBlt(dc,	rc.left, rc.top, RECT_CX(rc), RECT_CY(rc),
				dc_mem, 0, 0, SRCCOPY);

			SelectFont(dc_mem, font_old);
			SelectObject(dc_mem, bm_old);
			DeleteObject(bm_mem);
			DeleteDC(dc_mem);
			EndPaint(wnd, &ps);
		}
		return 0;
	case WM_UPDATEUISTATE:
		RedrawWindow(wnd, NULL, NULL, RDW_INVALIDATE);
		break;
	case WM_SETFOCUS:
		invalidate_all();
		if (!HWND(wp) || (HWND(wp) != wnd && !IsChild(wnd, HWND(wp))))
			notify_on_set_focus(HWND(wp));
		break;
	case WM_KILLFOCUS:
		invalidate_all();
		if (!HWND(wp) || (HWND(wp) != wnd && !IsChild(wnd, HWND(wp))))
			notify_on_kill_focus(HWND(wp));
		break;
	case WM_MOUSEACTIVATE:
		if (GetFocus() != wnd)
			m_inline_edit_prevent = true;
		return MA_ACTIVATE;
	case WM_LBUTTONDOWN:
		{
			bool b_was_focused = GetFocus() == wnd;
			if (!b_was_focused)
				m_inline_edit_prevent = true;
			exit_inline_edit();
			SetFocus(wnd);
			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			t_hit_test_result hit_result;
			hit_test_ex(pt, hit_result);
			m_lbutton_down_hittest = hit_result;
			bool b_shift_down = (wp & MK_SHIFT) != 0;
			m_lbutton_down_ctrl = (wp & MK_CONTROL) != 0 && !m_single_selection; //Cheat.

			if (hit_result.result == hit_test_on || hit_result.result == hit_test_obscured_below || hit_result.result == hit_test_obscured_above)
			{
				if (!m_inline_edit_prevent)
					m_inline_edit_prevent = !((get_item_selected(hit_result.index) && !m_wnd_inline_edit && (get_selection_count(2) == 1)));
				if (hit_result.result == hit_test_obscured_below)
					scroll(true, SB_LINEDOWN);
				if (hit_result.result == hit_test_obscured_above)
					scroll(true, SB_LINEUP);

				m_dragging_initial_point = pt;
				/*if (b_shift_down && m_lbutton_down_ctrl)
				{
					move_selection (hit_result.index-get_focus_item());
				}
				else */
				if (b_shift_down && !m_single_selection)
				{
					t_size focus = get_focus_item();
					t_size start = m_alternate_selection ? focus : m_shift_start;
					bit_array_range br(min(start, hit_result.index), abs(t_ssize(start - hit_result.index))+1);
					if (m_lbutton_down_ctrl && !m_alternate_selection)
					{
						set_selection_state(br,
							br, true, false);
					}
					else
					{
						set_selection_state(m_alternate_selection ? (bit_array&)br : (bit_array&)bit_array_true(), m_alternate_selection && !get_item_selected(focus) ? (bit_array&)bit_array_not(br) : (bit_array&)br, true, false);
						//set_selection_state(bit_array_true(),
						//	br, true, false);
					}
					set_focus_item(hit_result.index, true, false);
					UpdateWindow(get_wnd());
				}
				else
				{
					m_selecting_move = get_item_selected(hit_result.index);
					m_selecting_moved=false;
					m_selecting_start = hit_result.index;
					m_selecting_start_column = hit_result.column;
					if (!m_lbutton_down_ctrl)
					{
						if (!m_selecting_move)
							set_item_selected_single(hit_result.index);
						else
							set_focus_item(hit_result.index);
						m_selecting = true;//!m_single_selection;
					}
				}
				SetCapture(wnd);
			}
			else if (hit_result.result == hit_test_on_group)
			{
				if (!m_single_selection)
				{
					t_size index=0, count=0;
					if (!m_lbutton_down_ctrl)
					{
						get_item_group(hit_result.index, hit_result.group_level, index, count);
						set_selection_state(bit_array_true(), bit_array_range(index, count));
						if (count)
							set_focus_item(index);
					}
				}
			}
			else //if (hit_result.result != hit_test_)
			{
				if (!m_single_selection)
					set_selection_state(bit_array_true(), bit_array_false());
			}
			//console::formatter() << hit_result.result ;
		}
		return 0;
	case WM_LBUTTONUP:
		{
			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			if (m_selecting_move && ! m_selecting_moved && !m_lbutton_down_ctrl)
			{
				if (m_selecting_start < m_items.get_count())
				{
					if (!m_inline_edit_prevent && 1 && get_item_selected(m_selecting_start) && true/*m_prev_sel*/)
					{
						{
							exit_inline_edit();
							pfc::list_t<t_size> indices;
							indices.add_item(m_selecting_start);
							if (notify_before_create_inline_edit(indices, m_selecting_start_column, true))
							{
								m_inline_edit_indices = indices;
								m_inline_edit_column = m_selecting_start_column;
								if (m_inline_edit_column >= 0)
								{
									m_timer_inline_edit = (SetTimer(wnd, EDIT_TIMER_ID, GetDoubleClickTime(), 0) != 0);
								}
							}
						}

					}
					set_item_selected_single(m_selecting_start);
					//set_focus_item(m_selecting_start);
				}
			}
			if (m_selecting)
			{
				m_selecting = false;
				destroy_timer_scroll_down();
				destroy_timer_scroll_up();
			}
			else if (m_lbutton_down_ctrl)
			{
				const t_hit_test_result & hit_result = m_lbutton_down_hittest;
				if (wp & MK_CONTROL)
				{
					if (hit_result.result == hit_test_on || hit_result.result == hit_test_obscured_below || hit_result.result == hit_test_obscured_above)
					{
						if (m_selecting_start < m_items.get_count())
						{
							set_item_selected(m_selecting_start, !get_item_selected(m_selecting_start));
							set_focus_item(m_selecting_start);
						}
					}
					else if (hit_result.result == hit_test_on_group)
					{
						if (hit_result.index < m_items.get_count() && hit_result.group_level < m_group_count)
						{
							t_size index=0, count=0;
							get_item_group(hit_result.index, hit_result.group_level, index, count);
							if (count)
							{
								set_selection_state(bit_array_range(index, count), bit_array_range(index, count, !is_range_selected(index, count)), true, false);
								set_focus_item(index);
							}
						}
					}
				}
				m_lbutton_down_ctrl = false;
			}
			m_selecting_start = pfc_infinite;
			m_selecting_start_column = pfc_infinite;
			m_selecting_move = false;
			m_selecting_moved = false;
			m_inline_edit_prevent = false;
			if (GetCapture() == wnd)
				ReleaseCapture();
		}
		return 0;
	case WM_RBUTTONUP:
		{
			m_inline_edit_prevent = false;
			m_dragging_rmb = false;
			m_dragging_rmb_initial_point.x=0; m_dragging_rmb_initial_point.y=0;
		}
		break;
	case WM_RBUTTONDOWN:
		{
			SetFocus(wnd);

			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			t_hit_test_result hit_result;
			hit_test_ex(pt, hit_result);
			if (hit_result.result == hit_test_on || hit_result.result == hit_test_obscured_below || hit_result.result == hit_test_obscured_above)
			{
				m_dragging_rmb = true;
				m_dragging_rmb_initial_point = pt;

				if (hit_result.result == hit_test_obscured_below)
					scroll(true, SB_LINEDOWN);
				if (hit_result.result == hit_test_obscured_above)
					scroll(true, SB_LINEUP);

				if (!get_item_selected(hit_result.index))
				{
					if (m_single_selection)
					{
						set_focus_item(hit_result.index);
					}
					else
						set_item_selected_single(hit_result.index, true, notification_source_rmb);
				}
				else if (get_focus_item() != hit_result.index)
					set_focus_item(hit_result.index);
			}
			else if (hit_result.result == hit_test_on_group)
			{
				t_size index=0, count=0;
				get_item_group(hit_result.index, hit_result.group_level, index, count);
				set_selection_state(bit_array_true(), bit_array_range(index, count));
				if (count)
					set_focus_item(index);
			}
		}

		break;
	case WM_MOUSEMOVE:
		{
			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			if (!m_selecting)
			{
				if (m_show_tooltips && ( pt.y >=0 && (t_size)pt.y > get_items_top()))
				{
					t_hit_test_result hit_result;
					hit_test_ex(pt, hit_result);

					if ((hit_result.result == hit_test_on || hit_result.result == hit_test_obscured_below || hit_result.result == hit_test_obscured_above) && hit_result.column != -1)
					{
						if (m_tooltip_last_index != hit_result.index || m_tooltip_last_column != hit_result.column)
						{
							t_size cx = 0;
							{
								t_size i;
								//if (hit_result.column == 0)
									cx = get_total_indentation();
								//else
									for (i=0; i<hit_result.column; i++)
										cx += m_columns[i].m_display_size;
							}
							bool is_clipped = is_item_clipped(hit_result.index, hit_result.column);
							if (!m_limit_tooltips_to_clipped_items || is_clipped)
							{
								pfc::string8 temp;
								titleformat_compiler::remove_color_marks(get_item_text(hit_result.index,hit_result.column), temp);
								temp.replace_char(9, 0x20);
								if (temp.length() > 128)
								{
									temp.truncate (128);
									temp << "\xe2\x80\xa6";
								}
								create_tooltip(temp);

								POINT a;
								a.x = cx + 4 - m_horizontal_scroll_position;
								a.y = (get_item_position(hit_result.index)-m_scroll_position) + get_items_top();
								ClientToScreen(get_wnd(), &a);

								int text_cx = get_text_width(temp, temp.length());

								m_rc_tooltip.top = a.y + ((m_item_height-uGetFontHeight(m_font))/2);
								m_rc_tooltip.bottom = a.y + m_item_height;
								m_rc_tooltip.left = a.x;
								m_rc_tooltip.right = a.x + text_cx;//m_columns[hit_result.column].m_display_size;

								//if (!is_clipped)
								//{
									//if (m_columns[hit_result.column].m_alignment == ui_helpers::ALIGN_RIGHT)
										//m_rc_tooltip.left = m_rc_tooltip.right - ui_helpers::get_text_width(temp);
								//}
							}
							else destroy_tooltip();


						}
						m_tooltip_last_index = hit_result.index;
						m_tooltip_last_column = hit_result.column;
					}
					else destroy_tooltip();
				}
			}
			if (m_selecting_move || (m_single_selection && m_selecting) || m_dragging_rmb)
			{
				const unsigned cx_drag = (unsigned)abs(GetSystemMetrics(SM_CXDRAG));
				const unsigned cy_drag = (unsigned)abs(GetSystemMetrics(SM_CYDRAG));
				
				bool b_enter_drag = false;

				if (!m_dragging_rmb && (wp & MK_LBUTTON)
					&& (abs (m_dragging_initial_point.x - pt.x) > cx_drag || abs (m_dragging_initial_point.y - pt.y) > cy_drag))
					b_enter_drag = true;
				if (m_dragging_rmb && (wp & MK_RBUTTON)
					&& (abs (m_dragging_rmb_initial_point.x - pt.x) > cx_drag || abs (m_dragging_rmb_initial_point.y - pt.y) > cy_drag))
					b_enter_drag = true;
				
				if (b_enter_drag)
				{
					destroy_tooltip();
					if (do_drag_drop(wp))
					{
						m_selecting_moved = false;
						m_selecting_move = false;
						m_selecting = false;
						m_dragging_rmb = false;
						m_dragging_rmb_initial_point.x=0; m_dragging_rmb_initial_point.y=0;
					}
				}
			}
			if (m_selecting && !m_single_selection)
			{
				//t_size index;
				if (!m_selecting_move)
				{
					t_hit_test_result hit_result;
					hit_test_ex(pt, hit_result);
					{
						if (hit_result.result == hit_test_above || hit_result.result == hit_test_below ||hit_result.result == hit_test_on
							//|| hit_result.result == hit_test_obscured_below || hit_result.result == hit_test_obscured_above
							|| hit_result.result == hit_test_right_of_item || hit_result.result == hit_test_left_of_item
							|| hit_result.result == hit_test_right_of_group || hit_result.result == hit_test_left_of_group
							|| hit_result.result == hit_test_below_items|| hit_result.result == hit_test_on_group)
						{
							if (hit_result.result == hit_test_below)
							{
								create_timer_scroll_down();
							}
							else destroy_timer_scroll_down();

							if (hit_result.result == hit_test_above)
							{
								create_timer_scroll_up();
							}
							else destroy_timer_scroll_up();

							if (hit_result.result == hit_test_obscured_below)
								scroll(true, SB_LINEDOWN);
							if (hit_result.result == hit_test_obscured_above)
								scroll(true, SB_LINEUP);

							if (hit_result.result == hit_test_on_group)
							{
								if (hit_result.index > m_selecting_start)
									hit_result.index --;
							}
							if (hit_result.result == hit_test_below_items && hit_result.index < m_selecting_start && hit_result.index + 1 < get_item_count()) //Items removed whilst selecting.. messy
								hit_result.index++;

							/*if (m_selecting_move)
							{
								if (m_selecting_start!=hit_result.index)
								{
									m_selecting_moved = true;
									move_selection(hit_result.index-m_selecting_start);
									m_selecting_start=hit_result.index;
								}
							}
							else*/
							{
								if (get_focus_item()!=hit_result.index)
								{
									if (!is_visible(hit_result.index))
									{
										RECT rc;
										get_items_rect(&rc);
										if (hit_result.index > get_last_viewable_item()) 
											scroll(false,get_item_position(hit_result.index) - RECT_CY(rc) + m_item_height);
										else
											scroll(false,get_item_position(hit_result.index));
									}
									
									set_selection_state(bit_array_true(), bit_array_range(min(hit_result.index, m_selecting_start), (t_size)abs(int(m_selecting_start-hit_result.index))+1));
									set_focus_item(hit_result.index);
								}
							}
						}
					}
				}
			}
		}
		return 0;
	case WM_LBUTTONDBLCLK:
		{
			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			t_hit_test_result hit_result;
			hit_test_ex(pt, hit_result);
			if (hit_result.result == hit_test_on)
			{
				exit_inline_edit();
				m_inline_edit_prevent = true;
				t_size focus = get_focus_item();
				if (focus!=pfc_infinite)
					execute_default_action(focus, hit_result.column, false, (wp & MK_CONTROL) != 0);
				return 0;
			}
			else if (hit_result.result == hit_test_nowhere
				||hit_result.result ==hit_test_right_of_item
				||hit_result.result ==hit_test_right_of_group
				||hit_result.result ==hit_test_below_items)
					if (notify_on_doubleleftclick_nowhere())
						return 0;
		}
		break;
	case WM_MBUTTONDOWN:
		{
			SetFocus(wnd);
		}
		return 0;
	case WM_MBUTTONUP:
		{
			m_inline_edit_prevent = false;
			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			t_hit_test_result hit_result;
			hit_test_ex(pt, hit_result);
			if (notify_on_middleclick(hit_result.result == hit_test_on,hit_result.index))
				return 0;
		}
		break;
	case WM_MOUSEWHEEL:
		{
			LONG_PTR style = GetWindowLongPtr(get_wnd(),GWL_STYLE);
			bool b_horz = (!(style & WS_VSCROLL) || ((wp & MK_CONTROL))) && (style & WS_HSCROLL);

			SCROLLINFO si;
			memset(&si, 0, sizeof(SCROLLINFO));
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
			GetScrollInfo(get_wnd(), b_horz ? SB_HORZ : SB_VERT, &si);

			UINT scroll_lines = 3; //3 is default
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scroll_lines, 0); //we don't support Win9X

			if (!si.nPage) si.nPage++;

			if (scroll_lines == -1)
				scroll_lines = si.nPage-1;
			else
				scroll_lines *= m_item_height;

			int zDelta = short(HIWORD(wp));

			if (scroll_lines == 0)
				scroll_lines = 1;

			//console::formatter() << zDelta;

			int delta = -MulDiv(zDelta, scroll_lines, 120);

			// Limit scrolling to one page ?!?!?! It was in Columns Playlist code...
			if (delta < 0 && (UINT)(delta*-1) > si.nPage)
			{
				delta = si.nPage*-1;
				if (delta <-1) delta++;
			}
			else if (delta > 0 && (UINT)delta > si.nPage)
			{
				delta = si.nPage;
				if (delta >1) delta--;
			}

			exit_inline_edit();
			scroll(false, delta+(b_horz?m_horizontal_scroll_position : m_scroll_position), b_horz);

		}
		return 0;
	case WM_GETDLGCODE:
		return DefWindowProc(wnd, msg, wp, lp) | DLGC_WANTARROWS;
	case WM_SHOWWINDOW:
		if (wp == TRUE && lp == 0 && !m_shown)
		{
			t_size focus = get_focus_item();
			if (focus!=pfc_infinite)
				ensure_visible(focus);
			m_shown = true;
		}
		break;
	case WM_KEYDOWN:
		{
			//console::formatter() << "WM_KEYDOWN: " << (t_size)wp << " " << (t_size)lp;
			LRESULT ret=0;
			if (on_wm_keydown(wp, lp, ret, m_prevent_wm_char_processing))
				return ret;
		}
		break;
	case WM_CHAR:
		//console::formatter() << "WM_CHAR: " << (t_size)wp << " " << (t_size)lp;
		if (!m_prevent_wm_char_processing)
		{
			//if (!(HIWORD(lp) & KF_REPEAT))
			{
				if ((GetKeyState(VK_CONTROL) & KF_UP))
				{
					if (wp == 1) //Ctrl-A
					{
						if (!m_single_selection)
							set_selection_state(bit_array_true(), bit_array_true());
						return 0;
					}
					else if (wp == 26) //Ctrl-Z
					{
						if (notify_on_keyboard_keydown_undo())
							return 0;
					}
					else if (wp == 25) //Ctrl-Y
					{
						if (notify_on_keyboard_keydown_redo())
							return 0;
					}
					else if (wp == 24) //Ctrl-X
					{
						if (notify_on_keyboard_keydown_cut())
							return 0;
					}
					else if (wp == 3) //Ctrl-C
					{
						if (notify_on_keyboard_keydown_copy())
							return 0;
					}
					else if (wp == 6) //Ctrl-F
					{
						if (notify_on_keyboard_keydown_search())
							return 0;
					}
					else if (wp == 22) //Ctrl-V
					{
						if (notify_on_keyboard_keydown_paste())
							return 0;
					}
				}	
			}
			on_search_string_change(wp);
		}
		break;
	case WM_SYSKEYDOWN:
		{
			if (notify_on_keyboard_keydown_filter(WM_SYSKEYDOWN, wp, lp, m_prevent_wm_char_processing))
				return 0;
		}
		break;
	case WM_VSCROLL:
		{
			int val = LOWORD(wp);
			exit_inline_edit();
			scroll(true, LOWORD(wp));
		}
		return 0;
	case WM_HSCROLL:
		{
			int val = LOWORD(wp);
			exit_inline_edit();
			scroll(true, LOWORD(wp), true);
		}
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wp))
		{
		case 667:
			switch (HIWORD(wp))
			{
			case EN_KILLFOCUS:
				{
					HWND wnd_focus = GetFocus();
					if (!HWND(wnd_focus) || (HWND(wnd_focus) != wnd && !IsChild(wnd, wnd_focus)))
						notify_on_kill_focus(wnd_focus);
				}
				break;
			};
			break;
		case IDC_SEARCHBOX:
			switch (HIWORD(wp))
			{
			case EN_CHANGE:
				{
					notify_on_search_box_contents_change(string_utf8_from_window(HWND(lp)));
				}
				break;
			case EN_KILLFOCUS:
				{
					RedrawWindow(HWND(lp), NULL, NULL, RDW_INVALIDATE|RDW_ERASE|RDW_ERASENOW|RDW_UPDATENOW);
					HWND wnd_focus = GetFocus();
					if (!HWND(wnd_focus) || (HWND(wnd_focus) != wnd && !IsChild(wnd, wnd_focus)))
						notify_on_kill_focus(wnd_focus);
				}
				break;
			case EN_SETFOCUS:
				RedrawWindow(HWND(lp), NULL, NULL, RDW_INVALIDATE|RDW_ERASE|RDW_ERASENOW|RDW_UPDATENOW);
				break;
			};
			break;
		};
		break;
#if 1
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		if (lp && HWND(lp) == m_search_editbox)
		{
			/*POINT pt;
			GetMessagePos(&pt);
			HWND wnd_focus = GetFocus();

			bool b_hot = WindowFromPoint(pt) == m_search_editbox;*/
			bool b_focused = GetFocus() == m_search_editbox;

			if (b_focused)
				return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
			else if (m_search_box_hot)
				return (LRESULT)GetSysColorBrush(COLOR_WINDOW);//m_search_box_hot_brush.get();//GetSysColorBrush(COLOR_BTNFACE);
			else
				return (LRESULT)GetSysColorBrush(IsThemeActive() && IsAppThemed() ? COLOR_BTNFACE : COLOR_WINDOW);//m_search_box_nofocus_brush.get();//GetSysColorBrush(COLOR_3DLIGHT);
		}
		break;
#endif
	case WM_NOTIFY:
		if (m_wnd_header && ((LPNMHDR)lp)->hwndFrom == m_wnd_header)
		{
			LRESULT ret = 0;
			if (on_wm_notify_header((LPNMHDR)lp, ret))
				return ret;
		}
		else if (m_wnd_tooltip && ((LPNMHDR)lp)->hwndFrom == m_wnd_tooltip)
		{
			switch (((LPNMHDR)lp)->code)
			{
			case TTN_SHOW:
				{
					RECT rc = m_rc_tooltip;

					SendMessage(m_wnd_tooltip, TTM_ADJUSTRECT, TRUE, (LPARAM)&rc);

					SetWindowPos(m_wnd_tooltip, NULL, rc.left, rc.top, RECT_CX(rc), RECT_CY(rc), SWP_NOZORDER|SWP_NOACTIVATE);
					return TRUE;
				}
			}
		}
		break;
	case WM_TIMER:
		switch (wp)
		{
		case TIMER_END_SEARCH:
			{
				destroy_timer_search();
				m_search_string.reset();
			}
			return 0;
		case TIMER_SCROLL_UP:
			scroll(true, SB_LINEUP);
			return 0;
		case TIMER_SCROLL_DOWN:
			scroll(true, SB_LINEDOWN);
			return 0;
		case EDIT_TIMER_ID:
			{
				create_inline_edit(pfc::list_t<t_size>(m_inline_edit_indices), m_inline_edit_column);
				if (m_timer_inline_edit)
				{
					KillTimer(wnd, EDIT_TIMER_ID);
					m_timer_inline_edit = false;
				}
				return 0;
			}
		default:
			if (notify_on_timer(wp))
				return 0;
			break;
		};
		break;
	case MSG_KILL_INLINE_EDIT:
		exit_inline_edit();
		return 0;
	case WM_CONTEXTMENU:
		{
			POINT pt = {GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
			if (get_wnd() == (HWND)wp)
			{
				POINT px;
				{
					if (pt.x == -1 && pt.y == -1)
					{
						t_size focus = get_focus_item();
						unsigned last = get_last_viewable_item();
						if (focus >= m_items.get_count() || focus < get_next_item(m_scroll_position) || focus > last)
						{
							px.x=0;
							px.y=0;
						}
						else
						{
							RECT rc;
							get_items_rect(&rc);
							px.x=0;
							px.y=(get_item_position(focus)-m_scroll_position) + m_item_height/2 + rc.top;
						}
						pt = px;
						MapWindowPoints(wnd, HWND_DESKTOP, &pt, 1);
					}
					else
					{
						px = pt;
						ScreenToClient(wnd, &px);
						RECT rc;
						GetClientRect(wnd, &rc);
						if (!PtInRect(&rc, px))
							break;
					}

				}
				if (notify_on_contextmenu(pt))
					return 0;
			}
			else if (m_wnd_header && (HWND)wp == m_wnd_header)
			{
				POINT temp;
				temp.x = pt.x;
				temp.y = pt.y;
				ScreenToClient(m_wnd_header, &temp);
				HDHITTESTINFO hittest;
				hittest.pt.x = temp.x;
				hittest.pt.y = temp.y;
				uSendMessage(m_wnd_header, HDM_HITTEST, 0, (LPARAM)&hittest);

				if (notify_on_contextmenu_header(pt, hittest))
					return 0;
			}
		}
		break;
	}
	return DefWindowProc(wnd, msg, wp, lp);
}
