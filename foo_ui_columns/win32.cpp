#include "pch.h"

namespace cui::win32 {

bool check_windows_10_build(DWORD build_number)
{
    OSVERSIONINFOEX osviex{};
    osviex.dwOSVersionInfoSize = sizeof(osviex);

    DWORDLONG mask = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    mask = VerSetConditionMask(mask, VER_MINORVERSION, VER_GREATER_EQUAL);
    mask = VerSetConditionMask(mask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

    osviex.dwMajorVersion = 10;
    osviex.dwMinorVersion = 0;
    osviex.dwBuildNumber = build_number;

    return VerifyVersionInfoW(&osviex, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, mask) != FALSE;
}

bool is_windows_11_rtm_or_newer()
{
    static auto is_22000_or_newer = check_windows_10_build(22'000);
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

} // namespace cui::win32
