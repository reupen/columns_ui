#include "pch.h"
#include "playlist_switcher_v2.h"

namespace cui::panels::playlist_switcher {

void PlaylistSwitcher::on_items_added(size_t p_playlist, size_t p_start,
    const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection) noexcept
{
    refresh_items(p_playlist, 1);
}
void PlaylistSwitcher::on_items_removed(
    size_t p_playlist, const bit_array& p_mask, size_t p_old_count, size_t p_new_count) noexcept
{
    refresh_items(p_playlist, 1);
}

void PlaylistSwitcher::on_items_modified(size_t p_playlist, const bit_array& p_mask) noexcept
{
    refresh_items(p_playlist, 1);
}

void PlaylistSwitcher::on_items_replaced(size_t p_playlist, const bit_array& p_mask,
    const pfc::list_base_const_t<t_on_items_replaced_entry>& p_data) noexcept
{
    refresh_items(p_playlist, 1);
}

void PlaylistSwitcher::on_playlist_activate(size_t p_old, size_t p_new) noexcept
{
    if (p_old != pfc_infinite && p_old < get_item_count())
        refresh_items(p_old, 1, false);
    if (p_new != pfc_infinite && p_new < get_item_count()) {
        refresh_items(p_new, 1);
        set_item_selected_single(p_new, false);
        ensure_visible(p_new);
    } else
        set_selection_state(bit_array_true(), bit_array_false(), false);
}
void PlaylistSwitcher::on_playlist_created(size_t p_index, const char* p_name, size_t p_name_len) noexcept
{
    refresh_playing_playlist();
    add_items(p_index, 1);
}
void PlaylistSwitcher::on_playlists_reorder(const size_t* p_order, size_t p_count) noexcept
{
    refresh_playing_playlist();
    size_t start = 0;
    for (size_t i = 0; i < p_count; i++) {
        start = i;
        while (i < p_count && p_order[i] != i)
            i++;
        if (i > start)
            refresh_items(start, i - start);
    }
    size_t index = m_playlist_api->get_active_playlist();
    if (index != pfc_infinite)
        set_item_selected_single(index, false);
}
void PlaylistSwitcher::on_playlists_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) noexcept
{
    refresh_playing_playlist();
    remove_items(p_mask);
}
void PlaylistSwitcher::on_playlist_renamed(size_t p_index, const char* p_new_name, size_t p_new_name_len) noexcept
{
    refresh_items(p_index, 1);
}

void PlaylistSwitcher::on_playlist_locked(size_t p_playlist, bool p_locked) noexcept
{
    refresh_items(p_playlist, 1);
}

void PlaylistSwitcher::on_playback_starting(play_control::t_track_command p_command, bool p_paused) noexcept
{
    on_playing_playlist_change(get_playing_playlist());
}
void PlaylistSwitcher::on_playback_new_track(metadb_handle_ptr p_track) noexcept
{
    on_playing_playlist_change(get_playing_playlist());
}
void PlaylistSwitcher::on_playback_stop(play_control::t_stop_reason p_reason) noexcept
{
    if (p_reason != play_control::stop_reason_shutting_down)
        on_playing_playlist_change(get_playing_playlist());
}

} // namespace cui::panels::playlist_switcher
