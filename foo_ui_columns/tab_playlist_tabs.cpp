#include "pch.h"
#include "playlist_tabs.h"
#include "config.h"

static class TabPlaylistTabs : public PreferencesTab {
public:
    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            SendDlgItemMessage(wnd, IDC_MCLICK3, BM_SETCHECK, cfg_plm_rename, 0);
            SendDlgItemMessage(wnd, IDC_PLDRAG, BM_SETCHECK, cfg_drag_pl, 0);
            SendDlgItemMessage(wnd, IDC_PLAUTOHIDE, BM_SETCHECK, cfg_pl_autohide == 0, 0);
            SendDlgItemMessage(
                wnd, IDC_PLAYLIST_TABS_MCLICK, BM_SETCHECK, cui::config::cfg_playlist_tabs_middle_click, 0);

            SendDlgItemMessage(wnd, IDC_TABS_MULTILINE, BM_SETCHECK, cfg_tabs_multiline, 0);
            break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_PLAUTOHIDE:
                cfg_pl_autohide = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_UNCHECKED;
                cui::panels::playlist_tabs::g_on_autohide_tabs_change();
                break;
            case IDC_PLAYLIST_TABS_MCLICK:
                cui::config::cfg_playlist_tabs_middle_click
                    = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_TABS_MULTILINE:
                cfg_tabs_multiline = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                cui::panels::playlist_tabs::g_on_multiline_tabs_change();
                break;
            case IDC_MCLICK3:
                cfg_plm_rename = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_PLDRAG:
                cfg_drag_pl = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_PLAYLIST_TABS,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Playlist tabs"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:playlist_switcher:general";
        return true;
    }

private:
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
} g_tab_playlist_tabs;

PreferencesTab* g_get_tab_playlist_tabs()
{
    return &g_tab_playlist_tabs;
}
