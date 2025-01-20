#pragma once

#include "dark_mode.h"
#include "dark_mode_active_ui.h"

namespace cui::dark {

struct DialogDarkModeConfig {
    std::unordered_set<int> button_ids;
    std::unordered_set<int> checkbox_ids;
    std::unordered_set<int> combo_box_ids;
    std::unordered_set<int> edit_ids;
    std::unordered_set<int> list_box_ids;
    std::unordered_set<int> spin_ids;
    std::unordered_set<int> tree_view_ids;
    int last_button_id{IDOK};
};

INT_PTR modal_dialog_box(UINT resource_id, DialogDarkModeConfig dark_mode_config, HWND parent_window,
    std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message, bool poke = true);

HWND modeless_dialog_box(UINT resource_id, DialogDarkModeConfig dark_mode_config, HWND parent_window,
    std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message);

void modeless_info_box(
    HWND wnd_parent, const char* title, const char* message, uih::InfoBoxType type, bool no_wrap = false);

INT_PTR modal_info_box(HWND wnd_parent, const char* title, const char* message, uih::InfoBoxType type,
    uih::InfoBoxModalType modal_type, bool no_wrap = false);

} // namespace cui::dark
