#include "pch.h"
#include "config.h"
#include "prefs_utils.h"
#include "main_window.h"
#include "status_pane.h"

static class TabStatusPane : public PreferencesTab {
public:
    std::vector<MenuItemInfo> m_cache{};

    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_cache = cui::helpers::get_main_menu_items();

            populate_menu_combo(
                wnd, IDC_MENU_DBLCLK, IDC_MENU_DESC, cui::status_pane::double_click_action, m_cache, false);

            SendDlgItemMessage(wnd, IDC_SHOW_STATUSPANE, BM_SETCHECK, settings::show_status_pane, 0);

            uih::enhance_edit_control(wnd, IDC_STRING);
            uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, cui::status_pane::status_pane_script);
            break;
        case WM_DESTROY:
            m_cache.clear();
            break;
        case WM_COMMAND:
            switch (wp) {
            case EN_CHANGE << 16 | IDC_STRING:
                cui::status_pane::status_pane_script.set(uGetWindowText(reinterpret_cast<HWND>(lp)));
                break;
            case CBN_SELCHANGE << 16 | IDC_MENU_DBLCLK:
                on_menu_combo_change(wnd, lp, cui::status_pane::double_click_action, m_cache, IDC_MENU_DESC);
                break;
            case IDC_SHOW_STATUSPANE:
                settings::show_status_pane = SendMessage(reinterpret_cast<HWND>(lp), BM_GETCHECK, 0, 0) != 0;
                on_show_status_pane_change();
                break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_STATUS_PANE,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Status pane"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:status_bar";
        return true;
    }
    cui::prefs::PreferencesTabHelper m_helper{IDC_TITLE1};
} g_tab_status_pane;

PreferencesTab* g_get_tab_status_pane()
{
    return &g_tab_status_pane;
}
