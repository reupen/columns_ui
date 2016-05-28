#pragma once

#include "list_view_panel.h"

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
	extern const GUID g_guid_show_column_titles;
	extern const GUID g_guid_allow_sorting;
	extern const GUID g_guid_show_sort_indicators;

	class appearance_client_filter_impl : public cui::colours::client {
	public:
		static const GUID g_guid;

		const GUID & get_client_guid() const override { return g_guid; };

		void get_name(pfc::string_base & p_out) const override { p_out = "Filter Panel"; };

		t_size get_supported_colours() const override { return cui::colours::colour_flag_all; }; //bit-mask

		t_size get_supported_bools() const override { return cui::colours::bool_flag_use_custom_active_item_frame; }; //bit-mask
		bool get_themes_supported() const override { return true; };

		void on_colour_changed(t_size mask) const override;;

		void on_bool_changed(t_size mask) const override {};
	};

	class field_t
	{
	public:
		pfc::string8 m_name, m_field;
		bool m_last_sort_direction;

		field_t() : m_last_sort_direction(false) {}
	};

	class cfg_fields_t : public cfg_var, public pfc::list_t<field_t>
	{
	public:
		enum { stream_version_current = 0 };
		enum { sub_stream_version_current = 0 };

		void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) override;
		void get_data_raw(stream_writer * p_stream, abort_callback & p_abort) override;
		void reset();

		bool find_by_name(const char * p_name, size_t & p_index);
		bool have_name(const char * p_name);
		void fix_name(const char * p_name, pfc::string8 & p_out);
		void fix_name(pfc::string8 & p_name);
		
		cfg_fields_t(const GUID & p_guid) : cfg_var(p_guid) {reset();}
	};

	class cfg_favouriteslist : public cfg_var, public pfc::list_t<pfc::string8>
	{
	public:
		void get_data_raw(stream_writer * p_stream, abort_callback & p_abort) override;
		void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) override;

		bool have_item(const char * p_item);
		bool find_item(const char * p_item, t_size & index);

		cfg_favouriteslist(const GUID & p_guid) : cfg_var(p_guid) {}
	};

	class filter_panel_t;
	class filter_search_bar;

	extern cfg_favouriteslist cfg_favourites;

	extern cfg_string /*cfg_fields, */cfg_sort_string;
	extern cfg_bool cfg_sort, cfg_autosend, cfg_orderedbysplitters, cfg_showemptyitems, cfg_showsearchclearbutton;
	extern cfg_int cfg_doubleclickaction, cfg_middleclickaction, cfg_edgestyle;
	extern cfg_fields_t cfg_field_list;

	extern uih::ConfigInt32DpiAware cfg_vertical_item_padding;
	extern uih::ConfigBool cfg_show_column_titles, cfg_allow_sorting, cfg_show_sort_indicators;

	class filter_panel_t :
		public uie::container_ui_extension_t<t_list_view_panel<appearance_client_filter_impl>, uie::window>,
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

		~filter_panel_t() = default;;

		static pfc::list_t<filter_stream_t::ptr> g_streams;
		filter_stream_t::ptr m_stream;

		static pfc::ptr_list_t<filter_panel_t> g_windows;

		const GUID & get_extension_guid() const override;
		void get_name(pfc::string_base & out)const override;
		void get_category(pfc::string_base & out)const override;
		unsigned get_type() const override;


		enum { config_version_current = 1 };

		void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort) override;
		void get_config(stream_writer * p_writer, abort_callback & p_abort) const override;
		static t_size g_get_stream_index_by_window(const uie::window_ptr & ptr);
		static void g_on_orderedbysplitters_change();
		static void g_on_fields_change();
		static t_size g_get_field_index_by_name(const char * p_name);

		static void g_on_field_title_change(const char * p_old, const char * p_new);
		static void g_on_vertical_item_padding_change();
		static void g_on_show_column_titles_change();
		static void g_on_allow_sorting_change();
		static void g_on_show_sort_indicators_change();

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
			bool m_use_script, m_last_sort_direction;
			pfc::string8 m_script, m_name;
			pfc::list_t<pfc::string8> m_fields;

			field_data_t() : m_use_script(false), m_last_sort_direction(false) {};

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

		void notify_update_item_data(t_size index) override;

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

		void notify_on_create() override;
		void notify_on_initialisation() override;
		void notify_on_destroy() override;
		//void set_focus() {SetFocus(get_wnd());}
		void render_background(HDC dc, const RECT * rc) override;
		void render_item(HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc) override;

		t_size get_highlight_item() override;
		bool notify_on_keyboard_keydown_search() override;
#ifdef FILTER_OLD_SEARCH
		virtual void notify_on_search_box_contents_change(const char * p_str);
		virtual void notify_on_search_box_close();
		virtual bool notify_on_timer(UINT_PTR timerid);
#endif
		bool notify_on_contextmenu_header(const POINT & pt, const HDHITTESTINFO & ht) override;
		void notify_on_menu_select(WPARAM wp, LPARAM lp) override;
		bool notify_on_contextmenu(const POINT & pt) override;

		void notify_sort_column(t_size index, bool b_descending, bool b_selection_only) override;

		bool do_drag_drop(WPARAM wp) override;

		bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp, bool & b_processed) override;

		void move_selection(int delta) override {};
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
		void execute_default_action(t_size index, t_size column, bool b_keyboard, bool b_ctrl) override;
		bool notify_on_middleclick(bool on_item, t_size index) override;
		void send_results_to_playlist(bool b_play = false);
		void notify_on_selection_change(const bit_array & p_affected, const bit_array & p_status, notification_source_t p_notification_source) override;
		void update_first_node_text(bool b_update = false);

		void on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) override;
		void on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) override;
		void on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) override;
		void _on_items_modified(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);
		void _on_items_added(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);
		void _on_items_removed(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & p_data);

		bool notify_before_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, bool b_source_mouse) override;
		bool notify_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, pfc::string_base & p_text, t_size & p_flags, mmh::comptr_t<IUnknown> & pAutocompleteEntries) override;
		void notify_save_inline_edit(const char * value) override;
		void notify_exit_inline_edit() override;

		void notify_on_set_focus(HWND wnd_lost) override;
		void notify_on_kill_focus(HWND wnd_receiving) override;

		t_size get_drag_item_count() override {return m_drag_item_count;}

		ui_selection_holder::ptr m_selection_holder;

		t_size m_drag_item_count;
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
			static int g_compare_ptr_with_node(const node_t & i1, const node_t & i2);
		};
		pfc::list_t<node_t> m_nodes;
		bool m_pending_sort_direction;
	};
};
