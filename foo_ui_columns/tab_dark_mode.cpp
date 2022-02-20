#include "stdafx.h"
#include "tab_dark_mode.h"

#include "config_appearance.h"
#include "dark_mode.h"

bool TabDarkMode::is_active()
{
    return m_wnd != nullptr;
}

bool TabDarkMode::get_help_url(pfc::string_base& p_out)
{
    return false;
}

const char* TabDarkMode::get_name()
{
    return "Mode";
}

HWND TabDarkMode::create(HWND wnd)
{
    return m_helper.create(
        wnd, IDD_PREFS_DARK_MODE, [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
}

BOOL TabDarkMode::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;

        const auto disabled_wnd = GetDlgItem(wnd, IDC_DARK_MODE_DISABLED);
        const auto enabled_wnd = GetDlgItem(wnd, IDC_DARK_MODE_ENABLED);
        const auto use_system_setting_wnd = GetDlgItem(wnd, IDC_DARK_MODE_USE_SYSTEM_SETTING);
        const auto windows_too_old_wnd = GetDlgItem(wnd, IDC_WINDOWS_VERSION_TOO_OLD);
        const auto unknown_windows_build_wnd = GetDlgItem(wnd, IDC_UNKNOWN_WINDOWS_BUILD);

        if (!cui::dark::does_os_support_dark_mode()) {
            ShowWindow(windows_too_old_wnd, SW_SHOWNORMAL);
            EnableWindow(disabled_wnd, FALSE);
            EnableWindow(enabled_wnd, FALSE);
            EnableWindow(use_system_setting_wnd, FALSE);
            Button_SetCheck(disabled_wnd, BST_CHECKED);
            break;
        }

        if (!cui::dark::are_private_apis_allowed()) {
            ShowWindow(unknown_windows_build_wnd, SW_SHOWNORMAL);
        }

        switch (static_cast<cui::colours::DarkModeStatus>(cui::colours::dark_mode_status.get())) {
        case cui::colours::DarkModeStatus::Disabled:
            Button_SetCheck(disabled_wnd, BST_CHECKED);
            break;
        case cui::colours::DarkModeStatus::Enabled:
            Button_SetCheck(enabled_wnd, BST_CHECKED);
            break;
        case cui::colours::DarkModeStatus::UseSystemSetting:
            Button_SetCheck(use_system_setting_wnd, BST_CHECKED);
            break;
        }
        break;
    }
    case WM_DESTROY:
        m_wnd = nullptr;
        break;
    case WM_COMMAND:
        switch (wp) {
        case IDC_DARK_MODE_ENABLED:
            cui::colours::dark_mode_status.set(WI_EnumValue(cui::colours::DarkModeStatus::Enabled));
            return 0;
        case IDC_DARK_MODE_DISABLED:
            cui::colours::dark_mode_status.set(WI_EnumValue(cui::colours::DarkModeStatus::Disabled));
            return 0;
        case IDC_DARK_MODE_USE_SYSTEM_SETTING:
            cui::colours::dark_mode_status.set(WI_EnumValue(cui::colours::DarkModeStatus::UseSystemSetting));
            return 0;
        }
        break;
    }
    return 0;
}
