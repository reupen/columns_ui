#include "pch.h"

#include "system_tray.h"
#include "main_window.h"
#include "win32.h"

namespace cui::systray {

bool is_system_tray_icon_created = false;

namespace {

constexpr auto system_tray_icon_id = 1u;

wil::unique_hicon system_tray_icon;

const std::unordered_map<BalloonTipTitle, wil::zwstring_view> balloon_tip_title_map = {
    {BalloonTipTitle::NowPlaying, L"Now playing:"_zv},
    {BalloonTipTitle::Paused, L"Paused:"_zv},
    {BalloonTipTitle::Resumed, L"Resumed:"_zv},
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

std::optional<std::wstring> truncate_string(std::wstring_view text, size_t max_code_units)
{
    if (text.size() <= max_code_units)
        return {};

    std::optional<std::wstring> truncated_text{text};

    size_t code_unit_counter{};
    const wchar_t* pos{truncated_text->c_str()};

    while (true) {
        uint32_t code_point{};
        const auto code_point_length = pfc::utf16_decode_char(pos, &code_point);

        if (code_point_length == 0 || code_unit_counter + code_point_length > max_code_units - 1)
            break;

        pos += code_point_length;
        code_unit_counter += code_point_length;
    }

    truncated_text->resize(code_unit_counter + 1);
    (*truncated_text)[code_unit_counter] = L'…';

    return truncated_text;
}

template <size_t Size>
void copy_and_truncate_string(wil::zwstring_view source, wchar_t (&destination)[Size])
{
    static_assert(Size >= 2);

    const auto truncated_string = truncate_string(source, Size - 1);

    wcsncpy_s(destination, truncated_string ? truncated_string->c_str() : source.c_str(), _TRUNCATE);
}

void create_systray_icon(HICON icon, uint32_t callback_msg, wil::zwstring_view text)
{
    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = main_window.get_wnd();
    nid.uID = system_tray_icon_id;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.hIcon = icon;
    nid.uCallbackMessage = callback_msg;

    copy_and_truncate_string(text, nid.szTip);

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void update_systray_icon_icon(HICON icon)
{
    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = main_window.get_wnd();
    nid.uID = system_tray_icon_id;
    nid.uFlags = NIF_ICON;
    nid.hIcon = icon;

    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void update_systray_icon_tooltip(wil::zwstring_view text)
{
    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = main_window.get_wnd();
    nid.uID = system_tray_icon_id;
    // Set NIF_INFO to remove any previous balloon tip
    nid.uFlags = NIF_TIP | NIF_INFO;

    copy_and_truncate_string(text, nid.szTip);

    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void update_systray_icon_balloon_tip(wil::zwstring_view title, wil::zwstring_view body)
{
    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = main_window.get_wnd();
    nid.uID = system_tray_icon_id;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;

    copy_and_truncate_string(title, nid.szInfoTitle);
    copy_and_truncate_string(body, nid.szInfo);

    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void remove_systray_icon()
{
    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = main_window.get_wnd();
    nid.uID = system_tray_icon_id;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

} // namespace

void update_icon_tooltip(std::optional<BalloonTipTitle> balloon_tip_title, bool force_balloon)
{
    if (!is_system_tray_icon_created)
        return;

    const auto title = get_tooltip_text();
    const auto escaped_title = escape_tooltip_text(title);

    update_systray_icon_tooltip(mmh::to_utf16(escaped_title));

    if (balloon_tip_title && (cfg_balloon || force_balloon)) {
        update_systray_icon_balloon_tip(balloon_tip_title_map.at(*balloon_tip_title), mmh::to_utf16(title));
    }
}

void remove_icon()
{
    if (is_system_tray_icon_created) {
        remove_systray_icon();
        is_system_tray_icon_created = false;
    }
}

void deinitialise()
{
    remove_icon();
    system_tray_icon.reset();
}

void on_show_icon_change()
{
    if (!main_window.get_wnd())
        return;

    const auto is_iconic = IsIconic(main_window.get_wnd()) != 0;
    const auto close_to_icon = config::advbool_close_to_system_tray_icon.get();

    if (cfg_show_systray && !is_system_tray_icon_created) {
        create_icon();
    } else if (!cfg_show_systray && is_system_tray_icon_created
        && (!is_iconic || !(cfg_minimise_to_tray || close_to_icon))) {
        remove_icon();
        if (is_iconic)
            standard_commands::main_activate();
    }
}

void create_icon()
{
    const auto escaped_title = escape_tooltip_text(get_tooltip_text());

    if (is_system_tray_icon_created) {
        update_systray_icon_tooltip(mmh::to_utf16(escaped_title));
    } else {
        create_systray_icon(system_tray_icon.get(), MSG_SYSTEM_TRAY_ICON, mmh::to_utf16(escaped_title));
        // NIM_SETVERSION is not used as caused some undesirable behaviour with some mouse clicks.
    }

    is_system_tray_icon_created = true;
}

void create_icon_handle()
{
    const unsigned cx = GetSystemMetrics(SM_CXSMICON);
    const unsigned cy = GetSystemMetrics(SM_CYSMICON);

    wil::unique_hicon old_icon = std::move(system_tray_icon);

    if (cfg_custom_icon)
        system_tray_icon.reset(
            static_cast<HICON>(uLoadImage(nullptr, cfg_tray_icon_path, IMAGE_ICON, cx, cy, LR_LOADFROMFILE)));

    if (!system_tray_icon)
        system_tray_icon.reset(ui_control::get()->load_main_icon(cx, cy));

    if (is_system_tray_icon_created)
        update_systray_icon_icon(system_tray_icon.get());
}

} // namespace cui::systray
