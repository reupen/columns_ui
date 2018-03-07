#include "stdafx.h"
#include "status_pane.h"
#include "layout.h"
#include "font_notify.h"
#include "status_bar.h"
#include "notification_area.h"

const TCHAR* main_window_class_name = _T("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}");

const wchar_t* unsupported_os_message = L"Sorry, your operating system version is not supported by this version "
"of Columns UI. Please upgrade to Windows 7 Service Pack 1 or newer and try again.\n\n"
"Otherwise, uninstall the Columns UI component to return to the Default User Interface.";

class UserInterfaceImpl : public user_interface {
public:
    const char* get_name() override { return "Columns UI"; }

    HWND init(HookProc_t hook) override
    {
        if (!IsWindows7SP1OrGreater()) {
                MessageBox(nullptr, unsupported_os_message, L"Columns UI - Unsupported operating system",
                    MB_OK | MB_ICONEXCLAMATION);
            return nullptr;
        }

        if (main_window::config_get_is_first_run()) {
            if (!cfg_layout.get_presets().get_count())
                cfg_layout.reset_presets();
        }

        main_window::g_hookproc = hook;

        create_icon_handle();

        WNDCLASS wc{};
        wc.lpfnWndProc = static_cast<WNDPROC>(g_MainWindowProc);
        wc.style = CS_DBLCLKS;
        wc.hInstance = core_api::get_my_instance();
        wc.hIcon = ui_control::get()->get_main_icon();
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        wc.lpszClassName = main_window_class_name;

        ATOM cls = RegisterClass(&wc);

        RECT rc_work{};
        SystemParametersInfo(SPI_GETWORKAREA, NULL, &rc_work, NULL);

        const int cx = (rc_work.right - rc_work.left) * 80 / 100;
        const int cy = (rc_work.bottom - rc_work.top) * 80 / 100;

        const int left = (rc_work.right - rc_work.left - cx) / 2;
        const int top = (rc_work.bottom - rc_work.top - cy) / 2;

        if (main_window::config_get_is_first_run()) {
            cfg_plist_width = cx * 10 / 100;
        }

        const DWORD ex_styles = main_window::config_get_transparency_enabled() ? WS_EX_LAYERED : 0;
        const DWORD styles = WS_OVERLAPPED | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN
            | WS_CLIPSIBLINGS | WS_THICKFRAME;

        g_main_window = CreateWindowEx(ex_styles, main_window_class_name, _T("foobar2000"), styles, left, top, cx, cy,
            nullptr, nullptr, core_api::get_my_instance(), nullptr);

        main_window::on_transparency_enabled_change();

        const bool rem_pos = remember_window_pos();

        if (rem_pos && !main_window::config_get_is_first_run()) {
            SetWindowPlacement(g_main_window, &cfg_window_placement_columns.get_value());
            size_windows();
            ShowWindow(g_main_window, cfg_window_placement_columns.get_value().showCmd);

            if (g_icon_created && cfg_go_to_tray)
                ShowWindow(g_main_window, SW_HIDE);
        } else {
            size_windows();
            ShowWindow(g_main_window, SW_SHOWNORMAL);
        }

        if (g_rebar)
            ShowWindow(g_rebar, SW_SHOWNORMAL);
        if (g_status)
            ShowWindow(g_status, SW_SHOWNORMAL);
        if (g_status_pane.get_wnd())
            ShowWindow(g_status_pane.get_wnd(), SW_SHOWNORMAL);
        g_layout_window.show_window();

        RedrawWindow(g_main_window, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);

        if (main_window::config_get_is_first_run())
            SendMessage(g_main_window, MSG_RUN_INITIAL_SETUP, NULL, NULL);

        main_window::config_set_is_first_run();

        return g_main_window;
    }

    GUID get_guid() override { return {0xf12d0a24, 0xa8a4, 0x4618, {0x96, 0x59, 0x6f, 0x66, 0xde, 0x6, 0x75, 0x24}}; }

    void show_now_playing() override
    {
        auto play_api = play_control::get();
        update_systray(true, play_api->is_paused() ? 2 : 0, true);
    }
    void shutdown() override
    {
        DestroyWindow(g_main_window);
        UnregisterClass(main_window_class_name, core_api::get_my_instance());
        status_bar::volume_popup_window.class_release();
        g_main_window = nullptr;
        g_status = nullptr;
        if (g_imagelist) {
            ImageList_Destroy(g_imagelist);
            g_imagelist = nullptr;
        }
        if (g_icon)
            DestroyIcon(g_icon);
        g_icon = nullptr;
        status_bar::destroy_icon();

        font_cleanup();
    }

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
