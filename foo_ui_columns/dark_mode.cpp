#include "stdafx.h"
#include "dark_mode.h"

namespace cui::dark {

bool is_dark_mode_enabled()
{
    return false;
}

void enable_dark_mode_for_app()
{
    enum class PreferredAppMode { System = 1, Forced = 2, Disabled = 3 };

    using SetPreferredAppModeProc = int(__stdcall*)(int);
    using FlushMenuThemesProc = void(__stdcall*)();

    const wil::unique_hmodule uxtheme(THROW_LAST_ERROR_IF_NULL(LoadLibrary(L"uxtheme.dll")));

    const auto set_preferred_app_mode
        = reinterpret_cast<SetPreferredAppModeProc>(GetProcAddress(uxtheme.get(), MAKEINTRESOURCEA(135)));

    set_preferred_app_mode(WI_EnumValue(PreferredAppMode::Forced));
}

void enable_top_level_non_client_dark_mode(HWND wnd)
{
    // Valid in Windows 10 10.0.18985 and newer (effectively 20H1+)
    constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
    constexpr BOOL value = TRUE;
    DwmSetWindowAttribute(wnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

namespace {

COLORREF get_dark_system_colour(int system_colour_id)
{
    // Unfortunately, these are hard-coded as there doesn't seem to be a simple
    // way to get a similar set of dark mode colours from Windows.
    switch (system_colour_id) {
    case COLOR_HIGHLIGHTTEXT:
    case COLOR_MENUTEXT:
    case COLOR_WINDOWTEXT:
        return RGB(255, 255, 255);
    case COLOR_WINDOW:
        return RGB(32, 32, 32);
    case COLOR_HIGHLIGHT:
        return RGB(98, 98, 98);
    case COLOR_BTNTEXT:
        return RGB(255, 255, 255);
    case COLOR_BTNFACE:
        return RGB(51, 51, 51);
    case COLOR_WINDOWFRAME:
        return RGB(119, 119, 119);
    case COLOR_3DDKSHADOW:
        // Standard value: RGB(105, 105, 105)
        return RGB(28, 28, 28);
    case COLOR_3DHILIGHT:
        // Standard value: RGB(255, 255, 255)
        return RGB(100, 100, 100);
    case COLOR_3DLIGHT:
        // Standard value: RGB(227, 227, 227)
        return RGB(42, 42, 42);
    case COLOR_3DSHADOW:
        // Standard value: RGB(160, 160, 160)
        return RGB(100, 100, 100);
    default:
        return RGB(255, 0, 0);
    }
}

} // namespace

[[nodiscard]] COLORREF get_system_colour(int system_colour_id, bool is_dark)
{
    if (is_dark)
        return get_dark_system_colour(system_colour_id);

    return GetSysColor(system_colour_id);
}

[[nodiscard]] wil::unique_hbrush get_system_colour_brush(int system_colour_id, bool is_dark)
{
    return wil::unique_hbrush(CreateSolidBrush(get_system_colour(system_colour_id, is_dark)));
}

} // namespace cui::dark
