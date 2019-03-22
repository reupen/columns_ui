#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"
#include "seekbar.h"
#include "main_window.h"
#include "notification_area.h"

class config_object_notify_pfc : public config_object_notify {
public:
    t_size get_watched_object_count() override { return 1; }
    GUID get_watched_object(t_size p_index) override { return standard_config_objects::bool_playback_follows_cursor; }
    void on_watched_object_changed(const service_ptr_t<config_object>& p_object) override
    {
        bool val = p_object->get_data_bool_simple(false);
        pvt::ng_playlist_view_t::g_on_playback_follows_cursor_change(val);
    }
};

service_factory_single_t<config_object_notify_pfc> g_config_object_notify_pfc;

class play_callback_ui : public play_callback_static {
public:
    enum { flags = flag_on_playback_all };

    unsigned get_flags() override { return flags; }
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) override
    {
        g_playing = true;
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
            update_systray(true);
            cui::main_window.queue_taskbar_button_update();
        }

        seek_bar_extension::update_seekbars();
        seek_bar_extension::update_seek_timer();
    }

    void on_playback_stop(play_control::t_stop_reason p_reason) override
    {
        g_playing = false;
        seek_bar_extension::update_seek_timer();
        if (cui::main_window.get_wnd() && p_reason != play_control::stop_reason_shutting_down) {
            cui::main_window.reset_title();

            if (g_icon_created)
                uShellNotifyIcon(NIM_MODIFY, cui::main_window.get_wnd(), 1, MSG_NOTICATION_ICON, g_icon,
                    core_version_info_v2::get()->get_name());
            cui::main_window.queue_taskbar_button_update();
        }
        if (p_reason != play_control::stop_reason_shutting_down)
            seek_bar_extension::update_seekbars();
    }
    void on_playback_seek(double p_time) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
        }
        seek_bar_extension::update_seekbars(true);
    }
    void on_playback_pause(bool b_state) override
    {
        g_playing = (b_state == 0);
        seek_bar_extension::update_seek_timer();
        if (cui::main_window.get_wnd()) {
            update_systray(true, b_state ? 2 : 1);
            cui::main_window.update_title();
            cui::main_window.queue_taskbar_button_update();
        }
    }

    void on_playback_edited(metadb_handle_ptr p_track) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
        }
    }

    void on_playback_dynamic_info(const file_info& p_info) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
        }
    }
    void on_playback_dynamic_info_track(const file_info& p_info) override
    {
        if (cui::main_window.get_wnd()) {
            update_systray(true);
            cui::main_window.update_title();
        }
    }
    void on_playback_time(double p_time) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
        }
    }
    void on_volume_change(float p_new_val) override {}
};

static play_callback_static_factory_t<play_callback_ui> blah;
