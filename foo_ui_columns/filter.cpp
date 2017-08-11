#include "stdafx.h"
#include "filter.h"
#include "filter_search_bar.h"

HRESULT g_get_comctl32_vresion(DLLVERSIONINFO2 & p_dvi);

namespace filter_panel {

    cfg_fields_t cfg_field_list(guid_cfg_fields);

    //cfg_string cfg_fields(g_guid_cfg_fields, "Genre;Artist;Album");
    cfg_string cfg_sort_string(g_guid_cfg_sort_string, "%album artist% - %album% - %discnumber% - %tracknumber% - %title%");
    cfg_bool cfg_sort(g_guid_cfg_sort, true);
    cfg_bool cfg_autosend(g_guid_cfg_autosend, true);
    cfg_bool cfg_orderedbysplitters(g_guid_orderedbysplitters, true);
    cfg_bool cfg_showemptyitems(g_guid_showemptyitems, false);
    cfg_int cfg_doubleclickaction(g_guid_doubleclickaction, 1);
    cfg_int cfg_middleclickaction(g_guid_middleclickaction, 0);
    cfg_int cfg_edgestyle(g_guid_edgestyle, 2);
    fbh::ConfigInt32DpiAware cfg_vertical_item_padding(g_guid_itempadding, 4);
    fbh::ConfigBool cfg_show_column_titles(g_guid_show_column_titles, true);
    fbh::ConfigBool cfg_allow_sorting(g_guid_allow_sorting, true);
    fbh::ConfigBool cfg_show_sort_indicators(g_guid_show_sort_indicators, true);

    cfg_bool cfg_showsearchclearbutton(g_guid_showsearchclearbutton, true);

    cfg_favouriteslist cfg_favourites(g_guid_favouritequeries);



    bool filter_stream_t::is_visible()
    {
        for (t_size i = 0, count = m_windows.get_count(); i<count; i++)
            if (!m_windows[i]->is_visible()) return false;
        return true;
    }

    void filter_panel_t::set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort)
    {
        if (p_size)
        {
            t_size version;
            p_reader->read_lendian_t(version, p_abort);
            if (version <= config_version_current)
            {
                p_reader->read_string(m_field_data.m_name, p_abort);
                if (version >= 1)
                {
                    p_reader->read_lendian_t(m_show_search, p_abort);
                    try {
                        p_reader->read_lendian_t(m_pending_sort_direction, p_abort);
                    } catch (const exception_io_data_truncation &) {
                        
                    }
                }
            }
        }
    };
    void filter_panel_t::get_config(stream_writer * p_writer, abort_callback & p_abort) const
    {
        p_writer->write_lendian_t(t_size(config_version_current), p_abort);
        p_writer->write_string(m_field_data.m_name, p_abort);
        p_writer->write_lendian_t(m_show_search, p_abort);
        p_writer->write_lendian_t(get_sort_direction(), p_abort);
        /*p_writer->write_lendian_t(m_field_data.m_use_script, p_abort);
        p_writer->write_string(m_field_data.m_script, p_abort);
        t_uint32 count=m_field_data.m_fields.get_count(), i;
        p_writer->write_lendian_t(count, p_abort);
        for (i=0; i<count; i++)
        p_writer->write_string(m_field_data.m_fields[i], p_abort);*/
    };

    t_size filter_panel_t::g_get_stream_index_by_window(const uie::window_ptr & ptr)
    {
        t_size i, count = g_streams.get_count();
        for (i = 0; i<count; i++)
        {
            t_size j, subcount = g_streams[i]->m_windows.get_count();
            for (j = 0; j<subcount; j++)
                if (g_streams[i]->m_windows[j] == ptr.get_ptr())
                    return i;
        }
        return pfc_infinite;
    }
    void filter_panel_t::g_on_orderedbysplitters_change()
    {
        g_streams.remove_all();
        for (auto & window : g_windows)
        {
            window->refresh_stream();
        }
        //filter_search_bar::g_on_orderedbysplitters_change();
        t_size count = g_streams.get_count();
        for (t_size i = 0; i<count; i++)
        {
            if (g_streams[i]->m_windows.get_count())
            {
                filter_search_bar::g_initialise_filter_stream(g_streams[i]);
                pfc::list_t<filter_panel_t*> windows;
                g_streams[i]->m_windows[0]->get_windows(windows);
                if (windows.get_count())
                    g_update_subsequent_filters(windows, 0, false, false);
            }
        }
    }


    void filter_panel_t::g_on_fields_change()
    {
        g_load_fields();
        for (auto & window : g_windows)
        {
            t_size field_index = window->get_field_index();
            window->set_field(field_index == pfc_infinite ? field_data_t() : g_field_data[field_index], true);
        }
    }
    t_size filter_panel_t::g_get_field_index_by_name(const char * p_name)
    {
        t_size i, count = g_field_data.get_count();
        for (i = 0; i<count; i++)
        {
            if (!strcmp(g_field_data[i].m_name, p_name))
                return i;
        }
        return pfc_infinite;
    }

    void filter_panel_t::g_on_field_title_change(const char * p_old, const char * p_new)
    {
        t_size field_index = g_get_field_index_by_name(p_old);
        if (field_index != pfc_infinite)
        {
            for (auto & window : g_windows)
            {
                if (window->get_field_index() == field_index)
                {
                    window->m_field_data.m_name = p_new;
                    window->refresh_columns();
                    window->update_first_node_text(true);
                }
            }
            g_field_data[field_index].m_name = p_new;
        }
    }

    void filter_panel_t::g_on_vertical_item_padding_change()
    {
        for (auto & window : g_windows)
            window->set_vertical_item_padding(cfg_vertical_item_padding);
    }

    void filter_panel_t::g_on_show_column_titles_change()
    {
        for (auto & window : g_windows)
            window->set_show_header(cfg_show_column_titles);
    }

    void filter_panel_t::g_on_allow_sorting_change()
    {
        for (auto & window : g_windows)
            window->set_sorting_enabled(cfg_allow_sorting);
    }

    void filter_panel_t::g_on_show_sort_indicators_change()
    {
        for (auto & window : g_windows)
            window->set_show_sort_indicators(cfg_show_sort_indicators);
    }

    void filter_panel_t::g_on_field_query_change(const field_t & field)
    {
        t_size field_index = g_get_field_index_by_name(field.m_name);
        if (field_index != pfc_infinite)
        {
            g_create_field_data(field, g_field_data[field_index]);
            t_size i, count = g_streams.get_count();
            for (i = 0; i<count; i++)
            {
                if (g_streams[i]->m_windows.get_count())
                {
                    pfc::list_t<filter_panel_t*> windows;
                    g_streams[i]->m_windows[0]->get_windows(windows);
                    t_size j, subcount = windows.get_count();

                    for (j = 0; j<subcount; j++)
                    {
                        if (windows[j]->get_field_index() == field_index) //meh
                        {
                            windows[j]->set_field(g_field_data[field_index], true);
                            break;
                        }
                    }
                }
            }
        }
    }
    void filter_panel_t::g_on_showemptyitems_change(bool b_val)
    {
        if (g_windows.size())
        {
            g_showemptyitems = b_val;
            t_size i, count = g_streams.get_count();
            for (i = 0; i<count; i++)
            {
                if (g_streams[i]->m_windows.get_count())
                {
                    pfc::list_t<filter_panel_t*> windows;
                    g_streams[i]->m_windows[0]->get_windows(windows);
                    t_size j = windows.get_count();
                    if (windows.get_count())
                        g_update_subsequent_filters(windows, 0, false, false);

                }
            }
        }
    }
    void filter_panel_t::g_on_edgestyle_change()
    {
        for (auto & window : g_windows)
        {
            window->set_edge_style(cfg_edgestyle);
        }
    }
    void filter_panel_t::g_on_font_items_change()
    {
        LOGFONT lf;
        static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_filter_items_font_client, lf);
        for (auto & window : g_windows)
        {
            window->set_font(&lf);
        }
    }


    void filter_panel_t::g_on_font_header_change()
    {
        LOGFONT lf;
        static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_filter_header_font_client, lf);
        for (auto & window : g_windows)
        {
            window->set_header_font(&lf);
        }
    }
    void filter_panel_t::g_redraw_all()
    {
        for (auto & window : g_windows)
            RedrawWindow(window->get_wnd(), nullptr, nullptr, RDW_UPDATENOW | RDW_INVALIDATE);
    }
    void filter_panel_t::g_on_new_field(const field_t & field)
    {
        if (g_windows.size())
        {
            t_size index = g_field_data.get_count();
            g_field_data.set_count(index + 1);
            g_create_field_data(field, g_field_data[index]);
        }
    }
    void filter_panel_t::g_on_fields_swapped(t_size index_1, t_size index_2)
    {
        if (max(index_1, index_2) < g_field_data.get_count())
            g_field_data.swap_items(index_1, index_2);
        if (!cfg_orderedbysplitters)
        {
            t_size i, count = g_streams.get_count();
            for (i = 0; i<count; i++)
            {
                if (g_streams[i]->m_windows.get_count())
                {
                    pfc::list_t<filter_panel_t*> windows;
                    g_streams[i]->m_windows[0]->get_windows(windows);
                    t_size j, subcount = windows.get_count();
                    for (j = 0; j<subcount; j++)
                    {
                        t_size this_index = windows[j]->get_field_index();
                        if (this_index == index_1 || this_index == index_2)
                        {
                            g_update_subsequent_filters(windows, j, false, false);
                            break;
                        }
                        if (this_index > max(index_1, index_2))
                            break;
                    }
                }
            }
        }
    }
    void filter_panel_t::g_on_field_removed(t_size index)
    {
        t_size i, count = g_streams.get_count();
        for (i = 0; i<count; i++)
        {
            if (g_streams[i]->m_windows.get_count())
            {
                pfc::list_t<filter_panel_t*> windows;
                g_streams[i]->m_windows[0]->get_windows(windows);
                t_size j, subcount = windows.get_count();
                bool b_found = false; t_size index_found = pfc_infinite;
                for (j = 0; j<subcount; j++)
                {
                    t_size this_index = windows[j]->get_field_index();
                    if (index == this_index)
                    {
                        windows[j]->set_field(field_data_t());
                        if (!b_found)
                        {
                            index_found = j;
                            b_found = true;
                        }
                    }
                    if (this_index > index)
                        break;
                }
                if (b_found)
                    g_update_subsequent_filters(windows, index_found, false, false);
            }
        }
        if (index < g_field_data.get_count())
            g_field_data.remove_by_idx(index);
    }

    void filter_panel_t::refresh(bool b_allow_autosend)
    {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
        get_initial_handles(data);
        populate_list_from_chain(data, false);
        update_subsequent_filters(b_allow_autosend);
    }


    t_size filter_panel_t::get_field_index()
    {
        t_size i, count = g_field_data.get_count(), ret = pfc_infinite;
        for (i = 0; i<count; i++)
            if (!stricmp_utf8(g_field_data[i].m_name, m_field_data.m_name))
            {
                ret = i;
                break;
            }
        return ret;
    }

    void filter_panel_t::get_windows(pfc::list_base_t<filter_panel_t*> & windows)
    {
        if (cfg_orderedbysplitters)
        {
            pfc::list_t<uie::window_ptr> siblings;
            uie::window_host_ex::ptr hostex;
            if (get_host()->service_query_t(hostex))
                hostex->get_children(siblings);
            else
                siblings.add_item(this);
            pfc::list_t<t_size> indices;
            t_size i, count = siblings.get_count();
            for (i = 0; i<count; i++)
            {
                t_size index = pfc_infinite;
                if ((index = m_stream->m_windows.find_item(static_cast<filter_panel_t*>(siblings[i].get_ptr()))) != pfc_infinite) //meh
                    windows.add_item(m_stream->m_windows[index]);
            }
        }
        else
        {
            pfc::list_t<t_size> indices;
            t_size i, count = m_stream->m_windows.get_count();
            for (i = 0; i<count; i++)
            {
                t_size index = m_stream->m_windows[i]->get_field_index();
                if (index != pfc_infinite)
                {
                    indices.add_item(index);
                    windows.add_item(m_stream->m_windows[i]);
                }
            }
            mmh::Permuation permutation(windows.get_count());
            mmh::sort_get_permuation(indices.get_ptr(), permutation, (pfc::compare_t<t_size, t_size>), true, false);
            windows.reorder(permutation.get_ptr());
        }
    }

    filter_panel_t* filter_panel_t::get_next_window()
    {
        pfc::list_t<filter_panel_t*> windows;
        get_windows(windows);
        const auto window_index = windows.find_item(this);
        const auto window_count = windows.get_count();
        return window_index + 1 < window_count ? windows[window_index + 1] : nullptr;
    }

    void filter_panel_t::g_create_field_data(const field_t & field, field_data_t & p_out)
    {
        p_out.m_name = field.m_name;
        p_out.m_last_sort_direction = field.m_last_sort_direction;
        if (strchr(field.m_field, '$') || strchr(field.m_field, '%'))
        {
            p_out.m_use_script = true;
            p_out.m_script = field.m_field;
            p_out.m_fields.remove_all();
        }
        else
        {
            p_out.m_use_script = false;
            p_out.m_script.reset();
            p_out.m_fields.remove_all();
            const char * ptr = field.m_field;
            while (*ptr)
            {
                const char * start = ptr;
                while (*ptr && *ptr != ';')
                    ptr++;
                if (ptr>start)
                    p_out.m_fields.add_item(pfc::string8(start, ptr - start));
                while (*ptr == ';') ptr++;
            }
        }
    }

    void filter_panel_t::g_load_fields()
    {
        t_size i, count = filter_panel::cfg_field_list.get_count();
        g_field_data.set_count(count);
        for (i = 0; i<count; i++)
        {
            const field_t & field = filter_panel::cfg_field_list[i];
            g_create_field_data(field, g_field_data[i]);
        }
    }
    void filter_panel_t::g_update_subsequent_filters(const pfc::list_base_const_t<filter_panel_t* > & windows, t_size index, bool b_check_needs_update, bool b_update_playlist)
    {
        t_size i, count = windows.get_count();
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;

        {
            //pfc::hires_timer timer;
            //timer.start();
            for (i = index; i<count; i++)
            {
                handles.remove_all();
                windows[i]->get_initial_handles(handles);
                if (b_check_needs_update)
                {

                    metadb_handle_list items1(windows[i]->m_nodes[0].m_handles);
                    handles.sort_by_pointer();
                    items1.sort_by_pointer();
                    if (!pfc::comparator_array<>::compare(items1, handles))
                    {
                        b_update_playlist = false;
                        break;
                    }

                }
                windows[i]->populate_list_from_chain(handles, false);
            }
            //console::formatter() << "total populate: " << timer.query() << " s";
        }
            {
                //pfc::hires_timer timer;
                //timer.start();
                if (count && b_update_playlist && cfg_autosend)
                    windows[count - 1]->send_results_to_playlist();
                //console::formatter() << "send: " << timer.query() << " s";
            }
    }

    void filter_panel_t::update_subsequent_filters(bool b_allow_autosend)
    {
        pfc::ptr_list_t<filter_panel_t> windows;
        get_windows(windows);
        t_size pos = windows.find_item(this);

        if (pos != pfc_infinite)
        {
            g_update_subsequent_filters(windows, pos + 1, false, b_allow_autosend);
        }
    }

    void filter_panel_t::get_initial_handles(metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_out)
    {
        pfc::ptr_list_t<filter_panel_t> windows;
        get_windows(windows);
        t_size pos = windows.find_item(this);
        if (pos && pos != pfc_infinite)
        {
            windows[pos - 1]->get_selection_handles(p_out);
        }
        else
        {
            if (m_stream->m_source_overriden)
                p_out = m_stream->m_source_handles;
            else
                static_api_ptr_t<library_manager>()->get_all_items(p_out);
        }
    }

    void filter_panel_t::set_field(const field_data_t & field, bool b_force)
    {
        if (b_force || stricmp_utf8(field.m_name, m_field_data.m_name))
        {
            m_pending_sort_direction = field.m_last_sort_direction;

            pfc::ptr_list_t<filter_panel_t> windows_before;
            get_windows(windows_before);
            t_size pos_before = windows_before.find_item(this);
            if (pos_before != pfc_infinite)
            {
            }
            m_field_data = field;
            bool b_redraw = disable_redrawing();
            clear_all_items();
            refresh_columns();
            metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
            get_initial_handles(handles);
            populate_list(handles);
            pfc::ptr_list_t<filter_panel_t> windows_after;
            get_windows(windows_after);
            t_size pos_after = windows_after.find_item(this);
            t_size pos_update = min(pos_before, pos_after);
            if (b_redraw)
                enable_redrawing();
            //update_window();
            g_update_subsequent_filters(windows_after, pos_update, false, false);
        }
    }

    void filter_panel_t::notify_update_item_data(t_size index)
    {
        auto & subitems = get_item_subitems(index);
        subitems.set_size(1);

        subitems[0] = pfc::stringcvt::string_utf8_from_wide(m_nodes[index].m_value);
    }

    t_size filter_panel_t::get_highlight_item()
    {
        return pfc_infinite;
    }

    bool filter_panel_t::notify_on_keyboard_keydown_search()
    {
        return filter_search_bar::g_activate();
    }

    bool filter_panel_t::do_drag_drop(WPARAM wp)
    {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
        get_selection_handles(data);
        if (data.get_count() > 0)
        {
            if (cfg_sort)
            {
                service_ptr_t<titleformat_object> to;
                static_api_ptr_t<titleformat_compiler>()->compile_safe(to, cfg_sort_string);
                fbh::sort_metadb_handle_list_by_format(data, to, nullptr);
            }
            static_api_ptr_t<playlist_incoming_item_filter> incoming_api;
            IDataObject * pDataObject = incoming_api->create_dataobject(data);
            if (pDataObject)
            {
                m_drag_item_count = data.get_count();
                DWORD blah = DROPEFFECT_NONE;
                HRESULT hr = uih::ole::do_drag_drop(get_wnd(), wp, pDataObject, DROPEFFECT_COPY | DROPEFFECT_MOVE, DROPEFFECT_COPY, &blah);
                pDataObject->Release();
                m_drag_item_count = 0;
            }
        }
        return true;
    }

    bool filter_panel_t::notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp, bool & b_processed)
    {
        uie::window_ptr p_this = this;
        bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
        b_processed = ret;
        return ret;
    };

    void filter_panel_t::get_selection_handles(metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_out, bool fallback , bool b_sort )
    {
        bool b_found = false;
        t_size i, count = m_nodes.get_count();
        if (count)
            p_out.prealloc(count);
        for (i = 0; i<count; i++)
        {
            if (get_item_selected(i))
            {
                b_found = true;
                if (b_sort)
                    m_nodes[i].ensure_handles_sorted();
                p_out.add_items(m_nodes[i].m_handles);
            }
        }
        if (!b_found)
        {
            if (fallback)
            {
                if (b_sort)
                    m_nodes[0].ensure_handles_sorted();
                p_out.add_items(m_nodes[0].m_handles);
            }
        }
        else
        {
            fbh::metadb_handle_list_remove_duplicates(p_out);
        }
    }


    void filter_panel_t::do_selection_action(action_t action )
    {
        bit_array_bittable mask(m_nodes.get_count());
        get_selection_state(mask);
        //metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        //handles.prealloc(m_nodes.get_count());
        //get_selection_handles(handles);
        do_items_action(mask, action);
    }
    void filter_panel_t::do_items_action(const bit_array & p_nodes, action_t action)
    {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        handles.prealloc(m_nodes.get_count());
        t_size i, count = m_nodes.get_count();
        for (i = 0; i<count; i++)
            if (p_nodes[i]) handles.add_items(m_nodes[i].m_handles);

        if (!handles.get_count())
            return;

        fbh::metadb_handle_list_remove_duplicates(handles);

        static_api_ptr_t<playlist_manager_v3> playlist_api;
        static_api_ptr_t<play_control> playback_api;
        t_size index_insert = pfc_infinite;
        if (action == action_send_to_autosend && playback_api->is_playing())
        {
            t_size playlist = playlist_api->get_playing_playlist();
            pfc::string8 name;
            if (playlist_api->playlist_get_name(playlist, name) && !stricmp_utf8("Filter Results", name))
            {
                t_size index_old = playlist_api->find_playlist("Filter Results (Playback)", pfc_infinite);
                playlist_api->playlist_rename(playlist, "Filter Results (Playback)", pfc_infinite);
                index_insert = index_old < playlist ? playlist : playlist + 1;
                if (index_old != pfc_infinite)
                    playlist_api->remove_playlist(index_old);
            }
        }
        pfc::string8 playlist_name;
        t_size index = NULL;
        if (action == action_add_to_active)
        {
            index = playlist_api->get_active_playlist();
            playlist_api->playlist_undo_backup(index);
        }
        else
        {
            if (action == action_send_to_autosend)
                playlist_name = "Filter Results";
            else if (action == action_send_to_autosend_play)
                playlist_name = "Filter Results (Playback)";
            else if (action == action_send_to_new || action == action_send_to_new_play)
            {
                for (i = 0; i<count; i++)
                {
                    if (p_nodes[i])
                    {
                        if (playlist_name.get_length())
                            playlist_name << ", ";
                        playlist_name << pfc::stringcvt::string_utf8_from_wide(m_nodes[i].m_value);
                    }
                }
            }
            //t_size index_remove = playlist_api->find_playlist("Filter Results", pfc_infinite);

            if (index_insert != pfc_infinite)
                index = playlist_api->create_playlist(playlist_name, pfc_infinite, index_insert);
            else
                index = playlist_api->find_or_create_playlist(playlist_name, pfc_infinite);
            playlist_api->playlist_undo_backup(index);
            playlist_api->playlist_clear(index);
        }
#if 1
        if (cfg_sort)
        {
            service_ptr_t<titleformat_object> to;
            static_api_ptr_t<titleformat_compiler>()->compile_safe(to, cfg_sort_string);
            fbh::sort_metadb_handle_list_by_format(handles, to, nullptr);
        }
        if (action != action_add_to_active)
            playlist_api->playlist_add_items(index, handles, bit_array_false());
        else
        {
            playlist_api->playlist_clear_selection(index);
            playlist_api->playlist_add_items(index, handles, bit_array_true());
        }
        playlist_api->playlist_set_focus_item(index, playlist_api->playlist_get_item_count(index) - handles.get_count());
#else
        playlist_api->playlist_add_items_filter(index, handles, false);
#endif
        if (action != action_add_to_active)
        {
            playlist_api->set_active_playlist(index);
            if (action == action_send_to_autosend_play || action == action_send_to_new_play)
            {
                playlist_api->set_playing_playlist(index);
                playback_api->play_start(play_control::track_command_default);
            }
        }
    }
    
    void filter_panel_t::execute_default_action(t_size index, t_size column, bool b_keyboard, bool b_ctrl)
    {
        action_t action = static_cast<action_t>(cfg_doubleclickaction.get_value());
        do_selection_action(action);
    }
    
    bool filter_panel_t::notify_on_middleclick(bool on_item, t_size index)
    {
        if (cfg_middleclickaction && on_item && index < m_nodes.get_count())
        {
            action_t action = static_cast<action_t>(cfg_middleclickaction.get_value() - 1);
            do_items_action(bit_array_one(index), action);
            return true;
        }
        return false;
    }

    void filter_panel_t::send_results_to_playlist(bool b_play )
    {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        handles.prealloc(m_nodes.get_count());
        get_selection_handles(handles);
        static_api_ptr_t<playlist_manager> playlist_api;
        static_api_ptr_t<play_control> playback_api;
        t_size index_insert = pfc_infinite;
        if (!b_play && playback_api->is_playing())
        {
            t_size playlist = playlist_api->get_playing_playlist();
            pfc::string8 name;
            if (playlist_api->playlist_get_name(playlist, name) && !stricmp_utf8("Filter Results", name))
            {
                t_size index_old = playlist_api->find_playlist("Filter Results (Playback)", pfc_infinite);
                playlist_api->playlist_rename(playlist, "Filter Results (Playback)", pfc_infinite);
                index_insert = index_old < playlist ? playlist : playlist + 1;
                if (index_old != pfc_infinite)
                    playlist_api->remove_playlist(index_old);
            }
        }
        //t_size index_remove = playlist_api->find_playlist("Filter Results", pfc_infinite);
        t_size index = NULL;
        if (index_insert != pfc_infinite)
            index = playlist_api->create_playlist(b_play ? "Filter Results (Playback)" : "Filter Results", pfc_infinite, index_insert);
        else
            index = playlist_api->find_or_create_playlist(b_play ? "Filter Results (Playback)" : "Filter Results", pfc_infinite);
        playlist_api->playlist_clear(index);
#if 1
        if (cfg_sort)
        {
            service_ptr_t<titleformat_object> to;
            static_api_ptr_t<titleformat_compiler>()->compile_safe(to, cfg_sort_string);
            {
                fbh::sort_metadb_handle_list_by_format(handles, to, nullptr);
            }
        }
        playlist_api->playlist_add_items(index, handles, bit_array_false());
#else
        playlist_api->playlist_add_items_filter(index, handles, false);
#endif
        playlist_api->set_active_playlist(index);
        if (b_play)
        {
            playlist_api->set_playing_playlist(index);
            playback_api->play_start(play_control::track_command_default);
        }
        //if (index_remove != pfc_infinite)
        //    playlist_api->remove_playlist(index+1);
    }
    void filter_panel_t::notify_on_selection_change(const bit_array & p_affected, const bit_array & p_status, notification_source_t p_notification_source)
    {
        if (p_notification_source != notification_source_rmb)
        {
            //send_results_to_playlist();
            update_subsequent_filters();
            if (m_selection_holder.is_valid())
            {
                metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
                get_selection_handles(handles, false);
                m_selection_holder->set_selection(handles);
            }
        }
    }
    void filter_panel_t::update_first_node_text(bool b_update)
    {
        t_size nodes_count = m_nodes.get_count();
        if (nodes_count)
        {
            nodes_count -= 1;
            pfc::string8 temp;
            temp << "All";
            if (m_field_data.m_name.length())
            {
                temp << " (" << nodes_count << " " << m_field_data.m_name;
                if (nodes_count != 1)
                    temp << "s";
                temp << ")";
            }
            m_nodes[0].m_value.set_string(pfc::stringcvt::string_wide_from_utf8(temp));
            if (b_update)
                update_items(0, 1);
        }
    }



    void filter_panel_t::notify_on_set_focus(HWND wnd_lost)
    {
        m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        get_selection_handles(handles, false);
        m_selection_holder->set_selection(handles);
    }
    void filter_panel_t::notify_on_kill_focus(HWND wnd_receiving)
    {
        m_selection_holder.release();
    }


    void node_t::ensure_handles_sorted()
    {
        if (!m_handles_sorted)
        {
            if (cfg_sort)
            {
                service_ptr_t<titleformat_object> to;
                static_api_ptr_t<titleformat_compiler>()->compile_safe(to, cfg_sort_string);
                fbh::sort_metadb_handle_list_by_format(m_handles, to, nullptr);
            }
            m_handles_sorted = true;
        }
    }

    void node_t::remove_handles(metadb_handle_list_cref to_remove)
    {
        pfc::array_staticsize_t<bool> remove_mask(m_handles.get_count());
        pfc::fill_array_t(remove_mask, false);

        m_handles_sorted = false;
        mmh::in_place_sort(m_handles, pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, false);

        const size_t remove_handles_count = to_remove.get_count();
        for (size_t j = 0; j < remove_handles_count; j++) {
            size_t handle_index = -1;
            if (m_handles.bsearch_t(pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, to_remove[j], handle_index)) {
                remove_mask[handle_index] = true;
            }
        }
        m_handles.remove_mask(remove_mask.get_ptr());
    }

    int node_t::g_compare(const node_t & i1, const WCHAR * i2)
    {
        //return CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, i1.m_value, -1, i2, -1);
        return StrCmpLogicalW(i1.m_value, i2);
    }
    int node_t::g_compare_ptr(const node_t * i1, const WCHAR * i2)
    {
        //return CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, i1.m_value, -1, i2, -1);
        return StrCmpLogicalW(i1->m_value, i2);
    }

    int node_t::g_compare_ptr_with_node(const node_t & i1, const node_t & i2)
    {
        return StrCmpLogicalW(i1.m_value, i2.m_value);
    }


    void filter_panel_t::refresh_stream()
    {
        m_stream.release();
        if (cfg_orderedbysplitters)
        {
            t_size stream_index = pfc_infinite;
            uie::window_host_ex::ptr hostex;
            if (get_host()->service_query_t(hostex))
            {
                pfc::list_t<uie::window_ptr> siblings;
                hostex->get_children(siblings);
                t_size i, count=siblings.get_count();
                for (i=0; i<count; i++)
                {
                    if ((stream_index = g_get_stream_index_by_window(siblings[i])) != pfc_infinite)
                        break;
                }
            }
            if (stream_index != pfc_infinite)
                g_streams[stream_index]->m_windows.add_item(this);
            else
            {
                filter_stream_t::ptr streamnew = new filter_stream_t;
                streamnew->m_windows.add_item(this);
                stream_index = g_streams.add_item(streamnew);
            }
            m_stream = g_streams[stream_index];
        }
        else
        {
            if (!g_streams.get_count())
            {
                m_stream = new filter_stream_t;
                g_streams.add_item(m_stream);
            }
            else
                m_stream = g_streams[0];
            m_stream->m_windows.add_item(this);
        }
    }

    void filter_panel_t::refresh_groups()
    {
        set_group_count(0);
    }

    void filter_panel_t::refresh_columns()
    {
        set_columns(pfc::list_single_ref_t<Column>(Column(m_field_data.is_empty() ? "<no field>" : m_field_data.m_name, 200)));
        set_sort_column(0, m_pending_sort_direction);
    }
    /*void filter_panel_t::on_groups_change()
    {
    if (get_wnd())
    {
    clear_all_items();
    refresh_groups();
    //populate_list();
    }
    }

    void filter_panel_t::on_columns_change()
    {
    if (get_wnd())
    {
    clear_all_items();
    refresh_columns();
    //populate_list();
    }
    }*/
    void filter_panel_t::notify_on_initialisation()
    {
        //set_variable_height_items(true); //Implementation not finished
        set_edge_style(cfg_edgestyle);
        set_autosize(true);
        set_vertical_item_padding(cfg_vertical_item_padding);
        set_show_header(cfg_show_column_titles);
        set_sorting_enabled(cfg_allow_sorting);
        set_show_sort_indicators(cfg_show_sort_indicators);

        LOGFONT lf;
        static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_filter_items_font_client, lf);
        set_font(&lf);
        static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_filter_header_font_client, lf);
        set_header_font(&lf);

        t_size index = g_windows.size();
        if (index == 0)
        {
            g_showemptyitems = cfg_showemptyitems;
            g_load_fields();
        }

        refresh_stream();

        t_size field_index = get_field_index();
        if (field_index == pfc_infinite)
            //if (m_field_data.is_empty())
        {
            m_pending_sort_direction = false;

            pfc::array_t<bool> used;
            t_size field_count = g_field_data.get_count();
            used.set_count(field_count);
            used.fill_null();
            {
                t_size i, count = m_stream->m_windows.get_count();
                for (i=0; i<count; i++)
                {
                    t_size field_index = m_stream->m_windows[i]->get_field_index();
                    if (field_index != pfc_infinite)
                        used[field_index] = true;
                }
            }
            {
                t_size i;
                for (i=0; i<field_count; i++)
                {
                    if (!used[i])
                    {
                        m_field_data = g_field_data[i];
                        field_index = i;
                        break;
                    }
                }
            }
        }

        if (field_index == pfc_infinite)
            m_field_data.reset();
        else
            m_field_data = g_field_data[field_index];
    }


    void filter_panel_t::notify_on_create()
    {
        refresh_columns();
        refresh_groups();

        pfc::hires_timer timer0;
        timer0.start();
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        get_initial_handles(handles);
        populate_list(handles);
        double time = timer0.query();
        console::formatter formatter;
        formatter << "Filter Panel - " << m_field_data.m_name << ": initialised in " << pfc::format_float(time, 0, 3) <<" s";

        g_windows.push_back(this);
        fbh::library_callback_manager::register_callback(this);

    }


    void filter_panel_t::notify_on_destroy()
    {
        fbh::library_callback_manager::deregister_callback(this);

        m_selection_holder.release();

        m_stream->m_windows.remove_item(this);
        if (m_stream->m_windows.get_count() == 0)
            g_streams.remove_item(m_stream);
        m_stream.release();

        g_windows.erase(std::remove(g_windows.begin(), g_windows.end(), this), g_windows.end());
        if (g_windows.size() == 0)
            g_field_data.remove_all();
        m_nodes.remove_all();
    }


    const GUID & filter_panel_t::get_extension_guid() const
    {
        return g_extension_guid;
    }

    void filter_panel_t::get_name(pfc::string_base & out)const
    {
        out.set_string("Filter");
    }
    void filter_panel_t::get_category(pfc::string_base & out)const
    {
        out.set_string("Panels");
    }
    unsigned filter_panel_t::get_type() const{return uie::type_panel;}

    // {FB059406-5F14-4bd0-8A11-4242854CBBA5}
    const GUID filter_panel_t::g_extension_guid = 
    { 0xfb059406, 0xdddd, 0x4bd0, { 0x8a, 0x11, 0x42, 0x42, 0x85, 0x4c, 0xbb, 0xa5 } };

    uie::window_factory<filter_panel_t> g_filter;

    std::vector<filter_panel_t *> filter_panel_t::g_windows;
    bool filter_panel_t::g_showemptyitems = false;

    pfc::list_t<field_data_t> filter_panel_t::g_field_data;

    pfc::list_t<filter_stream_t::ptr> filter_panel_t::g_streams;

    // {4D6774AF-C292-44ac-8A8F-3B0855DCBDF4}
    const GUID appearance_client_filter_impl::g_guid =
    { 0x4d6774af, 0xc292, 0x44ac, { 0x8a, 0x8f, 0x3b, 0x8, 0x55, 0xdc, 0xbd, 0xf4 } };

    namespace {
        cui::colours::client::factory<appearance_client_filter_impl> g_appearance_client_impl;
    }


    class font_client_filter : public cui::fonts::client
    {
    public:
        const GUID & get_client_guid() const override
        {
            return g_guid_filter_items_font_client;
        }
        void get_name (pfc::string_base & p_out) const override
        {
            p_out = "Filter Panel: Items";
        }

        cui::fonts::font_type_t get_default_font_type() const override
        {
            return cui::fonts::font_type_items;
        }

        void on_font_changed() const override 
        {
            filter_panel_t::g_on_font_items_change();

        }
    };

    class font_header_client_filter : public cui::fonts::client
    {
    public:
        const GUID & get_client_guid() const override
        {
            return g_guid_filter_header_font_client;
        }
        void get_name (pfc::string_base & p_out) const override
        {
            p_out = "Filter Panel: Column Titles";
        }

        cui::fonts::font_type_t get_default_font_type() const override
        {
            return cui::fonts::font_type_items;
        }

        void on_font_changed() const override 
        {
            filter_panel_t::g_on_font_header_change();

        }
    };

    font_client_filter::factory<font_client_filter> g_font_client_filter;
    font_header_client_filter::factory<font_header_client_filter> g_font_header_client_filter;


    void appearance_client_filter_impl::on_colour_changed(t_size mask) const
    {
        filter_panel_t::g_redraw_all();
    }

}

