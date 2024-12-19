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

} // namespace cui::win32
