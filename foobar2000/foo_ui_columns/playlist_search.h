#ifndef _COLUMNS_PLAYLIST_SEARCH_H_
#define _COLUMNS_PLAYLIST_SEARCH_H_

/** What's all this? Never finished, obsolete or abandoned? */

#define SEARCH_CACHING_ENABLED

enum t_search_mode
{
	mode_pattern_beginning = 0,
	mode_query = 1,
};

class progressive_search
{
	bool m_running;
	metadb_handle_list_t<pfc::alloc_fast_aggressive> m_entries;
	service_ptr_t<titleformat_object> m_to;
	pfc::array_t<bool> m_filter;
	pfc::string8_fastalloc m_string;
	pfc::string8_fastalloc m_buffer;
	static_api_ptr_t<playlist_manager> api;
	unsigned active;
	t_uint32 m_mode;
#ifdef SEARCH_CACHING_ENABLED
	pfc::array_t<pfc::string_simple> m_formatted;
#endif
	void run()
	{
		static_api_ptr_t<search_filter_manager> filter_api;
		search_filter::ptr filter;
		bool b_clear = false;
		if (m_mode == mode_query)
		{
			b_clear = false;
			try {
				filter_api->create(m_string);
			}
			catch (const pfc::exception &)
			{
				b_clear = true;
			}
		}
		if (m_string.length() && !b_clear)
		{
			if (m_mode == mode_query)
				static_api_ptr_t<metadb>()->database_lock();
			bool b_first = true;
			unsigned focus;
			unsigned n, count = m_entries.get_count();
			if (m_mode == mode_query)
				filter->test_multi(m_entries, m_filter.get_ptr());

			for (n = 0; n < count; n++)
			{
				if (m_mode == mode_query)
				{
					if (m_filter[n])
					{
						if (b_first)
						{
							focus = n;
							b_first = false;
						}
					}
				}
				else
				{
					if (m_filter[n])
					{
#ifdef SEARCH_CACHING_ENABLED
						if (!stricmp_utf8_max(m_formatted[n], m_string, m_string.length()))
#else
						m_entries[n]->format_title(0, m_buffer, m_to, 0);
						if (!stricmp_utf8_max(m_buffer, m_string, m_string.length()))
#endif
						{
							if (b_first)
							{
								focus = n;
								b_first = false;
							}
						}
						else
							m_filter[n] = false;
					}
				}
			}
			if (m_mode == mode_query)
				static_api_ptr_t<metadb>()->database_unlock();
			api->playlist_set_selection(active, bit_array_true(), bit_array_table_t<bool>(m_filter.get_ptr(), m_filter.get_size()));
			unsigned the_focus = api->playlist_get_focus_item(active);
			if (!b_first && !(the_focus < m_filter.get_size() && m_filter[the_focus]))
			{
				api->playlist_set_focus_item(active, focus);
			}
			else if (!b_first && the_focus != -1) api->playlist_ensure_visible(active, the_focus);
		}
		else
		{
			api->playlist_clear_selection(active);
		}
	}
public:
	void init()
	{
		if (!m_running)
		{
			active = api->get_active_playlist();
			unsigned count = api->playlist_get_item_count(active);
			m_entries.prealloc(count);
			api->playlist_get_all_items(active, m_entries);
			m_filter.set_size(count);
			m_filter.fill(true);
			m_buffer.prealloc(256);
#ifdef SEARCH_CACHING_ENABLED
			unsigned n;
			m_formatted.set_size(count);
			if (m_mode == mode_pattern_beginning)
			{
				for (n = 0; n < count; n++)
				{
					m_entries[n]->format_title(0, m_buffer, m_to, 0);
					m_formatted[n] = m_buffer;
				}
			}
#endif
		}
		m_running = true;
	}
	void reset()
	{
		m_running = false;
		m_filter.set_size(0);
		m_entries.remove_all();
		m_string.force_reset();
		m_buffer.force_reset();
#ifdef SEARCH_CACHING_ENABLED
		m_formatted.set_size(0);
#endif
	}
	void add_char(unsigned c)
	{
		//hires_timer timer;
		//timer.start();
		init();
		m_string.add_char(c);
		run();
		//console::info(string_printf("search time: %u",(unsigned)(timer.query()*100000)));
	}
	void set_string(const char * src)
	{
		if (m_running) m_filter.fill(true);
		init();
		m_string.set_string(src);
		run();
	}
	void set_pattern(const char * src)
	{
		static_api_ptr_t<titleformat_compiler> tf_api;
		tf_api->compile(m_to, src);
	}
	void set_mode(t_uint32 mode)
	{
		m_mode = mode;
	}
	bool on_key(WPARAM wp)
	{
		switch (wp)
		{
		case VK_RETURN:
			if (m_running)
			{
				bool ctrl_down = 0 != (GetKeyState(VK_CONTROL) & KF_UP);
				if (ctrl_down)
				{
					metadb_handle_ptr mh;
					t_size focus = api->playlist_get_focus_item(active);
					if (focus != pfc_infinite) api->queue_add_item_playlist(active, focus);
				}
				else
				{
					api->set_playing_playlist(active);
					static_api_ptr_t<play_control> play_control;
					play_control->play_start(play_control::track_command_settrack);
				}
			}
			return true;
		case VK_DOWN:
		{
			if (m_running)
			{
				unsigned focus = api->playlist_get_focus_item(active);
				unsigned count = m_filter.get_size();
				unsigned n;
				for (n = focus + 1; n < count; n++)
				{
					if (m_filter[n])
					{
						api->playlist_set_focus_item(active, n);
						break;
					}
				}
			}
		}
		return true;
		case VK_UP:
		{
			if (m_running)
			{
				unsigned focus = api->playlist_get_focus_item(active);
				unsigned count = m_filter.get_size();
				unsigned n;
				for (n = focus; (n>0) && (n - 1 < count); n--)
				{
					if (m_filter[n - 1])
					{
						api->playlist_set_focus_item(active, n - 1);
						break;
					}
				}
			}
		}
		return true;
		}
		return false;
	}
	progressive_search() : m_running(false), active(0), m_mode(mode_pattern_beginning)
	{
	}
	~progressive_search()
	{
	}
};

class quickfind_window : public ui_helpers::container_window
{
	bool m_initialised;
	WNDPROC m_editproc;

	bool m_is_running;

	progressive_search m_search;

	unsigned height;
	pfc::string8 m_pattern;
	t_uint32 m_mode;

protected:
	HWND wnd_edit;
	HWND wnd_prev;

public:

	LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

	LRESULT WINAPI on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT WINAPI hook_proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

	quickfind_window();

	~quickfind_window();

	virtual class_data & get_class_data()const
	{
		__implement_get_class_data(_T("{89A3759F-348A-4e3f-BF43-3D16BC059186}"), true);
	}

	void on_size(unsigned cx, unsigned cy);
	void on_size();

private:
	gdi_object_t<HFONT>::ptr_t m_font;
};

#endif _COLUMNS_PLAYLIST_SEARCH_H_