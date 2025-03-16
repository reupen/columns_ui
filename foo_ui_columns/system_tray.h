#pragma once

namespace cui::systray {

enum class BalloonTipTitle {
    NowPlaying,
    Unpaused,
    Paused
};

void create_icon_handle();
void create_icon();
void update_icon_tooltip(std::optional<BalloonTipTitle> balloon_tip_title = {}, bool force_balloon = false);
void remove_icon();
void on_show_icon_change();

} // namespace cui::systray
