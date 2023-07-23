#include "pch.h"
#include "ng_playlist/ng_playlist.h"
#include "config.h"
#include "help.h"
#include "prefs_utils.h"

extern const char* default_global_style_script;

static cfg_int g_cur_tab2(GUID{0x5fb6e011, 0x1ead, 0x49fe, {0x45, 0x32, 0x1c, 0x8a, 0x61, 0x01, 0x91, 0x2b}}, 0);

class TabGlobal : public PreferencesTab {
public:
    static void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_GLOBAL, BM_SETCHECK, cfg_global, 0);
        SendDlgItemMessage(wnd, IDC_GLOBALSORT, BM_SETCHECK, cfg_global_sort, 0);
        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, (g_cur_tab2 == 0 ? cfg_globalstring : cfg_colour));
    }

    static void save_string(HWND wnd)
    {
        int id = g_cur_tab2;
        if (id >= 0 && id < 2) {
            if (id == 0)
                cfg_globalstring = uGetDlgItemText(wnd, IDC_STRING);
            else if (id == 1)
                cfg_colour = uGetDlgItemText(wnd, IDC_STRING);
        }
    }

    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            uTCITEM tabs{};

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

            uih::enhance_edit_control(wnd, IDC_STRING);
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
            cui::panels::playlist_view::PlaylistView::g_update_all_items();
        } break;

        case WM_COMMAND:
            switch (wp) {
            case IDC_GLOBAL:
                cfg_global = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_TFHELP: {
                RECT rc;
                GetWindowRect(GetDlgItem(wnd, IDC_TFHELP), &rc);
                //        MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)(&rc), 2);
                HMENU menu = CreatePopupMenu();

                enum {
                    IDM_TFHELP = 1,
                    IDM_STYLE_HELP,
                    IDM_GLOBALS_HELP,
                    IDM_SPEEDTEST,
                    IDM_PREVIEW,
                    IDM_EDITORFONT,
                    IDM_RESETSTYLE
                };

                uAppendMenu(menu, (MF_STRING), IDM_TFHELP, "Title formatting &help");
                uAppendMenu(menu, (MF_STRING), IDM_STYLE_HELP, "&Style script help");
                uAppendMenu(menu, (MF_STRING), IDM_GLOBALS_HELP, "&Global variables help");
                uAppendMenu(menu, (MF_SEPARATOR), 0, "");
                uAppendMenu(menu, (MF_STRING), IDM_SPEEDTEST, "Speed &test");
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
                } else if (cmd == IDM_STYLE_HELP) {
                    cui::help::open_colour_script_help(GetParent(wnd));
                } else if (cmd == IDM_GLOBALS_HELP) {
                    cui::help::open_global_variables_help(GetParent(wnd));
                } else if (cmd == IDM_SPEEDTEST) {
                    speedtest(g_columns, cfg_global != 0);
                } else if (cmd == IDM_PREVIEW) {
                    preview_to_console(uGetDlgItemText(wnd, IDC_STRING), g_cur_tab2 != 0 && cfg_global);
                } else if (cmd == IDM_EDITORFONT) {
                    auto font_description = cui::fonts::select_font(GetParent(wnd), cfg_editor_font->log_font);
                    if (font_description) {
                        cfg_editor_font = *font_description;
                        g_editor_font_notify.on_change();
                    }
                } else if (cmd == IDM_RESETSTYLE) {
                    cfg_colour = default_global_style_script;
                    if (g_cur_tab2 == 1)
                        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, cfg_colour);
                    cui::panels::playlist_view::PlaylistView::g_update_all_items();
                }
            }

            break;
            case IDC_GLOBALSORT:
                cfg_global_sort = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_APPLY:
                save_string(wnd);
                cui::panels::playlist_view::PlaylistView::g_update_all_items();
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
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
} g_tab_global;

PreferencesTab* g_get_tab_global()
{
    return &g_tab_global;
}
