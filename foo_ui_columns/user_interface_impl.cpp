#include "pch.h"
#include "status_pane.h"
#include "status_bar.h"
#include "notification_area.h"
#include "main_window.h"

extern bool g_icon_created;

class UserInterfaceImpl : public user_interface_v2 {
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
        if (cui::main_window.get_wnd()) {
            cfg_go_to_tray = false;
            if (g_icon_created && !cfg_show_systray)
                destroy_systray_icon();

            if (!is_visible()) {
                ShowWindow(cui::main_window.get_wnd(), SW_RESTORE);
                if ((GetWindowLong(cui::main_window.get_wnd(), GWL_EXSTYLE) & WS_EX_LAYERED))
                    RedrawWindow(cui::main_window.get_wnd(), nullptr, nullptr,
                        RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
            }
            SetForegroundWindow(cui::main_window.get_wnd());
        }
    }
    void hide() override
    {
        if (cui::main_window.get_wnd()) {
            ShowWindow(cui::main_window.get_wnd(), SW_MINIMIZE);
        }
    }
    bool is_visible() override
    {
        bool rv = false;
        if (cui::main_window.get_wnd()) {
            rv = IsWindowVisible(cui::main_window.get_wnd()) && !IsIconic(cui::main_window.get_wnd());
        }
        return rv;
    }
    void override_statusbar_text(const char* p_text) override
    {
        cui::status_bar::set_menu_item_description(p_text);
        cui::status_pane::g_status_pane.enter_menu_mode(p_text);
    }
    void revert_statusbar_text() override
    {
        cui::status_bar::clear_menu_item_description();
        cui::status_pane::g_status_pane.exit_menu_mode();
    }

    bool query_capability(const GUID& cap) override
    {
        cui::main_window.on_query_capability();
        if (cap == cap_suppress_core_shellhook)
            return false;
        if (cap == cap_suppress_core_uvc)
            return false;
        // The SDK documentation does not say what to do when a GUID for an unknown capability is encountered.
        // We return false (which is apparently what the Default UI does).
        return false;
    }
};

static user_interface_factory<UserInterfaceImpl> user_interface_impl;
