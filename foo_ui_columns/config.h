#pragma once

#include "columns_v2.h"
#include "config_host.h"

#define COLOUR_HELP "Style string - $set_style(text,<text colour>,<selected text colour>)\r\n"\
                    "$set_style(back,<background colour>,<selected background colour>[,<selected background colour no focus>])\r\n\r\n"\
                    "Square brackets denote an optional parameter."

extern cfg_int cfg_import_titles,
cfg_export_titles;

namespace columns
{
    const GUID & config_get_playlist_view_guid();
    const GUID & config_get_main_guid();
}

namespace fonts {
    extern const GUID playlist_switcher;
    extern const GUID playlist_tabs;
    extern const GUID splitter_tabs;
    extern const GUID status_bar;
    extern const GUID columns_playlist_items;
    extern const GUID ng_playlist_items;
    extern const GUID filter_items;
    extern const GUID columns_playlist_header;
    extern const GUID ng_playlist_header;
    extern const GUID filter_header;
}

void g_show_artwork_settings();

preferences_tab * g_get_tab_layout();
preferences_tab * g_get_tab_artwork();
preferences_tab * g_get_tab_display2();
preferences_tab * g_get_tab_sys();
preferences_tab * g_get_tab_playlist();
preferences_tab * g_get_tab_playlist_dd();
preferences_tab * g_get_tab_main();
preferences_tab * g_get_tab_status();
preferences_tab * g_get_tab_global();

extern preferences_tab * const g_tab_filter_fields, *const g_tab_filter_misc;

void refresh_appearance_prefs();
void colour_code_gen(HWND parent, UINT edit, bool markers, bool init);
bool colour_picker(HWND wnd, COLORREF & out, COLORREF custom);
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
            g_edit_font = CreateFontIndirect(&cfg_editor_font.get_value());
            SendMessage(wnd,WM_SETFONT,(WPARAM)g_edit_font,MAKELPARAM(1,0));
        }
    }
    void _release()
    {
        if (g_edit_font!=nullptr)
        {
            if (wnd) SendMessage(wnd,WM_SETFONT,(WPARAM)0,MAKELPARAM(0,0));
            DeleteObject(g_edit_font);
            g_edit_font=nullptr;
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
        wnd=nullptr;
        _release();
    }
    
    
    editor_font_notify() : g_edit_font(nullptr), wnd(nullptr) {};
    ~editor_font_notify()
    {
        if (g_edit_font) {DeleteObject(g_edit_font);g_edit_font=nullptr;}
    }
};
void speedtest(column_list_cref_t columns, bool b_global, bool b_legacy, bool b_date);

extern editor_font_notify g_editor_font_notify;
extern cfg_uint g_last_colour;
extern const GUID g_guid_columns_ui_preferences_page;

void on_global_colours_change();

cui::colours::colour_mode_t g_get_global_colour_mode();
void g_set_global_colour_mode(cui::colours::colour_mode_t p_mode);


class string_font_desc : private pfc::string8_fast_aggressive
{
public:
    operator const char * () const {return get_ptr();}
    string_font_desc(const LOGFONT & lf)
    {
        prealloc(64);
        HDC dc = GetDC(nullptr);        
        unsigned pt = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
        ReleaseDC(nullptr, dc);

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

namespace cui {
    namespace preferences {
        extern service_factory_single_t<config_host_generic> page_main;
        extern service_factory_single_t<config_host_generic> page_playlist_view;
        extern service_factory_single_t<config_host_generic> page_playlist_switcher;
        extern service_factory_single_t<config_host_generic> page_filters;
    }
}