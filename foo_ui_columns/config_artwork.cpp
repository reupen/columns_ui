#include "pch.h"
#include "legacy_artwork_config.h"
#include "artwork.h"
#include "config.h"
#include "win32.h"

namespace cui::artwork_panel {

extern cfg_uint cfg_edge_style;

} // namespace cui::artwork_panel

namespace cui::prefs {

namespace {

pfc::string8 format_legacy_artwork_sources()
{
    pfc::string8 text;
    for (auto&& [scripts, type] : artwork::legacy::legacy_sources) {
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

class TabArtwork : public PreferencesTab {
public:
    void refresh_me(HWND wnd)
    {
        m_initialising = true;

        const HWND edge_style_wnd = GetDlgItem(wnd, IDC_EDGESTYLE);
        ComboBox_AddString(edge_style_wnd, L"None");
        ComboBox_AddString(edge_style_wnd, L"Sunken");
        ComboBox_AddString(edge_style_wnd, L"Grey");
        ComboBox_SetCurSel(edge_style_wnd, cui::artwork_panel::cfg_edge_style);

        const HWND use_advanced_colour_wnd = GetDlgItem(wnd, IDC_USE_ADVANCED_COLOUR);

        if (!win32::check_windows_10_build(17'763)) {
            EnableWindow(use_advanced_colour_wnd, false);
            uSetWindowText(use_advanced_colour_wnd, "Use Advanced Colour (requires Window 10 version 1809 or newer)");
        } else if (artwork_panel::colour_management_mode
            == WI_EnumValue(cui::artwork_panel::ColourManagementMode::Advanced)) {
            Button_SetCheck(use_advanced_colour_wnd, BST_CHECKED);
        }

        const HWND click_action_wnd = GetDlgItem(wnd, IDC_CLICK_ACTION);

        if (fb2k::imageViewer::ptr api; fb2k::imageViewer::tryGet(api)) {
            uih::combo_box_add_string_data(click_action_wnd, L"Open foobar2000 picture viewer",
                WI_EnumValue(cui::artwork_panel::ClickAction::open_image_viewer));
        }

        uih::combo_box_add_string_data(click_action_wnd, L"Show in File Explorer",
            WI_EnumValue(cui::artwork_panel::ClickAction::show_in_file_explorer));

        uih::combo_box_add_string_data(click_action_wnd, L"Show next available artwork type",
            WI_EnumValue(cui::artwork_panel::ClickAction::show_next_artwork_type));

        const auto click_action_index = uih::combo_box_find_item_by_data(click_action_wnd, artwork_panel::click_action);

        if (click_action_index != CB_ERR) {
            ComboBox_SetCurSel(click_action_wnd, click_action_index);
        }

        m_initialising = false;
    }

    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            refresh_me(wnd);
            if (artwork::legacy::any_legacy_sources()) {
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
            case IDC_USE_ADVANCED_COLOUR: {
                const auto is_checked = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                artwork_panel::colour_management_mode
                    = WI_EnumValue(is_checked ? cui::artwork_panel::ColourManagementMode::Advanced
                                              : cui::artwork_panel::ColourManagementMode::Legacy);
                artwork_panel::ArtworkPanel::s_on_use_advanced_colour_change();
                break;
            }
            case IDC_EDGESTYLE | (CBN_SELCHANGE << 16):
                artwork_panel::cfg_edge_style = ComboBox_GetCurSel((HWND)lp);
                artwork_panel::ArtworkPanel::g_on_edge_style_change();
                break;
            case IDC_CLICK_ACTION | (CBN_SELCHANGE << 16): {
                auto index = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));

                if (index == CB_ERR)
                    break;

                artwork_panel::click_action
                    = gsl::narrow<int32_t>(ComboBox_GetItemData(reinterpret_cast<HWND>(lp), index));
                break;
            }
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
    PreferencesTabHelper m_helper{{IDC_TITLE1, IDC_TITLE2}};
};

TabArtwork g_tab_artwork;

} // namespace

PreferencesTab* g_get_tab_artwork()
{
    return &g_tab_artwork;
}

} // namespace cui::prefs
