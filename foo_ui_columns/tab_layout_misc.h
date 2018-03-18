#pragma once
#include "config_host.h"

namespace cui::prefs {

class LayoutMiscTab : public preferences_tab {
public:
    HWND create(HWND wnd) override;
    const char* get_name() override;
    bool get_help_url(pfc::string_base& p_out) override;

private:
    BOOL on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    bool m_initialised{};
    PreferencesTabHelper m_helper{{IDC_TITLE1, IDC_TITLE2}};
};

} // namespace cui::prefs
