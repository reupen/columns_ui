#pragma once

#include "config_appearance.h"
#include "config.h"

namespace cui::prefs {

class TabColours : public PreferencesTab {
    HWND m_wnd{nullptr};
    HWND m_wnd_colour_scheme{nullptr};
    HWND m_wnd_colours_element{nullptr};
    uih::FillWindow g_fill_text;
    uih::FillWindow g_fill_background;
    uih::FillWindow g_fill_selection_text;
    uih::FillWindow g_fill_selection_background;
    uih::FillWindow g_fill_selection_text_inactive;
    uih::FillWindow g_fill_selection_background_inactive;
    uih::FillWindow g_fill_active_item_frame;
    GUID m_element_guid{};
    colours::Entry::Ptr m_element_ptr;
    colours::client::ptr m_element_api;
    ColoursClientList m_colours_client_list;

public:
    void handle_external_configuration_change();

    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void apply();
    HWND create(HWND wnd) override;
    const char* get_name() override;
    bool get_help_url(pfc::string_base& p_out) override;
    bool is_active();

private:
    bool get_change_colour_enabled(colours::colour_identifier_t p_identifier);
    bool get_colour_patch_enabled(colours::colour_identifier_t p_identifier);

    void update_fills();
    void update_buttons();
    void update_scheme_combobox();
    void update_title() const;

    void on_colour_changed();

    prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

extern TabColours g_tab_appearance;

} // namespace cui::prefs
