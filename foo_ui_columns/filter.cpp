#include "pch.h"
#include "filter.h"

#include "filter_config_var.h"
#include "filter_search_bar.h"
#include "filter_utils.h"

namespace cui::panels::filter {

bool FilterStream::is_visible()
{
    for (size_t i = 0, count = m_windows.get_count(); i < count; i++)
        if (!m_windows[i]->is_visible())
            return false;
    return true;
}

void FilterPanel::set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    if (p_size) {
        const auto version = p_reader->read_lendian_t<uint32_t>(p_abort);
        if (version <= config_version_current) {
            p_reader->read_string(m_field_data.m_name, p_abort);
            if (version >= 1) {
                p_reader->read_lendian_t(m_show_search, p_abort);
                try {
                    p_reader->read_lendian_t(m_pending_sort_direction, p_abort);
                } catch (const exception_io_data_truncation&) {
                }
            }
        }
    }
}

void FilterPanel::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(static_cast<uint32_t>(config_version_current), p_abort);
    p_writer->write_string(m_field_data.m_name, p_abort);
    p_writer->write_lendian_t(m_show_search, p_abort);
    p_writer->write_lendian_t(get_sort_direction(), p_abort);
}

size_t FilterPanel::g_get_stream_index_by_window(const uie::window_ptr& ptr)
{
    size_t count = g_streams.get_count();
    for (size_t i = 0; i < count; i++) {
        size_t subcount = g_streams[i]->m_windows.get_count();
        for (size_t j = 0; j < subcount; j++)
            if (g_streams[i]->m_windows[j] == ptr.get_ptr())
                return i;
    }
    return pfc_infinite;
}

void FilterPanel::g_on_orderedbysplitters_change()
{
    g_streams.remove_all();
    for (auto& window : g_windows) {
        window->refresh_stream();
    }
    // filter_search_bar::g_on_orderedbysplitters_change();
    size_t count = g_streams.get_count();
    for (size_t i = 0; i < count; i++) {
        if (g_streams[i]->m_windows.get_count()) {
            FilterSearchToolbar::g_initialise_filter_stream(g_streams[i]);
            pfc::list_t<FilterPanel*> windows;
            g_streams[i]->m_windows[0]->get_windows(windows);
            if (windows.get_count())
                g_update_subsequent_filters(windows, 0, false, false);
        }
    }
}

void FilterPanel::g_on_fields_change()
{
    g_load_fields();
    for (auto& window : g_windows) {
        size_t field_index = window->get_field_index();
        window->set_field(field_index == pfc_infinite ? FieldData() : g_field_data[field_index], true);
    }
}

size_t FilterPanel::g_get_field_index_by_name(const char* p_name)
{
    size_t count = g_field_data.get_count();
    for (size_t i = 0; i < count; i++) {
        if (!strcmp(g_field_data[i].m_name, p_name))
            return i;
    }
    return pfc_infinite;
}

void FilterPanel::g_on_field_title_change(const char* p_old, const char* p_new)
{
    size_t field_index = g_get_field_index_by_name(p_old);
    if (field_index != pfc_infinite) {
        for (auto& window : g_windows) {
            if (window->get_field_index() == field_index) {
                window->m_field_data.m_name = p_new;
                window->refresh_columns();
                window->update_first_node_text(true);
            }
        }
        g_field_data[field_index].m_name = p_new;
    }
}

void FilterPanel::g_on_vertical_item_padding_change()
{
    for (auto& window : g_windows)
        window->set_vertical_item_padding(cfg_vertical_item_padding);
}

void FilterPanel::g_on_show_column_titles_change()
{
    for (auto& window : g_windows)
        window->set_show_header(cfg_show_column_titles);
}

void FilterPanel::g_on_allow_sorting_change()
{
    for (auto& window : g_windows)
        window->set_sorting_enabled(cfg_allow_sorting);
}

void FilterPanel::g_on_show_sort_indicators_change()
{
    for (auto& window : g_windows)
        window->set_show_sort_indicators(cfg_show_sort_indicators);
}

void FilterPanel::g_on_field_query_change(const Field& field)
{
    size_t field_index = g_get_field_index_by_name(field.m_name);
    if (field_index != pfc_infinite) {
        g_create_field_data(field, g_field_data[field_index]);
        size_t count = g_streams.get_count();
        for (size_t i = 0; i < count; i++) {
            if (g_streams[i]->m_windows.get_count()) {
                pfc::list_t<FilterPanel*> windows;
                g_streams[i]->m_windows[0]->get_windows(windows);
                size_t subcount = windows.get_count();

                for (size_t j = 0; j < subcount; j++) {
                    if (windows[j]->get_field_index() == field_index) // meh
                    {
                        windows[j]->set_field(g_field_data[field_index], true);
                        break;
                    }
                }
            }
        }
    }
}
void FilterPanel::g_on_showemptyitems_change(bool b_val, bool update_filters)
{
    if (g_windows.empty())
        return;

    g_showemptyitems = b_val;

    if (!update_filters)
        return;

    size_t count = g_streams.get_count();
    for (size_t i = 0; i < count; i++) {
        if (g_streams[i]->m_windows.get_count()) {
            pfc::list_t<FilterPanel*> windows;
            g_streams[i]->m_windows[0]->get_windows(windows);

            if (windows.get_count())
                g_update_subsequent_filters(windows, 0, false, false);
        }
    }
}
void FilterPanel::g_on_edgestyle_change()
{
    for (auto& window : g_windows) {
        window->set_edge_style(cfg_edgestyle);
    }
}

void FilterPanel::s_on_dark_mode_status_change()
{
    const auto is_dark = colours::is_dark_mode_active();
    for (auto&& window : g_windows)
        window->set_use_dark_mode(is_dark);
}

void FilterPanel::g_on_font_items_change()
{
    for (auto& window : g_windows)
        window->recreate_items_text_format();
}

void FilterPanel::g_on_font_header_change()
{
    for (auto& window : g_windows)
        window->recreate_header_text_format();
}

void FilterPanel::g_redraw_all()
{
    for (auto& window : g_windows)
        RedrawWindow(window->get_wnd(), nullptr, nullptr, RDW_UPDATENOW | RDW_INVALIDATE);
}
void FilterPanel::g_on_new_field(const Field& field)
{
    if (!g_windows.empty()) {
        size_t index = g_field_data.get_count();
        g_field_data.set_count(index + 1);
        g_create_field_data(field, g_field_data[index]);
    }
}
void FilterPanel::g_on_fields_swapped(size_t index_1, size_t index_2)
{
    if (std::max(index_1, index_2) < g_field_data.get_count())
        g_field_data.swap_items(index_1, index_2);
    if (!cfg_orderedbysplitters) {
        size_t count = g_streams.get_count();
        for (size_t i = 0; i < count; i++) {
            if (g_streams[i]->m_windows.get_count()) {
                pfc::list_t<FilterPanel*> windows;
                g_streams[i]->m_windows[0]->get_windows(windows);
                size_t subcount = windows.get_count();
                for (size_t j = 0; j < subcount; j++) {
                    size_t this_index = windows[j]->get_field_index();
                    if (this_index == index_1 || this_index == index_2) {
                        g_update_subsequent_filters(windows, j, false, false);
                        break;
                    }
                    if (this_index > std::max(index_1, index_2))
                        break;
                }
            }
        }
    }
}
void FilterPanel::g_on_field_removed(size_t index)
{
    size_t count = g_streams.get_count();
    for (size_t i = 0; i < count; i++) {
        if (g_streams[i]->m_windows.get_count()) {
            pfc::list_t<FilterPanel*> windows;
            g_streams[i]->m_windows[0]->get_windows(windows);
            size_t subcount = windows.get_count();
            bool b_found = false;
            size_t index_found = pfc_infinite;
            for (size_t j = 0; j < subcount; j++) {
                size_t this_index = windows[j]->get_field_index();
                if (index == this_index) {
                    windows[j]->set_field(FieldData());
                    if (!b_found) {
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

void FilterPanel::refresh(bool b_allow_autosend)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
    get_initial_handles(data);
    populate_list_from_chain(data, false);
    update_subsequent_filters(b_allow_autosend);
}

size_t FilterPanel::get_field_index()
{
    size_t count = g_field_data.get_count();
    size_t ret = pfc_infinite;
    for (size_t i = 0; i < count; i++)
        if (!stricmp_utf8(g_field_data[i].m_name, m_field_data.m_name)) {
            ret = i;
            break;
        }
    return ret;
}

void FilterPanel::get_windows(pfc::list_base_t<FilterPanel*>& windows)
{
    if (cfg_orderedbysplitters) {
        pfc::list_t<uie::window_ptr> siblings;
        uie::window_host_ex::ptr hostex;
        if (get_host()->service_query_t(hostex))
            hostex->get_children(siblings);
        else
            siblings.add_item(this);
        pfc::list_t<size_t> indices;
        size_t count = siblings.get_count();
        for (size_t i = 0; i < count; i++) {
            size_t index = pfc_infinite;
            if ((index = m_stream->m_windows.find_item(static_cast<FilterPanel*>(siblings[i].get_ptr())))
                != pfc_infinite) // meh
                windows.add_item(m_stream->m_windows[index]);
        }
    } else {
        pfc::list_t<size_t> indices;
        size_t count = m_stream->m_windows.get_count();
        for (size_t i = 0; i < count; i++) {
            size_t index = m_stream->m_windows[i]->get_field_index();
            if (index != pfc_infinite) {
                indices.add_item(index);
                windows.add_item(m_stream->m_windows[i]);
            }
        }
        mmh::Permutation permutation(windows.get_count());
        sort_get_permutation(indices.get_ptr(), permutation, (pfc::compare_t<size_t, size_t>), true, false);
        windows.reorder(permutation.data());
    }
}

FilterPanel* FilterPanel::get_next_window()
{
    pfc::list_t<FilterPanel*> windows;
    get_windows(windows);
    const auto window_index = windows.find_item(this);
    const auto window_count = windows.get_count();
    return window_index + 1 < window_count ? windows[window_index + 1] : nullptr;
}

void FilterPanel::g_create_field_data(const Field& field, FieldData& p_out)
{
    p_out.m_name = field.m_name;
    p_out.m_last_sort_direction = field.m_last_sort_direction;
    if (strchr(field.m_field, '$') || strchr(field.m_field, '%')) {
        p_out.m_use_script = true;
        p_out.m_script = field.m_field;
        p_out.m_fields.clear();
    } else {
        p_out.m_use_script = false;
        p_out.m_script.reset();
        p_out.m_fields.clear();
        const char* ptr = field.m_field;
        while (*ptr) {
            const char* start = ptr;
            while (*ptr && *ptr != ';')
                ptr++;
            if (ptr > start)
                p_out.m_fields.emplace_back(pfc::string8(start, ptr - start));
            while (*ptr == ';')
                ptr++;
        }
    }
}

void FilterPanel::g_load_fields()
{
    size_t count = cfg_field_list.get_count();
    g_field_data.set_count(count);
    for (size_t i = 0; i < count; i++) {
        const Field& field = cfg_field_list[i];
        g_create_field_data(field, g_field_data[i]);
    }
}
void FilterPanel::g_update_subsequent_filters(const pfc::list_base_const_t<FilterPanel*>& windows, size_t index,
    bool b_check_needs_update, bool b_update_playlist)
{
    size_t count = windows.get_count();
    metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;

    for (size_t i = index; i < count; i++) {
        handles.remove_all();
        windows[i]->get_initial_handles(handles);
        if (b_check_needs_update) {
            metadb_handle_list items1(windows[i]->m_nodes[0]->m_handles);
            handles.sort_by_pointer();
            items1.sort_by_pointer();
            if (!pfc::comparator_array<>::compare(items1, handles)) {
                b_update_playlist = false;
                break;
            }
        }
        windows[i]->populate_list_from_chain(handles, false);
    }
    if (count && b_update_playlist && cfg_autosend)
        windows[count - 1]->send_results_to_playlist();
}

void FilterPanel::update_subsequent_filters(bool b_allow_autosend)
{
    pfc::ptr_list_t<FilterPanel> windows;
    get_windows(windows);
    size_t pos = windows.find_item(this);

    if (pos != pfc_infinite) {
        g_update_subsequent_filters(windows, pos + 1, false, b_allow_autosend);
    }
}

void FilterPanel::get_initial_handles(metadb_handle_list_t<pfc::alloc_fast_aggressive>& p_out)
{
    pfc::ptr_list_t<FilterPanel> windows;
    get_windows(windows);
    size_t pos = windows.find_item(this);
    if (pos && pos != pfc_infinite) {
        windows[pos - 1]->get_selection_handles(p_out);
    } else {
        if (m_stream->m_source_overriden)
            p_out = m_stream->m_source_handles;
        else
            library_manager::get()->get_all_items(p_out);
    }
}

void FilterPanel::set_field(const FieldData& field, bool b_force)
{
    if (b_force || stricmp_utf8(field.m_name, m_field_data.m_name)) {
        m_pending_sort_direction = field.m_last_sort_direction;

        pfc::ptr_list_t<FilterPanel> windows_before;
        get_windows(windows_before);
        size_t pos_before = windows_before.find_item(this);

        m_field_data = field;
        bool b_redraw = disable_redrawing();
        clear_all_items();
        refresh_columns();

        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        get_initial_handles(handles);
        populate_list(handles);

        pfc::ptr_list_t<FilterPanel> windows_after;
        get_windows(windows_after);
        size_t pos_after = windows_after.find_item(this);
        size_t pos_update = std::min(pos_before, pos_after);
        if (b_redraw)
            enable_redrawing();
        // update_window();
        g_update_subsequent_filters(windows_after, pos_update, false, false);
    }
}

void FilterPanel::notify_update_item_data(size_t index)
{
    auto& subitems = get_item_subitems(index);
    subitems.resize(1);

    subitems[0] = pfc::stringcvt::string_utf8_from_wide(m_nodes[index]->m_value.c_str());
}

size_t FilterPanel::get_highlight_item()
{
    return pfc_infinite;
}

bool FilterPanel::notify_on_keyboard_keydown_search()
{
    return FilterSearchToolbar::s_activate();
}

bool FilterPanel::do_drag_drop(WPARAM wp)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
    get_selection_handles(data);
    if (data.get_count() > 0) {
        sort_tracks(data);
        const auto incoming_api = playlist_incoming_item_filter::get();
        auto pDataObject = incoming_api->create_dataobject_ex(data);
        if (pDataObject.is_valid()) {
            m_drag_item_count = data.get_count();
            DWORD blah = DROPEFFECT_NONE;
            uih::ole::do_drag_drop(
                get_wnd(), wp, pDataObject.get_ptr(), DROPEFFECT_COPY | DROPEFFECT_MOVE, DROPEFFECT_COPY, &blah);
            m_drag_item_count = 0;
        }
    }
    return true;
}

bool FilterPanel::notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp)
{
    uie::window_ptr p_this = this;
    bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
    return ret;
}

void FilterPanel::get_selection_handles(
    metadb_handle_list_t<pfc::alloc_fast_aggressive>& p_out, bool fallback, bool b_sort)
{
    bool b_found = false;
    size_t count = m_nodes.get_count();
    if (count)
        p_out.prealloc(count);
    for (size_t i = 0; i < count; i++) {
        if (get_item_selected(i)) {
            b_found = true;
            if (b_sort)
                m_nodes[i]->ensure_handles_sorted();
            p_out.add_items(m_nodes[i]->m_handles);
        }
    }
    if (!b_found) {
        if (fallback) {
            if (b_sort)
                m_nodes[0]->ensure_handles_sorted();
            p_out.add_items(m_nodes[0]->m_handles);
        }
    } else {
        fbh::metadb_handle_list_remove_duplicates(p_out);
    }
}

void FilterPanel::do_selection_action(Action action)
{
    bit_array_bittable mask(m_nodes.get_count());
    get_selection_state(mask);
    do_items_action(mask, action);
}

void FilterPanel::do_items_action(const bit_array& p_nodes, Action action)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
    handles.prealloc(m_nodes.get_count());
    size_t i;
    size_t count = m_nodes.get_count();
    for (i = 0; i < count; i++)
        if (p_nodes[i])
            handles.add_items(m_nodes[i]->m_handles);

    if (!handles.get_count())
        return;

    fbh::metadb_handle_list_remove_duplicates(handles);

    const auto playlist_api = playlist_manager_v3::get();
    const auto playback_api = play_control::get();
    size_t index_insert = pfc_infinite;
    if (action == action_send_to_autosend && playback_api->is_playing()) {
        size_t playlist = playlist_api->get_playing_playlist();
        pfc::string8 name;
        if (playlist_api->playlist_get_name(playlist, name) && !stricmp_utf8("Filter Results", name)) {
            size_t index_old = playlist_api->find_playlist("Filter Results (Playback)", pfc_infinite);
            playlist_api->playlist_rename(playlist, "Filter Results (Playback)", pfc_infinite);
            index_insert = index_old < playlist ? playlist : playlist + 1;
            if (index_old != pfc_infinite)
                playlist_api->remove_playlist(index_old);
        }
    }
    pfc::string8 playlist_name;
    size_t index = NULL;
    if (action == action_add_to_active) {
        index = playlist_api->get_active_playlist();
        playlist_api->playlist_undo_backup(index);
    } else {
        if (action == action_send_to_autosend)
            playlist_name = "Filter Results";
        else if (action == action_send_to_autosend_play)
            playlist_name = "Filter Results (Playback)";
        else if (action == action_send_to_new || action == action_send_to_new_play) {
            for (i = 0; i < count; i++) {
                if (p_nodes[i]) {
                    if (playlist_name.get_length())
                        playlist_name << ", ";
                    playlist_name << pfc::stringcvt::string_utf8_from_wide(m_nodes[i]->m_value.c_str());
                }
            }
        }

        if (index_insert != pfc_infinite)
            index = playlist_api->create_playlist(playlist_name, pfc_infinite, index_insert);
        else
            index = playlist_api->find_or_create_playlist(playlist_name, pfc_infinite);
        playlist_api->playlist_undo_backup(index);
        playlist_api->playlist_clear(index);
    }

    sort_tracks(handles);

    if (action != action_add_to_active)
        playlist_api->playlist_add_items(index, handles, bit_array_false());
    else {
        playlist_api->playlist_clear_selection(index);
        const auto item_index = playlist_api->playlist_get_item_count(index);
        playlist_api->playlist_add_items(index, handles, bit_array_true());
        playlist_api->playlist_set_focus_item(index, item_index);
    }

    if (action != action_add_to_active) {
        playlist_api->set_active_playlist(index);
        if (action == action_send_to_autosend_play || action == action_send_to_new_play) {
            playlist_api->set_playing_playlist(index);
            playback_api->start(play_control::track_command_default);
        }
    }
}

void FilterPanel::execute_default_action(size_t index, size_t column, bool b_keyboard, bool b_ctrl)
{
    auto action = static_cast<Action>(cfg_doubleclickaction.get_value());
    do_selection_action(action);
}

bool FilterPanel::notify_on_middleclick(bool on_item, size_t index)
{
    if (cfg_middleclickaction && on_item && index < m_nodes.get_count()) {
        auto action = static_cast<Action>(cfg_middleclickaction.get_value() - 1);
        do_items_action(bit_array_one(index), action);
        return true;
    }
    return false;
}

void FilterPanel::send_results_to_playlist(bool b_play)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
    handles.prealloc(m_nodes.get_count());
    get_selection_handles(handles);
    const auto playlist_api = playlist_manager::get();
    const auto playback_api = play_control::get();
    size_t index_insert = pfc_infinite;
    if (!b_play && playback_api->is_playing()) {
        size_t playlist = playlist_api->get_playing_playlist();
        pfc::string8 name;
        if (playlist_api->playlist_get_name(playlist, name) && !stricmp_utf8("Filter Results", name)) {
            size_t index_old = playlist_api->find_playlist("Filter Results (Playback)", pfc_infinite);
            playlist_api->playlist_rename(playlist, "Filter Results (Playback)", pfc_infinite);
            index_insert = index_old < playlist ? playlist : playlist + 1;
            if (index_old != pfc_infinite)
                playlist_api->remove_playlist(index_old);
        }
    }
    // size_t index_remove = playlist_api->find_playlist("Filter Results", pfc_infinite);
    size_t index = NULL;
    if (index_insert != pfc_infinite)
        index = playlist_api->create_playlist(
            b_play ? "Filter Results (Playback)" : "Filter Results", pfc_infinite, index_insert);
    else
        index = playlist_api->find_or_create_playlist(
            b_play ? "Filter Results (Playback)" : "Filter Results", pfc_infinite);
    playlist_api->playlist_clear(index);

    sort_tracks(handles);
    playlist_api->playlist_add_items(index, handles, bit_array_false());

    playlist_api->set_active_playlist(index);
    if (b_play) {
        playlist_api->set_playing_playlist(index);
        playback_api->play_start(play_control::track_command_default);
    }
    // if (index_remove != pfc_infinite)
    //    playlist_api->remove_playlist(index+1);
}
void FilterPanel::notify_on_selection_change(
    const bit_array& p_affected, const bit_array& p_status, notification_source_t p_notification_source)
{
    update_subsequent_filters();
    if (m_selection_holder.is_valid()) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        get_selection_handles(handles, false, true);
        m_selection_holder->set_selection(handles);
    }
}
void FilterPanel::update_first_node_text(bool b_update)
{
    size_t nodes_count = m_nodes.get_count();
    if (nodes_count) {
        nodes_count -= 1;
        pfc::string8 temp;
        temp << "All";
        if (m_field_data.m_name.length()) {
            temp << " (" << nodes_count << " " << m_field_data.m_name;
            if (nodes_count != 1)
                temp << "s";
            temp << ")";
        }
        m_nodes[0]->m_value = pfc::stringcvt::string_wide_from_utf8(temp).get_ptr();
        if (b_update)
            update_items(0, 1);
    }
}

void FilterPanel::notify_on_set_focus(HWND wnd_lost)
{
    m_selection_holder = ui_selection_manager::get()->acquire();
    metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
    get_selection_handles(handles, false, true);
    m_selection_holder->set_selection(handles);
}
void FilterPanel::notify_on_kill_focus(HWND wnd_receiving)
{
    m_selection_holder.release();
}

void Node::ensure_handles_sorted()
{
    if (!m_handles_sorted) {
        sort_tracks(m_handles);
        m_handles_sorted = true;
    }
}

void Node::remove_handles(metadb_handle_list_cref to_remove)
{
    pfc::array_staticsize_t<bool> remove_mask(m_handles.get_count());
    fill_array_t(remove_mask, false);

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

int Node::g_compare(const Node::Ptr& i1, const WCHAR* i2)
{
    return StrCmpLogicalW(i1->m_value.c_str(), i2);
}

int Node::g_compare_ptr_with_node(const Node::Ptr& i1, const Node::Ptr& i2)
{
    return StrCmpLogicalW(i1->m_value.c_str(), i2->m_value.c_str());
}

void FilterPanel::refresh_stream()
{
    m_stream.reset();
    if (cfg_orderedbysplitters) {
        size_t stream_index = pfc_infinite;
        uie::window_host_ex::ptr hostex;
        if (get_host()->service_query_t(hostex)) {
            pfc::list_t<uie::window_ptr> siblings;
            hostex->get_children(siblings);
            size_t count = siblings.get_count();
            for (size_t i = 0; i < count; i++) {
                if ((stream_index = g_get_stream_index_by_window(siblings[i])) != pfc_infinite)
                    break;
            }
        }
        if (stream_index != pfc_infinite)
            g_streams[stream_index]->m_windows.add_item(this);
        else {
            FilterStream::ptr streamnew = std::make_shared<FilterStream>();
            streamnew->m_windows.add_item(this);
            stream_index = g_streams.add_item(streamnew);
        }
        m_stream = g_streams[stream_index];
    } else {
        if (!g_streams.get_count()) {
            m_stream = std::make_shared<FilterStream>();
            g_streams.add_item(m_stream);
        } else
            m_stream = g_streams[0];
        m_stream->m_windows.add_item(this);
    }
}

void FilterPanel::refresh_groups()
{
    set_group_count(0);
}

void FilterPanel::refresh_columns()
{
    set_columns({{m_field_data.is_empty() ? "<no field>" : m_field_data.m_name.get_ptr(), 200}});
    set_sort_column(0, m_pending_sort_direction);
}

void FilterPanel::notify_on_initialisation()
{
    // set_variable_height_items(true); //Implementation not finished
    set_use_dark_mode(colours::is_dark_mode_active());
    set_edge_style(cfg_edgestyle);
    set_autosize(true);
    set_vertical_item_padding(cfg_vertical_item_padding);
    set_show_header(cfg_show_column_titles);
    set_sorting_enabled(cfg_allow_sorting);
    set_show_sort_indicators(cfg_show_sort_indicators);

    recreate_items_text_format();
    recreate_header_text_format();

    size_t index = g_windows.size();
    if (index == 0) {
        g_showemptyitems = cfg_showemptyitems;
        g_load_fields();
    }

    refresh_stream();

    size_t field_index = get_field_index();
    if (field_index == pfc_infinite)
    // if (m_field_data.is_empty())
    {
        m_pending_sort_direction = false;

        pfc::array_t<bool> used;
        size_t field_count = g_field_data.get_count();
        used.set_count(field_count);
        used.fill_null();
        {
            size_t count = m_stream->m_windows.get_count();
            for (size_t i = 0; i < count; i++) {
                size_t field_index = m_stream->m_windows[i]->get_field_index();
                if (field_index != pfc_infinite)
                    used[field_index] = true;
            }
        }
        {
            for (size_t i = 0; i < field_count; i++) {
                if (!used[i]) {
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

void FilterPanel::notify_on_create()
{
    refresh_columns();
    refresh_groups();

    auto library_v4 = library_manager_v4::tryGet();
    auto is_library_initialised = !library_v4.is_valid() || library_v4->is_initialized();

    pfc::hires_timer timer0;
    timer0.start();
    metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
    get_initial_handles(handles);
    populate_list(handles);
    double time = timer0.query();

    if (is_library_initialised && handles.size() > 0)
        console::print(fmt::format("Filter panel - {}: initialised in {} s", m_field_data.m_name.c_str(),
            pfc::format_float(time, 0, 3).c_str())
                .c_str());

    g_windows.push_back(this);
    fbh::library_callback_manager::register_callback(this);
}

void FilterPanel::notify_on_destroy()
{
    fbh::library_callback_manager::deregister_callback(this);

    m_selection_holder.release();

    m_stream->m_windows.remove_item(this);
    if (m_stream->m_windows.get_count() == 0)
        g_streams.remove_item(m_stream);
    m_stream.reset();

    std::erase(g_windows, this);
    if (g_windows.empty())
        g_field_data.remove_all();
    m_nodes.remove_all();
}

const GUID& FilterPanel::get_extension_guid() const
{
    return g_extension_guid;
}

void FilterPanel::get_name(pfc::string_base& out) const
{
    out.set_string("Filter");
}
void FilterPanel::get_category(pfc::string_base& out) const
{
    out.set_string("Panels");
}
unsigned FilterPanel::get_type() const
{
    return uie::type_panel;
}

// {FB059406-5F14-4bd0-8A11-4242854CBBA5}
const GUID FilterPanel::g_extension_guid
    = {0xfb059406, 0xdddd, 0x4bd0, {0x8a, 0x11, 0x42, 0x42, 0x85, 0x4c, 0xbb, 0xa5}};

uie::window_factory<FilterPanel> g_filter;

std::vector<FilterPanel*> FilterPanel::g_windows;
bool FilterPanel::g_showemptyitems = false;

pfc::list_t<FieldData> FilterPanel::g_field_data;

pfc::list_t<FilterStream::ptr> FilterPanel::g_streams;

namespace {
colours::client::factory<AppearanceClient> g_appearance_client_impl;
}

class FilterItemFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return items_font_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Filter panel: Items"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { FilterPanel::g_on_font_items_change(); }
};

class FilterHeaderFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return header_font_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Filter panel: Column titles"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { FilterPanel::g_on_font_header_change(); }
};

FilterItemFontClient::factory<FilterItemFontClient> g_font_client_filter;
FilterHeaderFontClient::factory<FilterHeaderFontClient> g_font_header_client_filter;

void AppearanceClient::on_colour_changed(uint32_t mask) const
{
    FilterPanel::g_redraw_all();
}

void AppearanceClient::on_bool_changed(uint32_t mask) const
{
    if (mask & colours::bool_flag_dark_mode_enabled)
        FilterPanel::s_on_dark_mode_status_change();
}

} // namespace cui::panels::filter
