#pragma once

#include "dark_mode.h"
#include "dark_mode_active_ui.h"

namespace cui::dark {

class DialogDarkModeHelper {
public:
    void add_buttons(std::initializer_list<int> ids) { m_button_ids = ids; }
    void add_checkboxes(std::initializer_list<int> ids) { m_checkbox_ids = ids; }
    void add_combo_boxes(std::initializer_list<int> ids) { m_combo_box_ids = ids; }
    void set_window_themes();
    [[nodiscard]] std::optional<INT_PTR> handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    [[nodiscard]] std::optional<INT_PTR> handle_wm_notify(HWND wnd, LPNMHDR lpnm);
    void on_dark_mode_change();
    void set_window_theme(auto&& ids, const wchar_t* dark_class, bool is_dark);

    HWND m_wnd{};
    wil::unique_hbrush m_main_background_brush;
    wil::unique_htheme m_button_theme;
    std::unordered_set<int> m_button_ids;
    std::unordered_set<int> m_checkbox_ids;
    std::unordered_set<int> m_combo_box_ids;
    std::unique_ptr<EventToken> m_dark_mode_status_callback;
};

void DialogDarkModeHelper::set_window_theme(auto&& ids, const wchar_t* dark_class, bool is_dark)
{
    if (!m_wnd)
        return;

    for (const auto id : ids)
        SetWindowTheme(GetDlgItem(m_wnd, id), is_dark ? dark_class : nullptr, nullptr);
}

} // namespace cui::dark
