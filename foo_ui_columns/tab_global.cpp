#include "pch.h"
#include "ng_playlist/ng_playlist.h"
#include "config.h"
#include "help.h"

extern const char* default_global_style_script;

namespace cui::prefs {

namespace {

enum class ColumnTabIndex : int32_t {
    VariablesScript,
    StyleScript,
};

cfg_int cfg_globals_tab_index(GUID{0x5fb6e011, 0x1ead, 0x49fe, {0x45, 0x32, 0x1c, 0x8a, 0x61, 0x01, 0x91, 0x2b}},
    WI_EnumValue(ColumnTabIndex::VariablesScript));

class TabGlobal : public PreferencesTab {
public:
    static void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_GLOBAL, BM_SETCHECK, cfg_global, 0);
        SendDlgItemMessage(wnd, IDC_GLOBALSORT, BM_SETCHECK, cfg_global_sort, 0);
        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0,
            (cfg_globals_tab_index == WI_EnumValue(ColumnTabIndex::VariablesScript) ? cfg_globalstring : cfg_colour));
    }

    static void save_string(HWND wnd)
    {
        switch (cfg_globals_tab_index) {
        case WI_EnumValue(ColumnTabIndex::VariablesScript):
            cfg_globalstring = uGetDlgItemText(wnd, IDC_STRING);
            break;
        case WI_EnumValue(ColumnTabIndex::StyleScript):
            cfg_colour = uGetDlgItemText(wnd, IDC_STRING);
            break;
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

            TabCtrl_SetCurSel(wnd_tab, cfg_globals_tab_index);

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
                    cfg_globals_tab_index = id;
                    uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0,
                        (cfg_globals_tab_index == WI_EnumValue(ColumnTabIndex::VariablesScript) ? cfg_globalstring
                                                                                                : cfg_colour));
                } break;
                }
                break;
            }
            break;

        case WM_DESTROY: {
            g_editor_font_notify.release();
            save_string(wnd);
            panels::playlist_view::PlaylistView::s_update_all_items();
        } break;

        case WM_COMMAND:
            switch (wp) {
            case IDC_GLOBAL:
                cfg_global = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_TFHELP: {
                RECT rc{};
                GetWindowRect(GetDlgItem(wnd, IDC_TFHELP), &rc);

                uih::Menu menu;
                uih::MenuCommandCollector collector;

                menu.append_command(
                    collector.add([] { standard_commands::main_titleformat_help(); }), L"Title formatting help");
                menu.append_command(
                    collector.add([wnd] { help::open_style_script_help(GetParent(wnd)); }), L"Style script help");
                menu.append_command(collector.add([wnd] { help::open_global_variables_help(GetParent(wnd)); }),
                    L"Global variables help");

                if (cfg_globals_tab_index == WI_EnumValue(ColumnTabIndex::VariablesScript)) {
                    menu.append_separator();
                    menu.append_command(
                        collector.add([wnd] { help::open_text_styling_help(GetParent(wnd)); }), L"Text styling help");
                }

                menu.append_separator();
                menu.append_command(collector.add([] { speedtest(g_columns, cfg_global != 0); }), L"Speed test");
                menu.append_command(collector.add([wnd] {
                    preview_to_console(uGetDlgItemText(wnd, IDC_STRING),
                        cfg_globals_tab_index != WI_EnumValue(ColumnTabIndex::VariablesScript) && cfg_global);
                    ;
                }),
                    L"Preview script");
                menu.append_separator();
                menu.append_command(collector.add([wnd] {
                    if (auto font_description = fonts::select_font(GetParent(wnd), cfg_editor_font->log_font)) {
                        cfg_editor_font = *font_description;
                        g_editor_font_notify.on_change();
                    };
                }),
                    L"Change editor font");
                menu.append_separator();
                menu.append_command(collector.add([wnd] {
                    cfg_colour = default_global_style_script;

                    if (cfg_globals_tab_index == WI_EnumValue(ColumnTabIndex::StyleScript))
                        uSendDlgItemMessageText(wnd, IDC_STRING, WM_SETTEXT, 0, cfg_colour);

                    panels::playlist_view::PlaylistView::s_update_all_items();
                }),
                    L"Reset style script");

                menu_helpers::win32_auto_mnemonics(menu.get());

                collector.execute(menu.run(wnd, {rc.left, rc.bottom}));
                break;
            }
            case IDC_GLOBALSORT:
                cfg_global_sort = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_APPLY:
                save_string(wnd);
                panels::playlist_view::PlaylistView::s_update_all_items();
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
        p_out = "https://columns-ui.readthedocs.io/page/playlist-view/global-variables.html";
        return true;
    }

private:
    PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

TabGlobal g_tab_global;

} // namespace

PreferencesTab* g_get_tab_global()
{
    return &g_tab_global;
}

} // namespace cui::prefs
