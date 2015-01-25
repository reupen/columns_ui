#ifndef _UI_EXTENSION_UTF8API_H_
#define _UI_EXTENSION_UTF8API_H_

/**
 * \file win32_helpers.h
 * \brief UTF-8 Win32 API wrappers
 * \author musicmusic
 */

#include <windows.h>
#include <WindowsX.h>
#include <commctrl.h>

#ifndef uT
#define uT(x) (pfc::stringcvt::string_os_from_utf8(x).get_ptr())
#define uTS(x,s) (pfc::stringcvt::string_os_from_utf8(x,s).get_ptr())
#endif
#define Tu(x) (pfc::stringcvt::string_utf8_from_os(x).get_ptr())
#define TSu(x,s) (pfc::stringcvt::string_utf8_from_os(x,s).get_ptr())

typedef HDITEMA uHDITEM;
typedef TOOLINFOA uTOOLINFO;
typedef REBARBANDINFOA uREBARBANDINFO;
typedef LOGFONTA uLOGFONT;

#define RECT_CX(rc) (rc.right-rc.left)
#define RECT_CY(rc) (rc.bottom-rc.top)

int uHeader_InsertItem(HWND wnd, int n, uHDITEM * hdi, bool insert = true); //set insert to false to set the item instead
int uHeader_SetItemText(HWND wnd, int n, const char * text);
int uHeader_SetItemWidth(HWND wnd, int n, UINT cx);
BOOL uToolTip_AddTool(HWND wnd, uTOOLINFO * ti, bool update = false);
BOOL uRebar_InsertItem(HWND wnd, int n, uREBARBANDINFO * rbbi, bool insert = true); //set insert to false to set the item instead

BOOL uTabCtrl_InsertItemText(HWND wnd, int idx, const char * text, bool insert = true); //fixes '&' characters also, set insert to false to set the item instead

inline void GetRelativeRect(HWND wnd, HWND wnd_parent, RECT * rc)//get rect of wnd in wnd_parent coordinates
{
	GetWindowRect(wnd, rc);
	MapWindowPoints(HWND_DESKTOP, wnd_parent, (LPPOINT)rc,2);
}

BOOL uComboBox_GetText(HWND combo,UINT index,pfc::string_base & out);
BOOL uComboBox_SelectString(HWND combo,const char * src);
BOOL uStatus_SetText(HWND wnd,int part,const char * text);

HFONT uCreateIconFont(); 
HFONT uCreateMenuFont(bool vertical = false);

void uGetMenuFont(LOGFONT * p_lf);
void uGetIconFont(LOGFONT * p_lf);

struct logfont_os_menu : public LOGFONT
{
	logfont_os_menu()
	{
		uGetMenuFont(this);
	}
};

struct logfont_os_icon : public LOGFONT
{
	logfont_os_icon()
	{
		uGetIconFont(this);
	}
};

inline void GetMessagePos(LPPOINT pt)
{
	DWORD mp = GetMessagePos();
	pt->x =  GET_X_LPARAM(mp);
	pt->y =  GET_Y_LPARAM(mp);
}


BOOL uFormatMessage(DWORD dw_error, pfc::string_base & out);

inline BOOL uGetLastErrorMessage(pfc::string_base & out)
{
	return uFormatMessage(GetLastError(), out);
}

DWORD uGetClassLong(HWND wnd, int index);
DWORD uSetClassLong(HWND wnd, int index, long new_long);

HWND uFindParentPopup(HWND wnd_child);

#if(WINVER >= 0x0500)
typedef MONITORINFOEXA uMONITORINFOEX, *uLPMONITORINFOEX;

BOOL uGetMonitorInfo(HMONITOR monitor, LPMONITORINFO lpmi);
HWND uRecursiveChildWindowFromPoint(HWND parent, POINT pt_parent);//pt_parent is in parent window coordinates!
#endif

namespace win32_helpers {
	void send_message_to_all_children(HWND wnd_parent, UINT msg, WPARAM wp, LPARAM lp);
	void send_message_to_direct_children(HWND wnd_parent, UINT msg, WPARAM wp, LPARAM lp);
	int message_box(HWND wnd,const TCHAR * text,const TCHAR * caption,UINT type);
	BOOL tooltip_add_tool(HWND wnd, TOOLINFO * ti, bool update = false);
};

//for compatibility only (namely fcs files)

void convert_logfont_utf8_to_os(const uLOGFONT & p_src, LOGFONT & p_dst);
void convert_logfont_os_to_utf8(const LOGFONT & p_src, uLOGFONT & p_dst);

struct logfont_os_from_utf8 : public LOGFONT
{
	logfont_os_from_utf8(const uLOGFONT & p_logfont)
	{
		convert_logfont_utf8_to_os(p_logfont, *this);
	}
};

struct logfont_utf8_from_os : public uLOGFONT
{
	logfont_utf8_from_os(const LOGFONT & p_logfont)
	{
		convert_logfont_os_to_utf8(p_logfont, *this);
	}
};

#endif