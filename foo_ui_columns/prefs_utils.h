#ifndef _COLOUMNS_PREFS_H_
#define _COLOUMNS_PREFS_H_

void populate_menu_combo(HWND wnd, unsigned ID, unsigned ID_DESC, const menu_item_identifier& p_item,
    menu_item_cache& p_cache, bool insert_none);
void on_menu_combo_change(
    HWND wnd, LPARAM lp, class cfg_menu_item& cfg_menu_store, menu_item_cache& p_cache, unsigned ID_DESC);

#endif