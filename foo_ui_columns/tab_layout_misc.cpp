#include "pch.h"

#include "splitter.h"
#include "tab_layout_misc.h"

namespace cui::prefs {

HWND LayoutMiscTab::create(HWND wnd)
{
    return m_helper.create(wnd, IDD_PREFS_LAYOUT_MISC,
        [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
}

const char* LayoutMiscTab::get_name()
{
    return "Misc";
}

bool LayoutMiscTab::get_help_url(pfc::string_base& p_out)
{
    p_out = "http://yuo.be/wiki/columns_ui:config:layout";
    return true;
}

INT_PTR LayoutMiscTab::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        SetDlgItemInt(wnd, IDC_SHOW_DELAY, cfg_sidebar_show_delay, FALSE);
        SetDlgItemInt(wnd, IDC_HIDE_DELAY, cfg_sidebar_hide_delay, FALSE);
        EnableWindow(GetDlgItem(wnd, IDC_SHOW_DELAY_SPIN), cfg_sidebar_use_custom_show_delay);
        EnableWindow(GetDlgItem(wnd, IDC_SHOW_DELAY), cfg_sidebar_use_custom_show_delay);
        SendDlgItemMessage(wnd, IDC_USE_CUSTOM_SHOW_DELAY, BM_SETCHECK, cfg_sidebar_use_custom_show_delay, 0);
        SendDlgItemMessage(wnd, IDC_ALLOW_LOCKED_PANEL_RESIZING, BM_SETCHECK, settings::allow_locked_panel_resizing, 0);

        SendDlgItemMessage(wnd, IDC_SHOW_DELAY_SPIN, UDM_SETRANGE32, 0, 10000);
        SendDlgItemMessage(wnd, IDC_HIDE_DELAY_SPIN, UDM_SETRANGE32, 0, 10000);

        const HWND wnd_custom_divider_width_spin = GetDlgItem(wnd, IDC_CUSTOM_DIVIDER_WIDTH_SPIN);

        SetDlgItemInt(wnd, IDC_CUSTOM_DIVIDER_WIDTH, settings::custom_splitter_divider_width, FALSE);
        SendMessage(wnd_custom_divider_width_spin, UDM_SETRANGE32, 0, 20);

        m_initialised = true;
        break;
    }
    case WM_DESTROY:
        m_initialised = false;
        break;
    case WM_COMMAND:
        switch (wp) {
        case IDC_HIDE_DELAY | EN_CHANGE << 16:
            if (m_initialised) {
                BOOL result;
                const int new_height = GetDlgItemInt(wnd, LOWORD(wp), &result, FALSE);
                if (result)
                    cfg_sidebar_hide_delay = new_height;
            }
            break;
        case IDC_SHOW_DELAY | EN_CHANGE << 16:
            if (m_initialised) {
                BOOL result;
                const int new_height = GetDlgItemInt(wnd, LOWORD(wp), &result, FALSE);
                if (result)
                    cfg_sidebar_show_delay = new_height;
            }
            break;
        case IDC_CUSTOM_DIVIDER_WIDTH | EN_CHANGE << 16:
            if (m_initialised) {
                BOOL result;
                const int new_width = GetDlgItemInt(wnd, LOWORD(wp), &result, FALSE);
                if (result)
                    settings::custom_splitter_divider_width = new_width;
                panels::splitter::FlatSplitterPanel::g_on_size_change();
            }
            break;
        case IDC_USE_CUSTOM_SHOW_DELAY:
            cfg_sidebar_use_custom_show_delay = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
            EnableWindow(GetDlgItem(wnd, IDC_SHOW_DELAY_SPIN), cfg_sidebar_use_custom_show_delay);
            EnableWindow(GetDlgItem(wnd, IDC_SHOW_DELAY), cfg_sidebar_use_custom_show_delay);
            break;
        case IDC_ALLOW_LOCKED_PANEL_RESIZING:
            settings::allow_locked_panel_resizing = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
            break;
        }
        break;
    }
    return 0;
}

} // namespace cui::prefs
