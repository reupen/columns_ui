#include "stdafx.h"

#include "artwork.h"
#include "config.h"

namespace artwork_panel
{
#if 0
	// {5A409F56-7388-42b0-91C7-ACB6CD0032DC}
	const GUID g_guid_cfg_front = 
	{ 0x5a409f56, 0x7388, 0x42b0, { 0x91, 0xc7, 0xac, 0xb6, 0xcd, 0x0, 0x32, 0xdc } };
	// {6015499C-575F-4bae-8D9C-6D6FD23A40FE}
	const GUID g_guid_cfg_back = 
	{ 0x6015499c, 0x575f, 0x4bae, { 0x8d, 0x9c, 0x6d, 0x6f, 0xd2, 0x3a, 0x40, 0xfe } };
	// {9DC64EB7-9252-4a88-AE13-4F1BAB216F90}
	const GUID g_guid_cfg_disc = 
	{ 0x9dc64eb7, 0x9252, 0x4a88, { 0xae, 0x13, 0x4f, 0x1b, 0xab, 0x21, 0x6f, 0x90 } };
#endif
	// {D6B58D7C-CABA-49f3-8E46-4CCD396B9FC7}
	const GUID g_guid_cfg_icon =
	{ 0xd6b58d7c, 0xcaba, 0x49f3, { 0x8e, 0x46, 0x4c, 0xcd, 0x39, 0x6b, 0x9f, 0xc7 } };
	// {A24038C7-C055-45ed-B631-CC8FD2A22473}
	const GUID g_guid_fb2k_artwork_mode =
	{ 0xa24038c7, 0xc055, 0x45ed, { 0xb6, 0x31, 0xcc, 0x8f, 0xd2, 0xa2, 0x24, 0x73 } };
	// {005C7B29-3915-4b83-A283-C01A4EDC4F3A}
	const GUID g_guid_track_mode =
	{ 0x5c7b29, 0x3915, 0x4b83, { 0xa2, 0x83, 0xc0, 0x1a, 0x4e, 0xdc, 0x4f, 0x3a } };
	// {A35E8697-0B8A-4e6f-9DBE-39EC4626524D}
	const GUID g_guid_preserve_aspect_ratio =
	{ 0xa35e8697, 0xb8a, 0x4e6f, { 0x9d, 0xbe, 0x39, 0xec, 0x46, 0x26, 0x52, 0x4d } };

	// {F5C8CE6B-5D68-4ce2-8B9F-874D8EDB03B3}
	const GUID g_guid_edge_style =
	{ 0xf5c8ce6b, 0x5d68, 0x4ce2, { 0x8b, 0x9f, 0x87, 0x4d, 0x8e, 0xdb, 0x3, 0xb3 } };

	// {F6E92FCD-7E02-4329-9DA3-D03AEDD66D07}
	static const GUID g_guid_cfg_front_scripts =
	{ 0xf6e92fcd, 0x7e02, 0x4329, { 0x9d, 0xa3, 0xd0, 0x3a, 0xed, 0xd6, 0x6d, 0x7 } };
	// {BD2474FC-2CF9-475f-AC0B-26130541526C}
	static const GUID g_guid_cfg_back_scripts =
	{ 0xbd2474fc, 0x2cf9, 0x475f, { 0xac, 0xb, 0x26, 0x13, 0x5, 0x41, 0x52, 0x6c } };
	// {70D71DF4-D1FF-4d19-9412-B949690ED43E}
	static const GUID g_guid_cfg_disc_scripts =
	{ 0x70d71df4, 0xd1ff, 0x4d19, { 0x94, 0x12, 0xb9, 0x49, 0x69, 0xe, 0xd4, 0x3e } };

	// {C1E7DA7E-1D3A-4f30-8384-0E47C49B6DD9}
	static const GUID g_guid_cfg_artist_scripts =
	{ 0xc1e7da7e, 0x1d3a, 0x4f30, { 0x83, 0x84, 0xe, 0x47, 0xc4, 0x9b, 0x6d, 0xd9 } };


	enum track_mode_t
	{
		track_auto_playlist_playing,
		track_playlist,
		track_playing,
		track_auto_selection_playing,
		track_selection,
	};

	const bool g_track_mode_includes_now_playing(t_size mode)
	{
		return mode == track_auto_playlist_playing || mode == track_auto_selection_playing || mode == track_playing;
	}

	const bool g_track_mode_includes_plalist(t_size mode)
	{
		return mode == track_auto_playlist_playing || mode == track_playlist;
	}

	const bool g_track_mode_includes_auto(t_size mode)
	{
		return mode == track_auto_playlist_playing || mode == track_auto_selection_playing;
	}

	const bool g_track_mode_includes_selection(t_size mode)
	{
		return mode == track_auto_selection_playing || mode == track_selection;
	}

#if 0
	cfg_string cfg_front(g_guid_cfg_front, "");
	cfg_string cfg_back(g_guid_cfg_back, "");
	cfg_string cfg_disc(g_guid_cfg_disc, "");
#endif
	//cfg_string cfg_icon(g_guid_cfg_icon, "");
	cfg_uint cfg_fb2k_artwork_mode(g_guid_fb2k_artwork_mode, fb2k_artwork_embedded_and_external);
	cfg_uint cfg_track_mode(g_guid_track_mode, track_auto_playlist_playing);
	cfg_bool cfg_preserve_aspect_ratio(g_guid_preserve_aspect_ratio, true);
	cfg_uint cfg_edge_style(g_guid_edge_style, 0);

	cfg_objList<pfc::string8> cfg_front_scripts(g_guid_cfg_front_scripts),
		cfg_back_scripts(g_guid_cfg_back_scripts),
		cfg_disc_scripts(g_guid_cfg_disc_scripts),
		cfg_artist_scripts(g_guid_cfg_artist_scripts);


	// {E32DCBA9-A2BF-4901-AB43-228628071410}
	static const GUID g_guid_colour_client =
	{ 0xe32dcba9, 0xa2bf, 0x4901, { 0xab, 0x43, 0x22, 0x86, 0x28, 0x7, 0x14, 0x10 } };

	const GUID g_artwork_types[] = { album_art_ids::cover_front, album_art_ids::cover_back, album_art_ids::disc, album_art_ids::artist };


	void artwork_panel_t::get_config(stream_writer * p_writer, abort_callback & p_abort) const
	{
		p_writer->write_lendian_t(m_track_mode, p_abort);
		p_writer->write_lendian_t((t_uint32)current_stream_version, p_abort);
		p_writer->write_lendian_t(m_preserve_aspect_ratio, p_abort);
		p_writer->write_lendian_t(m_lock_type, p_abort);
		p_writer->write_lendian_t(m_position, p_abort);
	}

	void artwork_panel_t::get_menu_items(ui_extension::menu_hook_t & p_hook)
	{
		p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_type_popup(this)));
		p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_source_popup(this)));
		p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_preserve_aspect_ratio(this)));
		p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_lock_type(this)));
		p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_options()));
	}

	artwork_panel_t::artwork_panel_t() : m_position(0), m_gdiplus_instance(NULL),
		m_gdiplus_initialised(false), m_track_mode(cfg_track_mode),
		m_preserve_aspect_ratio(cfg_preserve_aspect_ratio), m_lock_type(false) {};




	void artwork_panel_t::g_on_edge_style_change()
	{
		unsigned i, count = g_windows.get_count();
		for (i = 0; i < count; i++)
		{
			HWND wnd = g_windows[i]->get_wnd();
			if (wnd)
			{
				long flags = 0;
				if (cfg_edge_style == 1) flags |= WS_EX_CLIENTEDGE;
				if (cfg_edge_style == 2) flags |= WS_EX_STATICEDGE;
				SetWindowLongPtr(wnd, GWL_EXSTYLE, flags);
				SetWindowPos(wnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			}
		}
	}
	const GUID & artwork_panel_t::get_extension_guid() const
	{
		// {DEEAD6EC-F0B9-4919-B16D-280AEDDE7343}
		static const GUID guid =
		{ 0xdeead6ec, 0xf0b9, 0x4919, { 0xb1, 0x6d, 0x28, 0xa, 0xed, 0xde, 0x73, 0x43 } };
		return guid;
	}
	void artwork_panel_t::get_name(pfc::string_base & out)const
	{
		out = "Artwork view";
	}
	void artwork_panel_t::get_category(pfc::string_base & out)const
	{
		out = "Panels";
	}
	unsigned artwork_panel_t::get_type() const
	{
		return uie::type_panel;
	}
	void artwork_panel_t::on_repository_change()
	{
		if (m_artwork_loader.is_valid())
		{
			m_artwork_loader->ResetRepository();
			if (cfg_front_scripts.get_count())
				m_artwork_loader->SetScript(g_artwork_types[0], cfg_front_scripts);
			if (cfg_back_scripts.get_count())
				m_artwork_loader->SetScript(g_artwork_types[1], cfg_back_scripts);
			if (cfg_disc_scripts.get_count())
				m_artwork_loader->SetScript(g_artwork_types[2], cfg_disc_scripts);
			if (cfg_artist_scripts.get_count())
				m_artwork_loader->SetScript(g_artwork_types[3], cfg_artist_scripts);
		}
	}
	LRESULT artwork_panel_t::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_CREATE:
		{
			m_gdiplus_initialised = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplus_instance, &Gdiplus::GdiplusStartupInput(), NULL));
			m_artwork_loader = new artwork_reader_manager_t;//pfc::rcnew_t<artwork_reader_t>();
			//		m_nowplaying_artwork_loader.initialise(this);
			t_size i, count = tabsize(g_artwork_types);
			for (i = 0; i < count; i++)
				m_artwork_loader->AddType(g_artwork_types[i]);
			on_repository_change();
			m_artwork_loader->initialise();
			static_api_ptr_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_new_track | play_callback::flag_on_playback_stop | play_callback::flag_on_playback_edited, false);
			static_api_ptr_t<playlist_manager_v3>()->register_callback(this, playlist_callback_flags);
			g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
			force_reload_artwork();
			g_windows.add_item(this);
		}
		break;
		case WM_DESTROY:
			g_windows.remove_item(this);
			static_api_ptr_t<ui_selection_manager>()->unregister_callback(this);
			static_api_ptr_t<playlist_manager_v3>()->unregister_callback(this);
			static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
			m_selection_handles.remove_all();
			m_image.release();
			m_bitmap.release();
			if (m_gdiplus_initialised)
				Gdiplus::GdiplusShutdown(m_gdiplus_instance);
			m_gdiplus_initialised = false;
			if (m_artwork_loader.is_valid())
				m_artwork_loader->deinitialise();
			m_artwork_loader.release();
			//	m_nowplaying_artwork_loader.deinitialise();
			break;
		case WM_ERASEBKGND:
			return FALSE;
		case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
			if (!(lpwp->flags & SWP_NOSIZE))
			{
				flush_cached_bitmap();
				RedrawWindow(wnd, NULL, NULL, RDW_INVALIDATE);
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			bool b_found = false;
			t_size i, count = tabsize(g_artwork_types);
			for (i = 1; i < count; i++)
			{
				if (refresh_image((m_position + i) % (tabsize(g_artwork_types))))
				{
					m_position = (m_position + i) % (tabsize(g_artwork_types));
					b_found = true;
					break;
				}
			}
			//if (!b_found && i+1==count)
			//	show_emptycover();
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(wnd, &ps);
			RECT rc;
			GetClientRect(wnd, &rc);
			if (m_gdiplus_initialised)
			{
				if (!m_bitmap.is_valid())
					refresh_cached_bitmap();

				if (m_bitmap.is_valid())
				{
					HDC dcc = CreateCompatibleDC(dc);

					HBITMAP bm_old = SelectBitmap(dcc, m_bitmap);
					BitBlt(dc, 0, 0, RECT_CX(rc), RECT_CY(rc), dcc, 0, 0, SRCCOPY);
					SelectBitmap(dcc, bm_old);
					DeleteDC(dcc);
				}
				else
				{
					FillRect(ps.hdc, &rc, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(cui::colours::helper(g_guid_colour_client).get_colour(cui::colours::colour_background))));
					/*HTHEME thm = OpenThemeData(g_main_window, L"FLYOUT");
					DrawThemeBackground(thm, dc, 	FLYOUT_DIVIDER	, 0, &rc, NULL);
					CloseThemeData(thm);*/
				}

				/*Gdiplus::Graphics graphics(ps.hdc);

				if (!m_bitmap.is_valid() || Gdiplus::Ok != graphics.DrawCachedBitmap(&*m_bitmap, 0, 0))
				{
				refresh_cached_bitmap();
				if (!m_bitmap.is_valid() || (Gdiplus::Ok != graphics.DrawCachedBitmap(&*m_bitmap, 0, 0)))
				FillRect(ps.hdc, &rc, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(cui::colours::helper(g_guid_colour_client).get_colour(cui::colours::colour_background))));
				}
				//else FillRect(ps.hdc, &rc, GetSysColorBrush(COLOR_WINDOWTEXT));*/

			}
			EndPaint(wnd, &ps);
		}
		return 0;
		};
		return DefWindowProc(wnd, msg, wp, lp);
	}
	bool g_check_process_on_selection_changed()
	{
		HWND wnd_focus = GetFocus();
		if (wnd_focus == NULL)
			return false;

		DWORD processid = NULL;
		GetWindowThreadProcessId(wnd_focus, &processid);
		if (processid != GetCurrentProcessId())
			return false;

		return true;
	}
	void artwork_panel_t::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr> & p_selection)
	{
		if (g_check_process_on_selection_changed())
		{
			if (g_ui_selection_manager_is_now_playing_fallback())
				m_selection_handles.remove_all();
			else
				m_selection_handles = p_selection;

			if (g_track_mode_includes_selection(m_track_mode) && (!g_track_mode_includes_auto(m_track_mode) || !static_api_ptr_t<play_control>()->is_playing()))
			{
				if (m_selection_handles.get_count())
				{
					m_artwork_loader->Request(m_selection_handles[0], new service_impl_t<completion_notify_forwarder>(this));
				}
				else
				{
					flush_image();
					RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
					if (m_artwork_loader.is_valid())
						m_artwork_loader->Reset();
				}
			}
		}
	}

#if 1
	void artwork_panel_t::on_playback_stop(play_control::t_stop_reason p_reason)
	{
		if (g_track_mode_includes_now_playing(m_track_mode) && p_reason != play_control::stop_reason_starting_another && p_reason != play_control::stop_reason_shutting_down)
		{
			bool b_set = false;
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			if (m_track_mode == track_auto_playlist_playing)
			{
				static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
			}
			else if (m_track_mode == track_auto_selection_playing)
			{
				handles = m_selection_handles;
			}

			if (handles.get_count())
			{
				m_artwork_loader->Request(handles[0], new service_impl_t<completion_notify_forwarder>(this));
				b_set = true;
			}

			if (!b_set)
			{
				flush_image();
				RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
				if (m_artwork_loader.is_valid())
					m_artwork_loader->Reset();
			}
		}
	}
	void artwork_panel_t::on_playback_new_track(metadb_handle_ptr p_track)
	{
		if (g_track_mode_includes_now_playing(m_track_mode) && m_artwork_loader.is_valid())
			m_artwork_loader->Request(p_track, new service_impl_t<completion_notify_forwarder>(this));
	}
	void artwork_panel_t::force_reload_artwork()
	{
		metadb_handle_ptr handle;
		if (g_track_mode_includes_now_playing(m_track_mode) && static_api_ptr_t<play_control>()->is_playing())
		{
			static_api_ptr_t<play_control>()->get_now_playing(handle);
		}
		else if (g_track_mode_includes_plalist(m_track_mode))
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
			if (handles.get_count())
				handle = handles[0];
		}
		else if (g_track_mode_includes_selection(m_track_mode))
		{
			if (m_selection_handles.get_count())
				handle = m_selection_handles[0];
		}

		if (handle.is_valid())
		{
			m_artwork_loader->Request(handle, new service_impl_t<completion_notify_forwarder>(this));
		}
		else
		{
			flush_image();
			RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
			if (m_artwork_loader.is_valid())
				m_artwork_loader->Reset();
		}
	}

	void artwork_panel_t::on_playlist_switch()
	{
		if (g_track_mode_includes_plalist(m_track_mode) && (!g_track_mode_includes_auto(m_track_mode) || !static_api_ptr_t<play_control>()->is_playing()))
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
			if (handles.get_count())
			{
				m_artwork_loader->Request(handles[0], new service_impl_t<completion_notify_forwarder>(this));
			}
			else
			{
				flush_image();
				RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
				if (m_artwork_loader.is_valid())
					m_artwork_loader->Reset();
			}
		}
	}
	void artwork_panel_t::on_items_selection_change(const bit_array & p_affected, const bit_array & p_state)
	{
		if (g_track_mode_includes_plalist(m_track_mode) && (!g_track_mode_includes_auto(m_track_mode) || !static_api_ptr_t<play_control>()->is_playing()))
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
			if (handles.get_count())
			{
				m_artwork_loader->Request(handles[0], new service_impl_t<completion_notify_forwarder>(this));
			}
			else
			{
				flush_image();
				RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
				if (m_artwork_loader.is_valid())
					m_artwork_loader->Reset();
			}
		}
	}

	void artwork_panel_t::on_completion(unsigned p_code)
	{
		if (p_code == 1 && get_wnd())
		{
			bool b_found = false;
			t_size i, count = tabsize(g_artwork_types);
			if (m_lock_type)
				count = min(1, count);
			for (i = 0; i < count; i++)
			{
				if (refresh_image((m_position + i) % (tabsize(g_artwork_types))))
				{
					m_position = (m_position + i) % (tabsize(g_artwork_types));
					b_found = true;
					break;
				}
			}

			if (!b_found)
			{
				show_emptycover();
			}
		}
	}
#endif

	void artwork_panel_t::show_emptycover()
	{
		if (m_artwork_loader.is_valid() && m_artwork_loader->IsReady())
		{
			album_art_data_ptr data;
			if (m_artwork_loader->QueryEmptyCover(data))
			{
				pfc::com_ptr_t<mmh::win32::IStream_memblock> pStream = new mmh::win32::IStream_memblock((const t_uint8*)data->get_ptr(), data->get_size());
				{
					m_image = pfc::rcnew_t<Gdiplus::Bitmap>(pStream.get_ptr());
					pStream.release();
					if (m_image->GetLastStatus() == Gdiplus::Ok)
					{
						flush_cached_bitmap();
						RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
					}
				}
			}
			else
			{
				flush_image();
				RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
			}
		}
	}

	bool artwork_panel_t::refresh_image(t_size index)
	{
		if (m_artwork_loader.is_valid() && m_artwork_loader->IsReady())
		{
			album_art_data_ptr data;
			if (m_artwork_loader->Query(g_artwork_types[index], data))
				//if (m_nowplaying_artwork_loader.get_data(m_position, data))
			{
				pfc::com_ptr_t<mmh::win32::IStream_memblock> pStream = new mmh::win32::IStream_memblock((const t_uint8*)data->get_ptr(), data->get_size());
				///m_image.release();
				///flush_cached_bitmap();
				{
					m_image = pfc::rcnew_t<Gdiplus::Bitmap>(pStream.get_ptr());
					pStream.release();
					if (m_image->GetLastStatus() == Gdiplus::Ok)
					{
						flush_cached_bitmap();
						RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
						return true;
					}
				}
			}
		}
		return false;
	}
	void artwork_panel_t::flush_cached_bitmap()
	{
		m_bitmap.release();
	}
	void artwork_panel_t::flush_image()
	{
		m_image.release();
		flush_cached_bitmap();
	}
	void artwork_panel_t::refresh_cached_bitmap()
	{
		RECT rc;
		GetClientRect(get_wnd(), &rc);
		if (RECT_CX(rc) && RECT_CY(rc) && m_image.is_valid())
		{
			HDC dc = NULL, dcc = NULL;
			dc = GetDC(get_wnd());
			dcc = CreateCompatibleDC(dc);

			m_bitmap = CreateCompatibleBitmap(dc, RECT_CX(rc), RECT_CY(rc));

			HBITMAP bm_old = SelectBitmap(dcc, m_bitmap);

			unsigned err = 0;
			Gdiplus::Graphics graphics(dcc);
			err = graphics.GetLastStatus();

			//Gdiplus::Bitmap bm(RECT_CX(rc), RECT_CY(rc), &_graphics);
			//err = bm.GetLastStatus();

			//Gdiplus::Graphics graphics(&bm);
			//err = graphics.GetLastStatus();

			double ar_source = (double)m_image->GetWidth() / (double)m_image->GetHeight();
			double ar_dest = (double)RECT_CX(rc) / (double)RECT_CY(rc);
			unsigned cx = RECT_CX(rc), cy = RECT_CY(rc);

			graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
			graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

			COLORREF cr = cui::colours::helper(g_guid_colour_client).get_colour(cui::colours::colour_background);
			Gdiplus::SolidBrush br(Gdiplus::Color(LOBYTE(LOWORD(cr)), HIBYTE(LOWORD(cr)), LOBYTE(HIWORD(cr))));
			err = graphics.FillRectangle(&br, 0, 0, cx, cy);

			if (m_preserve_aspect_ratio)
			{
				if (ar_dest < ar_source)
					cy = (unsigned)floor((double)RECT_CX(rc) / ar_source);
				else if (ar_dest > ar_source)
					cx = (unsigned)floor((double)RECT_CY(rc) * ar_source);
			}
			if ((RECT_CY(rc) - cy) % 2) cy++;
			if ((RECT_CX(rc) - cx) % 2) cx++;

			if (m_image->GetWidth() >= 2 && m_image->GetHeight() >= 2)
			{
				Gdiplus::Rect destRect(INT((RECT_CX(rc) - cx) / 2), INT((RECT_CY(rc) - cy) / 2), cx, cy);
				graphics.SetClip(destRect);
				//destRect.Inflate(1,1);
				Gdiplus::ImageAttributes imageAttributes;
				imageAttributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);

				graphics.DrawImage(&*m_image, destRect, 0, 0, m_image->GetWidth(), m_image->GetHeight(), Gdiplus::UnitPixel, &imageAttributes);
				err = graphics.GetLastStatus();
			}
			//m_bitmap = pfc::rcnew_t<Gdiplus::CachedBitmap>(&bm, &_graphics);
			//err = m_bitmap->GetLastStatus();

			SelectBitmap(dcc, bm_old);

			DeleteDC(dcc);
			ReleaseDC(get_wnd(), dc);
		}
		else m_bitmap.release();
	}

	void artwork_panel_t::g_on_colours_change()
	{
		t_size i, count = g_windows.get_count();
		for (i = 0; i < count; i++)
		{
			g_windows[i]->flush_cached_bitmap();
			RedrawWindow(g_windows[i]->get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
		}
	}

	void g_on_repository_change()
	{
		artwork_panel_t::g_on_repository_change();
	}
	void artwork_panel_t::g_on_repository_change()
	{
		t_size i, count = g_windows.get_count();
		for (i = 0; i < count; i++)
		{
			g_windows[i]->on_repository_change();
		}
	}

	pfc::ptr_list_t<artwork_panel_t> artwork_panel_t::g_windows;

	uie::window_factory<artwork_panel_t> g_artwork_panel;

	class appearance_client_artwork_impl : public cui::colours::client
	{
	public:
		static const GUID g_guid;

		virtual const GUID & get_client_guid() const { return g_guid_colour_client; };
		virtual void get_name(pfc::string_base & p_out) const { p_out = "Artwork View"; };

		virtual t_size get_supported_colours() const { return cui::colours::colour_flag_background; }; //bit-mask
		virtual t_size get_supported_bools() const { return 0; }; //bit-mask
		virtual bool get_themes_supported() const { return false; };

		virtual void on_colour_changed(t_size mask) const
		{
			artwork_panel_t::g_on_colours_change();
		};
		virtual void on_bool_changed(t_size mask) const {};
	};

	namespace {
		cui::colours::client::factory<appearance_client_artwork_impl> g_appearance_client_impl;
	};

	artwork_panel_t::completion_notify_forwarder::completion_notify_forwarder(artwork_panel_t * p_this) : m_this(p_this)
	{

	}

	void artwork_panel_t::completion_notify_forwarder::on_completion(unsigned p_code)
	{
		m_this->on_completion(p_code);
	}


	artwork_panel_t::class_data & artwork_panel_t::get_class_data() const
	{
		DWORD flags = 0;
		if (cfg_edge_style == 1) flags |= WS_EX_CLIENTEDGE;
		if (cfg_edge_style == 2) flags |= WS_EX_STATICEDGE;
		__implement_get_class_data_ex2(_T("CUI Artwork View"), _T(""), false, true, 0, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CONTROLPARENT | flags, 0, IDC_HAND);
	}

	void artwork_panel_t::set_config(stream_reader * p_reader, t_size size, abort_callback & p_abort)
	{
		if (size)
		{
			p_reader->read_lendian_t(m_track_mode, p_abort);
			t_uint32 version = pfc_infinite;
			try {
				p_reader->read_lendian_t(version, p_abort);
			}
			catch (exception_io_data_truncation const &) {};

			if (version <= 3)
			{
				p_reader->read_lendian_t(m_preserve_aspect_ratio, p_abort);
				if (version >= 2)
				{
					p_reader->read_lendian_t(m_lock_type, p_abort);
					if (version >= 3)
					{
						p_reader->read_lendian_t(m_position, p_abort);
						if (m_position >= tabsize(g_artwork_types))
							m_position = 0;
					}
				}
			}
		}
	}


#if 0
	class now_playing_album_art_receiver
	{
	public:
		virtual void on_loading(const char * p_path) {}
		//! @p_data May be null when there is no album art data available for currently played file.
		virtual void on_data(const char * p_path) {}
		virtual void on_stopped() {}
	};
	class now_playing_album_art_manager
	{
	public:
		enum state_t
		{
			state_stopped,state_loading,state_loaded
		};
		class now_playing_album_art_callback_impl : public now_playing_album_art_callback_impl_base
		{
		public:
			virtual void on_loading(const char * p_path) 
			{
				m_state = state_loading;
				m_collater->on_loading(p_path);
			}
			//! @p_data May be null when there is no album art data available for currently played file.
			virtual void on_data(const char * p_path,album_art_data_ptr p_data) 
			{
				m_data = p_data;
				m_state = state_loaded;
				m_collater->on_data(p_path);
			}
			virtual void on_stopped()
			{
				m_data.release();
				m_state = state_stopped;
				m_collater->on_stopped();
			}

			virtual GUID get_wanted_type() {return g_artwork_types[m_index];}

			now_playing_album_art_callback_impl() : m_state(state_stopped), m_collater(NULL), m_index(NULL) {};

			state_t m_state;
			album_art_data_ptr m_data;
			now_playing_album_art_manager * m_collater;
			t_size m_index;
		};
		void on_loading(const char * p_path)
		{
			bool b_synchronised = true;
			t_size i, count = m_callbacks.get_count();
			for (i=0; i<count; i++)
				if (m_callbacks[i]->m_state != state_loading)
				{
					b_synchronised = false;
					break;
				}
			if (b_synchronised)
			{
				m_receiver->on_stopped();
			}
		}
		void on_data(const char * p_path) 
		{
			bool b_synchronised = true;
			t_size i, count = m_callbacks.get_count();
			for (i=0; i<count; i++)
				if (m_callbacks[i]->m_state != state_loaded)
				{
					b_synchronised = false;
					break;
				}
			if (b_synchronised)
			{
				m_receiver->on_data(p_path);
			}
		}
		void on_stopped()
		{
			bool b_synchronised = true;
			t_size i, count = m_callbacks.get_count();
			for (i=0; i<count; i++)
				if (m_callbacks[i]->m_state != state_stopped)
				{
					b_synchronised = false;
					break;
				}
			if (b_synchronised)
			{
				m_receiver->on_stopped();
			}
		}
		bool get_data(t_size index, album_art_data_ptr & p_out)
		{
			p_out = m_callbacks[index]->m_data;
			return p_out.is_valid();
		}
		void initialise(now_playing_album_art_receiver * p_receiver)
		{
			m_receiver = p_receiver;
			m_callbacks.set_size(tabsize(g_artwork_types));
			t_size i, count = m_callbacks.get_count();
			for (i=0; i<count; i++)
			{
				m_callbacks[i] = pfc::rcnew_t<now_playing_album_art_callback_impl>();
				m_callbacks[i]->m_collater=this;
				m_callbacks[i]->m_index=i;
			}
		}
		void deinitialise()
		{
			m_callbacks.remove_all();
		}
		pfc::list_t< pfc::rcptr_t<now_playing_album_art_callback_impl> > m_callbacks;
		now_playing_album_art_receiver * m_receiver;

		now_playing_album_art_manager() : m_receiver(NULL) {};
	};
#endif




	artwork_panel_t::menu_node_track_mode::menu_node_track_mode(artwork_panel_t * p_wnd, t_size p_value) : p_this(p_wnd), m_source(p_value)
	{

	}

	void artwork_panel_t::menu_node_track_mode::execute()
	{
		p_this->m_track_mode = m_source;
		cfg_track_mode = m_source;
		p_this->force_reload_artwork();
	}

	bool artwork_panel_t::menu_node_track_mode::get_description(pfc::string_base & p_out) const
	{
		return false;
	}

	bool artwork_panel_t::menu_node_track_mode::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
	{
		p_out = get_name(m_source);
		p_displayflags = (m_source == p_this->m_track_mode) ? ui_extension::menu_node_t::state_radiochecked : 0;
		return true;
	}

	const char * artwork_panel_t::menu_node_track_mode::get_name(t_size source)
	{
		if (source == track_playing) return "Playing item";
		if (source == track_playlist) return "Playlist selection";
		if (source == track_auto_selection_playing) return "Automatic (current selection/playing item)";
		if (source == track_selection) return "Current selection";
		return "Automatic (playlist selection/playing item)";
	}

	artwork_panel_t::menu_node_artwork_type::menu_node_artwork_type(artwork_panel_t * p_wnd, t_size p_value) : p_this(p_wnd), m_type(p_value)
	{

	}

	void artwork_panel_t::menu_node_artwork_type::execute()
	{
		if (!p_this->refresh_image(m_type))
		{
			p_this->show_emptycover();
		}
		p_this->m_position = m_type;
	}

	bool artwork_panel_t::menu_node_artwork_type::get_description(pfc::string_base & p_out) const
	{
		return false;
	}

	bool artwork_panel_t::menu_node_artwork_type::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
	{
		p_out = get_name(m_type);
		p_displayflags = (m_type == p_this->m_position) ? ui_extension::menu_node_t::state_radiochecked : 0;
		return true;
	}

	const char * artwork_panel_t::menu_node_artwork_type::get_name(t_size source)
	{
		if (source == 0) return "Front cover";
		else if (source == 1) return "Back cover";
		else if (source == 2) return "Disc cover";
		else return "Artist picture";
	}

	artwork_panel_t::menu_node_source_popup::menu_node_source_popup(artwork_panel_t * p_wnd)
	{
		m_items.add_item(new menu_node_track_mode(p_wnd, 3));
		m_items.add_item(new menu_node_track_mode(p_wnd, 0));
		m_items.add_item(new uie::menu_node_separator_t());
		m_items.add_item(new menu_node_track_mode(p_wnd, 2));
		m_items.add_item(new menu_node_track_mode(p_wnd, 4));
		m_items.add_item(new menu_node_track_mode(p_wnd, 1));
	}

	void artwork_panel_t::menu_node_source_popup::get_child(unsigned p_index, uie::menu_node_ptr & p_out) const
	{
		p_out = m_items[p_index].get_ptr();
	}

	unsigned artwork_panel_t::menu_node_source_popup::get_children_count() const
	{
		return m_items.get_count();
	}

	bool artwork_panel_t::menu_node_source_popup::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
	{
		p_out = "Displayed track";
		p_displayflags = 0;
		return true;
	}

	artwork_panel_t::menu_node_type_popup::menu_node_type_popup(artwork_panel_t * p_wnd)
	{
		m_items.add_item(new menu_node_artwork_type(p_wnd, 0));
		//m_items.add_item(new uie::menu_node_separator_t());
		m_items.add_item(new menu_node_artwork_type(p_wnd, 1));
		m_items.add_item(new menu_node_artwork_type(p_wnd, 2));
		m_items.add_item(new menu_node_artwork_type(p_wnd, 3));
	}

	void artwork_panel_t::menu_node_type_popup::get_child(unsigned p_index, uie::menu_node_ptr & p_out) const
	{
		p_out = m_items[p_index].get_ptr();
	}

	unsigned artwork_panel_t::menu_node_type_popup::get_children_count() const
	{
		return m_items.get_count();
	}

	bool artwork_panel_t::menu_node_type_popup::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
	{
		p_out = "Artwork type";
		p_displayflags = 0;
		return true;
	}

	artwork_panel_t::menu_node_preserve_aspect_ratio::menu_node_preserve_aspect_ratio(artwork_panel_t * p_wnd) : p_this(p_wnd)
	{

	}

	void artwork_panel_t::menu_node_preserve_aspect_ratio::execute()
	{
		p_this->m_preserve_aspect_ratio = !p_this->m_preserve_aspect_ratio;
		cfg_preserve_aspect_ratio = p_this->m_preserve_aspect_ratio;
		p_this->flush_cached_bitmap();
		RedrawWindow(p_this->get_wnd(), NULL, NULL, RDW_INVALIDATE | RDW_INVALIDATE);
	}

	bool artwork_panel_t::menu_node_preserve_aspect_ratio::get_description(pfc::string_base & p_out) const
	{
		return false;
	}

	bool artwork_panel_t::menu_node_preserve_aspect_ratio::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
	{
		p_out = "Preserve aspect ratio";
		p_displayflags = (p_this->m_preserve_aspect_ratio) ? ui_extension::menu_node_t::state_checked : 0;
		return true;
	}

	void artwork_panel_t::menu_node_options::execute()
	{
		g_show_artwork_settings();
	}

	bool artwork_panel_t::menu_node_options::get_description(pfc::string_base & p_out) const
	{
		return false;
	}

	bool artwork_panel_t::menu_node_options::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
	{
		p_out = "Options";
		p_displayflags = 0;
		return true;
	}

	artwork_panel_t::menu_node_lock_type::menu_node_lock_type(artwork_panel_t * p_wnd) : p_this(p_wnd)
	{

	}

	void artwork_panel_t::menu_node_lock_type::execute()
	{
		p_this->m_lock_type = !p_this->m_lock_type;
	}

	bool artwork_panel_t::menu_node_lock_type::get_description(pfc::string_base & p_out) const
	{
		return false;
	}

	bool artwork_panel_t::menu_node_lock_type::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
	{
		p_out = "Lock artwork type";
		p_displayflags = (p_this->m_lock_type) ? ui_extension::menu_node_t::state_checked : 0;
		return true;
	}

};

