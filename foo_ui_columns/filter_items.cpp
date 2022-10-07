#include "pch.h"
#include "filter.h"
#include "filter_config_var.h"

namespace cui::panels::filter {
void FilterPanel::populate_list_from_chain(const metadb_handle_list_t<pfc::alloc_fast>& handles, bool b_last_in_chain)
{
    bool b_redraw = disable_redrawing();
    std::vector<std::wstring> previous_nodes;
    bool b_all_was_selected = false;
    if (m_nodes.get_count()) {
        pfc::list_t<bool> sel_data;
        sel_data.set_count(m_nodes.get_count());
        pfc::bit_array_var_table selection(sel_data.get_ptr(), sel_data.get_count());
        get_selection_state(selection);
        size_t count = sel_data.get_count();
        b_all_was_selected = selection[0];
        for (size_t i = 1; i < count; i++)
            if (selection[i])
                previous_nodes.emplace_back(m_nodes[i].m_value);
    }

    populate_list(handles);

    size_t count = previous_nodes.size();
    pfc::array_t<bool> new_selection;
    new_selection.set_count(m_nodes.get_count());
    new_selection.fill_null();
    if (count || b_all_was_selected) {
        bool b_found = false;
        new_selection[0] = b_all_was_selected;
        for (size_t i = 0; i < count; i++) {
            size_t index;
            if (mmh::partial_bsearch(m_nodes.get_count() - 1, m_nodes, Node::g_compare, previous_nodes[i].c_str(), 1,
                    index, get_sort_direction())) {
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

void FilterPanel::add_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& added_tracks)
{
    FilterPanel* const next_window = get_next_window();
    if (m_field_data.is_empty()) {
        if (next_window)
            next_window->add_nodes(added_tracks);
        return;
    }

    const auto nothing_or_all_node_selected = get_nothing_or_all_node_selected();

    metadb_handle_list_t<pfc::alloc_fast_aggressive> tracks_for_next_window;
    tracks_for_next_window.prealloc(added_tracks.get_count());

    m_nodes[0].m_handles.add_items(added_tracks);

    std::vector<DataEntry> data_entries;
    make_data_entries(added_tracks, data_entries, g_showemptyitems);

    const DataEntry* p_data = data_entries.data();
    const auto count{data_entries.size()};

    auto transaction = start_transaction();

    for (size_t i{0}; i < count; i++) {
        const auto start = i;
        while (p_data[i].m_same_as_next && i + 1 < count)
            i++;
        const auto handles_count = 1 + i - start;

        size_t index_item;
        const auto exact_match = mmh::partial_bsearch(m_nodes.get_count() - 1, m_nodes, Node::g_compare,
            p_data[start].m_text.get_ptr(), 1, index_item, get_sort_direction());

        if (exact_match) {
            const size_t current_count = m_nodes[index_item].m_handles.get_count();
            const bool selected = !nothing_or_all_node_selected && get_item_selected(index_item);

            m_nodes[index_item].m_handles.set_count(current_count + handles_count);

            for (size_t k{0}; k < handles_count; k++)
                m_nodes[index_item].m_handles[current_count + k] = p_data[start + k].m_handle;

            if (selected && handles_count)
                tracks_for_next_window.add_items_fromptr(
                    m_nodes[index_item].m_handles.get_ptr() + current_count, handles_count);
        } else {
            Node node;
            node.m_value = p_data[start].m_text.get_ptr();
            node.m_handles.set_count(handles_count);

            for (size_t k{0}; k < handles_count; k++)
                node.m_handles[k] = p_data[start + k].m_handle;

            m_nodes.insert_item(node, index_item);
            InsertItem item;
            transaction.insert_items(index_item, 1, &item);
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

void FilterPanel::remove_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& removed_tracks)
{
    FilterPanel* const next_window = get_next_window();
    if (m_field_data.is_empty()) {
        if (next_window)
            next_window->remove_nodes(removed_tracks);
        return;
    }

    metadb_handle_list_t<pfc::alloc_fast_aggressive> tracks_for_next_window;
    tracks_for_next_window.prealloc(removed_tracks.get_count());

    const auto nothing_or_all_node_selected = get_nothing_or_all_node_selected();
    m_nodes[0].remove_handles(removed_tracks);

    std::vector<DataEntry> data_entries;
    make_data_entries(removed_tracks, data_entries, g_showemptyitems);

    const DataEntry* p_data = data_entries.data();
    const size_t count = data_entries.size();
    pfc::array_t<bool> mask_nodes;
    mask_nodes.set_count(m_nodes.get_count());
    mask_nodes.fill_null();

    for (size_t i{0}; i < count; i++) {
        const auto start = i;
        while (p_data[i].m_same_as_next && i + 1 < count)
            i++;
        const auto group_size = 1 + i - start;

        size_t index_item;
        const auto exact_match = mmh::partial_bsearch(m_nodes.get_count() - 1, m_nodes, Node::g_compare,
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

void FilterPanel::on_items_added(const pfc::list_base_const_t<metadb_handle_ptr>& handles)
{
    pfc::ptr_list_t<FilterPanel> windows;
    get_windows(windows);
    size_t index = windows.find_item(this);
    if (index == 0 || index == pfc_infinite) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles_copy{handles};
        add_nodes(handles_copy);
    }
}

void FilterPanel::on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr>& handles)
{
    pfc::ptr_list_t<FilterPanel> windows;
    get_windows(windows);
    size_t index = windows.find_item(this);
    if (index == 0 || index == pfc_infinite) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles_copy{handles};
        remove_nodes(handles_copy);
    }
}

void FilterPanel::update_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& modified_tracks)
{
    FilterPanel* const next_window = get_next_window();
    if (m_field_data.is_empty()) {
        if (next_window)
            next_window->update_nodes(modified_tracks);
        return;
    }

    const auto modified_tracks_count = modified_tracks.get_count();
    const auto nothing_or_all_node_selected = get_nothing_or_all_node_selected();

    std::vector<DataEntry> data_entries;

    metadb_handle_list_t<pfc::alloc_fast_aggressive> tracks_for_next_window;
    tracks_for_next_window.prealloc(modified_tracks_count);

    make_data_entries(modified_tracks, data_entries, g_showemptyitems);

    auto node_count = m_nodes.get_count();
    for (size_t node_index{1}; node_index < node_count; node_index++) {
        m_nodes[node_index].remove_handles(modified_tracks);
    }

    const DataEntry* p_data = data_entries.data();
    const auto data_entries_count = data_entries.size();

    auto transaction = start_transaction();

    for (size_t i{0}; i < data_entries_count; i++) {
        const auto start = i;
        while (p_data[i].m_same_as_next && i + 1 < data_entries_count)
            i++;
        const auto handles_count = 1 + i - start;

        size_t index_item;
        const auto exact_match = mmh::partial_bsearch(m_nodes.get_count() - 1, m_nodes, Node::g_compare,
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
            Node node;
            node.m_value = p_data[start].m_text.get_ptr();
            node.m_handles.set_count(handles_count);

            for (size_t k{0}; k < handles_count; k++)
                node.m_handles[k] = p_data[start + k].m_handle;

            m_nodes.insert_item(node, index_item);
            InsertItem item;
            transaction.insert_items(index_item, 1, &item);
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
    transaction.remove_items(pfc::bit_array_table(mask_nodes.get_ptr(), mask_nodes.get_size()));
    update_first_node_text(true);

    if (next_window) {
        if (nothing_or_all_node_selected)
            next_window->update_nodes(modified_tracks);
        else if (tracks_for_next_window.get_count())
            next_window->update_nodes(tracks_for_next_window);
    }
}

void FilterPanel::on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr>& handles)
{
    pfc::ptr_list_t<FilterPanel> windows;
    get_windows(windows);
    size_t index = windows.find_item(this);
    if (index == 0 || index == pfc_infinite) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles_copy{handles};
        update_nodes(handles_copy);
    }
}

size_t FilterPanel::make_data_entries(const metadb_handle_list_t<pfc::alloc_fast_aggressive>& tracks,
    std::vector<DataEntry>& p_out, bool b_show_empty) const
{
    if (m_field_data.is_empty())
        return 0;

    const auto track_count = tracks.get_count();
    const auto tracks_ptr = tracks.get_ptr();
    const auto has_metadb_v2 = static_api_test_t<metadb_v2>();

    if (m_field_data.m_use_script) {
        if (has_metadb_v2)
            p_out = make_data_entries_using_script_fb2k_v2(tracks, b_show_empty);
        else
            p_out = make_data_entries_using_script_fb2k_v1(tracks, b_show_empty);
    } else {
        if (has_metadb_v2)
            p_out = make_data_entries_using_metadata_fb2k_v2(tracks, b_show_empty);
        else
            p_out = make_data_entries_using_metadata_fb2k_v1(tracks, b_show_empty);
    }

    mmh::in_place_sort(p_out, DataEntry::g_compare, false, get_sort_direction(), true);
    const auto data_entries_count{p_out.size()};

    DataEntry* p_data = p_out.data();
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

std::vector<DataEntry> FilterPanel::make_data_entries_using_script_fb2k_v2(
    const metadb_handle_list_t<pfc::alloc_fast_aggressive>& tracks, bool b_show_empty) const
{
    const auto track_count = tracks.get_count();
    std::vector<DataEntry> data_entries{track_count};

    titleformat_object_wrapper to(m_field_data.m_script);
    std::atomic<size_t> node_count{0};

    metadb_v2::get()->queryMultiParallel_(
        tracks, [&tracks, &data_entries, &to, &node_count, b_show_empty](size_t index, const metadb_v2::rec_t& rec) {
            metadb_handle_v2::ptr track;
            track &= tracks[index];

            std::string title;
            mmh::StringAdaptor adapted_string(title);

            track->formatTitle_v2(rec, nullptr, adapted_string, to, nullptr);

            if (!b_show_empty && title.empty())
                return;

            const size_t node_index = node_count++;
            data_entries[node_index].m_handle = track;
            data_entries[node_index].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(title.c_str()));
            pfc::stringcvt::convert_utf8_to_wide_unchecked(data_entries[node_index].m_text.get_ptr(), title.c_str());
        });

    data_entries.resize(node_count);
    return data_entries;
}

std::vector<DataEntry> FilterPanel::make_data_entries_using_script_fb2k_v1(
    const metadb_handle_list_t<pfc::alloc_fast_aggressive>& tracks, bool b_show_empty) const
{
    const auto track_count = tracks.get_count();
    std::vector<DataEntry> data_entries{track_count};

    titleformat_object_wrapper to(m_field_data.m_script);
    std::atomic<size_t> node_count{0};

    concurrency::parallel_for(
        size_t{0}, track_count, [&data_entries, &tracks, &node_count, &to, b_show_empty](size_t index) {
            pfc::string8_fastalloc buffer;
            buffer.prealloc(32);
            tracks[index]->format_title(nullptr, buffer, to, nullptr);

            if (!b_show_empty && buffer.is_empty())
                return;

            const size_t node_index = node_count++;
            data_entries[node_index].m_handle = tracks[index];
            data_entries[node_index].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(buffer));
            pfc::stringcvt::convert_utf8_to_wide_unchecked(data_entries[node_index].m_text.get_ptr(), buffer);
        });

    data_entries.resize(node_count);
    return data_entries;
}

std::vector<DataEntry> FilterPanel::make_data_entries_using_metadata_fb2k_v2(
    const metadb_handle_list_t<pfc::alloc_fast_aggressive>& tracks, bool b_show_empty) const
{
    const auto track_count = tracks.get_count();
    size_t field_count = m_field_data.m_fields.size();

    concurrency::concurrent_vector<DataEntry> concurrent_out;
    concurrent_out.reserve(track_count);

    metadb_v2::get()->queryMultiParallel_(tracks,
        [this, &tracks, &concurrent_out, field_count, b_show_empty](size_t track_index, const metadb_v2::rec_t& rec) {
            if (rec.info.is_empty())
                return;

            const auto& info = rec.info->info();
            for (size_t field_index{}; field_index < field_count; field_index++) {
                const auto meta_index = info.meta_find(m_field_data.m_fields[field_index]);
                if (meta_index == std::numeric_limits<size_t>::max())
                    continue;

                const auto value_count = info.meta_enum_value_count(meta_index);

                for (size_t value_index{}; value_index < value_count; value_index++) {
                    const char* str = info.meta_enum_value(meta_index, value_index);

                    if (!b_show_empty && *str == 0)
                        continue;

                    DataEntry entry;
                    entry.m_handle = tracks[track_index];
                    entry.m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(str));
                    pfc::stringcvt::convert_utf8_to_wide_unchecked(entry.m_text.get_ptr(), str);

                    concurrent_out.push_back(std::move(entry));
                }
                break;
            }
        });

    std::vector<DataEntry> data_entries;
    data_entries.reserve(concurrent_out.size());
    std::ranges::move(concurrent_out, std::back_inserter(data_entries));
    return data_entries;
}

std::vector<DataEntry> FilterPanel::make_data_entries_using_metadata_fb2k_v1(
    const metadb_handle_list_t<pfc::alloc_fast_aggressive>& tracks, bool b_show_empty) const
{
    class HandleInfo {
    public:
        metadb_info_container::ptr m_info;
        size_t m_value_count{};
        size_t m_field_index{};
    };

    const auto track_count = tracks.get_count();
    const size_t field_count = m_field_data.m_fields.size();

    std::vector<DataEntry> data_entries;
    data_entries.reserve(track_count);

    pfc::list_t<HandleInfo> infos;
    infos.set_count(track_count);
    size_t counter = 0;

    for (size_t track_index{0}; track_index < track_count; track_index++) {
        if (!tracks[track_index]->get_info_ref(infos[track_index].m_info))
            continue;

        for (size_t field_index{0}; field_index < field_count; field_index++) {
            infos[track_index].m_field_index
                = infos[track_index].m_info->info().meta_find(m_field_data.m_fields[field_index]);
            infos[track_index].m_value_count = infos[track_index].m_field_index != pfc_infinite
                ? infos[track_index].m_info->info().meta_enum_value_count(infos[track_index].m_field_index)
                : 0;
            counter += infos[track_index].m_value_count;

            if (infos[track_index].m_value_count > 0)
                break;
        }
    }

    data_entries.resize(counter);
    std::atomic<size_t> out_counter{0};

    concurrency::parallel_for(
        size_t{0}, track_count, [&tracks, &data_entries, &out_counter, &infos, b_show_empty](size_t track_index) {
            for (size_t value_index{0}; value_index < infos[track_index].m_value_count; value_index++) {
                const char* str
                    = infos[track_index].m_info->info().meta_enum_value(infos[track_index].m_field_index, value_index);

                if (!b_show_empty && *str == 0)
                    return;

                const size_t out_index = out_counter++;
                data_entries[out_index].m_handle = tracks[track_index];
                data_entries[out_index].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(str));
                pfc::stringcvt::convert_utf8_to_wide_unchecked(data_entries[out_index].m_text.get_ptr(), str);
            }
        });

    data_entries.resize(out_counter);
    return data_entries;
}

void FilterPanel::populate_list(const metadb_handle_list_t<pfc::alloc_fast>& handles)
{
    clear_all_items();
    m_nodes.remove_all();

    std::vector<DataEntry> data_entries;
    const auto node_count = make_data_entries(handles, data_entries, g_showemptyitems);

    pfc::list_t<InsertItem, pfc::alloc_fast_aggressive> items;
    items.prealloc(node_count);

    DataEntry* p_data = data_entries.data();
    const size_t data_entries_count{data_entries.size()};

    m_nodes.set_count(node_count + 1);
    Node* p_nodes = m_nodes.get_ptr();
    p_nodes[0].m_handles.add_items(handles);
    p_nodes[0].m_value = L"All";

    for (size_t i{0}, j{1}; i < data_entries_count; i++) {
        const size_t start{i};
        while (p_data[i].m_same_as_next && i + 1 < data_entries_count)
            i++;
        const size_t handles_count{1 + i - start};

        PFC_ASSERT(j < m_nodes.get_count());

        p_nodes[j].m_handles.set_count(handles_count);
        for (size_t k{0}; k < handles_count; k++)
            p_nodes[j].m_handles[k] = p_data[start + k].m_handle;
        p_nodes[j].m_value = p_data[start].m_text.get_ptr();
        j++;
    }
    update_first_node_text();

    items.set_count(m_nodes.get_count());
    insert_items(0, items.get_count(), items.get_ptr());
}

void FilterPanel::notify_sort_column(size_t index, bool b_descending, bool b_selection_only)
{
    const auto node_count = m_nodes.get_count();
    if (node_count > 2) {
        mmh::Permutation sort_permuation(node_count - 1);
        const auto* nodes = m_nodes.get_ptr();
        ++nodes;
        sort_get_permutation(nodes, sort_permuation, Node::g_compare_ptr_with_node, false, b_descending, true);

        m_nodes.reorder_partial(1, sort_permuation.data(), node_count - 1);

        reorder_items_partial(1, sort_permuation.data(), node_count - 1);
        ensure_visible(get_focus_item());

        size_t field_index;
        if (cfg_field_list.find_by_name(m_field_data.m_name, field_index)) {
            cfg_field_list[field_index].m_last_sort_direction = b_descending;
        }
    }
}
} // namespace cui::panels::filter
