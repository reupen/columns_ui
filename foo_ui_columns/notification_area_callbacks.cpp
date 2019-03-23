#include "stdafx.h"
#include "main_window.h"
#include "notification_area.h"

extern HICON g_icon;
extern bool g_icon_created;

class NotificationAreaPlayCallback : public play_callback_static {
public:
    unsigned get_flags() override { return flags; }
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) override { update_systray(true); }

    void on_playback_stop(play_control::t_stop_reason p_reason) override
    {
        if (g_icon_created && p_reason != play_control::stop_reason_shutting_down) {
            uShellNotifyIcon(NIM_MODIFY, cui::main_window.get_wnd(), 1, MSG_NOTICATION_ICON, g_icon,
                core_version_info_v2::get()->get_name());
        }
    }
    void on_playback_seek(double p_time) override {}
    void on_playback_pause(bool b_state) override { update_systray(true, b_state ? 2 : 1); }

    void on_playback_edited(metadb_handle_ptr p_track) override {}

    void on_playback_dynamic_info(const file_info& p_info) override {}
    void on_playback_dynamic_info_track(const file_info& p_info) override { update_systray(true); }
    void on_playback_time(double p_time) override {}
    void on_volume_change(float p_new_val) override {}

private:
    static constexpr auto flags = flag_on_playback_new_track | flag_on_playback_stop | flag_on_playback_pause
        | flag_on_playback_dynamic_info_track;
};

static play_callback_static_factory_t<NotificationAreaPlayCallback> notification_area_play_callback;
