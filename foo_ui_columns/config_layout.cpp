#include "stdafx.h"
#include "config.h"
#include "tab_layout_misc.h"
#include "tab_layout.h"

namespace cui::prefs {
constexpr GUID layout_page_guid = {0x1d904b23, 0x9357, 0x4cf7, {0xbf, 0xdf, 0xd3, 0x9e, 0xa0, 0x30, 0x73, 0xb1}};
}

cui::prefs::LayoutMiscTab g_tab_layout_misc;
cui::prefs::LayoutTab g_tab_layout;

PreferencesTab* tabs_layout[] = {&g_tab_layout, &g_tab_layout_misc};

cfg_int cfg_child_layout(GUID{0xe5c3db3c, 0xccd8, 0x4c8e, {0x9f, 0x74, 0xdf, 0xcc, 0x57, 0x6a, 0x41, 0xea}}, 0);

service_factory_single_t<PreferencesTabsHost> page_layout("Layout", tabs_layout, std::size(tabs_layout),
    cui::prefs::layout_page_guid, g_guid_columns_ui_preferences_page, &cfg_child_layout);
