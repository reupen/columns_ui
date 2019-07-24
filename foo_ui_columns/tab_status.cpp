#include "stdafx.h"
#include "config.h"
#include "prefs_utils.h"
#include "main_window.h"
#include "status_bar.h"

static class TabStatus : public PreferencesTab {
public:
    bool m_initialised{};
    MenuItemCache* m_cache{};

    static void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_VOL, BM_SETCHECK, cfg_show_vol, 0);
        SendDlgItemMessage(wnd, IDC_SELTIME, BM_SETCHECK, cfg_show_seltime, 0);
        SendDlgItemMessage(wnd, IDC_SHOW_STATUS, BM_SETCHECK, cfg_status, 0);
        SendDlgItemMessage(wnd, IDC_SHOW_STATUSPANE, BM_SETCHECK, settings::show_status_pane, 0);
        SendDlgItemMessage(wnd, IDC_SHOW_LOCK, BM_SETCHECK, main_window::config_get_status_show_lock(), 0);

        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_status_bar_script.get());
    }

    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            m_cache = new MenuItemCache;

            populate_menu_combo(wnd, IDC_MENU_DBLCLK, IDC_MENU_DESC, cfg_statusdbl, *m_cache, false);

            refresh_me(wnd);
            m_initialised = true;
        }

        break;
        case WM_DESTROY: {
            delete m_cache;
            m_cache = nullptr;
            m_initialised = false;
        } break;
        case WM_COMMAND:
            switch (wp) {
            case (EN_CHANGE << 16) | IDC_STRING:
                main_window::config_status_bar_script.set(string_utf8_from_window((HWND)lp));
                break;
            case (CBN_SELCHANGE << 16) | IDC_MENU_DBLCLK: {
                on_menu_combo_change(wnd, lp, cfg_statusdbl, *m_cache, IDC_MENU_DESC);
            } break;
            case IDC_SHOW_LOCK: {
                main_window::config_set_status_show_lock(SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0);
            } break;
            case IDC_VOL: {
                cfg_show_vol = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                status_bar::set_part_sizes(status_bar::t_part_volume);
            } break;
            case IDC_SELTIME: {
                cfg_show_seltime = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                status_bar::set_part_sizes(status_bar::t_part_length | status_bar::t_part_volume);

            } break;
            case IDC_SHOW_STATUS: {
                cfg_status = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                on_show_status_change();
            } break;
            case IDC_SHOW_STATUSPANE: {
                settings::show_status_pane = SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0;
                on_show_status_pane_change();
            } break;
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
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1, IDC_TITLE2, IDC_TITLE3}};
} g_tab_status;

PreferencesTab* g_get_tab_status()
{
    return &g_tab_status;
}
