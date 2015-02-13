#pragma once

#include "foo_ui_columns.h"

extern const GUID g_guid_item_details;
extern const GUID g_guid_item_details_tracking_mode;
extern const GUID g_guid_item_details_script;
extern const GUID g_guid_item_details_font_client;
extern const GUID g_guid_item_details_colour_client;
extern const GUID g_guid_item_details_hscroll;
extern const GUID g_guid_item_details_horizontal_alignment;
extern const GUID g_guid_item_details_vertical_alignment;
extern const GUID g_guid_item_details_word_wrapping;
extern const GUID g_guid_item_details_edge_style;

extern cfg_uint cfg_item_details_tracking_mode;
extern cfg_uint cfg_item_details_edge_style;
extern cfg_bool cfg_item_details_hscroll;
extern cfg_uint cfg_item_details_horizontal_alignment;
extern cfg_uint cfg_item_details_vertical_alignment;
extern cfg_bool cfg_item_details_word_wrapping;


class line_info_t
{
public:
	t_size m_bytes;
	t_size m_raw_bytes;

	line_info_t() : m_bytes(NULL), m_raw_bytes(NULL) {};
};

class display_line_info_t : public line_info_t
{
public:
	t_size m_width;
	t_size m_height;
};

class font_data_t
{
public:
	pfc::string8 m_face;
	t_size m_point;
	bool m_bold, m_underline, m_italic;

	font_data_t() : m_point(10), m_bold(false), m_underline(false), m_italic(false) {};

	static int g_compare(const font_data_t & item1, const font_data_t & item2)
	{
		int ret = stricmp_utf8(item1.m_face, item2.m_face);
		if (ret == 0) ret = pfc::compare_t(item1.m_point, item2.m_point);
		if (ret == 0) ret = pfc::compare_t(item1.m_bold, item2.m_bold);
		if (ret == 0) ret = pfc::compare_t(item1.m_underline, item2.m_underline);
		if (ret == 0) ret = pfc::compare_t(item1.m_italic, item2.m_italic);
		return ret;
	}
};

bool operator == (const font_data_t & item1, const font_data_t & item2);

class font_change_data_t
{
public:
	font_data_t m_font_data;
	bool m_reset;
	t_size m_character_index;

	font_change_data_t() : m_reset(false), m_character_index(NULL) {};
};

typedef pfc::list_t<font_change_data_t, pfc::alloc_fast_aggressive> font_change_data_list_t;

typedef pfc::list_t<line_info_t, pfc::alloc_fast_aggressive> line_info_list_t;
typedef pfc::list_t<display_line_info_t, pfc::alloc_fast_aggressive> display_line_info_list_t;

void g_parse_font_format_string(const char * str, t_size len, font_data_t & p_out);
void g_get_text_font_data(const char * p_text, pfc::string8_fast_aggressive & p_new_text, font_change_data_list_t & p_out);



class font_t : public pfc::refcounted_object_root
{
public:
	typedef pfc::refcounted_object_ptr_t<font_t> ptr_t;


	font_data_t m_data;

	gdi_object_t<HFONT>::ptr_t m_font;
	t_size m_height;
	//font_t (const font_t & p_font) : m_font(p_font.m_font), m_height(p_font.m_height), m_data(p_font.m_data) {};
};

typedef pfc::list_t<font_t::ptr_t> font_list_t;

class font_change_info_t
{
public:
	class font_change_entry_t
	{
	public:
		t_size m_text_index;
		font_t::ptr_t m_font;
	};

	font_list_t m_fonts;
	pfc::array_t<font_change_entry_t> m_font_changes;
	font_t::ptr_t m_default_font;

	bool find_font(const font_data_t & p_font, t_size & index);

	void reset(bool bKeepHandles = false);
};


//void get_multiline_text_dimensions(HDC dc, pfc::string8_fast_aggressive & text, line_info_list_t & indices, t_size line_height, SIZE & sz, bool b_word_wrapping = false, t_size max_width = pfc_infinite);
void g_get_multiline_text_dimensions(HDC dc, pfc::string8_fast_aggressive & text_new, const line_info_list_t & rawLines, display_line_info_list_t & displayLines, const font_change_info_t & p_font_data, t_size line_height, SIZE & sz, bool b_word_wrapping = false, t_size max_width = pfc_infinite);
//void get_multiline_text_dimensions_const(HDC dc, const char * text, const line_info_list_t & indices, t_size line_height, SIZE & sz, bool b_word_wrapping = false, t_size max_width = pfc_infinite);
void g_get_multiline_text_dimensions_const(HDC dc, const char * text, const line_info_list_t & indices, const font_change_info_t & p_font_data, t_size line_height, SIZE & sz, bool b_word_wrapping = false, t_size max_width = pfc_infinite);
void g_get_text_multiline_data(const char * text, pfc::string8_fast_aggressive & p_out, pfc::list_t<line_info_t, pfc::alloc_fast_aggressive> & indices);

void g_parse_font_format_string(const char * str, t_size len, font_data_t & p_out);
void g_get_text_font_data(const char * p_text, pfc::string8_fast_aggressive & p_new_text, font_change_data_list_t & p_out);
void g_get_text_font_info(const font_change_data_list_t & p_data, font_change_info_t & p_info);

void g_text_out_multiline_font(HDC dc, const RECT & rc_topleft, t_size line_height, const char * text, const font_change_info_t & p_font_data, const display_line_info_list_t & newLineDataWrapped, const SIZE & sz, COLORREF cr_text, ui_helpers::alignment align, bool b_hscroll, bool word_wrapping);

class titleformat_hook_change_font : public titleformat_hook
{
public:
	virtual bool process_field(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, bool & p_found_flag);

	virtual bool process_function(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag);

	titleformat_hook_change_font(const LOGFONT & lf);
private:
	pfc::string8 m_default_font_face;
	t_size m_default_font_size;
};



class font_code_generator_t
{
	class string_font_code : private pfc::string8_fast_aggressive
	{
	public:
		operator const char * () const;
		string_font_code(const LOGFONT & lf);
	};
public:
	void run(HWND parent, UINT edit);
	void initialise(const LOGFONT & p_lf_default, HWND parent, UINT edit);
private:
	LOGFONT m_lf;
};


class item_details_t :
	public uie::container_ui_extension,
	public ui_selection_callback,
	public play_callback,
	public playlist_callback_single,
	public metadb_io_callback_dynamic
{
	class message_window_t : public ui_helpers::container_window
	{
		virtual class_data & get_class_data() const;
		virtual LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	};

	static message_window_t g_message_window;

	enum
	{
		MSG_REFRESH = WM_USER + 2,
	};

	virtual class_data & get_class_data()const;

public:
	enum track_mode_t
	{
		track_auto_playlist_playing,
		track_playlist,
		track_playing,
		track_auto_selection_playing,
		track_selection,
	};

	const bool g_track_mode_includes_now_playing(t_size mode);

	const bool g_track_mode_includes_plalist(t_size mode);

	const bool g_track_mode_includes_auto(t_size mode);

	const bool g_track_mode_includes_selection(t_size mode);

	class menu_node_track_mode : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
		t_size m_source;
	public:
		static const char * get_name(t_size source);
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
		virtual bool get_description(pfc::string_base & p_out)const;
		virtual void execute();
		menu_node_track_mode(item_details_t * p_wnd, t_size p_value);;
	};

	class menu_node_source_popup : public ui_extension::menu_node_popup_t
	{
		pfc::list_t<ui_extension::menu_node_ptr> m_items;
	public:
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
		virtual unsigned get_children_count()const;
		virtual void get_child(unsigned p_index, uie::menu_node_ptr & p_out)const;
		menu_node_source_popup(item_details_t * p_wnd);;
	};

	class menu_node_alignment : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
		t_size m_type;
	public:
		static const char * get_name(t_size source);
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
		virtual bool get_description(pfc::string_base & p_out)const;
		virtual void execute();
		menu_node_alignment(item_details_t * p_wnd, t_size p_value);;
	};

	class menu_node_alignment_popup : public ui_extension::menu_node_popup_t
	{
		pfc::list_t<ui_extension::menu_node_ptr> m_items;
	public:
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
		virtual unsigned get_children_count()const;
		virtual void get_child(unsigned p_index, uie::menu_node_ptr & p_out)const;
		menu_node_alignment_popup(item_details_t * p_wnd);;
	};

	class menu_node_options : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
	public:
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
		virtual bool get_description(pfc::string_base & p_out)const;
		virtual void execute();
		menu_node_options(item_details_t * p_wnd);;
	};
	class menu_node_hscroll : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
	public:
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
		virtual bool get_description(pfc::string_base & p_out)const;
		virtual void execute();
		menu_node_hscroll(item_details_t * p_wnd);;
	};

	class menu_node_wwrap : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
	public:
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
		virtual bool get_description(pfc::string_base & p_out)const;
		virtual void execute();
		menu_node_wwrap(item_details_t * p_wnd);;
	};

	//UIE funcs
	virtual const GUID & get_extension_guid() const;
	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;
	unsigned get_type() const;

	enum { stream_version_current = 2 };
	void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort);
	void get_config(stream_writer * p_writer, abort_callback & p_abort) const;
	bool have_config_popup()const;
	bool show_config_popup(HWND wnd_parent);

	void get_menu_items(ui_extension::menu_hook_t & p_hook);

	//UI SEL API
	void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr> & p_selection);

	//PC
	void on_playback_starting(play_control::t_track_command p_command, bool p_paused);
	void on_playback_new_track(metadb_handle_ptr p_track);
	void on_playback_stop(play_control::t_stop_reason p_reason);
	void on_playback_seek(double p_time);
	void on_playback_pause(bool p_state);
	void on_playback_edited(metadb_handle_ptr p_track);
	void on_playback_dynamic_info(const file_info & p_info);
	void on_playback_dynamic_info_track(const file_info & p_info);
	void on_playback_time(double p_time);
	void on_volume_change(float p_new_val);

	//PL
	enum {
		playlist_callback_flags =
		playlist_callback_single::flag_on_items_selection_change
		| playlist_callback_single::flag_on_playlist_switch
	};
	void on_playlist_switch();
	void on_item_focus_change(t_size p_from, t_size p_to);;

	void on_items_added(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data, const bit_array & p_selection);
	void on_items_reordered(const t_size * p_order, t_size p_count);
	void on_items_removing(const bit_array & p_mask, t_size p_old_count, t_size p_new_count);
	void on_items_removed(const bit_array & p_mask, t_size p_old_count, t_size p_new_count);
	void on_items_selection_change(const bit_array & p_affected, const bit_array & p_state);
	void on_items_modified(const bit_array & p_mask);
	void on_items_modified_fromplayback(const bit_array & p_mask, play_control::t_display_level p_level);
	void on_items_replaced(const bit_array & p_mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & p_data);
	void on_item_ensure_visible(t_size p_idx);

	void on_playlist_renamed(const char * p_new_name, t_size p_new_name_len);
	void on_playlist_locked(bool p_locked);

	void on_default_format_changed();
	void on_playback_order_changed(t_size p_new_index);

	//ML
	void on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook);

	static void g_on_app_activate(bool b_activated);
	static void g_redraw_all();
	static void g_on_font_change();
	static void g_on_colours_change();

	void set_horizontal_alignment(t_size horizontal_alignment);
	void set_vertical_alignment(t_size vertical_alignment);

	void set_edge_style(t_size edge_style);

	void on_edge_style_change();

	void set_script(const char * p_script);

	void set_config_wnd(HWND wnd);

	item_details_t();;
private:
	LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

	void register_callback();
	void deregister_callback();
	void on_app_activate(bool b_activated);
	void refresh_contents(bool b_new_track = true);
	void update_font_change_info();
	void reset_font_change_info();
	void update_display_info(HDC dc);
	void update_display_info();
	void reset_display_info();
	void on_tracking_mode_change();
	bool check_process_on_selection_changed();

	void scroll(INT sb, int position, bool b_absolute);

	void invalidate_all(bool b_update = true);
	void update_now();
	void update_scrollbar_range(bool b_set_pos = true);

	void on_size();
	void on_size(t_size cx, t_size cy);

	void on_font_change();
	void on_colours_change();

	static pfc::ptr_list_t<item_details_t> g_windows;

	ui_selection_holder::ptr m_selection_holder;
	metadb_handle_list m_handles, m_selection_handles;
	bool m_callback_registered, m_nowplaying_active;//, m_update_scrollbar_range_in_progress;
	t_size m_tracking_mode;

	bool m_font_change_info_valid;
	font_change_info_t m_font_change_info;
	font_change_data_list_t m_font_change_data;

	line_info_list_t m_line_info;
	display_line_info_list_t m_display_line_info;
	bool m_display_info_valid;

	SIZE m_display_sz;

	pfc::string8 m_current_text, m_script, m_current_text_raw;
	pfc::string8_fast_aggressive m_current_display_text;
	titleformat_object::ptr m_to;

	t_size m_horizontal_alignment, m_vertical_alignment;
	t_size m_edge_style;
	bool m_hscroll, m_word_wrapping;

	HWND m_wnd_config;

	//gdi_object_t<HFONT>::ptr_t m_default_font;

	//HINSTANCE m_library_richedit;
	//HWND m_wnd_richedit;
};

class item_details_config_t
{
public:
	pfc::string8 m_script;
	t_size m_edge_style, m_horizontal_alignment, m_vertical_alignment;
	font_code_generator_t m_font_code_generator;
	bool m_modal, m_timer_active;
	service_ptr_t<item_details_t> m_this;
	HWND m_wnd;

	enum { timer_id = 100 };

	static BOOL CALLBACK g_DialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	item_details_config_t(const char * p_text, t_size edge_style, t_size halign, t_size valign);;

	bool run_modal(HWND wnd);
	void run_modeless(HWND wnd, item_details_t* p_this);
private:
	void kill_timer();
	void start_timer();
	void on_timer();
	BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
};
