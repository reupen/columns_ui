#include "stdafx.h"
#include "playlist_switcher_v2.h"

namespace cui::panels::playlist_switcher {

void PlaylistSwitcher::on_items_added(t_size p_playlist, t_size p_start,
    const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const pfc::bit_array& p_selection)
{
    refresh_items(p_playlist, 1);
}
void PlaylistSwitcher::on_items_removed(
    t_size p_playlist, const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count)
{
    refresh_items(p_playlist, 1);
}

void PlaylistSwitcher::on_items_modified(t_size p_playlist, const pfc::bit_array& p_mask)
{
    refresh_items(p_playlist, 1);
}

void PlaylistSwitcher::on_items_replaced(
    t_size p_playlist, const pfc::bit_array& p_mask, const pfc::list_base_const_t<t_on_items_replaced_entry>& p_data)
{
    refresh_items(p_playlist, 1);
}

void PlaylistSwitcher::on_playlist_activate(t_size p_old, t_size p_new)
{
    if (p_old != pfc_infinite && p_old < get_item_count())
        refresh_items(p_old, 1, false);
    if (p_new != pfc_infinite && p_new < get_item_count()) {
        refresh_items(p_new, 1);
        set_item_selected_single(p_new, false);
        ensure_visible(p_new);
    } else
        set_selection_state(pfc::bit_array_true(), pfc::bit_array_false(), false);
};
void PlaylistSwitcher::on_playlist_created(t_size p_index, const char* p_name, t_size p_name_len)
{
    refresh_playing_playlist();
    add_items(p_index, 1);
};
void PlaylistSwitcher::on_playlists_reorder(const t_size* p_order, t_size p_count)
{
    refresh_playing_playlist();
    t_size start = 0;
    for (t_size i = 0; i < p_count; i++) {
        start = i;
        while (i < p_count && p_order[i] != i)
            i++;
        if (i > start)
            refresh_items(start, i - start);
    }
    t_size index = m_playlist_api->get_active_playlist();
    if (index != pfc_infinite)
        set_item_selected_single(index, false);
};
void PlaylistSwitcher::on_playlists_removed(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count)
{
    refresh_playing_playlist();
    remove_items(p_mask);
}
void PlaylistSwitcher::on_playlist_renamed(t_size p_index, const char* p_new_name, t_size p_new_name_len)
{
    refresh_items(p_index, 1);
};

void PlaylistSwitcher::on_playlist_locked(t_size p_playlist, bool p_locked)
{
    refresh_items(p_playlist, 1);
};

void PlaylistSwitcher::on_playback_starting(play_control::t_track_command p_command, bool p_paused)
{
    on_playing_playlist_change(get_playing_playlist());
};
void PlaylistSwitcher::on_playback_new_track(metadb_handle_ptr p_track)
{
    on_playing_playlist_change(get_playing_playlist());
};
void PlaylistSwitcher::on_playback_stop(play_control::t_stop_reason p_reason)
{
    if (p_reason != play_control::stop_reason_shutting_down)
        on_playing_playlist_change(get_playing_playlist());
};

} // namespace cui::panels::playlist_switcher
