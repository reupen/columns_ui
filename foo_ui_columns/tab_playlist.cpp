#include "stdafx.h"

static class tab_playlist : public preferences_tab
{
	static bool initialised, playlist_switcher_string_changed;

	static void refresh_me(HWND wnd)
	{
		SendDlgItemMessage(wnd, IDC_MCLICK, BM_SETCHECK, cfg_mclick, 0);
		//SendDlgItemMessage(wnd,IDC_MCLICK2,BM_SETCHECK,cfg_mclick2,0);
		SendDlgItemMessage(wnd, IDC_MCLICK3, BM_SETCHECK, cfg_plm_rename, 0);
		//SendDlgItemMessage(wnd,IDC_SHIFT_LMB,BM_SETCHECK,cfg_playlists_shift_lmb,0);
		//SendDlgItemMessage(wnd,IDC_DELETE,BM_SETCHECK,cfg_playlist_panel_delete,0);
		//SendDlgItemMessage(wnd,IDC_TABS,BM_SETCHECK,cfg_tabs,0);
		//SendDlgItemMessage(wnd,IDC_AUTOSWITCH,BM_SETCHECK,cfg_drag_autoswitch,0);
		SendDlgItemMessage(wnd, IDC_PLISTEDGE, CB_SETCURSEL, cfg_plistframe, 0);
		SendDlgItemMessage(wnd, IDC_PLDRAG, BM_SETCHECK, cfg_drag_pl, 0);
		SendDlgItemMessage(wnd, IDC_PLAUTOHIDE, BM_SETCHECK, cfg_pl_autohide, 0);

		SendDlgItemMessage(wnd, IDC_SPINPL, UDM_SETPOS32, 0, settings::playlist_switcher_item_padding);
		SendDlgItemMessage(wnd, IDC_TABS_MULTILINE, BM_SETCHECK, cfg_tabs_multiline, 0);
		SendDlgItemMessage(wnd, IDC_SIDEBAR_TOOLTIPS, BM_SETCHECK, cfg_playlist_sidebar_tooltips, 0);
		SendDlgItemMessage(wnd, IDC_USE_PLAYLIST_TF, BM_SETCHECK, cfg_playlist_switcher_use_tagz, 0);
		uSendDlgItemMessageText(wnd, IDC_PLAYLIST_TF, WM_SETTEXT, 0, cfg_playlist_switcher_tagz);


	}

public:
	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{

		switch (msg)
		{
		case WM_INITDIALOG:
		{
			uSendDlgItemMessageText(wnd, IDC_PLISTEDGE, CB_ADDSTRING, 0, "None");
			uSendDlgItemMessageText(wnd, IDC_PLISTEDGE, CB_ADDSTRING, 0, "Sunken");
			uSendDlgItemMessageText(wnd, IDC_PLISTEDGE, CB_ADDSTRING, 0, "Grey");

			SendDlgItemMessage(wnd, IDC_SPINPL, UDM_SETRANGE32, -100, 100);
			SendDlgItemMessage(wnd, IDC_SWITCH_SPIN, UDM_SETRANGE32, 0, 10000);

			refresh_me(wnd);
			initialised = true;
		}

		break;
		case WM_DESTROY:
		{
			initialised = false;
			SendMessage(wnd, WM_COMMAND, IDC_APPLY, 0);

		}
		break;
		case WM_COMMAND:
			switch (wp)
			{

			case (EN_CHANGE << 16) | IDC_PLHEIGHT:
			{
				if (initialised)
				{
					BOOL result;
					int new_height = GetDlgItemInt(wnd, IDC_PLHEIGHT, &result, TRUE);
					if (result) settings::playlist_switcher_item_padding = new_height;
					//						if (g_plist) SendMessage(g_plist, LB_SETITEMHEIGHT, 0, get_pl_item_height());
					playlist_switcher_t::g_on_vertical_item_padding_change();
				}

			}
			break;
			case IDC_APPLY:
				if (playlist_switcher_string_changed)
				{
					playlist_switcher_t::g_refresh_all_items();
					playlist_switcher_string_changed = false;
				}
				break;

			case IDC_PLAUTOHIDE:
			{
				cfg_pl_autohide = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				g_on_autohide_tabs_change();
				//					if (g_main_window)
				//					{
				//						bool move = false;
				//					if (create_plist()) move = true;
				//						if (create_tabs()) move = true; 
				//						if (move) {move_window_controls();RedrawWindow(g_main_window, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);}
				//					}
			}
			break;
			case IDC_MCLICK:
				cfg_mclick = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				break;
				//case IDC_SHIFT_LMB:
				//		cfg_playlists_shift_lmb = SendMessage((HWND)lp,BM_GETCHECK,0,0);
				//		break;
				//case IDC_DELETE:
				//cfg_playlist_panel_delete = SendMessage((HWND)lp,BM_GETCHECK,0,0);
				//break;
			case (EN_CHANGE << 16) | IDC_PLAYLIST_TF:
				cfg_playlist_switcher_tagz = string_utf8_from_window((HWND)lp);
				playlist_switcher_string_changed = true;
				break;
			case IDC_SIDEBAR_TOOLTIPS:
				cfg_playlist_sidebar_tooltips = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				break;
			case IDC_USE_PLAYLIST_TF:
				cfg_playlist_switcher_use_tagz = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				playlist_switcher_t::g_refresh_all_items();
				playlist_switcher_string_changed = false;
				break;
			case IDC_TABS_MULTILINE:
			{
				cfg_tabs_multiline = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				g_on_multiline_tabs_change();
#if 0
				if (g_main_window && g_tab)
				{
					//		create_tabs();
					long flags = WS_CHILD | TCS_HOTTRACK | TCS_TABS | (cfg_tabs_multiline ? TCS_MULTILINE : TCS_SINGLELINE) | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_SINGLELINE;

					SetWindowLongPtr(g_tab, GWL_STYLE, flags);
					move_window_controls();
				}
#endif
			}
			break;
			case IDC_MCLICK3:
			{
				cfg_plm_rename = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			}
			break;
			case IDC_PLDRAG:
			{
				cfg_drag_pl = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			}
			break;
#if 0
			case IDC_MCLICK2:
			{
				cfg_mclick2 = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			}
			break;
			case IDC_TABS:
			{
				cfg_tabs = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				if (g_main_window)
				{
					create_tabs();
					move_window_controls();
				}
			}
			break;
#endif
			case (CBN_SELCHANGE << 16) | IDC_PLISTEDGE:
			{
				cfg_plistframe = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
				playlist_switcher_t::g_on_edgestyle_change();
			}
			break;

			}
		}
		return 0;
	}
	virtual HWND create(HWND wnd) { return uCreateDialog(IDD_PLAYLISTS, wnd, ConfigProc); }
	virtual const char * get_name() { return "General"; }
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:playlist_switcher:general";
		return true;
	}
} g_tab_playlist;

bool tab_playlist::initialised = 0;
bool tab_playlist::playlist_switcher_string_changed = 0;

preferences_tab * g_get_tab_playlist()
{
	return &g_tab_playlist;
}

