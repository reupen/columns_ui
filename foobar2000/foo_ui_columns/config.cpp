#include "foo_ui_columns.h"

cfg_struct_t<LOGFONT> cfg_editor_font(create_guid(0xd429d322,0xd236,0x7356,0x33,0x25,0x4b,0x67,0xc5,0xd4,0x50,0x3e),get_menu_font());
cfg_int cfg_import_titles(create_guid(0xcd062463,0x488f,0xc7ec,0x56,0xf2,0x90,0x7f,0x0a,0xfe,0x77,0xda),1);
cfg_int cfg_export_titles(create_guid(0x96094997,0xbf50,0x202d,0x98,0x01,0xfc,0x02,0xf3,0x94,0x30,0x63),0);
cfg_int cfg_child(create_guid(0xf20b83d0,0x5890,0xba6f,0xe8,0x62,0x69,0x30,0xe2,0x6b,0xc8,0x1c),0);
cfg_int cfg_child_panels(create_guid(0x1a8d8760,0x4f60,0x4800,0x93,0x81,0x32,0x32,0x66,0xa0,0x6c,0xff),0);
cfg_int cfg_child_playlist(create_guid(0xbc6c99d4,0x51c1,0xf76e,0x10,0x9c,0x62,0x92,0x92,0xbd,0xbd,0xb2),0);

void refresh_all_playlist_views()
{
	if (playlist_view::g_get_cache().is_active())
	{
		g_to_global.release();
		g_to_global_colour.release();
		playlist_view::g_reset_columns();
		unsigned m, pcount = playlist_view::list_playlist.get_count();
		for (m=0; m<pcount; m++)
		{
			playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
			p_playlist->create_header();
			if (p_playlist->wnd_header)
				p_playlist->move_header();
		}
		playlist_view::update_all_windows();
	}
}

void preview_to_console(const char * spec, bool extra)
{
	static_api_ptr_t<playlist_manager> playlist_api;

	int count = playlist_api->activeplaylist_get_item_count();

	if (!count) popup_message::g_show("Activate a non-empty playlist and try again","No track to preview");
	else
	{
		int idx = playlist_api->activeplaylist_get_focus_item();
		if (idx >= count) idx = count-1;
		if (idx <0) idx =0;

		pfc::string8 temp;

		bool b_legacy = cfg_oldglobal != 0;
		bool b_date = cfg_playlist_date != 0;
		SYSTEMTIME st;
		if (b_date) GetLocalTime(&st);

		global_variable_list extra_items;
		playlist_view::g_get_cache().active_make_extra(idx, extra_items, b_date ? &st : 0, b_legacy);
		service_ptr_t<titleformat_object> to_temp;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_temp, spec);

		//0.9 fallout
		playlist_api->activeplaylist_item_format_title(idx, &titleformat_hook_impl_splitter(extra ? &titleformat_hook_set_global<false,true>(extra_items, b_legacy) : 0, b_date ? &titleformat_hook_date(&st) : 0), temp, to_temp, 0, play_control::display_level_all);
	//	if (map) temp.replace_char(6, 3);
		popup_message::g_show(temp,pfc::string8() << "Preview of track "<< (idx+1));
	}
	//console::popup();
}

void colour_code_gen(HWND parent, UINT edit, bool markers, bool init)
{
	COLORREF COLOR = g_last_colour;
	COLORREF COLORS[16] = {g_last_colour,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	if (init || uChooseColor(&COLOR, parent, &COLORS[0]))
	{
		g_last_colour = COLOR;

		pfc::string_formatter text;
		text << "$rgb(" << unsigned(COLOR & 0xff) << "," << unsigned(COLOR>>8 & 0xff) << "," << unsigned(COLOR>>16 & 0xff) << ")";
		
		uSendDlgItemMessageText(parent, edit, WM_SETTEXT, 0, text);
	}
}

bool colour_picker(HWND wnd, cfg_int & out, COLORREF custom)
{
	bool rv = false;
	COLORREF COLOR = out;
	COLORREF COLORS[16] = {custom,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	if (uChooseColor(&COLOR, wnd, &COLORS[0]))
	{
		out = COLOR;
		rv = true;
	}
	return rv;
}

bool colour_picker(HWND wnd, COLORREF & out, COLORREF custom)
{
	bool rv = false;
	COLORREF COLOR = out;
	COLORREF COLORS[16] = {custom,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	if (uChooseColor(&COLOR, wnd, &COLORS[0]))
	{
		out = COLOR;
		rv = true;
	}
	return rv;
}

bool colour_picker2(HWND wnd, config_item_t<COLORREF> & p_out, COLORREF custom)
{
	bool rv = false;
	COLORREF COLOR = p_out.get();
	COLORREF COLORS[16] = {custom,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	if (uChooseColor(&COLOR, wnd, &COLORS[0]))
	{
		p_out.set(COLOR);
		rv = true;
	}
	return rv;
}

bool font_picker(HWND wnd, cfg_struct_t<LOGFONT> & out)
{
	bool rv = false;
	LOGFONT temp = out;
	if (font_picker(temp,wnd))
	{
		out = temp;
		rv = true;
	}
	return rv;
}



static class tab_display2 : public preferences_tab
{
	static bool initialised;
	static menu_item_cache * p_menu_cache;
	
	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd,IDC_HEADER,BM_SETCHECK,cfg_header,0);
		//uSendDlgItemMessage(wnd,IDC_HORIZ_WHEEL,BM_SETCHECK,cfg_scroll_h_no_v,0);
		uSendDlgItemMessage(wnd,IDC_NOHSCROLL,BM_SETCHECK,cfg_nohscroll,0);
		uSendDlgItemMessage(wnd,IDC_ELLIPSIS,BM_SETCHECK,cfg_ellipsis,0);
		uSendDlgItemMessage(wnd,IDC_PLEDGE,CB_SETCURSEL,cfg_frame,0);

		uSendDlgItemMessage(wnd,IDC_INLINE_MODE,BM_SETCHECK,main_window::config_get_inline_metafield_edit_mode()!=0,0);

		uSendDlgItemMessage(wnd,IDC_SELECTION_MODEL,BM_SETCHECK,cfg_alternative_sel,0);
		//uSendDlgItemMessage(wnd,IDC_HORIZ_WHEEL,BM_SETCHECK,cfg_scroll_h_no_v,0);
		
		uSendDlgItemMessage(wnd,IDC_TOOLTIPS,BM_SETCHECK,cfg_tooltip,0);
		//uSendDlgItemMessage(wnd,IDC_SORTSELONLY,BM_SETCHECK,cfg_sortsel,0);
		uSendDlgItemMessage(wnd,IDC_TOOLTIPS_CLIPPED,BM_SETCHECK,cfg_tooltips_clipped,0);
		EnableWindow(GetDlgItem(wnd, IDC_TOOLTIPS_CLIPPED), cfg_tooltip);
		
		uSendDlgItemMessage(wnd,IDC_HHTRACK,BM_SETCHECK,cfg_header_hottrack,0);
		
		uSendDlgItemMessage(wnd,IDC_SPIN1,UDM_SETPOS32,0,cfg_height);

		uSendDlgItemMessage(wnd,IDC_SORT_ARROWS,BM_SETCHECK,cfg_show_sort_arrows,0);
		uSendDlgItemMessage(wnd,IDC_DROP_AT_END,BM_SETCHECK,cfg_drop_at_end,0);

		uSendDlgItemMessage(wnd,IDC_SHOWARTWORK,BM_SETCHECK,pvt::cfg_show_artwork,0);
		uSendDlgItemMessage(wnd,IDC_ARTWORKREFLECTION,BM_SETCHECK,pvt::cfg_artwork_reflection,0);
		uSendDlgItemMessage(wnd,IDC_LOWPRIORITY,BM_SETCHECK,pvt::cfg_artwork_lowpriority,0);
		uSendDlgItemMessage(wnd,IDC_ARTWORKWIDTHSPIN,UDM_SETRANGE32,0,MAXLONG);
		uSendDlgItemMessage(wnd,IDC_ARTWORKWIDTHSPIN, UDM_SETPOS32, NULL, pvt::cfg_artwork_width);
		
	}
	
public:
	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				p_menu_cache = new(std::nothrow) menu_item_cache;
				uSendDlgItemMessageText(wnd,IDC_PLEDGE,CB_ADDSTRING,0,"None");
				uSendDlgItemMessageText(wnd,IDC_PLEDGE,CB_ADDSTRING,0,"Sunken");
				uSendDlgItemMessageText(wnd,IDC_PLEDGE,CB_ADDSTRING,0,"Grey");

				uSendDlgItemMessage(wnd,IDC_SPIN1,UDM_SETRANGE32,-100,100);
		//		uSendDlgItemMessage(wnd,IDC_SPINPL,UDM_SETRANGE32,-100,100);
		//		uSendDlgItemMessage(wnd,IDC_SPINSEL,UDM_SETRANGE32,0,3);

				populate_menu_combo(wnd, IDC_PLAYLIST_DOUBLE, IDC_MENU_DESC, cfg_playlist_double, *p_menu_cache, true);

				unsigned n, count = playlist_mclick_actions::get_count();
				for (n=0;n<count;n++)
				{
					uSendDlgItemMessageText(wnd,IDC_PLAYLIST_MIDDLE,CB_ADDSTRING,0,playlist_mclick_actions::g_pma_actions[n].name);
					uSendDlgItemMessage(wnd,IDC_PLAYLIST_MIDDLE, CB_SETITEMDATA,n,playlist_mclick_actions::g_pma_actions[n].id);
				}

				uSendDlgItemMessage(wnd, IDC_PLAYLIST_MIDDLE,CB_SETCURSEL,playlist_mclick_actions::id_to_idx(cfg_playlist_middle_action),0);


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
			switch(wp)
			{
				
			case IDC_DROP_AT_END:
				{
					cfg_drop_at_end = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
			case (EN_CHANGE<<16)|IDC_HEIGHT:
				{
					if (initialised)
					{
						BOOL result;
						int new_height = GetDlgItemInt(wnd, IDC_HEIGHT, &result, TRUE);
						if (result) cfg_height = new_height;
						refresh_all_playlist_views();
						pvt::ng_playlist_view_t::g_on_vertical_item_padding_change();
					}
					
				}
				break;
			case (CBN_SELCHANGE<<16)|IDC_PLAYLIST_DOUBLE:
				{
						on_menu_combo_change(wnd, lp, cfg_playlist_double, *p_menu_cache, IDC_MENU_DESC);
				}
				break;
			case (CBN_SELCHANGE<<16)|IDC_PLAYLIST_MIDDLE:
				{
						cfg_playlist_middle_action = uSendMessage((HWND)lp,CB_GETITEMDATA,uSendMessage((HWND)lp,CB_GETCURSEL,0,0),0);
				}
				break;
			case IDC_TOOLTIPS:
				cfg_tooltip = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				EnableWindow(GetDlgItem(wnd, IDC_TOOLTIPS_CLIPPED), cfg_tooltip);
				pvt::ng_playlist_view_t::g_on_show_tooltips_change();
				break;

			case IDC_SELECTION_MODEL:
				cfg_alternative_sel = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				pvt::ng_playlist_view_t::g_on_alternate_selection_change();
				break;
			case IDC_SHOWARTWORK:
				pvt::cfg_show_artwork = SendMessage((HWND)lp,BM_GETCHECK,0,0) != BST_UNCHECKED;
				pvt::ng_playlist_view_t::g_on_show_artwork_change();
				break;
			case IDC_ARTWORKREFLECTION:
				pvt::cfg_artwork_reflection = SendMessage((HWND)lp,BM_GETCHECK,0,0) != BST_UNCHECKED;
				pvt::ng_playlist_view_t::g_on_artwork_width_change();
				break;
			case IDC_LOWPRIORITY:
				pvt::cfg_artwork_lowpriority = SendMessage((HWND)lp,BM_GETCHECK,0,0) != BST_UNCHECKED;
				break;
			case (EN_CHANGE<<16)|IDC_ARTWORKWIDTH:
				if (initialised)
				{
					pvt::cfg_artwork_width = strtoul_n(string_utf8_from_window((HWND)lp), pfc_infinite);
					pvt::ng_playlist_view_t::g_on_artwork_width_change();
				}
				break;
			case IDC_SORT_ARROWS:
				cfg_show_sort_arrows = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				pvt::ng_playlist_view_t::g_on_show_sort_indicators_change();
				break;
			case IDC_TOOLTIPS_CLIPPED:
				cfg_tooltips_clipped = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				pvt::ng_playlist_view_t::g_on_show_tooltips_change();
				break;
				
			case IDC_ELLIPSIS:
				cfg_ellipsis = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
				

			case IDC_HEADER:
				{
					cfg_header = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					unsigned m, pcount = playlist_view::list_playlist.get_count();
					for (m=0; m<pcount; m++)
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
					cfg_nohscroll = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					playlist_view::update_all_windows();
					pvt::ng_playlist_view_t::g_on_autosize_change();
				}
				break;
				
			case IDC_HHTRACK:
				{
					cfg_header_hottrack = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					unsigned m, pcount = playlist_view::list_playlist.get_count();
					for (m=0; m<pcount; m++)
					{
						playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
						long flags = WS_CHILD | WS_VISIBLE |  HDS_HOTTRACK | HDS_HORZ | (cfg_nohscroll ? 0 : HDS_FULLDRAG) | (cfg_header_hottrack ? HDS_BUTTONS : 0);
						uSetWindowLong(p_playlist->wnd_header, GWL_STYLE, flags);
					}
					pvt::ng_playlist_view_t::g_on_sorting_enabled_change();
				}
				break;
			case (CBN_SELCHANGE<<16)|IDC_PLEDGE:
				{
					cfg_frame = uSendMessage((HWND)lp,CB_GETCURSEL,0,0);
					{
						unsigned m, pcount = playlist_view::list_playlist.get_count();
						for (m=0; m<pcount; m++)
						{
							playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
							if (p_playlist->wnd_playlist)
							{
								long flags = 0;
								if (cfg_frame == 1) flags |= WS_EX_CLIENTEDGE;
								if (cfg_frame == 2) flags |= WS_EX_STATICEDGE;
								
								uSetWindowLong(p_playlist->wnd_playlist, GWL_EXSTYLE, flags);
								
								SetWindowPos(p_playlist->wnd_playlist,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
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
					main_window::config_set_inline_metafield_edit_mode(SendMessage((HWND)lp,BM_GETCHECK,0,0)!=0);
				}
				break;
			}
		}
		return 0;
	}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_DISPLAY2,wnd,ConfigProc);}
	virtual const char * get_name() {return "General";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:playlist_view:general";
		return true;
	}
} g_tab_display2;

menu_item_cache * tab_display2::p_menu_cache = 0;



static class tab_playlist : public preferences_tab
{
	static bool initialised, playlist_switcher_string_changed;
	
	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd,IDC_MCLICK,BM_SETCHECK,cfg_mclick,0);
		//uSendDlgItemMessage(wnd,IDC_MCLICK2,BM_SETCHECK,cfg_mclick2,0);
		uSendDlgItemMessage(wnd,IDC_MCLICK3,BM_SETCHECK,cfg_plm_rename,0);
		//uSendDlgItemMessage(wnd,IDC_SHIFT_LMB,BM_SETCHECK,cfg_playlists_shift_lmb,0);
		//uSendDlgItemMessage(wnd,IDC_DELETE,BM_SETCHECK,cfg_playlist_panel_delete,0);
		//uSendDlgItemMessage(wnd,IDC_TABS,BM_SETCHECK,cfg_tabs,0);
		//uSendDlgItemMessage(wnd,IDC_AUTOSWITCH,BM_SETCHECK,cfg_drag_autoswitch,0);
		uSendDlgItemMessage(wnd,IDC_PLISTEDGE,CB_SETCURSEL,cfg_plistframe,0);
		uSendDlgItemMessage(wnd,IDC_PLDRAG,BM_SETCHECK,cfg_drag_pl,0);
		uSendDlgItemMessage(wnd,IDC_PLAUTOHIDE,BM_SETCHECK,cfg_pl_autohide,0);

		uSendDlgItemMessage(wnd,IDC_SPINPL,UDM_SETPOS32,0,cfg_plheight);
		uSendDlgItemMessage(wnd,IDC_TABS_MULTILINE,BM_SETCHECK,cfg_tabs_multiline,0);
		uSendDlgItemMessage(wnd,IDC_SIDEBAR_TOOLTIPS,BM_SETCHECK,cfg_playlist_sidebar_tooltips,0);
		uSendDlgItemMessage(wnd,IDC_USE_PLAYLIST_TF,BM_SETCHECK,cfg_playlist_switcher_use_tagz,0);
		uSendDlgItemMessageText(wnd,IDC_PLAYLIST_TF,WM_SETTEXT,0,cfg_playlist_switcher_tagz);
		
		
	}
	
public:
	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				uSendDlgItemMessageText(wnd,IDC_PLISTEDGE,CB_ADDSTRING,0,"None");
				uSendDlgItemMessageText(wnd,IDC_PLISTEDGE,CB_ADDSTRING,0,"Sunken");
				uSendDlgItemMessageText(wnd,IDC_PLISTEDGE,CB_ADDSTRING,0,"Grey");
				
				uSendDlgItemMessage(wnd,IDC_SPINPL,UDM_SETRANGE32,-100,100);
				uSendDlgItemMessage(wnd,IDC_SWITCH_SPIN,UDM_SETRANGE32,0,10000);

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
			switch(wp)
			{
				
			case (EN_CHANGE<<16)|IDC_PLHEIGHT:
				{
					if (initialised)
					{
						BOOL result;
						int new_height = GetDlgItemInt(wnd, IDC_PLHEIGHT, &result, TRUE);
						if (result) cfg_plheight = new_height;
//						if (g_plist) uSendMessage(g_plist, LB_SETITEMHEIGHT, 0, get_pl_item_height());
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
					cfg_pl_autohide = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
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
				cfg_mclick = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
			//case IDC_SHIFT_LMB:
			//		cfg_playlists_shift_lmb = SendMessage((HWND)lp,BM_GETCHECK,0,0);
			//		break;
			//case IDC_DELETE:
					//cfg_playlist_panel_delete = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				//break;
			case (EN_CHANGE<<16)|IDC_PLAYLIST_TF:
					cfg_playlist_switcher_tagz = string_utf8_from_window((HWND)lp);
					playlist_switcher_string_changed = true;
				break;
			case IDC_SIDEBAR_TOOLTIPS:
					cfg_playlist_sidebar_tooltips = SendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
			case IDC_USE_PLAYLIST_TF:
					cfg_playlist_switcher_use_tagz = SendMessage((HWND)lp,BM_GETCHECK,0,0);
					playlist_switcher_t::g_refresh_all_items();
					playlist_switcher_string_changed = false;
				break;
			case IDC_TABS_MULTILINE:
				{
					cfg_tabs_multiline = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					g_on_multiline_tabs_change();
#if 0
					if (g_main_window && g_tab)
					{
				//		create_tabs();
						long flags = WS_CHILD |  TCS_HOTTRACK | TCS_TABS | (cfg_tabs_multiline ? TCS_MULTILINE : TCS_SINGLELINE) |WS_VISIBLE|WS_CLIPSIBLINGS  |TCS_SINGLELINE;
						
						uSetWindowLong(g_tab, GWL_STYLE, flags);
						move_window_controls();
					}
#endif
				}
				break;
			case IDC_MCLICK3:
				{
					cfg_plm_rename = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
			case IDC_PLDRAG:
				{
					cfg_drag_pl = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
#if 0
			case IDC_MCLICK2:
				{
					cfg_mclick2 = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
			case IDC_TABS:
				{
					cfg_tabs = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					if (g_main_window)
					{
						create_tabs();
						move_window_controls();
					}
				}
				break;
#endif
			case (CBN_SELCHANGE<<16)|IDC_PLISTEDGE:
				{
					cfg_plistframe = uSendMessage((HWND)lp,CB_GETCURSEL,0,0);
					playlist_switcher_t::g_on_edgestyle_change();
				}
				break;
				
			}
		}
		return 0;
	}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_PLAYLISTS,wnd,ConfigProc);}
	virtual const char * get_name() {return "General";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:playlist_switcher:general";
		return true;
	}
} g_tab_playlist;

static class tab_playlist_dd : public preferences_tab
{
	static bool initialised;
	
	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd,IDC_AUTOSWITCH,BM_SETCHECK,cfg_drag_autoswitch,0);

		uSendDlgItemMessage(wnd,IDC_SWITCH_SPIN,UDM_SETPOS32,0,cfg_autoswitch_delay);
		SendDlgItemMessage(wnd,IDC_ACTIVATE_TARGET,BM_SETCHECK,main_window::config_get_activate_target_playlist_on_dropped_items(),0);
		
		//uSendDlgItemMessage(wnd,IDC_DROP_NAME,BM_SETCHECK,cfg_pgen_dir,0);
		//uSendDlgItemMessage(wnd,IDC_DROP_PLAYLIST,BM_SETCHECK,cfg_pgen_playlist,0);
		uSendDlgItemMessage(wnd,IDC_DROP_USE_STRING,BM_SETCHECK,cfg_pgen_tf,0);
		uSendDlgItemMessage(wnd,IDC_REMOVE_UNDERSCORES,BM_SETCHECK,cfg_replace_drop_underscores,0);
		uSendDlgItemMessageText(wnd,IDC_DROP_STRING,WM_SETTEXT,0,cfg_pgenstring);
		
	}
	
public:
	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				
				uSendDlgItemMessage(wnd,IDC_SPINPL,UDM_SETRANGE32,-100,100);
				uSendDlgItemMessage(wnd,IDC_SWITCH_SPIN,UDM_SETRANGE32,0,10000);

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
			switch(wp)
			{
				
			case (EN_CHANGE<<16)|IDC_SWITCH_DELAY:
				{
					if (initialised)
					{
						BOOL result;
						unsigned new_height = GetDlgItemInt(wnd, IDC_SWITCH_DELAY, &result, FALSE);
						if (result) cfg_autoswitch_delay = new_height;
					}
				}
				break;
			case (EN_CHANGE<<16)|IDC_DROP_STRING:
					cfg_pgenstring = string_utf8_from_window((HWND)lp);
				break;
#if 0
			case IDC_DROP_NAME:
					cfg_pgen_dir = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
			case IDC_DROP_PLAYLIST:
					cfg_pgen_playlist = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
#endif
			case IDC_REMOVE_UNDERSCORES:
					cfg_replace_drop_underscores = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
			case IDC_DROP_USE_STRING:
					cfg_pgen_tf = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
			case IDC_ACTIVATE_TARGET:
				main_window::config_set_activate_target_playlist_on_dropped_items(0!=SendMessage((HWND)lp,BM_GETCHECK,0,0));
				break;
			case IDC_AUTOSWITCH:
				{
					cfg_drag_autoswitch = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
				
			}
		}
		return 0;
	}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_PLAYLISTS_DRAGDROP,wnd,ConfigProc);}
	virtual const char * get_name() {return "Drag & Drop";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:playlist_switcher:drag_and_drop";
		return true;
	}
} g_tab_playlist_dd;

void update_vis_host_frames();

static class tab_main : public preferences_tab
{
public:
	static bool initialised;
	
	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd,IDC_IMPORT_TITLES,BM_SETCHECK,cfg_import_titles,0);
		//uSendDlgItemMessage(wnd,IDC_EXPORT_TITLES,BM_SETCHECK,cfg_export_titles,0);

		uSendDlgItemMessage(wnd,IDC_TOOLBARS,BM_SETCHECK,cfg_toolbars,0);
		//uSendDlgItemMessage(wnd,IDC_KEYB,BM_SETCHECK,config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, true),0);
		uSendDlgItemMessage(wnd,IDC_USE_TRANSPARENCY,BM_SETCHECK,main_window::config_get_transparency_enabled(),0);
		uSendDlgItemMessage(wnd,IDC_TRANSPARENCY_SPIN,UDM_SETPOS32,0,main_window::config_get_transparency_level());

		if (!g_main_window)
			EnableWindow(GetDlgItem(wnd, IDC_QUICKSETUP), FALSE);
		
		uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_main_window_title_script.get());
	}
	
	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				HWND wnd_lv = GetDlgItem(wnd, IDC_LIBRARIES);
				g_set_listview_window_explorer_theme(wnd_lv);
				//uSetWindowLong(wnd_lv, GWL_EXSTYLE, uGetWindowLong(wnd_lv, GWL_EXSTYLE)|LVS_EX_INFOTIP );

				listview_helper::insert_column(wnd_lv, 0, "Library", 50);
				listview_helper::insert_column(wnd_lv, 1, "Version", 70);
				listview_helper::insert_column(wnd_lv, 2, "Extended Info", 150);

				uSendDlgItemMessage(wnd,IDC_TRANSPARENCY_SPIN,UDM_SETRANGE32,0,255);

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
			switch(wp)
			{
			case (EN_CHANGE<<16)|IDC_STRING:
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
					cfg_import_titles = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
			/*case IDC_EXPORT_TITLES:
				{
					cfg_export_titles = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;*/
			case (EN_CHANGE<<16)|IDC_TRANSPARENCY_LEVEL:
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
				main_window::config_set_transparency_enabled(uSendMessage((HWND)lp,BM_GETCHECK,0,0) != 0);
				break;
			case IDC_TOOLBARS:
					cfg_toolbars = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					on_show_toolbars_change();
				break;
			case IDC_RESET_TOOLBARS:
				{
					if (win32_helpers::message_box(wnd, _T("Warning! This will reset the toolbars to the default state. Continue?"),_T("Reset toolbars?"),MB_YESNO) == IDYES)
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
					config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, true) = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;*/
		}
	}
	return 0;
}
	static BOOL CALLBACK LibrariesProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		
		switch(msg)
		{
		case WM_DESTROY:
			modeless_dialog_manager::g_remove(wnd);
			break;
		case WM_COMMAND:
			switch(wp)
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
				g_set_listview_window_explorer_theme(wnd_lv);
				//uSetWindowLong(wnd_lv, GWL_EXSTYLE, uGetWindowLong(wnd_lv, GWL_EXSTYLE)|LVS_EX_INFOTIP );

				listview_helper::insert_column(wnd_lv, 0, "Library", 50);
				listview_helper::insert_column(wnd_lv, 1, "Version", 70);
				listview_helper::insert_column(wnd_lv, 2, "Extended Info", 150);

				{

					{
						DLLVERSIONINFO2 dvi;
						pfc::string8 path;
						DWORD ver = GetCommctl32Version(dvi, path);

						pfc::string8 temp;

						if (!ver)
							temp = "4.70";
						else if (dvi.info1.cbSize == sizeof (DLLVERSIONINFO2))
						{
							unsigned short * p_ver = (unsigned short *)&dvi.ullVersion;
							temp = uStringPrintf("%u.%u.%u.%u",p_ver[3], p_ver[2], p_ver[1], p_ver[0]);
						}
						else
							temp = uStringPrintf("%u.%u.%u",dvi.info1.dwMajorVersion,dvi.info1.dwMinorVersion,dvi.info1.dwBuildNumber);

						listview_helper::insert_item(wnd_lv, 0, "comctl32", 0);
						listview_helper::set_item_text(wnd_lv, 0, 1,temp);
						listview_helper::set_item_text(wnd_lv, 0, 2,uStringPrintf("Path: %s",path.get_ptr()));
					}

					pfc::string8 temp, zlib;

					pfc::string8 libpngver;
					unsigned libpngval = get_libpng_version(libpngver, temp);

					listview_helper::insert_item(wnd_lv, 1, "libpng", 0);

					
					if (libpngval == LIBPNG_FOUND)
					{
						listview_helper::set_item_text(wnd_lv, 1, 1,libpngver);
						listview_helper::set_item_text(wnd_lv, 1, 2,uStringPrintf("Path: %s",temp.get_ptr()));
//						console::info(uStringPrintf("libpng version %s found at %s",libpngver.get_ptr(), temp.get_ptr()));
					}
					else if (libpngval == LIBPNG_UNKNOWNVERSION)
					{
						listview_helper::set_item_text(wnd_lv, 1, 1,"Unknown");
						listview_helper::set_item_text(wnd_lv, 1, 2,uStringPrintf("Path: %s",temp.get_ptr()));
					}
					else 
					{
						pfc::string8 error;
						uGetLastErrorMessage(error);
						listview_helper::set_item_text(wnd_lv, 1, 1,"n/a");
						listview_helper::set_item_text(wnd_lv, 1, 2,uStringPrintf("Error: %s",error.get_ptr()));
					}

					listview_helper::insert_item(wnd_lv, 2, "zlib", 0);
					libpngval = get_zlib_version(zlib, temp);

					if (libpngval == LIBPNG_FOUND)
					{
						listview_helper::set_item_text(wnd_lv, 2, 1,zlib);
						listview_helper::set_item_text(wnd_lv, 2, 2,uStringPrintf("Path: %s",temp.get_ptr()));
					}
					else if (libpngval == LIBPNG_UNKNOWNVERSION)
					{
						listview_helper::set_item_text(wnd_lv, 2, 1,"Unknown");
						listview_helper::set_item_text(wnd_lv, 2, 2,uStringPrintf("Path: %s",temp.get_ptr()));
					}
					else 
					{
						pfc::string8 error;
						uGetLastErrorMessage(error);
						listview_helper::set_item_text(wnd_lv, 2, 1,"n/a");
						listview_helper::set_item_text(wnd_lv, 2, 2,uStringPrintf("Error: %s",error.get_ptr()));
					}

				}

			}
			break;
		}
		return 0;
	}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_MAIN,wnd,ConfigProc);}
	virtual const char * get_name() {return "Main";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:main";
		return true;
	}
} g_tab_main;


static class tab_sys : public preferences_tab
{
public:
	static bool initialised;

//	static ptr_list_autofree_t<char> status_items;
	
	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd,IDC_BALLOON,BM_SETCHECK,cfg_balloon,0);

		uSendDlgItemMessage(wnd,IDC_SHOW_SYSTRAY,BM_SETCHECK,cfg_show_systray,0);
//		EnableWindow(GetDlgItem(wnd, IDC_MINIMISE_TO_SYSTRAY), cfg_show_systray);

		uSendDlgItemMessage(wnd,IDC_MINIMISE_TO_SYSTRAY,BM_SETCHECK,cfg_minimise_to_tray,0);
		uSendDlgItemMessage(wnd,IDC_USE_CUSTOM_ICON,BM_SETCHECK,cfg_custom_icon,0);
		uSendDlgItemMessage(wnd,IDC_NOWPL,BM_SETCHECK,cfg_np,0);

		uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_notification_icon_script.get());
		
	}
	
	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		
		switch(msg)
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
			switch(wp)
			{


			case (EN_CHANGE<<16)|IDC_STRING:
				main_window::config_notification_icon_script.set(string_utf8_from_window((HWND)lp));
				break;
			
			case IDC_NOWPL:
				{
					cfg_np = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
			case IDC_USE_CUSTOM_ICON:
				{
					cfg_custom_icon = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					create_icon_handle();create_systray_icon();
				}
				break;
			case IDC_BROWSE_ICON:
				{
					pfc::string8 path = cfg_tray_icon_path;
					if (uGetOpenFileName(wnd, "Icon Files (*.ico)|*.ico|All Files (*.*)|*.*", 0, "ico", "Choose Icon", NULL, path, FALSE))
					{
						cfg_tray_icon_path = path;
						if (cfg_custom_icon) { create_icon_handle();create_systray_icon();}
					}
				}
				break;


			case IDC_MINIMISE_TO_SYSTRAY:
				{
					cfg_minimise_to_tray = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
			case IDC_SHOW_SYSTRAY:
				{
					cfg_show_systray = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
	//				EnableWindow(GetDlgItem(wnd, IDC_MINIMISE_TO_SYSTRAY), cfg_show_systray);

					if (g_main_window)
					{
						if (cfg_show_systray && !g_icon_created) create_systray_icon();
						else if (!cfg_show_systray && g_icon_created &&  (!IsIconic(g_main_window) || !cfg_minimise_to_tray)) destroy_systray_icon();
						if (g_status) update_systray();
					}
				}
				break;
			case IDC_BALLOON:
				{
					cfg_balloon = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				break;
		}
	}
	return 0;
}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_SYS,wnd,ConfigProc);}
	virtual const char * get_name() {return "Notification area";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:notification_area";
		return true;
	}
} g_tab_sys;

static class tab_status : public preferences_tab
{
public:
	static bool initialised;
	static menu_item_cache * p_cache;

//	static ptr_list_autofree_t<char> status_items;
	
	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd,IDC_VOL,BM_SETCHECK,cfg_show_vol,0);
		uSendDlgItemMessage(wnd,IDC_SELTIME,BM_SETCHECK,cfg_show_seltime,0);
		uSendDlgItemMessage(wnd,IDC_SHOW_STATUS,BM_SETCHECK,cfg_status,0);
		uSendDlgItemMessage(wnd,IDC_SHOW_STATUSPANE,BM_SETCHECK,settings::show_status_pane,0);
		uSendDlgItemMessage(wnd,IDC_SHOW_LOCK,BM_SETCHECK,main_window::config_get_status_show_lock(),0);
		
		uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_status_bar_script.get());
	}
	
	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				p_cache = new(std::nothrow) menu_item_cache;

				populate_menu_combo(wnd, IDC_MENU_DBLCLK, IDC_MENU_DESC, cfg_statusdbl, *p_cache, false);

				refresh_me(wnd);
				initialised = true;
			}
			
			break;
		case WM_DESTROY:
			{
				delete p_cache;
				p_cache=0;
//				status_items.free_all();
				initialised = false;
			}
			break;
		case WM_COMMAND:
			switch(wp)
			{


			
			case (EN_CHANGE<<16)|IDC_STRING:
				main_window::config_status_bar_script.set(string_utf8_from_window((HWND)lp));
				break;
			case (CBN_SELCHANGE<<16)|IDC_MENU_DBLCLK:
				{
					on_menu_combo_change(wnd, lp, cfg_statusdbl, *p_cache, IDC_MENU_DESC);
				}
				break;
			case IDC_STATUS_FONT:
				{
					LOGFONT temp = cfg_status_font;
					if (font_picker(temp,wnd))
					{
						cfg_status_font = temp;
						on_status_font_change();
					}
				}
				break;
			case IDC_SHOW_LOCK:
				{
					main_window::config_set_status_show_lock(SendMessage((HWND)lp,BM_GETCHECK,0,0)!=0);
				}
				break;
			case IDC_VOL:
				{
					cfg_show_vol = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					status_bar::set_part_sizes(status_bar::t_part_volume);
				}
				break;
			case IDC_SELTIME:
				{
					cfg_show_seltime = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					status_bar::set_part_sizes(status_bar::t_part_length|status_bar::t_part_volume);

				}
				break;
			case IDC_SHOW_STATUS:
				{
					cfg_status = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					on_show_status_change();
				}
				break;
			case IDC_SHOW_STATUSPANE:
				{
					settings::show_status_pane = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					on_show_status_pane_change();
				}
				break;
		}
	}
	return 0;
}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_STATUS,wnd,ConfigProc);}
	virtual const char * get_name() {return "Status bar";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:status_bar";
		return true;
	}
} g_tab_status;

menu_item_cache * tab_status::p_cache = 0;

static cfg_int g_cur_tab2(create_guid(0x5fb6e011,0x1ead,0x49fe,0x45,0x32,0x1c,0x8a,0x61,0x01,0x91,0x2b),0);


class tab_global : public preferences_tab
{
public:
	static WNDPROC editproc;

	static LRESULT WINAPI EditHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
	/*	case WM_KEYDOWN:
			if (!(HIWORD(lp) & KF_REPEAT) && (wp == 'a' || wp =='A') &&  (GetKeyState(VK_CONTROL) & KF_UP))
			{
				uSendMessage(wnd, EM_SETSEL, 0, -1);
				return 0;
			}
			
			break;*/
		case WM_CHAR:
		if (!(HIWORD(lp) & KF_REPEAT) && (wp == 1) &&  (GetKeyState(VK_CONTROL) & KF_UP))
		{
			uSendMessage(wnd, EM_SETSEL, 0, -1);
			return 0;
		}	
		break;
		}
		return uCallWindowProc(editproc,wnd,msg,wp,lp);
	}

	static void refresh_me(HWND wnd)
	{
		uSendDlgItemMessage(wnd,IDC_GLOBAL,BM_SETCHECK,cfg_global,0);
		uSendDlgItemMessage(wnd,IDC_OLDGLOBAL,BM_SETCHECK,cfg_oldglobal,0);
		uSendDlgItemMessage(wnd,IDC_GLOBALSORT,BM_SETCHECK,cfg_global_sort,0);
		uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, (g_cur_tab2 == 0 ? cfg_globalstring : cfg_colour));
		uSendDlgItemMessage(wnd,IDC_DATE,BM_SETCHECK,cfg_playlist_date,0);
	}
	
	static void save_string(HWND wnd)
	{
		int id = g_cur_tab2;
		if (id >=0 && id < 2)
		{
				if (id == 0) cfg_globalstring = string_utf8_from_window(wnd, IDC_STRING);
				else if (id == 1) cfg_colour = string_utf8_from_window(wnd, IDC_STRING);
		}
	}


	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				uTCITEM tabs;
				memset(&tabs, 0, sizeof(tabs));

				HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

				tabs.mask = TCIF_TEXT;
				tabs.pszText = "Variables";
				uTabCtrl_InsertItem(wnd_tab, 0, &tabs);
				tabs.pszText = "Style";
				uTabCtrl_InsertItem(wnd_tab, 1, &tabs);
				
				TabCtrl_SetCurSel(wnd_tab, g_cur_tab2);

				uSendDlgItemMessageText(wnd, IDC_CHAR7, WM_SETTEXT, 0, "\x07");
				colour_code_gen(wnd, IDC_COLOUR, false, true);

				uSendDlgItemMessage(wnd, IDC_STRING, EM_LIMITTEXT, 0, 0);

				refresh_me(wnd);
				editproc = (WNDPROC)uSetWindowLong(GetDlgItem(wnd, IDC_STRING),GWL_WNDPROC,(LPARAM)EditHook);

				g_editor_font_notify.set(GetDlgItem(wnd, IDC_STRING));
			}
			
			break;

		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom)
			{
			case IDC_TAB1:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
					{
						save_string(wnd);
						int id = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
						g_cur_tab2 = id;
						uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, (g_cur_tab2 ==0 ? cfg_globalstring : cfg_colour));
					}
					break;
				}
				break;
			}
			break;

		case WM_DESTROY:
			{
				g_editor_font_notify.release();
				save_string(wnd);
				refresh_all_playlist_views();
				pvt::ng_playlist_view_t::g_update_all_items();
			}
			break;
			
		case WM_COMMAND:
			switch(wp)
			{
			case IDC_GLOBAL:
					cfg_global = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
			case IDC_DATE:
					cfg_playlist_date = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
					set_day_timer();
					pvt::ng_playlist_view_t::g_on_use_date_info_change();
				break;
			case IDC_TFHELP:
				{
					RECT rc;
					GetWindowRect(GetDlgItem(wnd, IDC_TFHELP), &rc);
			//		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)(&rc), 2);
					HMENU menu = CreatePopupMenu();


					enum {IDM_TFHELP=1, IDM_GHELP=2, IDM_SPEEDTEST, IDM_PREVIEW, IDM_EDITORFONT, IDM_RESETSTYLE};
					
					uAppendMenu(menu,(MF_STRING),IDM_TFHELP,"Titleformatting &help");
					uAppendMenu(menu,(MF_STRING),IDM_GHELP,"&Global help");
					uAppendMenu(menu,(MF_SEPARATOR),0,"");
					uAppendMenu(menu,(MF_STRING),IDM_SPEEDTEST,"&Speed test");
					uAppendMenu(menu,(MF_STRING),IDM_PREVIEW,"&Preview to console");
					uAppendMenu(menu,(MF_SEPARATOR),0,"");
					uAppendMenu(menu,(MF_STRING),IDM_EDITORFONT,"Change editor &font");
					uAppendMenu(menu,(MF_SEPARATOR),0,"");
					uAppendMenu(menu,(MF_STRING),IDM_RESETSTYLE,"&Reset style string");
					
					
					int cmd = TrackPopupMenu(menu,TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,rc.left,rc.bottom,0,wnd,0);
					DestroyMenu(menu);
					if (cmd == IDM_TFHELP)
					{
						standard_commands::main_titleformat_help();
					}
					else if (cmd == IDM_GHELP)
					{
						uMessageBox(wnd, COLOUR_HELP "\n\nNew global format: $set_global(var, val), retreive values using $get_global(var)", "Global help", 0);
					}
					else if (cmd == IDM_SPEEDTEST)
					{
						speedtest(g_columns, cfg_global != 0, cfg_oldglobal != 0, cfg_playlist_date != 0);
					}
					else if (cmd == IDM_PREVIEW)
					{
						preview_to_console(string_utf8_from_window(wnd, IDC_STRING), g_cur_tab2 != 0 && cfg_global);
					}
					else if (cmd == IDM_EDITORFONT)
					{
						if (font_picker(wnd, cfg_editor_font))
							g_editor_font_notify.on_change();
					}
					else if (cmd == IDM_RESETSTYLE)
					{
						extern const char * g_default_colour;
						cfg_colour = g_default_colour;
						if (g_cur_tab2==1)
							uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, cfg_colour);
						refresh_all_playlist_views();
						pvt::ng_playlist_view_t::g_update_all_items();
					}
				}


				break;
			case IDC_OLDGLOBAL:
					cfg_oldglobal = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
			case IDC_GLOBALSORT:
					cfg_global_sort = uSendMessage((HWND)lp,BM_GETCHECK,0,0);
				break;
			case IDC_APPLY:
					save_string(wnd);
					refresh_all_playlist_views();
					pvt::ng_playlist_view_t::g_update_all_items();
				break;			
			case IDC_PICK_COLOUR:
					colour_code_gen(wnd, IDC_COLOUR, false, false);
				break;			
		}
	}
	return 0;
}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_GLOBAL,wnd,ConfigProc);}
	virtual const char * get_name() {return "Globals";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:playlist_view:globals";
		return true;
	}

} g_tab_global;

pvt::preferences_tab_impl g_tab_grouping;

static preferences_tab * g_tabs[] = 
{
	&g_tab_main,
	//&g_tab_layout,
	g_get_tab_layout(),
	//&g_tab_display,
	&g_tab_status,
	&g_tab_sys,
	g_get_tab_filter(),
	g_get_tab_artwork(),
};

static preferences_tab * g_tabs_panels[] = 
{
	&g_tab_playlist,
	//&g_tab_playlist_colours,
	&g_tab_playlist_dd,
};

static preferences_tab * g_tabs_playlist_view[] = 
{
	&g_tab_display2,
	&g_tab_grouping,
	g_get_tab_columns_v3(),
	&g_tab_global,
};

class config_host : public preferences_page
{
public:
	static HWND child;
private:


	static void make_child(HWND wnd)
	{
		//HWND wnd_destroy = child;
		if (child)
		{
			ShowWindow(child, SW_HIDE);
			DestroyWindow(child);
			child=0;
		}

		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
		
		RECT tab;
		
		GetWindowRect(wnd_tab,&tab);
		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);
		
		TabCtrl_AdjustRect(wnd_tab,FALSE,&tab);
		
		unsigned count = tabsize(g_tabs);
		if (cfg_child >= count) cfg_child = 0;

		if (cfg_child < count && cfg_child >= 0)
		{
			child = g_tabs[cfg_child]->create(wnd);
		}
	
		//SetWindowPos(wnd_tab,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
		if (child) 
		{
			EnableThemeDialogTexture(child, ETDT_ENABLETAB);
		}

		SetWindowPos(child, 0, tab.left, tab.top, tab.right-tab.left, tab.bottom-tab.top, SWP_NOZORDER);
		SetWindowPos(wnd_tab,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
		ShowWindow(child, SW_SHOWNORMAL);
		//UpdateWindow(child);

		//if (wnd_destroy)
			//DestroyWindow(wnd_destroy);
	}

	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
				//uSendMessage(wnd_tab, TCM_SETMINTABWIDTH, 0, 35);
				unsigned n, count = tabsize(g_tabs);
				for (n=0; n<count; n++)
				{
					uTabCtrl_InsertItemText(wnd_tab, n, g_tabs[n]->get_name());
				}
				TabCtrl_SetCurSel(wnd_tab, cfg_child);
				make_child(wnd);
			}
			break;
		case WM_DESTROY:
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom)
			{
			case IDC_TAB1:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
					{
						cfg_child = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
						make_child(wnd);
					}
					break;
				}
				break;
			}
			break;
		
			
			case WM_PARENTNOTIFY:
				switch(wp)
				{
				case WM_DESTROY:
					{
						if (child && (HWND)lp == child) child = 0;
					}
					break;	
				}
				break;
		}
		return 0;
	}

public:
	HWND create(HWND parent)
	{
		return uCreateDialog(IDD_HOST,parent,ConfigProc);
	}
	const char * get_name() {return "Columns UI";}
	const char * get_parent_name() {return "Display";}

	static const GUID guid;

	virtual GUID get_guid()
	{
		return guid;
	}
	virtual GUID get_parent_guid() {return preferences_page::guid_display;}
	virtual bool reset_query()
	{
		return false;
	}
	virtual void reset()
	{
	};
	virtual bool get_help_url(pfc::string_base & p_out)
	{
		if (!(cfg_child < tabsize (g_tabs) && g_tabs[cfg_child]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}

};

// {DF6B9443-DCC5-4647-8F8C-D685BF25BD09}
const GUID config_host::guid = 
{ 0xdf6b9443, 0xdcc5, 0x4647, { 0x8f, 0x8c, 0xd6, 0x85, 0xbf, 0x25, 0xbd, 0x9 } };

void g_show_artwork_settings()
{
	cfg_child = 5;
	static_api_ptr_t<ui_control>()->show_preferences(config_host::guid);
}
class config_panels : public preferences_page
{
public:
	static HWND child;
private:


	virtual bool get_help_url(pfc::string_base & p_out)
	{
		if (!(cfg_child_panels < tabsize (g_tabs_panels) && g_tabs_panels[cfg_child_panels]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}
	static void make_child(HWND wnd)
	{
		//HWND wnd_destroy = child;
		if (child)
		{
			ShowWindow(child, SW_HIDE);
			DestroyWindow(child);
			child=0;
		}

		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
		
		RECT tab;
		
		GetWindowRect(wnd_tab,&tab);
		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);
		
		TabCtrl_AdjustRect(wnd_tab,FALSE,&tab);
		
		unsigned count = tabsize(g_tabs_panels);
		if (cfg_child_panels >= count) cfg_child_panels = 0;

		if (cfg_child_panels < count && cfg_child_panels >= 0)
		{
			child = g_tabs_panels[cfg_child_panels]->create(wnd);
		}

		//SetWindowPos(child,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
		if (child) 
		{
			{
				EnableThemeDialogTexture(child, ETDT_ENABLETAB);
			}
		}

		SetWindowPos(child, 0, tab.left, tab.top, tab.right-tab.left, tab.bottom-tab.top, SWP_NOZORDER);
		SetWindowPos(wnd_tab,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
		ShowWindow(child, SW_SHOWNORMAL);
		//UpdateWindow(child);

		//if (wnd_destroy)
		//	DestroyWindow(wnd_destroy);
	}

	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{

				HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
				unsigned n, count = tabsize(g_tabs_panels);


				for (n=0; n<count; n++)
				{
					uTabCtrl_InsertItemText(wnd_tab, n, g_tabs_panels[n]->get_name());
				}
				
				TabCtrl_SetCurSel(wnd_tab, cfg_child_panels);
				
				
				make_child(wnd);
				
			}
			
			break;
		case WM_DESTROY:
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom)
			{
			case IDC_TAB1:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
					{
						cfg_child_panels = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
						make_child(wnd);
					}
					break;
				}
				break;
			}
			break;
			
	
			case WM_PARENTNOTIFY:
				switch(wp)
				{
				case WM_DESTROY:
					{
						if (child && (HWND)lp == child) child = 0;
					}
					break;	
				}
				break;
		}
		return 0;
	}
	



public:
	HWND create(HWND parent)
	{
		return uCreateDialog(IDD_HOST,parent,ConfigProc);
	}
	const char * get_name() {return "Playlist switcher";}

	static const GUID guid;

	virtual GUID get_guid()
	{
		return guid;
	}
	virtual GUID get_parent_guid() {return config_host::guid;}
	virtual bool reset_query()	{return false;}
	virtual void reset() {};

};

// {779F2FA6-3B76-4829-9E02-2E579CA510BF}
const GUID config_panels::guid = 
{ 0x779f2fa6, 0x3b76, 0x4829, { 0x9e, 0x2, 0x2e, 0x57, 0x9c, 0xa5, 0x10, 0xbf } };

class config_playlist : public preferences_page
{
public:
	static HWND child;
private:

	virtual bool get_help_url(pfc::string_base & p_out)
	{
		if (!(cfg_child_playlist < tabsize (g_tabs_playlist_view) && g_tabs_playlist_view[cfg_child_playlist]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}

	static void make_child(HWND wnd)
	{
		//HWND wnd_destroy = child;
		if (child)
		{
			ShowWindow(child, SW_HIDE);
			DestroyWindow(child);
			child=0;
		}

		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
		
		RECT tab;
		
		GetWindowRect(wnd_tab,&tab);
		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);
		
		TabCtrl_AdjustRect(wnd_tab,FALSE,&tab);
		
		unsigned count = tabsize(g_tabs_playlist_view);
		if (cfg_child_playlist >= count) cfg_child_playlist = 0;

		if (cfg_child_playlist < count && cfg_child_playlist >= 0)
		{
			child = g_tabs_playlist_view[cfg_child_playlist]->create(wnd);
		}

		//SetWindowPos(child,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
		if (child) 
		{
			{
				EnableThemeDialogTexture(child, ETDT_ENABLETAB);
			}
		}

		SetWindowPos(child, 0, tab.left, tab.top, tab.right-tab.left, tab.bottom-tab.top, SWP_NOZORDER);
		SetWindowPos(wnd_tab,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
		ShowWindow(child, SW_SHOWNORMAL);
		//UpdateWindow(child);

		//if (wnd_destroy)
		//	DestroyWindow(wnd_destroy);
	}

	static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{

				HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
				unsigned n, count = tabsize(g_tabs_playlist_view);


				for (n=0; n<count; n++)
				{
					uTabCtrl_InsertItemText(wnd_tab, n, g_tabs_playlist_view[n]->get_name());
				}
				
				TabCtrl_SetCurSel(wnd_tab, cfg_child_playlist);
				
				
				make_child(wnd);
				
			}
			
			break;
		case WM_DESTROY:
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom)
			{
			case IDC_TAB1:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
					{
						cfg_child_playlist = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
						make_child(wnd);
					}
					break;
				}
				break;
			}
			break;
			
			
			case WM_PARENTNOTIFY:
				switch(wp)
				{
				case WM_DESTROY:
					{
						if (child && (HWND)lp == child) child = 0;
					}
					break;	
				}
				break;
		}
		return 0;
	}
	



public:
	HWND create(HWND parent)
	{
		return uCreateDialog(IDD_HOST,parent,ConfigProc);
	}
	const char * get_name() {return "Playlist view";}
	static const GUID guid;

	virtual GUID get_guid()
	{
		return guid;
	}
	virtual GUID get_parent_guid() {return config_host::guid;}
	virtual bool reset_query()	{return false;}
	virtual void reset() {};

};


// {B8CA5FC9-7463-48e8-9879-0D9517F3E7A9}
const GUID config_playlist::guid = 
{ 0xb8ca5fc9, 0x7463, 0x48e8, { 0x98, 0x79, 0xd, 0x95, 0x17, 0xf3, 0xe7, 0xa9 } };

namespace columns
{
	const GUID & config_get_playlist_view_guid()
	{
		return config_playlist::guid;
	}
	const GUID & config_get_main_guid()
	{
		return config_host::guid;
	}
};


/*bool config_ui::initialising = false;
bool config_ui2::initialised = false;
bool config_ui3::initialised = false;*/
HWND config_host::child = 0;
HWND config_panels::child = 0;
HWND config_playlist::child = 0;
//config_tab_item * config_host::config_tab = 0;
bool tab_playlist::initialised = 0;
bool tab_playlist::playlist_switcher_string_changed = 0;
bool tab_playlist_dd::initialised = 0;
//bool tab_display::initialised = 0;
bool tab_display2::initialised = 0;

bool tab_main::initialised = 0;
bool tab_sys::initialised = 0;
bool tab_status::initialised = 0;
//ptr_list_autofree_t<char> config_host::tab_sys:: status_items;
//bool tab_columns::initialising = 0;
//WNDPROC tab_columns::editproc = 0;
WNDPROC tab_global::editproc = 0;

//WNDPROC config_extra::editproc;

/*static service_factory_single_t<config,config_ui3> blah;
static service_factory_single_t<config,config_ui2> foo39;
static service_factory_single_t<config,config_ui> foo3;
static service_factory_single_t<config,config_extra> foog3;
*/

static preferences_page_factory_t<config_host> foog43;

static preferences_page_factory_t<config_panels> foog4;
static preferences_page_factory_t<config_playlist> foog3;


void g_set_tab(const char * name)
{
	unsigned n, count = tabsize(g_tabs_playlist_view);
	
	for (n=0; n<count; n++)
	{
		if (!strcmp(g_tabs_playlist_view[n]->get_name(), name))
		{
			cfg_child_playlist = n;
			break;
		}
	}
}