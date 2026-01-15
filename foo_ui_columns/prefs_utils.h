#pragma once

void populate_menu_combo(HWND wnd, unsigned ID, unsigned ID_DESC, const MenuItemIdentifier& p_item,
    const std::vector<MenuItemInfo>& p_cache, bool insert_none);
void on_menu_combo_change(HWND wnd, LPARAM lp, class ConfigMenuItem& cfg_menu_store,
    const std::vector<MenuItemInfo>& p_cache, unsigned ID_DESC);

namespace cui::prefs {

HFONT create_default_ui_font(unsigned point_size);
HFONT create_default_title_font();
void show_generic_title_formatting_tools_menu(HWND dialog_wnd, HWND button_wnd, GUID font_id);

} // namespace cui::prefs
