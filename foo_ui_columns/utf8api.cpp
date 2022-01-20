#include "stdafx.h"
#include "utf8api.h"

namespace win32_helpers {

int status_bar_get_text_width(HWND wnd, const char* p_text)
{
    HFONT fnt = GetWindowFont(wnd);

    HDC dc = GetDC(wnd);
    HFONT fnt_old = SelectFont(dc, fnt);
    const auto width = (unsigned)uih::get_text_width(dc, p_text, strlen(p_text));
    SelectFont(dc, fnt_old);
    ReleaseDC(wnd, dc);

    return width;
}

} // namespace win32_helpers
