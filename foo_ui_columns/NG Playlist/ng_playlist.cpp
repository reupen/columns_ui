/**
* \file main.cpp
*
* \brief Example panel component
*
* This component is an example of a very simple multiple instance panel that does not take keyboard input
*/

#include "..\stdafx.h"

#include "ng_playlist.h"
#include "ng_playlist_groups.h"

namespace artwork_panel
{
	//extern cfg_string cfg_front;
	extern cfg_bool cfg_use_fb2k_artwork;
};

namespace pvt {

	// {775E2746-6019-4b45-83B3-D7DF3A5BAE57}
	const GUID g_groups_guid = 
	{ 0x775e2746, 0x6019, 0x4b45, { 0x83, 0xb3, 0xd7, 0xdf, 0x3a, 0x5b, 0xae, 0x57 } };

	// {85068AD6-A45D-4968-9898-A34F1F3C40EA}
	const GUID g_show_artwork_guid = 
	{ 0x85068ad6, 0xa45d, 0x4968, { 0x98, 0x98, 0xa3, 0x4f, 0x1f, 0x3c, 0x40, 0xea } };

	// {3003B49E-AD9C-464f-8B54-E9F864D52BBA}
	const GUID g_artwork_width_guid = 
	{ 0x3003b49e, 0xad9c, 0x464f, { 0x8b, 0x54, 0xe9, 0xf8, 0x64, 0xd5, 0x2b, 0xba } };

	// {C764D238-403F-4a8c-B310-30D23E4C42F6}
	const GUID g_artwork_reflection = 
	{ 0xc764d238, 0x403f, 0x4a8c, { 0xb3, 0x10, 0x30, 0xd2, 0x3e, 0x4c, 0x42, 0xf6 } };

	// {CB1C1C5D-4F99-4c24-B239-A6E008E5BA7C}
	const GUID g_artwork_lowpriority = 
	{ 0xcb1c1c5d, 0x4f99, 0x4c24, { 0xb2, 0x39, 0xa6, 0xe0, 0x8, 0xe5, 0xba, 0x7c } };

	// {A28CC736-2B8B-484c-B7A9-4CC312DBD357}
	const GUID g_guid_grouping = 
	{ 0xa28cc736, 0x2b8b, 0x484c, { 0xb7, 0xa9, 0x4c, 0xc3, 0x12, 0xdb, 0xd3, 0x57 } };

	cfg_groups_t g_groups(g_groups_guid);

	cfg_bool cfg_show_artwork(g_show_artwork_guid,false),
		cfg_artwork_reflection(g_artwork_reflection, true),
		cfg_artwork_lowpriority(g_artwork_lowpriority, true),
		cfg_grouping(g_guid_grouping, true);
	uih::ConfigUint32DpiAware cfg_artwork_width(g_artwork_width_guid, 100);

	void cfg_groups_t::swap (t_size index1, t_size index2)
	{
		m_groups.swap_items(index1, index2);
		ng_playlist_view_t::g_on_groups_change();
	}
	void cfg_groups_t::replace_group (t_size index, const group_t & p_group)
	{
		m_groups.replace_item(index, p_group);
		ng_playlist_view_t::g_on_groups_change();
	}
	t_size cfg_groups_t::add_group (const group_t & p_group)
	{
		t_size ret = m_groups.add_item(p_group);
		ng_playlist_view_t::g_on_groups_change();
		return ret;
	}

	void cfg_groups_t::set_groups(const pfc::list_base_const_t<group_t> & p_groups, bool b_update_views) {
		m_groups.remove_all();
		m_groups.add_items(p_groups);
		if (b_update_views)
			ng_playlist_view_t::g_on_groups_change();
	}

	void cfg_groups_t::remove_group (t_size index)
	{
		m_groups.remove_by_idx(index);
		ng_playlist_view_t::g_on_groups_change();
	}

	ng_playlist_view_t::ng_playlist_view_t()
		: m_dragging(false), m_dragging_initial_playlist(pfc_infinite), m_ignore_callback(false),
		m_gdiplus_token(NULL), m_gdiplus_initialised(false), m_mainmenu_manager_base(NULL),
		m_contextmenu_manager_base(NULL), m_day_timer_active(false)
	{};

	ng_playlist_view_t::~ng_playlist_view_t()
	{}

	void ng_playlist_view_t::populate_list()
	{
		metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
		m_playlist_api->activeplaylist_get_all_items(data);
		on_items_added(0, data, bit_array_false());
	}
	void ng_playlist_view_t::refresh_groups(bool b_update_columns)
	{
		static_api_ptr_t<titleformat_compiler> p_compiler;
		service_ptr_t<titleformat_object> p_script, p_script_group;
		m_scripts.remove_all();

		pfc::string8 filter, playlist_name;
		m_playlist_api->activeplaylist_get_name(playlist_name);

		t_size i, count = cfg_grouping ? g_groups.get_groups().get_count() : 0, used_count=0;
		for (i=0; i<count; i++)
		{
			bool b_valid = false;
			if (1)
			{
				switch(g_groups.get_groups()[i].filter_type)
				{
				case FILTER_NONE:
					{
						b_valid = true;
						break;
					}
				case FILTER_SHOW:
					{
						if (wildcard_helper::test(playlist_name,g_groups.get_groups()[i].filter_playlists,true))
							b_valid = true;
					}
					break;
				case FILTER_HIDE:
					{
						if (!wildcard_helper::test(playlist_name,g_groups.get_groups()[i].filter_playlists,true))
							b_valid = true;
					}
					break;
				}
			}
			if (b_valid)
			{
				p_compiler->compile_safe(p_script_group, g_groups.get_groups()[i].string);
				m_scripts.add_item(p_script_group);
				used_count++;
			}
		}
		set_group_count(used_count, b_update_columns);
	}

	t_size ng_playlist_view_t::column_index_display_to_actual (t_size display_index)
	{
		t_size i, count = m_column_mask.get_count(), counter=0;
		for (i=0; i<count; i++)
		{
			if (m_column_mask[i])
				if (counter++ == display_index)
					return i;
		}
		throw pfc::exception_bug_check();
	}

	t_size ng_playlist_view_t::column_index_actual_to_display (t_size actual_index)
	{
		t_size i, count = m_column_mask.get_count(), counter=0;
		for (i=0; i<count; i++)
		{
			if (m_column_mask[i])
			{
				counter++;
				if (i == actual_index)
					return counter;
			}
		}
		return pfc_infinite;
		//throw pfc::exception_bug_check();
	}
	void ng_playlist_view_t::on_column_widths_change()
	{
		t_size i , count = m_column_mask.get_count();
		pfc::list_t<t_size> widths;
		for (i=0; i<count; i++)
			if (m_column_mask[i])
				widths.add_item(g_columns[i]->width);
		set_column_widths(widths);
	}

	void ng_playlist_view_t::g_on_column_widths_change(const ng_playlist_view_t * p_skip)
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			if (g_windows[i] != p_skip)
				g_windows[i]->on_column_widths_change();
	}
	void ng_playlist_view_t::refresh_columns()
	{
		static_api_ptr_t<titleformat_compiler> p_compiler;
		service_ptr_t<titleformat_object> p_script, p_script_group;
		m_script_global.release();
		m_script_global_style.release();

		m_column_data.remove_all();
		m_edit_fields.remove_all();
		pfc::list_t<t_column> columns;

		pfc::string8 filter, playlist_name;
		m_playlist_api->activeplaylist_get_name(playlist_name);

		if (cfg_global)
			p_compiler->compile_safe(m_script_global, cfg_globalstring);
		p_compiler->compile_safe(m_script_global_style, cfg_colour);

		t_size i, count = g_columns.get_count();
		m_column_mask.set_size(count);
		for (i=0; i<count; i++)
		{
			column_t * source = g_columns[i].get_ptr();
			bool b_valid = false;
			if (source->show)
			{
				switch(source->filter_type)
				{
				case FILTER_NONE:
					{
						b_valid = true;
						break;
					}
				case FILTER_SHOW:
					{
						if (wildcard_helper::test(playlist_name,source->filter,true))
							b_valid = true;
					}
					break;
				case FILTER_HIDE:
					{
						if (!wildcard_helper::test(playlist_name,source->filter,true))
							b_valid = true;
					}
					break;
				}
			}
			if (b_valid)
			{
				column_data_t temp;
				columns.add_item(t_column(source->name, source->width, source->parts, (ui_helpers::alignment)source->align));
				p_compiler->compile_safe(temp.m_display_script, source->spec);
				if (source->use_custom_colour)
					p_compiler->compile_safe(temp.m_style_script, source->colour_spec);
				if (source->use_custom_sort)
					p_compiler->compile_safe(temp.m_sort_script, source->sort_spec);
				else
					temp.m_sort_script = temp.m_display_script;
				m_column_data.add_item(temp);
				m_edit_fields.add_item(source->edit_field);
			}
			m_column_mask[i] = b_valid;
		}
		set_columns(columns);
	}
	void ng_playlist_view_t::g_on_groups_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->on_groups_change();
	}
	void ng_playlist_view_t::on_groups_change()
	{
		if (get_wnd())
		{
			clear_all_items();
			refresh_groups(true);
			populate_list();
		}
	}

	void ng_playlist_view_t::update_all_items(bool b_update_display)
	{
		static_api_ptr_t<titleformat_compiler> p_compiler;
		service_ptr_t<titleformat_object> p_script, p_script_group;

		m_script_global.release();
		m_script_global_style.release();

		if (cfg_global)
			p_compiler->compile_safe(m_script_global, cfg_globalstring);
		p_compiler->compile_safe(m_script_global_style, cfg_colour);

		refresh_all_items_text(b_update_display);
	}
	void ng_playlist_view_t::refresh_all_items_text(bool b_update_display)
	{
		update_items(0, get_item_count(), false);
		invalidate_all();
	}
	void ng_playlist_view_t::update_items(t_size index, t_size count, bool b_update_display)
	{
		t_size i,j,cg;
		for (i=0; i<count; i++)
		{
			cg = get_item(i+index)->get_group_count();
			for (j=0; j<cg; j++)
				get_item(i+index)->get_group(j)->m_style_data.release();
			get_item(i+index)->m_style_data.set_count(0);
		}
		t_list_view::update_items(index, count, b_update_display);
	}
	void ng_playlist_view_t::g_on_autosize_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_autosize(cfg_nohscroll!=0);
	}
	void ng_playlist_view_t::g_on_show_artwork_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_show_group_info_area(cfg_show_artwork);
	}
	void ng_playlist_view_t::g_on_alternate_selection_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_alternate_selection_model(cfg_alternative_sel != 0);
	}
	void ng_playlist_view_t::g_on_artwork_width_change(const ng_playlist_view_t * p_skip)
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
		{
			ng_playlist_view_t * p_window = g_windows[i];
			if (p_window != p_skip)
			{
				p_window->flush_artwork_images();
				p_window->set_group_info_area_size(cfg_artwork_width, cfg_artwork_width + (cfg_artwork_reflection ? (cfg_artwork_width*3)/11 : 0) );
			}
		}
	}
	void ng_playlist_view_t::g_flush_artwork(bool b_redraw, const ng_playlist_view_t * p_skip)
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
		{
			ng_playlist_view_t * p_window = g_windows[i];
			if (p_window != p_skip)
			{
				p_window->flush_artwork_images();
				if (b_redraw)
					p_window->invalidate_all();
			}
		}
	}
	void ng_playlist_view_t::g_on_artwork_repositories_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
		{
			ng_playlist_view_t * p_window = g_windows[i];
			if (p_window->m_artwork_manager.is_valid())
			{
				p_window->m_artwork_manager->set_script(album_art_ids::cover_front, artwork_panel::cfg_front_scripts);
			}
		}
	}
	void ng_playlist_view_t::g_on_vertical_item_padding_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_vertical_item_padding(settings::playlist_view_item_padding);
	} 
	void ng_playlist_view_t::g_on_font_change()
	{
		LOGFONT lf;
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_items_font, lf);
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_font(&lf);
	} 
	void ng_playlist_view_t::g_on_header_font_change()
	{
		LOGFONT lf;
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_header_font, lf);
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_header_font(&lf);
	} 
	void ng_playlist_view_t::g_on_group_header_font_change()
	{
		LOGFONT lf;
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_group_header_font, lf);
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_group_font(&lf);
	} 
	void ng_playlist_view_t::g_update_all_items()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->update_all_items();
	}
	void ng_playlist_view_t::g_on_show_header_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_show_header(cfg_header!=0);
	}
	void ng_playlist_view_t::g_on_sorting_enabled_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_sorting_enabled(cfg_header_hottrack!=0);
	}
	void ng_playlist_view_t::g_on_show_sort_indicators_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_show_sort_indicators(cfg_show_sort_arrows!=0);
	}
	void ng_playlist_view_t::g_on_edge_style_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_edge_style(cfg_frame);
	}
	void ng_playlist_view_t::g_on_use_date_info_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->on_use_date_info_change();
	}
	void ng_playlist_view_t::g_on_time_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->on_time_change();
	}
	void ng_playlist_view_t::on_use_date_info_change()
	{
		if (cfg_playlist_date)
			set_day_timer();
		else
			kill_day_timer();
		update_items(0, get_item_count());
	}
	void ng_playlist_view_t::g_on_show_tooltips_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
		{
			g_windows[i]->set_show_tooltips(cfg_tooltip!=0);
			g_windows[i]->set_limit_tooltips_to_clipped_items(cfg_tooltips_clipped!=0);
		}
	}
	void ng_playlist_view_t::g_on_playback_follows_cursor_change(bool b_val)
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->set_always_show_focus(b_val);
	}
	void ng_playlist_view_t::g_on_columns_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->on_columns_change();
	}
	void ng_playlist_view_t::on_columns_change()
	{
		if (get_wnd())
		{
			clear_all_items();
			refresh_columns();
			populate_list();
		}
	}


	int g_compare_wchar(const pfc::array_t<WCHAR> & a, const pfc::array_t<WCHAR> & b)
	{
		return StrCmpLogicalW(a.get_ptr(), b.get_ptr());
	}
	void ng_playlist_view_t::notify_sort_column(t_size index, bool b_descending, bool b_selection_only)
	{
		unsigned active_playlist = m_playlist_api->get_active_playlist();
		if (active_playlist != -1 && (!m_playlist_api->playlist_lock_is_present(active_playlist) || !(m_playlist_api->playlist_lock_get_filter_mask(active_playlist) & playlist_lock::filter_reorder)))
		{
			unsigned n,count = m_playlist_api->activeplaylist_get_item_count();
			
			pfc::list_t< pfc::array_t<WCHAR>, pfc::alloc_fast_aggressive > data;
			pfc::list_t< t_size, pfc::alloc_fast_aggressive > source_indices;
			data.set_count(count);
			source_indices.prealloc(count);

			pfc::string8_fast_aggressive temp;
			pfc::string8_fast_aggressive temp2;
			temp.prealloc(512);

			bool extra = m_script_global.is_valid() && cfg_global_sort;

			bit_array_bittable mask(count);
			if (b_selection_only)
				m_playlist_api->activeplaylist_get_selection_mask(mask);

			bool date = cfg_playlist_date != 0;
			SYSTEMTIME st;
			if (date) GetLocalTime(&st);

			t_size counter = 0;

			for(n=0;n<count;n++)
			{

				if (!b_selection_only || mask[n] == true)
				{
					global_variable_list extra_items;

					{

					if (extra)
						m_playlist_api->activeplaylist_item_format_title(n, &titleformat_hook_splitter_pt3(&titleformat_hook_set_global<true,false>(extra_items,false), date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), pfc::string8(), m_script_global, 0, play_control::display_level_none);

					m_playlist_api->activeplaylist_item_format_title(n, &titleformat_hook_splitter_pt3(extra ? &titleformat_hook_set_global<false,true>(extra_items, false) : 0 , date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), temp, m_column_data[index].m_sort_script, 0, play_control::display_level_none);
					}

					const char * ptr = temp.get_ptr();
					if (strchr(ptr,3))
					{
						titleformat_compiler::remove_color_marks(ptr,temp2);
						ptr = temp2;
					}

					
					data[counter].set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(ptr));
					pfc::stringcvt::convert_utf8_to_wide_unchecked(data[counter].get_ptr(), ptr);

					counter++;
					if (b_selection_only)
						source_indices.add_item(n);
				}

			}
			data.set_size(counter);

			/*if (descending)
			{
				data.sort(sort_info_callback_base<sort_info*>::desc_sort_callback());
			}
			else
			{
				sort_info_callback_base<sort_info*>::asc_sort_callback cc;
				data.sort(cc);
			}*/

			mmh::permutation_t order(data.get_count());
			g_sort_get_permutation_qsort_v2(data.get_ptr(), order, g_compare_wchar, true, b_descending);

			m_playlist_api->activeplaylist_undo_backup();
			if (b_selection_only)
			{
				mmh::permutation_t order2(count);
				t_size count2 = data.get_count();
				for(n=0;n<count2;n++)
				{
					order2[source_indices[n]] = source_indices[order[n]];
				}
				m_playlist_api->activeplaylist_reorder_items(order2.get_ptr(), count);
			}
			else
				m_playlist_api->activeplaylist_reorder_items(order.get_ptr(), count);
			
			//if (!selection_only)
			{
			}
		}
	}
	void ng_playlist_view_t::notify_on_initialisation()
	{
		set_group_info_area_size(cfg_artwork_width, cfg_artwork_width + (cfg_artwork_reflection ? (cfg_artwork_width*3)/11 : 0) );
		set_show_group_info_area(cfg_show_artwork);
		set_show_header(cfg_header != 0);
		set_autosize(cfg_nohscroll != 0);
		set_always_show_focus(config_object::g_get_data_bool_simple(standard_config_objects::bool_playback_follows_cursor, false));
		set_vertical_item_padding(settings::playlist_view_item_padding);
		LOGFONT lf;
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_items_font, lf);
		set_font(&lf);
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_header_font, lf);
		set_header_font(&lf);
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_group_header_font, lf);
		set_group_font(&lf);
		set_sorting_enabled(cfg_header_hottrack!=0);
		set_show_sort_indicators(cfg_show_sort_arrows!=0);
		set_edge_style(cfg_frame);
		set_show_tooltips(cfg_tooltip!=0);
		set_limit_tooltips_to_clipped_items(cfg_tooltips_clipped!=0);
		set_alternate_selection_model(cfg_alternative_sel != 0);
		set_allow_header_rearrange(true);
		
		m_gdiplus_initialised = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplus_token, &Gdiplus::GdiplusStartupInput(), NULL));
		m_artwork_manager = new artwork_reader_manager_ng_t;
		m_artwork_manager->initialise();
		m_artwork_manager->add_type(album_art_ids::cover_front);
		m_artwork_manager->set_script(album_art_ids::cover_front, artwork_panel::cfg_front_scripts);

		m_playlist_api = standard_api_create_t<playlist_manager>();
		m_playlist_cache.initialise_playlist_callback();
		initialise_playlist_callback(playlist_callback::flag_on_playlist_activate);

		refresh_columns();
		refresh_groups();
	}
	void ng_playlist_view_t::notify_on_create()
	{


		populate_list();

		m_playlist_api->register_callback(static_cast<playlist_callback_single*>(this), playlist_callback::flag_all);

		pfc::com_ptr_t<IDropTarget_playlist> IDT_playlist = new IDropTarget_playlist(this);
		RegisterDragDrop(get_wnd(), IDT_playlist.get_ptr());

		if (g_windows.get_count() == 0)
			g_global_mesage_window.create(NULL);
		g_windows.add_item(this);

		if (cfg_playlist_date)
			set_day_timer();
	}


	void ng_playlist_view_t::notify_on_destroy()
	{
		g_windows.remove_item(this);
		if (g_windows.get_count() == 0)
			g_global_mesage_window.destroy();

		RevokeDragDrop(get_wnd());
		m_playlist_api->unregister_callback(static_cast<playlist_callback_single*>(this));
		m_playlist_cache.deinitialise_playlist_callback();
		deinitialise_playlist_callback();
		m_column_data.remove_all();
		m_script_global.release();
		m_script_global_style.release();
		m_playlist_api.release();
		m_column_mask.set_size(0);

		m_artwork_manager->deinitialise();
		m_artwork_manager.release();

		m_selection_holder.release();

		if (m_gdiplus_initialised)
		{
			Gdiplus::GdiplusShutdown(m_gdiplus_token);
			m_gdiplus_initialised = false;
			m_gdiplus_token = NULL;
		}
	}

	void ng_playlist_view_t::notify_on_set_focus(HWND wnd_lost)
	{
		m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
		m_selection_holder->set_playlist_selection_tracking();
	}
	void ng_playlist_view_t::notify_on_kill_focus(HWND wnd_receiving)
	{
		m_selection_holder.release();
	}
	bool ng_playlist_view_t::notify_on_contextmenu_header(const POINT & pt, const HDHITTESTINFO & hittest)
	{
		uie::window_ptr p_this_temp = this;
		enum {IDM_ASC=1, IDM_DES=2, IDM_SEL_ASC, IDM_SEL_DES, IDM_AUTOSIZE, IDM_PREFS, IDM_EDIT_COLUMN, IDM_ARTWORK, IDM_CUSTOM_BASE};

		HMENU menu = CreatePopupMenu();
		HMENU selection_menu = CreatePopupMenu();
		t_size index = pfc_infinite;
		if (!(hittest.flags & HHT_NOWHERE) && is_header_column_real(hittest.iItem)) 
		{
			index = header_column_to_real_column(hittest.iItem);
			uAppendMenu(menu,(MF_STRING),IDM_ASC,"&Sort ascending");
			uAppendMenu(menu,(MF_STRING),IDM_DES,"Sort &descending");
			uAppendMenu(selection_menu,(MF_STRING),IDM_SEL_ASC,"Sort a&scending");
			uAppendMenu(selection_menu,(MF_STRING),IDM_SEL_DES,"Sort d&escending");
			uAppendMenu(menu,MF_STRING|MF_POPUP,(UINT)selection_menu,"Se&lection");
			uAppendMenu(menu,(MF_SEPARATOR),0,"");
			uAppendMenu(menu,(MF_STRING),IDM_EDIT_COLUMN,"&Edit this column");
			uAppendMenu(menu,(MF_SEPARATOR),0,"");
			uAppendMenu(menu,(MF_STRING|(cfg_nohscroll ? MF_CHECKED : MF_UNCHECKED)),IDM_AUTOSIZE,"&Auto-sizing columns");
			uAppendMenu(menu,(MF_STRING),IDM_PREFS,"&Preferences");
			uAppendMenu(menu,(MF_SEPARATOR),0,"");
			uAppendMenu(menu,(MF_STRING|(cfg_show_artwork ? MF_CHECKED : MF_UNCHECKED)),IDM_ARTWORK,"&Artwork");
			uAppendMenu(menu,(MF_SEPARATOR),0,"");

			pfc::string8 playlist_name;
			static_api_ptr_t<playlist_manager> playlist_api;
			playlist_api->activeplaylist_get_name(playlist_name);

			pfc::string8_fast_aggressive filter, name;

			int s,e=g_columns.get_count();
			for (s=0;s<e;s++)
			{
				bool add = false;
				switch(g_columns[s]->filter_type)
				{
				case FILTER_NONE:
					{
						add = true;
						break;
					}
				case FILTER_SHOW:
					{
						if (wildcard_helper::test(playlist_name,g_columns[s]->filter,true))
						{
							add = true;
						}
					}
					break;
				case FILTER_HIDE:
					{
						if (!wildcard_helper::test(playlist_name,g_columns[s]->filter,true))
						{
							add = true;
						}
					}
					break;
				}
				if (add)
				{
					uAppendMenu(menu,MF_STRING|(g_columns[s]->show ? MF_CHECKED : MF_UNCHECKED),IDM_CUSTOM_BASE+s,g_columns[s]->name);
				}
			}


		}
		else
		{
			uAppendMenu(menu,(MF_STRING|(cfg_nohscroll ? MF_CHECKED : MF_UNCHECKED)),IDM_AUTOSIZE,"&Auto-sizing columns");
			uAppendMenu(menu,(MF_STRING),IDM_PREFS,"&Preferences");
			uAppendMenu(menu,(MF_SEPARATOR),0,"");
			uAppendMenu(menu,(MF_STRING|(cfg_show_artwork ? MF_CHECKED : MF_UNCHECKED)),IDM_ARTWORK,"&Artwork");
		}


		menu_helpers::win32_auto_mnemonics(menu);

		int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,get_wnd(),0);
		DestroyMenu(menu);

		if (cmd == IDM_ASC)
		{

			sort_by_column(index, false);
		}
		else if (cmd == IDM_DES)
		{
			sort_by_column(index, true);
		}
		else if (cmd == IDM_SEL_ASC)
		{
			sort_by_column(index, false, true);
		}
		else if (cmd == IDM_SEL_DES)
		{
			sort_by_column(index, true, true);
		}
		else if (cmd == IDM_EDIT_COLUMN)
		{
			g_set_tab("Columns");
			cfg_cur_prefs_col =  column_index_display_to_actual(index); //get_idx
			static_api_ptr_t<ui_control>()->show_preferences(columns::config_get_playlist_view_guid());
		}
		else if (cmd == IDM_AUTOSIZE)
		{
			cfg_nohscroll = cfg_nohscroll == 0;
			playlist_view::update_all_windows();
			pvt::ng_playlist_view_t::g_on_autosize_change();
		}
		else if (cmd == IDM_PREFS)
		{
			static_api_ptr_t<ui_control>()->show_preferences(columns::config_get_playlist_view_guid());
		}
		else if (cmd == IDM_ARTWORK)
		{
			cfg_show_artwork=!cfg_show_artwork;
			pvt::ng_playlist_view_t::g_on_show_artwork_change();
		}
		else if (cmd >= IDM_CUSTOM_BASE)
		{
			if (t_size(cmd - IDM_CUSTOM_BASE) < g_columns.get_count())
			{
				g_columns[cmd - IDM_CUSTOM_BASE]->show = !g_columns[cmd - IDM_CUSTOM_BASE]->show; //g_columns
				//if (!cfg_nohscroll) 
				//playlist_view::g_save_columns();
				//g_cache.flush_all();
				playlist_view::g_reset_columns();
				playlist_view::update_all_windows();
				pvt::ng_playlist_view_t::g_on_columns_change();
			}

		}
		return true;
	}
	void ng_playlist_view_t::notify_on_menu_select(WPARAM wp, LPARAM lp)
	{
		if (HIWORD(wp) & MF_POPUP)
		{
			m_status_text_override.release();
		}
		else 
		{
			if (m_contextmenu_manager.is_valid() || m_mainmenu_manager.is_valid())
			{
				unsigned id = LOWORD(wp);

				bool set = false;

				pfc::string8 desc;

				if (m_mainmenu_manager.is_valid() && id < m_contextmenu_manager_base)
				{
					set = m_mainmenu_manager->get_description(id - m_mainmenu_manager_base, desc);
				}
				else if (m_contextmenu_manager.is_valid() && id >= m_contextmenu_manager_base)
				{
					contextmenu_node * node = m_contextmenu_manager->find_by_id(id - m_contextmenu_manager_base);
					if (node) set = node->get_description(desc);
				}

				ui_status_text_override::ptr p_status_override;

				if (set)
				{
					get_host()->override_status_text_create(p_status_override);

					if (p_status_override.is_valid())
					{
						p_status_override->override_text(desc);
					}
				}
				m_status_text_override = p_status_override;
			}
		}
	}


	bool ng_playlist_view_t::notify_on_contextmenu(const POINT & pt)
	{
		uie::window_ptr p_this_temp = this;
		if (m_playlist_api->activeplaylist_get_selection_count(1) > 0)
		{
			enum { ID_PLAY=1, ID_CUT, ID_COPY, ID_PASTE, ID_SELECTION, ID_CUSTOM_BASE = 0x8000 };
			HMENU menu = CreatePopupMenu();

			service_ptr_t<mainmenu_manager> p_manager_selection;
			service_ptr_t<contextmenu_manager> p_manager_context;
			p_manager_selection = standard_api_create_t<mainmenu_manager>();
			contextmenu_manager::g_create(p_manager_context);
			if (p_manager_selection.is_valid())
			{
				p_manager_selection->instantiate(mainmenu_groups::edit_part2_selection);
				p_manager_selection->generate_menu_win32(menu,ID_SELECTION,ID_CUSTOM_BASE-ID_SELECTION,standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0);
				if (GetMenuItemCount(menu) > 0) uAppendMenu(menu,MF_SEPARATOR,0,"");
			}

			AppendMenu(menu,MF_STRING,ID_CUT,L"Cut");
			AppendMenu(menu,MF_STRING,ID_COPY,L"Copy");
			if (playlist_utils::check_clipboard())
				AppendMenu(menu,MF_STRING,ID_PASTE,L"Paste");
			AppendMenu(menu,MF_SEPARATOR,0,NULL);
			if (p_manager_context.is_valid())
			{
				const keyboard_shortcut_manager::shortcut_type shortcuts[] = {keyboard_shortcut_manager::TYPE_CONTEXT_PLAYLIST, keyboard_shortcut_manager::TYPE_CONTEXT};
				p_manager_context->set_shortcut_preference(shortcuts, tabsize(shortcuts));
				p_manager_context->init_context_playlist(standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0);

				p_manager_context->win32_build_menu(menu,ID_CUSTOM_BASE,-1);
			}
			menu_helpers::win32_auto_mnemonics(menu);
			m_contextmenu_manager_base = ID_CUSTOM_BASE;
			m_mainmenu_manager_base = ID_SELECTION;

			m_mainmenu_manager = p_manager_selection;
			m_contextmenu_manager = p_manager_context;

			int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,get_wnd(),0);
			
			m_status_text_override.release();

			DestroyMenu(menu);
			if (cmd)
			{
				if (cmd == ID_CUT)
				{
					playlist_utils::cut();
				}
				else if (cmd == ID_COPY)
				{
					playlist_utils::copy();
				}
				else if (cmd == ID_PASTE)
				{
					playlist_utils::paste(get_wnd());
				}
				else if (cmd >= ID_SELECTION && cmd<ID_CUSTOM_BASE)
				{
					if (p_manager_selection.is_valid())
					{
						p_manager_selection->execute_command(cmd - ID_SELECTION);
					}
				}
				else if (cmd >= ID_CUSTOM_BASE)
				{
					if (p_manager_context.is_valid())
					{
						p_manager_context->execute_by_id(cmd - ID_CUSTOM_BASE);
					}
				}
			}
			m_mainmenu_manager.release();
			m_contextmenu_manager.release();
		}
		return true;
	}

	void ng_playlist_view_t::notify_update_item_data(t_size index)
	{
		t_string_list_fast & p_out = get_item_subitems(index);
		item_ng_t * p_item = get_item(index);

		t_size group_index = 0, group_count = 0;
		//t_list_view::get_item_group(index, get_group_count()-1, group_index, group_count);

		pfc::string8_fast_aggressive temp, str_dummy;
		temp.prealloc(32);
		global_variable_list globals;
		bool b_global = m_script_global.is_valid(), b_date=cfg_playlist_date != 0;
		SYSTEMTIME st;
		memset(&st, 0, sizeof(SYSTEMTIME));
		if (b_date) 
			GetLocalTime(&st);

		if (b_global)
			m_playlist_api->activeplaylist_item_format_title(index, &titleformat_hook_splitter_pt3(&titleformat_hook_set_global<true,false>(globals,false), b_date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), str_dummy, m_script_global, 0, play_control::display_level_all);

		style_data_cell_info_t style_data_item = style_data_cell_info_t::g_create_default();

		bool colour_global_av = false;
		t_size i,count = m_column_data.get_count(), count_display_groups=get_item_display_group_count(index);
		p_out.set_count(count);
		get_item(index)->m_style_data.set_count(count);

		metadb_handle_ptr ptr;
		m_playlist_api->activeplaylist_get_item_handle(ptr, index);
			
		t_size item_index = get_item_display_index(index);
		for (i=0; i<count_display_groups; i++)
		{
			t_size count_groups=p_item->get_group_count();
			style_data_cell_info_t style_data_group = style_data_cell_info_t::g_create_default();
			if (ptr.is_valid() &&  m_script_global_style.is_valid())
				//m_playlist_api->activeplaylist_item_format_title(index, 
				//&titleformat_hook_splitter_pt3(&titleformat_hook_style_v2(style_data_group, item_index-i-1, true),b_global ? &titleformat_hook_set_global<false,true>(globals, false) : 0,NULL), temp, m_script_global_style, 0, play_control::display_level_all);
				ptr->format_title(&titleformat_hook_splitter_pt3(&titleformat_hook_style_v2(style_data_group, item_index-i-1, true),b_global ? &titleformat_hook_set_global<false,true>(globals, false) : 0,b_date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), temp, m_script_global_style, NULL);
			//count_display_groups > 0 => count_groups > 0
			style_cache_manager::g_add_object(style_data_group, p_item->get_group(count_groups-i-1)->m_style_data);
		}

		for (i=0;i<count;i++)
		{
			m_playlist_api->activeplaylist_item_format_title(index, &titleformat_hook_splitter_pt3(b_global ? &titleformat_hook_set_global<false,true>(globals, false) :0, b_date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), temp, m_column_data[i].m_display_script, NULL, playback_control::display_level_all);
			p_out[i]=temp;

			style_data_cell_info_t style_temp = style_data_cell_info_t::g_create_default();

			bool b_custom = m_column_data[i].m_style_script.is_valid();

			{
				if (!colour_global_av)
				{
					if (m_script_global_style.is_valid())
						m_playlist_api->activeplaylist_item_format_title(index, 
						&titleformat_hook_splitter_pt3(&titleformat_hook_style_v2(style_data_item, item_index),b_global ? &titleformat_hook_set_global<false,true>(globals, false) : 0,b_date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), temp, m_script_global_style, 0, play_control::display_level_all);

					colour_global_av = true;
				}
				style_temp = style_data_item;
			}
			if (b_custom)
			{
				if (m_column_data[i].m_style_script.is_valid())
					m_playlist_api->activeplaylist_item_format_title(index, 
					&titleformat_hook_splitter_pt3(&titleformat_hook_style_v2(style_temp, item_index),b_global ? &titleformat_hook_set_global<false,true>(globals, false) : 0, b_date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()) 
					, temp, m_column_data[i].m_style_script, 0, play_control::display_level_all);
			}
			style_cache_manager::g_add_object(style_temp, get_item(index)->m_style_data[i]);
			
		}
	}


	const style_data_t & ng_playlist_view_t::get_style_data(t_size index)
	{
		if (get_item(index)->m_style_data.get_count() != get_column_count())
		{
			notify_update_item_data(index);
		}
		return get_item(index)->m_style_data;
	}
	bool ng_playlist_view_t::notify_on_middleclick(bool on_item, t_size index)
	{
		return playlist_mclick_actions::run(cfg_playlist_middle_action, on_item,index);
	}
	bool ng_playlist_view_t::notify_on_doubleleftclick_nowhere()
	{
		if (cfg_playlist_double.get_value().m_command != pfc::guid_null)
			return mainmenu_commands::g_execute(cfg_playlist_double.get_value().m_command);
		return false;
	}
	void ng_playlist_view_t::get_insert_items(/*t_size p_playlist, */ t_size start, t_size count, pfc::list_t<t_list_view::t_item_insert, pfc::alloc_fast_aggressive> & items)
	{
		//profiler(get_insert_items);
		t_size i;
		items.set_count(count);
		pfc::string8_fast_aggressive temp;
		temp.prealloc(128);
		t_size columns_count = m_column_data.get_count();
		t_size groups_count = m_scripts.get_count();
		for (i=0; i<count; i++)
		{
			t_size j;

			items[i].m_groups.set_count(groups_count);
			for (j=0; j<groups_count; j++)
			{
				metadb_handle_ptr ptr;
				if (m_playlist_api->activeplaylist_get_item_handle(ptr, i+start))
					ptr->format_title(NULL, temp, m_scripts[j], NULL);

				items[i].m_groups[j]=temp;
			}
		}
	}

	void ng_playlist_view_t::flush_items()
	{
		pfc::list_t<t_list_view::t_item_insert, pfc::alloc_fast_aggressive> items;
		get_insert_items(0, m_playlist_api->activeplaylist_get_item_count(), items);
		replace_items(0, items);
	}
	void ng_playlist_view_t::reset_items()
	{
		clear_all_items();
		pfc::list_t<t_list_view::t_item_insert, pfc::alloc_fast_aggressive> items;
		get_insert_items(0, m_playlist_api->activeplaylist_get_item_count(), items);
		insert_items(0, items.get_size(), items.get_ptr());
	}

	t_size ng_playlist_view_t::get_highlight_item() 
	{
		if (static_api_ptr_t<play_control>()->is_playing())
		{
			t_size playing_index, playing_playlist;
			m_playlist_api->get_playing_item_location(&playing_playlist, &playing_index);
			if (playing_playlist == m_playlist_api->get_active_playlist())
				return playing_index;
		}
		return pfc_infinite;
	}

	bool ng_playlist_view_t::notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp, bool & b_processed)
	{
		uie::window_ptr p_this = this;
		bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
		b_processed = ret;
		return ret;
	};
	bool ng_playlist_view_t::notify_on_keyboard_keydown_remove()	
	{
		m_playlist_api->activeplaylist_undo_backup();
		m_playlist_api->activeplaylist_remove_selection();
		return true;
	};

	bool ng_playlist_view_t::notify_on_keyboard_keydown_search()
	{
		return standard_commands::main_playlist_search();
	};

	bool ng_playlist_view_t::notify_on_keyboard_keydown_undo()
	{
		m_playlist_api->activeplaylist_undo_restore();
		return true;
	};
	bool ng_playlist_view_t::notify_on_keyboard_keydown_redo()
	{
		m_playlist_api->activeplaylist_redo_restore();
		return true;
	};
	bool ng_playlist_view_t::notify_on_keyboard_keydown_cut()	
	{
		return playlist_utils::cut();
	};
	bool ng_playlist_view_t::notify_on_keyboard_keydown_copy()	
	{
		return playlist_utils::copy();
	};
	bool ng_playlist_view_t::notify_on_keyboard_keydown_paste()	
	{
		return playlist_utils::paste(get_wnd());
	};

	t_size ng_playlist_view_t::storage_get_focus_item()
	{
		return static_api_ptr_t<playlist_manager>()->activeplaylist_get_focus_item();
	}
	void ng_playlist_view_t::storage_set_focus_item(t_size index)
	{
		pfc::vartoggle_t<bool> tog(m_ignore_callback, true);
		static_api_ptr_t<playlist_manager>()->activeplaylist_set_focus_item(index);
	}
	void ng_playlist_view_t::storage_get_selection_state(bit_array_var & out)
	{
		static_api_ptr_t<playlist_manager>()->activeplaylist_get_selection_mask(out);
	}
	bool ng_playlist_view_t::storage_set_selection_state(const bit_array & p_affected,const bit_array & p_status, bit_array_var * p_changed)
	{
		pfc::vartoggle_t<bool> tog(m_ignore_callback, true);

		static_api_ptr_t<playlist_manager> api;

		t_size i, count = api->activeplaylist_get_item_count();
		bit_array_bittable previous_state(count);

		api->activeplaylist_get_selection_mask(previous_state);

		bool b_changed = false;

		for (i=0; i<count; i++)
			if (p_affected.get(i) && ( p_status.get(i) != previous_state.get(i) ) )
			{
				b_changed = true;
				if (p_changed)
					p_changed->set(i, true);
				else break;
			}

		api->activeplaylist_set_selection(p_affected, p_status);
		return b_changed;
	}
	bool ng_playlist_view_t::storage_get_item_selected(t_size index)
	{
		return static_api_ptr_t<playlist_manager>()->activeplaylist_is_item_selected(index);
	}
	t_size ng_playlist_view_t::storage_get_selection_count(t_size max)
	{
		return static_api_ptr_t<playlist_manager>()->activeplaylist_get_selection_count(max);
	}

	void ng_playlist_view_t::execute_default_action (t_size index, t_size column, bool b_keyboard, bool b_ctrl) 
	{
		if (b_keyboard && b_ctrl)
		{
			t_size active = m_playlist_api->get_active_playlist();
			if (active != -1)
				m_playlist_api->queue_add_item_playlist(active, index);
		}
		else
		{
			m_playlist_api->activeplaylist_execute_default_action(index);
		}
	};
	void ng_playlist_view_t::move_selection (int delta)
	{
		m_playlist_api->activeplaylist_undo_backup ();
		m_playlist_api->activeplaylist_move_selection (delta);
	}


	const GUID & ng_playlist_view_t::get_extension_guid() const
	{
		return g_extension_guid;
	}

	void ng_playlist_view_t::get_name(pfc::string_base & out)const
	{
		out.set_string("NG Playlist");
	}
	bool ng_playlist_view_t::get_short_name(pfc::string_base & out)const
	{
		out.set_string("Playlist");
		return true;
	}
	void ng_playlist_view_t::get_category(pfc::string_base & out)const
	{
		out.set_string("Playlist Views");
	}
	unsigned ng_playlist_view_t::get_type() const{return uie::type_panel|uie::type_playlist;}

	// {FB059406-5F14-4bd0-8A11-4242854CBBA5}
	const GUID ng_playlist_view_t::g_extension_guid = 
	{ 0xfb059406, 0x5f14, 0x4bd0, { 0x8a, 0x11, 0x42, 0x42, 0x85, 0x4c, 0xbb, 0xa5 } };

	uie::window_factory<ng_playlist_view_t> g_pvt;

	pfc::ptr_list_t<ng_playlist_view_t> ng_playlist_view_t::g_windows;
	ng_playlist_view_t::ng_global_mesage_window ng_playlist_view_t::g_global_mesage_window;

	// {C882D3AC-C014-44df-9C7E-2DADF37645A0}
	const GUID appearance_client_ngpv_impl::g_guid = 
	{ 0xc882d3ac, 0xc014, 0x44df, { 0x9c, 0x7e, 0x2d, 0xad, 0xf3, 0x76, 0x45, 0xa0 } };
	appearance_client_ngpv_impl::factory<appearance_client_ngpv_impl> g_appearance_client_ngpv_impl;

	// {19F8E0B3-E822-4f07-B200-D4A67E4872F9}
	const GUID g_guid_items_font = 
	{ 0x19f8e0b3, 0xe822, 0x4f07, { 0xb2, 0x0, 0xd4, 0xa6, 0x7e, 0x48, 0x72, 0xf9 } };

	// {30FBD64C-2031-4f0b-A937-F21671A2E195}
	const GUID g_guid_header_font = 
	{ 0x30fbd64c, 0x2031, 0x4f0b, { 0xa9, 0x37, 0xf2, 0x16, 0x71, 0xa2, 0xe1, 0x95 } };

	// {FB127FFA-1B35-4572-9C1A-4B96A5C5D537}
	const GUID g_guid_group_header_font = 
	{ 0xfb127ffa, 0x1b35, 0x4572, { 0x9c, 0x1a, 0x4b, 0x96, 0xa5, 0xc5, 0xd5, 0x37 } };

	class font_client_ngpv : public cui::fonts::client
	{
	public:
		virtual const GUID & get_client_guid() const
		{
			return g_guid_items_font;
		}
		virtual void get_name (pfc::string_base & p_out) const
		{
			p_out = "NG Playlist: Items";
		}

		virtual cui::fonts::font_type_t get_default_font_type() const
		{
			return cui::fonts::font_type_items;
		}

		virtual void on_font_changed() const 
		{
			ng_playlist_view_t::g_on_font_change();
			
		}
	};

	class font_header_client_ngpv : public cui::fonts::client
	{
	public:
		virtual const GUID & get_client_guid() const
		{
			return g_guid_header_font;
		}
		virtual void get_name (pfc::string_base & p_out) const
		{
			p_out = "NG Playlist: Column Titles";
		}

		virtual cui::fonts::font_type_t get_default_font_type() const
		{
			return cui::fonts::font_type_items;
		}

		virtual void on_font_changed() const 
		{
			ng_playlist_view_t::g_on_header_font_change();
			
		}
	};

	class font_group_header_client_ngpv : public cui::fonts::client
	{
	public:
		virtual const GUID & get_client_guid() const
		{
			return g_guid_group_header_font;
		}
		virtual void get_name (pfc::string_base & p_out) const
		{
			p_out = "NG Playlist: Group Titles";
		}

		virtual cui::fonts::font_type_t get_default_font_type() const
		{
			return cui::fonts::font_type_items;
		}

		virtual void on_font_changed() const 
		{
			ng_playlist_view_t::g_on_group_header_font_change();
			
		}
	};

	font_client_ngpv::factory<font_client_ngpv> g_font_client_ngpv;
	font_header_client_ngpv::factory<font_header_client_ngpv> g_font_header_client_ngpv;
	font_group_header_client_ngpv::factory<font_group_header_client_ngpv> g_font_group_header_client_ngpv;

	void appearance_client_ngpv_impl::on_colour_changed(t_size mask) const
	{
		if (cfg_show_artwork && cfg_artwork_reflection && (mask & (cui::colours::colour_flag_background)))
			ng_playlist_view_t::g_flush_artwork();
		ng_playlist_view_t::g_update_all_items();
	}

}
