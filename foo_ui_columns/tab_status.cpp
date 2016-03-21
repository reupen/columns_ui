#include "stdafx.h"

static class tab_status : public preferences_tab
{
public:
	static bool initialised;
	static menu_item_cache * p_cache;

	//	static ptr_list_autofree_t<char> status_items;

	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd, IDC_VOL, BM_SETCHECK, cfg_show_vol, 0);
		uSendDlgItemMessage(wnd, IDC_SELTIME, BM_SETCHECK, cfg_show_seltime, 0);
		uSendDlgItemMessage(wnd, IDC_SHOW_STATUS, BM_SETCHECK, cfg_status, 0);
		uSendDlgItemMessage(wnd, IDC_SHOW_STATUSPANE, BM_SETCHECK, settings::show_status_pane, 0);
		uSendDlgItemMessage(wnd, IDC_SHOW_LOCK, BM_SETCHECK, main_window::config_get_status_show_lock(), 0);

		uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_status_bar_script.get());
	}

	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{

		switch (msg)
		{
		case WM_INITDIALOG:
		{
			p_cache = new menu_item_cache;

			populate_menu_combo(wnd, IDC_MENU_DBLCLK, IDC_MENU_DESC, cfg_statusdbl, *p_cache, false);

			refresh_me(wnd);
			initialised = true;
		}

		break;
		case WM_DESTROY:
		{
			delete p_cache;
			p_cache = 0;
			//				status_items.free_all();
			initialised = false;
		}
		break;
		case WM_COMMAND:
			switch (wp)
			{



			case (EN_CHANGE << 16) | IDC_STRING:
				main_window::config_status_bar_script.set(string_utf8_from_window((HWND)lp));
				break;
			case (CBN_SELCHANGE << 16) | IDC_MENU_DBLCLK:
			{
				on_menu_combo_change(wnd, lp, cfg_statusdbl, *p_cache, IDC_MENU_DESC);
			}
			break;
			case IDC_STATUS_FONT:
			{
				LOGFONT temp = cfg_status_font;
				if (font_picker(temp, wnd))
				{
					cfg_status_font = temp;
					on_status_font_change();
				}
			}
			break;
			case IDC_SHOW_LOCK:
			{
				main_window::config_set_status_show_lock(SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0);
			}
			break;
			case IDC_VOL:
			{
				cfg_show_vol = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				status_bar::set_part_sizes(status_bar::t_part_volume);
			}
			break;
			case IDC_SELTIME:
			{
				cfg_show_seltime = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				status_bar::set_part_sizes(status_bar::t_part_length | status_bar::t_part_volume);

			}
			break;
			case IDC_SHOW_STATUS:
			{
				cfg_status = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				on_show_status_change();
			}
			break;
			case IDC_SHOW_STATUSPANE:
			{
				settings::show_status_pane = SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0;
				on_show_status_pane_change();
			}
			break;
			}
		}
		return 0;
	}
	virtual HWND create(HWND wnd) { return uCreateDialog(IDD_STATUS, wnd, ConfigProc); }
	virtual const char * get_name() { return "Status bar"; }
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:status_bar";
		return true;
	}
} g_tab_status;

menu_item_cache * tab_status::p_cache = 0;
bool tab_status::initialised = 0;

preferences_tab * g_get_tab_status()
{
	return &g_tab_status;
}

