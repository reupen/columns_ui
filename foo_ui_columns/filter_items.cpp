#include "stdafx.h"
#include "filter.h"
#include "filter_config_var.h"

namespace filter_panel {
void filter_panel_t::populate_list_from_chain(
    const metadb_handle_list_t<pfc::alloc_fast>& handles, bool b_last_in_chain)
{
    bool b_redraw = disable_redrawing();
    pfc::list_t<pfc::string_simple_t<WCHAR>> previous_nodes;
    bool b_all_was_selected = false;
    if (m_nodes.get_count()) {
        pfc::list_t<bool> sel_data;
        sel_data.set_count(m_nodes.get_count());
        pfc::bit_array_var_table selection(sel_data.get_ptr(), sel_data.get_count());
        get_selection_state(selection);
        t_size count = sel_data.get_count();
        b_all_was_selected = selection[0];
        for (t_size i = 1; i < count; i++)
            if (selection[i])
                previous_nodes.add_item(m_nodes[i].m_value);
    }

    populate_list(handles);

    t_size count = previous_nodes.get_count();
    pfc::array_t<bool> new_selection;
    new_selection.set_count(m_nodes.get_count());
    new_selection.fill_null();
    if (count || b_all_was_selected) {
        bool b_found = false;
        new_selection[0] = b_all_was_selected;
        for (t_size i = 0; i < count; i++) {
            t_size index;
            if (mmh::partial_bsearch(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, previous_nodes[i].get_ptr(),
                    1, index, get_sort_direction())) {
                new_selection[index] = true;
                b_found = true;
            }
        }
        if (!b_found)
            new_selection[0] = true; // m_nodes.get_count() >= 1
        set_selection_state(pfc::bit_array_var_table(new_selection.get_ptr(), new_selection.get_count()),
            pfc::bit_array_var_table(new_selection.get_ptr(), new_selection.get_count()), false);
    }

    if (b_redraw)
        enable_redrawing();
}

void filter_panel_t::add_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& added_tracks)
{
    filter_panel_t* const next_window = get_next_window();
    if (m_field_data.is_empty()) {
        if (next_window)
            next_window->add_nodes(added_tracks);
        return;
    }

    const auto nothing_or_all_node_selected = get_nothing_or_all_node_selected();

    metadb_handle_list_t<pfc::alloc_fast_aggressive> tracks_for_next_window;
    tracks_for_next_window.prealloc(added_tracks.get_count());

    m_nodes[0].m_handles.add_items(added_tracks);

    pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data_entries;
    make_data_entries(added_tracks, data_entries, g_showemptyitems);

    const data_entry_t* p_data = data_entries.get_ptr();
    const auto count{data_entries.get_count()};

    for (size_t i{0}; i < count; i++) {
        const auto start = i;
        while (p_data[i].m_same_as_next && i + 1 < count)
            i++;
        const auto handles_count = 1 + i - start;

        t_size index_item;
        const auto exact_match = mmh::partial_bsearch(m_nodes.get_count() - 1, m_nodes, node_t::g_compare,
            p_data[start].m_text.get_ptr(), 1, index_item, get_sort_direction());

        if (exact_match) {
            const t_size current_count = m_nodes[index_item].m_handles.get_count();
            const bool selected = !nothing_or_all_node_selected && get_item_selected(index_item);

            m_nodes[index_item].m_handles.set_count(current_count + handles_count);

            for (size_t k{0}; k < handles_count; k++)
                m_nodes[index_item].m_handles[current_count + k] = p_data[start + k].m_handle;

            if (selected && handles_count)
                tracks_for_next_window.add_items_fromptr(
                    m_nodes[index_item].m_handles.get_ptr() + current_count, handles_count);
        } else {
            node_t node;
            node.m_value = p_data[start].m_text.get_ptr();
            node.m_handles.set_count(handles_count);

            for (size_t k{0}; k < handles_count; k++)
                node.m_handles[k] = p_data[start + k].m_handle;

            m_nodes.insert_item(node, index_item);
            InsertItem item;
            insert_items(index_item, 1, &item);
        }
    }

    update_first_node_text(true);

    if (next_window) {
        if (nothing_or_all_node_selected)
            next_window->add_nodes(added_tracks);
        else if (tracks_for_next_window.get_count())
            next_window->add_nodes(tracks_for_next_window);
    }
}

void filter_panel_t::remove_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& removed_tracks)
{
    filter_panel_t* const next_window = get_next_window();
    if (m_field_data.is_empty()) {
        if (next_window)
            next_window->remove_nodes(removed_tracks);
        return;
    }

    metadb_handle_list_t<pfc::alloc_fast_aggressive> tracks_for_next_window;
    tracks_for_next_window.prealloc(removed_tracks.get_count());

    const auto nothing_or_all_node_selected = get_nothing_or_all_node_selected();
    m_nodes[0].remove_handles(removed_tracks);

    pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data_entries;
    make_data_entries(removed_tracks, data_entries, g_showemptyitems);

    const data_entry_t* p_data = data_entries.get_ptr();
    const size_t count = data_entries.get_count();
    pfc::array_t<bool> mask_nodes;
    mask_nodes.set_count(m_nodes.get_count());
    mask_nodes.fill_null();

    for (size_t i{0}; i < count; i++) {
        const auto start = i;
        while (p_data[i].m_same_as_next && i + 1 < count)
            i++;
        const auto group_size = 1 + i - start;

        size_t index_item;
        const auto exact_match = mmh::partial_bsearch(m_nodes.get_count() - 1, m_nodes, node_t::g_compare,
            p_data[start].m_text.get_ptr(), 1, index_item, get_sort_direction());

        if (exact_match) {
            const auto selected = !nothing_or_all_node_selected && get_item_selected(index_item);

            for (size_t k{0}; k < group_size; k++) {
                m_nodes[index_item].m_handles.remove_item(p_data[start + k].m_handle);
                if (selected)
                    tracks_for_next_window.add_item(p_data[start + k].m_handle);
            }

            if (m_nodes[index_item].m_handles.get_count() == 0) {
                mask_nodes[index_item] = true;
            }
        }
    }
    m_nodes.remove_mask(mask_nodes.get_ptr());
    remove_items(pfc::bit_array_table(mask_nodes.get_ptr(), mask_nodes.get_size()));
    update_first_node_text(true);

    if (next_window) {
        if (nothing_or_all_node_selected)
            next_window->remove_nodes(removed_tracks);
        else if (tracks_for_next_window.get_count())
            next_window->remove_nodes(tracks_for_next_window);
    }
}

void filter_panel_t::on_items_added(const pfc::list_base_const_t<metadb_handle_ptr>& handles)
{
    pfc::ptr_list_t<filter_panel_t> windows;
    get_windows(windows);
    t_size index = windows.find_item(this);
    if (index == 0 || index == pfc_infinite) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles_copy{handles};
        add_nodes(handles_copy);
    }
}

void filter_panel_t::on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr>& handles)
{
    pfc::ptr_list_t<filter_panel_t> windows;
    get_windows(windows);
    t_size index = windows.find_item(this);
    if (index == 0 || index == pfc_infinite) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles_copy{handles};
        remove_nodes(handles_copy);
    }
}

void filter_panel_t::update_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& modified_tracks)
{
    filter_panel_t* const next_window = get_next_window();
    if (m_field_data.is_empty()) {
        if (next_window)
            next_window->update_nodes(modified_tracks);
        return;
    }

    const auto modified_tracks_count = modified_tracks.get_count();
    const auto nothing_or_all_node_selected = get_nothing_or_all_node_selected();

    pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data_entries;
    data_entries.prealloc(modified_tracks_count);

    metadb_handle_list_t<pfc::alloc_fast_aggressive> tracks_for_next_window;
    tracks_for_next_window.prealloc(modified_tracks_count);

    make_data_entries(modified_tracks, data_entries, g_showemptyitems);

    auto node_count = m_nodes.get_count();
    for (size_t node_index{1}; node_index < node_count; node_index++) {
        m_nodes[node_index].remove_handles(modified_tracks);
    }

    const data_entry_t* p_data = data_entries.get_ptr();
    const auto data_entries_count = data_entries.get_count();

    for (size_t i{0}; i < data_entries_count; i++) {
        const auto start = i;
        while (p_data[i].m_same_as_next && i + 1 < data_entries_count)
            i++;
        const auto handles_count = 1 + i - start;

        size_t index_item;
        const auto exact_match = mmh::partial_bsearch(m_nodes.get_count() - 1, m_nodes, node_t::g_compare,
            p_data[start].m_text.get_ptr(), 1, index_item, get_sort_direction());

        if (exact_match) {
            const auto current_count = m_nodes[index_item].m_handles.get_count();
            m_nodes[index_item].m_handles.set_count(current_count + handles_count);

            const auto selected = !nothing_or_all_node_selected && get_item_selected(index_item);

            for (size_t k{0}; k < handles_count; k++) {
                m_nodes[index_item].m_handles[current_count + k] = p_data[start + k].m_handle;
            }

            if (selected && handles_count)
                tracks_for_next_window.add_items_fromptr(
                    m_nodes[index_item].m_handles.get_ptr() + current_count, handles_count);
        } else {
            node_t node;
            node.m_value = p_data[start].m_text.get_ptr();
            node.m_handles.set_count(handles_count);

            for (size_t k{0}; k < handles_count; k++)
                node.m_handles[k] = p_data[start + k].m_handle;

            m_nodes.insert_item(node, index_item);
            InsertItem item;
            insert_items(index_item, 1, &item, false);
        }
    }

    node_count = m_nodes.get_count();

    pfc::array_t<bool> mask_nodes;
    mask_nodes.set_count(node_count);
    mask_nodes[0] = false;
    for (size_t node_index{1}; node_index < node_count; ++node_index) {
        mask_nodes[node_index] = m_nodes[node_index].m_handles.get_count() == 0;
    }
    m_nodes.remove_mask(mask_nodes.get_ptr());
    remove_items(pfc::bit_array_table(mask_nodes.get_ptr(), mask_nodes.get_size()), true);
    update_first_node_text(true);

    if (next_window) {
        if (nothing_or_all_node_selected)
            next_window->update_nodes(modified_tracks);
        else if (tracks_for_next_window.get_count())
            next_window->update_nodes(tracks_for_next_window);
    }
}

void filter_panel_t::on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr>& handles)
{
    pfc::ptr_list_t<filter_panel_t> windows;
    get_windows(windows);
    t_size index = windows.find_item(this);
    if (index == 0 || index == pfc_infinite) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles_copy{handles};
        update_nodes(handles_copy);
    }
}

size_t filter_panel_t::make_data_entries(const metadb_handle_list_t<pfc::alloc_fast_aggressive>& tracks,
    pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive>& p_out, bool b_show_empty)
{
    class handle_info_t {
    public:
        metadb_info_container::ptr m_info;
        t_size m_value_count{};
        t_size m_field_index{};
    };

    if (m_field_data.is_empty())
        return 0;

    const auto track_count = tracks.get_count();
    const auto tracks_ptr = tracks.get_ptr();

    if (m_field_data.m_use_script) {
        titleformat_object_wrapper to(m_field_data.m_script);
        p_out.set_count(track_count);
        data_entry_t* pp_out = p_out.get_ptr();
        std::atomic<size_t> node_count{0};
        concurrency::parallel_for(
            size_t{0}, track_count, [&node_count, &to, tracks_ptr, b_show_empty, pp_out](size_t i) {
                pfc::string8_fastalloc buffer;
                buffer.prealloc(32);
                tracks_ptr[i]->format_title(nullptr, buffer, to, nullptr);
                if (b_show_empty || pfc::strlen_max(buffer, 1)) {
                    const size_t node_index = node_count++;
                    pp_out[node_index].m_handle = tracks_ptr[i];
                    pp_out[node_index].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(buffer));
                    pfc::stringcvt::convert_utf8_to_wide_unchecked(pp_out[node_index].m_text.get_ptr(), buffer);
                }
            });
        p_out.set_count(node_count);
    } else {
        pfc::list_t<handle_info_t> infos;
        infos.set_count(track_count);
        handle_info_t* p_infos = infos.get_ptr();

        t_size counter = 0, field_count = m_field_data.m_fields.get_count();

        for (size_t i{0}; i < track_count; i++) {
            if (tracks_ptr[i]->get_info_ref(p_infos[i].m_info)) {
                for (size_t l{0}; l < field_count; l++) {
                    p_infos[i].m_field_index = p_infos[i].m_info->info().meta_find(m_field_data.m_fields[l]);
                    p_infos[i].m_value_count = p_infos[i].m_field_index != pfc_infinite
                        ? p_infos[i].m_info->info().meta_enum_value_count(p_infos[i].m_field_index)
                        : 0;
                    counter += p_infos[i].m_value_count;
                    if (p_infos[i].m_value_count)
                        break;
                }
            } else
                p_infos[i].m_value_count = 0;
        }

        p_out.set_count(counter);

        data_entry_t* pp_out = p_out.get_ptr();
        std::atomic<size_t> out_counter{0};

        concurrency::parallel_for(
            size_t{0}, track_count, [&out_counter, tracks_ptr, p_infos, b_show_empty, pp_out](size_t i) {
                for (size_t j{0}; j < p_infos[i].m_value_count; j++) {
                    const char* str = p_infos[i].m_info->info().meta_enum_value(p_infos[i].m_field_index, j);
                    if (b_show_empty || *str) {
                        size_t out_index = out_counter++;
                        pp_out[out_index].m_handle = tracks_ptr[i];
                        pp_out[out_index].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(str));
                        pfc::stringcvt::convert_utf8_to_wide_unchecked(pp_out[out_index].m_text.get_ptr(), str);
                    }
                }
            });
        p_out.set_count(out_counter);
    }

    mmh::in_place_sort(p_out, data_entry_t::g_compare, false, get_sort_direction(), true);
    const auto data_entries_count{p_out.get_count()};

    data_entry_t* p_data = p_out.get_ptr();
    concurrency::combinable<size_t> counts;
    concurrency::parallel_for(size_t{0}, data_entries_count, [&counts, p_data, data_entries_count](size_t i) {
        if (i + 1 == data_entries_count) {
            p_data[i].m_same_as_next = false;
        } else {
            p_data[i].m_same_as_next = !StrCmpI(p_data[i].m_text.get_ptr(), p_data[i + 1].m_text.get_ptr());
        }

        if (!p_data[i].m_same_as_next)
            ++counts.local();
    });
    return counts.combine(std::plus<>());
}

void filter_panel_t::populate_list(const metadb_handle_list_t<pfc::alloc_fast>& handles)
{
    clear_all_items();
    m_nodes.remove_all();

    pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data_entries;

    const auto node_count = make_data_entries(handles, data_entries, g_showemptyitems);

    pfc::list_t<uih::ListView::InsertItem, pfc::alloc_fast_aggressive> items;
    items.prealloc(node_count);

    data_entry_t* p_data = data_entries.get_ptr();
    const size_t data_entries_count{data_entries.get_count()};

    m_nodes.set_count(node_count + 1);
    node_t* p_nodes = m_nodes.get_ptr();
    p_nodes[0].m_handles.add_items(handles);
    p_nodes[0].m_value.set_string(L"All");

    for (size_t i{0}, j{1}; i < data_entries_count; i++) {
        const size_t start{i};
        while (p_data[i].m_same_as_next && i + 1 < data_entries_count)
            i++;
        const size_t handles_count{1 + i - start};

        PFC_ASSERT(j < m_nodes.get_count());

        p_nodes[j].m_handles.set_count(handles_count);
        for (t_size k{0}; k < handles_count; k++)
            p_nodes[j].m_handles[k] = p_data[start + k].m_handle;
        p_nodes[j].m_value = p_data[start].m_text.get_ptr();
        j++;
    }
    update_first_node_text();

    items.set_count(m_nodes.get_count());
    insert_items(0, items.get_count(), items.get_ptr());
}

void filter_panel_t::notify_sort_column(t_size index, bool b_descending, bool b_selection_only)
{
    const auto node_count = m_nodes.get_count();
    if (node_count > 2) {
        mmh::Permutation sort_permuation(node_count - 1);
        const auto* nodes = m_nodes.get_ptr();
        ++nodes;
        mmh::sort_get_permutation(nodes, sort_permuation, node_t::g_compare_ptr_with_node, false, b_descending, true);

        m_nodes.reorder_partial(1, sort_permuation.get_ptr(), node_count - 1);

        reorder_items_partial(1, sort_permuation.get_ptr(), node_count - 1);
        ensure_visible(get_focus_item());

        size_t field_index;
        if (cfg_field_list.find_by_name(m_field_data.m_name, field_index)) {
            cfg_field_list[field_index].m_last_sort_direction = b_descending;
        }
    }
}
} // namespace filter_panel