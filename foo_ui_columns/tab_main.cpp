#include "stdafx.h"
#include "fcl.h"
#include "config.h"
#include "rebar.h"
#include "main_window.h"

extern HWND g_rebar;

static class TabMain : public PreferencesTab {
public:
    bool m_initialised{};

    static void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_TOOLBARS, BM_SETCHECK, cfg_toolbars, 0);
        // SendDlgItemMessage(wnd,IDC_KEYB,BM_SETCHECK,config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus,
        // true),0);
        SendDlgItemMessage(wnd, IDC_USE_TRANSPARENCY, BM_SETCHECK, main_window::config_get_transparency_enabled(), 0);
        SendDlgItemMessage(wnd, IDC_TRANSPARENCY_SPIN, UDM_SETPOS32, 0, main_window::config_get_transparency_level());

        if (!cui::main_window.get_wnd())
            EnableWindow(GetDlgItem(wnd, IDC_QUICKSETUP), FALSE);

        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_main_window_title_script.get());
    }

    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            SendDlgItemMessage(wnd, IDC_TRANSPARENCY_SPIN, UDM_SETRANGE32, 0, 255);
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
                main_window::config_main_window_title_script.set(string_utf8_from_window((HWND)lp));
                break;

            case IDC_QUICKSETUP:
                SendMessage(cui::main_window.get_wnd(), MSG_RUN_INITIAL_SETUP, NULL, NULL);
                break;
            case IDC_FCL_EXPORT:
                g_export_layout(wnd);
                break;
            case IDC_FCL_IMPORT:
                g_import_layout(wnd);
                break;
            case (EN_CHANGE << 16) | IDC_TRANSPARENCY_LEVEL: {
                if (m_initialised) {
                    BOOL result;
                    unsigned new_val = GetDlgItemInt(wnd, IDC_TRANSPARENCY_LEVEL, &result, FALSE);
                    if (result) {
                        main_window::config_set_transparency_level((unsigned char)new_val);
                    }
                }

            } break;
            case IDC_USE_TRANSPARENCY:
                main_window::config_set_transparency_enabled(SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0);
                break;
            case IDC_TOOLBARS:
                cfg_toolbars = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                on_show_toolbars_change();
                break;
            case IDC_RESET_TOOLBARS: {
                if (win32_helpers::message_box(wnd,
                        _T("Warning! This will reset the toolbars to the default state. Continue?"),
                        _T("Reset toolbars?"), MB_YESNO)
                    == IDYES) {
                    extern ConfigRebar g_cfg_rebar;

                    if (cui::main_window.get_wnd())
                        destroy_rebar();
                    g_cfg_rebar.reset();
                    if (cui::main_window.get_wnd()) {
                        create_rebar();
                        if (g_rebar) {
                            ShowWindow(g_rebar, SW_SHOWNORMAL);
                            UpdateWindow(g_rebar);
                        }
                        cui::main_window.resize_child_windows();
                    }
                }
            } break; /*
                     case IDC_KEYB:
                     {
                     config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus,
                     true) = SendMessage((HWND)lp,BM_GETCHECK,0,0);
                     }
                     break;*/
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(
            wnd, IDD_PREFS_MAIN, [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Main"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:main";
        return true;
    }
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1, IDC_TITLE2}};
} g_tab_main;

PreferencesTab* g_get_tab_main()
{
    return &g_tab_main;
}
