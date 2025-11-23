#include "pch.h"
#include "ng_playlist/ng_playlist.h"
#include "config.h"

namespace cui::prefs {

namespace {

class TabPlaylistViewArtwork : public PreferencesTab {
    void refresh_me(HWND wnd)
    {
        SendDlgItemMessage(wnd, IDC_SHOWARTWORK, BM_SETCHECK, panels::playlist_view::cfg_show_artwork, 0);
        SendDlgItemMessage(wnd, IDC_STICKY_ARTWORK, BM_SETCHECK, panels::playlist_view::cfg_sticky_artwork, 0);
        SendDlgItemMessage(wnd, IDC_ARTWORKREFLECTION, BM_SETCHECK, panels::playlist_view::cfg_artwork_reflection, 0);

        SendDlgItemMessage(wnd, IDC_ARTWORKWIDTHSPIN, UDM_SETRANGE32, 0, MAXLONG);

        SendDlgItemMessage(wnd, IDC_ARTWORKWIDTHSPIN, UDM_SETPOS32, NULL, panels::playlist_view::cfg_artwork_width);
    }

public:
    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            uih::enhance_edit_control(wnd, IDC_ARTWORKWIDTH);
            refresh_me(wnd);
            m_initialised = true;
            break;
        case WM_DESTROY:
            m_initialised = false;
            break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_SHOWARTWORK:
                panels::playlist_view::cfg_show_artwork = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                panels::playlist_view::PlaylistView::g_on_show_artwork_change();
                break;
            case IDC_STICKY_ARTWORK:
                panels::playlist_view::cfg_sticky_artwork
                    = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                panels::playlist_view::PlaylistView::s_on_sticky_artwork_change();
                break;
            case IDC_ARTWORKREFLECTION:
                panels::playlist_view::cfg_artwork_reflection
                    = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                panels::playlist_view::PlaylistView::g_on_artwork_width_change();
                break;
            case (EN_CHANGE << 16) | IDC_ARTWORKWIDTH:
                if (m_initialised) {
                    panels::playlist_view::cfg_artwork_width
                        = mmh::strtoul_n(uGetWindowText(reinterpret_cast<HWND>(lp)).get_ptr(), pfc_infinite);
                    panels::playlist_view::PlaylistView::g_on_artwork_width_change();
                }
                break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_PVIEW_ARTWORK,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Artwork"; }

private:
    bool m_initialised{};
    PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

TabPlaylistViewArtwork g_tab_pview_artwork;

} // namespace

PreferencesTab* g_get_tab_pview_artwork()
{
    return &g_tab_pview_artwork;
}

} // namespace cui::prefs
