#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"

#include "config_host.h"

#include "config.h"
#include "config_columns_v2.h"

cfg_struct_t<LOGFONT> cfg_editor_font(
    GUID{0xd429d322, 0xd236, 0x7356, {0x33, 0x25, 0x4b, 0x67, 0xc5, 0xd4, 0x50, 0x3e}}, get_menu_font());
cfg_int cfg_child(GUID{0xf20b83d0, 0x5890, 0xba6f, {0xe8, 0x62, 0x69, 0x30, 0xe2, 0x6b, 0xc8, 0x1c}}, 0);
cfg_int cfg_child_panels(GUID{0x1a8d8760, 0x4f60, 0x4800, {0x93, 0x81, 0x32, 0x32, 0x66, 0xa0, 0x6c, 0xff}}, 0);
cfg_int cfg_child_playlist(GUID{0xbc6c99d4, 0x51c1, 0xf76e, {0x10, 0x9c, 0x62, 0x92, 0x92, 0xbd, 0xbd, 0xb2}}, 0);

pvt::preferences_tab_impl g_tab_grouping;

static preferences_tab* g_tabs[] = {
    g_get_tab_main(),
    g_get_tab_status(),
    g_get_tab_sys(),
    g_get_tab_artwork(),
};

static preferences_tab* g_tabs_panels[] = {
    g_get_tab_playlist(),
    //&g_tab_playlist_colours,
    g_get_tab_playlist_dd(),
};

static preferences_tab* g_tabs_playlist_view[] = {
    g_get_tab_display2(),
    &g_tab_grouping,
    &tab_columns_v3::get_instance(),
    g_get_tab_global(),
};

// {DF6B9443-DCC5-4647-8F8C-D685BF25BD09}
const GUID g_guid_columns_ui_preferences_page
    = {0xdf6b9443, 0xdcc5, 0x4647, {0x8f, 0x8c, 0xd6, 0x85, 0xbf, 0x25, 0xbd, 0x9}};

void g_show_artwork_settings()
{
    cfg_child = 5;
    static_api_ptr_t<ui_control>()->show_preferences(g_guid_columns_ui_preferences_page);
}

// {779F2FA6-3B76-4829-9E02-2E579CA510BF}
const GUID guid_playlist_switcher_page = {0x779f2fa6, 0x3b76, 0x4829, {0x9e, 0x2, 0x2e, 0x57, 0x9c, 0xa5, 0x10, 0xbf}};

// {B8CA5FC9-7463-48e8-9879-0D9517F3E7A9}
const GUID guid_playlist_view_page = {0xb8ca5fc9, 0x7463, 0x48e8, {0x98, 0x79, 0xd, 0x95, 0x17, 0xf3, 0xe7, 0xa9}};

namespace columns {
const GUID& config_get_playlist_view_guid()
{
    return guid_playlist_view_page;
}

const GUID& config_get_main_guid()
{
    return g_guid_columns_ui_preferences_page;
}
}; // namespace columns

namespace cui {
namespace prefs {
service_factory_single_t<config_host_generic> page_main("Columns UI", g_tabs, tabsize(g_tabs),
    g_guid_columns_ui_preferences_page, preferences_page::guid_display, &cfg_child);
service_factory_single_t<config_host_generic> page_playlist_view("Playlist view", g_tabs_playlist_view,
    tabsize(g_tabs_playlist_view), guid_playlist_view_page, g_guid_columns_ui_preferences_page, &cfg_child_playlist);
service_factory_single_t<config_host_generic> page_playlist_switcher("Playlist switcher", g_tabs_panels,
    tabsize(g_tabs_panels), guid_playlist_switcher_page, g_guid_columns_ui_preferences_page, &cfg_child_panels);
} // namespace preferences
} // namespace cui