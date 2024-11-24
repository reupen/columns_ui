#pragma once
#include "font_utils.h"

namespace cui::utils {

class DirectWriteFontPicker {
public:
    std::optional<INT_PTR> handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void on_font_changed(std::function<void(const fonts::FontDescription&)> on_font_changed);
    void set_font_description(fonts::FontDescription font_description);
    void set_font_selection_allowed(bool value) const;

private:
    constexpr static auto font_dropdown_font_size = 9.5f;
    constexpr static auto custom_axis_values_label = L"Custom axis values";

    void handle_wm_init_dialog(HWND wnd);
    std::optional<INT_PTR> handle_wm_command(WPARAM wp, LPARAM lp);
    std::optional<INT_PTR> handle_wm_notify(LPNMHDR nmhdr);
    std::optional<INT_PTR> handle_wm_measure_item(LPMEASUREITEMSTRUCT mis);
    std::optional<INT_PTR> handle_wm_draw_item(LPDRAWITEMSTRUCT dis);

    const wchar_t* get_font_face_combobox_item_text(uint32_t index) const;
    wil::com_ptr_t<IDWriteFontFamily> get_icon_font_family() const;
    uih::direct_write::TextFormat& get_family_text_format(size_t index);
    uih::direct_write::TextFormat& get_face_text_format(size_t index);

    void handle_family_change(bool skip_face_change = false);
    void handle_face_change();
    void notify_font_changed() const;
    void store_font_face();
    void update_font_size_edit();
    void update_font_size_spin() const;

    bool m_is_updating_font_size_edit{};
    HWND m_wnd{};
    HWND m_configure_axes_button{};
    HWND m_font_family_combobox{};
    HWND m_font_face_combobox{};
    HWND m_font_size_edit{};
    HWND m_font_size_spin{};
    std::function<void(const fonts::FontDescription&)> m_on_font_changed;
    std::optional<fonts::FontDescription> m_font_description;
    uih::direct_write::Context::Ptr m_direct_write_context;
    std::vector<uih::direct_write::FontFamily> m_font_families;
    std::vector<uih::direct_write::Font> m_font_faces;
    std::vector<std::optional<uih::direct_write::TextFormat>> m_font_families_text_formats;
    std::vector<std::optional<uih::direct_write::TextFormat>> m_font_faces_text_formats;
    std::optional<uih::direct_write::TextFormat> m_custom_axis_values_text_format;
    std::optional<std::reference_wrapper<uih::direct_write::FontFamily>> m_font_family;
    std::optional<std::reference_wrapper<uih::direct_write::Font>> m_font_face;
};

} // namespace cui::utils
