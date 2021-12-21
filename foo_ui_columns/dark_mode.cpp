#include "stdafx.h"

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

} // namespace cui::dark
