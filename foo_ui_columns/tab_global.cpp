#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"
#include "config.h"

static cfg_int g_cur_tab2(GUID{0x5fb6e011, 0x1ead, 0x49fe, {0x45, 0x32, 0x1c, 0x8a, 0x61, 0x01, 0x91, 0x2b}}, 0);

class tab_global : public preferences_tab {
public:
    static void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_GLOBAL, BM_SETCHECK, cfg_global, 0);
        SendDlgItemMessage(wnd, IDC_GLOBALSORT, BM_SETCHECK, cfg_global_sort, 0);
        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, (g_cur_tab2 == 0 ? cfg_globalstring : cfg_colour));
        SendDlgItemMessage(wnd, IDC_DATE, BM_SETCHECK, cfg_playlist_date, 0);
    }

    static void save_string(HWND wnd)
    {
        int id = g_cur_tab2;
        if (id >= 0 && id < 2) {
            if (id == 0)
                cfg_globalstring = string_utf8_from_window(wnd, IDC_STRING);
            else if (id == 1)
                cfg_colour = string_utf8_from_window(wnd, IDC_STRING);
        }
    }

    static LRESULT WINAPI s_on_edit_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        auto p_this = reinterpret_cast<tab_global*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
        return p_this ? p_this->on_edit_hooked_message(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
    }

    LRESULT on_edit_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_CHAR:
            if (!(HIWORD(lp) & KF_REPEAT) && (wp == 1) && (GetKeyState(VK_CONTROL) & KF_UP)) {
                SendMessage(wnd, EM_SETSEL, 0, -1);
                return 0;
            }
            break;
        }
        return CallWindowProc(m_edit_proc, wnd, msg, wp, lp);
    }

    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            uTCITEM tabs;
            memset(&tabs, 0, sizeof(tabs));

            HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

            tabs.mask = TCIF_TEXT;
            tabs.pszText = const_cast<char*>("Variables");
            uTabCtrl_InsertItem(wnd_tab, 0, &tabs);
            tabs.pszText = const_cast<char*>("Style");
            uTabCtrl_InsertItem(wnd_tab, 1, &tabs);

            TabCtrl_SetCurSel(wnd_tab, g_cur_tab2);

            colour_code_gen(wnd, IDC_COLOUR, false, true);

            SendDlgItemMessage(wnd, IDC_STRING, EM_LIMITTEXT, 0, 0);

            refresh_me(wnd);

            const auto wnd_edit = GetDlgItem(wnd, IDC_STRING);
            SetWindowLongPtr(wnd_edit, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
            m_edit_proc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtr(wnd_edit, GWLP_WNDPROC, reinterpret_cast<LPARAM>(s_on_edit_hooked_message)));

            g_editor_font_notify.set(GetDlgItem(wnd, IDC_STRING));
        }

        break;

        case WM_NOTIFY:
            switch (((LPNMHDR)lp)->idFrom) {
            case IDC_TAB1:
                switch (((LPNMHDR)lp)->code) {
                case TCN_SELCHANGE: {
                    save_string(wnd);
                    int id = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
                    g_cur_tab2 = id;
                    uSendDlgItemMessageText(
                        wnd, IDC_STRING, WM_SETTEXT, 0, (g_cur_tab2 == 0 ? cfg_globalstring : cfg_colour));
                } break;
                }
                break;
            }
            break;

        case WM_DESTROY: {
            g_editor_font_notify.release();
            save_string(wnd);
            pvt::ng_playlist_view_t::g_update_all_items();
        } break;

        case WM_COMMAND:
            switch (wp) {
            case IDC_GLOBAL:
                cfg_global = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                break;
            case IDC_DATE:
                cfg_playlist_date = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                pvt::ng_playlist_view_t::g_on_use_date_info_change();
                break;
            case IDC_TFHELP: {
                RECT rc;
                GetWindowRect(GetDlgItem(wnd, IDC_TFHELP), &rc);
                //        MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)(&rc), 2);
                HMENU menu = CreatePopupMenu();

                enum { IDM_TFHELP = 1, IDM_GHELP = 2, IDM_SPEEDTEST, IDM_PREVIEW, IDM_EDITORFONT, IDM_RESETSTYLE };

                uAppendMenu(menu, (MF_STRING), IDM_TFHELP, "Titleformatting &help");
                uAppendMenu(menu, (MF_STRING), IDM_GHELP, "&Global help");
                uAppendMenu(menu, (MF_SEPARATOR), 0, "");
                uAppendMenu(menu, (MF_STRING), IDM_SPEEDTEST, "&Speed test");
                uAppendMenu(menu, (MF_STRING), IDM_PREVIEW, "&Preview to console");
                uAppendMenu(menu, (MF_SEPARATOR), 0, "");
                uAppendMenu(menu, (MF_STRING), IDM_EDITORFONT, "Change editor &font");
                uAppendMenu(menu, (MF_SEPARATOR), 0, "");
                uAppendMenu(menu, (MF_STRING), IDM_RESETSTYLE, "&Reset style string");

                int cmd = TrackPopupMenu(
                    menu, TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rc.left, rc.bottom, 0, wnd, nullptr);
                DestroyMenu(menu);
                if (cmd == IDM_TFHELP) {
                    standard_commands::main_titleformat_help();
                } else if (cmd == IDM_GHELP) {
                    uMessageBox(wnd,
                        COLOUR_HELP
                        "\n\nNew global format: $set_global(var, val), retreive values using $get_global(var)",
                        "Global help", 0);
                } else if (cmd == IDM_SPEEDTEST) {
                    speedtest(g_columns, cfg_global != 0, cfg_playlist_date != 0);
                } else if (cmd == IDM_PREVIEW) {
                    preview_to_console(string_utf8_from_window(wnd, IDC_STRING), g_cur_tab2 != 0 && cfg_global);
                } else if (cmd == IDM_EDITORFONT) {
                    if (font_picker(wnd, cfg_editor_font))
                        g_editor_font_notify.on_change();
                } else if (cmd == IDM_RESETSTYLE) {
                    extern const char* g_default_colour;
                    cfg_colour = g_default_colour;
                    if (g_cur_tab2 == 1)
                        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, cfg_colour);
                    pvt::ng_playlist_view_t::g_update_all_items();
                }
            }

            break;
            case IDC_GLOBALSORT:
                cfg_global_sort = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                break;
            case IDC_APPLY:
                save_string(wnd);
                pvt::ng_playlist_view_t::g_update_all_items();
                break;
            case IDC_PICK_COLOUR:
                colour_code_gen(wnd, IDC_COLOUR, false, false);
                break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_PVIEW_GLOBALS,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Globals"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:playlist_view:globals";
        return true;
    }

private:
    WNDPROC m_edit_proc{};
    cui::prefs::PreferencesTabHelper m_helper{IDC_TITLE1};
} g_tab_global;

preferences_tab* g_get_tab_global()
{
    return &g_tab_global;
}
