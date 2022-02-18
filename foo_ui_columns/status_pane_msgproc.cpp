#include "stdafx.h"

#include "dark_mode.h"
#include "status_pane.h"

namespace cui::status_pane {

void g_split_string_by_crlf(const char* text, pfc::string_list_impl& p_out)
{
    const char* ptr = text;
    while (*ptr) {
        const char* start = ptr;
        t_size counter = 0;
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

        m_font.reset(fonts::helper(g_guid_font).get_font());
        m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"Window") : nullptr;
        m_dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>(
            [wnd] { RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE); });

        update_playlist_data();
        update_playback_status_text();
        static_api_ptr_t<playlist_manager>()->register_callback(this, flag_all);
        static_api_ptr_t<play_callback_manager>()->register_callback(
            this, flag_on_playback_all | flag_on_volume_change, false);
    } break;
    case WM_DESTROY:
        static_api_ptr_t<playlist_manager>()->unregister_callback(this);
        static_api_ptr_t<play_callback_manager>()->unregister_callback(this);

        m_volume_control.destroy();

        m_dark_mode_notifier.reset();
        m_font.reset();
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
        helpers::execute_main_menu_command(cfg_statusdbl);
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

        const auto font_height = uGetFontHeight(m_font.get());
        const auto line_height = font_height + uih::scale_dpi_value(3);

        RECT rc_client{};
        GetClientRect(wnd, &rc_client);

        RECT rc_volume{};
        GetRelativeRect(m_volume_control.get_wnd(), wnd, &rc_volume);

        render_background(dc.get(), rc_client);

        RECT rc_text{rc_client};
        rc_text.right = rc_volume.left - uih::scale_dpi_value(20);

        RECT rc_line_1 = rc_text;
        rc_line_1.top += uih::scale_dpi_value(4);
        rc_line_1.bottom = rc_line_1.top + line_height;

        RECT rc_line_2 = rc_text;
        rc_line_2.top = rc_line_1.bottom;
        rc_line_2.bottom = rc_line_2.top + line_height;

        const auto _ = wil::SelectObject(dc.get(), m_font.get());

        const char* placeholder = "999999999 items selected";
        const auto default_text_colour = dark::get_system_colour(COLOR_BTNTEXT, colours::is_dark_mode_active());
        int placeholder_len
            = uih::get_text_width(dc.get(), placeholder, strlen(placeholder)) + uih::scale_dpi_value(20);

        pfc::string8 items_text;
        items_text << m_item_count << " item";
        if (m_item_count != 1)
            items_text << "s";
        if (m_selection)
            items_text << " selected";

        text_out_colours_tab(dc.get(), items_text, -1, uih::scale_dpi_value(1), uih::scale_dpi_value(3), &rc_line_1,
            false, default_text_colour, false, false, uih::ALIGN_LEFT);
        if (m_item_count) {
            pfc::string_formatter formatter;
            text_out_colours_tab(dc.get(), formatter << "Length: " << m_length_text, -1, uih::scale_dpi_value(1),
                uih::scale_dpi_value(3), &rc_line_2, false, default_text_colour, false, false, uih::ALIGN_LEFT);
        }

        if (m_menu_active) {
            {
                RECT rc_item = rc_line_1;
                text_out_colours_tab(dc.get(), m_menu_text, -1, uih::scale_dpi_value(1) + placeholder_len,
                    uih::scale_dpi_value(3), &rc_item, false, default_text_colour, false, false, uih::ALIGN_LEFT,
                    nullptr, true, true, nullptr);
            }
        } else {
            placeholder = "Playing:  ";
            t_size placeholder2_len = uih::get_text_width(dc.get(), placeholder, strlen(placeholder));

            t_size now_playing_x_end = uih::scale_dpi_value(4) + placeholder_len + placeholder2_len;
            {
                RECT rc_item = rc_line_1;
                // rc_item.right = 4 + placeholder_len + placeholder2_len;
                text_out_colours_tab(dc.get(), m_track_label, -1, uih::scale_dpi_value(4) + placeholder_len, 0,
                    &rc_item, false, default_text_colour, false, false, uih::ALIGN_LEFT, nullptr, true, true, nullptr);
            }
            if (playing1.get_length()) {
                pfc::string_list_impl playingstrings;
                g_split_string_by_crlf(playing1.get_ptr(), playingstrings);
                t_size lines = playingstrings.get_count();
                if (lines)
                    text_out_colours_tab(dc.get(), playingstrings[0], pfc_infinite, now_playing_x_end, 0, &rc_line_1,
                        false, default_text_colour, true, false, uih::ALIGN_LEFT);
                if (lines > 1)
                    text_out_colours_tab(dc.get(), playingstrings[1], pfc_infinite, now_playing_x_end, 0, &rc_line_2,
                        false, default_text_colour, true, false, uih::ALIGN_LEFT);
            }
        }

        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

} // namespace cui::status_pane
