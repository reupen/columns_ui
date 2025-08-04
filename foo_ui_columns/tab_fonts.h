#pragma once

#include "config_appearance.h"
#include "config.h"
#include "font_picker.h"

namespace cui::prefs {

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
    FontManagerData::entry_ptr_t get_current_resolved_entry() const;
    void on_font_changed();

    HWND m_wnd{};
    HWND m_element_combobox{};
    HWND m_mode_combobox{};
    FontManagerData::entry_ptr_t m_element_ptr;
    fonts::client::ptr m_element_api;
    FontsClientList m_fonts_client_list;
    std::optional<utils::DirectWriteFontPicker> m_direct_write_font_picker;
    PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

} // namespace cui::prefs
