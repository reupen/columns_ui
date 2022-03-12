#include "pch.h"
#include "ng_playlist/ng_playlist.h"

class PlaybackFollowsCursorCallback : public config_object_notify {
public:
    size_t get_watched_object_count() override { return 1; }
    GUID get_watched_object(size_t p_index) override { return standard_config_objects::bool_playback_follows_cursor; }
    void on_watched_object_changed(const service_ptr_t<config_object>& p_object) override
    {
        const auto val = p_object->get_data_bool_simple(false);
        cui::panels::playlist_view::PlaylistView::g_on_playback_follows_cursor_change(val);
    }
};

service_factory_single_t<PlaybackFollowsCursorCallback> playback_follows_cursor_callback;
