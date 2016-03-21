#include "stdafx.h"

static class tab_main : public preferences_tab
{
public:
	static bool initialised;

	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd, IDC_IMPORT_TITLES, BM_SETCHECK, cfg_import_titles, 0);
		//uSendDlgItemMessage(wnd,IDC_EXPORT_TITLES,BM_SETCHECK,cfg_export_titles,0);

		uSendDlgItemMessage(wnd, IDC_TOOLBARS, BM_SETCHECK, cfg_toolbars, 0);
		//uSendDlgItemMessage(wnd,IDC_KEYB,BM_SETCHECK,config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, true),0);
		uSendDlgItemMessage(wnd, IDC_USE_TRANSPARENCY, BM_SETCHECK, main_window::config_get_transparency_enabled(), 0);
		uSendDlgItemMessage(wnd, IDC_TRANSPARENCY_SPIN, UDM_SETPOS32, 0, main_window::config_get_transparency_level());

		if (!g_main_window)
			EnableWindow(GetDlgItem(wnd, IDC_QUICKSETUP), FALSE);

		uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_main_window_title_script.get());
	}

	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{

		switch (msg)
		{
		case WM_INITDIALOG:
		{
			HWND wnd_lv = GetDlgItem(wnd, IDC_LIBRARIES);
			uih::SetListViewWindowExplorerTheme(wnd_lv);
			//SetWindowLongPtr(wnd_lv, GWL_EXSTYLE, GetWindowLongPtr(wnd_lv, GWL_EXSTYLE)|LVS_EX_INFOTIP );

			listview_helper::insert_column(wnd_lv, 0, "Library", 50);
			listview_helper::insert_column(wnd_lv, 1, "Version", 70);
			listview_helper::insert_column(wnd_lv, 2, "Extended Info", 150);

			uSendDlgItemMessage(wnd, IDC_TRANSPARENCY_SPIN, UDM_SETRANGE32, 0, 255);

			refresh_me(wnd);
			initialised = true;
		}

		break;
		case WM_DESTROY:
		{
			initialised = false;
		}
		break;
		case WM_COMMAND:
			switch (wp)
			{
			case (EN_CHANGE << 16) | IDC_STRING:
				main_window::config_main_window_title_script.set(string_utf8_from_window((HWND)lp));
				break;

			case IDC_QUICKSETUP:
				SendMessage(g_main_window, MSG_RUN_INITIAL_SETUP, NULL, NULL);
				break;


			case IDC_POPULATE:
			{
				HWND wndlib = uCreateDialog(IDD_LIBRARIES, wnd, LibrariesProc);
				ShowWindow(wndlib, SW_SHOWNORMAL);
			}
			break;

			/*case IDC_EXPORT:
			{
			refresh_config_columns();
			export(wnd);
			config_columns.delete_all();
			//					uDialogBox(IDD_EXPORT,wnd,ExportProc,0);
			}
			break;*/
			case IDC_IMPORT:
			{
				import(wnd);
				//					refresh_me(wnd);
			}
			break;
			case IDC_FCL_EXPORT:
				g_export_layout(wnd);
				break;
			case IDC_FCL_IMPORT:
				g_import_layout(wnd);
				break;
			case IDC_IMPORT_TITLES:
			{
				cfg_import_titles = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			}
			break;
			/*case IDC_EXPORT_TITLES:
			{
			cfg_export_titles = SendMessage((HWND)lp,BM_GETCHECK,0,0);
			}
			break;*/
			case (EN_CHANGE << 16) | IDC_TRANSPARENCY_LEVEL:
			{
				if (initialised)
				{
					BOOL result;
					unsigned new_val = GetDlgItemInt(wnd, IDC_TRANSPARENCY_LEVEL, &result, FALSE);
					if (result)
					{
						main_window::config_set_transparency_level((unsigned char)new_val);
					}
				}

			}
			break;
			case IDC_USE_TRANSPARENCY:
				main_window::config_set_transparency_enabled(SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0);
				break;
			case IDC_TOOLBARS:
				cfg_toolbars = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				on_show_toolbars_change();
				break;
			case IDC_RESET_TOOLBARS:
			{
				if (win32_helpers::message_box(wnd, _T("Warning! This will reset the toolbars to the default state. Continue?"), _T("Reset toolbars?"), MB_YESNO) == IDYES)
				{
					extern cfg_rebar g_cfg_rebar;

					if (g_main_window) destroy_rebar();
					g_cfg_rebar.reset();
					if (g_main_window)
					{
						create_rebar();
						if (g_rebar)
						{
							ShowWindow(g_rebar, SW_SHOWNORMAL);
							UpdateWindow(g_rebar);
						}
						size_windows();
					}
				}
			}
			break;/*
				  case IDC_KEYB:
				  {
				  config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, true) = SendMessage((HWND)lp,BM_GETCHECK,0,0);
				  }
				  break;*/
			}
		}
		return 0;
	}
	static BOOL CALLBACK LibrariesProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{

		switch (msg)
		{
		case WM_DESTROY:
			modeless_dialog_manager::g_remove(wnd);
			break;
		case WM_COMMAND:
			switch (wp)
			{
			case IDOK:
			case IDCANCEL:
			{
				DestroyWindow(wnd);
			}
			break;
			}
		case WM_INITDIALOG:
		{
			modeless_dialog_manager::g_add(wnd);
			HWND wnd_lv = GetDlgItem(wnd, IDC_LIBRARIES);
			uih::SetListViewWindowExplorerTheme(wnd_lv);
			//SetWindowLongPtr(wnd_lv, GWL_EXSTYLE, GetWindowLongPtr(wnd_lv, GWL_EXSTYLE)|LVS_EX_INFOTIP );

			listview_helper::insert_column(wnd_lv, 0, "Library", 50);
			listview_helper::insert_column(wnd_lv, 1, "Version", 70);
			listview_helper::insert_column(wnd_lv, 2, "Extended Info", 150);


			{
				DLLVERSIONINFO2 dvi;
				pfc::string8 path;
				HRESULT hr = uih::GetComCtl32Version(dvi, &path);

				pfc::string8 temp;

				if (FAILED(hr))
					temp = "4.70";
				else if (dvi.info1.cbSize == sizeof(DLLVERSIONINFO2))
				{
					unsigned short * p_ver = (unsigned short *)&dvi.ullVersion;
					temp = uStringPrintf("%u.%u.%u.%u", p_ver[3], p_ver[2], p_ver[1], p_ver[0]);
				}
				else
					temp = uStringPrintf("%u.%u.%u", dvi.info1.dwMajorVersion, dvi.info1.dwMinorVersion, dvi.info1.dwBuildNumber);

				listview_helper::insert_item(wnd_lv, 0, "comctl32", 0);
				listview_helper::set_item_text(wnd_lv, 0, 1, temp);
				listview_helper::set_item_text(wnd_lv, 0, 2, uStringPrintf("Path: %s", path.get_ptr()));
			}


		}
		break;
		}
		return 0;
	}
	virtual HWND create(HWND wnd) { return uCreateDialog(IDD_MAIN, wnd, ConfigProc); }
	virtual const char * get_name() { return "Main"; }
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:main";
		return true;
	}
} g_tab_main;

bool tab_main::initialised = 0;

preferences_tab * g_get_tab_main()
{
	return &g_tab_main;
}

