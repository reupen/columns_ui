#ifndef _COLUMNS_SPLITTER_H_
#define _COLUMNS_SPLITTER_H_

enum orientation_t {
	horizontal,
	vertical,
};

class splitter_window_impl :
	public uie::container_ui_extension_t < ui_helpers::container_window, uie::splitter_window_v2 >
{
public:

	virtual orientation_t get_orientation()const = 0;
	static unsigned g_get_caption_size();
	void get_category(pfc::string_base & p_out) const;
	virtual unsigned get_type() const;;

	virtual void insert_panel(unsigned index, const uie::splitter_item_t *  p_item);

	virtual void remove_panel(unsigned index);;
	virtual void replace_panel(unsigned index, const uie::splitter_item_t *  p_item);

	virtual unsigned get_panel_count()const;;
	virtual uie::splitter_item_t * get_panel(unsigned index)const;;
	enum { stream_version_current = 0 };
	virtual void set_config(stream_reader * config, t_size p_size, abort_callback & p_abort);;
	virtual void import_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort);;
	virtual void export_config(stream_writer * p_writer, abort_callback & p_abort) const;;

	virtual void get_config(stream_writer * out, abort_callback & p_abort) const;;

	virtual bool have_config_popup(unsigned index) const;

	virtual bool show_config_popup(HWND wnd, unsigned index);

	bool is_index_valid(unsigned index) const;

	virtual bool get_config_item_supported(unsigned index, const GUID & p_type) const;

	virtual bool get_config_item(unsigned index, const GUID & p_type, stream_writer * p_out, abort_callback & p_abort) const;

	virtual bool set_config_item(unsigned index, const GUID & p_type, stream_reader * p_source, abort_callback & p_abort);;

	class splitter_host_impl : public ui_extension::window_host_ex
	{
		service_ptr_t<splitter_window_impl > m_this;
	public:
		virtual const GUID & get_host_guid()const;

		virtual bool get_keyboard_shortcuts_enabled()const;
		virtual void get_children(pfc::list_base_t<uie::window::ptr> & p_out);

		virtual void on_size_limit_change(HWND wnd, unsigned flags);;

		//unsigned get_orientation();
		orientation_t get_orientation()const;

		virtual unsigned is_resize_supported(HWND wnd)const;

		virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height);

		virtual bool override_status_text_create(service_ptr_t<ui_status_text_override> & p_out);

		virtual bool is_visible(HWND wnd) const;

		virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility) const;
		virtual bool set_window_visibility(HWND wnd, bool visibility);

		void set_window_ptr(splitter_window_impl * p_ptr);

		virtual void relinquish_ownership(HWND wnd);

	};
	virtual bool is_point_ours(HWND wnd_point, const POINT & pt_screen, pfc::list_base_t<uie::window::ptr> & p_hierarchy);
	virtual void get_supported_panels(const pfc::list_base_const_t<uie::window::ptr> & p_windows, bit_array_var & p_mask_unsupported);
private:


	struct t_size_limit
	{
		unsigned min_height;
		unsigned max_height;
		unsigned min_width;
		unsigned max_width;
		t_size_limit()
			: min_height(0), max_height(0), min_width(0), max_width(0)
		{};
	};
	class panel :
		public pfc::refcounted_object_root
	{
	public:
		class panel_container :
			public ui_helpers::container_window,
			private message_hook_manager::message_hook
		{
		public:
			enum { MSG_AUTOHIDE_END = WM_USER + 2 };

			panel_container(panel * p_panel);;
			~panel_container();
			void set_window_ptr(splitter_window_impl * p_ptr);
			void enter_autohide_hook();
			//private:
			virtual class_data & get_class_data()const;
			virtual LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
			virtual bool on_hooked_message(message_hook_manager::t_message_hook_type p_type, int code, WPARAM wp, LPARAM lp);
			service_ptr_t< splitter_window_impl > m_this;

			HTHEME m_theme;
			panel * m_panel;

			bool m_hook_active;
			bool m_timer_active;
		} m_container;

		GUID m_guid;
		unsigned m_caption_orientation;
		bool m_locked;
		bool m_hidden;
		bool m_autohide;
		HWND m_wnd;
		HWND m_wnd_child;
		bool m_show_caption;
		pfc::array_t<t_uint8> m_child_data;
		t_size_limit m_size_limits;
		uie::window_ptr m_child;
		bool m_show_toggle_area;
		bool m_use_custom_title;
		pfc::string8 m_custom_title;

		service_ptr_t<class splitter_host_impl> m_interface;

		unsigned m_size;

		uie::splitter_item_full_t * create_splitter_item(bool b_set_ptr = true);

		void set_from_splitter_item(const uie::splitter_item_t * p_source);

		void export(stream_writer * out, abort_callback & p_abort);
		void write(stream_writer * out, abort_callback & p_abort);
		void import(stream_reader*t, abort_callback & p_abort);
		void read(stream_reader*t, abort_callback & p_abort);

		void set_hidden(bool val);

		void on_size();
		void on_size(unsigned cx, unsigned cy);

		void destroy();
		panel();;
	};
	class panel_list : public pfc::list_t < pfc::refcounted_object_ptr_t<panel> >
	{
	public:
		bool move_up(unsigned idx);
		bool move_down(unsigned idx);
		bool find_by_wnd(HWND wnd, unsigned & p_out);
		bool find_by_wnd_child(HWND wnd, unsigned & p_out);
	};

	void start_autohide_dehide(unsigned index, bool b_next_too = true);

	virtual LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	void get_panels_sizes(unsigned width, unsigned height, pfc::list_base_t<unsigned> & p_out);
	bool find_by_divider_pt(POINT & pt, unsigned & p_out);
	bool test_divider_pt(const POINT & pt, unsigned p_out);

	unsigned get_panel_divider_size(unsigned index);

	void on_size_changed(unsigned width, unsigned height);
	void on_size_changed();
	void save_sizes();

	void save_sizes(unsigned width, unsigned height);

	int override_size(unsigned & panel, int delta);

	void refresh_children();
	void destroy_children();

	//unsigned get_orientation();
	panel_list m_panels;
	HWND m_wnd;

	int m_last_position;
	unsigned m_panel_dragging;
	bool m_panel_dragging_valid;

	static gdi_object_t<HFONT>::ptr_t g_font_menu_horizontal;
	static gdi_object_t<HFONT>::ptr_t g_font_menu_vertical;
	static unsigned g_count;
public:
	splitter_window_impl();;

	//
};



class WindowEnum_t
{
	static BOOL CALLBACK g_EnumWindowsProc(HWND wnd, LPARAM lp)
	{
		return ((WindowEnum_t*)lp)->EnumWindowsProc(wnd);
	}
	BOOL EnumWindowsProc(HWND wnd)
	{
		if (GetWindow(wnd, GW_OWNER) == m_wnd_owner && IsWindowVisible(wnd))
			m_wnd_list.add_item(wnd);
		return TRUE;
	}
	HWND m_wnd_owner;
public:
	void run()
	{
		EnumWindows(&g_EnumWindowsProc, (LPARAM)this);
	}
	pfc::list_t<HWND, pfc::alloc_fast> m_wnd_list;
	WindowEnum_t(HWND wnd_owner) : m_wnd_owner(wnd_owner) {};
};

void g_get_panel_list(uie::window_info_list_simple & p_out, uie::window_host_ptr & p_host);
void g_append_menu_panels(HMENU menu, const uie::window_info_list_simple & panels, UINT base);
void g_append_menu_splitters(HMENU menu, const uie::window_info_list_simple & panels, UINT base);
void g_run_live_edit_contextmenu(HWND wnd, POINT pt_menu, window_transparent_fill & p_overlay, const RECT & rc_overlay, uie::window_ptr ptr, uie::splitter_window_ptr p_container, t_size index, uie::window_host_ptr & p_host);
void clip_minmaxinfo(MINMAXINFO & mmi);

#endif
