#include "pch.h"

#include "config_host.h"
#include "prefs_utils.h"

void PreferencesInstanceTabsHost::make_child()
{
    destroy_child();

    RECT tab;

    GetWindowRect(m_wnd_tabs, &tab);
    MapWindowPoints(HWND_DESKTOP, m_wnd, (LPPOINT)&tab, 2);

    TabCtrl_AdjustRect(m_wnd_tabs, FALSE, &tab);

    if (m_active_tab >= (int)m_tabs.size())
        m_active_tab = 0;

    if (m_active_tab < (int)m_tabs.size() && m_active_tab >= 0) {
        m_child = m_tabs[m_active_tab]->create(m_wnd);
    }

    if (m_child) {
        EnableThemeDialogTexture(m_child, ETDT_ENABLETAB);
    }

    SetWindowPos(m_child, nullptr, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
    SetWindowPos(m_wnd_tabs, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ShowWindow(m_child, SW_SHOWNORMAL);
}

namespace cui::prefs {
HWND PreferencesTabHelper::create(
    HWND parent_window, UINT id, std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message_callback)
{
    m_on_message_callback = std::move(on_message_callback);
    auto on_message_ = [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); };

    auto [wnd, _] = fbh::auto_dark_modeless_dialog_box(id, parent_window, std::move(on_message_));
    return wnd;
}

INT_PTR PreferencesTabHelper::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        on_initdialog(wnd);
        break;
    case WM_NCDESTROY:
        on_ncdestroy();
        break;
    default:
        break;
    }
    return m_on_message_callback(wnd, msg, wp, lp);
}

void PreferencesTabHelper::on_initdialog(HWND wnd)
{
    m_wnd = wnd;
    m_h1_font.reset(create_default_title_font());

    if (!m_h2_ctrl_ids.empty())
        m_h2_font.reset(create_default_ui_font(10));

    const auto set_font = [wnd](auto&& font, auto&& ctrl_ids) {
        const auto windows = helpers::get_child_windows(wnd, [wnd, &ctrl_ids](HWND wnd_child) {
            return GetAncestor(wnd_child, GA_PARENT) == wnd && ctrl_ids.contains(GetDlgCtrlID(wnd_child));
        });

        for (auto&& child : windows) {
            SetWindowFont(child, font.get(), FALSE);
        }
    };

    set_font(m_h1_font, m_h1_ctrl_ids);
    set_font(m_h2_font, m_h2_ctrl_ids);
}

void PreferencesTabHelper::on_ncdestroy()
{
    m_h1_font.reset();
    m_h2_font.reset();
    m_wnd = nullptr;
}

} // namespace cui::prefs

void PreferencesTabsHost::show_tab(const char* tab_name)
{
    const auto previous_tab = m_active_tab.get_value();

    for (size_t n = 0; n < m_tabs.size(); n++) {
        if (!strcmp(m_tabs[n]->get_name(), tab_name)) {
            m_active_tab = gsl::narrow<int>(n);
            break;
        }
    }

    if (previous_tab != m_active_tab && !m_instances.empty()) {
        const auto instance = *m_instances.rbegin();
        instance->on_active_tab_change();
    }

    ui_control::get()->show_preferences(get_guid());
}

void PreferencesInstanceTabsHost::on_active_tab_change()
{
    if (m_wnd_tabs) {
        TabCtrl_SetCurSel(m_wnd_tabs, m_active_tab);

        if (IsWindowVisible(m_wnd))
            make_child();
    }
}

INT_PTR PreferencesInstanceTabsHost::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_wnd_tabs = GetDlgItem(wnd, IDC_TAB1);
        const auto count = m_tabs.size();
        for (size_t n = 0; n < count; n++) {
            uTabCtrl_InsertItemText(m_wnd_tabs, gsl::narrow<int>(n), m_tabs[n]->get_name());
        }
        TabCtrl_SetCurSel(m_wnd_tabs, m_active_tab);
        make_child();
    } break;
    case WM_DESTROY:
        m_wnd_tabs = nullptr;
        m_wnd = nullptr;
        break;
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);
        // Temporary workaround for various bugs that occur due to foobar2000 1.0+
        // having a dislike for destroying preference pages
        if (lpwp->flags & SWP_HIDEWINDOW) {
            destroy_child();
        } else if (lpwp->flags & SWP_SHOWWINDOW && !m_child) {
            make_child();
        }
    } break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lp)->idFrom) {
        case IDC_TAB1:
            switch (((LPNMHDR)lp)->code) {
            case TCN_SELCHANGE: {
                m_active_tab = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
                make_child();
            } break;
            }
            break;
        }
        break;

    case WM_PARENTNOTIFY:
        switch (wp) {
        case WM_DESTROY: {
            if (m_child && (HWND)lp == m_child)
                m_child = nullptr;
        } break;
        }
        break;
    }
    return 0;
}
