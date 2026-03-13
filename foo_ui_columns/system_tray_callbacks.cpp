#include "pch.h"
#include "system_tray.h"

namespace cui::systray {

namespace {

class SystemTrayPlayCallback : public play_callback_static {
public:
    unsigned get_flags() override { return flags; }
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override
    {
        update_icon_tooltip(BalloonTipTitle::NowPlaying);
    }

    void on_playback_stop(play_control::t_stop_reason p_reason) noexcept override
    {
        if (is_system_tray_icon_created && p_reason != play_control::stop_reason_shutting_down)
            update_icon_tooltip();
    }

    void on_playback_seek(double p_time) override {}
    void on_playback_pause(bool b_state) noexcept override
    {
        update_icon_tooltip(b_state ? BalloonTipTitle::Paused : BalloonTipTitle::Resumed);
    }

    void on_playback_edited(metadb_handle_ptr p_track) override {}

    void on_playback_dynamic_info(const file_info& p_info) override {}
    void on_playback_dynamic_info_track(const file_info& p_info) noexcept override
    {
        update_icon_tooltip(BalloonTipTitle::NowPlaying);
    }
    void on_playback_time(double p_time) override {}
    void on_volume_change(float p_new_val) override {}

private:
    static constexpr auto flags = flag_on_playback_new_track | flag_on_playback_stop | flag_on_playback_pause
        | flag_on_playback_dynamic_info_track;
};

play_callback_static_factory_t<SystemTrayPlayCallback> _system_tray_play_callback;

} // namespace

} // namespace cui::systray
