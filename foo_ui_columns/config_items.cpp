#include "stdafx.h"

#include "main_window.h"

namespace main_window
{
    const char* default_status_pane_script =
        "// This is the default script for the content of the main status bar pane during "
        "playback.\r\n\r\n"
        "$if(%is_status_pane%,%artist% - %title%$crlf(),$if(%ispaused%,Paused,Playing) | )"
        "%codec% | %bitrate% kbps | %samplerate% Hz | $caps(%channels%) | %playback_time%[ / %length%]";

    const char* default_notification_icon_script =
        "//This is the default script for the content of the notification area icon tooltip "
        "during playback.\r\n\r\n"
        "[%title%]$crlf()[%artist%][$crlf()%album%]";

    const char* default_main_window_title_script =
        "//This is the default script for the title of the main window during playback.\r\n\r\n"
        "[%title% - ]foobar2000";

    void on_status_bar_script_change(const char*)
    {
        if (g_main_window)
            SendMessage(g_main_window, MSG_UPDATE_STATUS, 0, 0);
    }

    void on_main_window_title_script_change(const char*)
    {
        if (g_main_window)
            SendMessage(g_main_window, MSG_UPDATE_TITLE, 0, 0);
    }

    fbh::ConfigString config_status_bar_script(
        GUID{0xb5ca645b, 0xa5e0, 0x4c70,{0xa5, 0x98, 0xcd, 0x62, 0x5c, 0xf3, 0xcc, 0x37}},
        default_status_pane_script,
        &on_status_bar_script_change
    );

    fbh::ConfigString config_notification_icon_script(
        GUID{0x85d128cf, 0x8b01, 0x4ae9,{0xb8, 0x1c, 0x6b, 0xc4, 0xbe, 0x67, 0x59, 0x9f}},
        default_notification_icon_script
    );

    fbh::ConfigString config_main_window_title_script(
        GUID{0x28b799fb, 0xbc22, 0x4e1c,{0xb9, 0x99, 0xf1, 0xe6, 0xb1, 0xf2, 0x60, 0x40}},
        default_main_window_title_script,
        &on_main_window_title_script_change
    );

    // {2B6EAF5C-970A-4432-B809-12E8CEF6DCDE}
    static const GUID guid_inline_metafield_edit_mode =
    { 0x2b6eaf5c, 0x970a, 0x4432, { 0xb8, 0x9, 0x12, 0xe8, 0xce, 0xf6, 0xdc, 0xde } };

    cfg_int_t<t_uint32> cfg_inline_metafield_edit_mode(guid_inline_metafield_edit_mode, config_get_inline_metafield_edit_mode_default_value());

    // {737954D1-1814-4d94-852D-A5CB8D7025ED}
    static const GUID guid_activate_target_playlist_on_dropped_items =
    { 0x737954d1, 0x1814, 0x4d94, { 0x85, 0x2d, 0xa5, 0xcb, 0x8d, 0x70, 0x25, 0xed } };

    cfg_bool
        cfg_firstrun(GUID{0xf7c0139, 0x8698, 0x42bd, 0xb2, 0xe0, 0x3f, 0x59, 0x94, 0xaa, 0xde, 0x43}, true),
        cfg_activate_target_playlist_on_dropped_items(guid_activate_target_playlist_on_dropped_items, config_get_activate_target_playlist_on_dropped_items_default_value());

    cfg_int
        cfg_transparency_enabled(GUID{0xd5ab3806, 0x8670, 0xf09d, 0x47, 0xdd, 0x1f, 0x45, 0x05, 0x8d, 0x83, 0x8f}, config_get_transparency_enabled_default_value()),
        cfg_transparency_level(GUID{0x7d50b4ac, 0xfcd4, 0x0a98, 0xd3, 0x64, 0x58, 0xc8, 0x2e, 0x56, 0x19, 0xd7}, config_get_transparency_level_default_value()),
        cfg_status_show_lock(GUID{0x0ec529ab, 0xa4cb, 0x0fa9, 0xa1, 0xbc, 0x7d, 0x24, 0x56, 0xdc, 0xb5, 0x40}, config_get_status_show_lock_default_value());

    void on_transparency_enabled_change()
    {
        if (g_main_window)
        {
            LONG current_style = GetWindowLong(g_main_window, GWL_EXSTYLE);
            if (cfg_transparency_enabled/* && IsWindowVisible(g_main_window)*/)
            {
                if (!(current_style & WS_EX_LAYERED))
                {
                    SetWindowLong(g_main_window,
                        GWL_EXSTYLE,
                        current_style | WS_EX_LAYERED);
                }
                on_transparency_level_change();
            }
            else if (current_style & WS_EX_LAYERED)
            {
                SetWindowLong(g_main_window,
                    GWL_EXSTYLE,
                    current_style ^ WS_EX_LAYERED);
                RedrawWindow(g_main_window,
                    nullptr,
                    nullptr,
                    RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
            }
        }
    }
    void on_transparency_level_change()
    {
        if (g_main_window && cfg_transparency_enabled)
        {
            SetLayeredWindowAttributes(g_main_window, 0, cfg_transparency_level, LWA_ALPHA);
        }
    }
    void config_set_transparency_enabled(bool b_val)
    {
        cfg_transparency_enabled = b_val;
        on_transparency_enabled_change();
    }
    bool config_get_transparency_enabled()
    {
        return cfg_transparency_enabled != 0;
    }
    bool config_get_transparency_enabled_default_value()
    {
        return false;
    }
    bool config_get_is_first_run()
    {
        return cfg_firstrun;
    }
    void config_set_is_first_run()
    {
        cfg_firstrun = false;
    }
    void config_reset_transparency_enabled()
    {
        config_set_transparency_enabled(config_get_transparency_enabled_default_value());
    }
    void config_set_transparency_level(unsigned char b_val)
    {
        cfg_transparency_level = b_val;
        on_transparency_level_change();
    }
    unsigned char config_get_transparency_level()
    {
        return (unsigned char)cfg_transparency_level;
    }
    unsigned char config_get_transparency_level_default_value()
    {
        return 255;
    }
    void config_reset_transparency_level()
    {
        config_set_transparency_level(config_get_transparency_level_default_value());
    }
    void config_set_status_show_lock(bool b_val)
    {
        cfg_status_show_lock = b_val;
        status_bar::set_part_sizes(status_bar::t_part_lock | status_bar::t_part_length | status_bar::t_part_volume);
    }
    bool config_get_status_show_lock()
    {
        return cfg_status_show_lock != 0;
    }
    bool config_get_status_show_lock_default_value()
    {
        return true;
    }
    void config_reset_status_show_lock()
    {
        config_set_status_show_lock(config_get_status_show_lock_default_value());
    }
    bool config_get_activate_target_playlist_on_dropped_items(){ return cfg_activate_target_playlist_on_dropped_items; }
    bool config_get_activate_target_playlist_on_dropped_items_default_value(){ return true; }
    void config_set_activate_target_playlist_on_dropped_items(bool b_val){ cfg_activate_target_playlist_on_dropped_items = b_val; }
    void config_reset_activate_target_playlist_on_dropped_items(){ config_set_activate_target_playlist_on_dropped_items(config_get_activate_target_playlist_on_dropped_items_default_value()); }

    void config_set_inline_metafield_edit_mode(t_uint32 value)
    {
        cfg_inline_metafield_edit_mode = value;
    }
    t_uint32 config_get_inline_metafield_edit_mode()
    {
        return cfg_inline_metafield_edit_mode;
    }
    void config_reset_inline_metafield_edit_mode()
    {
        config_set_inline_metafield_edit_mode(config_get_inline_metafield_edit_mode_default_value());
    }
    t_uint32 config_get_inline_metafield_edit_mode_default_value(){ return mode_columns; };
};

