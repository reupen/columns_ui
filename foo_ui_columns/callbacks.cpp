#include "stdafx.h"
#include "main_window.h"

class play_callback_ui : public play_callback_static {
public:
    enum { flags = flag_on_playback_all };

    unsigned get_flags() override { return flags; }
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
            cui::main_window.queue_taskbar_button_update();
        }
    }

    void on_playback_stop(play_control::t_stop_reason p_reason) override
    {
        if (cui::main_window.get_wnd() && p_reason != play_control::stop_reason_shutting_down) {
            cui::main_window.reset_title();
            cui::main_window.queue_taskbar_button_update();
        }
    }
    void on_playback_seek(double p_time) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
        }
    }
    void on_playback_pause(bool b_state) override
    {
        if (cui::main_window.get_wnd()) {
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
