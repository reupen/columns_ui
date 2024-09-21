#include "pch.h"

#include "dark_mode.h"
#include "status_pane.h"

namespace cui::status_pane {

void g_split_string_by_crlf(const char* text, pfc::string_list_impl& p_out)
{
    const char* ptr = text;
    while (*ptr) {
        const char* start = ptr;

        while (*ptr && *ptr != '\r' && *ptr != '\n') {
            ptr++;
        }

        p_out.add_item(pfc::string8(start, ptr - start));

        if (*ptr == '\r')
            ptr++;
        if (*ptr == '\n')
            ptr++;
    }
}

LRESULT StatusPane::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        ShowWindow(m_volume_control.create(wnd), SW_SHOWNORMAL);

        m_direct_write_context = uih::direct_write::Context::s_create();
        recreate_font();
        m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"Window") : nullptr;
        m_dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>(
            [wnd] { RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE); });

        update_playlist_data();
        update_playback_status_text();
        playlist_manager::get()->register_callback(this, flag_all);
        play_callback_manager::get()->register_callback(this, flag_on_playback_all | flag_on_volume_change, false);
    } break;
    case WM_DESTROY:
        playlist_manager::get()->unregister_callback(this);
        play_callback_manager::get()->unregister_callback(this);

        m_volume_control.destroy();

        m_dark_mode_notifier.reset();
        m_direct_write_context.reset();
        m_text_format.reset();

        if (m_theme) {
            CloseThemeData(m_theme);
            m_theme = nullptr;
        }
        break;
    case WM_THEMECHANGED: {
        if (m_theme)
            CloseThemeData(m_theme);
        m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"Window") : nullptr;
    } break;
    case WM_ERASEBKGND:
        return FALSE;
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            const auto vol_cy = GetSystemMetrics(SM_CYSMICON) * 3 / 2;
            const auto vol_cx = vol_cy * 4;
            const auto vol_x = lpwp->cx - vol_cx - uih::scale_dpi_value(4);
            const auto vol_y = uih::scale_dpi_value(2) + (lpwp->cy - vol_cy) / 2;
            SetWindowPos(m_volume_control.get_wnd(), nullptr, vol_x, vol_y, vol_cx, vol_cy, SWP_NOZORDER);
            // on_size(lpwp->cx, lpwp->cy);
        }
    } break;
    case WM_LBUTTONDBLCLK:
        helpers::execute_main_menu_command(double_click_action);
        return 0;
    case WM_PRINTCLIENT: {
        if (lp & PRF_ERASEBKGND) {
            auto dc = (HDC)wp;

            RECT rc;
            GetClientRect(wnd, &rc);
            render_background(dc, rc);
        }
    } break;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        const auto paint_dc = wil::BeginPaint(wnd, &ps);
        uih::BufferedDC dc(paint_dc.get(), ps.rcPaint);

        RECT rc_client{};
        GetClientRect(wnd, &rc_client);

        RECT rc_volume{};
        GetRelativeRect(m_volume_control.get_wnd(), wnd, &rc_volume);

        render_background(dc.get(), rc_client);

        if (!m_text_format)
            break;

        const auto line_height = m_font_height + 3_spx;

        RECT rc_text{rc_client};
        rc_text.right = rc_volume.left - 20_spx;

        RECT rc_line_1 = rc_text;
        rc_line_1.top += 4_spx;
        rc_line_1.bottom = rc_line_1.top + line_height;

        RECT rc_line_2 = rc_text;
        rc_line_2.top = rc_line_1.bottom;
        rc_line_2.bottom = rc_line_2.top + line_height;

        constexpr auto selected_items_placeholder = L"999999999 items selected"sv;
        const auto default_text_colour = get_colour(dark::ColourID::StatusPaneText, colours::is_dark_mode_active());
        const auto selected_items_width = m_text_format->measure_text_width(selected_items_placeholder) + 20_spx;

        pfc::string8 items_text;
        items_text << m_item_count << " item";
        if (m_item_count != 1)
            items_text << "s";
        if (m_selection)
            items_text << " selected";

        constexpr auto simple_text_opts
            = uih::direct_write::TextOutOptions{.enable_colour_codes = false, .enable_tab_columns = false};

        uih::direct_write::text_out_columns_and_colours(*m_text_format, wnd, dc.get(), mmh::to_string_view(items_text),
            1_spx, 3_spx, rc_line_1, default_text_colour, simple_text_opts);

        if (m_item_count) {
            uih::direct_write::text_out_columns_and_colours(*m_text_format, wnd, dc.get(),
                fmt::format("Length: {}", m_length_text.c_str()), 1_spx, 3_spx, rc_line_2, default_text_colour,
                simple_text_opts);
        }

        if (m_menu_active) {
            uih::direct_write::text_out_columns_and_colours(*m_text_format, wnd, dc.get(),
                mmh::to_string_view(m_menu_text), 1_spx + selected_items_width, 3_spx, rc_line_1, default_text_colour,
                simple_text_opts);
        } else {
            constexpr auto playback_status_placeholder = L"Playing:  "sv;
            const auto playback_status_width = m_text_format->measure_text_width(playback_status_placeholder);

            uih::direct_write::text_out_columns_and_colours(*m_text_format, wnd, dc.get(),
                mmh::to_string_view(m_track_label), 4_spx + selected_items_width, 0, rc_line_1, default_text_colour,
                simple_text_opts);

            if (playing1.get_length()) {
                const auto now_playing_x_end = 4_spx + selected_items_width + playback_status_width;

                pfc::string_list_impl formatted_title_lines;
                g_split_string_by_crlf(playing1.get_ptr(), formatted_title_lines);

                size_t lines = formatted_title_lines.get_count();

                if (lines > 0)
                    uih::direct_write::text_out_columns_and_colours(*m_text_format, wnd, dc.get(),
                        std::string_view{formatted_title_lines[0]}, now_playing_x_end, 0, rc_line_1,
                        default_text_colour);

                if (lines > 1)
                    uih::direct_write::text_out_columns_and_colours(*m_text_format, wnd, dc.get(),
                        std::string_view{formatted_title_lines[1]}, now_playing_x_end, 0, rc_line_2,
                        default_text_colour);
            }
        }

        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

} // namespace cui::status_pane
