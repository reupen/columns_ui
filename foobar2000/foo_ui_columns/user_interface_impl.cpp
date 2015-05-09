#include "stdafx.h"

const TCHAR * main_window_class_name = _T("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}");

class ui_test : public user_interface
{
public:

	virtual const char * get_name() { return "Columns UI"; }

	virtual HWND init(HookProc_t hook)
	{
		{
			OSVERSIONINFOEX osvi;
			memset(&osvi, 0, sizeof(osvi));
			osvi.dwOSVersionInfoSize = sizeof(osvi);
			if (GetVersionEx((LPOSVERSIONINFO)&osvi))
			{
				if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 && osvi.wServicePackMajor == 0)
				{
					pfc::string_formatter message;
					message << "Sorry, your operating system Windows XP ";
					if (!osvi.wServicePackMajor)
						message << "(no service pack installed)";
					else
						message << "Service Pack " << osvi.wServicePackMajor;
					message << " is not supported by Columns UI. Please upgrade to Service Pack 1 or newer and try again.\n\nOtherwise, uninstall the Columns UI component to return to the Default User Interface.",
						MessageBox(NULL, uT(message), _T("Columns UI - Unsupported operating system"), MB_OK | MB_ICONEXCLAMATION);
					return NULL;

				}
			}
		}
		//		performance_counter startup;

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
		wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
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
			g_import_fonts_to_unified();
		}

		g_colours_fonts_imported = true;

		g_main_window = CreateWindowEx(main_window::config_get_transparency_enabled() ? WS_EX_LAYERED : 0 /*WS_EX_TOOLWINDOW*/, main_window_class_name, _T("foobar2000"), WS_OVERLAPPED | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
			WS_THICKFRAME, left, top, cx, cy, 0, 0, core_api::get_my_instance(), NULL);

		main_window::on_transparency_enabled_change();

		bool rem_pos = remember_window_pos();

		if (rem_pos && !main_window::config_get_is_first_run())
		{
			SetWindowPlacement(g_main_window, &cfg_window_placement_columns.get_value());
			size_windows();
			ShowWindow(g_main_window, cfg_window_placement_columns.get_value().showCmd);

			if (g_icon_created && (cfg_window_placement_columns.get_value().showCmd == SW_SHOWMINIMIZED) && cfg_minimise_to_tray)
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

		RedrawWindow(g_main_window, 0, 0, RDW_UPDATENOW | RDW_ALLCHILDREN);

		if (main_window::config_get_is_first_run())
			SendMessage(g_main_window, MSG_RUN_INITIAL_SETUP, NULL, NULL);

		main_window::config_set_is_first_run();

		return g_main_window;
	}

	GUID get_guid()
	{
		// {F12D0A24-A8A4-4618-9659-6F66DE067524}
		static const GUID guid_columns =
		{ 0xf12d0a24, 0xa8a4, 0x4618, { 0x96, 0x59, 0x6f, 0x66, 0xde, 0x6, 0x75, 0x24 } };

		return guid_columns;
	}

	virtual void show_now_playing()
	{
		static_api_ptr_t<play_control> play_api;
		update_systray(true, play_api->is_paused() ? 2 : 0, true);
	}
	virtual void shutdown()
	{

		//if (!endsession) 
		{
			//if (IsWindow(g_main_window))
			DestroyWindow(g_main_window);
			UnregisterClass(main_window_class_name, core_api::get_my_instance());
			status_bar::volume_popup_window.class_release();
		}
		g_main_window = 0;
		g_status = 0;
		if (g_imagelist) { ImageList_Destroy(g_imagelist); g_imagelist = 0; }
		if (g_icon) DestroyIcon(g_icon); g_icon = 0;
		status_bar::destroy_icon();

		font_cleanup();
	}

	virtual void activate()
	{
		if (g_main_window)
		{
			if (g_icon_created && !cfg_show_systray) destroy_systray_icon();

			if (!is_visible())
			{
				ShowWindow(g_main_window, SW_RESTORE);
				if ((GetWindowLong(g_main_window, GWL_EXSTYLE) & WS_EX_LAYERED))
					RedrawWindow(g_main_window,
					NULL,
					NULL,
					RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
			}
			SetForegroundWindow(g_main_window);
		}
	}
	virtual void hide()
	{
		if (g_main_window)
		{
			//			if (is_visible()) 
			ShowWindow(g_main_window, SW_MINIMIZE);
		}
	}
	virtual bool is_visible()
	{
		bool rv = false;
		if (g_main_window)
		{
			rv = IsWindowVisible(g_main_window) && !IsIconic(g_main_window);
		}
		return rv;
	}
	virtual void override_statusbar_text(const char * p_text)
	{
		status_bar::menudesc = p_text;
		status_set_menu(true);
		g_status_pane.enter_menu_mode(p_text);
	};
	virtual void revert_statusbar_text()
	{
		status_set_menu(false);
		g_status_pane.exit_menu_mode();
	}
};

static user_interface_factory<ui_test> fooui;
