#include "stdafx.h"
#include "config.h"
#include "main_window.h"

static class tab_sys : public preferences_tab {
public:
    static bool initialised;

    //    static ptr_list_autofree_t<char> status_items;

    static void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_BALLOON, BM_SETCHECK, cfg_balloon, 0);

        SendDlgItemMessage(wnd, IDC_SHOW_SYSTRAY, BM_SETCHECK, cfg_show_systray, 0);
        //        EnableWindow(GetDlgItem(wnd, IDC_MINIMISE_TO_SYSTRAY), cfg_show_systray);

        SendDlgItemMessage(wnd, IDC_MINIMISE_TO_SYSTRAY, BM_SETCHECK, cfg_minimise_to_tray, 0);
        SendDlgItemMessage(wnd, IDC_USE_CUSTOM_ICON, BM_SETCHECK, cfg_custom_icon, 0);
        SendDlgItemMessage(wnd, IDC_NOWPL, BM_SETCHECK, cfg_np, 0);

        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_notification_icon_script.get());
    }

    static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            refresh_me(wnd);
            initialised = true;
        }

        break;
        case WM_DESTROY: {
            initialised = false;
        } break;
        case WM_COMMAND:
            switch (wp) {
            case (EN_CHANGE << 16) | IDC_STRING:
                main_window::config_notification_icon_script.set(string_utf8_from_window((HWND)lp));
                break;

            case IDC_NOWPL: {
                cfg_np = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
            } break;
            case IDC_USE_CUSTOM_ICON: {
                cfg_custom_icon = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                create_icon_handle();
                create_systray_icon();
            } break;
            case IDC_BROWSE_ICON: {
                pfc::string8 path = cfg_tray_icon_path;
                if (uGetOpenFileName(wnd, "Icon Files (*.ico)|*.ico|All Files (*.*)|*.*", 0, "ico", "Choose Icon",
                        nullptr, path, FALSE)) {
                    cfg_tray_icon_path = path;
                    if (cfg_custom_icon) {
                        create_icon_handle();
                        create_systray_icon();
                    }
                }
            } break;

            case IDC_MINIMISE_TO_SYSTRAY: {
                cfg_minimise_to_tray = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
            } break;
            case IDC_SHOW_SYSTRAY: {
                cfg_show_systray = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                //                EnableWindow(GetDlgItem(wnd, IDC_MINIMISE_TO_SYSTRAY), cfg_show_systray);

                if (g_main_window) {
                    auto is_iconic = IsIconic(g_main_window) != 0;
                    if (cfg_show_systray && !g_icon_created) {
                        create_systray_icon();
                    } else if (!cfg_show_systray && g_icon_created
                        && (!is_iconic
                               || (!cfg_minimise_to_tray
                                      && !g_advbool_close_to_tray.get_static_instance().get_state()))) {
                        destroy_systray_icon();
                        if (is_iconic)
                            standard_commands::main_activate();
                    }
                    if (g_status)
                        update_systray();
                }
            } break;
            case IDC_BALLOON: {
                cfg_balloon = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
            } break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override { return uCreateDialog(IDD_SYS, wnd, ConfigProc); }
    const char* get_name() override { return "Notification area"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:notification_area";
        return true;
    }
} g_tab_sys;

bool tab_sys::initialised = false;

preferences_tab* g_get_tab_sys()
{
    return &g_tab_sys;
}
