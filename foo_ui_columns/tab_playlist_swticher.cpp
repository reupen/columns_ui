#include "stdafx.h"
#include "playlist_switcher_v2.h"
#include "config.h"

static class TabPlaylistSwitcher : public PreferencesTab {
public:
    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            uSendDlgItemMessageText(wnd, IDC_PLISTEDGE, CB_ADDSTRING, 0, "None");
            uSendDlgItemMessageText(wnd, IDC_PLISTEDGE, CB_ADDSTRING, 0, "Sunken");
            uSendDlgItemMessageText(wnd, IDC_PLISTEDGE, CB_ADDSTRING, 0, "Grey");

            SendDlgItemMessage(wnd, IDC_SPINPL, UDM_SETRANGE32, -100, 100);

            SendDlgItemMessage(wnd, IDC_MCLICK, BM_SETCHECK, cfg_mclick, 0);
            SendDlgItemMessage(wnd, IDC_PLISTEDGE, CB_SETCURSEL, cfg_plistframe, 0);
            SendDlgItemMessage(wnd, IDC_SPINPL, UDM_SETPOS32, 0, settings::playlist_switcher_item_padding);
            SendDlgItemMessage(wnd, IDC_USE_PLAYLIST_TF, BM_SETCHECK, cfg_playlist_switcher_use_tagz, 0);
            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_TF, WM_SETTEXT, 0, cfg_playlist_switcher_tagz);

            m_initialised = true;
        } break;
        case WM_DESTROY:
            m_initialised = false;
            break;
        case WM_COMMAND:
            switch (wp) {
            case (EN_CHANGE << 16) | IDC_PLHEIGHT:
                if (m_initialised) {
                    BOOL result;
                    int new_height = GetDlgItemInt(wnd, IDC_PLHEIGHT, &result, TRUE);
                    if (result)
                        settings::playlist_switcher_item_padding = new_height;
                    PlaylistSwitcher::g_on_vertical_item_padding_change();
                }
                break;
            case IDC_MCLICK:
                cfg_mclick = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                break;
            case (EN_CHANGE << 16) | IDC_PLAYLIST_TF:
                cfg_playlist_switcher_tagz = string_utf8_from_window((HWND)lp);
                if (cfg_playlist_switcher_use_tagz)
                    PlaylistSwitcher::g_refresh_all_items();
                break;
            case IDC_USE_PLAYLIST_TF:
                cfg_playlist_switcher_use_tagz = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                PlaylistSwitcher::g_refresh_all_items();
                break;
            case (CBN_SELCHANGE << 16) | IDC_PLISTEDGE:
                cfg_plistframe = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
                PlaylistSwitcher::g_on_edgestyle_change();
                break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_PLAYLIST_SWITCHER,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Playlist switcher"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:playlist_switcher:general";
        return true;
    }

private:
    cui::prefs::PreferencesTabHelper m_helper{IDC_TITLE1};
    bool m_initialised{};

} g_tab_playlist_switcher;

PreferencesTab* g_get_tab_playlist_switcher()
{
    return &g_tab_playlist_switcher;
}
