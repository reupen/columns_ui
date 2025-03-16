#include "pch.h"

#include "system_tray.h"
#include "main_window.h"
#include "win32.h"

extern HICON g_icon;
extern bool g_icon_created;

namespace cui::systray {

namespace {

const std::unordered_map<systray::BalloonTipTitle, const char*> balloon_tip_title_map = {
    {systray::BalloonTipTitle::NowPlaying, "Now playing:"},
    {systray::BalloonTipTitle::Paused, "Paused:"},
    {systray::BalloonTipTitle::Unpaused, "Unpaused:"},
};

std::string get_tooltip_text()
{
    const auto playback_api = playback_control::get();

    metadb_handle_ptr track;
    playback_api->get_now_playing(track);
    std::string title;

    if (track.is_valid()) {
        service_ptr_t<titleformat_object> to_systray;
        titleformat_compiler::get()->compile_safe(to_systray, main_window::config_system_tray_icon_script.get());

        mmh::StringAdaptor adapted_string(title);
        playback_api->playback_format_title_ex(
            track, nullptr, adapted_string, to_systray, nullptr, play_control::display_level_titles);

        track.release();
    } else {
        title = core_version_info_v2::get()->get_name();
    }

    return title;
}

std::string escape_tooltip_text(std::string text)
{
    if (win32::is_windows_11_rtm_or_newer())
        return text;

    std::string escaped_text;
    mmh::StringAdaptor adapted_escaped_text(escaped_text);
    uFixAmpersandChars(text.c_str(), adapted_escaped_text);

    return escaped_text;
}

} // namespace

void update_icon_tooltip(std::optional<BalloonTipTitle> balloon_tip_title, bool force_balloon)
{
    if (!g_icon_created)
        return;

    const auto title = get_tooltip_text();
    const auto escaped_title = escape_tooltip_text(title);

    if (balloon_tip_title && (cfg_balloon || force_balloon)) {
        uShellNotifyIconEx(
            NIM_MODIFY, main_window.get_wnd(), 1, MSG_SYSTEM_TRAY_ICON, g_icon, escaped_title.c_str(), "", "");
        uShellNotifyIconEx(NIM_MODIFY, main_window.get_wnd(), 1, MSG_SYSTEM_TRAY_ICON, g_icon, escaped_title.c_str(),
            balloon_tip_title_map.at(*balloon_tip_title), title.c_str());
    } else
        uShellNotifyIcon(NIM_MODIFY, main_window.get_wnd(), 1, MSG_SYSTEM_TRAY_ICON, g_icon, escaped_title.c_str());
}

void remove_icon()
{
    if (g_icon_created) {
        uShellNotifyIcon(NIM_DELETE, main_window.get_wnd(), 1, MSG_SYSTEM_TRAY_ICON, nullptr, nullptr);
        g_icon_created = false;
    }
}

void on_show_icon_change()
{
    if (!main_window.get_wnd())
        return;

    const auto is_iconic = IsIconic(main_window.get_wnd()) != 0;
    const auto close_to_icon = config::advbool_close_to_system_tray_icon.get();

    if (cfg_show_systray && !g_icon_created) {
        create_icon();
    } else if (!cfg_show_systray && g_icon_created && (!is_iconic || !(cfg_minimise_to_tray || close_to_icon))) {
        remove_icon();
        if (is_iconic)
            standard_commands::main_activate();
    }
}

void create_icon()
{
    const auto tooltip_text = escape_tooltip_text(get_tooltip_text());
    uShellNotifyIcon(g_icon_created ? NIM_MODIFY : NIM_ADD, main_window.get_wnd(), 1, MSG_SYSTEM_TRAY_ICON, g_icon,
        tooltip_text.c_str());

    // NIM_SETVERSION is not used as caused some undesirable behaviour with some mouse clicks.

    g_icon_created = true;
}

void create_icon_handle()
{
    const unsigned cx = GetSystemMetrics(SM_CXSMICON);
    const unsigned cy = GetSystemMetrics(SM_CYSMICON);

    if (g_icon) {
        DestroyIcon(g_icon);
        g_icon = nullptr;
    }

    if (cfg_custom_icon)
        g_icon
            = (HICON)uLoadImage(core_api::get_my_instance(), cfg_tray_icon_path, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);

    if (!g_icon)
        g_icon = ui_control::get()->load_main_icon(cx, cy);
}

} // namespace cui::systray
