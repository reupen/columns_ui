#include "pch.h"
#include "dark_mode.h"

#include "system_appearance_manager.h"

namespace cui::dark {

namespace {

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
    static auto is_22000_or_newer = check_windows_10_build(22000);
    return is_22000_or_newer;
}

} // namespace

bool does_os_support_dark_mode()
{
    static auto is_19041_or_newer = check_windows_10_build(19041);
    return is_19041_or_newer;
}

bool is_native_dark_spin_available()
{
    // Earliest known build number – exact build number unknown.
    return check_windows_10_build(22579);
}

bool are_private_apis_allowed()
{
    OSVERSIONINFO osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
#pragma warning(suppress : 4996)
    GetVersionEx(&osvi);

    if (osvi.dwMajorVersion != 10 || osvi.dwMinorVersion != 0)
        return false;

    return osvi.dwBuildNumber >= 19041 && osvi.dwBuildNumber <= 22623;
}

void set_app_mode(PreferredAppMode mode)
{
    if (!are_private_apis_allowed())
        return;

    using SetPreferredAppModeProc = int(__stdcall*)(int);
    using FlushMenuThemesProc = void(__stdcall*)();

    const wil::unique_hmodule uxtheme(THROW_LAST_ERROR_IF_NULL(LoadLibrary(L"uxtheme.dll")));

    const auto set_preferred_app_mode
        = reinterpret_cast<SetPreferredAppModeProc>(GetProcAddress(uxtheme.get(), MAKEINTRESOURCEA(135)));

    const auto flush_menu_themes
        = reinterpret_cast<FlushMenuThemesProc>(GetProcAddress(uxtheme.get(), MAKEINTRESOURCEA(136)));

    set_preferred_app_mode(WI_EnumValue(mode));
    flush_menu_themes();
}

void set_titlebar_mode(HWND wnd, bool is_dark)
{
    const BOOL value = is_dark;
    DwmSetWindowAttribute(wnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

void force_titlebar_redraw(HWND wnd)
{
    if (!IsWindowVisible(wnd))
        return;

    // The below is a hack to force the titlebar to redraw (nothing else works).
    RECT rc{};
    if (!GetWindowRect(wnd, &rc))
        return;

    const auto cx = RECT_CX(rc);
    const auto cy = RECT_CY(rc);

    if (cx <= 0)
        return;

    SetWindowPos(wnd, nullptr, 0, 0, cx - 1, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(wnd, nullptr, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

namespace {

consteval COLORREF create_grey(const int value)
{
    return RGB(value, value, value);
}

enum class DarkColourID : COLORREF {
    DARK_000,
    DARK_200,
    DARK_300,
    DARK_400,
    DARK_500,
    DARK_600,
    DARK_750,
    DARK_999,
};

COLORREF get_base_dark_colour(DarkColourID colour_id)
{
    switch (colour_id) {
    case DarkColourID::DARK_000:
        return is_windows_11_rtm_or_newer() ? create_grey(25) : create_grey(32);
    case DarkColourID::DARK_200:
        return create_grey(51);
    case DarkColourID::DARK_300:
        return create_grey(77);
    case DarkColourID::DARK_400:
        return create_grey(88);
    case DarkColourID::DARK_500:
        return create_grey(98);
    case DarkColourID::DARK_600:
        return create_grey(119);
    case DarkColourID::DARK_750:
        return create_grey(166);
    case DarkColourID::DARK_999:
        return create_grey(255);
    default:
        uBugCheck();
    }
}

wil::unique_hbrush get_dark_colour_brush(ColourID colour_id)
{
    return wil::unique_hbrush(CreateSolidBrush(get_dark_colour(colour_id)));
}

int get_light_colour_system_id(ColourID colour_id)
{
    switch (colour_id) {
    case ColourID::LayoutBackground:
        return COLOR_BTNFACE;
    case ColourID::PanelCaptionText:
        return COLOR_MENUTEXT;
    case ColourID::PanelCaptionBackground:
        return COLOR_BTNFACE;
    case ColourID::StatusBarText:
        return COLOR_MENUTEXT;
    case ColourID::StatusPaneBackground:
        return COLOR_BTNFACE;
    case ColourID::StatusPaneText:
        return COLOR_BTNTEXT;
    case ColourID::StatusPaneTopLine:
        return COLOR_3DDKSHADOW;
    case ColourID::ToolbarFlatHotBackground:
        return COLOR_HIGHLIGHT;
    case ColourID::ToolbarFlatHotText:
        return COLOR_HIGHLIGHTTEXT;
    case ColourID::VolumeChannelTopEdge:
        return COLOR_3DSHADOW;
    case ColourID::VolumeChannelBottomAndRightEdge:
        return COLOR_3DHILIGHT;
    case ColourID::VolumePopupBackground:
        return COLOR_BTNFACE;
    case ColourID::VolumePopupBorder:
        return COLOR_3DLIGHT;
    case ColourID::VolumePopupText:
        return COLOR_MENUTEXT;
    default:
        uBugCheck();
    }
}

COLORREF get_light_colour(ColourID colour_id)
{
    return GetSysColor(get_light_colour_system_id(colour_id));
}

wil::unique_hbrush get_light_colour_brush(ColourID colour_id)
{
    return wil::unique_hbrush(GetSysColorBrush(get_light_colour_system_id(colour_id)));
}

} // namespace

COLORREF get_dark_colour(ColourID colour_id)
{
    switch (colour_id) {
    case ColourID::EditBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::LayoutBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::PanelCaptionText:
        return get_base_dark_colour(DarkColourID::DARK_999);
    case ColourID::PanelCaptionBackground:
        return get_base_dark_colour(DarkColourID::DARK_300);
    case ColourID::ToolbarDivider:
        return get_base_dark_colour(DarkColourID::DARK_400);
    case ColourID::ToolbarFlatHotBackground:
        return get_base_dark_colour(DarkColourID::DARK_500);
    case ColourID::ToolbarFlatHotText:
        return get_base_dark_colour(DarkColourID::DARK_999);
    case ColourID::RebarBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::RebarBandBorder:
        return get_base_dark_colour(DarkColourID::DARK_400);
    case ColourID::SpinBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::SpinButtonBorder:
        return get_base_dark_colour(DarkColourID::DARK_400);
    case ColourID::SpinButtonArrow:
        return get_base_dark_colour(DarkColourID::DARK_999);
    case ColourID::SpinButtonBackground:
        return get_base_dark_colour(DarkColourID::DARK_000);
    case ColourID::SpinHotButtonBackground:
        return get_base_dark_colour(DarkColourID::DARK_300);
    case ColourID::SpinPressedButtonBackground:
        return get_base_dark_colour(DarkColourID::DARK_500);
    case ColourID::StatusBarBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::StatusBarText:
        return get_base_dark_colour(DarkColourID::DARK_999);
    case ColourID::StatusPaneBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::StatusPaneText:
        return get_base_dark_colour(DarkColourID::DARK_999);
    case ColourID::StatusPaneTopLine:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::TabControlBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::TabControlItemBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::TabControlItemText:
        return get_base_dark_colour(DarkColourID::DARK_999);
    case ColourID::TabControlItemBorder:
        return get_base_dark_colour(DarkColourID::DARK_000);
    case ColourID::TabControlActiveItemBackground:
        return get_base_dark_colour(DarkColourID::DARK_500);
    case ColourID::TabControlHotItemBackground:
        return get_base_dark_colour(DarkColourID::DARK_300);
    case ColourID::TabControlHotActiveItemBackground:
        return get_base_dark_colour(DarkColourID::DARK_500);
    case ColourID::TrackbarChannel:
        return get_base_dark_colour(DarkColourID::DARK_400);
    case ColourID::TrackbarThumb:
        if (const auto modern_colours = system_appearance_manager::get_modern_colours())
            return modern_colours->accent;

        return get_base_dark_colour(DarkColourID::DARK_600);
    case ColourID::TrackbarHotThumb:
        if (const auto modern_colours = system_appearance_manager::get_modern_colours())
            return modern_colours->accent_light_1;

        return get_base_dark_colour(DarkColourID::DARK_750);
    case ColourID::TrackbarDisabledThumb:
        return get_base_dark_colour(DarkColourID::DARK_400);
    case ColourID::TreeViewBackground:
        return get_base_dark_colour(DarkColourID::DARK_000);
    case ColourID::TreeViewText:
        return get_base_dark_colour(DarkColourID::DARK_999);
    case ColourID::VolumeChannelTopEdge:
        return get_base_dark_colour(DarkColourID::DARK_500);
    case ColourID::VolumeChannelBottomAndRightEdge:
        return get_base_dark_colour(DarkColourID::DARK_500);
    case ColourID::VolumePopupBackground:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case ColourID::VolumePopupBorder:
        return get_base_dark_colour(DarkColourID::DARK_300);
    case ColourID::VolumePopupText:
        return get_base_dark_colour(DarkColourID::DARK_999);
    default:
        uBugCheck();
    }
}

Gdiplus::Color get_dark_gdiplus_colour(ColourID colour_id)
{
    Gdiplus::Color colour;
    colour.SetFromCOLORREF(get_dark_colour(colour_id));
    return colour;
}

COLORREF get_colour(ColourID colour_id, bool is_dark)
{
    return is_dark ? get_dark_colour(colour_id) : get_light_colour(colour_id);
}

wil::unique_hbrush get_colour_brush(ColourID colour_id, bool is_dark)
{
    return is_dark ? get_dark_colour_brush(colour_id) : get_light_colour_brush(colour_id);
}

LazyResource<wil::unique_hbrush> get_colour_brush_lazy(ColourID colour_id, bool is_dark)
{
    return LazyResource<wil::unique_hbrush>{[colour_id, is_dark] { return get_colour_brush(colour_id, is_dark); }};
}

COLORREF get_dark_system_colour(int system_colour_id)
{
    // Unfortunately, these are hard-coded as there doesn't seem to be a simple
    // way to get a similar set of dark mode colours from Windows.
    switch (system_colour_id) {
    case COLOR_BTNTEXT:
    case COLOR_HIGHLIGHTTEXT:
    case COLOR_WINDOWTEXT:
        return get_base_dark_colour(DarkColourID::DARK_999);
    case COLOR_WINDOW:
        return get_base_dark_colour(DarkColourID::DARK_000);
    case COLOR_HIGHLIGHT:
        return get_base_dark_colour(DarkColourID::DARK_500);
    case COLOR_BTNFACE:
        return get_base_dark_colour(DarkColourID::DARK_200);
    case COLOR_WINDOWFRAME:
        return get_base_dark_colour(DarkColourID::DARK_600);
    default:
        return RGB(255, 0, 0);
    }
}

COLORREF get_system_colour(int system_colour_id, bool is_dark)
{
    if (is_dark)
        return get_dark_system_colour(system_colour_id);

    return GetSysColor(system_colour_id);
}

wil::unique_hbrush get_system_colour_brush(int system_colour_id, bool is_dark)
{
    if (is_dark)
        return wil::unique_hbrush(CreateSolidBrush(get_system_colour(system_colour_id, true)));

    // HBRUSHes returned by GetSysColorBrush don't need destroying, but doing so does no harm
    // according to the docs
    return wil::unique_hbrush(GetSysColorBrush(system_colour_id));
}

void draw_layout_background(HWND wnd, HDC dc)
{
    RECT rc{};
    GetClientRect(wnd, &rc);

    const auto brush = get_colour_brush(ColourID::LayoutBackground, colours::is_dark_mode_active());
    FillRect(dc, &rc, brush.get());
}

void handle_modern_background_paint(HWND wnd, HWND wnd_button, bool is_dark)
{
    const auto top_background_brush = get_system_colour_brush(COLOR_WINDOW, is_dark);
    const auto bottom_background_brush = get_system_colour_brush(COLOR_3DFACE, is_dark);
    uih::handle_modern_background_paint(wnd, wnd_button, top_background_brush.get(), bottom_background_brush.get());
}

} // namespace cui::dark
