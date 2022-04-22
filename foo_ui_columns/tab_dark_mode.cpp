#include "pch.h"
#include "tab_dark_mode.h"

#include "config_appearance.h"
#include "dark_mode.h"
#include "system_appearance_manager.h"

constexpr auto unknown_windows_build_message
    = "Your version of Windows has not been tested for dark mode compatibility. Some dark mode features have been disabled."sv;
constexpr auto windows_too_old_message = "Dark mode requires Windows 10 version 2004 or newer."sv;
constexpr auto dark_mode_unavailable_message = "Dark mode is unavailable due to the current system settings."sv;

bool TabDarkMode::is_active()
{
    return m_wnd != nullptr;
}

void TabDarkMode::refresh()
{
    if (!is_active() || m_is_updating)
        return;

    const auto disabled_wnd = GetDlgItem(m_wnd, IDC_DARK_MODE_DISABLED);
    const auto enabled_wnd = GetDlgItem(m_wnd, IDC_DARK_MODE_ENABLED);
    const auto use_system_setting_wnd = GetDlgItem(m_wnd, IDC_DARK_MODE_USE_SYSTEM_SETTING);
    const auto message_wnd = GetDlgItem(m_wnd, IDC_DARK_MODE_MESSAGE);

    const auto has_os_dark_mode_support = cui::dark::does_os_support_dark_mode();
    const auto is_dark_mode_available = cui::system_appearance_manager::is_dark_mode_available();

    EnableWindow(disabled_wnd, is_dark_mode_available);
    EnableWindow(enabled_wnd, is_dark_mode_available);
    EnableWindow(use_system_setting_wnd, is_dark_mode_available);

    if (!is_dark_mode_available) {
        if (!has_os_dark_mode_support)
            uSetWindowText(message_wnd, windows_too_old_message.data());
        else
            uSetWindowText(message_wnd, dark_mode_unavailable_message.data());

        Button_SetCheck(disabled_wnd, BST_CHECKED);
        Button_SetCheck(enabled_wnd, BST_UNCHECKED);
        Button_SetCheck(use_system_setting_wnd, BST_UNCHECKED);
        return;
    }

    if (!cui::dark::are_private_apis_allowed()) {
        uSetWindowText(message_wnd, unknown_windows_build_message.data());
    } else {
        uSetWindowText(message_wnd, "");
    }

    const auto current_mode = static_cast<cui::colours::DarkModeStatus>(cui::colours::dark_mode_status.get());

    Button_SetCheck(disabled_wnd, current_mode == cui::colours::DarkModeStatus::Disabled ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(enabled_wnd, current_mode == cui::colours::DarkModeStatus::Enabled ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(use_system_setting_wnd,
        current_mode == cui::colours::DarkModeStatus::UseSystemSetting ? BST_CHECKED : BST_UNCHECKED);
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

INT_PTR TabDarkMode::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        refresh();
        break;
    }
    case WM_DESTROY:
        m_wnd = nullptr;
        break;
    case WM_COMMAND: {
        pfc::vartoggle_t _(m_is_updating, true);

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
    }
    return 0;
}
