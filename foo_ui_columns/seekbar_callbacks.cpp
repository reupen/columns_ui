#include "pch.h"
#include "seekbar.h"

namespace cui::toolbars::seekbar {

class SeekBarPlayCallback : public play_callback_static {
public:
    unsigned get_flags() override { return flags; }
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override
    {
        SeekBarToolbar::update_seekbars();
        SeekBarToolbar::update_seek_timer();
    }

    void on_playback_stop(play_control::t_stop_reason p_reason) noexcept override
    {
        SeekBarToolbar::update_seek_timer();
        if (p_reason != play_control::stop_reason_shutting_down)
            SeekBarToolbar::update_seekbars();
    }
    void on_playback_seek(double p_time) noexcept override { SeekBarToolbar::update_seekbars(true); }
    void on_playback_pause(bool b_state) noexcept override { SeekBarToolbar::update_seek_timer(); }

    void on_playback_edited(metadb_handle_ptr p_track) override {}

    void on_playback_dynamic_info(const file_info& p_info) override {}
    void on_playback_dynamic_info_track(const file_info& p_info) override {}
    void on_playback_time(double p_time) override {}
    void on_volume_change(float p_new_val) override {}

private:
    static constexpr auto flags
        = flag_on_playback_new_track | flag_on_playback_stop | flag_on_playback_seek | flag_on_playback_pause;
};

static play_callback_static_factory_t<SeekBarPlayCallback> seek_bar_play_callback;

} // namespace cui::toolbars::seekbar
