#include "stdafx.h"
#include "status_bar.h"

extern HWND g_status;

namespace cui::status_bar {

class StatusBarPlayCalllback : public play_callback_static {
public:
    enum { flags = flag_on_playback_all | flag_on_volume_change };

    unsigned get_flags() override { return flags; }

    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override
    {
        set_playback_information("Loading track.."sv);
    }

    void on_playback_new_track(metadb_handle_ptr p_track) override { regenerate_text(); }

    void on_playback_stop(play_control::t_stop_reason p_reason) override
    {
        if (p_reason != play_control::stop_reason_shutting_down) {
            set_playback_information({core_version_info::g_get_version_string()});
        }
    }
    void on_playback_seek(double p_time) override { regenerate_text(); }
    void on_playback_pause(bool b_state) override { regenerate_text(); }

    void on_playback_edited(metadb_handle_ptr p_track) override { regenerate_text(); }

    void on_playback_dynamic_info(const file_info& p_info) override { regenerate_text(); }
    void on_playback_dynamic_info_track(const file_info& p_info) override { regenerate_text(); }
    void on_playback_time(double p_time) override { regenerate_text(); }
    void on_volume_change(float p_new_val) override { set_part_sizes(t_part_volume); }
};

static play_callback_static_factory_t<StatusBarPlayCalllback> status_bar_play_callback;

class StatusBarPlaylistCallback : public playlist_callback_static {
    unsigned get_flags() override { return flag_on_playlist_activate | flag_on_playlist_locked; }

    void on_items_added(size_t p_playlist, size_t start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const bit_array& p_selection) override
    {
    }
    void on_items_reordered(size_t p_playlist, const size_t* order, size_t count) override {}

    void on_items_removing(size_t p_playlist, const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override
    {
    }
    void on_items_removed(size_t p_playlist, const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override
    {
    }

    void on_items_selection_change(size_t p_playlist, const bit_array& affected, const bit_array& state) override {}
    void on_item_focus_change(size_t p_playlist, size_t from, size_t to) override {}
    void on_items_modified(size_t p_playlist, const bit_array& p_mask) override {}
    void on_items_modified_fromplayback(
        size_t p_playlist, const bit_array& p_mask, play_control::t_display_level p_level) override
    {
    }
    void on_items_replaced(size_t p_playlist, const bit_array& p_mask,
        const pfc::list_base_const_t<t_on_items_replaced_entry>& p_data) override
    {
    }
    void on_item_ensure_visible(size_t p_playlist, size_t idx) override {}

    void on_playlist_activate(size_t p_old, size_t p_new) override
    {
        if (main_window::config_get_status_show_lock())
            set_part_sizes(t_part_lock);
    }

    void on_playlist_created(size_t p_index, const char* p_name, size_t p_name_len) override {}
    void on_playlists_reorder(const size_t* p_order, size_t p_count) override {}
    void on_playlists_removing(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override {}
    void on_playlists_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override {}
    void on_playlist_renamed(size_t p_index, const char* p_new_name, size_t p_new_name_len) override {}

    void on_default_format_changed() override {}
    void on_playback_order_changed(size_t p_new_index) override {}
    void on_playlist_locked(size_t p_playlist, bool p_locked) override
    {
        if (main_window::config_get_status_show_lock())
            set_part_sizes(t_part_lock);
    }

    virtual void on_item_replaced(size_t pls, size_t item, metadb_handle* from, metadb_handle* to) {}
};

static service_factory_single_t<StatusBarPlaylistCallback> status_bar_playlist_callback;

} // namespace cui::status_bar
