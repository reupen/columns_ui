#ifndef _COLUMNS_REBAR_H_
#define _COLUMNS_REBAR_H_

/*!
 * \file rebar.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Classes used for the toolbars (rebar control) of the main window
 */

#include "stdafx.h"
#include "extern.h"

struct band_cache_entry
{
	GUID guid;
	unsigned width;
};

class band_cache : public pfc::list_t<band_cache_entry>
{
public:
	void add_entry(const GUID & guid, unsigned width);
	unsigned get_width(const GUID & guid);
	void write(stream_writer * out, abort_callback & p_abort);
	void read(stream_reader * data, abort_callback & p_abort);

	inline void copy(band_cache & in)
	{
		remove_all();
		add_items(in);
	}
};

class cfg_band_cache_t : public cfg_var
{
private:
	band_cache entries;
	
	virtual void get_data_raw(stream_writer * out, abort_callback & p_abort);
	virtual void set_data_raw(stream_reader * p_reader, unsigned p_sizehint, abort_callback & p_abort);
	
public:
	explicit inline cfg_band_cache_t(const GUID & p_guid) : cfg_var(p_guid) {reset();};
	void get_band_cache(band_cache & out);
	void set_band_cache(band_cache & in);
	void reset();
};

class rebar_band_info
{
public:
	GUID guid;
	HWND wnd;
	ui_extension::window_ptr p_ext;
	pfc::array_t<t_uint8> config;
	// Although we store the DPI, this does virtually nothing as remaining space is automatically 
	// distributed among the remaining bands.
	uih::IntegerAndDpi<uint32_t> width;
	bool rbbs_break;

	rebar_band_info(GUID id = pfc::guid_null, unsigned h = 100);
	void rebar_band_info::_export(stream_writer * out, t_uint32 type, abort_callback & p_abort);
	void rebar_band_info::import(stream_reader * p_reader, t_uint32 type, abort_callback & p_abort);
	void rebar_band_info::write(stream_writer * out, abort_callback & p_abort);
	void rebar_band_info::read(stream_reader * p_reader, abort_callback & p_abort);
	void rebar_band_info::write_extra(stream_writer * out, abort_callback & p_abort);
	void rebar_band_info::read_extra(stream_reader * p_reader, abort_callback & p_abort);
	void rebar_band_info::copy(rebar_band_info & out);
};

class rebar_info : public ptr_list_autodel_t<rebar_band_info>
{
public:
	void rebar_info::set_rebar_info(rebar_info & in);
	rebar_band_info * find_by_wnd(HWND wnd);
	unsigned find_by_wnd_n(HWND wnd);
	void rebar_info::add_band(const GUID & id, unsigned width = 125, bool new_line = false, const ui_extension::window_ptr & p_ext = uie::window_ptr_null);
	void rebar_info::insert_band(unsigned idx, const GUID & id, unsigned width = 125, bool new_line = false, const ui_extension::window_ptr & p_ext = uie::window_ptr_null);
};

class cfg_rebar : public cfg_var
{
private:
	enum class StreamVersion:uint32_t {
		Version0 = 0,
		Version1 = 1,
		VersionCurrent = Version1
	};

	rebar_info entries;
	
	virtual void cfg_rebar::get_data_raw(stream_writer * out, abort_callback & p_abort);
	virtual void cfg_rebar::set_data_raw(stream_reader * p_reader, unsigned p_sizehint, abort_callback & p_abort);
	
public:
	void export_config(stream_writer * p_out, t_uint32 mode, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort);
	void import_config(stream_reader * p_reader, t_size size, t_uint32 mode, pfc::list_base_t<GUID> & panels, abort_callback & p_abort);

	explicit inline cfg_rebar::cfg_rebar(const GUID & p_guid) : cfg_var(p_guid) {reset();};
	void cfg_rebar::get_rebar_info(rebar_info & out);
	void cfg_rebar::set_rebar_info(rebar_info & in);
	void cfg_rebar::reset();
};

class rebar_window
{
private:
	void destroy_bands();
public:
	HWND wnd_rebar;
	rebar_info bands;
	band_cache cache;
	
	
	rebar_window();
	HWND init(rebar_info & new_bands);

	void add_band(const GUID & guid, unsigned width = 100, const ui_extension::window_ptr & p_ext = ui_extension::window_ptr_null);
	void insert_band(unsigned idx, const GUID & guid, unsigned width = 100, const ui_extension::window_ptr & p_ext = ui_extension::window_ptr_null);
	void update_bands();
	void delete_band(HWND wnd, bool destroy = true);

	void update_band(HWND wnd, bool size = false/*bool min_height, bool max_height, bool min_width, bool max_width*/);
	void update_band(unsigned n, bool size = false);

	bool check_band(const GUID & id);
	bool find_band(const GUID & id, unsigned & out);
	bool delete_band(const GUID & id);
	void delete_band(unsigned idx);

	void on_themechanged ();

	bool on_menu_char (unsigned short c);
	void show_accelerators();
	bool set_menu_focus();
	void hide_accelerators ();
	bool is_menu_focused();
	bool get_previous_menu_focus_window(HWND & wnd_previous) const;

	//save bands on layout changed - easier
	
	void save_bands();
	void destroy();
	void refresh_bands(bool force_destroy_bands = true, bool save = true);

	~rebar_window();
};

ui_extension::window_host & get_rebar_host();

/*
void save_bar(unsigned ID, unsigned width, unsigned style);
void destroy_rebar(bool des = true);
//LRESULT WINAPI RebarHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
void insert_rebar_item(unsigned id, int insert_idx = -1);
void build_rebar(bool visible = true);

namespace toolbar_images
{
	void create_toolbar_imagelist();
	void destroy_toolbar_imagelist();
};
*/

#endif