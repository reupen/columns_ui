#include "pch.h"
#include "config.h"

static class TabPlaylistSwitcherDragAndDrop : public PreferencesTab {
    static bool initialised;

    static void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_AUTOSWITCH, BM_SETCHECK, cfg_drag_autoswitch, 0);

        SendDlgItemMessage(wnd, IDC_SWITCH_SPIN, UDM_SETPOS32, 0, cfg_autoswitch_delay);
        SendDlgItemMessage(wnd, IDC_ACTIVATE_TARGET, BM_SETCHECK,
            main_window::config_get_activate_target_playlist_on_dropped_items(), 0);

        // SendDlgItemMessage(wnd,IDC_DROP_NAME,BM_SETCHECK,cfg_pgen_dir,0);
        // SendDlgItemMessage(wnd,IDC_DROP_PLAYLIST,BM_SETCHECK,cfg_pgen_playlist,0);
        SendDlgItemMessage(wnd, IDC_DROP_USE_STRING, BM_SETCHECK, cfg_pgen_tf, 0);
        SendDlgItemMessage(wnd, IDC_REMOVE_UNDERSCORES, BM_SETCHECK, cfg_replace_drop_underscores, 0);
        uSendDlgItemMessageText(wnd, IDC_DROP_STRING, WM_SETTEXT, 0, cfg_pgenstring);
    }

public:
    static INT_PTR CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            uih::enhance_edit_control(wnd, IDC_DROP_STRING);
            uih::enhance_edit_control(wnd, IDC_SWITCH_DELAY);
            SendDlgItemMessage(wnd, IDC_SWITCH_SPIN, UDM_SETRANGE32, 0, 10000);

            refresh_me(wnd);
            initialised = true;
        }

        break;
        case WM_DESTROY: {
            initialised = false;
        } break;
        case WM_COMMAND:
            switch (wp) {
            case (EN_CHANGE << 16) | IDC_SWITCH_DELAY: {
                if (initialised) {
                    BOOL result;
                    unsigned new_height = GetDlgItemInt(wnd, IDC_SWITCH_DELAY, &result, FALSE);
                    if (result)
                        cfg_autoswitch_delay = new_height;
                }
            } break;
            case (EN_CHANGE << 16) | IDC_DROP_STRING:
                cfg_pgenstring = uGetWindowText((HWND)lp);
                break;
#if 0
            case IDC_DROP_NAME:
                cfg_pgen_dir = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_DROP_PLAYLIST:
                cfg_pgen_playlist = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
#endif
            case IDC_REMOVE_UNDERSCORES:
                cfg_replace_drop_underscores = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_DROP_USE_STRING:
                cfg_pgen_tf = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                break;
            case IDC_ACTIVATE_TARGET:
                main_window::config_set_activate_target_playlist_on_dropped_items(
                    0 != SendMessage((HWND)lp, BM_GETCHECK, 0, 0));
                break;
            case IDC_AUTOSWITCH: {
                cfg_drag_autoswitch = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
            } break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_PLAYLISTS_DRAGDROP,
            [](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Drag and drop"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:playlist_switcher:drag_and_drop";
        return true;
    }

private:
    cui::prefs::PreferencesTabHelper m_helper{IDC_TITLE1};
} g_tab_playlist_dd;

bool TabPlaylistSwitcherDragAndDrop::initialised = false;

PreferencesTab* g_get_tab_playlist_dd()
{
    return &g_tab_playlist_dd;
}
