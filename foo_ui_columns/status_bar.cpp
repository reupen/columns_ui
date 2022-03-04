#include "stdafx.h"
#include "status_bar.h"

#include "main_window.h"

extern HWND g_status;

namespace cui::status_bar {

namespace {

struct StatusBarState {
    WNDPROC status_proc{};
    std::string playback_information_text;
    std::string menu_item_description;
    std::string playlist_lock_text;
    std::string track_length_text;
    std::string track_count_text;
    std::string volume_text;
    std::unique_ptr<colours::dark_mode_notifier> dark_mode_notifier;
};

std::optional<StatusBarState> state;

} // namespace

extern HFONT g_status_font;

LRESULT WINAPI g_status_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_ERASEBKGND:
        return FALSE;
    case WM_PAINT: {
        PAINTSTRUCT ps;

        HDC dc = BeginPaint(wnd, &ps);
        HDC dc_mem = CreateCompatibleDC(dc);

        RECT rc;
        GetClientRect(wnd, &rc);

        HBITMAP bm_mem = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
        auto bm_old = (HBITMAP)SelectObject(dc_mem, bm_mem);

        if (colours::is_dark_mode_active())
            FillRect(dc_mem, &rc, dark::get_colour_brush(dark::ColourID::StatusBarBackground, true).get());
        else
            CallWindowProc(state->status_proc, wnd, WM_ERASEBKGND, (WPARAM)dc_mem, NULL);

        SendMessage(wnd, WM_PRINTCLIENT, (WPARAM)dc_mem, PRF_CHECKVISIBLE | PRF_CLIENT | PRF_ERASEBKGND);

        BitBlt(dc, rc.left, rc.top, rc.right, rc.bottom, dc_mem, 0, 0, SRCCOPY);

        SelectObject(dc_mem, bm_old);
        DeleteObject(bm_mem);
        DeleteDC(dc_mem);

        EndPaint(wnd, &ps);
    }
        return 0;
    }

    return CallWindowProc(state->status_proc, wnd, msg, wp, lp);
}

void set_playback_information(std::string_view text)
{
    if (g_status) {
        state->playback_information_text = text;

        if (state->menu_item_description.empty())
            SendMessage(g_status, SB_SETTEXT, SBT_OWNERDRAW | 0, WI_EnumValue(StatusBarPartID::PlaybackInformation));
    }
}

void set_menu_item_description(std::string_view text)
{
    if (g_status) {
        state->menu_item_description = text;

        const auto part_id = text.empty() ? StatusBarPartID::PlaybackInformation : StatusBarPartID::MenuItemDescription;

        SendMessage(g_status, SB_SETTEXT, SBT_OWNERDRAW | 0, WI_EnumValue(part_id));
    }
}

void clear_menu_item_description()
{
    if (g_status) {
        state->menu_item_description.clear();
        SendMessage(g_status, SB_SETTEXT, SBT_OWNERDRAW | 0, WI_EnumValue(StatusBarPartID::PlaybackInformation));
    }
}

void regenerate_text()
{
    if (!g_status)
        return;

    metadb_handle_ptr track;
    const auto play_api = play_control::get();
    play_api->get_now_playing(track);
    if (track.is_valid()) {
        titleformat_object::ptr to_status;
        pfc::string8 text;
        titleformat_compiler::get()->compile_safe(to_status, main_window::config_status_bar_script.get());
        play_api->playback_format_title_ex(track, nullptr, text, to_status, nullptr, play_control::display_level_all);
        set_playback_information(text.get_ptr());
    } else {
        set_playback_information(core_version_info::g_get_version_string());
    }
}

void destroy_window()
{
    if (g_status) {
        DestroyWindow(g_status);
        g_status = nullptr;
    }
    if (g_status_font) {
        DeleteObject(g_status_font);
        g_status_font = nullptr;
    }
    state.reset();
}

volume_popup_t volume_popup_window;

std::string get_playlist_lock_text()
{
    const auto api = playlist_manager::get();
    const auto playlist_index = api->get_active_playlist();

    pfc::string8 lock_name;
    api->playlist_lock_query_name(playlist_index, lock_name);

    return fmt::format("{} {}", u8"🔒"_pcc, lock_name.get_ptr());
}

int calculate_volume_size(const char* p_text)
{
    return win32_helpers::status_bar_get_text_width(g_status, p_text);
}

int calculate_selected_length_size(const char* p_text)
{
    return std::max(win32_helpers::status_bar_get_text_width(g_status, p_text),
        win32_helpers::status_bar_get_text_width(g_status, "0d 00:00:00"));
}

int calculate_selected_count_size(const char* p_text)
{
    return std::max(win32_helpers::status_bar_get_text_width(g_status, p_text),
        win32_helpers::status_bar_get_text_width(g_status, "0,000 tracks"));
}

int calculate_playback_lock_size(const char* p_text)
{
    return win32_helpers::status_bar_get_text_width(g_status, p_text);
}

std::string get_selected_length_text(unsigned dp = 0)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> sels;
    double length = 0;

    const auto playlist_api = playlist_manager::get();
    const auto metadb_api = metadb::get();

    const auto count = playlist_api->activeplaylist_get_selection_count(pfc_infinite);

    sels.prealloc(count);

    playlist_api->activeplaylist_get_selected_items(sels);
    length = sels.calc_total_duration();

    return pfc::format_time_ex(length, dp).get_ptr();
}

std::string get_selected_count_text(unsigned dp = 0)
{
    const auto playlist_api = playlist_manager::get();

    const auto count = playlist_api->activeplaylist_get_selection_count(pfc_infinite);

    return fmt::format(std::locale(""), "{:L} {}", count, count == 1 ? "track" : "tracks");
}

std::string get_volume_text()
{
    const float volume = playback_control::get()->get_volume();
    return fmt::format("{:0.02f} dB", volume);
}

void set_part_sizes(unsigned p_parts)
{
    if (g_status) {
        RECT rect;
        GetClientRect(g_status, &rect);
        const auto scrollbar_height = GetSystemMetrics(SM_CXVSCROLL);
        rect.right -= scrollbar_height;

        if (rect.right < rect.left)
            rect.right = rect.left;

        int borders[3]{};
        SendMessage(g_status, SB_GETBORDERS, 0, reinterpret_cast<LPARAM>(&borders));
        const auto part_spacing = borders[2];
        const auto part_padding = 2 * (4_spx + part_spacing);

        pfc::list_t<int> m_parts;

        m_parts.add_item(-1); // dummy

        uint8_t track_length_pos{};
        uint8_t track_count_pos{};
        uint8_t playlist_lock_pos{};
        uint8_t volume_pos{};

        const auto playlist_api = playlist_manager::get();
        const auto active = playlist_api->get_active_playlist();

        const bool show_playlist_lock_part
            = main_window::config_get_status_show_lock() && playlist_api->playlist_lock_is_present(active);

        if (show_playlist_lock_part) {
            if (p_parts & t_part_lock) {
                state->playlist_lock_text = get_playlist_lock_text();
            }

            const auto part_size = calculate_playback_lock_size(state->playlist_lock_text.c_str()) + part_padding;
            playlist_lock_pos = gsl::narrow<uint8_t>(m_parts.add_item(part_size));
        }

        if (cfg_show_seltime) {
            if ((p_parts & t_part_length))
                state->track_length_text = get_selected_length_text();

            const auto part_size = calculate_selected_length_size(state->track_length_text.c_str()) + part_padding;
            track_length_pos = gsl::narrow<uint8_t>(m_parts.add_item(part_size));
        }

        if (cfg_show_selcount) {
            if ((p_parts & t_part_count))
                state->track_count_text = get_selected_count_text();

            const auto part_size = calculate_selected_count_size(state->track_count_text.c_str()) + part_padding;
            track_count_pos = gsl::narrow<uint8_t>(m_parts.add_item(part_size));
        }

        if (cfg_show_vol) {
            if ((p_parts & t_part_volume))
                state->volume_text = get_volume_text();

            const auto part_size = calculate_volume_size(state->volume_text.c_str()) + part_padding;
            volume_pos = gsl::narrow<uint8_t>(m_parts.add_item(part_size));
        }

        m_parts[0] = rect.right - rect.left;

        const auto count = m_parts.get_count();
        for (size_t n = 1; n < count; n++)
            m_parts[0] -= m_parts[n];

        if (count > 1) {
            for (size_t n = count - 2; n; n--) {
                for (size_t i = 0; i < n; i++)
                    m_parts[n] += m_parts[i];
            }
        }
        m_parts[count - 1] = -1;

        SendMessage(g_status, SB_SETPARTS, m_parts.get_count(), (LPARAM)m_parts.get_ptr());

        if (cfg_show_vol && (p_parts & t_part_volume))
            SendMessage(g_status, SB_SETTEXT, SBT_OWNERDRAW | volume_pos, WI_EnumValue(StatusBarPartID::Volume));

        if (cfg_show_seltime && (p_parts & t_part_length))
            SendMessage(
                g_status, SB_SETTEXT, SBT_OWNERDRAW | track_length_pos, WI_EnumValue(StatusBarPartID::TrackLength));

        if (cfg_show_selcount && (p_parts & t_part_count))
            SendMessage(
                g_status, SB_SETTEXT, SBT_OWNERDRAW | track_count_pos, WI_EnumValue(StatusBarPartID::TrackCount));

        if (show_playlist_lock_part && (p_parts & t_part_lock)) {
            SendMessage(
                g_status, SB_SETTEXT, SBT_OWNERDRAW | playlist_lock_pos, WI_EnumValue(StatusBarPartID::PlaylistLock));
        }
    }
}

void create_window()
{
    if (cfg_status && !g_status) {
        g_status = CreateWindowEx(0, STATUSCLASSNAME, nullptr, WS_CHILD | SBARS_SIZEGRIP, 0, 0, 0, 0,
            main_window.get_wnd(), (HMENU)ID_STATUS, core_api::get_my_instance(), nullptr);

        state = StatusBarState{};

        state->status_proc = (WNDPROC)SetWindowLongPtr(g_status, GWLP_WNDPROC, (LPARAM)(g_status_hook));

        SetWindowPos(g_status, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        on_status_font_change();

        state->dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>(
            [wnd = g_status] { RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE); });

        set_part_sizes(t_parts_all);

        regenerate_text();
    } else if (!cfg_status && g_status) {
        destroy_window();
    }
}

namespace {

RECT get_adjusted_draw_item_rect(const RECT rect, const unsigned index, const StatusBarPartID part_id)
{
    int borders[3]{};
    SendMessage(g_status, SB_GETBORDERS, 0, reinterpret_cast<LPARAM>(&borders));
    const auto part_spacing = borders[2];

    RECT adjusted_rect = rect;
    adjusted_rect.left += 4_spx;
    adjusted_rect.right -= 4_spx;

    if (index > 0)
        adjusted_rect.left -= part_spacing;

    return adjusted_rect;
}

std::string_view get_draw_item_text(const StatusBarPartID part_id)
{
    switch (part_id) {
    case StatusBarPartID::PlaybackInformation:
        return state->playback_information_text;
    case StatusBarPartID::MenuItemDescription:
        return state->menu_item_description;
    case StatusBarPartID::PlaylistLock:
        return state->playlist_lock_text;
    case StatusBarPartID::TrackLength:
        return state->track_length_text;
    case StatusBarPartID::TrackCount:
        return state->track_count_text;
    case StatusBarPartID::Volume:
        return state->volume_text;
    default:
        uBugCheck();
    }
}

void draw_item_content(const HDC dc, const StatusBarPartID part_id, const std::string_view text, const RECT rc)
{
    if (text.empty())
        return;

    const auto text_colour = dark::get_colour(dark::ColourID::StatusBarText, colours::is_dark_mode_active());

    if (part_id == StatusBarPartID::PlaybackInformation) {
        text_out_colours_tab(dc, text.data(), gsl::narrow<int>(text.size()), 0, 0, &rc, FALSE, text_colour, true, false,
            uih::ALIGN_LEFT);
        return;
    }

    const auto utf16_text = pfc::stringcvt::string_wide_from_utf8(text.data(), text.size());
    SetTextColor(dc, text_colour);
    SetBkMode(dc, TRANSPARENT);
    const int x = rc.left;
    const int y = rc.top + (RECT_CY(rc) - uih::get_dc_font_height(dc)) / 2;
    ExtTextOutW(dc, x, y, ETO_CLIPPED, &rc, utf16_text, gsl::narrow<UINT>(utf16_text.length()), nullptr);
}

} // namespace

std::optional<LRESULT> handle_draw_item(const LPDRAWITEMSTRUCT lpdis)
{
    const auto part_id = static_cast<StatusBarPartID>(lpdis->itemData);
    const RECT rc = get_adjusted_draw_item_rect(lpdis->rcItem, lpdis->itemID, part_id);

    if (rc.right <= rc.left)
        return TRUE;

    const std::string_view text = get_draw_item_text(part_id);

    draw_item_content(lpdis->hDC, part_id, text, rc);

    return TRUE;
}

} // namespace cui::status_bar
