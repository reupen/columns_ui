#pragma once

#include "config_appearance.h"
#include "config.h"

class tab_appearance_fonts : public preferences_tab
{
	HWND m_wnd;
	HWND m_wnd_colours_mode;
	HWND m_wnd_colours_element;
	fonts_manager_data::entry_ptr_t m_element_ptr;
	cui::fonts::client::ptr m_element_api;
	fonts_client_list_t m_fonts_client_list;
public:
	tab_appearance_fonts();;

	void refresh_me(HWND wnd);


	static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

	void update_mode_combobox();

	void get_font(LOGFONT & lf);

	void update_change();

	void update_font_desc();

	void on_font_changed();

	BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	void apply();
	virtual HWND create(HWND wnd);
	virtual const char * get_name();
	bool get_help_url(pfc::string_base & p_out);
	bool is_active();

private:
	bool initialising;
};
