#pragma once

namespace cui::systray {

extern bool is_system_tray_icon_created;

enum class BalloonTipTitle {
    NowPlaying,
    Paused,
    Resumed,
};

void create_icon_handle();
void create_icon();
void update_icon_tooltip(std::optional<BalloonTipTitle> balloon_tip_title = {}, bool force_balloon = false);
void remove_icon();
void deinitialise();
void on_show_icon_change();

} // namespace cui::systray
