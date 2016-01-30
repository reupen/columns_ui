#ifndef _CUI_FILTER_H_
#define _CUI_FILTER_H_
namespace filter_panel {

	extern const GUID g_guid_cfg_sort;
	extern const GUID g_guid_cfg_sort_string;
	extern const GUID g_guid_cfg_autosend;
	extern const GUID g_guid_doubleclickaction;
	extern const GUID g_guid_middleclickaction;
	extern const GUID g_guid_filter_items_font_client;
	extern const GUID g_guid_filter_header_font_client;
	extern const GUID guid_cfg_fields;
	extern const GUID g_guid_edgestyle;
	extern const GUID g_guid_orderedbysplitters;
	extern const GUID g_guid_showemptyitems;
	extern const GUID g_guid_itempadding;
	extern const GUID g_guid_favouritequeries;
	extern const GUID g_guid_showsearchclearbutton;

	class field_t
	{
	public:
		pfc::string8 m_name, m_field;
	};
	class cfg_fields_t : public cfg_var, public pfc::list_t<field_t>
	{
	public:
		enum {stream_version=0};
		virtual void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort);
		virtual void get_data_raw(stream_writer * p_stream, abort_callback & p_abort);
		void reset();
		bool have_name(const char * p_name);
		void fix_name(const char * p_name, pfc::string8 & p_out);
		void fix_name(pfc::string8 & p_name);
		cfg_fields_t(const GUID & p_guid) : cfg_var(p_guid) {reset();}
	};

	class cfg_favouriteslist : public cfg_var, public pfc::list_t<pfc::string8>
	{
	public:
		void get_data_raw(stream_writer * p_stream, abort_callback & p_abort);
		void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort);

		bool have_item(const char * p_item);
		bool find_item(const char * p_item, t_size & index);

	public:
		cfg_favouriteslist(const GUID & p_guid) : cfg_var(p_guid) {}
	};

	class filter_panel_t;
	class filter_search_bar;

	extern cfg_favouriteslist cfg_favourites;

	extern cfg_string /*cfg_fields, */cfg_sort_string;
	extern cfg_bool cfg_sort, cfg_autosend, cfg_orderedbysplitters, cfg_showemptyitems, cfg_showsearchclearbutton;
	extern cfg_int cfg_doubleclickaction, cfg_middleclickaction, cfg_edgestyle, cfg_itempadding;
	extern cfg_fields_t cfg_field_list;

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

			bool is_visible();

			filter_stream_t();
		};

		friend class filter_search_bar;

		enum { TIMER_QUERY = TIMER_BASE };

		filter_panel_t();;

		~filter_panel_t() {};

		static pfc::list_t<filter_stream_t::ptr> g_streams;
		filter_stream_t::ptr m_stream;

		static pfc::ptr_list_t<filter_panel_t> g_windows;

		virtual const GUID & get_extension_guid() const;
		virtual void get_name(pfc::string_base & out)const;
		virtual void get_category(pfc::string_base & out)const;
		unsigned get_type() const;


		enum { config_version_current = 1 };
		virtual void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort);
		virtual void get_config(stream_writer * p_writer, abort_callback & p_abort) const;
		static t_size g_get_stream_index_by_window(const uie::window_ptr & ptr);
		static void g_on_orderedbysplitters_change();
		static void g_on_fields_change();
		static t_size g_get_field_index_by_name(const char * p_name);

		static void g_on_field_title_change(const char * p_old, const char * p_new);
		static void g_on_vertical_item_padding_change();

		static void g_on_field_query_change(const field_t & field);
		static void g_on_showemptyitems_change(bool b_val);
		static void g_on_edgestyle_change();
		static void g_on_font_items_change();

		static void g_on_font_header_change();
		static void g_redraw_all();
		static void g_on_new_field(const field_t & field);
		static void g_on_fields_swapped(t_size index_1, t_size index_2);
		static void g_on_field_removed(t_size index);

		void refresh(bool b_allow_autosend = true);

	private:
		static const GUID g_extension_guid;

		class field_data_t
		{
		public:
			bool m_use_script;
			pfc::string8 m_script, m_name;
			pfc::list_t<pfc::string8> m_fields;

			field_data_t() : m_use_script(false) {};

			bool is_empty() { return !m_use_script && !m_fields.get_count(); }
			void reset() { *this = field_data_t(); }
		};

		static pfc::list_t<field_data_t> g_field_data;

		field_data_t m_field_data;

		t_size get_field_index();

		void get_windows(pfc::list_base_t<filter_panel_t*> & windows);

		static void g_create_field_data(const field_t & field, field_data_t & p_out);

		static void g_load_fields();
		static void g_update_subsequent_filters(const pfc::list_base_const_t<filter_panel_t* > & windows, t_size index, bool b_check_needs_update = false, bool b_update_playlist = true);

		void update_subsequent_filters(bool b_allow_autosend = true);

		void get_initial_handles(metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_out);

		void set_field(const field_data_t & field, bool b_force = false);

		virtual void notify_update_item_data(t_size index);

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

		bool is_visible() { RECT rc; return get_wnd() && IsWindowVisible(get_wnd()) && GetClientRect(get_wnd(), &rc) && RECT_CY(rc) > 0; }

		virtual void notify_on_create();
		virtual void notify_on_initialisation();
		virtual void notify_on_destroy();
		//void set_focus() {SetFocus(get_wnd());}
		virtual void render_background(HDC dc, const RECT * rc);
		virtual void render_item(HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc);
		virtual void render_get_colour_data(colour_data_t & p_out) override;

		virtual t_size get_highlight_item();
		virtual bool notify_on_keyboard_keydown_search();
#ifdef FILTER_OLD_SEARCH
		virtual void notify_on_search_box_contents_change(const char * p_str);
		virtual void notify_on_search_box_close();
		virtual bool notify_on_timer(UINT_PTR timerid);
#endif
		virtual bool notify_on_contextmenu_header(const POINT & pt, const HDHITTESTINFO & ht);
		void notify_on_menu_select(WPARAM wp, LPARAM lp);
		virtual bool notify_on_contextmenu(const POINT & pt);

		virtual bool do_drag_drop(WPARAM wp);

		virtual bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp, bool & b_processed);
		virtual void move_selection(int delta)
		{};
		void get_selection_handles(metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_out, bool fallback = true, bool b_sort = false);
		enum action_t
		{
			action_send_to_autosend,
			action_send_to_autosend_play,
			action_send_to_new,
			action_send_to_new_play,
			action_add_to_active,
		};
		void do_selection_action(action_t action = action_send_to_autosend);
		void do_items_action(const bit_array & p_nodes, action_t action = action_send_to_autosend);
		virtual void execute_default_action(t_size index, t_size column, bool b_keyboard, bool b_ctrl);
		bool notify_on_middleclick(bool on_item, t_size index);
		void send_results_to_playlist(bool b_play = false);
		virtual void notify_on_selection_change(const bit_array & p_affected, const bit_array & p_status, notification_source_t p_notification_source);
		void update_first_node_text(bool b_update = false);

		virtual void on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
		virtual void on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
		virtual void on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
		void _on_items_modified(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);
		void _on_items_added(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);
		void _on_items_removed(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);

		virtual bool notify_before_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, bool b_source_mouse);
		virtual bool notify_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, pfc::string_base & p_text, t_size & p_flags, mmh::comptr_t<IUnknown> & pAutocompleteEntries);
		virtual void notify_save_inline_edit(const char * value);
		virtual void notify_exit_inline_edit();

		void notify_on_set_focus(HWND wnd_lost);
		void notify_on_kill_focus(HWND wnd_receiving);
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

			void ensure_handles_sorted();

			node_t();

			static int g_compare(const node_t & i1, const WCHAR * i2);
			static int g_compare_ptr(const node_t * i1, const WCHAR * i2);
		};
		//metadb_handle_list m_handles;
		//t_string_list_fast m_strings;
		pfc::list_t<node_t> m_nodes;
		//static int g_compare_node();
	};


	class filter_search_bar :
		public uie::container_uie_window_v2
	{
		class menu_node_show_clear_button : public ui_extension::menu_node_command_t
		{
			service_ptr_t<filter_search_bar> p_this;
		public:
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const;
			virtual bool get_description(pfc::string_base & p_out) const;
			virtual void execute();
			menu_node_show_clear_button(filter_search_bar * wnd) : p_this(wnd) {};
		};
	public:
		static bool g_activate();
		//static void g_on_orderedbysplitters_change();
		static bool g_filter_search_bar_has_stream(filter_search_bar const* p_seach_bar, const filter_panel_t::filter_stream_t * p_stream);

		template <class TStream>
		static void g_initialise_filter_stream(const TStream & p_stream)
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


		virtual void get_name(pfc::string_base & out) const { out = "Filter search"; }
		virtual const GUID & get_extension_guid() const { return cui::toolbars::guid_filter_search_bar; }
		virtual void get_category(pfc::string_base & out) const { out = "Toolbars"; }
		virtual unsigned get_type() const { return uie::type_toolbar; }
		//virtual HBRUSH get_class_background() const {return HBRUSH(COLOR_WINDOW+1);}
		virtual t_uint32 get_flags() const { return flag_default_flags_plus_transparent_background; }

		filter_search_bar();

	private:
		virtual const GUID & get_class_guid() { return cui::toolbars::guid_filter_search_bar; }

		virtual void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort);
		virtual void get_config(stream_writer * p_writer, abort_callback & p_abort) const;
		virtual void get_menu_items(uie::menu_hook_t & p_hook);

		void on_show_clear_button_change();
		void on_search_editbox_change();
		void commit_search_results(const char * str, bool b_force_autosend = false, bool b_stream_update = false);

		void update_favourite_icon(const char * p_new = NULL);

		virtual LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
		void create_edit();
		void on_size(t_size cx, t_size cy);
		void activate();

		static LRESULT WINAPI g_on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
		LRESULT on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

		using uie::container_uie_window_v2::on_size;

		enum { id_edit = 668, id_toolbar };
		enum { TIMER_QUERY = 1001 };
		enum { idc_clear = 1001, idc_favourite = 1002, msg_favourite_selected = WM_USER + 2 };
		enum { config_version_current = 0 };

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

	class appearance_client_filter_impl : public cui::colours::client
	{
	public:
		static const GUID g_guid;

		virtual const GUID & get_client_guid() const { return g_guid; };
		virtual void get_name(pfc::string_base & p_out) const { p_out = "Filter Panel"; };

		virtual t_size get_supported_colours() const { return cui::colours::colour_flag_all; }; //bit-mask
		virtual t_size get_supported_fonts() const { return 0; }; //bit-mask
		virtual t_size get_supported_bools() const { return cui::colours::bool_flag_use_custom_active_item_frame; }; //bit-mask
		virtual bool get_themes_supported() const { return true; };

		virtual void on_colour_changed(t_size mask) const
		{
			filter_panel_t::g_redraw_all();
		};
		virtual void on_font_changed(t_size mask) const {};
		virtual void on_bool_changed(t_size mask) const {};
	};


};

#endif