#include "stdafx.h"

LRESULT album_list_window::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{

	switch (msg)
	{
	case WM_CREATE:
	{
		list_wnd.add_item(this);

		initialised = true;

		modeless_dialog_manager::g_add(wnd);

		create_tree();
		create_filter();

		if (cfg_populate) refresh_tree();

		static_api_ptr_t<library_manager_v3>()->register_callback(this);
	}
	break;
	/*case WM_GETMINMAXINFO:
	{
	LPMINMAXINFO mmi = LPMINMAXINFO(lp);
	mmi->ptMinTrackSize.y = cfg_height;
	return 0;
	}*/
	case WM_SIZE:
		on_size(LOWORD(lp), HIWORD(lp));
		break;
		/*	case DM_GETDEFID:
		return (DC_HASDEFID<<16|IDOK);
		case WM_GETDLGCODE:
		return DLGC_DEFPUSHBUTTON;*/
		//		break;
	case WM_TIMER:
		if (wp == EDIT_TIMER_ID)
		{
			refresh_tree();
			KillTimer(wnd, wp);
			m_timer = false;
		}
		break;
	case WM_COMMAND:
		switch (wp)
		{
		case IDC_FILTER | (EN_CHANGE << 16) :
			if (m_timer)
				KillTimer(wnd_edit, 500);
			m_timer = SetTimer(wnd, EDIT_TIMER_ID, 500, NULL) != 0;
			return TRUE;
		case IDOK:
			if (GetKeyState(VK_SHIFT) & KF_UP) do_playlist(p_selection, false);
			else if (GetKeyState(VK_CONTROL) & KF_UP) do_playlist(p_selection, true, true);
			else do_playlist(p_selection, true);
			return 0;
		}
		break;
	case WM_CONTEXTMENU:
	{
		enum { ID_SEND = 1, ID_ADD, ID_NEW, ID_AUTOSEND, ID_REFRESH, ID_FILT, ID_CONF, ID_VIEW_BASE };

		HMENU menu = CreatePopupMenu();

		POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
		service_ptr_t<contextmenu_manager> p_menu_manager;

		unsigned IDM_MANAGER_BASE = 0;

		HWND list = wnd_tv;

		HTREEITEM treeitem = NULL;

		TVHITTESTINFO ti;
		memset(&ti, 0, sizeof(ti));

		if (pt.x != -1 && pt.y != -1)
		{
			ti.pt = pt;
			ScreenToClient(list, &ti.pt);
			uSendMessage(list, TVM_HITTEST, 0, (long)&ti);
			if (ti.hItem && (ti.flags & TVHT_ONITEM))
			{
				//FIX THIS AND AUTOSEND
				//TreeView_Select(list, ti.hItem, TVGN_DROPHILITE);
				//uSendMessage(list,TVM_SELECTITEM,TVGN_DROPHILITE,(long)ti.hItem);
				treeitem = ti.hItem;
			}
		}
		else
		{
			treeitem = TreeView_GetSelection(list);
			RECT rc;
			if (treeitem && TreeView_GetItemRect(wnd_tv, treeitem, &rc, TRUE))
			{
				MapWindowPoints(wnd_tv, HWND_DESKTOP, (LPPOINT)&rc, 2);

				pt.x = rc.left;
				pt.y = rc.top + (rc.bottom - rc.top) / 2;

			}
			else
			{
				GetMessagePos(&pt);
			}
		}

		TreeView_Select(list, treeitem, TVGN_DROPHILITE);

		HMENU menu_view = CreatePopupMenu();
		unsigned n, m = cfg_view_list.get_count();
		string8_fastalloc temp;
		temp.prealloc(32);

		uAppendMenu(menu_view, MF_STRING | (!stricmp_utf8(directory_structure_view_name, view) ? MF_CHECKED : 0), ID_VIEW_BASE + 0, directory_structure_view_name);

		list_t<string_simple, pfc::alloc_fast> views;

		views.add_item(string_simple(directory_structure_view_name));

		for (n = 0; n<m; n++)
		{
			temp = cfg_view_list.get_name(n);
			string_simple item(temp.get_ptr());

			if (item)
			{
				uAppendMenu(menu_view, MF_STRING | (!stricmp_utf8(temp, view) ? MF_CHECKED : 0), ID_VIEW_BASE + views.add_item(item), temp);
			}

		}


		IDM_MANAGER_BASE = ID_VIEW_BASE + views.get_count();

		uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)menu_view, "View");

		if (!m_populated && !cfg_populate)
			uAppendMenu(menu, MF_STRING, ID_REFRESH, "Populate");
		uAppendMenu(menu, MF_STRING | (m_filter ? MF_CHECKED : 0), ID_FILT, "Filter");
		uAppendMenu(menu, MF_STRING, ID_CONF, "Settings");

		bool show_shortcuts = standard_config_objects::query_show_keyboard_shortcuts_in_menus();

		node * p_node = NULL;
		TVITEMEX tvi;
		memset(&tvi, 0, sizeof(tvi));
		tvi.hItem = treeitem;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		TreeView_GetItem(list, &tvi);
		p_node = (node*)tvi.lParam;

		if (treeitem && p_node)
		{
			uAppendMenu(menu, MF_SEPARATOR, 0, "");
			uAppendMenu(menu, MF_STRING, ID_SEND, (show_shortcuts ? "&Send to playlist\tEnter" : "&Send to playlist"));
			uAppendMenu(menu, MF_STRING, ID_ADD, show_shortcuts ? "&Add to playlist\tShift+Enter" : "&Add to playlist");
			uAppendMenu(menu, MF_STRING, ID_NEW, show_shortcuts ? "Send to &new playlist\tCtrl+Enter" : "Send to &new playlist");
			uAppendMenu(menu, MF_STRING, ID_AUTOSEND, "Send to &autosend playlist");

			uAppendMenu(menu, MF_SEPARATOR, 0, "");

			contextmenu_manager::g_create(p_menu_manager);
			p_node->sort_entries();

			if (p_menu_manager.is_valid())
			{
				p_menu_manager->init_context(p_node->get_entries(), 0);

				p_menu_manager->win32_build_menu(menu, IDM_MANAGER_BASE, -1);
				menu_helpers::win32_auto_mnemonics(menu);
			}
		}

		int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, get_wnd(), 0);
		DestroyMenu(menu);

		TreeView_Select(list, NULL, TVGN_DROPHILITE);

		if (cmd)
		{
			if (p_menu_manager.is_valid() && (unsigned)cmd >= IDM_MANAGER_BASE)
			{
				p_menu_manager->execute_by_id(cmd - IDM_MANAGER_BASE);
			}
			else if (cmd >= ID_VIEW_BASE)
			{
				unsigned n = cmd - ID_VIEW_BASE;
				if (n<views.get_count())
				{
					view = views[n].get_ptr();
					refresh_tree();
				}
			}
			else if (cmd<ID_VIEW_BASE)
			{
				unsigned cmd2 = 0;
				switch (cmd)
				{
				case ID_NEW:
					do_playlist(p_node, true, true);
					break;
				case ID_SEND:
					do_playlist(p_node, true);
					break;
				case ID_ADD:
					do_playlist(p_node, false);
					break;
				case ID_AUTOSEND:
					do_autosend_playlist(p_node, view, true);
					break;
				case ID_CONF:
				{
					static_api_ptr_t<ui_control>()->show_preferences(g_guid_preferences_album_list_panel);
				}
				break;
				case ID_FILT:
				{
					m_filter = !m_filter;
					create_or_destroy_filter();
				}
				break;
				case ID_REFRESH:
					if (!m_populated && !cfg_populate)
						refresh_tree();
					break;
				}
				if (cmd2) uSendMessage(get_wnd(), WM_COMMAND, cmd2, 0);
			}
		}

		p_menu_manager.release();

		/*			if (treeitem_context && (treeitem_context != treeitem) && cfg_autosend)
		TreeView_SelectItem(wnd_tv,treeitem);*/


	}
	return 0;
	case WM_NOTIFY:
	{
		LPNMHDR hdr = (LPNMHDR)lp;

		switch (hdr->idFrom)
		{

		case IDC_TREE:
		{
			if (hdr->code == TVN_ITEMEXPANDING)
			{
				LPNMTREEVIEW param = (LPNMTREEVIEW)hdr;
				if (cfg_picmixer && (param->action == TVE_EXPAND))
				{
					TreeView_CollapseOtherNodes(param->hdr.hwndFrom, param->itemNew.hItem);
				}
			}

			else if (hdr->code == TVN_SELCHANGED)
			{
				LPNMTREEVIEW param = (LPNMTREEVIEW)hdr;

				p_selection = (node*)param->itemNew.lParam;
				if ((param->action == TVC_BYMOUSE || param->action == TVC_BYKEYBOARD))
				{
					if (cfg_autosend)
						do_autosend_playlist(p_selection, view);
				}
				if (m_selection_holder.is_valid())
				{
					m_selection_holder->set_selection(p_selection.is_valid() ? p_selection->get_entries() : metadb_handle_list());
				}
#if 0
				if (cfg_picmixer)
				{
					HTREEITEM ti_parent_old = TreeView_GetParent(param->hdr.hwndFrom, param->itemOld.hItem);
					HTREEITEM ti_parent_new = TreeView_GetParent(param->hdr.hwndFrom, param->itemNew.hItem);

					if (/*ti_parent_old != param->itemNew.hItem &&  */!TreeView_IsChild(param->hdr.hwndFrom, param->itemNew.hItem, param->itemOld.hItem))
					{
						HTREEITEM ti = //TreeView_GetLevel(param->hdr.hwndFrom, param->itemNew.hItem) < TreeView_GetLevel(param->hdr.hwndFrom, param->itemOld.hItem) ? 
							TreeView_GetCommonParentChild(param->hdr.hwndFrom, param->itemOld.hItem, param->itemNew.hItem)
							//: param->itemOld.hItem
							;
						if (ti && ti != TVI_ROOT) TreeView_Expand(param->hdr.hwndFrom, ti, TVE_COLLAPSE);
					}

					if (ti_parent_new)
					{

						HTREEITEM child = TreeView_GetChild(param->hdr.hwndFrom, ti_parent_new);
						while (child)
						{
							if (child != param->itemNew.hItem)
							{

							}
						}
					}
				}
#endif
			}
		}
		break;
		}

	}
	break;
	case WM_DESTROY:
		static_api_ptr_t<library_manager_v3>()->unregister_callback(this);
		modeless_dialog_manager::g_remove(wnd);
		destroy_tree();
		destroy_filter();
		m_selection_holder.release();
		m_root.release();
		p_selection.release();
		if (initialised)
		{
			list_wnd.remove_item(this);
			if (list_wnd.get_count() == 0)
			{
				DeleteFont(g_font);
				g_font = 0;
			}
			initialised = false;
		}
		break;
	}
	return DefWindowProc(wnd, msg, wp, lp);
}
