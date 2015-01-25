#include "stdafx.h"

#if 0

class uxtheme_manager
{
public:
	uxtheme_manager()
		: m_refcount(0), m_uxtheme(NULL), m_failed_once(false)
	{};
protected:
	uxtheme_handle * add_reference()
	{
		if (!m_refcount)
		{
			assert (!m_uxtheme);
			if (!m_failed_once)
			{
				HINSTANCE inst = LoadLibrary(_T("uxtheme.dll"));
				m_failed_once = inst==NULL;
				if (inst)
				{
					m_uxtheme = new uxtheme_handle(inst);
				}
			}
		}
		if (m_uxtheme!=0)
			m_refcount++;
		return m_uxtheme;
	}
	void release()
	{
		if (!--m_refcount && m_uxtheme)
		{
			delete m_uxtheme;
			m_uxtheme=NULL;
		}
	}
private:
	uxtheme_handle * m_uxtheme;
	size_t m_refcount;
	bool m_failed_once;

	friend class uxtheme_ptr;
} g_uxtheme_manager;

uxtheme_ptr::uxtheme_ptr() : m_uxtheme(NULL) {};
uxtheme_ptr::uxtheme_ptr(const uxtheme_ptr & p_source)
{
	if (p_source.is_valid())
		m_uxtheme = g_uxtheme_manager.add_reference();
	else m_uxtheme = NULL;
}

uxtheme_ptr::~uxtheme_ptr() {release();}

bool uxtheme_ptr::is_valid()const
{
	return m_uxtheme!=0;
}

void uxtheme_ptr::release()
{
	if (is_valid())
	{
		g_uxtheme_manager.release();
		m_uxtheme=NULL;
	}
}

bool uxtheme_ptr::load()
{
	if (!is_valid())
	{
		m_uxtheme=g_uxtheme_manager.add_reference();
	}
	return m_uxtheme !=0;
}

uxtheme_handle::uxtheme_handle(HINSTANCE inst) : inst_uxtheme(inst),pOpenThemeData(0), 
	pCloseThemeData(0), pDrawThemeBackground(0), pDrawThemeText(0), 
	pGetThemeBackgroundContentRect(0), pGetThemeBackgroundExtent(0), pGetThemePartSize(0), 
	pGetThemeTextExtent(0), pGetThemeTextMetrics(0), pGetThemeBackgroundRegion(0), 
	pHitTestThemeBackground(0), pDrawThemeEdge(0), pDrawThemeIcon(0), pIsThemePartDefined(0),
	pIsThemeBackgroundPartiallyTransparent(0), pGetThemeColor(0), pGetThemeMetric(0), 
	pGetThemeString(0), pGetThemeBool(0), pGetThemeInt(0), pGetThemeEnumValue(0), 
	pGetThemePosition(0), pGetThemeFont(0), pGetThemeRect(0), pGetThemeMargins(0), 
	pGetThemeIntList(0), pGetThemePropertyOrigin(0), pSetWindowTheme(0), 
	pGetThemeFilename(0), pGetThemeSysColor(0), pGetThemeSysColorBrush(0), 
	pGetThemeSysBool(0), pGetThemeSysSize(0), pGetThemeSysFont(0), pGetThemeSysString(0), 
	pGetThemeSysInt(0), pIsThemeActive(0), pIsAppThemed(0), pGetWindowTheme(0), 
	pEnableThemeDialogTexture(0), pIsThemeDialogTextureEnabled(0), pGetThemeAppProperties(0), 
	pSetThemeAppProperties(0), pGetCurrentThemeName(0), pGetThemeDocumentationProperty(0), 
	pDrawThemeParentBackground(0), pEnableTheming(0), pDrawThemeBackgroundEx(0)	
{
};

uxtheme_handle::~uxtheme_handle()
{
	if (inst_uxtheme)
		FreeLibrary(inst_uxtheme);
};

bool uxtheme_handle::g_create(uxtheme_ptr & p_out)
{
	return p_out.load();
}



HTHEME uxtheme_handle::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	HTHEME rv = 0;
	if (!pOpenThemeData)
		pOpenThemeData = (OpenThemeDataProc)GetProcAddress(inst_uxtheme, "OpenThemeData");
	if (pOpenThemeData)
		rv = pOpenThemeData(hwnd, pszClassList);
	return rv;
}

HRESULT uxtheme_handle::CloseThemeData(HTHEME hTheme)
{
	HRESULT rv = E_FAIL;
	if (!pCloseThemeData)
		pCloseThemeData = (CloseThemeDataProc)GetProcAddress(inst_uxtheme, "CloseThemeData");
	if (pCloseThemeData)
		rv = pCloseThemeData(hTheme);
	return rv;
}

HRESULT uxtheme_handle::DrawThemeBackground(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect)
{
	HRESULT rv = E_FAIL;
	if (!pDrawThemeBackground)
		pDrawThemeBackground = (DrawThemeBackgroundProc)GetProcAddress(inst_uxtheme, "DrawThemeBackground");
	if (pDrawThemeBackground)
		rv = pDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
	return rv;
}

HRESULT uxtheme_handle::DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
	int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
	DWORD dwTextFlags2, const RECT *pRect)
{
	HRESULT rv = E_FAIL;
	if (!pDrawThemeText)
		pDrawThemeText = (DrawThemeTextProc)GetProcAddress(inst_uxtheme, "DrawThemeText");
	if (pDrawThemeText)
		rv = pDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
	return rv;
}

HRESULT uxtheme_handle::GetThemeBackgroundContentRect(HTHEME hTheme, OPTIONAL HDC hdc, 
	int iPartId, int iStateId,  const RECT *pBoundingRect, 
	OUT RECT *pContentRect)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeBackgroundContentRect)
		pGetThemeBackgroundContentRect = (GetThemeBackgroundContentRectProc)GetProcAddress(inst_uxtheme, "GetThemeBackgroundContentRect");
	if (pGetThemeBackgroundContentRect)
		rv = pGetThemeBackgroundContentRect(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);
	return rv;
}

HRESULT uxtheme_handle::GetThemeBackgroundExtent(HTHEME hTheme, OPTIONAL HDC hdc,
	int iPartId, int iStateId, const RECT *pContentRect, 
	OUT RECT *pExtentRect)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeBackgroundExtent)
		pGetThemeBackgroundExtent = (GetThemeBackgroundExtentProc)GetProcAddress(inst_uxtheme, "GetThemeBackgroundExtent");
	if (pGetThemeBackgroundExtent)
		rv = pGetThemeBackgroundExtent(hTheme, hdc, iPartId, iStateId, pContentRect, pExtentRect);
	return rv;
}

HRESULT uxtheme_handle::GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
	OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemePartSize)
		pGetThemePartSize = (GetThemePartSizeProc)GetProcAddress(inst_uxtheme, "GetThemePartSize");
	if (pGetThemePartSize)
		rv = pGetThemePartSize(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
	return rv;
}

HRESULT uxtheme_handle::GetThemeTextExtent(HTHEME hTheme, HDC hdc, 
	int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
	DWORD dwTextFlags, OPTIONAL const RECT *pBoundingRect, 
	OUT RECT *pExtentRect)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeTextExtent)
		pGetThemeTextExtent = (GetThemeTextExtentProc)GetProcAddress(inst_uxtheme, "GetThemeTextExtent");
	if (pGetThemeTextExtent)
		rv = pGetThemeTextExtent(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, 
	dwTextFlags, pBoundingRect, pExtentRect);
	return rv;
}

HRESULT uxtheme_handle::GetThemeTextMetrics(HTHEME hTheme, OPTIONAL HDC hdc, 
	int iPartId, int iStateId, OUT TEXTMETRIC* ptm)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeTextMetrics)
		pGetThemeTextMetrics = (GetThemeTextMetricsProc)GetProcAddress(inst_uxtheme, "GetThemeTextMetrics");
	if (pGetThemeTextMetrics)
		rv = pGetThemeTextMetrics(hTheme, hdc, iPartId, iStateId, ptm);
	return rv;
}

HRESULT uxtheme_handle::GetThemeBackgroundRegion(HTHEME hTheme, OPTIONAL HDC hdc,  
	int iPartId, int iStateId, const RECT *pRect, OUT HRGN *pRegion)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeBackgroundRegion)
		pGetThemeBackgroundRegion = (GetThemeBackgroundRegionProc)GetProcAddress(inst_uxtheme, "GetThemeBackgroundRegion");
	if (pGetThemeBackgroundRegion)
		rv = pGetThemeBackgroundRegion(hTheme, hdc, iPartId, iStateId, pRect, pRegion);
	return rv;
}

HRESULT uxtheme_handle::HitTestThemeBackground(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
	int iStateId, DWORD dwOptions, const RECT *pRect, OPTIONAL HRGN hrgn, 
	POINT ptTest, OUT WORD *pwHitTestCode)
{
	HRESULT rv = E_FAIL;
	if (!pHitTestThemeBackground)
		pHitTestThemeBackground = (HitTestThemeBackgroundProc)GetProcAddress(inst_uxtheme, "HitTestThemeBackground");
	if (pHitTestThemeBackground)
		rv = pHitTestThemeBackground(hTheme, hdc, iPartId, iStateId, dwOptions, pRect, hrgn, ptTest, pwHitTestCode);
	return rv;
}

HRESULT uxtheme_handle::DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
	const RECT *pDestRect, UINT uEdge, UINT uFlags, OPTIONAL OUT RECT *pContentRect)
{
	HRESULT rv = E_FAIL;
	if (!pDrawThemeEdge)
		pDrawThemeEdge = (DrawThemeEdgeProc)GetProcAddress(inst_uxtheme, "DrawThemeEdge");
	if (pDrawThemeEdge)
		rv = pDrawThemeEdge(hTheme, hdc, iPartId, iStateId, pDestRect, uEdge, uFlags, pContentRect);
	return rv;
}

HRESULT uxtheme_handle::DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, 
	int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex)
{
	HRESULT rv = E_FAIL;
	if (!pDrawThemeIcon)
		pDrawThemeIcon = (DrawThemeIconProc)GetProcAddress(inst_uxtheme, "DrawThemeIcon");
	if (pDrawThemeIcon)
		rv = pDrawThemeIcon(hTheme, hdc, iPartId, iStateId, pRect, himl, iImageIndex);
	return rv;
}

BOOL uxtheme_handle::IsThemePartDefined(HTHEME hTheme, int iPartId, int iStateId)
{
	BOOL rv = FALSE;
	if (!pIsThemePartDefined)
		pIsThemePartDefined = (IsThemePartDefinedProc)GetProcAddress(inst_uxtheme, "IsThemePartDefined");
	if (pIsThemePartDefined)
		rv = pIsThemePartDefined(hTheme, iPartId, iStateId);
	return rv;
}

BOOL uxtheme_handle::IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, 
	int iPartId, int iStateId)
{
	BOOL rv = FALSE;
	if (!pIsThemeBackgroundPartiallyTransparent)
		pIsThemeBackgroundPartiallyTransparent = (IsThemeBackgroundPartiallyTransparentProc)GetProcAddress(inst_uxtheme, "IsThemeBackgroundPartiallyTransparent");
	if (pIsThemeBackgroundPartiallyTransparent)
		rv = pIsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId);
	return rv;
}

HRESULT uxtheme_handle::GetThemeColor(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT COLORREF *pColor)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeColor)
		pGetThemeColor = (GetThemeColorProc)GetProcAddress(inst_uxtheme, "GetThemeColor");
	if (pGetThemeColor)
		rv = pGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
	return rv;
}

HRESULT uxtheme_handle::GetThemeMetric(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
	int iStateId, int iPropId, OUT int *piVal)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeMetric)
		pGetThemeMetric = (GetThemeMetricProc)GetProcAddress(inst_uxtheme, "GetThemeMetric");
	if (pGetThemeMetric)
		rv = pGetThemeMetric(hTheme, hdc, iPartId, iStateId, iPropId, piVal);
	return rv;
}

HRESULT uxtheme_handle::GetThemeString(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeString)
		pGetThemeString = (GetThemeStringProc)GetProcAddress(inst_uxtheme, "GetThemeString");
	if (pGetThemeString)
		rv = pGetThemeString(hTheme, iPartId, iStateId, iPropId, pszBuff, cchMaxBuffChars);
	return rv;
}

HRESULT uxtheme_handle::GetThemeBool(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT BOOL *pfVal)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeBool)
		pGetThemeBool = (GetThemeBoolProc)GetProcAddress(inst_uxtheme, "GetThemeBool");
	if (pGetThemeBool)
		rv = pGetThemeBool(hTheme, iPartId, iStateId, iPropId, pfVal);
	return rv;
}

HRESULT uxtheme_handle::GetThemeInt(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT int *piVal)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeInt)
		pGetThemeInt = (GetThemeIntProc)GetProcAddress(inst_uxtheme, "GetThemeInt");
	if (pGetThemeInt)
		rv = pGetThemeInt(hTheme, iPartId, iStateId, iPropId, piVal);
	return rv;
}

HRESULT uxtheme_handle::GetThemeEnumValue(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT int *piVal)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeEnumValue)
		pGetThemeEnumValue = (GetThemeEnumValueProc)GetProcAddress(inst_uxtheme, "GetThemeEnumValue");
	if (pGetThemeEnumValue)
		rv = pGetThemeEnumValue(hTheme, iPartId, iStateId, iPropId, piVal);
	return rv;
}

HRESULT uxtheme_handle::GetThemePosition(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT POINT *pPoint)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemePosition)
		pGetThemePosition = (GetThemePositionProc)GetProcAddress(inst_uxtheme, "GetThemePosition");
	if (pGetThemePosition)
		rv = pGetThemePosition(hTheme, iPartId, iStateId, iPropId, pPoint);
	return rv;
}

HRESULT uxtheme_handle::GetThemeFont(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
	int iStateId, int iPropId, OUT LOGFONT *pFont)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeFont)
		pGetThemeFont = (GetThemeFontProc)GetProcAddress(inst_uxtheme, "GetThemeFont");
	if (pGetThemeFont)
		rv = pGetThemeFont(hTheme, hdc, iPartId, iStateId, iPropId, pFont);
	return rv;
}

HRESULT uxtheme_handle::GetThemeRect(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT RECT *pRect)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeRect)
		pGetThemeRect = (GetThemeRectProc)GetProcAddress(inst_uxtheme, "GetThemeRect");
	if (pGetThemeRect)
		rv = pGetThemeRect(hTheme, iPartId, iStateId, iPropId, pRect);
	return rv;
}

HRESULT uxtheme_handle::GetThemeMargins(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
	int iStateId, int iPropId, OPTIONAL RECT *prc, OUT MARGINS *pMargins)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeMargins)
		pGetThemeMargins = (GetThemeMarginsProc)GetProcAddress(inst_uxtheme, "GetThemeMargins");
	if (pGetThemeMargins)
		rv = pGetThemeMargins(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
	return rv;
}

HRESULT uxtheme_handle::GetThemeIntList(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT INTLIST *pIntList)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeIntList)
		pGetThemeIntList = (GetThemeIntListProc)GetProcAddress(inst_uxtheme, "GetThemeIntList");
	if (pGetThemeIntList)
		rv = pGetThemeIntList(hTheme, iPartId, iStateId, iPropId, pIntList);
	return rv;
}

HRESULT uxtheme_handle::GetThemePropertyOrigin(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT enum PROPERTYORIGIN *pOrigin)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemePropertyOrigin)
		pGetThemePropertyOrigin = (GetThemePropertyOriginProc)GetProcAddress(inst_uxtheme, "GetThemePropertyOrigin");
	if (pGetThemePropertyOrigin)
		rv = pGetThemePropertyOrigin(hTheme, iPartId, iStateId, iPropId, pOrigin);
	return rv;
}

HRESULT uxtheme_handle::SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
	LPCWSTR pszSubIdList)
{
	HRESULT rv = E_FAIL;
	if (!pSetWindowTheme)
		pSetWindowTheme = (SetWindowThemeProc)GetProcAddress(inst_uxtheme, "SetWindowTheme");
	if (pSetWindowTheme)
		rv = pSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
	return rv;
}

HRESULT uxtheme_handle::GetThemeFilename(HTHEME hTheme, int iPartId, 
	int iStateId, int iPropId, OUT LPWSTR pszThemeFileName, int cchMaxBuffChars)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeFilename)
		pGetThemeFilename = (GetThemeFilenameProc)GetProcAddress(inst_uxtheme, "GetThemeFilename");
	if (pGetThemeFilename)
		rv = pGetThemeFilename(hTheme, iPartId, iStateId, iPropId, pszThemeFileName, cchMaxBuffChars);
	return rv;
}

COLORREF uxtheme_handle::GetThemeSysColor(HTHEME hTheme, int iColorId)
{
	COLORREF rv = 0;
	if (!pGetThemeSysColor)
		pGetThemeSysColor = (GetThemeSysColorProc)GetProcAddress(inst_uxtheme, "GetThemeSysColor");
	if (pGetThemeSysColor)
		rv = pGetThemeSysColor(hTheme, iColorId);
	return rv;
}

HBRUSH uxtheme_handle::GetThemeSysColorBrush(HTHEME hTheme, int iColorId)
{
	HBRUSH rv = 0;
	if (!pGetThemeSysColorBrush)
		pGetThemeSysColorBrush = (GetThemeSysColorBrushProc)GetProcAddress(inst_uxtheme, "GetThemeSysColorBrush");
	if (pGetThemeSysColorBrush)
		rv = pGetThemeSysColorBrush(hTheme, iColorId);
	return rv;
}

BOOL uxtheme_handle::GetThemeSysBool(HTHEME hTheme, int iBoolId)
{
	BOOL rv = FALSE;
	if (!pGetThemeSysBool)
		pGetThemeSysBool = (GetThemeSysBoolProc)GetProcAddress(inst_uxtheme, "GetThemeSysBool");
	if (pGetThemeSysBool)
		rv = pGetThemeSysBool(hTheme, iBoolId);
	return rv;
}

int uxtheme_handle::GetThemeSysSize(HTHEME hTheme, int iSizeId)
{
	int rv = 0;
	if (!pGetThemeSysSize)
		pGetThemeSysSize = (GetThemeSysSizeProc)GetProcAddress(inst_uxtheme, "GetThemeSysSize");
	if (pGetThemeSysSize)
		rv = pGetThemeSysSize(hTheme, iSizeId);
	return rv;
}

HRESULT uxtheme_handle::GetThemeSysFont(HTHEME hTheme, int iFontId, OUT LOGFONT *plf)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeSysFont)
		pGetThemeSysFont = (GetThemeSysFontProc)GetProcAddress(inst_uxtheme, "GetThemeSysFont");
	if (pGetThemeSysFont)
		rv = pGetThemeSysFont(hTheme, iFontId, plf);
	return rv;
}

HRESULT uxtheme_handle::GetThemeSysString(HTHEME hTheme, int iStringId, 
	OUT LPWSTR pszStringBuff, int cchMaxStringChars)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeSysString)
		pGetThemeSysString = (GetThemeSysStringProc)GetProcAddress(inst_uxtheme, "GetThemeSysString");
	if (pGetThemeSysString)
		rv = pGetThemeSysString(hTheme, iStringId, pszStringBuff, cchMaxStringChars);
	return rv;
}

HRESULT uxtheme_handle::GetThemeSysInt(HTHEME hTheme, int iIntId, int *piValue)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeSysInt)
		pGetThemeSysInt = (GetThemeSysIntProc)GetProcAddress(inst_uxtheme, "GetThemeSysInt");
	if (pGetThemeSysInt)
		rv = pGetThemeSysInt(hTheme, iIntId, piValue);
	return rv;
}

BOOL uxtheme_handle::IsThemeActive()
{
	HRESULT rv = E_FAIL;
	if (!pIsThemeActive)
		pIsThemeActive = (IsThemeActiveProc)GetProcAddress(inst_uxtheme, "IsThemeActive");
	if (pIsThemeActive)
		rv = pIsThemeActive();
	return rv;
}

BOOL uxtheme_handle::IsAppThemed()
{
	BOOL rv = 0;
	if (!pIsAppThemed)
		pIsAppThemed = (IsAppThemedProc)GetProcAddress(inst_uxtheme, "IsAppThemed");
	if (pIsAppThemed)
		rv = pIsAppThemed();
	return rv;
}

HTHEME uxtheme_handle::GetWindowTheme(HWND hwnd)
{
	HTHEME rv = 0;
	if (!pGetWindowTheme)
		pGetWindowTheme = (GetWindowThemeProc)GetProcAddress(inst_uxtheme, "GetWindowTheme");
	if (pGetWindowTheme)
		rv = pGetWindowTheme(hwnd);
	return rv;
}

HRESULT uxtheme_handle::EnableThemeDialogTexture(HWND wnd, DWORD dw_flags)
{
	HRESULT rv = E_FAIL;
	if (!pEnableThemeDialogTexture)
		pEnableThemeDialogTexture = (EnableThemeDialogTextureProc)GetProcAddress(inst_uxtheme, "EnableThemeDialogTexture");
	if (pEnableThemeDialogTexture)
		rv = (*pEnableThemeDialogTexture)(wnd, dw_flags);
	return rv;
}

BOOL uxtheme_handle::IsThemeDialogTextureEnabled(HWND hwnd)
{
	BOOL rv = FALSE;
	if (!pIsThemeDialogTextureEnabled)
		pIsThemeDialogTextureEnabled = (IsThemeDialogTextureEnabledProc)GetProcAddress(inst_uxtheme, "IsThemeDialogTextureEnabled");
	if (pIsThemeDialogTextureEnabled)
		rv = (*pIsThemeDialogTextureEnabled)(hwnd);
	return rv;
}

DWORD uxtheme_handle::GetThemeAppProperties()
{
	DWORD rv = 0;
	if (!pGetThemeAppProperties)
		pGetThemeAppProperties = (GetThemeAppPropertiesProc)GetProcAddress(inst_uxtheme, "GetThemeAppProperties");
	if (pGetThemeAppProperties)
		rv = (*pGetThemeAppProperties)();
	return rv;
}

void uxtheme_handle::SetThemeAppProperties(DWORD dwFlags)
{
	if (!pSetThemeAppProperties)
		pSetThemeAppProperties = (SetThemeAppPropertiesProc)GetProcAddress(inst_uxtheme, "SetThemeAppProperties");
	if (pSetThemeAppProperties)
		(*pSetThemeAppProperties)(dwFlags);
}

HRESULT uxtheme_handle::GetCurrentThemeName(OUT LPWSTR pszThemeFileName, int cchMaxNameChars, 
	OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars, OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars)
{
	HRESULT rv = E_FAIL;
	if (!pGetCurrentThemeName)
		pGetCurrentThemeName = (GetCurrentThemeNameProc)GetProcAddress(inst_uxtheme, "GetCurrentThemeName");
	if (pGetCurrentThemeName)
		rv = (*pGetCurrentThemeName)(pszThemeFileName, cchMaxNameChars, pszColorBuff, cchMaxColorChars,
			pszSizeBuff, cchMaxSizeChars);
	return rv;
}

HRESULT uxtheme_handle::GetThemeDocumentationProperty(LPCWSTR pszThemeName,
	LPCWSTR pszPropertyName, OUT LPWSTR pszValueBuff, int cchMaxValChars)
{
	HRESULT rv = E_FAIL;
	if (!pGetThemeDocumentationProperty)
		pGetThemeDocumentationProperty = (GetThemeDocumentationPropertyProc)GetProcAddress(inst_uxtheme, "GetThemeDocumentationProperty");
	if (pGetThemeDocumentationProperty)
		rv = (*pGetThemeDocumentationProperty)(pszThemeName, pszPropertyName, pszValueBuff, cchMaxValChars);
	return rv;
}

HRESULT uxtheme_handle::DrawThemeParentBackground(HWND hwnd, HDC hdc, OPTIONAL RECT* prc)
{
	HRESULT rv = E_FAIL;
	if (!pDrawThemeParentBackground)
		pDrawThemeParentBackground = (DrawThemeParentBackgroundProc)GetProcAddress(inst_uxtheme, "DrawThemeParentBackground");
	if (pDrawThemeParentBackground)
		rv = (*pDrawThemeParentBackground)(hwnd, hdc, prc);
	return rv;
}

HRESULT uxtheme_handle::EnableTheming(BOOL fEnable)
{
	HRESULT rv = E_FAIL;
	if (!pEnableTheming)
		pEnableTheming = (EnableThemingProc)GetProcAddress(inst_uxtheme, "EnableTheming");
	if (pEnableTheming)
		rv = (*pEnableTheming)(fEnable);
	return rv;
}

HRESULT uxtheme_handle::DrawThemeBackgroundEx(HTHEME hTheme, HDC hdc, 
	int iPartId, int iStateId, const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions)
{
	HRESULT rv = E_FAIL;
	if (!pDrawThemeBackgroundEx)
		pDrawThemeBackgroundEx = (DrawThemeBackgroundExProc)GetProcAddress(inst_uxtheme, "DrawThemeBackgroundEx");
	if (pDrawThemeBackgroundEx)
		rv = (*pDrawThemeBackgroundEx)(hTheme, hdc, iPartId, iStateId, pRect, pOptions);
	return rv;
}

#endif