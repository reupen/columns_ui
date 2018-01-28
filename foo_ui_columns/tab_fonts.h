#pragma once

#include "config_appearance.h"
#include "config.h"

class tab_appearance_fonts : public preferences_tab {
    HWND m_wnd{nullptr};
    HWND m_wnd_colours_mode{nullptr};
    HWND m_wnd_colours_element{nullptr};
    fonts_manager_data::entry_ptr_t m_element_ptr;
    cui::fonts::client::ptr m_element_api;
    fonts_client_list_t m_fonts_client_list;

public:
    void refresh_me(HWND wnd);

    static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    void update_mode_combobox();

    void get_font(LOGFONT& lf);

    void update_change();

    void update_font_desc();

    void on_font_changed();

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void apply();
    HWND create(HWND wnd) override;
    const char* get_name() override;
    bool get_help_url(pfc::string_base& p_out) override;
    bool is_active();

private:
    bool initialising{false};
};
