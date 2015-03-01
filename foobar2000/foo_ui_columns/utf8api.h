#ifndef COLUMNS_UTF8API_H
#define COLUMNS_UTF8API_H

/*!
 * \file utf8api.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Status bar text drawing functions only
 */
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