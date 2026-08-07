#include "pch.h"

namespace cui::win32 {

bool is_windows_11_rtm_or_newer()
{
    static auto is_22000_or_newer = mmh::check_windows_10_build(22'000);
    return is_22000_or_newer;
}

std::wstring get_display_device_key(HMONITOR monitor)
{
    static concurrency::concurrent_unordered_map<HMONITOR, std::wstring> cache{};

    if (const auto iter = cache.find(monitor); iter != cache.end())
        return iter->second;

    MONITORINFOEX monitor_info{};
    monitor_info.cbSize = sizeof(monitor_info);
    if (!GetMonitorInfo(monitor, &monitor_info)) {
        cache.insert({monitor, {}});
        return {};
    }

    DISPLAY_DEVICE display_device{};
    display_device.cb = sizeof(display_device);

    if (!EnumDisplayDevicesW(monitor_info.szDevice, 0, &display_device, 0)) {
#ifdef _DEBUG
        console::print("Columns UI – EnumDisplayDevicesW failed");
#endif
        cache.insert({monitor, {}});
        return {};
    }

    cache.insert({monitor, display_device.DeviceKey});
    return display_device.DeviceKey;
}

void add_window_styles(HWND wnd, DWORD styles_to_add)
{
    const auto current_styles = GetWindowLongPtr(wnd, GWL_STYLE);
    SetWindowLongPtr(wnd, GWL_STYLE, current_styles | styles_to_add);
}

void remove_window_styles(HWND wnd, DWORD styles_to_remove)
{
    const auto current_styles = GetWindowLongPtr(wnd, GWL_STYLE);
    SetWindowLongPtr(wnd, GWL_STYLE, current_styles & ~static_cast<DWORD_PTR>(styles_to_remove));
}

void handle_tab_key(HWND wnd)
{
    if (const auto focused_wnd = uie::window::g_on_tab(wnd))
        uih::show_focus_indicator(focused_wnd);
}

namespace {

struct EnumThreadWindowsContext {
    HMENU m_menu{};
    HWND m_found_menu_wnd{};
};

} // namespace

HWND find_window_for_menu(HMENU menu)
{
    EnumThreadWindowsContext context{.m_menu = menu};

    EnumThreadWindows(
        0,
        [](HWND wnd, LPARAM lp) {
            const auto context = reinterpret_cast<EnumThreadWindowsContext*>(lp);

            std::array<wchar_t, 256> class_name{};
            if (GetClassName(wnd, class_name.data(), gsl::narrow<int>(class_name.size())) == 0)
                return TRUE;

            if (wcsncmp(L"#32768", class_name.data(), class_name.size()) != 0)
                return TRUE;

            if (reinterpret_cast<HMENU>(SendMessage(wnd, MN_GETHMENU, 0, 0)) != context->m_menu)
                return TRUE;

            context->m_found_menu_wnd = wnd;
            return FALSE;
        },
        reinterpret_cast<LPARAM>(&context));

    return context.m_found_menu_wnd;
}

} // namespace cui::win32
