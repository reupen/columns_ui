#include "pch.h"
#include "legacy_artwork_config.h"
#include "artwork.h"
#include "config.h"

namespace cui::artwork_panel {
extern cfg_uint cfg_edge_style;
} // namespace cui::artwork_panel

pfc::string8 format_legacy_artwork_sources()
{
    pfc::string8 text;
    for (auto&& [scripts, type] : cui::artwork::legacy::legacy_sources) {
        if (scripts->get_count() == 0)
            continue;

        text << type << "\r\n---\r\n";

        for (auto&& script : *scripts) {
            text << script << ".*\r\n";
        }

        text << "\r\n";
    }
    return text;
}

static class TabArtwork : public PreferencesTab {
public:
    void refresh_me(HWND wnd)
    {
        m_initialising = true;
        const HWND wnd_combo = GetDlgItem(wnd, IDC_EDGESTYLE);
        ComboBox_AddString(wnd_combo, L"None");
        ComboBox_AddString(wnd_combo, L"Sunken");
        ComboBox_AddString(wnd_combo, L"Grey");
        ComboBox_SetCurSel(wnd_combo, cui::artwork_panel::cfg_edge_style);
        m_initialising = false;
    }

    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            refresh_me(wnd);
            if (cui::artwork::legacy::any_legacy_sources()) {
                ShowWindow(GetDlgItem(wnd, IDC_OPEN_DISPLAY_PREFERENCES), SW_HIDE);
            } else {
                ShowWindow(GetDlgItem(wnd, IDC_VIEW_OLD_ARTWORK_SOURCES_TEXT), SW_HIDE);
                ShowWindow(GetDlgItem(wnd, IDC_VIEW_OLD_ARTWORK_SOURCES), SW_HIDE);
                ShowWindow(GetDlgItem(wnd, IDC_PREVIOUS_OPEN_DISPLAY_PREFERENCES), SW_HIDE);
            }
        } break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_PREVIOUS_OPEN_DISPLAY_PREFERENCES:
            case IDC_OPEN_DISPLAY_PREFERENCES:
                ui_control::get()->show_preferences(preferences_page::guid_display);
                break;
            case IDC_VIEW_OLD_ARTWORK_SOURCES:
                popup_message_v2::g_show(
                    GetAncestor(wnd, GA_ROOT), format_legacy_artwork_sources(), "Previous Columns UI Artwork Sources");
                break;
            case IDC_EDGESTYLE | (CBN_SELCHANGE << 16):
                cui::artwork_panel::cfg_edge_style = ComboBox_GetCurSel((HWND)lp);
                cui::artwork_panel::ArtworkPanel::g_on_edge_style_change();
                break;
            }
        }
        return 0;
    }
    void apply() {}
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_ARTWORK,
            [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Artwork"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:artwork";
        return true;
    }

private:
    bool m_initialising{false};
    cui::prefs::PreferencesTabHelper m_helper{IDC_TITLE1, IDC_TITLE2};
} g_tab_artwork;

PreferencesTab* g_get_tab_artwork()
{
    return &g_tab_artwork;
}
