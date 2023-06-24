#include "pch.h"
#include "ng_playlist.h"

namespace cui::panels::playlist_view {
void PlaylistView::on_items_added(size_t playlist, size_t start,
    const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection)
{
    if (playlist != m_playlist_api->get_active_playlist())
        return;

    clear_sort_column();
    InsertItemsContainer items;
    get_insert_items(start, p_data.get_count(), items);
    insert_items(start, items.get_count(), items.get_ptr());
    refresh_all_items_text();
}

void PlaylistView::on_items_reordered(size_t playlist, const size_t* p_order, size_t p_count)
{
    if (playlist != m_playlist_api->get_active_playlist())
        return;

    clear_sort_column();
    for (auto i : std::ranges::views::iota(size_t{}, p_count)) {
        const size_t start = i;
        while (i < p_count && p_order[i] != i) {
            i++;
        }
        if (i > start) {
            InsertItemsContainer items;
            get_insert_items(start, i - start, items);
            replace_items(start, items);
        }
    }
}

void PlaylistView::on_items_removed(size_t playlist, const bit_array& p_mask, size_t p_old_count, size_t p_new_count)
{
    if (playlist != m_playlist_api->get_active_playlist())
        return;

    clear_sort_column();

    if (p_new_count == 0) {
        remove_all_items();
        return;
    }

    remove_items(p_mask);
    refresh_all_items_text();
}

void PlaylistView::on_items_selection_change(size_t playlist, const bit_array& p_affected, const bit_array& p_state)
{
    if (m_ignore_callback || playlist != m_playlist_api->get_active_playlist())
        return;

    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
}

void PlaylistView::on_item_focus_change(size_t playlist, size_t p_from, size_t p_to)
{
    if (m_ignore_callback || playlist != m_playlist_api->get_active_playlist())
        return;

    on_focus_change(p_from, p_to);
}

void PlaylistView::on_items_modified(size_t playlist, const bit_array& p_mask)
{
    if (playlist != m_playlist_api->get_active_playlist())
        return;

    clear_sort_column();
    const size_t count = m_playlist_api->activeplaylist_get_item_count();

    for (auto i : std::ranges::views::iota(size_t{}, count)) {
        const size_t start = i;
        while (i < count && p_mask[i]) {
            i++;
        }
        if (i > start) {
            InsertItemsContainer items;
            get_insert_items(start, i - start, items);
            replace_items(start, items);
        }
    }
}

void PlaylistView::on_items_modified_fromplayback(
    size_t playlist, const bit_array& p_mask, play_control::t_display_level p_level)
{
    if (core_api::is_shutting_down() || playlist != m_playlist_api->get_active_playlist())
        return;

    const size_t count = m_playlist_api->activeplaylist_get_item_count();

    for (auto i : std::ranges::views::iota(size_t{}, count)) {
        const size_t start = i;
        while (i < count && p_mask[i]) {
            i++;
        }
        if (i > start) {
            update_items(start, i - start);
        }
    }
}

void PlaylistView::on_items_replaced(
    size_t playlist, const bit_array& p_mask, const pfc::list_base_const_t<t_on_items_replaced_entry>& p_data)
{
    on_items_modified(playlist, p_mask);
}

void PlaylistView::on_item_ensure_visible(size_t playlist, size_t p_idx)
{
    if (playlist != m_playlist_api->get_active_playlist())
        return;

    ensure_visible(p_idx);
}

void PlaylistView::on_playlist_activate(size_t p_old, size_t p_new)
{
    if (p_old != std::numeric_limits<size_t>::max()) {
        m_playlist_cache.set_item(p_old, save_scroll_position());
    }

    clear_sort_column();
    clear_all_items();

    const auto scroll_position = p_new != std::numeric_limits<size_t>::max()
        ? m_playlist_cache.get_item(p_new).saved_scroll_position
        : std::nullopt;

    if (!scroll_position)
        _set_scroll_position(0);

    const auto focus = m_playlist_api->activeplaylist_get_focus_item();

    refresh_groups();
    refresh_columns();
    populate_list(scroll_position);

    if (!scroll_position && focus != std::numeric_limits<size_t>::max())
        ensure_visible(focus);
}

void PlaylistView::on_playlist_renamed(size_t playlist, const char* p_new_name, size_t p_new_name_len)
{
    if (playlist != m_playlist_api->get_active_playlist())
        return;

    clear_sort_column();
    clear_all_items();
    refresh_groups();
    refresh_columns();
    populate_list();
}

} // namespace cui::panels::playlist_view
