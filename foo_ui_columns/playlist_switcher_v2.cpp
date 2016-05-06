#include "stdafx.h"
#include "playlist_switcher_v2.h"

// {70A5C273-67AB-4bb6-B61C-F7975A6871FD}
const GUID playlist_switcher_t::g_guid_font = 
{ 0x70a5c273, 0x67ab, 0x4bb6, { 0xb6, 0x1c, 0xf7, 0x97, 0x5a, 0x68, 0x71, 0xfd } };

pfc::ptr_list_t<playlist_switcher_t> playlist_switcher_t::g_windows;

void playlist_switcher_t::get_insert_items (t_size base, t_size count, pfc::list_t<t_list_view::t_item_insert> & p_out)
	{
		p_out.set_count(count);

		t_size i;
		for (i=0;i<count; i++)
		{
			p_out[i].m_subitems.set_count(1);
			pfc::string8 temp;
			m_playlist_api->playlist_get_name(i+base, temp);
			p_out[i].m_subitems[0].set_string( playlist_format_name_t(i+base, temp, get_playing_playlist()) );
		}
	}

	void playlist_switcher_t::refresh_all_items()
	{
		remove_items(bit_array_true(), false);

		add_items(0, m_playlist_api->get_playlist_count());

		t_size index = m_playlist_api->get_active_playlist();
		if (index != pfc_infinite)
			set_item_selected_single(index, false);
	}

	void playlist_switcher_t::refresh_items (t_size base, t_size count, bool b_update)
	{
		pfc::list_t<t_list_view::t_item_insert> items_insert;
		get_insert_items(base, count, items_insert);
		replace_items(base, items_insert, b_update);
	}

	void playlist_switcher_t::add_items (t_size base, t_size count)
	{
		pfc::list_t<t_list_view::t_item_insert> items_insert;
		get_insert_items(base, count, items_insert);
		insert_items(base, items_insert.get_count(), items_insert.get_ptr());
	}

	void playlist_switcher_t::refresh_columns()
	{
		set_columns(pfc::list_single_ref_t<t_column>(t_column("Name", 100)));
	}

	void playlist_switcher_t::move_selection (int delta) 
	{
		t_size count = m_playlist_api->get_playlist_count();
		order_helper order(count);
		{
			t_size from = get_selected_item_single();
			if (from != pfc_infinite)
			{
				t_size to = from;
				if (delta)
				{
					if (delta > 0)
						while (delta && to+1  < count)
						{
							order.swap(from,from+1);
							to++;
							delta--;
						}
					else if (delta < 0)
						while (delta && to > 0)
						{
							order.swap(from,from-1);
							to--;
							delta++;
						}
					{
						m_playlist_api->reorder(order.get_ptr(),count);
					}
				}
			}
		}
	}
	void playlist_switcher_t::g_on_edgestyle_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
		{
			g_windows[i]->set_edge_style(cfg_plistframe);
		}
	}
	void playlist_switcher_t::g_on_vertical_item_padding_change()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
		{
			g_windows[i]->set_vertical_item_padding(settings::playlist_switcher_item_padding);
		}
	}
	void playlist_switcher_t::g_redraw_all()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			RedrawWindow(g_windows[i]->get_wnd(), NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
	}
	void playlist_switcher_t::g_refresh_all_items()
	{
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
			g_windows[i]->refresh_all_items();
	}
	void playlist_switcher_t::g_on_font_items_change()
	{
		LOGFONT lf;
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_font, lf);
		t_size i, count = g_windows.get_count();
		for (i=0; i<count; i++)
		{
			g_windows[i]->set_font(&lf);
		}
	}
	void playlist_switcher_t::notify_on_initialisation() 
	{
		set_autosize(true);
		set_single_selection(true);
		set_show_header(false);
		set_edge_style(cfg_plistframe);
		set_vertical_item_padding(settings::playlist_switcher_item_padding);

		LOGFONT lf;
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_font, lf);
		set_font(&lf);
	};
	void playlist_switcher_t::notify_on_create()
	{
		m_playlist_api = standard_api_create_t<playlist_manager_v3>();
		m_playback_api = standard_api_create_t<playback_control>();

		m_playing_playlist = get_playing_playlist();

		refresh_columns();

		refresh_all_items();

		m_playlist_api->register_callback(this, playlist_callback::flag_all);
		standard_api_create_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_all, false);

		pfc::com_ptr_t<IDropTarget_t> drop_target = new IDropTarget_t(this);
		RegisterDragDrop(get_wnd(), drop_target.get_ptr());

		g_windows.add_item(this);
	}
	void playlist_switcher_t::notify_on_destroy()
	{
		m_selection_holder.release();

		g_windows.remove_item(this);

		RevokeDragDrop(get_wnd());

		standard_api_create_t<play_callback_manager>()->unregister_callback(this);
		m_playlist_api->unregister_callback(this);
		m_playlist_api.release();
		m_playback_api.release();
	}


namespace
{
	uie::window_factory<playlist_switcher_t> g_playlist_switcher;
}