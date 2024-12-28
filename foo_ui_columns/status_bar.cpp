#include "pch.h"
#include "status_bar.h"

#include "font_utils.h"
#include "main_window.h"
#include "metadb_helpers.h"

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
    wil::unique_hfont font;
    uih::direct_write::Context::Ptr direct_write_ctx;
    std::optional<uih::direct_write::TextFormat> direct_write_text_format;
    wil::unique_hbitmap lock_bitmap;
    wil::unique_hicon lock_icon;
    std::unique_ptr<colours::dark_mode_notifier> dark_mode_notifier;
};

std::optional<StatusBarState> state;

constexpr GUID font_client_status_guid = {0xb9d5ea18, 0x5827, 0x40be, {0xa8, 0x96, 0x30, 0x2a, 0x71, 0xbc, 0xaa, 0x9c}};

void on_status_font_change()
{
    if (!g_status)
        return;

    SetWindowFont(g_status, nullptr, FALSE);

    state->lock_icon.reset();
    state->lock_bitmap.reset();

    const auto font = fb2k::std_api_get<fonts::manager_v3>()->get_client_font(font_client_status_guid);
    const auto log_font = font->log_font();
    state->font.reset(CreateFontIndirect(&log_font));
    state->direct_write_text_format.reset();

    try {
        if (!state->direct_write_ctx)
            state->direct_write_ctx = uih::direct_write::Context::s_create();
    }
    CATCH_LOG()

    if (state->direct_write_ctx)
        state->direct_write_text_format = fonts::get_text_format(state->direct_write_ctx, font);

    SetWindowFont(g_status, state->font.get(), TRUE);

    set_part_sizes(t_parts_all);
    main_window.resize_child_windows();
}

class StatusBarFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return font_client_status_guid; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Status bar"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_labels; }

    void on_font_changed() const override { on_status_font_change(); }
};

StatusBarFontClient::factory<StatusBarFontClient> g_font_client_status;

} // namespace

LRESULT WINAPI g_status_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) noexcept
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
            FillRect(dc_mem, &rc, get_colour_brush(dark::ColourID::StatusBarBackground, true).get());
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
    if (volume_popup_window.get_wnd())
        volume_popup_window.destroy();

    if (g_status) {
        DestroyWindow(g_status);
        g_status = nullptr;
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
    return lock_name.get_ptr();
}

int calculate_volume_size(std::string_view text)
{
    if (!state->direct_write_text_format)
        return 0;

    return state->direct_write_text_format->measure_text_width(mmh::to_utf16(text));
}

int calculate_selected_length_size(std::string_view text)
{
    if (!state->direct_write_text_format)
        return 0;

    return std::max(state->direct_write_text_format->measure_text_width(mmh::to_utf16(text)),
        state->direct_write_text_format->measure_text_width(L"0d 00:00:00"));
}

int calculate_selected_count_size(std::string_view text)
{
    if (!state->direct_write_text_format)
        return 0;

    return std::max(state->direct_write_text_format->measure_text_width(mmh::to_utf16(text)),
        state->direct_write_text_format->measure_text_width(L"0,000 tracks"));
}

int calculate_playback_lock_size(std::string_view text)
{
    const auto icon_width = uih::get_font_height(state->font.get()) - 2_spx + 2_spx;

    return icon_width
        + (state->direct_write_text_format ? state->direct_write_text_format->measure_text_width(mmh::to_utf16(text))
                                           : 0);
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
    length = helpers::calculate_tracks_total_length(sels);

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

            const auto part_size = calculate_playback_lock_size(state->playlist_lock_text) + part_padding;
            playlist_lock_pos = gsl::narrow<uint8_t>(m_parts.add_item(part_size));
        }

        if (cfg_show_seltime) {
            if ((p_parts & t_part_length))
                state->track_length_text = get_selected_length_text();

            const auto part_size = calculate_selected_length_size(state->track_length_text) + part_padding;
            track_length_pos = gsl::narrow<uint8_t>(m_parts.add_item(part_size));
        }

        if (cfg_show_selcount) {
            if ((p_parts & t_part_count))
                state->track_count_text = get_selected_count_text();

            const auto part_size = calculate_selected_count_size(state->track_count_text) + part_padding;
            track_count_pos = gsl::narrow<uint8_t>(m_parts.add_item(part_size));
        }

        if (cfg_show_vol) {
            if ((p_parts & t_part_volume))
                state->volume_text = get_volume_text();

            const auto part_size = calculate_volume_size(state->volume_text) + part_padding;
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
            main_window.get_wnd(), reinterpret_cast<HMENU>(ID_STATUS), core_api::get_my_instance(), nullptr);

        if (!g_status)
            return;

        state = StatusBarState{};

        state->status_proc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(g_status, GWLP_WNDPROC, reinterpret_cast<LPARAM>(g_status_hook)));

        SetWindowPos(g_status, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        on_status_font_change();

        state->dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>([wnd = g_status] {
            state->lock_bitmap.reset();
            state->lock_icon.reset();
            RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
        });

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

void draw_item_content(
    const HWND wnd, const HDC dc, const StatusBarPartID part_id, const std::string_view text, const RECT rc)
{
    if (text.empty())
        return;

    const auto text_colour = get_colour(dark::ColourID::StatusBarText, colours::is_dark_mode_active());

    if (part_id == StatusBarPartID::PlaybackInformation) {
        if (state->direct_write_text_format)
            uih::direct_write::text_out_columns_and_colours(
                *state->direct_write_text_format, wnd, dc, text, 0, 0, rc, text_colour);
        return;
    }

    const auto font_height = uih::get_dc_font_height(dc);
    const auto icon_size = font_height - 2_spx;
    int x = rc.left;

    if (part_id == StatusBarPartID::PlaylistLock && icon_size > 0) {
        const auto icon_y = rc.top + (wil::rect_height(rc) - icon_size) / 2;

        if (icons::use_svg_icon(icon_size, icon_size)) {
            if (!state->lock_bitmap) {
                state->lock_bitmap
                    = render_svg(icons::built_in::padlock, icon_size, icon_size, svg_services::PixelFormat::PBGRA);
            }

            const wil::unique_hdc compatible_dc(CreateCompatibleDC(dc));
            auto _ = wil::SelectObject(compatible_dc.get(), state->lock_bitmap.get());
            constexpr BLENDFUNCTION blend_function{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
            GdiAlphaBlend(
                dc, x, icon_y, icon_size, icon_size, compatible_dc.get(), 0, 0, icon_size, icon_size, blend_function);
        } else {
            if (!state->lock_icon) {
                state->lock_icon = load_icon(icons::built_in::padlock, icon_size, icon_size);
            }

            DrawIconEx(dc, x, icon_y, state->lock_icon.get(), icon_size, icon_size, 0, nullptr, DI_NORMAL);
        }
        x += icon_size + 2_spx;
    }

    if (state->direct_write_text_format)
        uih::direct_write::text_out_columns_and_colours(*state->direct_write_text_format, wnd, dc, text, x - rc.left, 0,
            rc, text_colour, {.enable_colour_codes = false, .enable_tab_columns = false});
}

} // namespace

std::optional<LRESULT> handle_draw_item(const LPDRAWITEMSTRUCT lpdis)
{
    const auto part_id = static_cast<StatusBarPartID>(lpdis->itemData);
    const RECT rc = get_adjusted_draw_item_rect(lpdis->rcItem, lpdis->itemID, part_id);

    if (rc.right <= rc.left)
        return TRUE;

    const std::string_view text = get_draw_item_text(part_id);

    draw_item_content(lpdis->hwndItem, lpdis->hDC, part_id, text, rc);

    return TRUE;
}

} // namespace cui::status_bar
