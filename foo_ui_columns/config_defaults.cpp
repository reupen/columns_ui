#include "pch.h"

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
