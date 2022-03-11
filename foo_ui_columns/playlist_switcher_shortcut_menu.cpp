#include "pch.h"

#include "common.h"
#include "playlist_switcher_v2.h"

namespace cui::panels::playlist_switcher {

bool PlaylistSwitcher::notify_on_contextmenu(const POINT& pt, bool from_keyboard)
{
    uie::window_ptr p_this_temp = this;
    HMENU menu = CreatePopupMenu();

    t_size index = get_focus_item();

    playlist_position_reference_tracker indexTracked(false);
    indexTracked.m_playlist = index;

    t_size num = m_playlist_api->get_playlist_count();
    t_size active = m_playlist_api->get_active_playlist();
    bool b_index_valid = index < num;

    if (b_index_valid)
        set_highlight_selected_item(index);

    const auto autoplaylist_api = autoplaylist_manager::get();
    autoplaylist_client_v2::ptr autoplaylist;

    try {
        autoplaylist_client::ptr ptr = autoplaylist_api->query_client(index);
        ptr->service_query_t(autoplaylist);
    } catch (pfc::exception const&) {
    }

    metadb_handle_list_t<pfc::alloc_fast_aggressive> data;

    if (b_index_valid) {
        data.prealloc(m_playlist_api->playlist_get_item_count(index));
        m_playlist_api->playlist_get_all_items(index, data);

        if (data.get_count())
            AppendMenu(menu, MF_STRING, ID_PLAY, _T("Play"));
        if (active != index)
            AppendMenu(menu, MF_STRING, ID_SWITCH, _T("Activate"));
        AppendMenu(menu, MF_STRING, ID_RENAME, _T("Rename"));
        AppendMenu(menu, MF_STRING, ID_REMOVE, _T("Remove"));
        /*if (index>0)
            AppendMenu(menu,MF_STRING,ID_UP,_T("Move up"));
        if (index+1<num)
            AppendMenu(menu,MF_STRING,ID_DOWN,_T("Move down"));*/
        if (autoplaylist.is_valid() && autoplaylist->show_ui_available()) {
            AppendMenu(menu, MF_SEPARATOR, 0, nullptr);

            pfc::string8 name;
            autoplaylist->get_display_name(name);
            name << " properties";

            uAppendMenu(menu, MF_STRING, ID_AUTOPLAYLIST, name);
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
    if (b_index_valid) {
        AppendMenu(menu, MF_STRING, ID_SAVE, _T("Save as..."));
    }

    if (num)
        AppendMenu(menu, MF_STRING, ID_SAVE_ALL, _T("Save all as..."));
    pfc::array_t<unsigned> recycler_ids;
    {
        const auto recycler_count
            = gsl::narrow<unsigned>(std::min(m_playlist_api->recycler_get_count(), size_t{UINT32_MAX}));
        if (recycler_count) {
            recycler_ids.set_count(recycler_count);
            HMENU recycler_popup = CreatePopupMenu();
            pfc::string8_fast_aggressive temp;
            for (t_size i = 0; i < recycler_count; i++) {
                m_playlist_api->recycler_get_name(i, temp);
                recycler_ids[i] = m_playlist_api->recycler_get_id(i); // Menu Message Loop !
                uAppendMenu(recycler_popup, MF_STRING, ID_RECYCLER_BASE + i, temp);
            }
            AppendMenu(recycler_popup, MF_SEPARATOR, 0, nullptr);
            AppendMenu(recycler_popup, MF_STRING, ID_RECYCLER_CLEAR, _T("Clear"));
            AppendMenu(menu, MF_POPUP, (UINT_PTR)recycler_popup, _T("History"));
        }
        m_contextmenu_manager_base = ID_RECYCLER_BASE + recycler_count;
    }
    if (b_index_valid) {
        MENUITEMINFO mi{};
        mi.cbSize = sizeof(MENUITEMINFO);
        mi.fMask = MIIM_STATE;
        mi.fState = MFS_DEFAULT;

        if (data.get_count() || active != index)
            SetMenuItemInfo(menu, data.get_count() ? ID_PLAY : ID_SWITCH, FALSE, &mi);
    }

    if (data.get_count() > 0) {
        AppendMenu(menu, MF_SEPARATOR, 0, nullptr);

        HMENU submenu = CreatePopupMenu();

        contextmenu_manager::g_create(m_contextmenu_manager);

        if (m_contextmenu_manager.is_valid() && submenu) {
            m_contextmenu_manager->init_context(data, 0);

            m_contextmenu_manager->win32_build_menu(submenu, m_contextmenu_manager_base, -1);
        }
        AppendMenu(menu, MF_POPUP, (UINT_PTR)submenu, _T("Items"));
    }
    menu_helpers::win32_auto_mnemonics(menu);

    const auto cmd = static_cast<unsigned>(
        TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, get_wnd(), nullptr));
    m_status_text_override.release();

    DestroyMenu(menu);

    remove_highlight_selected_item();

    index = indexTracked.m_playlist;

    if (cmd > 0) {
        if ((t_size)cmd >= m_contextmenu_manager_base) {
            if (m_contextmenu_manager.is_valid()) {
                m_contextmenu_manager->execute_by_id(cmd - m_contextmenu_manager_base);
            }
        } else if (cmd >= ID_RECYCLER_BASE) {
            if (t_size(cmd - ID_RECYCLER_BASE) < recycler_ids.get_count())
                m_playlist_api->recycler_restore_by_id(recycler_ids[cmd - ID_RECYCLER_BASE]);
        } else {
            switch (cmd) {
            case ID_PLAY:
                m_playlist_api->set_playing_playlist(index);
                play_control::get()->start();
                break;
            case ID_AUTOPLAYLIST:
                if (autoplaylist.is_valid())
                    autoplaylist->show_ui(index);
                break;
            case ID_RECYCLER_CLEAR:
                m_playlist_api->recycler_purge(bit_array_true());
                break;
            case ID_CUT:
                if (b_index_valid)
                    playlist_manager_utils::cut(pfc::list_single_ref_t<t_size>(index));
                break;
            case ID_COPY:
                if (b_index_valid)
                    playlist_manager_utils::copy(pfc::list_single_ref_t<t_size>(index));
                break;
            case ID_PASTE:
                if (b_index_valid)
                    playlist_manager_utils::paste(get_wnd(), index + 1);
                break;
            case ID_SWITCH:
                if (b_index_valid)
                    m_playlist_api->set_active_playlist(index);
                break;
            case ID_REMOVE:
                if (b_index_valid)
                    m_playlist_api->remove_playlist_switch(index);
                break;
            case ID_RENAME:
                if (b_index_valid) {
                    activate_inline_editing(index, 0);
                }
                break;
            case ID_NEW: {
                m_playlist_api->create_playlist(
                    pfc::string8("Untitled"), pfc_infinite, m_playlist_api->get_playlist_count());
            } break;
            case ID_SAVE: {
                pfc::string8 name;
                m_playlist_api->playlist_get_name(index, name);
                g_save_playlist(get_wnd(), data, name);
                // standard_commands::main_save_playlist();
            } break;
            case ID_LOAD: {
                standard_commands::main_load_playlist();
            } break;
            case ID_SAVE_ALL: {
                standard_commands::main_save_all_playlists();
            } break;
            case ID_UP:
                if (index > 0) {
                    order_helper order(num);
                    order.swap(index, index - 1);
                    m_playlist_api->reorder(order.get_ptr(), num);
                }
                break;
            case ID_DOWN:
                if (index + 1 < num) {
                    order_helper order(num);
                    order.swap(index, index + 1);
                    m_playlist_api->reorder(order.get_ptr(), num);
                }
                break;
            }
        }
    }
    m_contextmenu_manager.release();
    m_contextmenu_manager_base = NULL;

    /*t_size index_active = m_playlist_api->get_active_playlist();
    if (index_active != pfc_infinite && index_active < get_item_count())
    {
        set_item_selected_single(index_active, false);
    }
    else
        set_selection_state(pfc::bit_array_true(), pfc::bit_array_false(), false);*/

    return true;
}

} // namespace cui::panels::playlist_switcher
