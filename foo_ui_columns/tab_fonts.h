#pragma once

#include "config_appearance.h"
#include "config.h"

class TabFonts : public PreferencesTab {
    HWND m_wnd{nullptr};
    HWND m_wnd_colours_mode{nullptr};
    HWND m_wnd_colours_element{nullptr};
    FontManagerData::entry_ptr_t m_element_ptr;
    cui::fonts::client::ptr m_element_api;
    FontsClientList m_fonts_client_list;

public:
    void refresh_me(HWND wnd);

    void update_mode_combobox();

    void get_font(LOGFONT& lf);

    void update_change();

    void update_font_desc();

    void on_font_changed();

    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void apply();
    HWND create(HWND wnd) override;
    const char* get_name() override;
    bool get_help_url(pfc::string_base& p_out) override;
    bool is_active();

private:
    bool initialising{false};
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
};
