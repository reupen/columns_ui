#include "foo_ui_columns.h"

//#include "../utf8api/utf8api.h"

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

static bool check_colour_marks(const char * src, unsigned int len = -1)
{
	const char * ptr = src;
	while(*ptr && (unsigned)(ptr-src) < len)
	{
		if (*ptr==3)
		{
			return true;
		}
		ptr++;
	}
	return false;
}

void remove_color_marks(const char * src,pfc::string8 & out)//helper
{
	out.reset();
	while(*src)
	{
		if (*src==3)
		{
			src++;
			while(*src && *src!=3) src++;
			if (*src==3) src++;
		}
		else out.add_byte(*src++);
	}
}

unsigned get_trunc_len(const char * src, unsigned len)
{
	unsigned rv = len;

	const char * ptr = src;
	ptr += len-1;
	while (ptr>src && (*ptr == ' ' || *ptr == '.')) {ptr --;rv--;}
	return rv;
}




// here we depend on valid pointer anyway, no point using param_ stuff (or rather cant be bothered to make utf16/ansi => utf8 ones)
BOOL uGetTextExtentExPoint(HDC dc, const char * text, int length, int max_width, LPINT max_chars, LPINT width_array, LPSIZE sz, unsigned & width_out, bool trunc)
{
	const char * src = text;
	pfc::string8 temp;

	if (check_colour_marks(text, length))
	{ 
		remove_color_marks(text, temp);
		src = temp;
	}

		pfc::stringcvt::string_wide_from_utf8 text_w(src);
		if (GetTextExtentExPointW(dc, text_w.get_ptr(), text_w.length(), max_width, max_chars, width_array, sz))
		{
			pfc::stringcvt::string_utf8_from_wide w_utf8(text_w.get_ptr(), *max_chars);
			*max_chars = trunc ? get_trunc_len(w_utf8, w_utf8.length()) : w_utf8.length();
			width_out = ui_helpers::get_text_width_color(dc, w_utf8, *max_chars);
			return TRUE;
		}
		return FALSE;
}

/*
static void font_utf8_from_utf16(uLOGFONT * dst,const LOGFONTW * src)
{
	memcpy(dst,src,sizeof(*src)-sizeof(src->lfFaceName));
	strncpy_addnull(dst->lfFaceName,pfc::stringcvt::string_utf8_from_wide(src->lfFaceName),tabsize(dst->lfFaceName));
}

static void font_utf8_from_ansi(uLOGFONT * dst,const LOGFONTA * src)
{
	memcpy(dst,src,sizeof(*src)-sizeof(src->lfFaceName));
	strncpy_addnull(dst->lfFaceName,string_utf8_from_ansi(src->lfFaceName),tabsize(dst->lfFaceName));
}*/
/*
unsigned uGetMenuHeight()
{
	unsigned rv=0;
	
	if (IsUnicode())
	{
		NONCLIENTMETRICSW ncm;
		memset(&ncm, 0, sizeof(NONCLIENTMETRICSW));
		ncm.cbSize = sizeof(NONCLIENTMETRICSW);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
		rv = ncm.iMenuHeight;
	}
	else
	{
		NONCLIENTMETRICSA ncm;
		memset(&ncm, 0, sizeof(NONCLIENTMETRICSA));
		ncm.cbSize = sizeof(NONCLIENTMETRICSA);
		SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
		
		//		uLOGFONT lf_menu;
		//		font_utf8_from_utf16(lf_menu, ncm.lfMenuFont);
		
		rv = ncm.iMenuHeight;
	}
	return rv;
}*/

BOOL uDrawPanelTitle(HDC dc, const RECT * rc_clip, const char * text, int len)
{
//	COLORREF cr_back = GetSysColor(is_winxp_or_newer() ? COLOR_MENUBAR : COLOR_3DFACE);
	COLORREF cr_fore=GetSysColor(COLOR_MENUTEXT);
	COLORREF cr_line=GetSysColor(COLOR_3DSHADOW);

	{
		/*	__int64 start,end;
			start=get_timestamp();
			end=get_timestamp();
			console::info(pfc::string_printf("%u",(unsigned)(end-start)));*/
//		HBRUSH brush = CreateSolidBrush(cr_back);
//		FillRect(dc, rc_clip, brush);
//		DeleteObject(brush);
	}
	
	{
		
//		if (fnt_menu)
		{
			SetBkMode(dc,TRANSPARENT);
			SetTextColor(dc, cr_fore);

			unsigned height = uGetTextHeight(dc);
			int extra = (rc_clip->bottom - rc_clip->top - height -1)/2;

	//		HFONT old = SelectFont(dc, fnt_menu);
			uExtTextOut(dc, 5, extra, ETO_CLIPPED, rc_clip, text, len, 0);
	//		SelectFont(dc, old);

			
	/*		HPEN pen = CreatePen(PS_SOLID, 1, cr_line);
			HPEN pen_old = (HPEN)SelectObject(dc, pen);
			

			MoveToEx(dc, rc_clip->left, rc_clip->bottom-1, 0);
			LineTo(dc, rc_clip->left-0, rc_clip->top);

			MoveToEx(dc, rc_clip->left, rc_clip->top, 0);
			LineTo(dc, rc_clip->right-1, rc_clip->top);
			MoveToEx(dc, rc_clip->right-1, rc_clip->top, 0);
			LineTo(dc, rc_clip->right-1, rc_clip->bottom-0);*/

	/*		MoveToEx(dc, rc_clip->left, rc_clip->bottom-1, 0);
			LineTo(dc, rc_clip->right-0, rc_clip->bottom-1);

			SelectObject(dc, pen_old);
			DeleteObject(pen);*/
			

			return TRUE;
		}
	}
	return FALSE;
}

//#include <tmschema.h>

namespace win32_helpers
{
	unsigned status_bar_get_text_width (HWND wnd, HTHEME thm, const char * p_text, bool b_customfont)
	{
		HFONT fnt = NULL;
		bool b_release_fnt = false;
		unsigned rv = NULL;

		DLLVERSIONINFO2 dvi;

		//uxtheme_api_ptr p_uxtheme;

		bool b_themed = thm != NULL;//p_uxtheme->IsThemeActive() && p_uxtheme->IsAppThemed();

		HRESULT hr = get_comctl32_version(dvi);

		if (!(SUCCEEDED(hr) && dvi.info1.dwMajorVersion == 6))
			//uxtheme_handle::g_create(p_uxtheme);
			b_themed=false;


		if (b_customfont || !b_themed)
		{
			fnt = (HFONT)SendMessage(wnd, WM_GETFONT, NULL, NULL);
			b_release_fnt =false;
		}
		else
		{
			LOGFONT lf;

			//bool b_got_theme_font = false;

			if (b_themed && SUCCEEDED(GetThemeFont(thm, NULL, NULL, NULL, TMT_FONT, &lf)))
			{
				fnt = CreateFontIndirect(&lf);
				b_release_fnt = true;
			}
			else
			{
				fnt = (HFONT)SendMessage(wnd, WM_GETFONT, NULL, NULL);
				b_release_fnt =false;
			}
#if 0
			if (!b_got_theme_font)
			{

				NONCLIENTMETRICS ncm;
				memset(&ncm, 0, sizeof(NONCLIENTMETRICS));
				ncm.cbSize = sizeof(NONCLIENTMETRICS);
				SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
				lf = ncm.lfStatusFont;
			}
			fnt = CreateFontIndirect(&lf);
			b_release_fnt = true;
#endif
		}

		HDC dc = GetDC(wnd);
		HFONT fnt_old = SelectFont(dc, fnt);
		rv = (unsigned)ui_helpers::get_text_width(dc, p_text, strlen(p_text));
		SelectFont(dc, fnt_old);
		ReleaseDC(wnd, dc);

		if (b_release_fnt)
			DeleteFont(fnt);
		return rv;
	}
};