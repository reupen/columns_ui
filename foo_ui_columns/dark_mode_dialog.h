#pragma once

#include "dark_mode.h"
#include "dark_mode_active_ui.h"

namespace cui::dark {

struct DialogDarkModeConfig {
    std::unordered_set<int> button_ids;
    std::unordered_set<int> checkbox_ids;
    std::unordered_set<int> combo_box_ids;
    std::unordered_set<int> edit_ids;
};

INT_PTR modal_dialog_box(UINT resource_id, DialogDarkModeConfig dark_mode_config, HWND parent_window,
    std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message);

HWND modeless_dialog_box(UINT resource_id, DialogDarkModeConfig dark_mode_config, HWND parent_window,
    std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message);

} // namespace cui::dark
