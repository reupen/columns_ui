#include "stdafx.h"
#include "NG Playlist/ng_playlist.h"
#include "config.h"
#include "prefs_utils.h"

static class tab_display2 : public preferences_tab
{
	static bool initialised;
	static menu_item_cache * p_menu_cache;

	static void refresh_me(HWND wnd)
	{
		SendDlgItemMessage(wnd, IDC_HEADER, BM_SETCHECK, cfg_header, 0);
		//SendDlgItemMessage(wnd,IDC_HORIZ_WHEEL,BM_SETCHECK,cfg_scroll_h_no_v,0);
		SendDlgItemMessage(wnd, IDC_NOHSCROLL, BM_SETCHECK, cfg_nohscroll, 0);
		SendDlgItemMessage(wnd, IDC_ELLIPSIS, BM_SETCHECK, cfg_ellipsis, 0);
		SendDlgItemMessage(wnd, IDC_PLEDGE, CB_SETCURSEL, cfg_frame, 0);

		SendDlgItemMessage(wnd, IDC_INLINE_MODE, BM_SETCHECK, main_window::config_get_inline_metafield_edit_mode() != 0, 0);

		SendDlgItemMessage(wnd, IDC_SELECTION_MODEL, BM_SETCHECK, cfg_alternative_sel, 0);
		//SendDlgItemMessage(wnd,IDC_HORIZ_WHEEL,BM_SETCHECK,cfg_scroll_h_no_v,0);

		SendDlgItemMessage(wnd, IDC_TOOLTIPS, BM_SETCHECK, cfg_tooltip, 0);
		//SendDlgItemMessage(wnd,IDC_SORTSELONLY,BM_SETCHECK,cfg_sortsel,0);
		SendDlgItemMessage(wnd, IDC_TOOLTIPS_CLIPPED, BM_SETCHECK, cfg_tooltips_clipped, 0);
		EnableWindow(GetDlgItem(wnd, IDC_TOOLTIPS_CLIPPED), cfg_tooltip);

		SendDlgItemMessage(wnd, IDC_HHTRACK, BM_SETCHECK, cfg_header_hottrack, 0);

		SendDlgItemMessage(wnd, IDC_SPIN1, UDM_SETPOS32, 0, settings::playlist_view_item_padding);

		SendDlgItemMessage(wnd, IDC_SORT_ARROWS, BM_SETCHECK, cfg_show_sort_arrows, 0);
		SendDlgItemMessage(wnd, IDC_DROP_AT_END, BM_SETCHECK, cfg_drop_at_end, 0);

		SendDlgItemMessage(wnd, IDC_SHOWARTWORK, BM_SETCHECK, pvt::cfg_show_artwork, 0);
		SendDlgItemMessage(wnd, IDC_ARTWORKREFLECTION, BM_SETCHECK, pvt::cfg_artwork_reflection, 0);
		SendDlgItemMessage(wnd, IDC_LOWPRIORITY, BM_SETCHECK, pvt::cfg_artwork_lowpriority, 0);
		SendDlgItemMessage(wnd, IDC_ARTWORKWIDTHSPIN, UDM_SETRANGE32, 0, MAXLONG);
		SendDlgItemMessage(wnd, IDC_ARTWORKWIDTHSPIN, UDM_SETPOS32, NULL, pvt::cfg_artwork_width);

	}

public:
	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{

		switch (msg)
		{
		case WM_INITDIALOG:
		{
			p_menu_cache = new(std::nothrow) menu_item_cache;
			uSendDlgItemMessageText(wnd, IDC_PLEDGE, CB_ADDSTRING, 0, "None");
			uSendDlgItemMessageText(wnd, IDC_PLEDGE, CB_ADDSTRING, 0, "Sunken");
			uSendDlgItemMessageText(wnd, IDC_PLEDGE, CB_ADDSTRING, 0, "Grey");

			SendDlgItemMessage(wnd, IDC_SPIN1, UDM_SETRANGE32, -100, 100);
			//		SendDlgItemMessage(wnd,IDC_SPINPL,UDM_SETRANGE32,-100,100);
			//		SendDlgItemMessage(wnd,IDC_SPINSEL,UDM_SETRANGE32,0,3);

			populate_menu_combo(wnd, IDC_PLAYLIST_DOUBLE, IDC_MENU_DESC, cfg_playlist_double, *p_menu_cache, true);

			unsigned n, count = playlist_mclick_actions::get_count();
			for (n = 0; n < count; n++)
			{
				uSendDlgItemMessageText(wnd, IDC_PLAYLIST_MIDDLE, CB_ADDSTRING, 0, playlist_mclick_actions::g_pma_actions[n].name);
				SendDlgItemMessage(wnd, IDC_PLAYLIST_MIDDLE, CB_SETITEMDATA, n, playlist_mclick_actions::g_pma_actions[n].id);
			}

			SendDlgItemMessage(wnd, IDC_PLAYLIST_MIDDLE, CB_SETCURSEL, playlist_mclick_actions::id_to_idx(cfg_playlist_middle_action), 0);


			refresh_me(wnd);
			initialised = true;
		}

		break;
		case WM_DESTROY:
		{
			initialised = false;
			delete p_menu_cache;
		}
		break;
		case WM_COMMAND:
			switch (wp)
			{

			case IDC_DROP_AT_END:
			{
				cfg_drop_at_end = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
			}
			break;
			case (EN_CHANGE << 16) | IDC_HEIGHT:
			{
				if (initialised)
				{
					BOOL result;
					int new_height = GetDlgItemInt(wnd, IDC_HEIGHT, &result, TRUE);
					if (result) settings::playlist_view_item_padding = new_height;
					refresh_all_playlist_views();
					pvt::ng_playlist_view_t::g_on_vertical_item_padding_change();
				}

			}
			break;
			case (CBN_SELCHANGE << 16) | IDC_PLAYLIST_DOUBLE:
			{
				on_menu_combo_change(wnd, lp, cfg_playlist_double, *p_menu_cache, IDC_MENU_DESC);
			}
			break;
			case (CBN_SELCHANGE << 16) | IDC_PLAYLIST_MIDDLE:
			{
				cfg_playlist_middle_action = SendMessage((HWND)lp, CB_GETITEMDATA, SendMessage((HWND)lp, CB_GETCURSEL, 0, 0), 0);
			}
			break;
			case IDC_TOOLTIPS:
				cfg_tooltip = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				EnableWindow(GetDlgItem(wnd, IDC_TOOLTIPS_CLIPPED), cfg_tooltip);
				pvt::ng_playlist_view_t::g_on_show_tooltips_change();
				break;

			case IDC_SELECTION_MODEL:
				cfg_alternative_sel = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				pvt::ng_playlist_view_t::g_on_alternate_selection_change();
				break;
			case IDC_SHOWARTWORK:
				pvt::cfg_show_artwork = SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != BST_UNCHECKED;
				pvt::ng_playlist_view_t::g_on_show_artwork_change();
				break;
			case IDC_ARTWORKREFLECTION:
				pvt::cfg_artwork_reflection = SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != BST_UNCHECKED;
				pvt::ng_playlist_view_t::g_on_artwork_width_change();
				break;
			case IDC_LOWPRIORITY:
				pvt::cfg_artwork_lowpriority = SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != BST_UNCHECKED;
				break;
			case (EN_CHANGE << 16) | IDC_ARTWORKWIDTH:
				if (initialised)
				{
					pvt::cfg_artwork_width = strtoul_n(string_utf8_from_window((HWND)lp), pfc_infinite);
					pvt::ng_playlist_view_t::g_on_artwork_width_change();
				}
				break;
			case IDC_SORT_ARROWS:
				cfg_show_sort_arrows = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				pvt::ng_playlist_view_t::g_on_show_sort_indicators_change();
				break;
			case IDC_TOOLTIPS_CLIPPED:
				cfg_tooltips_clipped = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				pvt::ng_playlist_view_t::g_on_show_tooltips_change();
				break;

			case IDC_ELLIPSIS:
				cfg_ellipsis = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				break;


			case IDC_HEADER:
			{
				cfg_header = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				unsigned m, pcount = playlist_view::list_playlist.get_count();
				for (m = 0; m < pcount; m++)
				{
					playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
					p_playlist->create_header();
					if (p_playlist->wnd_header) p_playlist->move_header();
					else p_playlist->update_scrollbar();
				}
				pvt::ng_playlist_view_t::g_on_show_header_change();
			}
			break;
			case IDC_NOHSCROLL:
			{
				cfg_nohscroll = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				playlist_view::update_all_windows();
				pvt::ng_playlist_view_t::g_on_autosize_change();
			}
			break;

			case IDC_HHTRACK:
			{
				cfg_header_hottrack = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
				unsigned m, pcount = playlist_view::list_playlist.get_count();
				for (m = 0; m < pcount; m++)
				{
					playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
					long flags = WS_CHILD | WS_VISIBLE | HDS_HOTTRACK | HDS_HORZ | (cfg_nohscroll ? 0 : HDS_FULLDRAG) | (cfg_header_hottrack ? HDS_BUTTONS : 0);
					SetWindowLongPtr(p_playlist->wnd_header, GWL_STYLE, flags);
				}
				pvt::ng_playlist_view_t::g_on_sorting_enabled_change();
			}
			break;
			case (CBN_SELCHANGE << 16) | IDC_PLEDGE:
			{
				cfg_frame = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
				{
					unsigned m, pcount = playlist_view::list_playlist.get_count();
					for (m = 0; m < pcount; m++)
					{
						playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
						if (p_playlist->wnd_playlist)
						{
							long flags = 0;
							if (cfg_frame == 1) flags |= WS_EX_CLIENTEDGE;
							if (cfg_frame == 2) flags |= WS_EX_STATICEDGE;

							SetWindowLongPtr(p_playlist->wnd_playlist, GWL_EXSTYLE, flags);

							SetWindowPos(p_playlist->wnd_playlist, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
							//					move_window_controls();
							//					RedrawWindow(g_test, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
						}
					}
				}
				pvt::ng_playlist_view_t::g_on_edge_style_change();
			}
			break;
			case IDC_INLINE_MODE:
			{
				main_window::config_set_inline_metafield_edit_mode(SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0);
			}
			break;
			}
		}
		return 0;
	}
	HWND create(HWND wnd) override { return uCreateDialog(IDD_DISPLAY2, wnd, ConfigProc); }
	const char * get_name() override { return "General"; }
	bool get_help_url(pfc::string_base & p_out) override
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:playlist_view:general";
		return true;
	}
} g_tab_display2;

menu_item_cache * tab_display2::p_menu_cache = nullptr;
bool tab_display2::initialised = 0;

preferences_tab * g_get_tab_display2()
{
	return &g_tab_display2;
}

