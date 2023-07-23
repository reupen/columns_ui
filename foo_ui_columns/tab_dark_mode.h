#pragma once
#pragma once

#include "config.h"

class TabDarkMode : public PreferencesTab {
public:
    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void apply() {}
    HWND create(HWND wnd) override;
    const char* get_name() override;
    bool get_help_url(pfc::string_base& p_out) override;
    bool is_active();
    void refresh();

private:
    bool m_is_updating{};
    HWND m_wnd{nullptr};
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

extern TabDarkMode g_tab_dark_mode;
