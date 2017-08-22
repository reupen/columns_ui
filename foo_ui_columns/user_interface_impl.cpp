#include "stdafx.h"
#include "status_pane.h"
#include "layout.h"
#include "config.h"
#include "font_notify.h"
#include "status_bar.h"

const TCHAR * main_window_class_name = _T("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}");

class ui_test : public user_interface
{
public:

    const char * get_name() override { return "Columns UI"; }

    HWND init(HookProc_t hook) override
    {
        {
            if (!IsWindowsXPSP2OrGreater())
            {
                pfc::string_formatter message;
                message << "Sorry, your operating system version is not supported by Columns UI. Please upgrade to Windows XP Service Pack 2 or newer and try again.\n\n"
                    "Otherwise, uninstall the Columns UI component to return to the Default User Interface.",
                    MessageBox(nullptr, uT(message), _T("Columns UI - Unsupported operating system"), MB_OK | MB_ICONEXCLAMATION);
                return nullptr;
            }
        }
        //        performance_counter startup;

        if (main_window::config_get_is_first_run())
        {
            if (!cfg_layout.get_presets().get_count())
                cfg_layout.reset_presets();
        }

        main_window::g_hookproc = hook;

        WNDCLASS  wc;
        memset(&wc, 0, sizeof(wc));

        create_icon_handle();

        wc.lpfnWndProc = (WNDPROC)g_MainWindowProc;
        wc.style = CS_DBLCLKS;
        wc.hInstance = core_api::get_my_instance();
        wc.hIcon = static_api_ptr_t<ui_control>()->get_main_icon();//g_main_icon;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = main_window_class_name;

        ATOM cls = RegisterClass(&wc);

        RECT rc_work;
        SystemParametersInfo(SPI_GETWORKAREA, NULL, &rc_work, NULL);

        const unsigned cx = (rc_work.right - rc_work.left) * 80 / 100;
        const unsigned cy = (rc_work.bottom - rc_work.top) * 80 / 100;

        unsigned left = (rc_work.right - rc_work.left - cx) / 2;
        unsigned top = (rc_work.bottom - rc_work.top - cy) / 2;

        if (main_window::config_get_is_first_run())
        {
            cfg_plist_width = cx * 10 / 100;
        }
        else if (!g_colours_fonts_imported)
        {
            g_import_pv_colours_to_unified_global();
        }

        g_colours_fonts_imported = true;

        g_main_window = CreateWindowEx(main_window::config_get_transparency_enabled() ? WS_EX_LAYERED : 0 /*WS_EX_TOOLWINDOW*/, main_window_class_name, _T("foobar2000"), WS_OVERLAPPED | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
            WS_THICKFRAME, left, top, cx, cy, nullptr, nullptr, core_api::get_my_instance(), nullptr);

        main_window::on_transparency_enabled_change();

        bool rem_pos = remember_window_pos();

        if (rem_pos && !main_window::config_get_is_first_run())
        {
            SetWindowPlacement(g_main_window, &cfg_window_placement_columns.get_value());
            size_windows();
            ShowWindow(g_main_window, cfg_window_placement_columns.get_value().showCmd);

            if (g_icon_created && cfg_go_to_tray)
                ShowWindow(g_main_window, SW_HIDE);
        }
        else
        {
            size_windows();
            ShowWindow(g_main_window, SW_SHOWNORMAL);
        }

        if (g_rebar) ShowWindow(g_rebar, SW_SHOWNORMAL);
        if (g_status) ShowWindow(g_status, SW_SHOWNORMAL);
        if (g_status_pane.get_wnd()) ShowWindow(g_status_pane.get_wnd(), SW_SHOWNORMAL);
        g_layout_window.show_window();

        RedrawWindow(g_main_window, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);

        if (main_window::config_get_is_first_run())
            SendMessage(g_main_window, MSG_RUN_INITIAL_SETUP, NULL, NULL);

        main_window::config_set_is_first_run();

        return g_main_window;
    }

    GUID get_guid() override
    {
        // {F12D0A24-A8A4-4618-9659-6F66DE067524}
        static const GUID guid_columns =
        { 0xf12d0a24, 0xa8a4, 0x4618, { 0x96, 0x59, 0x6f, 0x66, 0xde, 0x6, 0x75, 0x24 } };

        return guid_columns;
    }

    void show_now_playing() override
    {
        static_api_ptr_t<play_control> play_api;
        update_systray(true, play_api->is_paused() ? 2 : 0, true);
    }
    void shutdown() override
    {

        //if (!endsession) 
        {
            //if (IsWindow(g_main_window))
            DestroyWindow(g_main_window);
            UnregisterClass(main_window_class_name, core_api::get_my_instance());
            status_bar::volume_popup_window.class_release();
        }
        g_main_window = nullptr;
        g_status = nullptr;
        if (g_imagelist) { ImageList_Destroy(g_imagelist); g_imagelist = nullptr; }
        if (g_icon) DestroyIcon(g_icon); g_icon = nullptr;
        status_bar::destroy_icon();

        font_cleanup();
    }

    void activate() override
    {
        if (g_main_window)
        {
            cfg_go_to_tray = false;
            if (g_icon_created && !cfg_show_systray) destroy_systray_icon();

            if (!is_visible())
            {
                ShowWindow(g_main_window, SW_RESTORE);
                if ((GetWindowLong(g_main_window, GWL_EXSTYLE) & WS_EX_LAYERED))
                    RedrawWindow(g_main_window,
                    nullptr,
                    nullptr,
                    RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
            }
            SetForegroundWindow(g_main_window);
        }
    }
    void hide() override
    {
        if (g_main_window)
        {
            //            if (is_visible()) 
            ShowWindow(g_main_window, SW_MINIMIZE);
        }
    }
    bool is_visible() override
    {
        bool rv = false;
        if (g_main_window)
        {
            rv = IsWindowVisible(g_main_window) && !IsIconic(g_main_window);
        }
        return rv;
    }
    void override_statusbar_text(const char * p_text) override
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

static user_interface_factory<ui_test> fooui;
