#include "pch.h"
#include "config.h"
#include "prefs_utils.h"
#include "main_window.h"
#include "status_bar.h"

static class TabStatusBar : public PreferencesTab {
public:
    std::vector<MenuItemInfo> m_cache{};

    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_cache = cui::helpers::get_main_menu_items();

            populate_menu_combo(wnd, IDC_MENU_DBLCLK, IDC_MENU_DESC, cfg_statusdbl, m_cache, false);

            SendDlgItemMessage(wnd, IDC_VOL, BM_SETCHECK, cfg_show_vol, 0);
            SendDlgItemMessage(wnd, IDC_SELTIME, BM_SETCHECK, cfg_show_seltime, 0);
            SendDlgItemMessage(wnd, IDC_SELCOUNT, BM_SETCHECK, cfg_show_selcount, 0);
            SendDlgItemMessage(wnd, IDC_SHOW_STATUS, BM_SETCHECK, cfg_status, 0);
            SendDlgItemMessage(wnd, IDC_SHOW_LOCK, BM_SETCHECK, main_window::config_get_status_show_lock(), 0);

            uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_status_bar_script.get());
            break;
        case WM_DESTROY:
            m_cache.clear();
            break;
        case WM_COMMAND:
            switch (wp) {
            case EN_CHANGE << 16 | IDC_STRING:
                main_window::config_status_bar_script.set(uGetWindowText((HWND)lp));
                break;
            case CBN_SELCHANGE << 16 | IDC_MENU_DBLCLK:
                on_menu_combo_change(wnd, lp, cfg_statusdbl, m_cache, IDC_MENU_DESC);
                break;
            case IDC_SHOW_LOCK:
                main_window::config_set_status_show_lock(SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0);
                break;
            case IDC_VOL:
                cfg_show_vol = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                set_part_sizes(cui::status_bar::t_part_volume);
                break;
            case IDC_SELCOUNT:
                cfg_show_selcount = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                cui::status_bar::set_part_sizes(cui::status_bar::t_part_count | cui::status_bar::t_part_volume);
                break;
            case IDC_SELTIME:
                cfg_show_seltime = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                cui::status_bar::set_part_sizes(
                    cui::status_bar::t_part_length | cui::status_bar::t_part_count | cui::status_bar::t_part_volume);
                break;
            case IDC_SHOW_STATUS:
                cfg_status = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                on_show_status_change();
                break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_STATUS_BAR,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Status bar"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:status_bar";
        return true;
    }
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
} g_tab_status_bar;

PreferencesTab* g_get_tab_status_bar()
{
    return &g_tab_status_bar;
}
