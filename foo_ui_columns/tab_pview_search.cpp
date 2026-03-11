#include "pch.h"
#include "ng_playlist/ng_playlist.h"
#include "config.h"

namespace cui::prefs {

namespace {

class TabPlaylistViewSearch : public PreferencesTab {
public:
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_PVIEW_SEARCH,
            [this](auto&&... args) { return handle_message(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Search"; }

private:
    void enable_controls() const
    {
        const auto should_enable_title_format_fields
            = playlist_search::cfg_search_bar_mode != WI_EnumValue(playlist_search::SearchMode::mode_query);
        EnableWindow(m_search_script_wnd, should_enable_title_format_fields);
        EnableWindow(m_ignore_symbols_wnd, should_enable_title_format_fields);

        uSetWindowText(
            m_search_script_wnd, should_enable_title_format_fields ? playlist_search::cfg_search_bar_script : "");
        Button_SetCheck(m_ignore_symbols_wnd,
            should_enable_title_format_fields && cui::playlist_search::cfg_search_bar_ignore_symbols ? BST_CHECKED : 0);
    }

    BOOL handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_initialising = true;
            m_wnd = wnd;
            m_use_search_bar_wnd = GetDlgItem(wnd, IDC_USE_SEARCH_BAR);
            m_search_bar_mode_wnd = GetDlgItem(wnd, IDC_SEARCH_BAR_MODE);
            m_search_script_wnd = GetDlgItem(wnd, IDC_SEARCH_BAR_SCRIPT);
            m_ignore_symbols_wnd = GetDlgItem(wnd, IDC_IGNORE_SYMBOLS);

            uih::enhance_edit_control(wnd, IDC_SEARCH_BAR_SCRIPT);

            ComboBox_AddString(m_search_bar_mode_wnd, L"Match beginnings of words in formatted title");
            ComboBox_AddString(m_search_bar_mode_wnd, L"Match anywhere in formatted title");
            ComboBox_AddString(m_search_bar_mode_wnd, L"Use query language");

            Button_SetCheck(m_use_search_bar_wnd, panels::playlist_view::cfg_use_search_bar ? BST_CHECKED : 0);
            ComboBox_SetCurSel(m_search_bar_mode_wnd, playlist_search::cfg_search_bar_mode);

            enable_controls();
            m_initialising = false;
            break;
        case WM_DESTROY:
            m_wnd = nullptr;
            break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_USE_SEARCH_BAR:
                panels::playlist_view::cfg_use_search_bar
                    = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                break;
            case IDC_IGNORE_SYMBOLS:
                playlist_search::cfg_search_bar_ignore_symbols
                    = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                playlist_search::PlaylistSearch::s_mark_results_stale();
                break;
            case IDC_SEARCH_BAR_MODE | CBN_SELCHANGE << 16:
                playlist_search::cfg_search_bar_mode = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));

                m_initialising = true;
                enable_controls();
                m_initialising = false;

                playlist_search::PlaylistSearch::s_mark_results_stale();
                break;
            case IDC_SEARCH_BAR_SCRIPT | EN_CHANGE << 16:
                if (!m_initialising) {
                    playlist_search::cfg_search_bar_script = uGetWindowText(reinterpret_cast<HWND>(lp));
                    playlist_search::PlaylistSearch::s_mark_results_stale();
                }
                break;
            }
        }
        return 0;
    }

    HWND m_wnd{};
    HWND m_use_search_bar_wnd{};
    HWND m_search_bar_mode_wnd{};
    HWND m_search_script_wnd{};
    HWND m_ignore_symbols_wnd{};
    bool m_initialising{};
    PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

TabPlaylistViewSearch g_tab_playlist_view_search;

} // namespace

PreferencesTab* g_get_tab_playlist_view_search()
{
    return &g_tab_playlist_view_search;
}

} // namespace cui::prefs
