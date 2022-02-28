#include "stdafx.h"
#include "main_window.h"

COLORREF get_default_colour(colours::ColourID index, bool themed)
{
    switch (index) {
    case colours::COLOUR_TEXT:
        return GetSysColor(COLOR_WINDOWTEXT);
    case colours::COLOUR_SELECTED_TEXT:
        return GetSysColor(COLOR_HIGHLIGHTTEXT);
    case colours::COLOUR_BACK:
        return GetSysColor(COLOR_WINDOW);
    case colours::COLOUR_SELECTED_BACK:
        return GetSysColor(COLOR_HIGHLIGHT);
    case colours::COLOUR_FRAME:
        return GetSysColor(COLOR_WINDOWFRAME);
    case colours::COLOUR_SELECTED_BACK_NO_FOCUS:
        return GetSysColor(COLOR_BTNFACE);
    case colours::COLOUR_SELECTED_TEXT_NO_FOCUS:
        return GetSysColor(COLOR_BTNTEXT);
    default:
        return 0x0000FF;
    }
}

WINDOWPLACEMENT get_def_window_pos()
{
    WINDOWPLACEMENT rv{};
    rv.showCmd = SW_SHOWNORMAL;
    rv.length = sizeof(rv);
    rv.ptMaxPosition.x = -1;
    rv.ptMaxPosition.y = -1;
    rv.ptMinPosition.x = -1;
    rv.ptMinPosition.y = -1;
    rv.rcNormalPosition.right = 700;
    rv.rcNormalPosition.bottom = 500;
    return rv;
}

LOGFONT get_menu_font()
{
    LOGFONT font{};
    uGetMenuFont(&font);
    return font;
}

LOGFONT get_icon_font()
{
    LOGFONT font{};
    uGetIconFont(&font);
    return font;
}
