#include "stdafx.h"
#include "font_notify.h"
#include "status_bar.h"

pfc::string8 status_bar::menudesc;

bool menu = false;

void status_set_menu(bool on)
{
    if (!on)
        status_bar::menudesc.reset();
    bool old_menu = menu;
    menu = on;
    // CHECK !!
    if (!(!old_menu && !on))
        status_update_main(on);
}

void status_update_main(bool is_caller_menu_desc)
{
    if (g_status) {
        if (menu && is_caller_menu_desc)
            SendMessage(g_status, SB_SETTEXT, SBT_OWNERDRAW, (LPARAM)(&status_bar::menudesc));
        else if (!menu)
            SendMessage(g_status, SB_SETTEXT, SBT_OWNERDRAW, (LPARAM)(&statusbartext));
    }
}

void update_status()
{
    metadb_handle_ptr track;
    static_api_ptr_t<play_control> play_api;
    play_api->get_now_playing(track);
    if (track.is_valid()) {
        service_ptr_t<titleformat_object> to_status;
        static_api_ptr_t<titleformat_compiler>()->compile_safe(to_status, main_window::config_status_bar_script.get());
        play_api->playback_format_title_ex(
            track, nullptr, statusbartext, to_status, nullptr, play_control::display_level_all);

        track.release();
    } else {
        statusbartext = core_version_info::g_get_version_string();
    }
    status_update_main(false);
}

WNDPROC status_proc = nullptr;

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

        CallWindowProc(status_proc, wnd, WM_ERASEBKGND, (WPARAM)dc_mem, NULL);
        SendMessage(wnd, WM_PRINTCLIENT, (WPARAM)dc_mem, PRF_CHECKVISIBLE | PRF_CLIENT | PRF_ERASEBKGND);

        BitBlt(dc, rc.left, rc.top, rc.right, rc.bottom, dc_mem, 0, 0, SRCCOPY);

        SelectObject(dc_mem, bm_old);
        DeleteObject(bm_mem);
        DeleteDC(dc_mem);

        EndPaint(wnd, &ps);
    }
        return 0;
    }

    return CallWindowProc(status_proc, wnd, msg, wp, lp);
}

namespace status_bar {
bool b_lock = false;
unsigned u_length_pos = 0;
unsigned u_lock_pos = 0;
unsigned u_vol_pos = 0;
HICON icon_lock = nullptr;
HTHEME thm_status = nullptr;
HICON get_icon()
{
    if (!icon_lock) {
        icon_lock = (HICON)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDI_LOCK), IMAGE_ICON, 0, 0, 0);
    }
    return icon_lock;
}
void destroy_icon()
{
    if (icon_lock) {
        DeleteObject(icon_lock);
        icon_lock = nullptr;
    }
}
void destroy_theme_handle()
{
    {
        if (status_bar::thm_status) {
            CloseThemeData(status_bar::thm_status);
            status_bar::thm_status = nullptr;
        }
    }
}
void create_theme_handle()
{
    if (!status_bar::thm_status) {
        status_bar::thm_status = IsThemeActive() && IsAppThemed() ? OpenThemeData(g_status, L"Status") : nullptr;
    }
}
void destroy_status_window()
{
    destroy_theme_handle();
    if (g_status) {
        DestroyWindow(g_status);
        g_status = nullptr;
    }
}
}; // namespace status_bar

namespace status_bar {
volume_popup_t volume_popup_window;
unsigned u_volume_size;
unsigned u_selected_length_size;
unsigned u_playback_lock_size;

bool get_text(unsigned index, pfc::string_base& p_out)
{
    unsigned ret = SendMessage(g_status, SB_GETTEXTLENGTH, index, 0);
    if (HIWORD(ret) != SBT_OWNERDRAW) {
        unsigned short len = LOWORD(ret);
        pfc::array_t<TCHAR> buffer;
        buffer.set_size(len + 1);
        ret = SendMessage(g_status, SB_GETTEXT, index, (LPARAM)buffer.get_ptr());
        buffer[len] = NULL;
        p_out.set_string(pfc::stringcvt::string_utf8_from_os(buffer.get_ptr(), len));
        return true;
    }
    return false;
}

void calculate_volume_size(const char* p_text)
{
    u_volume_size = win32_helpers::status_bar_get_text_width(g_status, status_bar::thm_status, p_text) + 12;
}

void calculate_selected_length_size(const char* p_text)
{
    u_selected_length_size
        = max(win32_helpers::status_bar_get_text_width(g_status, status_bar::thm_status, p_text),
              win32_helpers::status_bar_get_text_width(g_status, status_bar::thm_status, "0d 00:00:00"))
        + 12;
}

void calculate_playback_lock_size(const char* p_text)
{
    u_playback_lock_size
        = win32_helpers::status_bar_get_text_width(g_status, status_bar::thm_status, p_text) + 14 + 16 + 4;
}

void get_selected_length_text(pfc::string_base& p_out, unsigned dp = 0)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> sels;
    double length = 0;

    static_api_ptr_t<playlist_manager> playlist_api;
    static_api_ptr_t<metadb> metadb_api;

    unsigned count = playlist_api->activeplaylist_get_selection_count(pfc_infinite);

    sels.prealloc(count);

    playlist_api->activeplaylist_get_selected_items(sels);
    length = sels.calc_total_duration();

    p_out = pfc::format_time_ex(length, dp);
}

void get_volume_text(pfc::string_base& p_out)
{
    static_api_ptr_t<play_control> play_api;
    float volume = play_api->get_volume();

    p_out.add_string(pfc::format_float(volume, 0, 2));
    p_out.add_string(" dB");
}

void set_part_sizes(unsigned p_parts)
{
    if (g_status) {
        bool b_old_lock = status_bar::b_lock;
        unsigned u_old_lock_pos = status_bar::u_lock_pos;

        RECT rc2;
        GetClientRect(cui::main_window.get_wnd(), &rc2);
        int scrollbar_height = GetSystemMetrics(SM_CXVSCROLL);
        if (!IsZoomed(cui::main_window.get_wnd()))
            rc2.right -= scrollbar_height;

        int blah[3];
        SendMessage(g_status, SB_GETBORDERS, 0, (LPARAM)&blah);

        if (rc2.right < rc2.left)
            rc2.right = rc2.left;

        pfc::list_t<int> m_parts;

        m_parts.add_item(-1); // dummy

        pfc::string8 text_lock, text_volume, text_length;
        static_api_ptr_t<playlist_manager> playlist_api;
        unsigned active = playlist_api->get_active_playlist();

        status_bar::b_lock
            = main_window::config_get_status_show_lock() && playlist_api->playlist_lock_is_present(active);

        if (status_bar::b_lock) {
            if (b_old_lock && !(p_parts & t_part_lock))
                get_text(u_old_lock_pos, text_lock);
            else
                playlist_api->playlist_lock_query_name(active, text_lock);
            calculate_playback_lock_size(text_lock);
            status_bar::u_lock_pos = m_parts.add_item(u_playback_lock_size);
        }

        if (cfg_show_seltime) {
            if (!(p_parts & t_part_length))
                get_text(u_length_pos, text_length);
            else
                get_selected_length_text(text_length); // blah

            calculate_selected_length_size(text_length);

            status_bar::u_length_pos = m_parts.add_item(u_selected_length_size);
        }

        if (cfg_show_vol) {
            if (!(p_parts & t_part_volume))
                get_text(u_vol_pos, text_volume);
            else
                get_volume_text(text_volume); // blah
            calculate_volume_size(text_volume);
            status_bar::u_vol_pos = m_parts.add_item(u_volume_size);
        }

        m_parts[0] = rc2.right - rc2.left;

        unsigned n, count = m_parts.get_count();
        for (n = 1; n < count; n++)
            m_parts[0] -= m_parts[n];

        if (count > 1) {
            for (n = count - 2; n; n--) {
                for (unsigned i = 0; i < n; i++)
                    m_parts[n] += m_parts[i];
            }
        }
        m_parts[count - 1] = -1;

        if (b_old_lock && (!status_bar::b_lock || status_bar::u_lock_pos != u_old_lock_pos))
            SendMessage(g_status, SB_SETICON, u_old_lock_pos, 0);

        SendMessage(g_status, SB_SETPARTS, m_parts.get_count(), (LPARAM)m_parts.get_ptr());

        if (cfg_show_vol && (p_parts & t_part_volume))
            uStatus_SetText(g_status, u_vol_pos, text_volume);
        if (cfg_show_seltime && (p_parts & t_part_length))
            uStatus_SetText(g_status, u_length_pos, text_length);

        if (status_bar::b_lock && (p_parts & t_part_lock)) {
            SendMessage(g_status, SB_SETICON, status_bar::u_lock_pos, (LPARAM)status_bar::get_icon());
            uStatus_SetText(g_status, status_bar::u_lock_pos, text_lock);
        }
    }
}

}; // namespace status_bar

void create_status()
{
    if (cfg_status && !g_status) {
        g_status = CreateWindowEx(0, STATUSCLASSNAME, nullptr, WS_CHILD | SBARS_SIZEGRIP, 0, 0, 0, 0,
            cui::main_window.get_wnd(), (HMENU)ID_STATUS, core_api::get_my_instance(), nullptr);

        status_proc = (WNDPROC)SetWindowLongPtr(g_status, GWLP_WNDPROC, (LPARAM)(g_status_hook));

        status_bar::create_theme_handle();

        SetWindowPos(g_status, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        on_status_font_change();

        status_bar::set_part_sizes(status_bar::t_parts_all);

        status_update_main(false);
    } else if (!cfg_status && g_status) {
        status_bar::destroy_status_window();
    }
}
