#include "ui_extension.h"

class param_utf16_from_utf8 : public pfc::stringcvt::string_wide_from_utf8
{
	bool is_null;
	WORD low_word;
public:
	param_utf16_from_utf8(const char * p) : 
		is_null(p==0), 
		low_word( HIWORD((DWORD)p)==0 ? LOWORD((DWORD)p) : 0),
		pfc::stringcvt::string_wide_from_utf8( p && HIWORD((DWORD)p)!=0 ?p:"") 
		{}
	inline operator const WCHAR *()
	{
		return get_ptr();
	}
	const WCHAR * get_ptr()
	{
		return low_word ? (const WCHAR*)(DWORD)low_word : is_null ? 0 : pfc::stringcvt::string_wide_from_utf8::get_ptr();
	}
	
};

class param_ansi_from_utf8 : public pfc::stringcvt::string_ansi_from_utf8
{
	bool is_null;
	WORD low_word;
public:
	param_ansi_from_utf8(const char * p) : 
		is_null(p==0), 
		low_word( HIWORD((DWORD)p)==0 ? LOWORD((DWORD)p) : 0),
		pfc::stringcvt::string_ansi_from_utf8( p && HIWORD((DWORD)p)!=0 ?p:"") 
		{}
	inline operator const char *()
	{
		return get_ptr();
	}
	const char * get_ptr()
	{
		return low_word ? (const char*)(DWORD)low_word : is_null ? 0 : pfc::stringcvt::string_ansi_from_utf8::get_ptr();
	}
	
};

namespace win32_helpers {
	void send_message_to_all_children(HWND wnd_parent, UINT msg, WPARAM wp, LPARAM lp)
	{
		HWND wnd = GetWindow(wnd_parent, GW_CHILD);
		if (wnd)
			do
			{
				send_message_to_all_children ( wnd, msg, wp, lp );
				SendMessage ( wnd, msg, wp, lp );
			}
			while (wnd = GetWindow(wnd, GW_HWNDNEXT));
	}
	void send_message_to_direct_children(HWND wnd_parent, UINT msg, WPARAM wp, LPARAM lp)
	{
		HWND wnd = GetWindow(wnd_parent, GW_CHILD);
		if (wnd)
			do
			{
				SendMessage ( wnd, msg, wp, lp );
			}
			while (wnd = GetWindow(wnd, GW_HWNDNEXT));
	}
	int message_box(HWND wnd,const TCHAR * text,const TCHAR * caption,UINT type)
	{
		modal_dialog_scope scope(wnd);
		return MessageBox(wnd,text,caption,type);
	}
};


int uHeader_SetItemWidth(HWND wnd, int n, UINT cx)
{
	uHDITEM hdi;
	memset(&hdi, 0, sizeof(hdi));
	hdi.mask = HDI_WIDTH;
	hdi.cxy = cx;
	return uHeader_InsertItem(wnd, n, &hdi, false);
}
int uHeader_SetItemText(HWND wnd, int n, const char * text)
{
	uHDITEM hdi;
	memset(&hdi, 0, sizeof(hdi));
	hdi.mask = HDI_TEXT;
	hdi.cchTextMax = NULL;
	hdi.pszText = const_cast<char*>(text);
	return uHeader_InsertItem(wnd, n, &hdi, false);
}

int uHeader_InsertItem(HWND wnd, int n, uHDITEM * hdi, bool insert)
{
	if (IsUnicode())
	{
		param_utf16_from_utf8 text((hdi->mask & HDI_TEXT) ? hdi->pszText : 0);
		HDITEMW hdw;
		hdw.cchTextMax = text.length();
		hdw.cxy = hdi->cxy;
		hdw.fmt = hdi->fmt;
		hdw.hbm = hdi->hbm;
		hdw.iImage = hdi->iImage;
		hdw.iOrder = hdi->iOrder;
		hdw.lParam = hdi->lParam;
		hdw.mask = hdi->mask;
		hdw.pszText = const_cast<WCHAR *>(text.get_ptr());

		return uSendMessage(wnd, insert ? HDM_INSERTITEMW : HDM_SETITEMW, n, (LPARAM)&hdw);
		
	}
	else
	{
		param_ansi_from_utf8 text((hdi->mask & HDI_TEXT) ? hdi->pszText : 0);

		HDITEMA hda;
		hda.cchTextMax = hdi->cchTextMax;
		hda.cxy = hdi->cxy;
		hda.fmt = hdi->fmt;
		hda.hbm = hdi->hbm;
		hda.iImage = hdi->iImage;
		hda.iOrder = hdi->iOrder;
		hda.lParam = hdi->lParam;
		hda.mask = hdi->mask;
		hda.pszText = const_cast<char *>(text.get_ptr());

		return uSendMessage(wnd, insert ? HDM_INSERTITEMA : HDM_SETITEMA, n, (LPARAM)&hda);
	}
}

BOOL uRebar_InsertItem(HWND wnd, int n, uREBARBANDINFO * rbbi, bool insert)
{
	BOOL rv = FALSE;

	{
		param_utf16_from_utf8 text((rbbi->fMask & RBBIM_TEXT) ? rbbi->lpText : 0);

		REBARBANDINFOW  rbw;
		rbw.cbSize = REBARBANDINFOW_V6_SIZE;

		rbw.fMask = rbbi->fMask;
		rbw.fStyle = rbbi->fStyle;
		rbw.clrFore = rbbi->clrFore;
		rbw.clrBack = rbbi->clrBack;
		rbw.lpText = const_cast<WCHAR *>(text.get_ptr());;
		rbw.cch = rbbi->cch;
		rbw.iImage = rbbi->iImage;
		rbw.hwndChild = rbbi->hwndChild;
		rbw.cxMinChild = rbbi->cxMinChild;
		rbw.cyMinChild = rbbi->cyMinChild;
		rbw.cx = rbbi->cx;
		rbw.hbmBack = rbbi->hbmBack;
		rbw.wID = rbbi->wID;
	#if (_WIN32_IE >= 0x0400) //we should check size of structure instead so keeping compatibility is possible, but whatever
		rbw.cyChild = rbbi->cyChild;
		rbw.cyMaxChild = rbbi->cyMaxChild;
		rbw.cyIntegral = rbbi->cyIntegral;
		rbw.cxIdeal = rbbi->cxIdeal;
		rbw.lParam = rbbi->lParam;
		rbw.cxHeader = rbbi->cxHeader;
	#endif
		
		rv = uSendMessage(wnd, insert ? RB_INSERTBANDW : RB_SETBANDINFOW, n, (LPARAM)&rbw);

	}
	return rv;
}

#ifdef UNICODE
#define param_os_from_utf8 param_utf16_from_utf8
#else
#define param_os_from_utf8 param_ansi_from_utf8
#endif

namespace win32_helpers {
BOOL tooltip_add_tool(HWND wnd, TOOLINFO * ti, bool update)
{
	BOOL rv = FALSE;
	rv = SendMessage(wnd, update ? TTM_UPDATETIPTEXT : TTM_ADDTOOL, 0, (LPARAM)ti);
	return rv;
}
}

BOOL uToolTip_AddTool(HWND wnd, uTOOLINFO * ti, bool update)
{
	BOOL rv = FALSE;
	if (IsUnicode())
	{
		param_utf16_from_utf8 text(ti->lpszText);
		TOOLINFOW tiw;
		
		tiw.cbSize = sizeof(tiw);
		tiw.uFlags = ti->uFlags;
		tiw.hwnd = ti->hwnd;
		tiw.uId = ti->uId;
		tiw.rect = ti->rect;
		tiw.hinst = ti->hinst;
		tiw.lParam = ti->lParam;
		tiw.lpszText = const_cast<WCHAR*>(text.get_ptr());

		rv = uSendMessage(wnd, update ? TTM_UPDATETIPTEXTW : TTM_ADDTOOLW, 0, (LPARAM)  &tiw);
	}
	else
	{
		param_ansi_from_utf8 text(ti->lpszText);
		TOOLINFOA tia;
		
		tia.cbSize = sizeof(tia);
		tia.uFlags = ti->uFlags;
		tia.hwnd = ti->hwnd;
		tia.uId = ti->uId;
		tia.rect = ti->rect;
		tia.hinst = ti->hinst;
		tia.lParam = ti->lParam;
		tia.lpszText = const_cast<char*>(text.get_ptr());

		rv = uSendMessage(wnd, update ? TTM_UPDATETIPTEXTA : TTM_ADDTOOLA, 0, (LPARAM)  &tia);
	}
	return rv;
}

BOOL uTabCtrl_InsertItemText(HWND wnd, int idx, const char * text, bool insert)
{
	pfc::string8 temp2;
	uTCITEM tabs;
	memset(&tabs, 0, sizeof(uTCITEM));
	tabs.mask = TCIF_TEXT;
	uFixAmpersandChars_v2(text, temp2);
	tabs.pszText = const_cast<char *>(temp2.get_ptr());
	return insert ? uTabCtrl_InsertItem(wnd, idx, &tabs) : uTabCtrl_SetItem(wnd, idx, &tabs);
}

BOOL uComboBox_GetText(HWND combo,UINT index,pfc::string_base & out)
{
	unsigned len = uSendMessage(combo,CB_GETLBTEXTLEN,index,0);
	if (len==CB_ERR || len>16*1024*1024) return FALSE;
	if (len==0) {out.reset();return TRUE;}

	if (IsUnicode())
	{
		pfc::array_t<WCHAR> temp;
		temp.set_size(len+1);
		if (temp.get_ptr()==0) return FALSE;
		temp.fill(0);
		len = uSendMessage(combo,CB_GETLBTEXT,index,(LPARAM)temp.get_ptr());
		if (len==CB_ERR) return false;
		out.set_string(pfc::stringcvt::string_utf8_from_wide(temp.get_ptr()));
	}
	else
	{
		pfc::array_t<char> temp;
		temp.set_size(len+1);
		if (temp.get_ptr()==0) return FALSE;
		temp.fill(0);
		len = uSendMessage(combo,CB_GETLBTEXT,index,(LPARAM)temp.get_ptr());
		if (len==CB_ERR) return false;
		out.set_string(pfc::stringcvt::string_utf8_from_ansi(temp.get_ptr()));
	}
	return TRUE;
}

BOOL uComboBox_SelectString(HWND combo,const char * src)
{
	unsigned idx = CB_ERR;
	idx = uSendMessageText(combo,CB_FINDSTRINGEXACT,-1,src);
	if (idx==CB_ERR) return false;
	uSendMessage(combo,CB_SETCURSEL,idx,0);
	return TRUE;
}


BOOL uStatus_SetText(HWND wnd,int part,const char * text)
{
	BOOL rv = 0;

	if (IsUnicode())
	{
		rv = uSendMessage(wnd,SB_SETTEXTW,part,(LPARAM)(param_utf16_from_utf8(text).get_ptr()));
	}
	else
	{
		rv = uSendMessage(wnd,SB_SETTEXTA,part,(LPARAM)(param_ansi_from_utf8(text).get_ptr()));
	}

//	rv = uSendMessageText(wnd, IsUnicode() ? SB_SETTEXTW : SB_SETTEXTA, part, text);
	return rv;
}

HFONT uCreateIconFont()
{
	HFONT fnt_menu = 0;
	
	if (IsUnicode())
	{
		LOGFONTW lf;
		memset(&lf, 0, sizeof(LOGFONTW));
		SystemParametersInfoW(SPI_GETICONTITLELOGFONT, 0, &lf, 0);
		
		fnt_menu = CreateFontIndirectW(&lf);
	}
	else
	{
		LOGFONTA lf;
		memset(&lf, 0, sizeof(LOGFONTA));
		SystemParametersInfoA(SPI_GETICONTITLELOGFONT, 0, &lf, 0);
		
		fnt_menu = CreateFontIndirectA(&lf);
	}
	
	return fnt_menu;
	
}

HFONT uCreateMenuFont(bool vertical)
{
	HFONT fnt_menu = 0;
	
	if (IsUnicode())
	{
		NONCLIENTMETRICSW ncm;
		memset(&ncm, 0, sizeof(NONCLIENTMETRICSW));
		ncm.cbSize = sizeof(NONCLIENTMETRICSW);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
		
		if (vertical)
		{
			ncm.lfMenuFont.lfEscapement = 900;
			ncm.lfMenuFont.lfOrientation = 900;
		}
		
		fnt_menu = CreateFontIndirectW(&ncm.lfMenuFont);
	}
	else
	{
		NONCLIENTMETRICSA ncm;
		memset(&ncm, 0, sizeof(NONCLIENTMETRICSA));
		ncm.cbSize = sizeof(NONCLIENTMETRICSA);
		SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
		
		if (vertical)
		{
			ncm.lfMenuFont.lfEscapement = 900;
			ncm.lfMenuFont.lfOrientation = 900;
		}
		
		fnt_menu = CreateFontIndirectA(&ncm.lfMenuFont);
	}
	
	return fnt_menu;
}

void uGetMenuFont(LOGFONT * p_lf)
{
	NONCLIENTMETRICS ncm;
	memset(&ncm, 0, sizeof(NONCLIENTMETRICS));
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

	*p_lf = ncm.lfMenuFont;
}

void uGetIconFont(LOGFONT * p_lf)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, 0, p_lf, 0);
}


BOOL uFormatMessage(DWORD dw_error, pfc::string_base & out)
{
	BOOL rv = FALSE;
	if (IsUnicode())
	{
		pfc::array_t<WCHAR> buffer;
		buffer.set_size(256);
		//MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, dw_error, 0, buffer.get_ptr(), buffer.get_size(), 0))
		{
			out.set_string(pfc::stringcvt::string_utf8_from_wide(buffer.get_ptr()));
			rv = TRUE;
		}
	}
	else
	{
		pfc::array_t<char> buffer;
		buffer.set_size(256);
		if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, dw_error, 0, buffer.get_ptr(), buffer.get_size(), 0))
		{
			//is it actually ANSI ?
			out.set_string(pfc::stringcvt::string_utf8_from_ansi(buffer.get_ptr()));
			rv = TRUE;
		}
	}
	return rv;
}


DWORD uSetClassLong(HWND wnd, int index, long new_long)
{
	if (IsUnicode())
	{
		return SetClassLongW(wnd,index, new_long);
	} 
	else
	{
		return SetClassLongA(wnd,index, new_long);
	} 
}

DWORD uGetClassLong(HWND wnd, int index)
{
	if (IsUnicode())
	{
		return GetClassLongW(wnd,index);
	} 
	else
	{
		return GetClassLongA(wnd,index);
	} 
}

#if(WINVER >= 0x0500)
HWND uRecursiveChildWindowFromPoint(HWND parent, POINT pt_parent)
{
	HWND wnd = RealChildWindowFromPoint(parent, pt_parent);
	if (wnd && wnd != parent)
	{
		HWND wnd_last = wnd;
		POINT pt = pt_parent;
		MapWindowPoints(parent, wnd_last, &pt, 1);
		for (;;)
		{
			wnd = RealChildWindowFromPoint(wnd_last, pt);
			if (!wnd) return 0;
			if (wnd == wnd_last) return wnd;
			MapWindowPoints(wnd_last, wnd, &pt, 1);
			wnd_last = wnd;
			RECT rc;
			GetClientRect(wnd_last, &rc);
			if (!PtInRect(&rc, pt)) return wnd_last;
		}
	}
	return 0;
}



BOOL uGetMonitorInfo(HMONITOR monitor, LPMONITORINFO lpmi)
{
	BOOL rv = FALSE;
	if (IsUnicode())
	{
		if (lpmi->cbSize == sizeof(uMONITORINFOEX))
		{
			uLPMONITORINFOEX lpmiex = (uLPMONITORINFOEX)lpmi;

			MONITORINFOEXW mi;
			memset (&mi, 0, sizeof(MONITORINFOEXW));
			mi.cbSize = sizeof(MONITORINFOEXW);

			rv = GetMonitorInfoW(monitor, &mi);

			lpmi->rcMonitor = mi.rcMonitor;
			lpmi->rcWork = mi.rcWork;
			lpmi->dwFlags = mi.dwFlags;

			pfc::stringcvt::string_utf8_from_wide temp(mi.szDevice,CCHDEVICENAME);
			strcpy_utf8_truncate(temp, lpmiex->szDevice, CCHDEVICENAME);

			//unsafe
		/*	unsigned len = wcslen_max(mi.szDevice,CCHDEVICENAME);
			convert_utf16_to_utf8(mi.szDevice,lpmiex->szDevice,len);*/
		}
		else
			rv = GetMonitorInfoW(monitor, lpmi);
	}
	else
	{
		if (lpmi->cbSize == sizeof(uMONITORINFOEX))
		{
			uLPMONITORINFOEX lpmiex = (uLPMONITORINFOEX)lpmi;

			MONITORINFOEXA mi;
			memset (&mi, 0, sizeof(MONITORINFOEXA));
			mi.cbSize = sizeof(MONITORINFOEXA);

			rv = GetMonitorInfoA(monitor, &mi);

			lpmi->rcMonitor = mi.rcMonitor;
			lpmi->rcWork = mi.rcWork;
			lpmi->dwFlags = mi.dwFlags;

			pfc::stringcvt::string_utf8_from_ansi temp(mi.szDevice,CCHDEVICENAME);
			pfc::strcpy_utf8_truncate(temp, lpmiex->szDevice, CCHDEVICENAME);

			/*unsigned len = strlen_max(mi.szDevice,CCHDEVICENAME);
			convert_ansi_to_utf8(mi.szDevice,lpmiex->szDevice,len);*/
		}
		else
			rv = GetMonitorInfoA(monitor, lpmi);
	}
	return rv;
}

void strncpy_addnull(char * dest,const char * src,int max)
{
	int n;
	for(n=0;n<max-1 && src[n];n++)
		dest[n] = src[n];
	dest[n] = 0;
}

void wcsncpy_addnull(WCHAR * dest,const WCHAR * src,int max)
{
	int n;
	for(n=0;n<max-1 && src[n];n++)
		dest[n] = src[n];
	dest[n] = 0;
}

#ifdef UNICODE
#define tcsncpy_addnull wcsncpy_addnull
#else
#define tcsncpy_addnull strncpy_addnull
#endif

typedef LOGFONTA uLOGFONT;

void convert_logfont_utf8_to_os(const uLOGFONT & p_src, LOGFONT & p_dst)
{
	p_dst.lfHeight = p_src.lfHeight; 
	p_dst.lfWidth = p_src.lfWidth; 
	p_dst.lfEscapement = p_src.lfEscapement; 
	p_dst.lfOrientation = p_src.lfOrientation; 
	p_dst.lfWeight = p_src.lfWeight; 
	p_dst.lfItalic = p_src.lfItalic; 
	p_dst.lfUnderline = p_src.lfUnderline; 
	p_dst.lfStrikeOut = p_src.lfStrikeOut; 
	p_dst.lfCharSet = p_src.lfCharSet; 
	p_dst.lfOutPrecision = p_src.lfOutPrecision; 
	p_dst.lfClipPrecision = p_src.lfClipPrecision; 
	p_dst.lfQuality = p_src.lfQuality; 
	p_dst.lfPitchAndFamily = p_src.lfPitchAndFamily; 
	tcsncpy_addnull(p_dst.lfFaceName,pfc::stringcvt::string_os_from_utf8(p_src.lfFaceName),tabsize(p_dst.lfFaceName));
}

void convert_logfont_os_to_utf8(const LOGFONT & p_src, uLOGFONT & p_dst)
{
	p_dst.lfHeight = p_src.lfHeight; 
	p_dst.lfWidth = p_src.lfWidth; 
	p_dst.lfEscapement = p_src.lfEscapement; 
	p_dst.lfOrientation = p_src.lfOrientation; 
	p_dst.lfWeight = p_src.lfWeight; 
	p_dst.lfItalic = p_src.lfItalic; 
	p_dst.lfUnderline = p_src.lfUnderline; 
	p_dst.lfStrikeOut = p_src.lfStrikeOut; 
	p_dst.lfCharSet = p_src.lfCharSet; 
	p_dst.lfOutPrecision = p_src.lfOutPrecision; 
	p_dst.lfClipPrecision = p_src.lfClipPrecision; 
	p_dst.lfQuality = p_src.lfQuality; 
	p_dst.lfPitchAndFamily = p_src.lfPitchAndFamily; 
	strncpy_addnull(p_dst.lfFaceName,pfc::stringcvt::string_utf8_from_os(p_src.lfFaceName),tabsize(p_dst.lfFaceName));
}

#endif
