#include "foo_ui_columns.h"

HRESULT g_get_comctl32_vresion(DLLVERSIONINFO2 & p_dvi);

namespace filter_panel {

	/*// {B3F4431D-C498-45f8-BCC6-387D2FA063F8}
	const GUID g_guid_cfg_fields = 
	{ 0xb3f4431d, 0xc498, 0x45f8, { 0xbc, 0xc6, 0x38, 0x7d, 0x2f, 0xa0, 0x63, 0xf8 } };*/

	// {E9F1557A-4650-4b59-ACD6-B2AD6EE9B87D}
	const GUID g_guid_cfg_sort = 
	{ 0xe9f1557a, 0x4650, 0x4b59, { 0xac, 0xd6, 0xb2, 0xad, 0x6e, 0xe9, 0xb8, 0x7d } };

	// {4B39F556-9D06-464e-AD16-0F5FA02CF510}
	const GUID g_guid_cfg_sort_string = 
	{ 0x4b39f556, 0x9d06, 0x464e, { 0xad, 0x16, 0xf, 0x5f, 0xa0, 0x2c, 0xf5, 0x10 } };

	// {A5EDCFEC-897D-4101-AB8A-39ED80073AD1}
	const GUID g_guid_cfg_autosend = 
	{ 0xa5edcfec, 0x897d, 0x4101, { 0xab, 0x8a, 0x39, 0xed, 0x80, 0x7, 0x3a, 0xd1 } };

	// {FF6EA2BC-8D58-4a1f-AB13-BC0E4E6F8A6D}
	const GUID g_guid_doubleclickaction = 
	{ 0xff6ea2bc, 0x8d58, 0x4a1f, { 0xab, 0x13, 0xbc, 0xe, 0x4e, 0x6f, 0x8a, 0x6d } };

	// {8AC152D3-1756-4c46-B43B-A42F8100FD2D}
	const GUID g_guid_middleclickaction = 
	{ 0x8ac152d3, 0x1756, 0x4c46, { 0xb4, 0x3b, 0xa4, 0x2f, 0x81, 0x0, 0xfd, 0x2d } };

	// {D93F1EF3-4AEE-4632-B5BF-0220CEC76DED}
	const GUID g_guid_filter_items_font_client = 
	{ 0xd93f1ef3, 0x4aee, 0x4632, { 0xb5, 0xbf, 0x2, 0x20, 0xce, 0xc7, 0x6d, 0xed } };

	// {FCA8752B-C064-41c4-9BE3-E125C7C7FC34}
	const GUID g_guid_filter_header_font_client = 
	{ 0xfca8752b, 0xc064, 0x41c4, { 0x9b, 0xe3, 0xe1, 0x25, 0xc7, 0xc7, 0xfc, 0x34 } };

	// {0078AB6A-26F7-4e5c-A859-ECD88740BBE6}
	const GUID guid_cfg_fields = 
	{ 0x78ab6a, 0x26f7, 0x4e5c, { 0xa8, 0x59, 0xec, 0xd8, 0x87, 0x40, 0xbb, 0xe6 } };

	// {1FEA2799-9D1C-4b42-AF83-E51772585593}
	const GUID g_guid_edgestyle = 
	{ 0x1fea2799, 0x9d1c, 0x4b42, { 0xaf, 0x83, 0xe5, 0x17, 0x72, 0x58, 0x55, 0x93 } };

	// {196FC1C1-388A-4fd8-9CE1-3320EAA0B4DC}
	const GUID g_guid_orderedbysplitters = 
	{ 0x196fc1c1, 0x388a, 0x4fd8, { 0x9c, 0xe1, 0x33, 0x20, 0xea, 0xa0, 0xb4, 0xdc } };

	// {CA5D2277-F09A-4262-BACE-279192ABAF7E}
	const GUID g_guid_showemptyitems = 
	{ 0xca5d2277, 0xf09a, 0x4262, { 0xba, 0xce, 0x27, 0x91, 0x92, 0xab, 0xaf, 0x7e } };

	// {580CC260-119B-4743-AC96-F395D46CC990}
	const GUID g_guid_itempadding = 
	{ 0x580cc260, 0x119b, 0x4743, { 0xac, 0x96, 0xf3, 0x95, 0xd4, 0x6c, 0xc9, 0x90 } };

	// {1002788F-898B-46f5-B6AD-179076328078}
	const GUID g_guid_favouritequeries = 
	{ 0x1002788f, 0x898b, 0x46f5, { 0xb6, 0xad, 0x17, 0x90, 0x76, 0x32, 0x80, 0x78 } };

	// {D88E24F5-2690-4daa-A44A-ABBD74D7C462}
	const GUID g_guid_showsearchclearbutton = 
	{ 0xd88e24f5, 0x2690, 0x4daa, { 0xa4, 0x4a, 0xab, 0xbd, 0x74, 0xd7, 0xc4, 0x62 } };

	cfg_fields_t cfg_field_list(guid_cfg_fields);

	//cfg_string cfg_fields(g_guid_cfg_fields, "Genre;Artist;Album");
	cfg_string cfg_sort_string(g_guid_cfg_sort_string, "%album artist% - %album% - %discnumber% - %tracknumber% - %title%");
	cfg_bool cfg_sort(g_guid_cfg_sort, true);
	cfg_bool cfg_autosend(g_guid_cfg_autosend, true);
	cfg_bool cfg_orderedbysplitters(g_guid_orderedbysplitters, true);
	cfg_bool cfg_showemptyitems(g_guid_showemptyitems, false);
	cfg_int cfg_doubleclickaction(g_guid_doubleclickaction, 1);
	cfg_int cfg_middleclickaction(g_guid_middleclickaction, 0);
	cfg_int cfg_edgestyle(g_guid_edgestyle, 2);
	cfg_int cfg_itempadding(g_guid_itempadding, 4);

	cfg_bool cfg_showsearchclearbutton(g_guid_showsearchclearbutton, true);

	class cfg_favouriteslist : public cfg_var, public pfc::list_t<pfc::string8>
	{
	public:
		void get_data_raw(stream_writer * p_stream,abort_callback & p_abort) {
			t_uint32 n, m = pfc::downcast_guarded<t_uint32>(get_count()), v=0;
			p_stream->write_lendian_t(v,p_abort);
			p_stream->write_lendian_t(m,p_abort);
			for(n=0;n<m;n++) p_stream->write_string(get_item(n),p_abort);
		}
		void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort) {
			t_uint32 n,count,version;
			p_stream->read_lendian_t(version,p_abort);
			if (version <= 0)
			{
				p_stream->read_lendian_t(count,p_abort);
				pfc::string8_fast_aggressive temp;
				temp.prealloc(32);
				for(n=0;n<count;n++) 
				{
					p_stream->read_string(temp,p_abort);
					add_item(temp);
				}
			}
		}

		bool have_item(const char * p_item) 
		{
			t_size i, count = get_count();
			for (i=0; i<count; i++)
				if (!strcmp(p_item, get_item(i))) return true;
			return false;
		}

		bool find_item(const char * p_item, t_size & index) 
		{
			t_size i, count = get_count();
			for (i=0; i<count; i++)
				if (!strcmp(p_item, get_item(i))) {index = i; return true;}
			return false;
		}

	public:
		cfg_favouriteslist(const GUID & p_guid) : cfg_var(p_guid) {}
	};
	
	cfg_favouriteslist cfg_favourites(g_guid_favouritequeries);

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
				index_insert = index_old < playlist ? playlist : playlist+1;
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
			static_api_ptr_t<titleformat_compiler>()->compile_safe(to,cfg_sort_string);
			{
				mmh::fb2k::g_sort_metadb_handle_list_by_format_v2(handles, to, NULL);
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
	class filter_search_bar : 
		public uie::container_uie_window_v2
	{
		class menu_node_show_clear_button : public ui_extension::menu_node_command_t
		{
			service_ptr_t<filter_search_bar> p_this;
		public:
			virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags) const
			{
				p_out = "Show clear button";
				p_displayflags= p_this->m_show_clear_button ? menu_node_t::state_checked : NULL;
				return true;
			}
			virtual bool get_description(pfc::string_base & p_out) const
			{
				return false;
			}
			virtual void execute()
			{
				p_this->m_show_clear_button = !p_this->m_show_clear_button;
				p_this->on_show_clear_button_change();
			}
			menu_node_show_clear_button(filter_search_bar * wnd) : p_this(wnd) {};
		};
	public:
		static bool g_activate();
		//static void g_on_orderedbysplitters_change();

		template <class TStream>
		static void g_initialise_filter_stream(const TStream &);

		virtual void get_name(pfc::string_base & out) const {out = "Filter search";}
		virtual const GUID & get_extension_guid() const {return cui::toolbars::guid_filter_search_bar;}
		virtual void get_category(pfc::string_base & out) const {out="Toolbars";}
		virtual unsigned get_type() const {return uie::type_toolbar;}
		//virtual HBRUSH get_class_background() const {return HBRUSH(COLOR_WINDOW+1);}
		virtual t_uint32 get_flags() const {return flag_default_flags_plus_transparent_background;}
		
		filter_search_bar() : m_search_editbox(NULL), m_favourite_state(false),
			m_query_timer_active(false), m_wnd_last_focused(NULL), m_imagelist(NULL),
			m_combo_cx(0), m_combo_cy(0), m_toolbar_cx(0), m_toolbar_cy(0), m_show_clear_button(cfg_showsearchclearbutton) {};

	private:
		virtual const GUID & get_class_guid() {return cui::toolbars::guid_filter_search_bar;}

		virtual void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort);
		virtual void get_config(stream_writer * p_writer, abort_callback & p_abort) const;
		virtual void get_menu_items (uie::menu_hook_t & p_hook);

		void on_show_clear_button_change();
		void on_search_editbox_change();
		void commit_search_results(const char * str, bool b_force_autosend = false, bool b_stream_update = false);

		void update_favourite_icon(const char * p_new = NULL);

		virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
		void create_edit();
		void on_size (t_size cx, t_size cy);
		void activate();

		static LRESULT WINAPI g_on_search_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
		LRESULT on_search_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

		using uie::container_uie_window_v2::on_size;

		enum {id_edit=668, id_toolbar};
		enum {TIMER_QUERY=1001};
		enum {idc_clear = 1001, idc_favourite=1002, msg_favourite_selected = WM_USER+2};
		enum {config_version_current=0};

		HWND m_search_editbox, m_wnd_toolbar;
		gdi_object_t<HFONT>::ptr_t m_font;
		WNDPROC m_proc_search_edit;
		bool m_favourite_state, m_query_timer_active, m_show_clear_button;
		pfc::string8 m_active_search_string;
		metadb_handle_list m_active_handles;
		HWND m_wnd_last_focused;
		HIMAGELIST m_imagelist;

		int m_combo_cx, m_combo_cy, m_toolbar_cx, m_toolbar_cy;

		static pfc::ptr_list_t<filter_search_bar> g_active_instances;
	};

	class filter_panel_t : 
		public uie::container_ui_extension_t<t_list_view, uie::window>, 
		private mmh::fb2k::library_callback_t
	{
	public:
		class filter_stream_t : public pfc::refcounted_object_root
		{
		public:
			typedef filter_stream_t self_t;
			typedef pfc::refcounted_object_ptr_t<self_t> ptr;
			/** Unordered */
			pfc::ptr_list_t<filter_panel_t> m_windows;
			//bool m_ordered_by_splitter;

			bool m_source_overriden;
			metadb_handle_list m_source_handles;

			bool is_visible() 
			{
				for (t_size i = 0, count = m_windows.get_count(); i<count; i++)
					if (!m_windows[i]->is_visible()) return false;
				return true;
			}

			filter_stream_t() : m_source_overriden(false) {/*filter_search_bar::g_initialise_filter_stream(this);*/};
		};

		friend class filter_search_bar;

		enum {TIMER_QUERY = TIMER_BASE};

		filter_panel_t() :
#ifdef FILTER_OLD_SEARCH
		m_query_active(false), m_query_timer_active(false), 
#endif
		m_show_search(false), m_contextmenu_manager_base(NULL) {};

		~filter_panel_t() {};

		static pfc::list_t<filter_stream_t::ptr> g_streams;
		filter_stream_t::ptr m_stream;

		static pfc::ptr_list_t<filter_panel_t> g_windows;

		virtual const GUID & get_extension_guid() const;
		virtual void get_name(pfc::string_base & out)const;
		virtual void get_category(pfc::string_base & out)const;
		unsigned get_type () const;


		enum {config_version_current=1};
		virtual void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort)
		{
			if (p_size)
			{
				t_size version;
				p_reader->read_lendian_t(version, p_abort);
				if (version <= config_version_current)
				{
					p_reader->read_string(m_field_data.m_name, p_abort);
					if (version >= 1)
					{
						p_reader->read_lendian_t(m_show_search, p_abort);
					}
				}
			}
		};
		virtual void get_config(stream_writer * p_writer, abort_callback & p_abort) const 
		{
			p_writer->write_lendian_t(t_size(config_version_current), p_abort);
			p_writer->write_string(m_field_data.m_name, p_abort);
			p_writer->write_lendian_t(m_show_search, p_abort);
			/*p_writer->write_lendian_t(m_field_data.m_use_script, p_abort);
			p_writer->write_string(m_field_data.m_script, p_abort);
			t_uint32 count=m_field_data.m_fields.get_count(), i;
			p_writer->write_lendian_t(count, p_abort);
			for (i=0; i<count; i++)
			p_writer->write_string(m_field_data.m_fields[i], p_abort);*/
		};
		static t_size g_get_stream_index_by_window(const uie::window_ptr & ptr)
		{
			t_size i, count = g_streams.get_count();
			for (i=0; i<count; i++)
			{
				t_size j, subcount=g_streams[i]->m_windows.get_count();
				for (j=0; j<subcount; j++)
					if (g_streams[i]->m_windows[j] == ptr.get_ptr())
						return i;
			}
			return pfc_infinite;
		}
		static void g_on_orderedbysplitters_change()
		{
			g_streams.remove_all();
			t_size i, count = g_windows.get_count();
			for (i=0; i<count; i++)
			{
				g_windows[i]->refresh_stream();
			}
			//filter_search_bar::g_on_orderedbysplitters_change();
			count = g_streams.get_count();
			for (i=0; i<count; i++)
			{
				if (g_streams[i]->m_windows.get_count())
				{
					filter_search_bar::g_initialise_filter_stream(g_streams[i]);
					pfc::list_t<filter_panel_t*> windows;
					g_streams[i]->m_windows[0]->get_windows(windows);
					if (windows.get_count())
						g_update_subsequent_filters(windows, 0, false, false);
				}
			}
		}
		static void g_on_fields_change()
		{
			g_load_fields();
			t_size i, count = g_windows.get_count();
			for (i=0; i<count; i++)
			{
				t_size field_index = g_windows[i]->get_field_index();
				g_windows[i]->set_field(field_index == pfc_infinite ? field_data_t() : g_field_data[field_index], true);
			}
		}
		static t_size g_get_field_index_by_name(const char * p_name)
		{
			t_size i, count = g_field_data.get_count();
			for (i=0; i<count; i++)
			{
				if (!strcmp(g_field_data[i].m_name, p_name))
					return i;
			}
			return pfc_infinite;
		}

		static void g_on_field_title_change(const char * p_old, const char * p_new)
		{
			t_size field_index  = g_get_field_index_by_name(p_old);
			if (field_index != pfc_infinite)
			{
				t_size i, count = g_windows.get_count();
				for (i=0; i<count; i++)
				{
					if (g_windows[i]->get_field_index() == field_index)
					{
						g_windows[i]->m_field_data.m_name = p_new;
						g_windows[i]->refresh_columns();
						g_windows[i]->update_first_node_text(true);
					}
				}
				g_field_data[field_index].m_name = p_new;
			}
		}
		static void g_on_vertical_item_padding_change()
		{
			t_size i, count = g_windows.get_count();
			for (i=0; i<count; i++)
			{
				g_windows[i]->set_vertical_item_padding(cfg_itempadding);
			}
		}
		static void g_on_field_query_change(const field_t & field)
		{
			t_size field_index  = g_get_field_index_by_name(field.m_name);
			if (field_index != pfc_infinite)
			{
				g_create_field_data(field, g_field_data[field_index]);
				t_size i, count = g_streams.get_count();
				for (i=0; i<count; i++)
				{
					if (g_streams[i]->m_windows.get_count())
					{
						pfc::list_t<filter_panel_t*> windows;
						g_streams[i]->m_windows[0]->get_windows(windows);
						t_size j, subcount=windows.get_count();

						for (j=0; j<subcount; j++)
						{
							if (windows[j]->get_field_index() == field_index) //meh
							{
								windows[j]->set_field(g_field_data[field_index], true);
								break;
							}
						}
					}
				}
			}
		}
		static void g_on_showemptyitems_change(bool b_val)
		{
			if (g_windows.get_count())
			{
				g_showemptyitems = b_val;
				t_size i, count = g_streams.get_count();
				for (i=0; i<count; i++)
				{
					if (g_streams[i]->m_windows.get_count())
					{
						pfc::list_t<filter_panel_t*> windows;
						g_streams[i]->m_windows[0]->get_windows(windows);
						t_size j=windows.get_count();
						if (windows.get_count())
							g_update_subsequent_filters(windows, 0, false, false);

					}
				}
			}
		}
		static void g_on_edgestyle_change()
		{
			t_size i, count = g_windows.get_count();
			for (i=0; i<count; i++)
			{
				g_windows[i]->set_edge_style(cfg_edgestyle);
			}
		}
		static void g_on_font_items_change()
		{
			LOGFONT lf;
			static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_filter_items_font_client, lf);
			t_size i, count = g_windows.get_count();
			for (i=0; i<count; i++)
			{
				g_windows[i]->set_font(&lf);
			}
		}

		static void g_on_font_header_change()
		{
			LOGFONT lf;
			static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_filter_header_font_client, lf);
			t_size i, count = g_windows.get_count();
			for (i=0; i<count; i++)
			{
				g_windows[i]->set_header_font(&lf);
			}
		}
		static void g_redraw_all()
		{
			t_size i, count = g_windows.get_count();
			for (i=0; i<count; i++)
				RedrawWindow(g_windows[i]->get_wnd(), NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		}
		static void g_on_new_field(const field_t & field)
		{
			if (g_windows.get_count())
			{
				t_size index = g_field_data.get_count();
				g_field_data.set_count(index+1);
				g_create_field_data(field, g_field_data[index]);
			}
		}
		static void g_on_fields_swapped(t_size index_1, t_size index_2)
		{
			if (max (index_1,index_2) < g_field_data.get_count())
				g_field_data.swap_items(index_1, index_2);
			if (!cfg_orderedbysplitters)
			{
				t_size i, count = g_streams.get_count();
				for (i=0; i<count; i++)
				{
					if (g_streams[i]->m_windows.get_count())
					{
						pfc::list_t<filter_panel_t*> windows;
						g_streams[i]->m_windows[0]->get_windows(windows);
						t_size j, subcount=windows.get_count();
						for (j=0; j<subcount; j++)
						{
							t_size this_index = windows[j]->get_field_index();
							if (this_index == index_1 || this_index == index_2)
							{
								g_update_subsequent_filters(windows, j, false, false);
								break;
							}
							if (this_index > max(index_1, index_2))
								break;
						}
					}
				}
			}
		}
		static void g_on_field_removed(t_size index)
		{
			t_size i, count = g_streams.get_count();
			for (i=0; i<count; i++)
			{
				if (g_streams[i]->m_windows.get_count())
				{
					pfc::list_t<filter_panel_t*> windows;
					g_streams[i]->m_windows[0]->get_windows(windows);
					t_size j, subcount=windows.get_count();
					bool b_found = false; t_size index_found = pfc_infinite;
					for (j=0; j<subcount; j++)
					{
						t_size this_index = windows[j]->get_field_index();
						if (index == this_index)
						{
							windows[j]->set_field(field_data_t());
							if (!b_found)
							{
								index_found = j;
								b_found = true;
							}
						}
						if (this_index > index)
							break;
					}
					if (b_found)
						g_update_subsequent_filters(windows, index_found, false, false);
				}
			}
			if (index < g_field_data.get_count())
				g_field_data.remove_by_idx(index);
		}

		void refresh(bool b_allow_autosend = true)
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
			get_initial_handles(data);
			populate_list_from_chain(data, false);
			update_subsequent_filters(b_allow_autosend);
		}

	private:
		static const GUID g_extension_guid;

		class field_data_t
		{
		public:
			bool m_use_script;
			pfc::string8 m_script, m_name;
			pfc::list_t<pfc::string8> m_fields;

			field_data_t() : m_use_script(false) {};

			bool is_empty() {return !m_use_script && !m_fields.get_count();}
			void reset() {*this = field_data_t();}
		};

		static pfc::list_t<field_data_t> g_field_data;

		field_data_t m_field_data;

		t_size get_field_index()
		{
			t_size i, count = g_field_data.get_count(), ret = pfc_infinite;
			for (i=0; i<count; i++)
				if (!stricmp_utf8(g_field_data[i].m_name, m_field_data.m_name))
				{
					ret = i;
					break;
				}
				return ret;
		}

		void get_windows(pfc::list_base_t<filter_panel_t*> & windows)
		{
			if (cfg_orderedbysplitters)
			{
				pfc::list_t<uie::window_ptr> siblings;
				uie::window_host_ex::ptr hostex;
				if (get_host()->service_query_t(hostex))
					hostex->get_children(siblings);
				else
					siblings.add_item(this);
				pfc::list_t<t_size> indices;
				t_size i, count = siblings.get_count();
				for (i=0; i<count; i++)
				{
					t_size index = pfc_infinite;
					if ((index = m_stream->m_windows.find_item(static_cast<filter_panel_t*>(siblings[i].get_ptr()))) != pfc_infinite) //meh
						windows.add_item(m_stream->m_windows[index]);
				}
			}
			else
			{
				pfc::list_t<t_size> indices;
				t_size i, count = m_stream->m_windows.get_count();
				for (i=0; i<count; i++)
				{
					t_size index = m_stream->m_windows[i]->get_field_index();
					if (index != pfc_infinite)
					{
						indices.add_item(index);
						windows.add_item(m_stream->m_windows[i]);
					}
				}
				mmh::permutation_t permutation(windows.get_count());
				mmh::g_sort_get_permutation_qsort_v2(indices.get_ptr(), permutation, (pfc::compare_t<t_size,t_size>), true, false);
				windows.reorder(permutation.get_ptr());
			}
		}

		static void g_create_field_data(const field_t & field, field_data_t & p_out)
		{
			if (strchr(field.m_field, '$') || strchr(field.m_field, '%'))
			{
				p_out.m_use_script = true;
				p_out.m_script = field.m_field;
				p_out.m_fields.remove_all();
				p_out.m_name = field.m_name;
			}
			else
			{
				p_out.m_use_script = false;
				p_out.m_script.reset();
				p_out.m_fields.remove_all();
				const char * ptr = field.m_field;
				while (*ptr)
				{
					const char * start = ptr;
					while (*ptr && *ptr != ';')
						ptr++;
					if (ptr>start)
						p_out.m_fields.add_item(pfc::string8(start, ptr-start));
					while (*ptr == ';') ptr++;
				}
				p_out.m_name = field.m_name;
			}
		}

		static void g_load_fields()
		{
			t_size i, count = filter_panel::cfg_field_list.get_count();
			g_field_data.set_count(count);
			for (i=0; i<count; i++)
			{
				const field_t & field = filter_panel::cfg_field_list[i];
				g_create_field_data(field, g_field_data[i]);
			}
		}
		static void g_update_subsequent_filters(const pfc::list_base_const_t<filter_panel_t* > & windows, t_size index, bool b_check_needs_update = false, bool b_update_playlist = true)
		{
			t_size i, count = windows.get_count();
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;

			{
				//pfc::hires_timer timer;
				//timer.start();
				for (i=index; i<count; i++)
				{
					handles.remove_all();
					windows[i]->get_initial_handles(handles);
					if (b_check_needs_update)
					{

						metadb_handle_list items1(windows[i]->m_nodes[0].m_handles);
						handles.sort_by_pointer();
						items1.sort_by_pointer();
						if (!pfc::comparator_array<>::compare(items1, handles))
						{
							b_update_playlist = false;
							break;
						}

					}
					windows[i]->populate_list_from_chain(handles, false);
				}
				//console::formatter() << "total populate: " << timer.query() << " s";
			}
			{
				//pfc::hires_timer timer;
				//timer.start();
				if (count && b_update_playlist && cfg_autosend)
					windows[count-1]->send_results_to_playlist();
				//console::formatter() << "send: " << timer.query() << " s";
			}
		}

		void update_subsequent_filters(bool b_allow_autosend = true)
		{
			pfc::ptr_list_t<filter_panel_t> windows;
			get_windows(windows);
			t_size pos = windows.find_item(this);

			if (pos != pfc_infinite)
			{
				g_update_subsequent_filters(windows, pos+1, false, b_allow_autosend);
			}
		}

		void get_initial_handles(metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_out)
		{
			pfc::ptr_list_t<filter_panel_t> windows;
			get_windows(windows);
			t_size pos = windows.find_item(this);
			if (pos && pos != pfc_infinite)
			{
				windows[pos-1]->get_selection_handles(p_out);
			}
			else
			{
				if (m_stream->m_source_overriden)
					p_out = m_stream->m_source_handles;
				else
					static_api_ptr_t<library_manager>()->get_all_items(p_out);
			}
		}

		void set_field (const field_data_t & field, bool b_force = false)
		{
			if (b_force || stricmp_utf8(field.m_name, m_field_data.m_name))
			{
				pfc::ptr_list_t<filter_panel_t> windows_before;
				get_windows(windows_before);
				t_size pos_before = windows_before.find_item(this);
				if (pos_before != pfc_infinite)
				{
				}
				m_field_data = field;
				bool b_redraw = disable_redrawing();
				clear_all_items();
				refresh_columns();
				metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
				get_initial_handles(handles);
				populate_list(handles);
				pfc::ptr_list_t<filter_panel_t> windows_after;
				get_windows(windows_after);
				t_size pos_after = windows_after.find_item(this);
				t_size pos_update = min (pos_before, pos_after);
				if (b_redraw)
					enable_redrawing();
				//update_window();
				g_update_subsequent_filters(windows_after, pos_update, false, false);
			}
		}

		virtual void notify_update_item_data (t_size index)
		{
			get_item_subitems(index).add_item(pfc::string8_fast_aggressive(pfc::stringcvt::string_utf8_from_wide(m_nodes[index].m_value)));
		}

		class data_entry_t
		{
		public:
			metadb_handle_ptr m_handle;
			pfc::array_t<WCHAR> m_text;
			//pfc::array_t<t_uint8> m_sortkey;

			bool m_same_as_next;
			//pfc::string8_fast_aggressive m_text_utf8;
			__forceinline static int g_compare(const data_entry_t & i1, const data_entry_t & i2)
			{
				return StrCmpLogicalW(i1.m_text.get_ptr(), i2.m_text.get_ptr());
			}
			data_entry_t() : m_same_as_next(false) {};
		};

		//void get_data_entries(const pfc::list_base_const_t<metadb_handle_ptr> & handles, pfc::list_base_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out, bool b_show_empty = false);
		void get_data_entries_v2(const metadb_handle_ptr * p_handles, t_size count, pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out, bool b_show_empty);
		void get_data_entries_v2(const pfc::list_base_const_t<metadb_handle_ptr> & handles, pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out, bool b_show_empty);
		void populate_list(const metadb_handle_list_t<pfc::alloc_fast> & handles);
		void populate_list_from_chain(const metadb_handle_list_t<pfc::alloc_fast> & handles, bool b_last_in_chain);
		virtual void refresh_groups();
		virtual void refresh_columns();
		void refresh_stream();
		//virtual void on_groups_change();
		//virtual void on_columns_change();

		bool is_visible() {RECT rc; return get_wnd() && IsWindowVisible(get_wnd()) && GetClientRect(get_wnd(), &rc) && RECT_CY(rc) > 0;}

		virtual void notify_on_create();
		virtual void notify_on_initialisation();
		virtual void notify_on_destroy();
		//void set_focus() {SetFocus(get_wnd());}
		virtual void render_background(HDC dc, const RECT * rc);
		virtual void render_item(HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc);
		virtual t_size get_highlight_item() 
		{
			return pfc_infinite;
		}
		virtual bool notify_on_keyboard_keydown_search()
		{
#ifdef FILTER_OLD_SEARCH
			if (m_query_active)
			{
				focus_search_box();
			}
			else if (m_nodes.get_count())
			{
				show_search_box("Search");
				//m_search_original_handles = m_nodes[0].m_handles;
				m_query_active = true;
				m_show_search = true;
			}
#else
			return filter_search_bar::g_activate();
#endif
			return true;
		}
#ifdef FILTER_OLD_SEARCH
		virtual void notify_on_search_box_contents_change(const char * p_str) 
		{
			m_search_query = p_str;
			if (m_query_timer_active)
				KillTimer(get_wnd(), TIMER_QUERY);
			SetTimer(get_wnd(), TIMER_QUERY, 333, NULL);
			m_query_timer_active = true;
		};
		virtual void notify_on_search_box_close() 
		{
			//m_search_original_handles.remove_all();
			bool b_refresh = m_query_timer_active || m_search_query.get_length();
			m_search_query.reset();
			m_query_active = false;
			m_show_search = false;
			if (m_query_timer_active)
			{
				KillTimer(get_wnd(), TIMER_QUERY);
				m_query_timer_active=false;
			}
			if (b_refresh)
			{
				metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
				get_initial_handles(data);
				populate_list_from_chain(data, false);
				update_subsequent_filters();
			}
		};
		virtual bool notify_on_timer(UINT_PTR timerid)
		{
			if (timerid == TIMER_QUERY)
			{
				KillTimer(get_wnd(), TIMER_QUERY);
				m_query_timer_active = false;
				if (m_query_active)
				{
					metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
					get_initial_handles(data);
					populate_list_from_chain(data, false);
					update_subsequent_filters();
				}
				return true;
			}
			return false;
		}
#endif
		virtual bool notify_on_contextmenu_header(const POINT & pt, const HDHITTESTINFO & ht)
		{
			HMENU menu = CreatePopupMenu();
			t_size i, count = g_field_data.get_count();
			for (i=0; i<count; i++)
			{
				pfc::stringcvt::string_wide_from_utf8 wide(g_field_data[i].m_name);
				{
					MENUITEMINFO mii;
					memset(&mii, 0, sizeof(mii));
					mii.cbSize = sizeof (mii);
					mii.fMask = MIIM_FTYPE|MIIM_STRING|MIIM_ID|MIIM_STATE;
					mii.fType = MFT_STRING;
					mii.fState=MFS_ENABLED;
					if (!stricmp_utf8(g_field_data[i].m_name, m_field_data.m_name))
					{
						mii.fState|=MFS_CHECKED;
						mii.fType |=MFT_RADIOCHECK;
					}
					mii.dwTypeData = (LPWSTR)wide.get_ptr();
					mii.cch = wide.length();
					mii.wID = i+1;
					InsertMenuItem(menu, i, TRUE, &mii);
					//console::formatter() << (t_uint32)GetLastError();
				}
			}
			int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,get_wnd(),0);
			DestroyMenu(menu);
			if (cmd)
			{
				set_field(g_field_data[cmd-1]);
			}
			return true;
		}
		void notify_on_menu_select(WPARAM wp, LPARAM lp)
		{
			if (HIWORD(wp) & MF_POPUP)
			{
				m_status_text_override.release();
			}
			else 
			{
				if (m_contextmenu_manager.is_valid())
				{
					unsigned id = LOWORD(wp);

					bool set = false;

					pfc::string8 desc;

					if (m_contextmenu_manager.is_valid() && id >= m_contextmenu_manager_base)
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
		virtual bool notify_on_contextmenu(const POINT & pt)
		{
			uie::window_ptr p_this_temp = this;
			enum {ID_SEARCH=action_add_to_active+2, ID_BASE};
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			handles.prealloc(m_nodes.get_count());
			get_selection_handles(handles, true, true);
			HMENU menu = CreatePopupMenu();
#ifdef FILTER_OLD_SEARCH
			AppendMenu(menu, MF_STRING, ID_SEARCH, m_show_search ? L"Hide search" : L"Search");
			AppendMenu(menu, MF_SEPARATOR, NULL, NULL);
#endif
			{
				WCHAR * p_asend = L"Send to autosend playlist";
				WCHAR * p_asend_play = L"Send to autosend playlist and play";
				WCHAR * p_send = L"Send to playlist";
				WCHAR * p_send_play = L"Send to playlist and play";
				WCHAR * p_add = L"Add to active playlist";
				MENUITEMINFO mii;
				memset(&mii, 0, sizeof(mii));
				mii.cbSize = sizeof (mii);
				mii.fMask = MIIM_FTYPE|MIIM_STRING|MIIM_ID|MIIM_STATE;
				mii.fType = MFT_STRING;

				mii.dwTypeData = p_asend;
				mii.cch = wcslen(p_asend);
				mii.wID = action_send_to_autosend+1;
				mii.fState = MFS_ENABLED;
				if (cfg_doubleclickaction+1 == mii.wID)
					mii.fState |= MFS_DEFAULT;
				InsertMenuItem(menu, 2, TRUE, &mii);

				mii.dwTypeData = p_asend_play;
				mii.cch = wcslen(p_asend_play);
				mii.wID = action_send_to_autosend_play+1;
				mii.fState = MFS_ENABLED;
				if (cfg_doubleclickaction+1 == mii.wID)
					mii.fState |= MFS_DEFAULT;
				InsertMenuItem(menu, 3, TRUE, &mii);

				mii.dwTypeData = p_send;
				mii.cch = wcslen(p_send);
				mii.wID = action_send_to_new+1;
				mii.fState = MFS_ENABLED;
				if (cfg_doubleclickaction+1 == mii.wID)
					mii.fState |= MFS_DEFAULT;
				InsertMenuItem(menu, 4, TRUE, &mii);

				mii.dwTypeData = p_send_play;
				mii.cch = wcslen(p_send_play);
				mii.wID = action_send_to_new_play+1;
				mii.fState = MFS_ENABLED;
				if (cfg_doubleclickaction+1 == mii.wID)
					mii.fState |= MFS_DEFAULT;
				InsertMenuItem(menu, 5, TRUE, &mii);

				mii.dwTypeData = p_add;
				mii.cch = wcslen(p_add);
				mii.wID = action_add_to_active+1;
				mii.fState = MFS_ENABLED;
				if (cfg_doubleclickaction+1 == mii.wID)
					mii.fState |= MFS_DEFAULT;
				InsertMenuItem(menu, 6, TRUE, &mii);
			}
			AppendMenu(menu, MF_SEPARATOR, NULL, NULL);
			service_ptr_t<contextmenu_manager> manager;
			contextmenu_manager::g_create(manager);
			manager->init_context(handles,0);
			manager->win32_build_menu(menu, ID_BASE, -1);
			menu_helpers::win32_auto_mnemonics(menu);
			m_contextmenu_manager = manager;
			m_contextmenu_manager_base = ID_BASE;
			int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,get_wnd(),0);
			DestroyMenu(menu);
			m_contextmenu_manager.release();
			m_contextmenu_manager_base = NULL;
			m_status_text_override.release();
			if (cmd)
			{
				if (cmd == ID_SEARCH)
				{
#ifdef FILTER_OLD_SEARCH
					if (m_query_active)
					{
						close_search_box();
					}
					else
					{
						show_search_box("Search", true);
						m_query_active = true;
						m_show_search = true;
					}
#endif
				}
				else
					if (cmd >= ID_BASE)
					manager->execute_by_id(cmd-ID_BASE);
				else if (cmd > 0)
					do_selection_action((action_t)(cmd-1));
			}

			return true;
		}

		virtual bool do_drag_drop(WPARAM wp) 
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
			get_selection_handles(data);
			if (data.get_count() > 0)
			{
				if (cfg_sort)
				{
					service_ptr_t<titleformat_object> to;
					static_api_ptr_t<titleformat_compiler>()->compile_safe(to,cfg_sort_string);
					mmh::fb2k::g_sort_metadb_handle_list_by_format_v2(data, to, NULL);
				}
				static_api_ptr_t<playlist_incoming_item_filter> incoming_api;
				IDataObject * pDataObject = incoming_api->create_dataobject(data);
				if (pDataObject)
				{
					DWORD blah = DROPEFFECT_NONE;
					{
						pfc::com_ptr_t<mmh::ole::IDropSource_Generic> p_IDropSource_filter = new mmh::ole::IDropSource_Generic(get_wnd(), pDataObject, wp);
						HRESULT hr = DoDragDrop(pDataObject,p_IDropSource_filter.get_ptr(),DROPEFFECT_COPY|DROPEFFECT_MOVE,&blah);
					}
					pDataObject->Release();
				}
			}
			return true;
		}

		virtual bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp, bool & b_processed)
		{
			uie::window_ptr p_this = this;
			bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
			b_processed = ret;
			return ret;
		};
		virtual void move_selection (int delta)
		{};
		void get_selection_handles (metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_out, bool fallback = true, bool b_sort = false)
		{
			//pfc::hires_timer timer;
			bool b_found = false;
			t_size i, count=m_nodes.get_count();
			if (count)
				p_out.prealloc(count);
			for (i=0; i<count; i++)
			{
				if (get_item_selected(i))
				{
					b_found = true;
					if (b_sort)
						m_nodes[i].ensure_handles_sorted();
					p_out.add_items(m_nodes[i].m_handles);
				}
			}
			if (!b_found)
			{
				if (fallback)
				{
					if (b_sort)
						m_nodes[0].ensure_handles_sorted();
					p_out.add_items(m_nodes[0].m_handles);
				}
			}
			else
			{
				//timer.start();
				//p_out.remove_duplicates();
				mmh::fb2k::g_metadb_handle_list_remove_duplicates(p_out);
				//console::formatter() << "remove_duplicates: " << timer.query() << " s";
			}
		}
		enum action_t
		{
			action_send_to_autosend,
			action_send_to_autosend_play,
			action_send_to_new,
			action_send_to_new_play,
			action_add_to_active,
		};
		void do_selection_action(action_t action = action_send_to_autosend)
		{
			bit_array_bittable mask (m_nodes.get_count());
			get_selection_state(mask);
			//metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			//handles.prealloc(m_nodes.get_count());
			//get_selection_handles(handles);
			do_items_action(mask, action);
		}
		void do_items_action(const bit_array & p_nodes, action_t action = action_send_to_autosend)
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			handles.prealloc(m_nodes.get_count());
			t_size i, count = m_nodes.get_count();
			for (i=0; i<count; i++)
				if (p_nodes[i]) handles.add_items(m_nodes[i].m_handles);

			if (!handles.get_count())
				return;

			mmh::fb2k::g_metadb_handle_list_remove_duplicates(handles);

			static_api_ptr_t<playlist_manager_v3> playlist_api;
			static_api_ptr_t<play_control> playback_api;
			t_size index_insert = pfc_infinite;
			if (action == action_send_to_autosend && playback_api->is_playing())
			{
				t_size playlist = playlist_api->get_playing_playlist();
				pfc::string8 name;
				if (playlist_api->playlist_get_name(playlist, name) && !stricmp_utf8("Filter Results", name))
				{
					t_size index_old = playlist_api->find_playlist("Filter Results (Playback)", pfc_infinite);
					playlist_api->playlist_rename(playlist, "Filter Results (Playback)", pfc_infinite);
					index_insert = index_old < playlist ? playlist : playlist+1;
					if (index_old != pfc_infinite)
						playlist_api->remove_playlist(index_old);
				}
			}
			pfc::string8 playlist_name;
			t_size index = NULL;
			if (action == action_add_to_active)
			{
				index = playlist_api->get_active_playlist();
				playlist_api->playlist_undo_backup(index);
			}
			else
			{
				if (action == action_send_to_autosend)
					playlist_name = "Filter Results";
				else if (action == action_send_to_autosend_play)
					playlist_name = "Filter Results (Playback)";
				else if (action == action_send_to_new || action == action_send_to_new_play)
				{
					for (i=0; i<count; i++)
					{
						if (p_nodes[i])
						{
							if (playlist_name.get_length())
								playlist_name << ", ";
							playlist_name << pfc::stringcvt::string_utf8_from_wide(m_nodes[i].m_value);
						}
					}
				}
				//t_size index_remove = playlist_api->find_playlist("Filter Results", pfc_infinite);

				if (index_insert != pfc_infinite)
					index = playlist_api->create_playlist(playlist_name, pfc_infinite, index_insert);
				else
					index = playlist_api->find_or_create_playlist(playlist_name, pfc_infinite);
				playlist_api->playlist_undo_backup(index);
				playlist_api->playlist_clear(index);
			}
#if 1
			if (cfg_sort)
			{
				service_ptr_t<titleformat_object> to;
				static_api_ptr_t<titleformat_compiler>()->compile_safe(to,cfg_sort_string);
				mmh::fb2k::g_sort_metadb_handle_list_by_format_v2(handles, to, NULL);
			}
			if (action != action_add_to_active)
				playlist_api->playlist_add_items(index, handles, bit_array_false());
			else
			{
				playlist_api->playlist_clear_selection(index);
				playlist_api->playlist_add_items(index, handles, bit_array_true());
			}
			playlist_api->playlist_set_focus_item(index, playlist_api->playlist_get_item_count(index)-handles.get_count());
#else
			playlist_api->playlist_add_items_filter(index, handles, false);
#endif
			if (action != action_add_to_active)
			{
				playlist_api->set_active_playlist(index);
				if (action == action_send_to_autosend_play || action == action_send_to_new_play)
				{
					playlist_api->set_playing_playlist(index);
					playback_api->play_start(play_control::track_command_default);
				}
			}
		}
		virtual void execute_default_action (t_size index, t_size column, bool b_keyboard, bool b_ctrl) 
		{
			action_t action = (action_t)(filter_panel::cfg_doubleclickaction.get_value());
			/*if (cfg_doubleclickaction == 0)
			action = action_send_to_autosend;
			else if (cfg_doubleclickaction == 1)
			action = action_send_to_autosend_play;
			else if (cfg_doubleclickaction == 2)
			action = action_send_to_new;
			else if (cfg_doubleclickaction == 3)
			action = action_send_to_new_play;
			else if (cfg_doubleclickaction == 3)
			action = action_add_to_active;*/
			do_selection_action(action);
		};
		bool notify_on_middleclick(bool on_item, t_size index)
		{
			if (filter_panel::cfg_middleclickaction && on_item && index < m_nodes.get_count())
			{
				action_t action = (action_t)(filter_panel::cfg_middleclickaction.get_value()-1);
				do_items_action(bit_array_one(index), action);
				return true;
			}
			return false;
		}
		void send_results_to_playlist(bool b_play = false)
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			handles.prealloc(m_nodes.get_count());
			get_selection_handles(handles);
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
					index_insert = index_old < playlist ? playlist : playlist+1;
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
				static_api_ptr_t<titleformat_compiler>()->compile_safe(to,cfg_sort_string);
				{
					mmh::fb2k::g_sort_metadb_handle_list_by_format_v2(handles, to, NULL);
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
		virtual void notify_on_selection_change(const bit_array & p_affected,const bit_array & p_status, notification_source_t p_notification_source)
		{
			if (p_notification_source != notification_source_rmb)
			{
				//send_results_to_playlist();
				update_subsequent_filters();
				if (m_selection_holder.is_valid())
				{
					metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
					get_selection_handles(handles, false);
					m_selection_holder->set_selection(handles);
				}
			}
		}
		void update_first_node_text(bool b_update = false)
		{
			t_size nodes_count = m_nodes.get_count();
			if (nodes_count)
			{
				nodes_count -= 1;
				pfc::string8 temp;
				temp << "All";
				if (m_field_data.m_name.length())
				{
					temp << " (" << nodes_count << " " << m_field_data.m_name;
					if (nodes_count != 1)
						temp << "s";
					temp  << ")";
				}
				m_nodes[0].m_value.set_string(pfc::stringcvt::string_wide_from_utf8(temp));
				if (b_update)
					update_items(0, 1);
			}
		}
		virtual void on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
		virtual void on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
		virtual void on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
		void _on_items_modified(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);
		void _on_items_added(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);
		void _on_items_removed(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);

		virtual bool notify_before_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, bool b_source_mouse) 
		{
			if (!m_field_data.m_use_script && m_field_data.m_fields.get_count() && column == 0 && indices.get_count() ==1 && indices[0]!=0)
				return true;
			return false;
		};
		virtual bool notify_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, pfc::string_base & p_text, t_size & p_flags, mmh::comptr_t<IUnknown> & pAutocompleteEntries) 
		{
			t_size indices_count = indices.get_count();
			if (!m_field_data.m_use_script && m_field_data.m_fields.get_count() && indices_count == 1 && indices[0] < m_nodes.get_count())
			{
				m_edit_handles = m_nodes[indices[0]].m_handles;

				m_edit_fields = m_field_data.m_fields;

				p_text = (m_edit_previous_value = pfc::stringcvt::string_utf8_from_wide(m_nodes[indices[0]].m_value));

				return true;
			}
			return false;
		};
		virtual void notify_save_inline_edit(const char * value) 
		{
			static_api_ptr_t<metadb_io_v2> tagger_api;
			{
				metadb_handle_list ptrs(m_edit_handles);
				pfc::list_t<file_info_impl> infos;
				pfc::list_t<bool> mask;
				pfc::list_t<const file_info *> infos_ptr;
				t_size i, count = ptrs.get_count();
				mask.set_count(count);
				infos.set_count(count);
				//infos.set_count(count);
				for (i=0; i<count; i++)
				{
					assert(ptrs[i].is_valid());
					mask[i]= !ptrs[i]->get_info(infos[i]);
					infos_ptr.add_item(&infos[i]);
					if (!mask[i])
					{
						bool b_remove=true;
						t_size j, jcount=m_edit_fields.get_count();
						for (j=0; j<jcount; j++)
						{
							t_size field_index = infos[i].meta_find(m_edit_fields[j]);
							if (field_index != pfc_infinite)
							{
								t_size field_count = infos[i].meta_enum_value_count(field_index);
								t_size k;
								bool b_found = false;
								for (k=0; k<field_count; k++)
								{
									const char * ptr = infos[i].meta_enum_value(field_index, k);
									if (((!ptr && m_edit_previous_value.is_empty()) || !stricmp_utf8(m_edit_previous_value, ptr)) && strcmp(value, ptr))
									{
										infos[i].meta_modify_value(field_index, k, value);
										b_remove = false;
									}
								}
							}
						}
						mask[i] =b_remove;
					}
				}
				infos_ptr.remove_mask(mask.get_ptr());
				ptrs.remove_mask(mask.get_ptr());

				{
					service_ptr_t<file_info_filter_impl>  filter = new service_impl_t<file_info_filter_impl>(ptrs, infos_ptr);
					tagger_api->update_info_async(ptrs, filter, GetAncestor(get_wnd(), GA_ROOT), metadb_io_v2::op_flag_no_errors|metadb_io_v2::op_flag_background|metadb_io_v2::op_flag_delay_ui, NULL);
				}
			}
		};
		virtual void notify_exit_inline_edit() 
		{
			m_edit_fields.remove_all();
			m_edit_handles.remove_all();
			m_edit_previous_value.reset();
		};

		void notify_on_set_focus(HWND wnd_lost)
		{
			m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			get_selection_handles(handles, false);
			m_selection_holder->set_selection(handles);
		}
		void notify_on_kill_focus(HWND wnd_receiving)
		{
			m_selection_holder.release();
		}
		ui_selection_holder::ptr m_selection_holder;

		pfc::string8 m_edit_previous_value;
		pfc::list_t<pfc::string8> m_edit_fields;
		metadb_handle_list m_edit_handles;
		bool m_show_search;
#ifdef FILTER_OLD_SEARCH
		bool m_query_active, m_query_timer_active;
		//metadb_handle_list m_search_original_handles;
		pfc::string8 m_search_query;
#endif

		contextmenu_manager::ptr m_contextmenu_manager;
		UINT_PTR m_contextmenu_manager_base;
		ui_status_text_override::ptr m_status_text_override;

		static bool g_showemptyitems;
		static bool g_showallnode;

		class node_t
		{
		public:
			metadb_handle_list_t<pfc::alloc_fast_aggressive> m_handles;
			pfc::string_simple_t<WCHAR> m_value;
			bool m_handles_sorted;

			void ensure_handles_sorted()
			{
				if (!m_handles_sorted)
				{
					if (cfg_sort)
					{
						service_ptr_t<titleformat_object> to;
						static_api_ptr_t<titleformat_compiler>()->compile_safe(to,cfg_sort_string);
						mmh::fb2k::g_sort_metadb_handle_list_by_format_v2(m_handles, to, NULL);
					}
					m_handles_sorted = true;
				}
			}

			node_t() : m_handles_sorted(false) {};

			static int g_compare(const node_t & i1, const WCHAR * i2)
			{
				//return CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, i1.m_value, -1, i2, -1);
				return StrCmpLogicalW(i1.m_value, i2);
			}
			static int g_compare_ptr(const node_t * i1, const WCHAR * i2)
			{
				//return CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, i1.m_value, -1, i2, -1);
				return StrCmpLogicalW(i1->m_value, i2);
			}
		};
		//metadb_handle_list m_handles;
		//t_string_list_fast m_strings;
		pfc::list_t<node_t> m_nodes;
		//static int g_compare_node();
	};
	void filter_panel_t::refresh_stream()
	{
		m_stream.release();
		if (cfg_orderedbysplitters)
		{
			t_size stream_index = pfc_infinite;
			uie::window_host_ex::ptr hostex;
			if (get_host()->service_query_t(hostex))
			{
				pfc::list_t<uie::window_ptr> siblings;
				hostex->get_children(siblings);
				t_size i, count=siblings.get_count();
				for (i=0; i<count; i++)
				{
					if ((stream_index = g_get_stream_index_by_window(siblings[i])) != pfc_infinite)
						break;
				}
			}
			if (stream_index != pfc_infinite)
				g_streams[stream_index]->m_windows.add_item(this);
			else
			{
				filter_stream_t::ptr streamnew = new filter_stream_t;
				streamnew->m_windows.add_item(this);
				stream_index = g_streams.add_item(streamnew);
			}
			m_stream = g_streams[stream_index];
		}
		else
		{
			if (!g_streams.get_count())
			{
				m_stream = new filter_stream_t;
				g_streams.add_item(m_stream);
			}
			else
				m_stream = g_streams[0];
			m_stream->m_windows.add_item(this);
		}
	}

	void filter_panel_t::populate_list_from_chain(const metadb_handle_list_t<pfc::alloc_fast> & handles, bool b_last_in_chain)
	{
		//pfc::hires_timer t0;
		//t0.start();
		//SendMessage(get_wnd(), WM_SETREDRAW, FALSE, NULL);
		bool b_redraw = disable_redrawing();
		pfc::list_t<pfc::string_simple_t<WCHAR> > previous_nodes;
		bool b_all_was_selected = false;
		if (m_nodes.get_count())
		{
			pfc::list_t<bool> sel_data;
			sel_data.set_count(m_nodes.get_count());
			bit_array_var_table selection(sel_data.get_ptr(), sel_data.get_count());
			get_selection_state(selection);
			t_size i, count = sel_data.get_count();
			b_all_was_selected = selection[0];
			for (i=1; i<count; i++)
				if (selection[i])
					previous_nodes.add_item(m_nodes[i].m_value);
		}
		//console::formatter() << "popc: " << t0.query_reset();
		populate_list(handles);
		//t0.query_reset();
		{
			t_size i, count = previous_nodes.get_count();
			pfc::array_t<bool> new_selection;
			new_selection.set_count(m_nodes.get_count());
			new_selection.fill_null();
			if (count || b_all_was_selected)
			{
				bool b_found = false;
				new_selection[0] = b_all_was_selected;
				for (i=0; i<count; i++)
				{
					t_size index;
					if (mmh::bsearch_partial_t(m_nodes.get_count()-1,  m_nodes, node_t::g_compare, previous_nodes[i].get_ptr(), 1, index))
					{
						new_selection[index] = true;
						b_found = true;
					}
				}
				if (!b_found)
					new_selection[0] = true; //m_nodes.get_count() >= 1
				set_selection_state(bit_array_var_table(new_selection.get_ptr(), new_selection.get_count()), 
					bit_array_var_table(new_selection.get_ptr(), new_selection.get_count()), false);
				//if (b_last_in_chain)
				//	send_results_to_playlist();
				//else
				//	update_subsequent_filters();
			}
			else
			{
			}
		}
		if (b_redraw)
			enable_redrawing();
		//SendMessage(get_wnd(), WM_SETREDRAW, TRUE, NULL);
		//RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
		//console::formatter() << "popc: " << t0.query_reset();
	}

	void filter_panel_t::_on_items_added(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);


		metadb_handle_list actualHandles = handles;
#ifdef FILTER_OLD_SEARCH
		bool b_filter = m_query_active && strnlen(m_search_query,1);
		if (b_filter)
		{
			try {
				search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_search_query);
				pfc::array_t<bool> data;
				data.set_size(actualHandles.get_count());
				api->test_multi(actualHandles, data.get_ptr());
				actualHandles.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
			} catch (pfc::exception const &) {};
		}
#endif
		metadb_handle_list_t<pfc::alloc_fast_aggressive> handlesNotifyNext;
		handlesNotifyNext.prealloc(actualHandles.get_count());

		m_nodes[0].m_handles.add_items(actualHandles);

		bool b_no_selection = get_selection_count(1)==0 || get_item_selected(0);

		{
			pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;
			//data0.prealloc(handles.get_count());


			get_data_entries_v2(actualHandles.get_ptr(), actualHandles.get_count(), data0, g_showemptyitems);

			mmh::permutation_t permutation(data0.get_count());
			mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);

			pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
			pfc::list_base_const_t<data_entry_t> & data = data2;

			pfc::list_t<t_list_view::t_item_insert, pfc::alloc_fast_aggressive> items;
			items.prealloc(data.get_count());

			{
				if (!m_field_data.is_empty())
				{


					data_entry_t * p_data = data0.get_ptr();
					t_size * perm = permutation.get_ptr();

					//node_t node;
					t_size i, count = data.get_count(), counter=0;

					for (i=0; i<count; i++)
						if (i +1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i+1]].m_text.get_ptr())))
							counter++;

					for (i=0; i<count; i++)
					{
						t_size start = i;
						while (p_data[perm[i]].m_same_as_next && i+1<count)
							i++;
						t_size handles_count = 1+i-start, k;

						t_size index_item;
						bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count()-1,  m_nodes, node_t::g_compare, p_data[perm[start]].m_text.get_ptr(), 1, index_item);
						if (b_exact)
						{
							t_size current_count = m_nodes[index_item].m_handles.get_count();
							m_nodes[index_item].m_handles.set_count(current_count + handles_count);

							bool b_selected = !b_no_selection && get_item_selected(index_item);

							for (k=0;k<handles_count; k++)
								m_nodes[index_item].m_handles[current_count+k] = p_data[perm[start+k]].m_handle;

							if (b_selected && handles_count)
								handlesNotifyNext.add_items_fromptr(m_nodes[index_item].m_handles.get_ptr()+current_count, handles_count);
						}
						else
						{
							node_t node;
							node.m_value = p_data[perm[start]].m_text.get_ptr();
							node.m_handles.set_count(handles_count);

							for (k=0;k<handles_count; k++)
								node.m_handles[k] = p_data[perm[start+k]].m_handle;

							m_nodes.insert_item(node, index_item);
							insert_items(index_item, 1, &t_item_insert());
						}
					}

					update_first_node_text(true);
				}
			}
		}
		if (index+1<windows.get_count())
		{
			//if (index==0)
			//	g_update_subsequent_filters(windows, index+1, false, false);
			if (b_no_selection)
				windows[index+1]->_on_items_added(actualHandles);
			else if (handlesNotifyNext.get_count())
				windows[index+1]->_on_items_added(handlesNotifyNext);
		}
	}

	void filter_panel_t::_on_items_removed(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);

		metadb_handle_list_t<pfc::alloc_fast_aggressive> handlesNotifyNext;
		handlesNotifyNext.prealloc(handles.get_count());

		bool b_no_selection = get_selection_count(1)==0 || get_item_selected(0);
		{
			t_size j, count = handles.get_count();
			for (j=0; j<count; j++)
				m_nodes[0].m_handles.remove_item(handles[j]);
		}
		{
			pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;
			//data0.prealloc(handles.get_count());


			get_data_entries_v2(handles, data0, g_showemptyitems);

			mmh::permutation_t permutation(data0.get_count());
			mmh::g_sort_get_permutation_qsort_v2( data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);

			pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
			pfc::list_base_const_t<data_entry_t> & data = data2;

			{
				if (!m_field_data.is_empty())
				{

					data_entry_t * p_data = data0.get_ptr();
					t_size * perm = permutation.get_ptr();

					//node_t node;
					t_size i, count = data.get_count(), counter=0;

					for (i=0; i<count; i++)
						if (i +1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i+1]].m_text.get_ptr())))
							counter++;

					pfc::array_t<bool> mask0;
					mask0.set_count(m_nodes.get_count());
					mask0.fill_null();


					for (i=0; i<count; i++)
					{
						t_size start = i;
						while (p_data[perm[i]].m_same_as_next && i+1<count)
							i++;
						t_size handles_count = 1+i-start;

						t_size index_item;
						bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count()-1,  m_nodes, node_t::g_compare, p_data[perm[start]].m_text.get_ptr(), 1, index_item);
						if (b_exact)
						{
							//t_size current_count = m_nodes[index_item].m_handles.get_count();
							//m_nodes[index_item].m_handles.set_count(handles_count);

							bool b_selected = !b_no_selection && get_item_selected(index_item);

							t_size k;
							for (k=0; k<handles_count; k++)
							{
								m_nodes[index_item].m_handles.remove_item(p_data[perm[start+k]].m_handle);
								if ( b_selected ) handlesNotifyNext.add_item(p_data[perm[start+k]].m_handle);
							}


							if (m_nodes[index_item].m_handles.get_count() == 0)
							{
								mask0[index_item] = true;
							}
						}
					}

#if 0
					node_t node;
					t_size i, count = data.get_count();
					for (i=0; i<count; i++)
					{
						node.m_handles.add_item(data[i].m_handle);
						if (i +1 == count || StrCmpLogicalW(data[i].m_text.get_ptr(), data[i+1].m_text.get_ptr()))
						{
							t_size index_item;
							bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count()-1,  m_nodes, node_t::g_compare, data[i].m_text.get_ptr(), 1, index_item);
							if (b_exact)
							{
								t_size j, countj = node.m_handles.get_count();
								for (j=0; j<countj; j++)
									m_nodes[index_item].m_handles.remove_item(node.m_handles[j]);
								if (m_nodes[index_item].m_handles.get_count() == 0)
								{
									mask0[index_item] = true;
								}
							}
						}
					}
#endif
					m_nodes.remove_mask(mask0.get_ptr());
					remove_items(bit_array_table(mask0.get_ptr(), mask0.get_size()));
					update_first_node_text(true);
				}
			}
		}
		//if (index==0)
		//	g_update_subsequent_filters(windows, index+1, false, false);
		if (index+1<windows.get_count())
		{
			if (b_no_selection)
				windows[index+1]->_on_items_removed(handles);
			else if (handlesNotifyNext.get_count())
				windows[index+1]->_on_items_removed(handlesNotifyNext);
		}

	}

	void filter_panel_t::on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);
		if (index == 0 || index == pfc_infinite)
		{
			_on_items_added(metadb_handle_list_t<pfc::alloc_fast_aggressive>(handles));
		}
	}
	void filter_panel_t::on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);
		if (index == 0 || index == pfc_infinite)
		{
			_on_items_removed(metadb_handle_list_t<pfc::alloc_fast_aggressive>(handles));
		}
	}

	void filter_panel_t::_on_items_modified(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);

		pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;//, dataFilter;
		data0.prealloc(handles.get_count());


		metadb_handle_list actualHandles = handles;
#ifdef FILTER_OLD_SEARCH
		bool b_filter = m_query_active && strnlen(m_search_query,1);
		if (b_filter)
		{
			try {
				search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_search_query);
				pfc::array_t<bool> data;
				data.set_size(actualHandles.get_count());
				api->test_multi(actualHandles, data.get_ptr());
				actualHandles.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
			} catch (pfc::exception const &) {};
			//dataFilter.prealloc(filterHandles.get_count());
			//get_data_entries_v2(filterHandles, dataFilter, g_showemptyitems);

			//permutationFilter.set_count(dataFilter.get_count());
			//mmh::g_sort_get_permutation_qsort_v2(dataFilter.get_ptr(), permutationFilter, data_entry_t::g_compare, false);

			t_size j, count = handles.get_count();
			for (j=0; j<count; j++)
				m_nodes[0].m_handles.remove_item(handles[j]);
			m_nodes[0].m_handles.add_items(actualHandles);
		}
#endif

		metadb_handle_list_t<pfc::alloc_fast_aggressive> handlesNotifyNext;
		handlesNotifyNext.prealloc(actualHandles.get_count());

		get_data_entries_v2(actualHandles.get_ptr(), actualHandles.get_count(), data0, g_showemptyitems);

		mmh::permutation_t permutation(data0.get_count());
		mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);

		pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
		pfc::list_base_const_t<data_entry_t> & data = data2;
		//pfc::list_permutation_t<data_entry_t> dataFilter2(dataFilter, permutationFilter.get_ptr(), permutationFilter.get_count());

		//pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & dataAdd = b_filter ? dataFilter2 : data2;

		bool b_no_selection = get_selection_count(1)==0 || get_item_selected(0);

		{
			if (!m_field_data.is_empty())
			{
				{
					t_size j, countj = handles.get_count();
					t_size k, countk = m_nodes.get_count();
					for (k=1; k<countk ; k++)
					{
						pfc::array_t<bool> mask;
						mask.set_count(m_nodes[k].m_handles.get_count());
						mask.fill_null();
						for (j=0; j<countj; j++)
						{
							t_size index_found = m_nodes[k].m_handles.find_item(handles[j]);
							if (index_found != pfc_infinite)
								mask[index_found] = true;
						}
						m_nodes[k].m_handles.remove_mask(mask.get_ptr());
					}
				}

				data_entry_t * p_data = data0.get_ptr();
				t_size * perm = permutation.get_ptr();

				//node_t node;
				t_size i, count = data.get_count(), counter=0;

				for (i=0; i<count; i++)
					if (i +1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i+1]].m_text.get_ptr())))
						counter++;

				for (i=0; i<count; i++)
				{
					t_size start = i;
					while (p_data[perm[i]].m_same_as_next && i+1<count)
						i++;
					t_size handles_count = 1+i-start, k;

					t_size index_item;
					bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count()-1,  m_nodes, node_t::g_compare, p_data[perm[start]].m_text.get_ptr(), 1, index_item);
					if (b_exact)
					{
						t_size current_count = m_nodes[index_item].m_handles.get_count();
						m_nodes[index_item].m_handles.set_count(current_count + handles_count);

						bool b_selected = !b_no_selection && get_item_selected(index_item);

						for (k=0;k<handles_count; k++)
						{
							m_nodes[index_item].m_handles[current_count+k] = p_data[perm[start+k]].m_handle;
						}
						if (b_selected && handles_count)
							handlesNotifyNext.add_items_fromptr(m_nodes[index_item].m_handles.get_ptr()+current_count, handles_count);
					}
					else
					{
						node_t node;
						node.m_value = p_data[perm[start]].m_text.get_ptr();
						node.m_handles.set_count(handles_count);

						for (k=0;k<handles_count; k++)
							node.m_handles[k] = p_data[perm[start+k]].m_handle;

						m_nodes.insert_item(node, index_item);
						insert_items(index_item, 1, &t_item_insert(), false);
					}
				}

#if 0
				node_t node;
				t_size i, count = data.get_count();
				for (i=0; i<count; i++)
				{
					node.m_handles.add_item(data[i].m_handle);
					if (i +1 == count || StrCmpLogicalW(data[i].m_text.get_ptr(), data[i+1].m_text.get_ptr()))
					{
						t_size index_item;
						bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count()-1,  m_nodes, node_t::g_compare, data[i].m_text.get_ptr(), 1, index_item);
						if (b_exact)
						{
							/*t_size j, countj = node.m_handles.get_count();
							for (j=0; j<countj; j++)
							if (!*/
							m_nodes[index_item].m_handles.add_items(node.m_handles);
							node.m_handles.remove_all();
							//metadb_handle_list_helper::remove_duplicates(m_nodes[index_item].m_handles);
						}
						else
						{
							t_string_list_fast temp;
							node.m_value = data[i].m_text.get_ptr();
							m_nodes.insert_item(node, index_item);
							insert_items(index_item, 1, &t_item_insert());
							node.m_handles.remove_all();
						}
					}
				}
#endif
				{
					t_size k, countk = m_nodes.get_count();
					pfc::array_t<bool> mask0;
					mask0.set_count(countk);
					mask0[0] = false;
					//mask0.fill_null();
					for (k=1; k<countk ; k++)
					{
						mask0[k] = m_nodes[k].m_handles.get_count() == 0;
					}
					m_nodes.remove_mask(mask0.get_ptr());
					remove_items(bit_array_table(mask0.get_ptr(), mask0.get_size()), true);
				}
				update_first_node_text(true);
			}
		}
		if (index+1<windows.get_count())
		{
#ifdef FILTER_OLD_SEARCH
			if (b_filter)
				g_update_subsequent_filters(windows, index+1, false, false);
			else 
#endif
				if (b_no_selection)
				windows[index+1]->_on_items_modified(actualHandles);
			else if (handlesNotifyNext.get_count())
				windows[index+1]->_on_items_modified(handlesNotifyNext);
		}
	}

	void filter_panel_t::on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & handles)
	{
		//pfc::hires_timer timer;
		//timer.start();

		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);
		if (index == 0 || index == pfc_infinite)
		{
#if 1
			metadb_handle_list_t<pfc::alloc_fast_aggressive> actualHandles = handles;
			_on_items_modified(actualHandles);
#else
			{
				pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;//, dataFilter;
				data0.prealloc(handles.get_count());

				//mmh::permutation_t permutationFilter;
				metadb_handle_list actualHandles = handles;
				bool b_filter = m_query_active && strnlen(m_search_query,1);
				if (b_filter)
				{
					try {
						search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_search_query);
						pfc::array_t<bool> data;
						data.set_size(actualHandles.get_count());
						api->test_multi(actualHandles, data.get_ptr());
						actualHandles.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
					} catch (pfc::exception const &) {};
					//dataFilter.prealloc(filterHandles.get_count());
					//get_data_entries_v2(filterHandles, dataFilter, g_showemptyitems);

					//permutationFilter.set_count(dataFilter.get_count());
					//mmh::g_sort_get_permutation_qsort_v2(dataFilter.get_ptr(), permutationFilter, data_entry_t::g_compare, false);

					t_size j, count = handles.get_count();
					for (j=0; j<count; j++)
						m_nodes[0].m_handles.remove_item(handles[j]);
					m_nodes[0].m_handles.add_items(actualHandles);
				}

				metadb_handle_list_t<pfc::alloc_fast_aggressive> handlesNotifyNext;
				handlesNotifyNext.prealloc(actualHandles.get_count());

				get_data_entries_v2(actualHandles.get_ptr(), actualHandles.get_count(), data0, g_showemptyitems);

				mmh::permutation_t permutation(data0.get_count());
				mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);

				pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
				pfc::list_base_const_t<data_entry_t> & data = data2;
				//pfc::list_permutation_t<data_entry_t> dataFilter2(dataFilter, permutationFilter.get_ptr(), permutationFilter.get_count());

				//pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & dataAdd = b_filter ? dataFilter2 : data2;

				{
					if (!m_field_data.is_empty())
					{
						{
							t_size j, countj = handles.get_count();
							t_size k, countk = m_nodes.get_count();
							for (k=1; k<countk ; k++)
							{
								pfc::array_t<bool> mask;
								mask.set_count(m_nodes[k].m_handles.get_count());
								mask.fill_null();
								for (j=0; j<countj; j++)
								{
									t_size index_found = m_nodes[k].m_handles.find_item(handles[j]);
									if (index_found != pfc_infinite)
										mask[index_found] = true;
								}
								m_nodes[k].m_handles.remove_mask(mask.get_ptr());
							}
						}

						data_entry_t * p_data = data0.get_ptr();
						t_size * perm = permutation.get_ptr();

						//node_t node;
						t_size i, count = data.get_count(), counter=0;

						for (i=0; i<count; i++)
							if (i +1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i+1]].m_text.get_ptr())))
								counter++;

						for (i=0; i<count; i++)
						{
							t_size start = i;
							while (p_data[perm[i]].m_same_as_next && i+1<count)
								i++;
							t_size handles_count = 1+i-start, k;

							t_size index_item;
							bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count()-1,  m_nodes, node_t::g_compare, p_data[perm[start]].m_text.get_ptr(), 1, index_item);
							if (b_exact)
							{
								t_size current_count = m_nodes[index_item].m_handles.get_count();
								m_nodes[index_item].m_handles.set_count(current_count + handles_count);

								bool b_selected = get_item_selected(index_item);

								for (k=0;k<handles_count; k++)
								{
									m_nodes[index_item].m_handles[current_count+k] = p_data[perm[start+k]].m_handle;
								}
								if (b_selected && handles_count)
									handlesNotifyNext.add_items_fromptr(m_nodes[index_item].m_handles.get_ptr()+current_count, handles_count);
							}
							else
							{
								node_t node;
								node.m_value = p_data[perm[start]].m_text.get_ptr();
								node.m_handles.set_count(handles_count);

								for (k=0;k<handles_count; k++)
									node.m_handles[k] = p_data[perm[start+k]].m_handle;

								m_nodes.insert_item(node, index_item);
								insert_items(index_item, 1, &t_item_insert(), false);
							}
						}

#if 0
						node_t node;
						t_size i, count = data.get_count();
						for (i=0; i<count; i++)
						{
							node.m_handles.add_item(data[i].m_handle);
							if (i +1 == count || StrCmpLogicalW(data[i].m_text.get_ptr(), data[i+1].m_text.get_ptr()))
							{
								t_size index_item;
								bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count()-1,  m_nodes, node_t::g_compare, data[i].m_text.get_ptr(), 1, index_item);
								if (b_exact)
								{
									/*t_size j, countj = node.m_handles.get_count();
									for (j=0; j<countj; j++)
									if (!*/
									m_nodes[index_item].m_handles.add_items(node.m_handles);
									node.m_handles.remove_all();
									//metadb_handle_list_helper::remove_duplicates(m_nodes[index_item].m_handles);
								}
								else
								{
									t_string_list_fast temp;
									node.m_value = data[i].m_text.get_ptr();
									m_nodes.insert_item(node, index_item);
									insert_items(index_item, 1, &t_item_insert());
									node.m_handles.remove_all();
								}
							}
						}
#endif
						{
							t_size k, countk = m_nodes.get_count();
							pfc::array_t<bool> mask0;
							mask0.set_count(countk);
							mask0[0] = false;
							//mask0.fill_null();
							for (k=1; k<countk ; k++)
							{
								mask0[k] = m_nodes[k].m_handles.get_count() == 0;
							}
							m_nodes.remove_mask(mask0.get_ptr());
							remove_items(bit_array_table(mask0.get_ptr(), mask0.get_size()), true);
						}
						update_first_node_text(true);
					}
				}
			}
			if (index==0)
				g_update_subsequent_filters(windows, index+1, false, false);
			//if (index+1<windows.get_count())
			//	windows[index+1]->on_items_modified(handlesNotifyNext);
#endif
		}

		//console::formatter() << "filter_panel_t::on_items_modified() " << timer.query() << " s";
	}

#if 0
	void filter_panel_t::get_data_entries(const pfc::list_base_const_t<metadb_handle_ptr> & handles, pfc::list_base_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out)
	{
		pfc::stringcvt::string_wide_from_utf8_t<pfc::alloc_fast_aggressive> str_utf16;
		if (!m_field_data.is_empty())
		{
			if (m_field_data.m_use_script)
			{
				pfc::string8_fastalloc buffer;
				titleformat_object_wrapper to(m_field_data.m_script);
				buffer.prealloc(32);
				t_size i, count = handles.get_count();
				for (i=0; i<count; i++)
				{
					data_entry_t temp;
					temp.m_handle = handles[i];
					temp.m_handle->format_title(NULL, buffer, to, NULL);
					str_utf16.convert(buffer);
					temp.m_text = str_utf16.get_ptr();
					p_out.add_item(temp);
				}
			}
			else
			{
				in_metadb_sync sync;
				t_size i, count = handles.get_count(), k, kcount=m_field_data.m_fields.get_count();
				for (i=0; i<count; i++)
				{
					const file_info * info = NULL;
					if (handles[i]->get_info_locked(info))
					{
						for (k=0; k<kcount; k++)
						{
							t_size j, fieldcount = info->meta_get_count_by_name(m_field_data.m_fields[k]);
							for (j=0; j<fieldcount; j++)
							{
								data_entry_t temp;
								temp.m_handle = handles[i];
								//temp.m_text_utf8 = info->meta_get(m_field, j);
								str_utf16.convert(info->meta_get(m_field_data.m_fields[k], j));
								temp.m_text = str_utf16.get_ptr();
								p_out.add_item(temp);
							}
							if (fieldcount)
								break;
						}
					}
				}
			}
		}
	}
#endif

	void filter_panel_t::get_data_entries_v2(const pfc::list_base_const_t<metadb_handle_ptr> & handles, pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out, bool b_show_empty)
	{
		metadb_handle_list p_handles(handles);
		get_data_entries_v2(p_handles.get_ptr(), p_handles.get_count(), p_out, b_show_empty);
	}

	void filter_panel_t::get_data_entries_v2(const metadb_handle_ptr * p_handles, t_size count, pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out, bool b_show_empty)
	{
#if 0
		p_out.set_count(count);
		pfc::stringcvt::string_wide_from_utf8_t<pfc::alloc_fast_aggressive> str_utf16;
		pfc::string8_fast_aggressive buffer;
		pfc::string8 spec;
		spec << "%" << m_field << "%";
		titleformat_object::ptr to;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to, spec);
		t_size i;
		for (i=0; i<count; i++)
		{
			p_out[i].m_handle = p_handles[i];
			p_handles[i]->format_title(NULL, buffer, to, NULL);
			str_utf16.convert(buffer);
			p_out[i].m_text = str_utf16;
		}
#else
		class handle_info_t
		{
		public:
			const file_info * m_info;
			t_size m_value_count;
			t_size m_field_index;
		};
		//pfc::stringcvt::string_wide_from_utf8_t<pfc::alloc_fast_aggressive> str_utf16;
		if (!m_field_data.is_empty())
		{

			if (m_field_data.m_use_script)
			{
				pfc::string8_fastalloc buffer;
				titleformat_object_wrapper to(m_field_data.m_script);
				buffer.prealloc(32);
				p_out.set_count(count);
				data_entry_t * pp_out = p_out.get_ptr();
				t_size i, k=0;
				for (i=0; i<count; i++)
				{
					p_handles[i]->format_title(NULL, buffer, to, NULL);
					if (b_show_empty || pfc::strlen_max(buffer, 1))
					{
						pp_out[k].m_handle = p_handles[i];
						pp_out[k].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(buffer));
						pfc::stringcvt::convert_utf8_to_wide_unchecked(pp_out[k].m_text.get_ptr(), buffer);
						//int size = LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, NULL, NULL);
						//pp_out[k].m_sortkey.set_size(size);
						//LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, (LPWSTR)pp_out[k].m_sortkey.get_ptr(), size);
						k++;
					}
				}
				p_out.set_count(k);
			}
			else
			{
				pfc::list_t<handle_info_t> infos;
				infos.set_count(count);
				handle_info_t * p_infos = infos.get_ptr();

				class thread_get_meta_t : public mmh::thread_v2_t
				{
					DWORD on_thread()
					{
						t_size i, l, lcount=m_field_data->m_fields.get_count();

						for (i=m_thread_index; i<m_item_count; i+=m_thread_count)
						{
							if (m_handles[i]->get_info_locked(m_infos[i].m_info))
							{
								for (l=0; l<lcount; l++)
								{
									m_infos[i].m_field_index = m_infos[i].m_info->meta_find(m_field_data->m_fields[l]);
									m_infos[i].m_value_count = m_infos[i].m_field_index != pfc_infinite ? m_infos[i].m_info->meta_enum_value_count(m_infos[i].m_field_index) : 0;
									m_counter += m_infos[i].m_value_count;
									if (m_infos[i].m_value_count)
										break;
								}
							}
							else m_infos[i].m_value_count = 0;
						}

						m_done.set_state(true);
						m_continue.wait_for(-1);

						//count = m_out->get_count();
						data_entry_t * pp_out = m_out->get_ptr();
						for (i=m_thread_index; i<m_item_count; i+=m_thread_count)
						{
							t_size j;
							for (j=0; j<m_infos[i].m_value_count; j++)
							{
								const char * str = m_infos[i].m_info->meta_enum_value(m_infos[i].m_field_index, j);
								if (m_show_empty || *str)
								{
									long insert = InterlockedIncrement(m_interlocked_position)-1;
									pp_out[insert].m_handle = m_handles[i];
									pp_out[insert].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(str));
									pfc::stringcvt::convert_utf8_to_wide_unchecked(pp_out[insert].m_text.get_ptr(), str);
									//int size = LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, NULL, NULL);
									//pp_out[k].m_sortkey.set_size(size);
									//LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, (LPWSTR)pp_out[k].m_sortkey.get_ptr(), size);
								}
							}
						}

						return 0;
					}
				public:

					thread_get_meta_t() : m_counter(0) {};

					handle_info_t * m_infos;
					const metadb_handle_ptr * m_handles;
					t_size m_thread_index, m_thread_count, m_item_count, m_counter;
					const field_data_t * m_field_data;

					pfc::array_t<bool> * m_mask_remove;
					pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> * m_out;
					bool m_show_empty;

					volatile unsigned long * m_interlocked_position;

					win32_event m_done;
					win32_event m_continue;
				};

				in_metadb_sync sync;
				t_size i, counter = 0, k=0, l, lcount=m_field_data.m_fields.get_count();

#if 1
				for (i=0; i<count; i++)
				{
					if (p_handles[i]->get_info_locked(p_infos[i].m_info))
					{
						for (l=0; l<lcount; l++)
						{
							p_infos[i].m_field_index = p_infos[i].m_info->meta_find(m_field_data.m_fields[l]);
							p_infos[i].m_value_count = p_infos[i].m_field_index != pfc_infinite ? p_infos[i].m_info->meta_enum_value_count(p_infos[i].m_field_index) : 0;
							counter += p_infos[i].m_value_count;
							if (p_infos[i].m_value_count)
								break;
						}
					}
					else p_infos[i].m_value_count = 0;
				}

				p_out.set_count(counter);

				data_entry_t * pp_out = p_out.get_ptr();
				for (i=0; i<count; i++)
				{
					t_size j;
					for (j=0; j<p_infos[i].m_value_count; j++)
					{
						const char * str = p_infos[i].m_info->meta_enum_value(p_infos[i].m_field_index, j);
						if (b_show_empty || *str)
						{
							pp_out[k].m_handle = p_handles[i];
							pp_out[k].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(str));
							pfc::stringcvt::convert_utf8_to_wide_unchecked(pp_out[k].m_text.get_ptr(), str);
							//int size = LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, NULL, NULL);
							//pp_out[k].m_sortkey.set_size(size);
							//LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, (LPWSTR)pp_out[k].m_sortkey.get_ptr(), size);
							k++;
						}
					}
				}
				p_out.set_count(k);

#else
				t_size threadIndex, threadCount = GetOptimalWorkerThreadCount();
				volatile unsigned long interlocked_position = 0;

				pfc::array_staticsize_t<thread_get_meta_t> threads(threadCount);
				pfc::array_staticsize_t<HANDLE> threadHandles(threadCount), waitHandles(threadCount);

				pfc::array_t<bool> mask;

				for (threadIndex = 0; threadIndex<threadCount; threadIndex++)
				{
					threads[threadIndex].m_infos = p_infos;
					threads[threadIndex].m_handles = p_handles;
					threads[threadIndex].m_thread_count = threadCount;
					threads[threadIndex].m_thread_index = threadIndex;
					threads[threadIndex].m_item_count = count;
					threads[threadIndex].m_field_data = &m_field_data;
					//threads[threadIndex].m_mask_remove = &mask;
					threads[threadIndex].m_show_empty = b_show_empty;
					threads[threadIndex].m_out = &p_out;
					threads[threadIndex].m_interlocked_position = &interlocked_position;

					threads[threadIndex].m_done.create(true, false);
					threads[threadIndex].m_continue.create(true, false);
					waitHandles[threadIndex] = threads[threadIndex].m_done.get();
					threads[threadIndex].create_thread();
					threadHandles[threadIndex] = threads[threadIndex].get_thread();
				}

				WaitForMultipleObjects(threadCount, waitHandles.get_ptr(), TRUE, pfc_infinite);

				for (threadIndex = 0; threadIndex<threadCount; threadIndex++)
				{
					counter += threads[threadIndex].m_counter;
				}

				p_out.set_count(counter);

				for (threadIndex = 0; threadIndex<threadCount; threadIndex++)
				{
					threads[threadIndex].m_continue.set_state(true);
				}

				WaitForMultipleObjects(threadCount, threadHandles.get_ptr(), TRUE, pfc_infinite);

				for (threadIndex = 0; threadIndex<threadCount; threadIndex++)
				{
					threads[threadIndex].release_thread();
				}

				p_out.set_count(interlocked_position);
#endif
			}
#if 0
			for (i=0; i<count; i++)
			{
				const file_info * info = NULL;
				if (p_handles[i]->get_info_locked(info))
				{
					t_size j, fieldcount = info->meta_get_count_by_name(m_field);
					for (j=0; j<fieldcount; j++)
					{
						data_entry_t temp;
						temp.m_handle = p_handles[i];
						//temp.m_text_utf8 = info->meta_get(m_field, j);
						str_utf16.convert(info->meta_get(m_field, j));
						temp.m_text = str_utf16.get_ptr();
						p_out.add_item(temp);
					}
				}
			}
#endif
		}
#endif
	}

	void filter_panel_t::populate_list(const metadb_handle_list_t<pfc::alloc_fast> & handles)
	{
		clear_all_items();
		m_nodes.remove_all();

		//m_nodes.prealloc(handles.get_count());

		pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;
		//data0.prealloc(handles.get_count());

#ifdef FILTER_OLD_SEARCH
		metadb_handle_list filtered_list;
		bool b_filter = m_query_active && strnlen(m_search_query,1);
		const metadb_handle_list & actualHandles = b_filter ? filtered_list : handles;
		if (b_filter)
		{
			filtered_list = handles;
			try {
				search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_search_query);
				pfc::array_t<bool> data;
				data.set_size(filtered_list.get_count());
				api->test_multi(filtered_list, data.get_ptr());
				filtered_list.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
			} catch (pfc::exception const &) {};
		}
#else
		const metadb_handle_list & actualHandles = handles;
#endif

		{
			//pfc::hires_timer timer;
			//timer.start();
			get_data_entries_v2(actualHandles.get_ptr(), actualHandles.get_size(), data0, g_showemptyitems);
			//	console::formatter() << "get_data_entries_v2: " << timer.query() << " s";
		}

		mmh::permutation_t permutation(data0.get_count());
		{
			pfc::hires_timer timer;
			timer.start();
			mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);
			//	console::formatter() << "g_sort_get_permutation_qsort_v2: " << timer.query() << " s";
		}

		//data.reorder(permutation.get_ptr());
		pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
		pfc::list_base_const_t<data_entry_t> & data = data2;

		pfc::list_t<t_list_view::t_item_insert, pfc::alloc_fast_aggressive> items;
		items.prealloc(data.get_count());
		{
			{
				data_entry_t * p_data = data0.get_ptr();
				t_size * perm = permutation.get_ptr();
				//pfc::hires_timer timer;
				//timer.start();
				t_size i, count = data.get_count(),j;
				t_size counter = 0;
#if 1
				for (i=0; i<count; i++)
					if (i +1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i+1]].m_text.get_ptr())))
						//if (i +1 == count || !(p_data[perm[i]].m_same_as_next = !CompareString(LOCALE_USER_DEFAULT, NULL, p_data[perm[i]].m_text.get_ptr(), -1, p_data[perm[i+1]].m_text.get_ptr(), -1)))
						counter++;
				//counter++;
				/*if (i +1 == count)
				counter++ ;
				else 
				{
				t_size a = p_data[perm[i]].m_sortkey.get_size(), b = p_data[perm[i+1]].m_sortkey.get_size();
				if (!(p_data[perm[i]].m_same_as_next = !memcmp(p_data[perm[i]].m_sortkey.get_ptr(), p_data[perm[i+1]].m_sortkey.get_ptr(), min(a,b))))
				counter++;
				}*/

#else
				class thread_impl : public mmh::thread_t
				{
				public:
					pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> * entries;
					mmh::permutation_t * perm;
					t_size index, count, counter;

					thread_impl() : counter(0) {};

					DWORD on_thread()
					{
						t_size total_count = entries->get_count(),i;
						for (i=0;i<count;i++)
						{
							if (i+index+1 >= total_count) break;
							data_entry_t & ptr = (*entries)[(((*perm)[i+index]))];
							if (!(ptr.m_same_as_next = !StrCmpLogicalW(entries->get_item_ref((*perm)[i+index]).m_text.get_ptr(), entries->get_item_ref((*perm)[i+index+1]).m_text.get_ptr())))
								counter++;
						}
						return 0;
					}
				};
				thread_impl thread[2];
				t_size hc = tabsize(thread);
				pfc::array_t<HANDLE> hthreads;
				hthreads.set_count(hc);
				for (j=0; j<hc; j++)
				{
					thread[j].entries = &data0;
					thread[j].perm = &permutation;
					thread[j].index = j*(count/hc);
					thread[j].count = j + 1 ==hc ? (count/hc + (count - (count/hc)*hc)): count/hc;
					thread[j].create_thread();
					hthreads[j] = thread[j].get_thread();
				}

				DWORD ret = WaitForMultipleObjects(hthreads.get_count(), hthreads.get_ptr(), TRUE, pfc_infinite);
				for (j=0; j<hc; j++)
				{
					counter += thread[j].counter;
					thread[j].release_thread();
				}
				if (count) counter++;
#endif
				m_nodes.set_count(counter+1);
				//pfc::list_t<node_t> & p_nodes = m_nodes;
				node_t * p_nodes = m_nodes.get_ptr();
				{
					p_nodes[0].m_handles.add_items(actualHandles);
					p_nodes[0].m_value.set_string(L"All");
				}

				for (i=0,j=1; i<count; i++)
				{
					t_size start = i;
					while (p_data[perm[i]].m_same_as_next && i+1<count)
						i++;
					t_size handles_count = 1+i-start, k;
#ifdef _DEBUG
					PFC_ASSERT(j < counter+1);
#endif
					p_nodes[j].m_handles.set_count(handles_count);
					for (k=0;k<handles_count; k++)
						p_nodes[j].m_handles[k] = p_data[perm[start+k]].m_handle;
					p_nodes[j].m_value = p_data[perm[start]].m_text.get_ptr();
					j++;
#if 0
					node_t node;
					if (i +1 == count || !data[i].m_same_as_next /*StrCmpLogicalW(data[i].m_text, data[i+1].m_text)*/)
					{
						t_string_list_fast temp;
						node.m_value = data[i].m_text;
						//m_strings.add_item(data[i].m_text_utf8);
						//temp.add_item(data[i].m_text_utf8);
						m_nodes.add_item(node);
						node.m_handles.remove_all();
						//	start = i+1;
					}
#endif
				}
				update_first_node_text();
			}
		}
		items.set_count(m_nodes.get_count());
		insert_items(0, items.get_count(), items.get_ptr());
	}
	void filter_panel_t::refresh_groups()
	{
		set_group_count(0);
	}

	void filter_panel_t::refresh_columns()
	{
		set_columns(pfc::list_single_ref_t<t_column>(t_column(m_field_data.is_empty() ? "<no field>" : m_field_data.m_name, 200)));
	}
	/*void filter_panel_t::on_groups_change()
	{
	if (get_wnd())
	{
	clear_all_items();
	refresh_groups();
	//populate_list();
	}
	}

	void filter_panel_t::on_columns_change()
	{
	if (get_wnd())
	{
	clear_all_items();
	refresh_columns();
	//populate_list();
	}
	}*/
	void filter_panel_t::notify_on_initialisation()
	{
		//set_variable_height_items(true); //Implementation not finished
		set_edge_style(cfg_edgestyle);
		set_autosize(true);
		set_vertical_item_padding(cfg_itempadding);

		LOGFONT lf;
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_filter_items_font_client, lf);
		set_font(&lf);
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_filter_header_font_client, lf);
		set_header_font(&lf);

		t_size index = g_windows.get_count();
		if (index == 0)
		{
			g_showemptyitems = cfg_showemptyitems;
			g_load_fields();
		}

		refresh_stream();

		t_size field_index = get_field_index();
		if (field_index == pfc_infinite)
			//if (m_field_data.is_empty())
		{
			pfc::array_t<bool> used;
			t_size field_count = g_field_data.get_count();
			used.set_count(field_count);
			used.fill_null();
			{
				t_size i, count = m_stream->m_windows.get_count();
				for (i=0; i<count; i++)
				{
					t_size field_index = m_stream->m_windows[i]->get_field_index();
					if (field_index != pfc_infinite)
						used[field_index] = true;
				}
			}
			{
				t_size i;
				for (i=0; i<field_count; i++)
				{
					if (!used[i])
					{
						m_field_data = g_field_data[i];
						break;
					}
				}
			}
		}
		//else
		field_index = get_field_index();
		{
			if (field_index == pfc_infinite)
				m_field_data.reset();
			else
				m_field_data = g_field_data[field_index];
		}

	}


	void filter_panel_t::notify_on_create()
	{
#if 0
		void * buf = malloc(4096 * 16);
		assert(buf);
		buf = _expand(buf, 2 * 16);
		assert(buf);
		_expand(buf, 4096 * 16);  // verifier gets upset

#endif

		refresh_columns();
		refresh_groups();

#ifdef FILTER_OLD_SEARCH
		if (m_show_search)
		{
			show_search_box("Search", false);
			m_query_active = true;
		}
#endif

		pfc::hires_timer timer0;
		timer0.start();
		metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
		get_initial_handles(handles);
		populate_list(handles);
		double time = timer0.query();
		console::formatter() << "Filter Panel - " << m_field_data.m_name << ": initialised in " << pfc::format_float(time, 0, 3) <<" s";

		g_windows.add_item(this);
		mmh::fb2k::library_callback_manager::g_register_callback(this);

	}


	void filter_panel_t::notify_on_destroy()
	{
		mmh::fb2k::library_callback_manager::g_deregister_callback(this);

		m_selection_holder.release();

		m_stream->m_windows.remove_item(this);
		if (m_stream->m_windows.get_count() == 0)
			g_streams.remove_item(m_stream);
		m_stream.release();

		g_windows.remove_item(this);
		if (g_windows.get_count() == 0)
			g_field_data.remove_all();
		m_nodes.remove_all();
	}


	const GUID & filter_panel_t::get_extension_guid() const
	{
		return g_extension_guid;
	}

	void filter_panel_t::get_name(pfc::string_base & out)const
	{
		out.set_string("Filter");
	}
	void filter_panel_t::get_category(pfc::string_base & out)const
	{
		out.set_string("Panels");
	}
	unsigned filter_panel_t::get_type() const{return uie::type_panel;}

	// {FB059406-5F14-4bd0-8A11-4242854CBBA5}
	const GUID filter_panel_t::g_extension_guid = 
	{ 0xfb059406, 0xdddd, 0x4bd0, { 0x8a, 0x11, 0x42, 0x42, 0x85, 0x4c, 0xbb, 0xa5 } };

	uie::window_factory<filter_panel_t> g_filter;

	pfc::ptr_list_t<filter_panel_t> filter_panel_t::g_windows;
	bool filter_panel_t::g_showemptyitems = false;

	pfc::list_t<filter_panel_t::field_data_t> filter_panel_t::g_field_data;

	pfc::list_t<filter_panel_t::filter_stream_t::ptr> filter_panel_t::g_streams;

	void g_on_field_title_change(const char * p_old, const char * p_new)
	{
		filter_panel_t::g_on_field_title_change(p_old, p_new);
	}
	void g_on_field_query_change(const field_t & field)
	{
		filter_panel_t::g_on_field_query_change(field);
	}
	void g_on_showemptyitems_change(bool b_val)
	{
		filter_panel_t::g_on_showemptyitems_change(b_val);
	}
	void g_on_new_field(const field_t & field)
	{
		filter_panel_t::g_on_new_field(field);
	}
	void g_on_field_removed(t_size index)
	{
		filter_panel_t::g_on_field_removed(index);
	}
	void g_on_fields_swapped(t_size index_1, t_size index_2)
	{
		filter_panel_t::g_on_fields_swapped(index_1, index_2);
	}
	void g_on_fields_change()
	{
		filter_panel_t::g_on_fields_change();
	}

	void g_on_edgestyle_change()
	{
		filter_panel_t::g_on_edgestyle_change();
	}
	void g_on_orderedbysplitters_change()
	{
		filter_panel_t::g_on_orderedbysplitters_change();
	}
	void g_on_vertical_item_padding_change()
	{
		filter_panel_t::g_on_vertical_item_padding_change();
	}
	class appearance_client_filter_impl : public cui::colours::client
	{
	public:
		static const GUID g_guid;

		virtual const GUID & get_client_guid() const { return g_guid;};
		virtual void get_name (pfc::string_base & p_out) const {p_out = "Filter Panel";};

		virtual t_size get_supported_colours() const {return cui::colours::colour_flag_all;}; //bit-mask
		virtual t_size get_supported_fonts() const {return 0;}; //bit-mask
		virtual t_size get_supported_bools() const {return cui::colours::bool_flag_use_custom_active_item_frame;}; //bit-mask
		virtual bool get_themes_supported() const {return true;};

		virtual void on_colour_changed(t_size mask) const 
		{
			filter_panel_t::g_redraw_all();
		};
		virtual void on_font_changed(t_size mask) const {};
		virtual void on_bool_changed(t_size mask) const {};
	};

	// {4D6774AF-C292-44ac-8A8F-3B0855DCBDF4}
	const GUID appearance_client_filter_impl::g_guid = 
	{ 0x4d6774af, 0xc292, 0x44ac, { 0x8a, 0x8f, 0x3b, 0x8, 0x55, 0xdc, 0xbd, 0xf4 } };

	namespace {
		cui::colours::client::factory<appearance_client_filter_impl> g_appearance_client_impl;
	};




	void filter_panel_t::render_background(HDC dc, const RECT * rc)
	{
		cui::colours::helper p_helper(appearance_client_filter_impl::g_guid);
		gdi_object_t<HBRUSH>::ptr_t br = CreateSolidBrush(p_helper.get_colour(cui::colours::colour_background));
		FillRect(dc, rc, br);
	}

	void filter_panel_t::render_item(HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc)
	{
		cui::colours::helper p_helper(appearance_client_filter_impl::g_guid);
		const t_item * item = get_item(index);
		int theme_state = NULL;
		if (b_selected)
			theme_state = (b_highlight ? LISS_HOTSELECTED : (b_window_focused ? LISS_SELECTED : LISS_SELECTEDNOTFOCUS));
		else if (b_highlight)
			theme_state = LISS_HOT;

		bool b_theme_enabled = p_helper.get_themed();
		//NB Third param of IsThemePartDefined "must be 0". But this works.
		bool b_themed = b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_LISTITEM, theme_state);

		COLORREF cr_text = NULL;
		if (b_themed && theme_state)
		{
			cr_text = GetThemeSysColor(get_theme(), b_selected ? COLOR_BTNTEXT : COLOR_WINDOWTEXT);;
			{
				if (IsThemeBackgroundPartiallyTransparent(get_theme(), LVP_LISTITEM, theme_state))
					DrawThemeParentBackground(get_wnd(), dc, rc);
				DrawThemeBackground(get_theme(), dc, LVP_LISTITEM, theme_state, rc, NULL);
			}
		}

		RECT rc_subitem = *rc;
		t_size k, countk=get_column_count();

		for (k=0; k<countk; k++)
		{
			rc_subitem.right = rc_subitem.left + get_column_display_width(k);

			if (!(b_themed && theme_state))
			{
				if (b_selected)
					cr_text = !b_window_focused ? p_helper.get_colour(cui::colours::colour_inactive_selection_text) : p_helper.get_colour(cui::colours::colour_selection_text);
				else
					cr_text = p_helper.get_colour(cui::colours::colour_text);

				COLORREF cr_back;
				if (b_selected)
					cr_back = !b_window_focused ? p_helper.get_colour(cui::colours::colour_inactive_selection_background): p_helper.get_colour(cui::colours::colour_selection_background);
				else
					cr_back = p_helper.get_colour(cui::colours::colour_background);

				FillRect(dc, &rc_subitem, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(cr_back)));
			}
			ui_helpers::text_out_colours_tab(dc, get_item_text(index,k), strlen(get_item_text(index,k)), 1+(k==0?indentation:0), 3, &rc_subitem, b_selected, cr_text, true, true, (cfg_ellipsis != 0), get_column_alignment(k));

			rc_subitem.left = rc_subitem.right;
		}
		if (b_focused)
		{
			RECT rc_focus = *rc;
			if (b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_LISTITEM, LISS_SELECTED))
				InflateRect(&rc_focus, -1, -1);
			if (!p_helper.get_bool(cui::colours::bool_use_custom_active_item_frame))
			{
				DrawFocusRect(dc, &rc_focus);
			}
			else
			{
				gdi_object_t<HBRUSH>::ptr_t br = CreateSolidBrush(p_helper.get_colour(cui::colours::colour_active_item_frame));
				FrameRect(dc, &rc_focus, br);
			}
		}
	}

	class font_client_filter : public cui::fonts::client
	{
	public:
		virtual const GUID & get_client_guid() const
		{
			return g_guid_filter_items_font_client;
		}
		virtual void get_name (pfc::string_base & p_out) const
		{
			p_out = "Filter Panel: Items";
		}

		virtual cui::fonts::font_type_t get_default_font_type() const
		{
			return cui::fonts::font_type_items;
		}

		virtual void on_font_changed() const 
		{
			filter_panel_t::g_on_font_items_change();

		}
	};

	class font_header_client_filter : public cui::fonts::client
	{
	public:
		virtual const GUID & get_client_guid() const
		{
			return g_guid_filter_header_font_client;
		}
		virtual void get_name (pfc::string_base & p_out) const
		{
			p_out = "Filter Panel: Column Titles";
		}

		virtual cui::fonts::font_type_t get_default_font_type() const
		{
			return cui::fonts::font_type_items;
		}

		virtual void on_font_changed() const 
		{
			filter_panel_t::g_on_font_header_change();

		}
	};

	font_client_filter::factory<font_client_filter> g_font_client_filter;
	font_header_client_filter::factory<font_header_client_filter> g_font_header_client_filter;



/** FILTER SEARCH BAR */

	void g_get_search_bar_sibling_streams(filter_search_bar const * p_serach_bar, pfc::list_t<filter_panel_t::filter_stream_t::ptr> & p_out)
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

	void filter_search_bar::get_menu_items (uie::menu_hook_t & p_hook)
	{
		p_hook.add_node(new menu_node_show_clear_button(this));
	}

	void filter_search_bar::on_show_clear_button_change()
	{
		TBBUTTONINFO tbbi;
		memset(&tbbi, 0, sizeof(tbbi));
		tbbi.cbSize = sizeof(tbbi);
		tbbi.dwMask = TBIF_STATE;
		tbbi.fsState = TBSTATE_ENABLED|(m_show_clear_button ? NULL : TBSTATE_HIDDEN);
		SendMessage(m_wnd_toolbar, TB_SETBUTTONINFO, idc_clear, (LPARAM)&tbbi);
		//UpdateWindow(m_wnd_toolbar);
		RECT rc = {0};
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

	bool g_filter_search_bar_has_stream(filter_search_bar const* p_seach_bar, const filter_panel_t::filter_stream_t * p_stream)
	{
		pfc::list_t<filter_panel_t::filter_stream_t::ptr> p_streams;
		g_get_search_bar_sibling_streams(p_seach_bar, p_streams);
		return p_streams.have_item(const_cast<filter_panel_t::filter_stream_t *>(p_stream)); //meh
	}

	template <class TStream>
	void filter_search_bar::g_initialise_filter_stream(const TStream & p_stream)
	{
		for (t_size i = 0, count = g_active_instances.get_count(); i<count; i++)
		{

			if (!cfg_orderedbysplitters || g_filter_search_bar_has_stream(g_active_instances[i], p_stream.get_ptr()))
			{
				if (!g_active_instances[i]->m_active_search_string.is_empty())
				{
					p_stream->m_source_overriden = true;
					p_stream->m_source_handles = g_active_instances[i]->m_active_handles;
					break;
				}
			}
		}
	}

	void filter_search_bar::activate() 
	{
		m_wnd_last_focused = GetFocus();
		SetFocus(m_search_editbox);
	}

	void filter_search_bar::on_size (t_size cx, t_size cy)
	{
		//RECT rc_tbb = {0};
		//SendMessage(m_wnd_toolbar, TB_GETITEMRECT, 0, (LPARAM)(&rc_tbb));
		SetWindowPos(m_search_editbox, NULL, 0, 0, cx-m_toolbar_cx, 200, SWP_NOZORDER);
		SetWindowPos(m_wnd_toolbar, NULL, cx-m_toolbar_cx, 0, m_toolbar_cx, cy, SWP_NOZORDER);
	}
	void filter_search_bar::on_search_editbox_change ()
	{
		if (m_query_timer_active)
			KillTimer(get_wnd(), TIMER_QUERY);
		SetTimer(get_wnd(), TIMER_QUERY, 500, NULL);
		m_query_timer_active = true;
		update_favourite_icon();
	}
	void filter_search_bar::commit_search_results(const char * str, bool b_force_autosend, bool b_stream_update)
	{
		pfc::list_t<filter_panel_t::filter_stream_t::ptr> p_streams;
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

			if (b_reset) {m_active_handles.remove_all();}
			else if (b_diff)
			{
				static_api_ptr_t<library_manager>()->get_all_items(m_active_handles);
				try {
					search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_active_search_string);
					pfc::array_t<bool> data;
					data.set_size(m_active_handles.get_count());
					api->test_multi(m_active_handles, data.get_ptr());
					m_active_handles.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
				} catch (pfc::exception const &) {};
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
				filter_panel_t::filter_stream_t::ptr p_stream = p_streams[i];

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
			if (!b_stream_update && (stream_count == 0 || !b_autosent) && (cfg_autosend || b_force_autosend) )
				g_send_metadb_handles_to_playlist(m_active_handles);
		}
	}

	LRESULT filter_search_bar::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
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
				m_imagelist = NULL;
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
							LPNMTBGETINFOTIP lpnmtbgit = (LPNMTBGETINFOTIP) lp;
							pfc::string8 temp;
							if (lpnmtbgit->iItem == idc_favourite)
							{
								string_utf8_from_window query(m_search_editbox);
								temp = !query.is_empty() && cfg_favourites.have_item(query) ? "Remove from favourites" : "Add to favourites";
							}
							else if (lpnmtbgit->iItem == idc_clear)
								temp = "Clear";
							StringCchCopy(lpnmtbgit->pszText,lpnmtbgit->cchTextMax,pfc::stringcvt::string_wide_from_utf8(temp));
						}
						return 0;
#if 0
					case NM_CUSTOMDRAW:
						{
							LPNMTBCUSTOMDRAW lptbcd = (LPNMTBCUSTOMDRAW) lp;
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
											lptbcd->rcText.left-=LOWORD(SendMessage(m_wnd_toolbar, TB_GETPADDING, (WPARAM) 0, 0)) + 2;  //Hack for commctrl6
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
					PostMessage(m_search_editbox, CB_SETEDITSEL , 0, MAKELPARAM(0,-1));
					break;
				case CBN_KILLFOCUS:
					m_wnd_last_focused = NULL;
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

		m_search_editbox = CreateWindowEx(WS_EX_CLIENTEDGE, WC_COMBOBOX, L"" /*pfc::stringcvt::string_os_from_utf8("").get_ptr()*/, WS_CHILD|WS_CLIPSIBLINGS|ES_LEFT|
			WS_VISIBLE|WS_CLIPCHILDREN|CBS_DROPDOWN|CBS_AUTOHSCROLL|WS_TABSTOP|WS_VSCROLL, 0, 
			0, 100, 200, get_wnd(), HMENU(id_edit), core_api::get_my_instance(), 0);

		ComboBox_SetMinVisible(m_search_editbox, 25);

		m_wnd_toolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, 0, 
		WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT |TBSTYLE_TOOLTIPS | CCS_NORESIZE| CCS_NOPARENTALIGN| CCS_NODIVIDER, 
		0, 0, 0, 0, get_wnd(), (HMENU) id_toolbar, core_api::get_my_instance(), NULL); 
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
		tbb[0].fsState = TBSTATE_ENABLED|(m_show_clear_button ? NULL : TBSTATE_HIDDEN);
		tbb[0].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON; 
		//tbb[0].iString = (INT_PTR)(L"C"); 
		tbb[1].iBitmap = 0;
		tbb[1].idCommand = idc_favourite; 
		tbb[1].fsState = TBSTATE_ENABLED; 
		tbb[1].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON;
		tbb[1].iString = -1;
		//tb.iString = (INT_PTR)(L"Add to favourites");


		unsigned ex_style = SendMessage(m_wnd_toolbar, TB_GETEXTENDEDSTYLE, 0, 0);
		SendMessage(m_wnd_toolbar, TB_SETEXTENDEDSTYLE, 0, ex_style | TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);
		SendMessage(m_wnd_toolbar, TB_SETBITMAPSIZE, (WPARAM) 0, MAKELONG(cx,cy));
		SendMessage(m_wnd_toolbar, TB_GETPADDING, (WPARAM) 0, 0);

		SendMessage(m_wnd_toolbar, TB_SETIMAGELIST, (WPARAM) 0, (LPARAM) m_imagelist);
		SendMessage(m_wnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
		SendMessage(m_wnd_toolbar, TB_ADDBUTTONS, (WPARAM) tabsize(tbb), (LPARAM) &tbb[0]);
		ShowWindow(m_wnd_toolbar, SW_SHOWNORMAL);
		SendMessage(m_wnd_toolbar, TB_AUTOSIZE, 0, 0);

		SendMessage(m_search_editbox, WM_SETFONT, (WPARAM)m_font.get(), MAKELONG(TRUE,0));

		COMBOBOXINFO cbi;
		memset(&cbi, 0, sizeof(cbi));
		cbi.cbSize = sizeof(cbi);
		SendMessage(m_search_editbox, CB_GETCOMBOBOXINFO , NULL, (LPARAM)&cbi);

		RECT rc;
		GetClientRect(m_search_editbox, &rc);
		m_combo_cx += RECT_CX(rc) - RECT_CX(cbi.rcItem);

		GetWindowRect(m_search_editbox, &rc);
		m_combo_cy = rc.bottom - rc.top;

		SendMessage(m_wnd_toolbar, TB_GETITEMRECT, 1, (LPARAM)(&rc));

		m_toolbar_cx = rc.right;
		m_toolbar_cy = rc.bottom;

		SetWindowLongPtr(m_search_editbox,GWL_USERDATA,(LPARAM)(this));
		SetWindowLongPtr(cbi.hwndItem,GWL_USERDATA,(LPARAM)(this));
		m_proc_search_edit = (WNDPROC)SetWindowLongPtr(cbi.hwndItem,GWL_WNDPROC,(LPARAM)(g_on_search_edit_message));
		Edit_SetCueBannerText(cbi.hwndItem, uT("Search Filters"));

		for (t_size i = 0, count = cfg_favourites.get_count(); i<count; i++)
			ComboBox_AddString(m_search_editbox, uT(cfg_favourites[i]));

		on_size();
	}

	LRESULT WINAPI filter_search_bar::g_on_search_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		filter_search_bar * p_this;
		LRESULT rv;

		p_this = reinterpret_cast<filter_search_bar*>(GetWindowLongPtr(wnd,GWL_USERDATA));

		rv = p_this ? p_this->on_search_edit_message(wnd,msg,wp,lp) : DefWindowProc(wnd, msg, wp, lp);;

		return rv;
	}

	LRESULT filter_search_bar::on_search_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
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
		return CallWindowProc(m_proc_search_edit,wnd,msg,wp,lp);
	}


}

