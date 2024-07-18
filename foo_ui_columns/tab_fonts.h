#pragma once

#include "config_appearance.h"
#include "config.h"

class TabFonts : public PreferencesTab {
public:
    void update_mode_combobox() const;
    void restore_font_selection_state();
    void enable_or_disable_font_selection() const;

    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    HWND create(HWND wnd) override;
    const char* get_name() override;
    bool get_help_url(pfc::string_base& p_out) override;
    bool is_active() const;

private:
    std::optional<INT_PTR> handle_wm_drawitem(LPDRAWITEMSTRUCT dis);

    LOGFONT get_current_log_font() const;
    int get_current_font_size() const;
    void on_font_changed();

    void on_family_change();
    void on_face_change();
    void update_font_size_edit();
    void update_font_size_spin() const;
    void save_font_face() const;
    void save_size_edit() const;

    wil::com_ptr_t<IDWriteFontFamily> get_icon_font_family() const;
    uih::direct_write::TextFormat& get_family_text_format(size_t index);
    uih::direct_write::TextFormat& get_face_text_format(size_t index);

    constexpr static auto font_dropdown_font_size = 9.5f;

    bool m_is_updating_font_size_edit{};
    HWND m_wnd{};
    HWND m_element_combobox{};
    HWND m_mode_combobox{};
    HWND m_font_family_combobox{};
    HWND m_font_face_combobox{};
    HWND m_font_size_edit{};
    HWND m_font_size_spin{};

    FontManagerData::entry_ptr_t m_element_ptr;
    cui::fonts::client::ptr m_element_api;
    FontsClientList m_fonts_client_list;

    uih::direct_write::Context::Ptr m_direct_write_context;
    std::vector<uih::direct_write::FontFamily> m_font_families;
    std::vector<uih::direct_write::Font> m_font_faces;
    std::vector<std::optional<uih::direct_write::TextFormat>> m_font_families_text_formats;
    std::vector<std::optional<uih::direct_write::TextFormat>> m_font_faces_text_formats;
    std::optional<std::reference_wrapper<uih::direct_write::FontFamily>> m_font_family;
    std::optional<uih::direct_write::Font> m_font_face;

    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
};
