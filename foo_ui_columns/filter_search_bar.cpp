#include "stdafx.h"

#include "filter_search_bar.h"

namespace filter_panel {

	template <class tHandles>
	void g_send_metadb_handles_to_playlist(tHandles & handles, bool b_play = false)
	{
		static_api_ptr_t<playlist_manager> playlist_api;
		static_api_ptr_t<play_control> playback_api;
		t_size index_insert = pfc_infinite;
		if (!b_play && playback_api->is_playing())
		{
			t_size playlist = playlist_api->get_playing_playlist();
			pfc::string8 name;
			if (playlist_api->playlist_get_name(playlist, name) && !stricmp_utf8("Filter Results", name))
			{
				t_size index_old = playlist_api->find_playlist("Filter Results (Playback)", pfc_infinite);
				playlist_api->playlist_rename(playlist, "Filter Results (Playback)", pfc_infinite);
				index_insert = index_old < playlist ? playlist : playlist + 1;
				if (index_old != pfc_infinite)
					playlist_api->remove_playlist(index_old);
			}
		}
		//t_size index_remove = playlist_api->find_playlist("Filter Results", pfc_infinite);
		t_size index = NULL;
		if (index_insert != pfc_infinite)
			index = playlist_api->create_playlist(b_play ? "Filter Results (Playback)" : "Filter Results", pfc_infinite, index_insert);
		else
			index = playlist_api->find_or_create_playlist(b_play ? "Filter Results (Playback)" : "Filter Results", pfc_infinite);
		playlist_api->playlist_clear(index);
#if 1
		if (cfg_sort)
		{
			service_ptr_t<titleformat_object> to;
			static_api_ptr_t<titleformat_compiler>()->compile_safe(to, cfg_sort_string);
			{
				mmh::fb2k::g_sort_metadb_handle_list_by_format_v2(handles, to, nullptr);
			}
		}
		playlist_api->playlist_add_items(index, handles, bit_array_false());
#else
		playlist_api->playlist_add_items_filter(index, handles, false);
#endif
		playlist_api->set_active_playlist(index);
		if (b_play)
		{
			playlist_api->set_playing_playlist(index);
			playback_api->play_start(play_control::track_command_default);
		}
		//if (index_remove != pfc_infinite)
		//	playlist_api->remove_playlist(index+1);
	}

	bool filter_search_bar::menu_node_show_clear_button::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
	{
		p_out = "Show clear button";
		p_displayflags = p_this->m_show_clear_button ? menu_node_t::state_checked : NULL;
		return true;
	}
	bool filter_search_bar::menu_node_show_clear_button::get_description(pfc::string_base & p_out) const
	{
		return false;
	}
	void filter_search_bar::menu_node_show_clear_button::execute()
	{
		p_this->m_show_clear_button = !p_this->m_show_clear_button;
		p_this->on_show_clear_button_change();
	}

	filter_search_bar::filter_search_bar() : m_search_editbox(nullptr), m_wnd_toolbar(nullptr),
		m_proc_search_edit(nullptr), m_favourite_state(false), m_query_timer_active(false),
		m_show_clear_button(cfg_showsearchclearbutton), m_wnd_last_focused(nullptr), m_imagelist(nullptr), m_combo_cx(0), m_combo_cy(0),
		m_toolbar_cx(0), m_toolbar_cy(0) {};

	void g_get_search_bar_sibling_streams(filter_search_bar const * p_serach_bar, pfc::list_t<filter_stream_t::ptr> & p_out)
	{
		if (cfg_orderedbysplitters && p_serach_bar->get_wnd() && p_serach_bar->get_host().is_valid())
		{
			pfc::list_t<uie::window_ptr> siblings;
			uie::window_host_ex::ptr hostex;
			if (p_serach_bar->get_host()->service_query_t(hostex))
				hostex->get_children(siblings);

			//Let's avoid recursion for once
			t_size j = siblings.get_count();
			while (j)
			{
				j--;
				uie::window_ptr p_window = siblings[j];
				siblings.remove_by_idx(j);

				uie::splitter_window_ptr p_splitter;

				if (p_window->get_extension_guid() == cui::panels::guid_filter)
				{
					filter_panel_t* p_filter = static_cast<filter_panel_t*>(p_window.get_ptr());
					if (!p_out.have_item(p_filter->m_stream.get_ptr()))
						p_out.add_item(p_filter->m_stream);
				}
				else if (p_window->service_query_t(p_splitter))
				{
					t_size splitter_child_count = p_splitter->get_panel_count();
					for (t_size k = 0; k < splitter_child_count; k++)
					{
						uie::splitter_item_ptr p_splitter_child;
						p_splitter->get_panel(k, p_splitter_child);
						if (p_splitter_child->get_window_ptr().is_valid() && p_splitter_child->get_window_ptr()->get_wnd())
						{
							siblings.add_item(p_splitter_child->get_window_ptr());
							++j;
						}
					}
				}
			}
		}
	}

	pfc::ptr_list_t<filter_search_bar> filter_search_bar::g_active_instances;

	namespace {
		uie::window_factory<filter_search_bar> g_filter_search_bar;
	};

	void filter_search_bar::set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort)
	{
		if (p_size)
		{
			t_size version;
			p_reader->read_lendian_t(version, p_abort);
			if (version <= config_version_current)
			{
				p_reader->read_lendian_t(m_show_clear_button, p_abort);
			}
		}
	};
	void filter_search_bar::get_config(stream_writer * p_writer, abort_callback & p_abort) const
	{
		p_writer->write_lendian_t(t_size(config_version_current), p_abort);
		p_writer->write_lendian_t(m_show_clear_button, p_abort);
	}

	void filter_search_bar::get_menu_items(uie::menu_hook_t & p_hook)
	{
		p_hook.add_node(new menu_node_show_clear_button(this));
	}

	void filter_search_bar::on_show_clear_button_change()
	{
		TBBUTTONINFO tbbi;
		memset(&tbbi, 0, sizeof(tbbi));
		tbbi.cbSize = sizeof(tbbi);
		tbbi.dwMask = TBIF_STATE;
		tbbi.fsState = TBSTATE_ENABLED | (m_show_clear_button ? NULL : TBSTATE_HIDDEN);
		SendMessage(m_wnd_toolbar, TB_SETBUTTONINFO, idc_clear, (LPARAM)&tbbi);
		//UpdateWindow(m_wnd_toolbar);
		RECT rc = { 0 };
		SendMessage(m_wnd_toolbar, TB_GETITEMRECT, 1, (LPARAM)(&rc));

		m_toolbar_cx = rc.right;
		m_toolbar_cy = rc.bottom;

		if (get_host().is_valid()) get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
		on_size();
	}
	/*void filter_search_bar::g_on_orderedbysplitters_change()
	{
	for (t_size i = 0, count = g_active_instances.get_count(); i<count; i++)
	g_active_instances[i]->commit_search_results(string_utf8_from_window(g_active_instances[i]->m_search_editbox), false, true);
	}*/

	bool filter_search_bar::g_activate()
	{
		if (g_active_instances.get_count())
		{
			g_active_instances[0]->activate();
			return true;
		}
		return false;
	}

	bool filter_search_bar::g_filter_search_bar_has_stream(filter_search_bar const* p_seach_bar, const filter_stream_t * p_stream)
	{
		pfc::list_t<filter_stream_t::ptr> p_streams;
		g_get_search_bar_sibling_streams(p_seach_bar, p_streams);
		return p_streams.have_item(const_cast<filter_stream_t *>(p_stream)); //meh
	}

	void filter_search_bar::activate()
	{
		m_wnd_last_focused = GetFocus();
		SetFocus(m_search_editbox);
	}

	void filter_search_bar::on_size(t_size cx, t_size cy)
	{
		//RECT rc_tbb = {0};
		//SendMessage(m_wnd_toolbar, TB_GETITEMRECT, 0, (LPARAM)(&rc_tbb));
		SetWindowPos(m_search_editbox, nullptr, 0, 0, cx - m_toolbar_cx, 200, SWP_NOZORDER);
		SetWindowPos(m_wnd_toolbar, nullptr, cx - m_toolbar_cx, 0, m_toolbar_cx, cy, SWP_NOZORDER);
	}
	void filter_search_bar::on_search_editbox_change()
	{
		if (m_query_timer_active)
			KillTimer(get_wnd(), TIMER_QUERY);
		SetTimer(get_wnd(), TIMER_QUERY, 500, nullptr);
		m_query_timer_active = true;
		update_favourite_icon();
	}
	void filter_search_bar::commit_search_results(const char * str, bool b_force_autosend, bool b_stream_update)
	{
		pfc::list_t<filter_stream_t::ptr> p_streams;
		g_get_search_bar_sibling_streams(this, p_streams);
		if (p_streams.get_count() == 0)
			p_streams = filter_panel_t::g_streams;

		t_size stream_count = p_streams.get_count();
		bool b_diff = strcmp(m_active_search_string, str) != 0;
		if (!stream_count) b_force_autosend = b_diff;
		if (b_diff || b_force_autosend || b_stream_update)
		{
			m_active_search_string = str;

			bool b_reset = m_active_search_string.is_empty();

			if (b_reset) { m_active_handles.remove_all(); }
			else if (b_diff)
			{
				static_api_ptr_t<library_manager>()->get_all_items(m_active_handles);
				try {
					search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_active_search_string);
					pfc::array_t<bool> data;
					data.set_size(m_active_handles.get_count());
					api->test_multi(m_active_handles, data.get_ptr());
					m_active_handles.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
				}
				catch (pfc::exception const &) {};
			}

			/*bit_array_bittable mask_visible(stream_count);
			bool b_any_visible = false;
			for (t_size i = 0; i< stream_count; i++)
			{
			mask_visible.set(i, p_streams[i]->is_visible());
			if (mask_visible.get(i)) b_any_visible = true;
			}*/
			bool b_autosent = false;
			for (t_size i = 0; i< stream_count; i++)
			{
				filter_stream_t::ptr p_stream = p_streams[i];

				p_stream->m_source_overriden = !b_reset;
				p_stream->m_source_handles = m_active_handles;

				if (!b_stream_update)
				{
					t_size filter_count = p_stream->m_windows.get_count();
					if (filter_count)
					{
						bool b_stream_visible = p_stream->is_visible();//mask_visible.get(i);
						pfc::list_t<filter_panel_t*> ordered_windows;
						p_stream->m_windows[0]->get_windows(ordered_windows);
						if (ordered_windows.get_count())
						{
							if (b_diff)
								ordered_windows[0]->refresh((b_stream_visible || stream_count == 1) && !b_autosent);
							if (!b_autosent)
							{
								if ((b_stream_visible || stream_count == 1) && b_force_autosend && !cfg_autosend)
									ordered_windows[0]->send_results_to_playlist();
								b_autosent = b_stream_visible || stream_count == 1;
							}
						}
					}
				}
			}
			if (!b_stream_update && (stream_count == 0 || !b_autosent) && (cfg_autosend || b_force_autosend))
				g_send_metadb_handles_to_playlist(m_active_handles);
		}
	}

	LRESULT filter_search_bar::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_CREATE:
			m_font = uCreateIconFont();
			create_edit();
			g_active_instances.add_item(this);
			break;
		case WM_NCDESTROY:
			g_active_instances.remove_item(this);
			if (!core_api::is_shutting_down())
				commit_search_results("");
			m_font.release();
			m_active_handles.remove_all();
			m_active_search_string.reset();
			if (m_imagelist)
			{
				ImageList_Destroy(m_imagelist);
				m_imagelist = nullptr;
			}
			break;
		case WM_TIMER:
			switch (wp)
			{
			case TIMER_QUERY:
				KillTimer(get_wnd(), TIMER_QUERY);
				if (m_query_timer_active)
					commit_search_results(string_utf8_from_window(m_search_editbox));
				m_query_timer_active = false;
				return 0;
			}
			break;
		case WM_SETFOCUS:
			break;
		case WM_KILLFOCUS:
			break;
		case msg_favourite_selected:
			if (m_query_timer_active)
			{
				KillTimer(get_wnd(), TIMER_QUERY);
				m_query_timer_active = false;
			}
			commit_search_results(string_utf8_from_window(m_search_editbox));
			update_favourite_icon();
			return 0;
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO mmi = LPMINMAXINFO(lp);
			mmi->ptMinTrackSize.x = m_combo_cx + m_toolbar_cx;
			mmi->ptMinTrackSize.y = max(m_combo_cy, m_toolbar_cy);
			mmi->ptMaxTrackSize.y = mmi->ptMinTrackSize.y;
		}
		return 0;
		case WM_NOTIFY:
		{
			LPNMHDR lpnm = (LPNMHDR)lp;
			switch (lpnm->idFrom)
			{
			case id_toolbar:
				switch (lpnm->code)
				{
				case TBN_GETINFOTIP:
				{
					LPNMTBGETINFOTIP lpnmtbgit = (LPNMTBGETINFOTIP)lp;
					pfc::string8 temp;
					if (lpnmtbgit->iItem == idc_favourite)
					{
						string_utf8_from_window query(m_search_editbox);
						temp = !query.is_empty() && cfg_favourites.have_item(query) ? "Remove from favourites" : "Add to favourites";
					}
					else if (lpnmtbgit->iItem == idc_clear)
						temp = "Clear";
					StringCchCopy(lpnmtbgit->pszText, lpnmtbgit->cchTextMax, pfc::stringcvt::string_wide_from_utf8(temp));
				}
				return 0;
#if 0
				case NM_CUSTOMDRAW:
				{
					LPNMTBCUSTOMDRAW lptbcd = (LPNMTBCUSTOMDRAW)lp;
					switch ((lptbcd)->nmcd.dwDrawStage)
					{
					case CDDS_PREPAINT:
						return (CDRF_NOTIFYITEMDRAW);
					case CDDS_ITEMPREPAINT:
					{
						if (lptbcd->nmcd.dwItemSpec == idc_clear)
						{
							DLLVERSIONINFO2 dvi;
							HRESULT hr = g_get_comctl32_vresion(dvi);
							if (SUCCEEDED(hr) && dvi.info1.dwMajorVersion >= 6)
								lptbcd->rcText.left -= LOWORD(SendMessage(m_wnd_toolbar, TB_GETPADDING, (WPARAM)0, 0)) + 2;  //Hack for commctrl6
						}
					}
					break;
					}
				}
				break;
#endif
				}
				break;
			}
		};
		break;
		case WM_COMMAND:
			switch (LOWORD(wp))
			{
			case id_edit:
				switch (HIWORD(wp))
				{
				case CBN_EDITCHANGE:
					on_search_editbox_change();
					break;
				case CBN_SELCHANGE:
					PostMessage(wnd, msg_favourite_selected, NULL, NULL);
					break;
				case CBN_SETFOCUS:
					PostMessage(m_search_editbox, CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
					break;
				case CBN_KILLFOCUS:
					m_wnd_last_focused = nullptr;
					break;
				}
				break;
			case idc_clear:
				if (m_query_timer_active)
				{
					KillTimer(get_wnd(), TIMER_QUERY);
					m_query_timer_active = false;
				}
				ComboBox_SetText(m_search_editbox, L"");
				UpdateWindow(m_search_editbox);
				update_favourite_icon();
				commit_search_results("");
				break;
			case idc_favourite:
			{
				string_utf8_from_window query(m_search_editbox);
				t_size index;
				if (!query.is_empty())
				{
					if (cfg_favourites.find_item(query, index))
					{
						cfg_favourites.remove_by_idx(index);
						for (t_size i = 0, count = g_active_instances.get_count(); i<count; i++)
							if (g_active_instances[i]->m_search_editbox)
								ComboBox_DeleteString(g_active_instances[i]->m_search_editbox, index);
					}
					else
					{
						cfg_favourites.add_item(query);
						for (t_size i = 0, count = g_active_instances.get_count(); i<count; i++)
							if (g_active_instances[i]->m_search_editbox)
								ComboBox_AddString(g_active_instances[i]->m_search_editbox, uT(query));
					}
					update_favourite_icon();
				}
			}
			break;
			}
			break;
		}
		return DefWindowProc(wnd, msg, wp, lp);
	}

	void filter_search_bar::update_favourite_icon(const char * p_new)
	{
		bool new_state = cfg_favourites.have_item(p_new ? p_new : string_utf8_from_window(m_search_editbox));
		if (m_favourite_state != new_state)
		{
			TBBUTTONINFO tbbi;
			memset(&tbbi, 0, sizeof(tbbi));
			tbbi.cbSize = sizeof(tbbi);
			tbbi.dwMask = TBIF_IMAGE;
			tbbi.iImage = new_state ? 1 : 0;
			//tbbi.pszText = (LPWSTR)(new_state ? L"Remove from favourites" : L"Add to favourites");
			SendMessage(m_wnd_toolbar, TB_SETBUTTONINFO, idc_favourite, (LPARAM)&tbbi);
			UpdateWindow(m_wnd_toolbar);
			m_favourite_state = new_state;
		}
	}

	void filter_search_bar::create_edit()
	{
		m_favourite_state = false;

		m_search_editbox = CreateWindowEx(WS_EX_CLIENTEDGE, WC_COMBOBOX, L"" /*pfc::stringcvt::string_os_from_utf8("").get_ptr()*/, WS_CHILD | WS_CLIPSIBLINGS | ES_LEFT |
			WS_VISIBLE | WS_CLIPCHILDREN | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_TABSTOP | WS_VSCROLL, 0,
			0, 100, 200, get_wnd(), HMENU(id_edit), core_api::get_my_instance(), nullptr);

		ComboBox_SetMinVisible(m_search_editbox, 25);

		m_wnd_toolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, nullptr,
			WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER,
			0, 0, 0, 0, get_wnd(), (HMENU)id_toolbar, core_api::get_my_instance(), nullptr);
		//SetWindowTheme(m_wnd_toolbar, L"SearchButton", NULL);

		const unsigned cx = GetSystemMetrics(SM_CXSMICON), cy = GetSystemMetrics(SM_CYSMICON);

		m_imagelist = ImageList_Create(cx, cy, ILC_COLOR32, 0, 3);

		t_size i = 0;

		HICON icon = (HICON)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDI_STAROFF), IMAGE_ICON, cx, cy, NULL);
		if (icon)
		{
			ImageList_ReplaceIcon(m_imagelist, -1, icon);
			DestroyIcon(icon);
			if (icon = (HICON)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDI_STARON), IMAGE_ICON, cx, cy, NULL))
			{
				ImageList_ReplaceIcon(m_imagelist, -1, icon);
				DestroyIcon(icon);
				if (icon = (HICON)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDI_LEFT), IMAGE_ICON, cx, cy, NULL))
				{
					ImageList_ReplaceIcon(m_imagelist, -1, icon);
					DestroyIcon(icon);
				}
			}
		}

		TBBUTTON tbb[2];
		memset(&tbb, 0, sizeof(tbb));
		tbb[0].iBitmap = 2;
		tbb[0].idCommand = idc_clear;
		tbb[0].fsState = TBSTATE_ENABLED | (m_show_clear_button ? NULL : TBSTATE_HIDDEN);
		tbb[0].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
		//tbb[0].iString = (INT_PTR)(L"C"); 
		tbb[1].iBitmap = 0;
		tbb[1].idCommand = idc_favourite;
		tbb[1].fsState = TBSTATE_ENABLED;
		tbb[1].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
		tbb[1].iString = -1;
		//tb.iString = (INT_PTR)(L"Add to favourites");


		unsigned ex_style = SendMessage(m_wnd_toolbar, TB_GETEXTENDEDSTYLE, 0, 0);
		SendMessage(m_wnd_toolbar, TB_SETEXTENDEDSTYLE, 0, ex_style | TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);
		SendMessage(m_wnd_toolbar, TB_SETBITMAPSIZE, (WPARAM)0, MAKELONG(cx, cy));
		SendMessage(m_wnd_toolbar, TB_GETPADDING, (WPARAM)0, 0);

		SendMessage(m_wnd_toolbar, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)m_imagelist);
		SendMessage(m_wnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
		SendMessage(m_wnd_toolbar, TB_ADDBUTTONS, (WPARAM)tabsize(tbb), (LPARAM)&tbb[0]);
		ShowWindow(m_wnd_toolbar, SW_SHOWNORMAL);
		SendMessage(m_wnd_toolbar, TB_AUTOSIZE, 0, 0);

		SendMessage(m_search_editbox, WM_SETFONT, (WPARAM)m_font.get(), MAKELONG(TRUE, 0));

		COMBOBOXINFO cbi;
		memset(&cbi, 0, sizeof(cbi));
		cbi.cbSize = sizeof(cbi);
		SendMessage(m_search_editbox, CB_GETCOMBOBOXINFO, NULL, (LPARAM)&cbi);

		RECT rc;
		GetClientRect(m_search_editbox, &rc);
		m_combo_cx += RECT_CX(rc) - RECT_CX(cbi.rcItem);

		GetWindowRect(m_search_editbox, &rc);
		m_combo_cy = rc.bottom - rc.top;

		SendMessage(m_wnd_toolbar, TB_GETITEMRECT, 1, (LPARAM)(&rc));

		m_toolbar_cx = rc.right;
		m_toolbar_cy = rc.bottom;

		SetWindowLongPtr(m_search_editbox, GWLP_USERDATA, (LPARAM)(this));
		SetWindowLongPtr(cbi.hwndItem, GWLP_USERDATA, (LPARAM)(this));
		m_proc_search_edit = (WNDPROC)SetWindowLongPtr(cbi.hwndItem, GWLP_WNDPROC, (LPARAM)(g_on_search_edit_message));
		Edit_SetCueBannerText(cbi.hwndItem, uT("Search Filters"));

		for (t_size i = 0, count = cfg_favourites.get_count(); i<count; i++)
			ComboBox_AddString(m_search_editbox, uT(cfg_favourites[i]));

		on_size();
	}

	LRESULT WINAPI filter_search_bar::g_on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		filter_search_bar * p_this;
		LRESULT rv;

		p_this = reinterpret_cast<filter_search_bar*>(GetWindowLongPtr(wnd, GWLP_USERDATA));

		rv = p_this ? p_this->on_search_edit_message(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);;

		return rv;
	}

	LRESULT filter_search_bar::on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_KILLFOCUS:
			//m_wnd_last_focused = NULL;
			break;
		case WM_SETFOCUS:
			//m_wnd_last_focused = (HWND)wp;
			break;
		case WM_GETDLGCODE:
			//return CallWindowProc(m_proc_search_edit,wnd,msg,wp,lp)|DLGC_WANTALLKEYS;
			break;
		case WM_KEYDOWN:
			switch (wp)
			{
			case VK_TAB:
			{
				uie::window::g_on_tab(wnd);
			}
			//return 0;
			break;
			case VK_ESCAPE:
				if (m_wnd_last_focused && IsWindow(m_wnd_last_focused))
					SetFocus(m_wnd_last_focused);
				return 0;
			case VK_DELETE:
			{
#if 0
				if (ComboBox_GetDroppedState(m_search_editbox) == TRUE)
				{
					int index = ComboBox_GetCurSel(m_search_editbox);
					if (index != -1 && (t_size)index < cfg_favourites.get_count())
					{
						cfg_favourites.remove_by_idx(index);
						for (t_size i = 0, count = g_active_instances.get_count(); i<count; i++)
							if (g_active_instances[i]->m_search_editbox)
								ComboBox_DeleteString(g_active_instances[i]->m_search_editbox, index);
					}
				}
#endif
			}
			break;
			case VK_RETURN:
				if (m_query_timer_active)
				{
					KillTimer(get_wnd(), TIMER_QUERY);
					m_query_timer_active = false;
				}
				commit_search_results(string_utf8_from_window(m_search_editbox), true);
				return 0;
			}
			break;
		case WM_CHAR:
			switch (wp)
			{
			case VK_ESCAPE:
			case VK_RETURN:
				return 0;
			};
			break;
		}
		return CallWindowProc(m_proc_search_edit, wnd, msg, wp, lp);
	}


}