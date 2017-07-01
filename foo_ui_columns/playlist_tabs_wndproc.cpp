#include "stdafx.h"
#include "playlist_tabs.h"
#include "playlist_manager_utils.h"
#include "main_window.h"

enum { ID_SWITCH = 1, ID_REMOVE, ID_RENAME, ID_NEW, ID_SAVE, ID_SAVE_ALL, ID_LOAD, ID_UP, ID_DOWN, ID_CUT, ID_COPY, ID_PASTE, ID_AUTOPLAYLIST, ID_RECYCLER_CLEAR, ID_RECYCLER_BASE };

LRESULT playlists_tabs_extension::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_NCCREATE:
    {
        m_host_wnd = wnd;
        break;
    }
    case WM_CREATE:
    {
        initialised = true;
        list_wnd.add_item(this);
        pfc::com_ptr_t< playlists_tabs_drop_target > m_drop_target = new playlists_tabs_drop_target(this);
        RegisterDragDrop(wnd, m_drop_target.get_ptr());

        create_tabs();

        service_ptr_t<service_base> p_temp;

        g_tab_host.instance_create(p_temp);

        //Well simple reinterpret_cast without this mess should work fine but this is "correct"
        m_host = static_cast<window_host_impl*>(p_temp.get_ptr());
        if (m_host.is_valid())
        {
            m_host->set_this(this);
            create_child();
        }
        static_api_ptr_t<playlist_manager>()->register_callback(this, playlist_callback::flag_all);
        break;
    }

    case WM_SHOWWINDOW:
    {
        if (wp == TRUE && lp == NULL && !IsWindowVisible(m_child_wnd))
        {
            ShowWindow(m_child_wnd, SW_SHOWNORMAL);
        }
        break;
    }
    case WM_WINDOWPOSCHANGED:
    {
        LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE))
        {
            on_size(lpwp->cx, lpwp->cy);
        }
    }
    break;
    case MSG_RESET_SIZE_LIMITS:
        on_child_position_change();
        break;
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpmmi = LPMINMAXINFO(lp);
        *lpmmi = mmi;
    }
    return 0;
    case WM_DESTROY:
    {
        static_api_ptr_t<playlist_manager>()->unregister_callback(this);
        destroy_child();
        m_host.release();
        RevokeDragDrop(wnd);
        if (wnd_tabs)
            DestroyWindow(wnd_tabs);
        wnd_tabs = nullptr;
        list_wnd.remove_item(this);
        if (!list_wnd.get_count())
        {
            SendMessage(wnd, WM_SETFONT, 0, 0);
            if (g_font) DeleteObject(g_font);
            g_font = nullptr;
        }
        initialised = false;
    }
    break;
    case WM_NCDESTROY:
    {
        m_host_wnd = nullptr;
    }
    break;
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONUP:
    {
        if (wnd_tabs)
        {

            POINT temp;
            temp.x = GET_X_LPARAM(lp);
            temp.y = GET_Y_LPARAM(lp);

            HWND wnd_hit = ChildWindowFromPointEx(wnd, temp, CWP_SKIPINVISIBLE);

            if (wnd_hit == wnd_tabs)
            {
                TCHITTESTINFO hittest;
                hittest.pt.x = temp.x;
                hittest.pt.y = temp.y;
                int idx = TabCtrl_HitTest(wnd_tabs, &hittest);

                if (idx < 0)
                {
                    static_api_ptr_t<playlist_manager> playlist_api;
                    unsigned new_idx = playlist_api->create_playlist("Untitled", 12, playlist_api->get_playlist_count());
                    playlist_api->set_active_playlist(new_idx);
                    return 0;
                }
            }
        }
    }
    break;
    case WM_TIMER:
        if (wp == SWITCH_TIMER_ID)
        {
            m_switch_timer = 0;
            KillTimer(wnd, SWITCH_TIMER_ID);
            if (!m_playlist_switched)
            {
                static_api_ptr_t<playlist_manager> playlist_api;
                if (m_switch_playlist < playlist_api->get_playlist_count())
                    playlist_api->set_active_playlist(m_switch_playlist);

                m_playlist_switched = true;
            }
            return 0;
        }
        break;
    case WM_MENUSELECT:
    {
        if (HIWORD(wp) & MF_POPUP)
        {
            m_status_override.release();
        }
        else
        {
            if (p_manager.is_valid())
            {
                unsigned id = LOWORD(wp);

                bool set = true;

                pfc::string8 blah;

                if (id >= ID_CUSTOM_BASE)
                {
                    ::contextmenu_node * node = p_manager->find_by_id(id - ID_CUSTOM_BASE);
                    if (node) set = node->get_description(blah);
                }
                else if (id == ID_SWITCH)
                {
                    blah = "Activates this playlist.";
                }
                else if (id == ID_REMOVE)
                {
                    blah = "Removes this playlist.";
                }
                else if (id == ID_RENAME)
                {
                    blah = "Renames this playlist.";
                }
                else if (id == ID_NEW)
                {
                    blah = "Creates a new playlist.";
                }
                else if (id == ID_LOAD)
                {
                    blah = "Loads an existing playlist from a file.";
                }
                else if (id == ID_SAVE_ALL)
                {
                    blah = "Saves all playlists to individual files.";
                }
                else if (id == ID_SAVE)
                {
                    blah = "Saves this playlist to a file.";
                }
                else if (id == ID_UP)
                {
                    blah = "Moves this playlist up one position.";
                }
                else if (id == ID_DOWN)
                {
                    blah = "Moves this playlist down one position.";
                }
                else if (id == ID_COPY)
                {
                    blah = "Copies the selected items to the Clipboard.";
                }
                else if (id == ID_CUT)
                {
                    blah = "Removes the selected items and copies them to the Clipboard.";
                }
                else if (id == ID_PASTE)
                {
                    blah = "Inserts the items you have copied or cut to the selected location.";
                }
                else if (id == ID_AUTOPLAYLIST)
                {
                    blah = "Open autoplaylist properties.";
                }
                else
                    set = false;

                service_ptr_t<ui_status_text_override> p_status_override;

                if (set)
                {
                    get_host()->override_status_text_create(p_status_override);

                    if (p_status_override.is_valid())
                    {
                        p_status_override->override_text(blah);
                    }
                }
                m_status_override = p_status_override;
            }
        }
        break;
    }
    case WM_CONTEXTMENU:
        if (wnd_tabs)
        {
            uie::window_ptr p_this_temp = this;
            POINT pt = { (short)LOWORD(lp), (short)HIWORD(lp) };
            int old_idx = 0;
            unsigned idx = 0;

            bool b_keyb_invoked = pt.x == -1 && pt.y == -1;

            POINT pt_client = pt;
            ScreenToClient(wnd_tabs, &pt_client);

            RECT rc;
            if (m_child_wnd)
            {
                GetClientRect(m_child_wnd, &rc);
                MapWindowPoints(m_child_wnd, wnd_tabs, (LPPOINT)&rc, 2);
            }
            if (!b_keyb_invoked && m_child_wnd && m_child.is_valid() && PtInRect(&rc, pt_client))
            {
                pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> menu_hook_t = new ui_extension::menu_hook_impl;
                m_child->get_menu_items(*menu_hook_t.get_ptr());
                HMENU menu = CreatePopupMenu();
                menu_hook_t->win32_build_menu(menu, 1, pfc_infinite);
                int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, nullptr);
                menu_hook_t->execute_by_id(cmd);
                DestroyMenu(menu);
            }
            else
            {
                if (b_keyb_invoked)
                {
                    idx = TabCtrl_GetCurSel(wnd_tabs);
                    RECT rc_sel;
                    if (TabCtrl_GetItemRect(wnd_tabs, idx, &rc_sel))
                    {
                        MapWindowPoints(wnd_tabs, HWND_DESKTOP, (LPPOINT)&rc_sel, 2);
                        pt.x = rc_sel.left + (rc_sel.right - rc_sel.left) / 2;
                        pt.y = rc_sel.top + (rc_sel.bottom - rc_sel.top) / 2;
                    }
                    else GetMessagePos(&pt);

                }
                else
                {
                    TCHITTESTINFO hittest;
                    hittest.pt.x = pt_client.x;
                    hittest.pt.y = pt_client.y;
                    idx = TabCtrl_HitTest(wnd_tabs, &hittest);
                }


                static_api_ptr_t<playlist_manager_v3> playlist_api;

                unsigned num = playlist_api->get_playlist_count(), active = playlist_api->get_active_playlist();
                bool b_index_valid = idx<num;

                metadb_handle_list_t<pfc::alloc_fast_aggressive> data;

                static_api_ptr_t<autoplaylist_manager> autoplaylist_api;
                autoplaylist_client_v2::ptr autoplaylist;

                try
                {
                    autoplaylist_client::ptr ptr = autoplaylist_api->query_client(idx);
                    ptr->service_query_t(autoplaylist);
                }
                catch (pfc::exception const &) {};

                HMENU menu = CreatePopupMenu();

                playlist_position_reference_tracker position_tracker(false);
                position_tracker.m_playlist = idx;

                if (b_index_valid)
                {
                    if (active != idx)
                        AppendMenu(menu, MF_STRING, ID_SWITCH, _T("Activate"));
                    AppendMenu(menu, MF_STRING, ID_RENAME, _T("Rename..."));
                    AppendMenu(menu, MF_STRING, ID_REMOVE, _T("Remove"));
                    if (idx>0)
                        AppendMenu(menu, MF_STRING, ID_UP, _T("Move left"));
                    if (idx + 1<num)
                        AppendMenu(menu, MF_STRING, ID_DOWN, _T("Move right"));
                    if (autoplaylist.is_valid() && autoplaylist->show_ui_available())
                    {
                        AppendMenu(menu, MF_SEPARATOR, 0, nullptr);

                        pfc::string8 name;
                        autoplaylist->get_display_name(name);
                        name << " properties";

                        AppendMenu(menu, MF_STRING, ID_AUTOPLAYLIST, uT(name));
                    }
                    AppendMenu(menu, MF_SEPARATOR, 0, nullptr);

                    AppendMenu(menu, MF_STRING, ID_CUT, L"Cut");
                    AppendMenu(menu, MF_STRING, ID_COPY, L"Copy");
                    if (playlist_manager_utils::check_clipboard())
                        AppendMenu(menu, MF_STRING, ID_PASTE, L"Paste");
                    AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
                }

                AppendMenu(menu, MF_STRING, ID_NEW, _T("New"));
                AppendMenu(menu, MF_STRING, ID_LOAD, _T("Load..."));
                if (b_index_valid)
                {
                    AppendMenu(menu, MF_STRING, ID_SAVE, _T("Save as..."));
                }

                if (num)
                    AppendMenu(menu, MF_STRING, ID_SAVE_ALL, _T("Save all as..."));
                pfc::array_t<t_size> recycler_ids;
                {
                    t_size recycler_count = playlist_api->recycler_get_count();
                    if (recycler_count)
                    {
                        recycler_ids.set_count(recycler_count);
                        HMENU recycler_popup = CreatePopupMenu();
                        pfc::string8_fast_aggressive temp;
                        t_size i;
                        for (i = 0; i<recycler_count; i++)
                        {
                            playlist_api->recycler_get_name(i, temp);
                            recycler_ids[i] = playlist_api->recycler_get_id(i); //Menu Message Loop !
                            uAppendMenu(recycler_popup, MF_STRING, ID_RECYCLER_BASE + i, temp);
                        }
                        AppendMenu(recycler_popup, MF_SEPARATOR, 0, nullptr);
                        AppendMenu(recycler_popup, MF_STRING, ID_RECYCLER_CLEAR, _T("Clear"));
                        AppendMenu(menu, MF_POPUP, (UINT_PTR)recycler_popup, _T("History"));
                    }
                    ID_CUSTOM_BASE = ID_RECYCLER_BASE + recycler_count;
                }
                if (b_index_valid)
                {

                    data.prealloc(playlist_api->playlist_get_item_count(idx));
                    playlist_api->playlist_get_all_items(idx, data);

                    MENUITEMINFO mi;
                    memset(&mi, 0, sizeof(mi));
                    mi.cbSize = sizeof(MENUITEMINFO);
                    mi.fMask = MIIM_STATE;
                    mi.fState = MFS_DEFAULT;

                    SetMenuItemInfo(menu, (active != idx) ? ID_SWITCH : ID_RENAME, FALSE, &mi);
                }


                if (data.get_count() >0)
                {
                    uAppendMenu(menu, MF_SEPARATOR, 0, nullptr);

                    HMENU submenu = CreatePopupMenu();

                    contextmenu_manager::g_create(p_manager);
                    if (p_manager.is_valid())
                    {
                        p_manager->init_context(data, 0);

                        p_manager->win32_build_menu(submenu, ID_CUSTOM_BASE, -1);
                    }
                    AppendMenu(menu, MF_POPUP, (UINT_PTR)submenu, _T("Items"));
                }
                menu_helpers::win32_auto_mnemonics(menu);

                int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, nullptr);

                m_status_override.release();

                DestroyMenu(menu);


                num = playlist_api->get_playlist_count();
                active = playlist_api->get_active_playlist();
                b_index_valid = idx<num;

                if (cmd)
                {
                    if (cmd >= ID_CUSTOM_BASE)
                    {
                        if (p_manager.is_valid())
                        {
                            p_manager->execute_by_id(cmd - ID_CUSTOM_BASE);
                        }
                    }
                    else if (cmd >= ID_RECYCLER_BASE)
                    {
                        if (cmd - ID_RECYCLER_BASE < recycler_ids.get_count())
                            playlist_api->recycler_restore_by_id(recycler_ids[cmd - ID_RECYCLER_BASE]);
                    }
                    else
                    {
                        switch (cmd)
                        {
                        case ID_AUTOPLAYLIST:
                            if (autoplaylist.is_valid())
                                autoplaylist->show_ui(position_tracker.m_playlist);
                            break;
                        case ID_RECYCLER_CLEAR:
                            playlist_api->recycler_purge(bit_array_true());
                            break;
                        case ID_CUT:
                            if (b_index_valid) playlist_manager_utils::cut(pfc::list_single_ref_t<t_size>(idx));
                            break;
                        case ID_COPY:
                            if (b_index_valid) playlist_manager_utils::copy(pfc::list_single_ref_t<t_size>(idx));
                            break;
                        case ID_PASTE:
                            if (b_index_valid) playlist_manager_utils::paste(wnd, idx + 1);
                            break;
                        case ID_SWITCH:
                            if (b_index_valid)
                            {
                                playlist_api->set_active_playlist(idx);
                                old_idx = idx;
                            }
                            break;
                        case ID_REMOVE:
                            if (b_index_valid) remove_playlist_helper(idx);
                            break;
                        case ID_RENAME:
                            if (b_index_valid)
                            {
                                pfc::string8 temp;
                                if (playlist_api->playlist_get_name(idx, temp))
                                {
                                    if (g_rename_dialog(&temp, wnd))
                                    {//fucko: dialogobx has a messgeloop, someone might have called switcher api funcs in the meanwhile
                                        //            idx = ((HWND)wp == wnd_tabs) ? idx : SendMessage(g_plist,LB_GETCURSEL,0,0);
                                        num = playlist_api->get_playlist_count();
                                        if ((signed)idx >= 0 && idx<num)
                                        {
                                            playlist_api->playlist_rename(idx, temp, -1);
                                        }
                                    }
                                }
                            }
                            break;
                        case ID_NEW:
                        {
                            metadb_handle_list data;
                            playlist_api->playlist_add_items(playlist_api->create_playlist(pfc::string8("Untitled"), -1, playlist_api->get_playlist_count()), data, bit_array_false());
                        }
                        break;
                        case ID_SAVE:
                        {
                            pfc::string8 name;
                            playlist_api->playlist_get_name(idx, name);
                            g_save_playlist(wnd, data, name);
                        }
                        break;
                        case ID_LOAD:
                        {
                            standard_commands::main_load_playlist();
                        }
                        break;
                        case ID_SAVE_ALL:
                        {
                            standard_commands::main_save_all_playlists();
                        }
                        break;
                        case ID_UP:
                            if (idx>0)
                            {
                                order_helper order(num);
                                order.swap(idx, idx - 1);
                                playlist_api->reorder(order.get_ptr(), num);
                            }
                            break;
                        case ID_DOWN:
                            if (idx + 1<num)
                            {
                                order_helper order(num);
                                order.swap(idx, idx + 1);
                                playlist_api->reorder(order.get_ptr(), num);
                            }
                            break;
                        }
                    }
                }
                p_manager.release();
                ID_CUSTOM_BASE = NULL;
                data.remove_all();
            }
        }
        return 0;
    case WM_NOTIFY:
    {
        switch (((LPNMHDR)lp)->idFrom)
        {
        case 5002:
            switch (((LPNMHDR)lp)->code)
            {
            case TCN_SELCHANGE:
            {
                static_api_ptr_t<playlist_manager>()->set_active_playlist(TabCtrl_GetCurSel(((LPNMHDR)lp)->hwndFrom));
            }
            break;
            }
            break;
        }
    }
    break;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}
