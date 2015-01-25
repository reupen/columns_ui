#if 0

#ifndef _COLUMNS_SIDEBAR_H_
#define _COLUMNS_SIDEBAR_H_

#include "foo_ui_columns.h"

extern GUID null_guid;

class extension_info
{
public:
	GUID guid;
	HWND wnd_panel;
	HWND wnd_host;
	ui_extension::window_ptr p_ext;
	pfc::array_t<t_uint8> config;
	unsigned height, min_height, max_height, min_width, max_width;
	bool locked;
	bool show_caption;
	bool hidden;
	HTHEME theme;

	extension_info(GUID id = null_guid, unsigned h = 100) : guid(id), wnd_host(0), wnd_panel(0), p_ext(0), height(h), min_height(0), max_height(0), show_caption(true), 
		min_width(0), max_width(0),locked(false),hidden(false), theme(0) {};
	void extension_info::write(stream_writer * out, abort_callback & p_abort);
	void extension_info::read(stream_reader * p_reader, abort_callback & p_abort);
	void extension_info::copy(extension_info & out);
};

class sidebar_info : public ptr_list_autodel_t<extension_info>
{
	//mem_block_list<extension_info> panels;
public:
	void sidebar_info::set_sidebar_info(sidebar_info & in);
	bool sidebar_info::move_up(unsigned idx);
	bool sidebar_info::move_down(unsigned idx);
	extension_info * sidebar_info::find_by_wnd(HWND wnd);
};

class cfg_sidebar : public cfg_var
{
private:
	sidebar_info entries;
	
	virtual void get_data_raw(stream_writer * out, abort_callback & p_abort);
	virtual void set_data_raw(stream_reader * p_reader, unsigned p_sizehint, abort_callback & p_abort);
	
public:
	explicit inline cfg_sidebar(const GUID & p_guid) : cfg_var(p_guid) {reset();};
	void get_sidebar_info(sidebar_info & out);
	void set_sidebar_info(sidebar_info & in);
	void reset();
};


class sidebar_window
{
	static const TCHAR * class_name;
	static const TCHAR * panel_host_class_name;
	static bool class_registered;
	static bool panel_host_class_registered;

/*	static long ref;
	static void de_ref();
	static void add_ref();*/
	enum {IDM_CLOSE=1, IDM_MOVE_UP, IDM_MOVE_DOWN, IDM_LOCK, IDM_CAPTION, IDM_BASE};
	unsigned menu_ext_base;
	
	service_ptr_t<ui_status_text_override> m_status_override;
public:
	HWND wnd_sidebar;
	sidebar_info panels;
	
	static LRESULT WINAPI sidebar_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static LRESULT WINAPI panel_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static void unregister_class();
	static void on_size(HWND wnd,int width,int height);
	inline static void g_on_size(HWND wnd)
	{
		RECT rc;
		GetClientRect(wnd, &rc);
		on_size(wnd, rc.right, rc.bottom);
	}


	sidebar_window();
	HWND init(sidebar_info & new_panels, HWND parent);
	LRESULT WINAPI on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	
	void size_panels(bool set = false);
	void refresh_panels();
	int override_size(unsigned & panel, int delta);
	bool check_panel(const GUID & id);
	bool find_panel(const GUID & id, unsigned & out);
	bool find_panel_by_host(const HWND wnd, unsigned & out);
	bool find_panel_by_panel(const HWND wnd, unsigned & out);
	bool delete_panel(const GUID & id);

	void remove_panel(unsigned idx);

	bool on_menu_char (unsigned short c);
	void on_menu_key_down ();
	void on_sys_command_menu_key ();
	void hide_accelerators ();
	
	void destroy();
	~sidebar_window();
};

#endif
#endif