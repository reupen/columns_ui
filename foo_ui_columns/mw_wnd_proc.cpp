#include "stdafx.h"

INT_PTR g_taskbar_bitmaps[] = { IDI_STOP, IDI_PREV, IDI_PAUSE, IDI_PLAY, IDI_NEXT, IDI_RAND };

namespace statusbar_contextmenus
{
	enum { ID_BASE = 1 };
	service_ptr_t<contextmenu_manager> g_main_nowplaying;
}

namespace systray_contextmenus
{
	enum { ID_ACTIVATE = 1, ID_NOW_PLAYING_BASE = 2, ID_BASE_FILE_PREFS = 0x2000, ID_BASE_FILE_EXIT = 0x3000, ID_BASE_PLAYBACK = 0x4000 };
	service_ptr_t<mainmenu_manager> g_menu_file_prefs;
	service_ptr_t<mainmenu_manager> g_menu_file_exit;
	service_ptr_t<mainmenu_manager> g_menu_playback;
	service_ptr_t<contextmenu_manager> g_main_nowplaying;
}

get_msg_hook_t g_get_msg_hook;
static HWND wnd_last;

LRESULT CALLBACK g_MainWindowProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{

	static UINT WM_TASKBARCREATED;
	static UINT WM_TASKBARBUTTONCREATED;
	static UINT WM_SHELLHOOKMESSAGE;
	static bool g_last_sysray_r_down;
	static bool g_last_sysray_l_down;
	static bool g_last_sysray_x1_down;
	static bool g_last_sysray_x2_down;
	static bool g_sep_toggle;

	static HIMAGELIST g_imagelist_taskbar;


	if (main_window::g_hookproc)
	{
		LRESULT ret;
		if (main_window::g_hookproc(wnd, msg, wp, lp, &ret))
		{
			return ret;
		}
	}

	//ermm we should probably use some kind of class so we can initialise this as a const value
	if (WM_TASKBARCREATED && msg == WM_TASKBARCREATED)
	{
		if (g_icon_created)
		{
			g_icon_created = false;
			create_systray_icon();
			update_systray();
		}
		//		return 0;
	}

	if (WM_TASKBARBUTTONCREATED && msg == WM_TASKBARBUTTONCREATED)
	{
		mmh::comptr_t<ITaskbarList> p_ITaskbarList;
		if (SUCCEEDED(p_ITaskbarList.instantiate(CLSID_TaskbarList)))
		{
			main_window::g_ITaskbarList3 = p_ITaskbarList;
			if (main_window::g_ITaskbarList3.is_valid() && SUCCEEDED(main_window::g_ITaskbarList3->HrInit()))
			{
				const unsigned cx = GetSystemMetrics(SM_CXSMICON), cy = GetSystemMetrics(SM_CYSMICON);

				g_imagelist_taskbar = ImageList_Create(cx, cy, ILC_COLOR32, 0, 6);

				t_size i = 0;

				for (i = 0; i < 6; i++)
				{
					HICON icon = (HICON)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(g_taskbar_bitmaps[i]), IMAGE_ICON, cx, cy, NULL);
					ImageList_ReplaceIcon(g_imagelist_taskbar, -1, icon);
					DestroyIcon(icon);
				}

				if (SUCCEEDED(main_window::g_ITaskbarList3->ThumbBarSetImageList(wnd, g_imagelist_taskbar)))
				{
					g_update_taskbar_buttons_delayed(true);
				}
				else main_window::g_ITaskbarList3.release();
			}
		}
	}

	if (WM_SHELLHOOKMESSAGE && msg == WM_SHELLHOOKMESSAGE)
	{
		if (wp == HSHELL_APPCOMMAND)
		{
			short cmd = GET_APPCOMMAND_LPARAM(lp);
			WORD uDevice = GET_DEVICE_LPARAM(lp);
			WORD dwKeys = GET_KEYSTATE_LPARAM(lp);
			switch (cmd)
			{
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
				standard_commands::main_play_or_pause();
				return TRUE;
			case APPCOMMAND_MEDIA_PLAY:
				standard_commands::main_play();
				return TRUE;
			case APPCOMMAND_MEDIA_PAUSE:
				standard_commands::main_pause();
				return TRUE;
			case APPCOMMAND_MEDIA_STOP:
				standard_commands::main_stop();
				return TRUE;
			case APPCOMMAND_MEDIA_NEXTTRACK:
				standard_commands::main_next();
				return TRUE;
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
				standard_commands::main_previous();
				return TRUE;
			default:
				break;
			}
		}
	}

	switch (msg)
	{
	case WM_COMMAND:
	{
		switch (wp)
		{
		case taskbar_buttons::ID_STOP | (THBN_CLICKED << 16) :
			standard_commands::main_stop();
			break;
		case taskbar_buttons::ID_PLAY_OR_PAUSE | (THBN_CLICKED << 16) :
			standard_commands::main_play_or_pause();
			break;
		case taskbar_buttons::ID_PREV | (THBN_CLICKED << 16) :
			standard_commands::main_previous();
			break;
		case taskbar_buttons::ID_NEXT | (THBN_CLICKED << 16) :
			standard_commands::main_next();
			break;
		case taskbar_buttons::ID_RAND | (THBN_CLICKED << 16) :
			standard_commands::main_random();
			break;
		}
	}
	break;
	case WM_MENUSELECT:
	{
		if (HIWORD(wp) & MF_POPUP)
		{
			status_set_menu(false);
		}
		else
		{
			if (systray_contextmenus::g_menu_file_prefs.is_valid() ||
				systray_contextmenus::g_menu_file_exit.is_valid() ||
				systray_contextmenus::g_menu_playback.is_valid() ||
				systray_contextmenus::g_main_nowplaying.is_valid() ||
				statusbar_contextmenus::g_main_nowplaying.is_valid()
				)
			{
				unsigned id = LOWORD(wp);

				bool set = false;
				if (statusbar_contextmenus::g_main_nowplaying.is_valid())
				{
					contextmenu_node * node = statusbar_contextmenus::g_main_nowplaying->find_by_id(id - statusbar_contextmenus::ID_BASE);
					if (node) set = node->get_description(status_bar::menudesc);
				}

				if (systray_contextmenus::g_main_nowplaying.is_valid() && id < systray_contextmenus::ID_BASE_FILE_PREFS && id >= systray_contextmenus::ID_NOW_PLAYING_BASE)
				{
					contextmenu_node * node = systray_contextmenus::g_main_nowplaying->find_by_id(id - systray_contextmenus::ID_NOW_PLAYING_BASE);
					if (node) set = node->get_description(status_bar::menudesc);
				}
				else if (systray_contextmenus::g_menu_file_prefs.is_valid() && id < systray_contextmenus::ID_BASE_FILE_EXIT)
				{
					set = systray_contextmenus::g_menu_file_prefs->get_description(id - systray_contextmenus::ID_BASE_FILE_PREFS, status_bar::menudesc);
				}
				else if (systray_contextmenus::g_menu_file_exit.is_valid() && id < systray_contextmenus::ID_BASE_PLAYBACK)
				{
					set = systray_contextmenus::g_menu_file_exit->get_description(id - systray_contextmenus::ID_BASE_FILE_EXIT, status_bar::menudesc);
				}
				else if (systray_contextmenus::g_menu_playback.is_valid())
				{
					set = systray_contextmenus::g_menu_playback->get_description(id - systray_contextmenus::ID_BASE_PLAYBACK, status_bar::menudesc);
				}

				status_set_menu(set);
			}
		}
	}
	break;
	case WM_SYSCOMMAND:
		if (wp == SC_KEYMENU && !lp)
		{
			bool processed = false;
			if (g_rebar_window) processed = g_rebar_window->on_alt_up();
			if (!processed) g_layout_window.set_menu_focus();
			else g_layout_window.hide_menu_access_keys();
			return 0;
		}
		break;
	case WM_CONTEXTMENU:
		if (g_status && (HWND)wp == g_status)
		{
			POINT pt = { (short)(LOWORD(lp)), (short)(HIWORD(lp)) };
			enum { ID_CUSTOM_BASE = 1 };
			HMENU menu = CreatePopupMenu();
			service_ptr_t<contextmenu_manager> p_manager;
			contextmenu_manager::g_create(p_manager);
			if (p_manager.is_valid())
			{
				statusbar_contextmenus::g_main_nowplaying = p_manager;

				const keyboard_shortcut_manager::shortcut_type shortcuts[] = { keyboard_shortcut_manager::TYPE_CONTEXT_NOW_PLAYING };
				p_manager->set_shortcut_preference(shortcuts, tabsize(shortcuts));
				if (p_manager->init_context_now_playing(standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0))
				{

					p_manager->win32_build_menu(menu, ID_CUSTOM_BASE, -1);
				}

				menu_helpers::win32_auto_mnemonics(menu);

				int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, 0);
				DestroyMenu(menu);
				statusbar_contextmenus::g_main_nowplaying.release();

				if (cmd)
				{
					p_manager->execute_by_id(cmd - ID_CUSTOM_BASE);
				}


			}

		}
		else if ((HWND)wp == g_rebar)
		{
			if (g_rebar_window)
			{
				enum { IDM_LOCK = 1, IDM_CLOSE, IDM_BASE };

				ui_extension::window_info_list_simple moo;

				service_enum_t<ui_extension::window> e;
				uie::window_ptr l;

				POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };

				if (pt.x == -1 && pt.y == -1)
				{
					RECT rc;
					GetWindowRect(GetFocus(), &rc);
					pt.x = rc.left;
					pt.y = rc.bottom;
				}

				POINT pt_client = pt;

				ScreenToClient(g_rebar_window->wnd_rebar, &pt_client);

				unsigned IDM_EXT_BASE = IDM_BASE + 1;
				unsigned n;

				HWND child = RealChildWindowFromPoint(g_rebar_window->wnd_rebar, pt_client);

				RBHITTESTINFO rbht;
				rbht.pt = pt_client;

				int idx_hit = uSendMessage(g_rebar, RB_HITTEST, 0, (LPARAM)&rbht);

				uie::window_ptr p_ext;

				if (idx_hit < g_rebar_window->bands.get_count())
					p_ext = g_rebar_window->bands[idx_hit]->p_ext;

				pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> extension_menu_nodes = new ui_extension::menu_hook_impl;

				HMENU menu = CreatePopupMenu();

				if (e.first(l))
					do
					{
						if (g_rebar_window->check_band(l->get_extension_guid()) || (((false || (l->get_type() & ui_extension::type_toolbar))) && l->is_available(&get_rebar_host())))
						{
							ui_extension::window_info_simple info;

							l->get_name(info.name);
							l->get_category(info.category);
							info.guid = l->get_extension_guid();
							info.prefer_multiple_instances = l->get_prefer_multiple_instances();

							moo.add_item(info);

							l.release();
						}
					} while (e.next(l));

					moo.sort();

					unsigned count_exts = moo.get_count();
					HMENU popup;
					for (n = 0; n < count_exts; n++)
					{
						if (!n || uStringCompare(moo[n - 1].category, moo[n].category))
						{
							if (n) uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)popup, moo[n - 1].category);
							popup = CreatePopupMenu();
						}
						uAppendMenu(popup, (MF_STRING | (g_rebar_window->check_band(moo[n].guid) ? MF_CHECKED : 0)), IDM_BASE + n, moo[n].name);
						if (n == count_exts - 1) uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)popup, moo[n].category);
						IDM_EXT_BASE++;
					}

					uAppendMenu(menu, MF_SEPARATOR, 0, "");
					uAppendMenu(menu, (((cfg_lock) ? MF_CHECKED : 0) | MF_STRING), IDM_LOCK, "Lock the toolbars");
					if (idx_hit != -1) uAppendMenu(menu, (MF_STRING), IDM_CLOSE, "Remove toolbar");

					if (p_ext.is_valid())
					{
						p_ext->get_menu_items(*extension_menu_nodes.get_ptr());
						if (extension_menu_nodes->get_children_count() > 0)
							uAppendMenu(menu, MF_SEPARATOR, 0, 0);

						extension_menu_nodes->win32_build_menu(menu, IDM_EXT_BASE, pfc_infinite - IDM_EXT_BASE);
					}

					menu_helpers::win32_auto_mnemonics(menu);

					int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, 0);

					if (g_rebar_window)
					{

						if (cmd >= IDM_EXT_BASE)
						{
							extension_menu_nodes->execute_by_id(cmd);
						}

						//			if (p_ext.is_valid()) p_ext->menu_action(menu, IDM_EXT_BASE, cmd, user_data);

						DestroyMenu(menu);


						if (cmd == IDM_LOCK)
						{
							cfg_lock = (cfg_lock == 0);
							g_rebar_window->update_bands();
						}
						if (cmd == IDM_CLOSE)
						{
							g_rebar_window->delete_band(idx_hit);
						}
						else if (cmd > 0 && cmd - IDM_BASE < moo.get_count())
						{
							bool shift_down = (GetAsyncKeyState(VK_SHIFT) & (1 << 31)) != 0;
							//				bool ctrl_down = (GetAsyncKeyState(VK_CONTROL) & (1 << 31)) != 0;

							if (!shift_down && !moo[cmd - IDM_BASE].prefer_multiple_instances && g_rebar_window->check_band(moo[cmd - IDM_BASE].guid))
							{
								g_rebar_window->delete_band(moo[cmd - IDM_BASE].guid);
							}
							else
							{
								if (idx_hit != -1)
									g_rebar_window->insert_band(idx_hit + 1, moo[cmd - IDM_BASE].guid, g_rebar_window->cache.get_width(moo[cmd - IDM_BASE].guid));
								else
									g_rebar_window->add_band(moo[cmd - IDM_BASE].guid, g_rebar_window->cache.get_width(moo[cmd - IDM_BASE].guid));
							}
						}
					}

			}
		}
		break;
	case MSG_UPDATE_THUMBBAR:
		g_update_taskbar_buttons_now(wp != 0);
		break;
	case MSG_SET_AOT:
		SetWindowPos(wnd, wp ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		break;
	case MSG_UPDATE_STATUS:
		update_status();
		break;
	case MSG_UPDATE_TITLE:
		update_titlebar();
		break;
	case MSG_RUN_INITIAL_SETUP:
		setup_dialog_t::g_run();
		return 0;
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_DRAWITEM:
	{

		if (((LPDRAWITEMSTRUCT)lp)->CtlID == ID_STATUS)
		{
			RECT rc = ((LPDRAWITEMSTRUCT)lp)->rcItem;
			//			rc.right -= 3;
			if (!cfg_show_vol && !cfg_show_seltime && !IsZoomed(g_main_window))
			{
				RECT rc_main;
				GetClientRect(g_main_window, &rc_main);
				rc.right = rc_main.right - GetSystemMetrics(SM_CXVSCROLL);
			}
			else
			{
				int blah[3];
				uSendMessage(g_status, SB_GETBORDERS, 0, (LPARAM)&blah);
				rc.right -= blah[2];
			}

			if (rc.left > rc.right) rc.right = rc.left;

			if (((LPDRAWITEMSTRUCT)lp)->itemData)
			{
#if 0
				HDC dc_mem = CreateCompatibleDC(((LPDRAWITEMSTRUCT)lp)->hDC);

				HBITMAP bm_mem = CreateCompatibleBitmap(((LPDRAWITEMSTRUCT)lp)->hDC, rc.right - rc.left, rc.bottom - rc.top);
				HBITMAP bm_old = (HBITMAP)SelectObject(dc_mem, bm_mem);

				RECT rc2 = { 0, 0, rc.right - rc.left, rc.bottom - rc.top };

				OffsetWindowOrgEx(((LPDRAWITEMSTRUCT)lp)->hDC, 0 - rc.left, 0 - rc.top, 0);

				BitBlt(dc_mem, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
					((LPDRAWITEMSTRUCT)lp)->hDC, 0, 0, SRCCOPY);

				OffsetWindowOrgEx(((LPDRAWITEMSTRUCT)lp)->hDC, rc.left, rc.top, 0);


				ui_helpers::text_out_colours_tab(dc_mem, *(pfc::string8 *)((LPDRAWITEMSTRUCT)lp)->itemData,
					((pfc::string8 *)((LPDRAWITEMSTRUCT)lp)->itemData)->length(), 2, 0, &rc, FALSE, GetSysColor(COLOR_MENUTEXT),
					TRUE, true, false, ui_helpers::ALIGN_LEFT);

				BitBlt(((LPDRAWITEMSTRUCT)lp)->hDC, rc.left, rc.top, rc.right, rc.bottom,
					dc_mem, 0, 0, SRCCOPY);

				SelectObject(dc_mem, bm_old);
				DeleteObject(bm_mem);
				DeleteDC(dc_mem);
#else
				ui_helpers::text_out_colours_tab(((LPDRAWITEMSTRUCT)lp)->hDC, *(pfc::string8 *)((LPDRAWITEMSTRUCT)lp)->itemData,
					((pfc::string8 *)((LPDRAWITEMSTRUCT)lp)->itemData)->length(), 2, 0, &rc, FALSE, GetSysColor(COLOR_MENUTEXT),
					TRUE, true, false, ui_helpers::ALIGN_LEFT);
#endif
			}

			return TRUE;
		}


	}
	break;
	case WM_WINDOWPOSCHANGED:
	{
		LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
		if (!(lpwp->flags & SWP_NOSIZE))
		{
			ULONG_PTR styles = GetWindowLongPtr(wnd, GWL_STYLE);
			if (styles & WS_MINIMIZE)
			{
				g_minimised = true;
				if (!g_icon_created && cfg_minimise_to_tray) create_systray_icon();
				if (g_icon_created && cfg_minimise_to_tray) ShowWindow(wnd, SW_HIDE);
			}
			else
			{
				g_minimised = false;
				size_windows();
			}
		}
	}
	break;


	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		break;
	case WM_MENUCHAR:
	{
		unsigned short chr = LOWORD(wp);
		bool processed = false;
		if (g_rebar_window)
		{
			processed = g_rebar_window->on_menu_char(chr);
		}
		if (!processed)
			g_layout_window.on_menu_char(chr);
	}
	return (MNC_CLOSE << 16);
	case WM_SHOWWINDOW:
		break;
	case WM_KILLFOCUS:
		break;
	case WM_CREATE:
	{
		/* initialise ui */

		OSVERSIONINFOEX  vi;
		vi.dwOSVersionInfoSize = sizeof(vi);

		GetVersionEx((OSVERSIONINFO*)&vi);

		//			modeless_dialog_manager::add(wnd);

		WM_TASKBARCREATED = RegisterWindowMessage(L"TaskbarCreated");
		WM_SHELLHOOKMESSAGE = RegisterWindowMessage(TEXT("SHELLHOOK"));
		WM_TASKBARBUTTONCREATED = RegisterWindowMessage(L"TaskbarButtonCreated");

		win32_helpers::RegisterShellHookWindowHelper(wnd);

		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_WIN95_CLASSES;
		InitCommonControlsEx(&icex);

		cui::g_main_window.initialise();

		//g_gdiplus_initialised = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&g_gdiplus_instance, &Gdiplus::GdiplusStartupInput(), NULL));

		if (!g_keyboard_cues_enabled())
			SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEFOCUS), NULL);

		g_main_window = wnd;
		statusbartext = core_version_info::g_get_version_string();
		set_main_window_text("foobar2000"/*core_version_info::g_get_version_string()*/);
		if (cfg_show_systray) create_systray_icon();

		HRESULT hr = OleInitialize(0);
		pfc::com_ptr_t<drop_handler_interface> drop_handler = new drop_handler_interface;
		RegisterDragDrop(g_main_window, drop_handler.get_ptr());

		/* end of initialisation */

		//			ShowWindow(wnd, SW_SHOWNORMAL);

		make_ui();

		g_get_msg_hook.register_hook();

		//lets try recursively in wm_showwindow

		//			SetWindowPos(wnd, 0, cfg_window_placement_columns.get_value().rcNormalPosition.left, cfg_window_placement_columns.get_value().rcNormalPosition.top, cfg_window_placement_columns.get_value().rcNormalPosition.right - cfg_window_placement_columns.get_value().rcNormalPosition.left, cfg_window_placement_columns.get_value().rcNormalPosition.bottom - cfg_window_placement_columns.get_value().rcNormalPosition.top, SWP_NOZORDER);
		//			ShowWindow(wnd, SW_SHOWNORMAL);
		if (config_object::g_get_data_bool_simple(standard_config_objects::bool_ui_always_on_top, false))
			SendMessage(wnd, MSG_SET_AOT, TRUE, 0);


	}

	return 0;
	case WM_PARENTNOTIFY:
	{
		if (wp == WM_DESTROY)
		{
		}
	}
	break;
	case WM_SYSCOLORCHANGE:
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_TIMECHANGE:
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_SETTINGCHANGE:
		if (wp == SPI_SETNONCLIENTMETRICS)
		{
		}
		else if (wp == SPI_SETKEYBOARDCUES)
		{
			bool cues = g_keyboard_cues_enabled();
			SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(cues ? UIS_CLEAR : UIS_SET, UISF_HIDEFOCUS), NULL);
			//SendMessage(wnd, WM_UPDATEUISTATE, MAKEWPARAM(cues ? UIS_CLEAR : UIS_SET, UISF_HIDEFOCUS), NULL);
			//return 0;
		}
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_THEMECHANGED:
		if (mmh::osversion::is_windows_xp_or_newer())
		{
			if (g_rebar_window) g_rebar_window->on_themechanged();
			if (g_status)
			{
				status_bar::destroy_theme_handle();
				status_bar::create_theme_handle();
				status_bar::set_part_sizes(status_bar::t_parts_none);
			}
		}
		break;

	case WM_KEYDOWN:
		if (process_keydown(msg, lp, wp)) return 0;
		break;
	case WM_SYSKEYUP:
		if (process_keydown(msg, lp, wp)) return 0;
		break;
	case WM_SYSKEYDOWN:
		if (process_keydown(msg, lp, wp)) return 0;
		break;
	case MSG_NOTICATION_ICON:
		if (lp == WM_LBUTTONDOWN)
		{
			g_last_sysray_l_down = true;
		}
		else if (lp == WM_LBUTTONUP)
		{
			//bool b_wasDown = g_last_sysray_l_down;
			g_last_sysray_l_down = false;
			//if (b_wasDown) 
			standard_commands::main_activate_or_hide();
		}
		else if (lp == WM_RBUTTONDOWN)
		{
			g_last_sysray_r_down = true;
		}
#if 0		
		/* There was some misbehaviour with the newer messages. So we don't use them. */
		if (lp == NIN_SELECT || lp == NIN_KEYSELECT)
		{
			standard_commands::main_activate_or_hide();
			//PostMessage(wnd, WM_NULL, 0, 0);
			return TRUE;
		}
#endif
		else if (lp == WM_XBUTTONDOWN)
		{
			g_last_sysray_x1_down = HIBYTE(GetKeyState(VK_XBUTTON1)) != 0;
			g_last_sysray_x2_down = HIBYTE(GetKeyState(VK_XBUTTON2)) != 0;
			return TRUE;
		}
		else if (lp == WM_MOUSEMOVE)
		{
		}
		else if (lp == WM_XBUTTONUP)
		{
			if (g_advbool_notification_icon_x_buttons.get_static_instance().get_state())
			{
				if (g_last_sysray_x1_down && !g_last_sysray_x2_down)
					standard_commands::main_previous();
				if (g_last_sysray_x2_down && !g_last_sysray_x1_down)
					standard_commands::main_next();
			}
			g_last_sysray_x1_down = false;
			g_last_sysray_x2_down = false;
			return TRUE;
		}
		//else if (lp == WM_CONTEXTMENU) 
		else if (lp == WM_RBUTTONUP)
		{
			if (g_last_sysray_r_down)
			{

				SetForegroundWindow(wnd);

				POINT pt;// = {(short)LOWORD(lp),(short)HIWORD(lp)};
				GetCursorPos(&pt);

				HMENU menu = CreatePopupMenu();
				HMENU menu_now_playing = 0;

				service_ptr_t<contextmenu_manager> p_manager_selection;

				if (cfg_np)
				{
					contextmenu_manager::g_create(p_manager_selection);
					if (p_manager_selection.is_valid())
					{
						const keyboard_shortcut_manager::shortcut_type shortcuts[] = { keyboard_shortcut_manager::TYPE_CONTEXT_NOW_PLAYING };
						p_manager_selection->set_shortcut_preference(shortcuts, tabsize(shortcuts));

						if (p_manager_selection->init_context_now_playing(standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS_GLOBAL : 0))
						{
							menu_now_playing = CreatePopupMenu();


							p_manager_selection->win32_build_menu(menu_now_playing, systray_contextmenus::ID_NOW_PLAYING_BASE, systray_contextmenus::ID_BASE_FILE_PREFS - 1);

							pfc::string8_fast_aggressive title, name, title2, title3;
							static_api_ptr_t<play_control> play_api;
							metadb_handle_ptr track;
							if (play_api->get_now_playing(track))
							{
								metadb_info_container::ptr p_info;
								if (track->get_async_info_ref(p_info))
								{
									const char * ptr = p_info->info().meta_get("TITLE", 0);
									if (ptr)
										title2 = ptr;
								}
							}
							if (title2.length() > 25) { title2.truncate(24); title2 += "\xe2\x80\xa6"; }

							title.prealloc(14 + 25);
							title.set_string(uStringPrintf("Now Playing: %s", title2.get_ptr()));
							//			play_control::get()->playback_format_title_ex(track, title, "$puts(title,Now Playing: %title%)$ifgreater($len($get(title)),25,$cut($get(title),24)..,$get(title))",0,0,true);
							track.release();
							uFixAmpersandChars_v2(title, name);

							uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)menu_now_playing, name);

							uAppendMenu(menu, MF_SEPARATOR, 0, "");
						}
					}

				}


				service_ptr_t<mainmenu_manager> p_manager_context = standard_api_create_t<mainmenu_manager>();
				service_ptr_t<mainmenu_manager> p_manager_playback = standard_api_create_t<mainmenu_manager>();
				systray_contextmenus::g_menu_file_exit = standard_api_create_t<mainmenu_manager>();

				bool b_shortcuts = standard_config_objects::query_show_keyboard_shortcuts_in_menus();

				if (p_manager_playback.is_valid())
				{
					p_manager_playback->instantiate(mainmenu_groups::playback_controls);
					p_manager_playback->generate_menu_win32(menu, systray_contextmenus::ID_BASE_PLAYBACK, pfc_infinite, b_shortcuts ? mainmenu_manager::flag_show_shortcuts_global : 0);

					AppendMenu(menu, MF_SEPARATOR, 0, 0);
				}
				if (p_manager_context.is_valid())
				{

					p_manager_context->instantiate(mainmenu_groups::file_etc_preferences);
					p_manager_context->generate_menu_win32(menu, systray_contextmenus::ID_BASE_FILE_PREFS, systray_contextmenus::ID_BASE_FILE_EXIT - 1, b_shortcuts ? mainmenu_manager::flag_show_shortcuts_global : 0);
				}

				AppendMenu(menu, MF_SEPARATOR, 0, 0);
				t_size insert_point = GetMenuItemCount(menu);
				if (systray_contextmenus::g_menu_file_exit.is_valid())
				{

					systray_contextmenus::g_menu_file_exit->instantiate(mainmenu_groups::file_etc_exit);
					systray_contextmenus::g_menu_file_exit->generate_menu_win32(menu, systray_contextmenus::ID_BASE_FILE_EXIT, systray_contextmenus::ID_BASE_PLAYBACK - 1, b_shortcuts ? mainmenu_manager::flag_show_shortcuts_global : 0);
				}

				bool b_visible = static_api_ptr_t<ui_control>()->is_visible();
				InsertMenu(menu, insert_point, MF_STRING | MF_BYPOSITION, systray_contextmenus::ID_ACTIVATE, b_visible ? _T("Hide foobar2000") : _T("Show foobar2000"));

				systray_contextmenus::g_menu_file_prefs = p_manager_context;
				systray_contextmenus::g_menu_playback = p_manager_playback;
				systray_contextmenus::g_main_nowplaying = p_manager_selection;

				menu_helpers::win32_auto_mnemonics(menu);

				MENUITEMINFO mi;
				memset(&mi, 0, sizeof(mi));
				mi.cbSize = sizeof(MENUITEMINFO);
				mi.fMask = MIIM_STATE;
				mi.fState = MFS_DEFAULT;

				SetMenuItemInfo(menu, systray_contextmenus::ID_ACTIVATE, FALSE, &mi);
				int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, 0);



				DestroyMenu(menu);

				systray_contextmenus::g_menu_file_prefs.release();
				systray_contextmenus::g_menu_playback.release();
				systray_contextmenus::g_main_nowplaying.release();

				if (cmd)
				{
					if (cmd == systray_contextmenus::ID_ACTIVATE)
					{
						if (b_visible)
							static_api_ptr_t<ui_control>()->hide();
						else
							static_api_ptr_t<ui_control>()->activate();
					}
					else if (cmd < systray_contextmenus::ID_BASE_FILE_PREFS)
					{
						if (p_manager_selection.is_valid())
						{
							p_manager_selection->execute_by_id(cmd - systray_contextmenus::ID_NOW_PLAYING_BASE);
						}
					}
					else if (cmd < systray_contextmenus::ID_BASE_FILE_EXIT)
					{
						if (p_manager_context.is_valid())
						{
							p_manager_context->execute_command(cmd - systray_contextmenus::ID_BASE_FILE_PREFS);
						}
					}
					else if (cmd < systray_contextmenus::ID_BASE_PLAYBACK)
					{
						if (systray_contextmenus::g_menu_file_exit.is_valid())
						{
							systray_contextmenus::g_menu_file_exit->execute_command(cmd - systray_contextmenus::ID_BASE_FILE_EXIT);
						}
					}
					else
					{
						if (p_manager_playback.is_valid())
						{
							p_manager_playback->execute_command(cmd - systray_contextmenus::ID_BASE_PLAYBACK);
						}
					}
					systray_contextmenus::g_menu_file_exit.release();
				}

				PostMessage(wnd, WM_NULL, 0, 0);
			}
			return TRUE;
		}
		break;
	case WM_ACTIVATE:
	{
		if ((LOWORD(wp) == WA_INACTIVE))
		{
			if (!g_keyboard_cues_enabled())
				SendMessage(wnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), NULL);
			wnd_last = GetFocus();
			//if (is_win2k_or_newer())
			{
				if (g_rebar_window) g_rebar_window->hide_accelerators();
				g_layout_window.hide_menu_access_keys();
			}
		}
	}
	break;
	case WM_SETFOCUS:
	{
		if (wnd_last && IsWindow(wnd_last)) SetFocus(wnd_last);
		else g_layout_window.set_focus();

		// wnd_last = 0; // meh minimised fuko
	}
	break;
	case MSG_SET_FOCUS:
	{
	}
	break;
	case WM_ACTIVATEAPP:
	{
	}
	break;
	case MSG_SIZE:
		size_windows();
		return 0;


	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONUP:
		break;
	case WM_DESTROY:
	{
		g_get_msg_hook.deregister_hook();
		g_layout_window.destroy();
		win32_helpers::DeregisterShellHookWindowHelper(wnd);


		destroy_rebar(false);
		status_bar::destroy_status_window();
		main_window::g_ITaskbarList3.release();
		RevokeDragDrop(g_main_window);
		destroy_systray_icon();
		cui::g_main_window.deinitialise();
		OleUninitialize();
	}
	break;
	case WM_NCDESTROY:
		if (g_imagelist_taskbar)
			ImageList_Destroy(g_imagelist_taskbar);
		break;
	case WM_CLOSE:
	{
		standard_commands::main_exit();
	}
	return 0;
	case WM_TIMER:
	{

	}
	break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lp)->idFrom)
		{
		case ID_REBAR:
			switch (((LPNMHDR)lp)->code)
			{
			case RBN_HEIGHTCHANGE:
			{
				size_windows();
			}
			break;
			case RBN_LAYOUTCHANGED:
			{
				if (g_rebar_window)
				{
					g_rebar_window->save_bands();
				}
			}
			break;
			}
			break;
		case ID_STATUS:
			switch (((LPNMHDR)lp)->code)
			{
			case NM_RCLICK:
			case NM_CLICK:
			{
				LPNMMOUSE lpnm = (LPNMMOUSE)lp;
				unsigned u_parts = SendMessage(lpnm->hdr.hwndFrom, SB_GETPARTS, 0, 0);
				pfc::array_t<unsigned> parts;
				parts.set_size(u_parts);
				SendMessage(lpnm->hdr.hwndFrom, SB_GETPARTS, parts.get_size(), (LPARAM)parts.get_ptr());
				u_parts = parts.get_size();
				if (!IsZoomed(wnd) && u_parts && parts[u_parts - 1] == pfc_infinite)
				{
					RECT rc;
					GetClientRect(lpnm->hdr.hwndFrom, &rc);
					parts[u_parts - 1] = rc.right - GetSystemMetrics(SM_CXVSCROLL);
				}

				unsigned part = -1, n = 0;
				for (n = 0; n < u_parts; n++)
				{
					if ((unsigned)lpnm->pt.x < parts[n])
					{
						part = n;
						break;
					}
				}
				if (cfg_show_vol && /*lpnm->dwItemSpec*/ part == status_bar::u_vol_pos)
				{

					if (!status_bar::volume_popup_window.get_wnd())
					{
						//caption vertical. alt-f4 send crazy with two??
						RECT rc_status;
						GetWindowRect(lpnm->hdr.hwndFrom, &rc_status);
						HWND wndvol = status_bar::volume_popup_window.create(wnd);
						POINT pt = lpnm->pt;
						ClientToScreen(lpnm->hdr.hwndFrom, &pt);
						int cx = volume_popup_t::g_get_caption_size() + 28;
						int cy = 150;
						int x = pt.x;
						int y = pt.y;
						HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
						if (mon)
						{

							MONITORINFO mi;
							memset(&mi, 0, sizeof(MONITORINFO));
							mi.cbSize = sizeof(MONITORINFO);
							if (GetMonitorInfo(mon, &mi))
							{
								if (x + cx > mi.rcMonitor.right)
									x = x - cx > mi.rcMonitor.left ? x - cx : mi.rcMonitor.left;

								if (x < mi.rcMonitor.left)
									x = mi.rcMonitor.left;

								if (y + cy > mi.rcMonitor.bottom)
									y = y - cy > mi.rcMonitor.top ? y - cy : mi.rcMonitor.top;

								if (y < mi.rcMonitor.top)
									y = mi.rcMonitor.top;
							}
						}

						SetWindowPos(wndvol, 0, x, y, cx, cy, SWP_NOZORDER);
						ShowWindow(wndvol, SW_SHOWNORMAL);
					}
					return TRUE;
				}
			}
			break;
			case NM_DBLCLK:
			{
				unsigned long part = ((LPNMMOUSE)lp)->dwItemSpec;

				if (part == 0)
					mainmenu_commands::g_execute(cfg_statusdbl.get_value().m_command);
				//standard_commands::main_highlight_playing();
				else if (cfg_show_vol && part == status_bar::u_vol_pos)
				{
					//static_api_ptr_t<ui_control>()->show_preferences(preferences_page::guid_playback);
				}
				else if (cfg_show_seltime && part == status_bar::u_length_pos)
				{
					static_api_ptr_t<playlist_manager>()->activeplaylist_set_selection(bit_array_true(), bit_array_true());
				}

			}
			break;
			}
			break;
		}
		break;
	}
	return uDefWindowProc(wnd, msg, wp, lp);
}
