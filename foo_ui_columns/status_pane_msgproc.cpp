#include "stdafx.h"

#include "status_pane.h"

LRESULT status_pane::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        {
            ShowWindow(m_volume_control.create(wnd), SW_SHOWNORMAL);

            m_font = cui::fonts::helper(g_guid_font).get_font();
            m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"Window") : nullptr;

            update_playlist_data();
            static_api_ptr_t<playback_control> play_api;
            set_track_label(play_api->is_playing(), play_api->is_paused());
            static_api_ptr_t<playlist_manager>()->register_callback(this, playlist_callback_single::flag_all);
            static_api_ptr_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_all|play_callback::flag_on_volume_change, false);
        }
        break;
    case WM_DESTROY:
        static_api_ptr_t<playlist_manager>()->unregister_callback(this);
        static_api_ptr_t<play_callback_manager>()->unregister_callback(this);

        m_volume_control.destroy();

        m_font.release();
        if (m_theme)
        {
            CloseThemeData(m_theme);
            m_theme = nullptr;
        }
        break;
    case WM_THEMECHANGED:
        {
            if (m_theme) CloseThemeData(m_theme);
            m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"Window") : nullptr;
        }
        break;
    case WM_ERASEBKGND:
        return FALSE;
    case WM_WINDOWPOSCHANGED:
        {
            LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
            if (!(lpwp->flags & SWP_NOSIZE))
            {
                t_size vol_cy = GetSystemMetrics(SM_CYSMICON)*3/2, vol_cx = vol_cy * 4;
                SetWindowPos(m_volume_control.get_wnd(), nullptr, lpwp->cx - vol_cx - 4, 2+(lpwp->cy-vol_cy)/2, vol_cx, vol_cy, SWP_NOZORDER);
                //on_size(lpwp->cx, lpwp->cy);
            }
        }                
        break;
    case WM_LBUTTONDBLCLK:
        mainmenu_commands::g_execute(cfg_statusdbl.get_value().m_command);
        return 0;
    case WM_PRINTCLIENT:
        {
            if (lp & PRF_ERASEBKGND)
            {
                HDC dc = (HDC)wp;


                RECT rc;
                GetClientRect(wnd, &rc);
                render_background(dc, rc);
            }
        }
        break;
    case WM_PAINT:
        {
            ui_helpers::PaintScope ps(wnd);
            ui_helpers::MemoryDC dc(ps);

            RECT rc, rc_volume;
            GetClientRect(wnd, &rc);
            GetRelativeRect(m_volume_control.get_wnd(), wnd, &rc_volume);

            render_background(dc, rc);

            rc.right = rc_volume.left - 20;

            RECT rc_top = rc, rc_bottom= rc;
            rc_top.top += 4;
            rc_top.bottom = rc_top.top + (RECT_CY(rc)-6) / 2;
            rc_bottom.top = rc_top.bottom;
            rc_bottom.bottom -= 2;

            HFONT fnt_old = SelectFont(dc, m_font);

            const char * placeholder = "999999999 items selected";
            t_size placeholder_len = ui_helpers::get_text_width(dc, placeholder, strlen(placeholder)) + 20;

            pfc::string8 items_text; items_text<< m_item_count << " item";
            if (m_item_count != 1) items_text << "s";
            if (m_selection) items_text << " selected";

            ui_helpers::text_out_colours_tab(dc, items_text, -1, 4, 2, &rc_top, false, 0, false, false, false, ui_helpers::ALIGN_LEFT);
            if (m_item_count) {
                pfc::string_formatter formatter;
                ui_helpers::text_out_colours_tab(dc, formatter << "Length: " << m_length_text, -1, 4, 2, &rc_bottom, false, 0, false, false, false, ui_helpers::ALIGN_LEFT);
            }

            if (m_menu_active)
            {
                {
                    RECT rc_item = rc_top;
                    ui_helpers::text_out_colours_tab(dc, m_menu_text, -1, 4 + placeholder_len, 0, &rc_item, false, 0, false, false, false, ui_helpers::ALIGN_LEFT, nullptr, true, true, nullptr);
                }
            }
            else
            {
                placeholder = "Playing:  ";
                t_size placeholder2_len = ui_helpers::get_text_width(dc, placeholder, strlen(placeholder));

                t_size now_playing_x_end = 4 + placeholder_len + placeholder2_len;
                {
                    RECT rc_item = rc_top;
                    //rc_item.right = 4 + placeholder_len + placeholder2_len;
                    pfc::string_formatter formatter;
                    ui_helpers::text_out_colours_tab(dc, formatter << m_track_label << "  ", -1, 4 + placeholder_len, 0, &rc_item, false, 0, false, false, false, ui_helpers::ALIGN_LEFT, nullptr, true, true, nullptr);
                }
                if (playing1.get_length())
                {
                    pfc::string_list_impl playingstrings;
                    g_split_string_by_crlf(playing1.get_ptr(), playingstrings);
                    t_size lines = playingstrings.get_count();
                    if (lines)
                        ui_helpers::text_out_colours_tab(dc, playingstrings[0], pfc_infinite, now_playing_x_end, 0, &rc_top, false, 0, true, true, false, ui_helpers::ALIGN_LEFT);
                    if (lines > 1)
                        ui_helpers::text_out_colours_tab(dc, playingstrings[1], pfc_infinite, now_playing_x_end, 0, &rc_bottom, false, 0, true, true, false, ui_helpers::ALIGN_LEFT);
                }
            }

            SelectFont(dc, fnt_old);

            return 0;
        }
    };
    return DefWindowProc(wnd, msg, wp, lp);
}

