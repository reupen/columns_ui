#pragma once

#include "config_appearance.h"
#include "config.h"

class tab_appearance : public preferences_tab
{
    HWND m_wnd;
    HWND m_wnd_colours_mode;
    HWND m_wnd_colours_element;
    FillWindow g_fill_text;
    FillWindow g_fill_background;
    FillWindow g_fill_selection_text;
    FillWindow g_fill_selection_background;
    FillWindow g_fill_selection_text_inactive;
    FillWindow g_fill_selection_background_inactive;
    FillWindow g_fill_active_item_frame;
    GUID m_element_guid;
    colours_manager_data::entry_ptr_t m_element_ptr;
    cui::colours::client::ptr m_element_api;
    colours_client_list_t m_colours_client_list;
public:
    tab_appearance();;

    void refresh_me(HWND wnd);


    static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    void update_fills();
    bool get_change_colour_enabled(cui::colours::colour_identifier_t p_identifier);
    bool get_colour_patch_enabled(cui::colours::colour_identifier_t p_identifier);
    void update_buttons();
    void update_mode_combobox();

    void on_colour_changed();

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void apply();
    HWND create(HWND wnd) override;
    const char * get_name() override;
    bool get_help_url(pfc::string_base & p_out) override;
    bool is_active();

private:
    bool initialising;
};
