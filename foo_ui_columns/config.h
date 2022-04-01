#pragma once

#include "columns_v2.h"
#include "config_host.h"
#include "font_utils.h"

namespace columns {
const GUID& config_get_playlist_view_guid();
const GUID& config_get_main_guid();
} // namespace columns

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
} // namespace fonts

PreferencesTab* g_get_tab_layout();
PreferencesTab* g_get_tab_artwork();
PreferencesTab* g_get_tab_display2();
PreferencesTab* g_get_tab_pview_artwork();
PreferencesTab* g_get_tab_sys();
PreferencesTab* g_get_tab_playlist_switcher();
PreferencesTab* g_get_tab_playlist_tabs();
PreferencesTab* g_get_tab_playlist_dd();
PreferencesTab* g_get_tab_main();
PreferencesTab* g_get_tab_status();
PreferencesTab* g_get_tab_global();

void refresh_appearance_prefs();
void colour_code_gen(HWND parent, UINT edit, bool markers, bool init);
bool colour_picker(HWND wnd, COLORREF& out, COLORREF custom);
void preview_to_console(const char* spec, bool extra);

extern cui::fonts::ConfigFontDescription cfg_editor_font;

class EditorFontNotify {
    HFONT g_edit_font{nullptr};
    HWND wnd{nullptr};
    void _set()
    {
        if (wnd) {
            g_edit_font = CreateFontIndirect(&cfg_editor_font->log_font);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_edit_font, MAKELPARAM(1, 0));
        }
    }
    void _release()
    {
        if (g_edit_font != nullptr) {
            if (wnd)
                SendMessage(wnd, WM_SETFONT, (WPARAM)0, MAKELPARAM(0, 0));
            DeleteObject(g_edit_font);
            g_edit_font = nullptr;
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
        wnd = nullptr;
        _release();
    }

    EditorFontNotify() = default;
    EditorFontNotify(const EditorFontNotify&) = delete;
    EditorFontNotify& operator=(const EditorFontNotify&) = delete;
    EditorFontNotify(EditorFontNotify&&) = delete;
    EditorFontNotify& operator=(EditorFontNotify&&) = delete;
    ~EditorFontNotify()
    {
        if (g_edit_font) {
            DeleteObject(g_edit_font);
            g_edit_font = nullptr;
        }
    }
};
void speedtest(ColumnListCRef columns, bool b_global);

extern EditorFontNotify g_editor_font_notify;
extern cfg_uint g_last_colour;
extern const GUID g_guid_columns_ui_preferences_page;

void on_global_colours_change();

cui::colours::colour_mode_t g_get_global_colour_mode(bool is_dark = cui::colours::is_dark_mode_active());
void g_set_global_colour_mode(cui::colours::colour_mode_t p_mode, bool is_dark = cui::colours::is_dark_mode_active());

namespace cui {
namespace prefs {

extern service_factory_single_t<PreferencesTabsHost> page_main;
extern service_factory_single_t<PreferencesTabsHost> page_playlist_view;
extern service_factory_single_t<PreferencesTabsHost> page_playlist_switcher;
extern service_factory_single_t<PreferencesTabsHost> page_filters;
} // namespace prefs
} // namespace cui
