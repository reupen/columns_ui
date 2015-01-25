#ifndef COLUMNS_UTF8API_H
#define COLUMNS_UTF8API_H

#include "../../pfc/pfc.h"

#include <windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include "text_drawing.h"

namespace win32_helpers
{
	unsigned status_bar_get_text_width (HWND wnd, HTHEME thm, const char * p_text, bool b_customfont = false);
};


//BOOL uGetTextExtentExPoint(HDC dc, const char * text, int length, int max_width, LPINT max_chars, LPINT width_array, LPSIZE sz, unsigned & width_out, bool trunc = true);

BOOL uDrawPanelTitle(HDC dc, const RECT * rc_clip, const char * text, int len);

#endif