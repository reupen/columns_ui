#ifndef COLUMNS_UTF8API_H
#define COLUMNS_UTF8API_H

#include "../../pfc/pfc.h"

#include <windows.h>
#include <WindowsX.h>
#include <commctrl.h>
//#include "text_drawing.h"

namespace win32_helpers
{
	unsigned status_bar_get_text_width (HWND wnd, HTHEME thm, const char * p_text, bool b_customfont = false);
};

#endif