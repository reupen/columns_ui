#include "pch.h"
#include "config.h"
#include "system_tray.h"

namespace cui::prefs {

namespace {

class TabSystemTray : public PreferencesTab {
public:
    bool m_initialised{};

    static void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_BALLOON, BM_SETCHECK, cfg_balloon, 0);

        SendDlgItemMessage(wnd, IDC_SHOW_SYSTRAY, BM_SETCHECK, cfg_show_systray, 0);

        SendDlgItemMessage(wnd, IDC_MINIMISE_TO_SYSTRAY, BM_SETCHECK, cfg_minimise_to_tray, 0);
        SendDlgItemMessage(wnd, IDC_USE_CUSTOM_ICON, BM_SETCHECK, cfg_custom_icon, 0);
        SendDlgItemMessage(wnd, IDC_NOWPL, BM_SETCHECK, cfg_np, 0);

        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_system_tray_icon_script.get());
        EnableWindow(GetDlgItem(wnd, IDC_BROWSE_ICON), cfg_custom_icon);
    }

    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            uih::enhance_edit_control(wnd, IDC_STRING);
            refresh_me(wnd);
            m_initialised = true;
        }

        break;
        case WM_DESTROY: {
            m_initialised = false;
        } break;
        case WM_COMMAND:
            switch (wp) {
            case (EN_CHANGE << 16) | IDC_STRING:
                main_window::config_system_tray_icon_script.set(uGetWindowText((HWND)lp));
                break;

            case IDC_NOWPL: {
                cfg_np = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
            } break;
            case IDC_USE_CUSTOM_ICON: {
                cfg_custom_icon = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                EnableWindow(GetDlgItem(wnd, IDC_BROWSE_ICON), cfg_custom_icon);
                cui::systray::create_icon_handle();
                cui::systray::create_icon();
            } break;
            case IDC_BROWSE_ICON: {
                pfc::string8 path = cfg_tray_icon_path;
                if (uGetOpenFileName(wnd, "Icon Files (*.ico)|*.ico|All Files (*.*)|*.*", 0, "ico", "Choose Icon",
                        nullptr, path, FALSE)) {
                    cfg_tray_icon_path = path;
                    if (cfg_custom_icon) {
                        cui::systray::create_icon_handle();
                        cui::systray::create_icon();
                    }
                }
            } break;

            case IDC_MINIMISE_TO_SYSTRAY: {
                cfg_minimise_to_tray = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
            } break;
            case IDC_SHOW_SYSTRAY: {
                cfg_show_systray = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                //                EnableWindow(GetDlgItem(wnd, IDC_MINIMISE_TO_SYSTRAY), cfg_show_systray);
                cui::systray::on_show_icon_change();
            } break;
            case IDC_BALLOON: {
                cfg_balloon = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
            } break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_SYSTEM_TRAY,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "System tray"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:notification_area";
        return true;
    }
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

TabSystemTray g_tab_system_tray;

} // namespace

PreferencesTab* g_get_tab_system_tray()
{
    return &g_tab_system_tray;
}

} // namespace cui::prefs
