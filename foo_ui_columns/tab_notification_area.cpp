#include "stdafx.h"

static class tab_sys : public preferences_tab
{
public:
	static bool initialised;

	//	static ptr_list_autofree_t<char> status_items;

	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd, IDC_BALLOON, BM_SETCHECK, cfg_balloon, 0);

		uSendDlgItemMessage(wnd, IDC_SHOW_SYSTRAY, BM_SETCHECK, cfg_show_systray, 0);
		//		EnableWindow(GetDlgItem(wnd, IDC_MINIMISE_TO_SYSTRAY), cfg_show_systray);

		uSendDlgItemMessage(wnd, IDC_MINIMISE_TO_SYSTRAY, BM_SETCHECK, cfg_minimise_to_tray, 0);
		uSendDlgItemMessage(wnd, IDC_USE_CUSTOM_ICON, BM_SETCHECK, cfg_custom_icon, 0);
		uSendDlgItemMessage(wnd, IDC_NOWPL, BM_SETCHECK, cfg_np, 0);

		uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_notification_icon_script.get());

	}

	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{

		switch (msg)
		{
		case WM_INITDIALOG:
		{
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
				main_window::config_notification_icon_script.set(string_utf8_from_window((HWND)lp));
				break;

			case IDC_NOWPL:
			{
				cfg_np = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			}
			break;
			case IDC_USE_CUSTOM_ICON:
			{
				cfg_custom_icon = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				create_icon_handle(); create_systray_icon();
			}
			break;
			case IDC_BROWSE_ICON:
			{
				pfc::string8 path = cfg_tray_icon_path;
				if (uGetOpenFileName(wnd, "Icon Files (*.ico)|*.ico|All Files (*.*)|*.*", 0, "ico", "Choose Icon", NULL, path, FALSE))
				{
					cfg_tray_icon_path = path;
					if (cfg_custom_icon) { create_icon_handle(); create_systray_icon(); }
				}
			}
			break;


			case IDC_MINIMISE_TO_SYSTRAY:
			{
				cfg_minimise_to_tray = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			}
			break;
			case IDC_SHOW_SYSTRAY:
			{
				cfg_show_systray = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				//				EnableWindow(GetDlgItem(wnd, IDC_MINIMISE_TO_SYSTRAY), cfg_show_systray);

				if (g_main_window)
				{
					if (cfg_show_systray && !g_icon_created) create_systray_icon();
					else if (!cfg_show_systray && g_icon_created && (!IsIconic(g_main_window) || !cfg_minimise_to_tray)) destroy_systray_icon();
					if (g_status) update_systray();
				}
			}
			break;
			case IDC_BALLOON:
			{
				cfg_balloon = uSendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			}
			break;
			}
		}
		return 0;
	}
	virtual HWND create(HWND wnd) { return uCreateDialog(IDD_SYS, wnd, ConfigProc); }
	virtual const char * get_name() { return "Notification area"; }
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:notification_area";
		return true;
	}
} g_tab_sys;

bool tab_sys::initialised = 0;

preferences_tab * g_get_tab_sys()
{
	return &g_tab_sys;
}

