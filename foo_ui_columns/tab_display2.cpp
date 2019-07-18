#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"
#include "config.h"
#include "prefs_utils.h"
#include "playlist_item_helpers.h"

static class TabPlaylistViewGeneral : public PreferencesTab {
    void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_HEADER, BM_SETCHECK, cfg_header, 0);
        // SendDlgItemMessage(wnd,IDC_HORIZ_WHEEL,BM_SETCHECK,cfg_scroll_h_no_v,0);
        SendDlgItemMessage(wnd, IDC_NOHSCROLL, BM_SETCHECK, cfg_nohscroll, 0);
        SendDlgItemMessage(wnd, IDC_ELLIPSIS, BM_SETCHECK, cfg_ellipsis, 0);
        SendDlgItemMessage(wnd, IDC_PLEDGE, CB_SETCURSEL, cfg_frame, 0);

        SendDlgItemMessage(
            wnd, IDC_INLINE_MODE, BM_SETCHECK, main_window::config_get_inline_metafield_edit_mode() != 0, 0);

        SendDlgItemMessage(wnd, IDC_SELECTION_MODEL, BM_SETCHECK, cfg_alternative_sel, 0);
        // SendDlgItemMessage(wnd,IDC_HORIZ_WHEEL,BM_SETCHECK,cfg_scroll_h_no_v,0);

        SendDlgItemMessage(wnd, IDC_TOOLTIPS, BM_SETCHECK, cfg_tooltip, 0);
        // SendDlgItemMessage(wnd,IDC_SORTSELONLY,BM_SETCHECK,cfg_sortsel,0);
        SendDlgItemMessage(wnd, IDC_TOOLTIPS_CLIPPED, BM_SETCHECK, cfg_tooltips_clipped, 0);
        EnableWindow(GetDlgItem(wnd, IDC_TOOLTIPS_CLIPPED), cfg_tooltip);

        SendDlgItemMessage(wnd, IDC_HHTRACK, BM_SETCHECK, cfg_header_hottrack, 0);

        SendDlgItemMessage(wnd, IDC_SPIN1, UDM_SETPOS32, 0, settings::playlist_view_item_padding);

        SendDlgItemMessage(wnd, IDC_SORT_ARROWS, BM_SETCHECK, cfg_show_sort_arrows, 0);
        SendDlgItemMessage(wnd, IDC_DROP_AT_END, BM_SETCHECK, cfg_drop_at_end, 0);
    }

public:
    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            m_menu_cache = new menu_item_cache;
            uSendDlgItemMessageText(wnd, IDC_PLEDGE, CB_ADDSTRING, 0, "None");
            uSendDlgItemMessageText(wnd, IDC_PLEDGE, CB_ADDSTRING, 0, "Sunken");
            uSendDlgItemMessageText(wnd, IDC_PLEDGE, CB_ADDSTRING, 0, "Grey");

            SendDlgItemMessage(wnd, IDC_SPIN1, UDM_SETRANGE32, -100, 100);
            //        SendDlgItemMessage(wnd,IDC_SPINPL,UDM_SETRANGE32,-100,100);
            //        SendDlgItemMessage(wnd,IDC_SPINSEL,UDM_SETRANGE32,0,3);

            populate_menu_combo(wnd, IDC_PLAYLIST_DOUBLE, IDC_MENU_DESC, cfg_playlist_double, *m_menu_cache, true);

            unsigned count = cui::playlist_item_helpers::mclick_action::get_count();
            for (unsigned n = 0; n < count; n++) {
                uSendDlgItemMessageText(wnd, IDC_PLAYLIST_MIDDLE, CB_ADDSTRING, 0,
                    cui::playlist_item_helpers::mclick_action::g_pma_actions[n].name);
                SendDlgItemMessage(wnd, IDC_PLAYLIST_MIDDLE, CB_SETITEMDATA, n,
                    cui::playlist_item_helpers::mclick_action::g_pma_actions[n].id);
            }

            SendDlgItemMessage(wnd, IDC_PLAYLIST_MIDDLE, CB_SETCURSEL,
                cui::playlist_item_helpers::mclick_action::id_to_idx(cfg_playlist_middle_action), 0);

            refresh_me(wnd);
            m_initialised = true;
        }

        break;
        case WM_DESTROY: {
            m_initialised = false;
            delete m_menu_cache;
        } break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_DROP_AT_END: {
                cfg_drop_at_end = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
            } break;
            case (EN_CHANGE << 16) | IDC_HEIGHT: {
                if (m_initialised) {
                    BOOL result;
                    int new_height = GetDlgItemInt(wnd, IDC_HEIGHT, &result, TRUE);
                    if (result)
                        settings::playlist_view_item_padding = new_height;
                    pvt::PlaylistView::g_on_vertical_item_padding_change();
                }

            } break;
            case (CBN_SELCHANGE << 16) | IDC_PLAYLIST_DOUBLE: {
                on_menu_combo_change(wnd, lp, cfg_playlist_double, *m_menu_cache, IDC_MENU_DESC);
            } break;
            case (CBN_SELCHANGE << 16) | IDC_PLAYLIST_MIDDLE: {
                cfg_playlist_middle_action
                    = SendMessage((HWND)lp, CB_GETITEMDATA, SendMessage((HWND)lp, CB_GETCURSEL, 0, 0), 0);
            } break;
            case IDC_TOOLTIPS:
                cfg_tooltip = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                EnableWindow(GetDlgItem(wnd, IDC_TOOLTIPS_CLIPPED), cfg_tooltip);
                pvt::PlaylistView::g_on_show_tooltips_change();
                break;

            case IDC_SELECTION_MODEL:
                cfg_alternative_sel = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                pvt::PlaylistView::g_on_alternate_selection_change();
                break;
            case IDC_SORT_ARROWS:
                cfg_show_sort_arrows = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                pvt::PlaylistView::g_on_show_sort_indicators_change();
                break;
            case IDC_TOOLTIPS_CLIPPED:
                cfg_tooltips_clipped = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                pvt::PlaylistView::g_on_show_tooltips_change();
                break;

            case IDC_ELLIPSIS:
                cfg_ellipsis = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                pvt::PlaylistView::s_redraw_all();
                break;

            case IDC_HEADER: {
                cfg_header = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                pvt::PlaylistView::g_on_show_header_change();
            } break;
            case IDC_NOHSCROLL: {
                cfg_nohscroll = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                pvt::PlaylistView::g_on_autosize_change();
            } break;

            case IDC_HHTRACK: {
                cfg_header_hottrack = SendMessage((HWND)lp, BM_GETCHECK, 0, 0);
                pvt::PlaylistView::g_on_sorting_enabled_change();
            } break;
            case (CBN_SELCHANGE << 16) | IDC_PLEDGE: {
                cfg_frame = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
                pvt::PlaylistView::g_on_edge_style_change();
            } break;
            case IDC_INLINE_MODE: {
                main_window::config_set_inline_metafield_edit_mode(SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0);
            } break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_PVIEW_GENERAL,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "General"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:playlist_view:general";
        return true;
    }

private:
    bool m_initialised{};
    menu_item_cache* m_menu_cache{};
    cui::prefs::PreferencesTabHelper m_helper{IDC_TITLE1};
} g_tab_display2;

PreferencesTab* g_get_tab_display2()
{
    return &g_tab_display2;
}
