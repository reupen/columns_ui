#include "stdafx.h"
#include "status_pane.h"
#include "layout.h"
#include "font_notify.h"
#include "status_bar.h"
#include "notification_area.h"

class UserInterfaceImpl : public user_interface {
public:
    const char* get_name() override { return "Columns UI"; }

    HWND init(HookProc_t hook) override { return cui::main_window.initialise(hook); }

    GUID get_guid() override { return {0xf12d0a24, 0xa8a4, 0x4618, {0x96, 0x59, 0x6f, 0x66, 0xde, 0x6, 0x75, 0x24}}; }

    void show_now_playing() override
    {
        auto play_api = play_control::get();
        update_systray(true, play_api->is_paused() ? 2 : 0, true);
    }
    void shutdown() override { cui::main_window.shutdown(); }

    void activate() override
    {
        if (g_main_window) {
            cfg_go_to_tray = false;
            if (g_icon_created && !cfg_show_systray)
                destroy_systray_icon();

            if (!is_visible()) {
                ShowWindow(g_main_window, SW_RESTORE);
                if ((GetWindowLong(g_main_window, GWL_EXSTYLE) & WS_EX_LAYERED))
                    RedrawWindow(g_main_window, nullptr, nullptr,
                        RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
            }
            SetForegroundWindow(g_main_window);
        }
    }
    void hide() override
    {
        if (g_main_window) {
            ShowWindow(g_main_window, SW_MINIMIZE);
        }
    }
    bool is_visible() override
    {
        bool rv = false;
        if (g_main_window) {
            rv = IsWindowVisible(g_main_window) && !IsIconic(g_main_window);
        }
        return rv;
    }
    void override_statusbar_text(const char* p_text) override
    {
        status_bar::menudesc = p_text;
        status_set_menu(true);
        g_status_pane.enter_menu_mode(p_text);
    };
    void revert_statusbar_text() override
    {
        status_set_menu(false);
        g_status_pane.exit_menu_mode();
    }
};

static user_interface_factory<UserInterfaceImpl> user_interface_impl;
