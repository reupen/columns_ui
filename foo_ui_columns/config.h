#ifndef _COLUMNS_CONFIG_H_
#define _COLUMNS_CONFIG_H_

#define COLOUR_HELP "Style string - $set_style(text,<text colour>,<selected text colour>)\r\n$set_style(back,<background colour>,<selected background colour>[,<selected background colour no focus>])\r\n\r\n"\
					"Square brackets denote an optional parameter."

extern cfg_int cfg_import_titles,
cfg_export_titles;

namespace columns
{
	const GUID & config_get_playlist_view_guid();
	const GUID & config_get_main_guid();
}

void g_set_tab(const char * name);
void g_show_artwork_settings();

class preferences_tab
{
public:
	virtual HWND create(HWND wnd)=0;
	virtual const char * get_name()=0;
	virtual bool get_help_url(pfc::string_base & p_out)=0;
};

preferences_tab * g_get_tab_layout();
preferences_tab * g_get_tab_columns_v3();
preferences_tab * g_get_tab_filter();
preferences_tab * g_get_tab_artwork();
preferences_tab * g_get_tab_display2();
preferences_tab * g_get_tab_sys();
preferences_tab * g_get_tab_playlist();
preferences_tab * g_get_tab_playlist_dd();
preferences_tab * g_get_tab_main();
preferences_tab * g_get_tab_status();
preferences_tab * g_get_tab_global();


void colour_code_gen(HWND parent, UINT edit, bool markers, bool init);
bool colour_picker(HWND wnd, cfg_int & out, COLORREF custom);
bool colour_picker(HWND wnd, COLORREF & out, COLORREF custom);
bool colour_picker2(HWND wnd, config_item_t<COLORREF> & p_out, COLORREF custom);
BOOL font_picker(LOGFONT & p_font, HWND parent);
bool font_picker(HWND wnd, cfg_struct_t<LOGFONT> & out);
void preview_to_console(const char * spec, bool extra);

extern cfg_struct_t<LOGFONT> cfg_editor_font;

class editor_font_notify
{
	HFONT g_edit_font;
	HWND wnd;
	void _set()
	{
		if (wnd) 
		{
			g_edit_font = CreateFontIndirect(&(LOGFONT)cfg_editor_font);
			SendMessage(wnd,WM_SETFONT,(WPARAM)g_edit_font,MAKELPARAM(1,0));
		}
	}
	void _release()
	{
		if (g_edit_font!=0)
		{
			if (wnd) SendMessage(wnd,WM_SETFONT,(WPARAM)0,MAKELPARAM(0,0));
			DeleteObject(g_edit_font);
			g_edit_font=0;
		}
	}
public:
	void on_change()
	{
		_release();
		_set();
	}
	void set(HWND new_wnd)
	{
		_release();
		wnd = new_wnd;
		_set();
	}
	void release()
	{
		wnd=0;
		_release();
	}
	
	
	editor_font_notify() : g_edit_font(0) {};
	~editor_font_notify()
	{
		if (g_edit_font) {DeleteObject(g_edit_font);g_edit_font=0;}
	}
};
void speedtest(column_list_cref_t columns, bool b_global, bool b_legacy, bool b_date);

extern editor_font_notify g_editor_font_notify;
extern cfg_int g_last_colour;

void g_import_pv_colours_to_unified_global();
void g_import_fonts_to_unified(bool b_pv=true, bool b_ps=true, bool b_status=true);

cui::colours::colour_mode_t g_get_global_colour_mode();
void g_set_global_colour_mode(cui::colours::colour_mode_t p_mode);


class string_font_desc : private pfc::string8_fast_aggressive
{
public:
	operator const char * () const {return get_ptr();}
	string_font_desc(const LOGFONT & lf)
	{
		prealloc(64);
		HDC dc = GetDC(0);		
		unsigned pt = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
		ReleaseDC(0, dc);

		add_string(pfc::stringcvt::string_utf8_from_wide(lf.lfFaceName, tabsize(lf.lfFaceName)));
		add_byte(' ');
		add_string(pfc::format_int(pt));
		add_string("pt");
		if (lf.lfWeight == FW_BOLD)
			add_string(" Bold");
		if (lf.lfItalic)
			add_string(" Itallic");
	}
};

#endif