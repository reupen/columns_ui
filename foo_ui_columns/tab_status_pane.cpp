#include "pch.h"
#include "config.h"
#include "prefs_utils.h"
#include "main_window.h"
#include "status_pane.h"

namespace cui::prefs {

namespace {

class TabStatusPane : public PreferencesTab {
public:
    std::vector<MenuItemInfo> m_cache{};

    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_cache = helpers::get_main_menu_items();

            populate_menu_combo(wnd, IDC_MENU_DBLCLK, IDC_MENU_DESC, status_pane::double_click_action, m_cache, false);

            SendDlgItemMessage(wnd, IDC_SHOW_STATUSPANE, BM_SETCHECK, settings::show_status_pane, 0);

            uih::enhance_edit_control(wnd, IDC_STRING);
            uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, status_pane::status_pane_script);
            break;
        case WM_DESTROY:
            m_cache.clear();
            break;
        case WM_COMMAND:
            switch (wp) {
            case EN_CHANGE << 16 | IDC_STRING:
                status_pane::status_pane_script.set(uGetWindowText(reinterpret_cast<HWND>(lp)));
                break;
            case CBN_SELCHANGE << 16 | IDC_MENU_DBLCLK:
                on_menu_combo_change(wnd, lp, status_pane::double_click_action, m_cache, IDC_MENU_DESC);
                break;
            case IDC_SHOW_STATUSPANE:
                settings::show_status_pane = SendMessage(reinterpret_cast<HWND>(lp), BM_GETCHECK, 0, 0) != 0;
                on_show_status_pane_change();
                break;
            case IDC_TOOLS:
                show_generic_title_formatting_tools_menu(wnd, reinterpret_cast<HWND>(lp), status_pane::font_id);
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
    PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

TabStatusPane g_tab_status_pane;

} // namespace

PreferencesTab* g_get_tab_status_pane()
{
    return &g_tab_status_pane;
}

} // namespace cui::prefs
