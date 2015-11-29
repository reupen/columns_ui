#include "stdafx.h"

LRESULT WINAPI album_list_window::hook_proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	album_list_window * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<album_list_window*>(uGetWindowLong(wnd, GWL_USERDATA));

	rv = p_this ? p_this->on_hook(wnd, msg, wp, lp) : uDefWindowProc(wnd, msg, wp, lp);

	return rv;
}

static bool test_point_distance(DWORD pt1, DWORD pt2, int test)
{
	int dx = (short)LOWORD(pt1) - (short)LOWORD(pt2),
		dy = (short)HIWORD(pt1) - (short)HIWORD(pt2);
	return dx*dx + dy*dy > test*test;
}

LRESULT WINAPI album_list_window::on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static bool process_char = true;
	uie::window_ptr p_this;

	switch (msg)
	{
	case WM_KEYDOWN:
	{
		p_this = this;
		metadb_handle_list dummy;
		if (get_host()->get_keyboard_shortcuts_enabled() && wp != VK_LEFT && wp != VK_RIGHT && cfg_keyb && g_process_keydown_keyboard_shortcuts(wp)) { process_char = false; break; }
		else if (wp == VK_TAB)
		{
			ui_extension::window::g_on_tab(wnd);
		}
		process_char = true;
	}
	break;
	case WM_SYSKEYDOWN:
	{
		p_this = this;
		metadb_handle_list dummy;
		if (get_host()->get_keyboard_shortcuts_enabled() && wp != VK_LEFT && wp != VK_RIGHT && cfg_keyb && g_process_keydown_keyboard_shortcuts(wp)) { process_char = false; break; }
		process_char = true;
	}
	break;
	case  WM_CHAR:
		if (cfg_keyb && !process_char)
		{
			process_char = true;
			return 0;
		}
		break;
	case WM_SETFOCUS:
	{
		m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
		if (p_selection.is_valid())
			m_selection_holder->set_selection(p_selection->get_entries());
	}
	break;
	case WM_KILLFOCUS:
	{
		m_selection_holder.release();
	}
	break;
	case WM_RBUTTONUP:
		//SendMessage(wnd,TVM_SELECTITEM,TVGN_DROPHILITE,NULL);
		break;
	case WM_RBUTTONDOWN:
	{
		/*POINT pt = {(short)LOWORD(lp),(short)HIWORD(lp)};
		TVHITTESTINFO ti;
		memset(&ti,0,sizeof(ti));
		ti.pt = pt;
		SendMessage(wnd,TVM_HITTEST,0,(LPARAM)&ti);
		if ((ti.flags & TVHT_ONITEM) && ti.hItem) uSendMessage(wnd,TVM_SELECTITEM,TVGN_DROPHILITE,(LPARAM)ti.hItem);*/
	}
	break;
	case WM_GETDLGCODE:
	{
		MSG * msg = (LPMSG)lp;
		if (msg && cfg_keyb)
		{
			// let dialog manager handle it, otherwise to kill ping we have to process WM_CHAR to return 0 on wp == 0xd and 0xa
			if (!((msg->message == WM_KEYDOWN || msg->message == WM_KEYUP) && (msg->wParam == VK_RETURN || msg->wParam == VK_TAB))) return DLGC_WANTMESSAGE;
		}

		/*			if (cfg_keyb)
		;*///return DLGC_DEFPUSHBUTTON;
	}
	break;
	case WM_LBUTTONUP:
		clicked = false;
		break;
	case WM_LBUTTONDOWN:
		clicked = true;
		clickpoint = lp;
		break;
	case WM_MOUSEMOVE:
	{
		if (!(wp&MK_LBUTTON)/* || (wp&MK_RBUTTON)*/) { dragging = false; clicked = false; }
		else if (!dragging && clicked && test_point_distance(clickpoint, lp, 5))
		{
			TVHITTESTINFO ti;
			memset(&ti, 0, sizeof(ti));
			ti.pt.x = (short)LOWORD(clickpoint);
			ti.pt.y = (short)HIWORD(clickpoint);
			uSendMessage(wnd, TVM_HITTEST, 0, (LPARAM)&ti);

			if (ti.flags & TVHT_ONITEM)
			{
				//					HTREEITEM item = TreeView_GetSelection(wnd);
				if (ti.hItem) uSendMessage(wnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)ti.hItem);

				if (p_selection.is_valid())
				{
					static_api_ptr_t<playlist_incoming_item_filter> incoming_api;
					metadb_handle_list_t<pfc::alloc_fast_aggressive> items;
					if (cfg_add_items_use_core_sort)
					{
						incoming_api->filter_items(p_selection->get_entries(), items);
					}
					else
					{
						p_selection->sort_entries();
						items = p_selection->get_entries();
					}
					dragging = true;
					IDataObject * pDataObject = static_api_ptr_t<playlist_incoming_item_filter>()->create_dataobject(items);
					if (pDataObject)
					{
						DWORD blah;
						pfc::com_ptr_t<IDropSource_albumlist> p_IDropSource_albumlist = new IDropSource_albumlist(wnd_tv);
						DoDragDrop(pDataObject, p_IDropSource_albumlist.get_ptr(), DROPEFFECT_COPY, &blah);
						pDataObject->Release();
					}
					dragging = false;
					clicked = false;
				}

				//					uSendMessage(wnd,TVM_SELECTITEM,TVGN_CARET,(long)item);
			}
		}
	}
	break;
	case WM_MBUTTONDOWN:
	{
		TVHITTESTINFO ti;
		memset(&ti, 0, sizeof(ti));

		ti.pt.x = GET_X_LPARAM(lp);
		ti.pt.y = GET_Y_LPARAM(lp);
		uSendMessage(wnd, TVM_HITTEST, 0, (long)&ti);
		if (ti.flags & TVHT_ONITEM)
		{
			int action = cfg_middle;
			if (action)
			{
				POINT pt = { (short)LOWORD(lp),(short)HIWORD(lp) };
				TVHITTESTINFO ti;
				memset(&ti, 0, sizeof(ti));
				ti.pt = pt;
				uSendMessage(wnd, TVM_HITTEST, 0, (LPARAM)&ti);
				if (ti.hItem)
				{
					uSendMessage(wnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)ti.hItem);
					switch (action)
					{
					case 1:
						do_playlist(p_selection, true);
						break;
					case 2:
						do_playlist(p_selection, false);
						break;
					case 3:
						do_playlist(p_selection, true, true);
						break;
					case 4:
						do_autosend_playlist(p_selection, view, true);
						break;
					}

				}
			}
		}
	}

	break;
	case WM_LBUTTONDBLCLK:
		if (p_selection.is_valid() && p_selection->get_num_entries()>0)
		{
			TVHITTESTINFO ti;
			memset(&ti, 0, sizeof(ti));

			ti.pt.x = GET_X_LPARAM(lp);
			ti.pt.y = GET_Y_LPARAM(lp);
			uSendMessage(wnd, TVM_HITTEST, 0, (long)&ti);
			if (ti.flags & TVHT_ONITEM)
			{

				switch (cfg_dblclk)
				{
				case 0:
					if (p_selection == 0 || p_selection->get_num_children()>0) break;
					do_playlist(p_selection, true);
					return 0;
				case 1:
					do_playlist(p_selection, true);
					return 0;
				case 2:
					do_playlist(p_selection, false);
					return 0;
				case 3:
					do_playlist(p_selection, true, true);
					return 0;
				case 4:
					do_autosend_playlist(p_selection, view, true);
					return 0;
				}
				break;
			}
		}
	}
	return uCallWindowProc(treeproc, wnd, msg, wp, lp);
}
