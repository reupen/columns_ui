#ifndef _COLUMNS_UXTHEME_H_
#define _COLUMNS_UXTHEME_H_

/**
 * \file uxtheme.h
 * Uxtheme API wrapper (not fully tested)
 * \author musicmusic
 * \author microsoft
 */

//typedef pfc::refcounted_object_ptr_t<class uxtheme_handle> uxtheme_api_ptr;

//#if(WINVER >= 0x0501) //guess of platform sdk ver
#include <vssym32.h>
//#include <tmschema.h>
//#endif

#include <uxtheme.h>

#ifndef _UXTHEME_H_
typedef HANDLE HTHEME;
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif

//---------------------------------------------------------------------------
// NOTE: PartId's and StateId's used in the theme API are defined in the 
//       hdr file <tmschema.h> using the TM_PART and TM_STATE macros.  For
//       example, "TM_PART(BP, PUSHBUTTON)" defines the PartId "BP_PUSHBUTTON".

//---------------------------------------------------------------------------
//  OpenThemeData()     - Open the theme data for the specified HWND and 
//                        semi-colon separated list of class names. 
// 
//                        OpenThemeData() will try each class name, one at 
//                        a time, and use the first matching theme info
//                        found.  If a match is found, a theme handle
//                        to the data is returned.  If no match is found,
//                        a "NULL" handle is returned. 
//
//                        When the window is destroyed or a WM_THEMECHANGED
//                        msg is received, "CloseThemeData()" should be 
//                        called to close the theme handle.
//
//  hwnd                - window handle of the control/window to be themed
//
//  pszClassList        - class name (or list of names) to match to theme data
//                        section.  if the list contains more than one name, 
//                        the names are tested one at a time for a match.  
//                        If a match is found, OpenThemeData() returns a 
//                        theme handle associated with the matching class. 
//                        This param is a list (instead of just a single 
//                        class name) to provide the class an opportunity 
//                        to get the "best" match between the class and 
//                        the current theme.  For example, a button might
//                        pass L"OkButton, Button" if its ID=ID_OK.  If 
//                        the current theme has an entry for OkButton, 
//                        that will be used.  Otherwise, we fall back on 
//                        the normal Button entry.
//---------------------------------------------------------------------------
typedef HTHEME (STDAPICALLTYPE * OpenThemeDataProc)(HWND hwnd, LPCWSTR pszClassList);

//---------------------------------------------------------------------------
//  CloseTHemeData()    - closes the theme data handle.  This should be done 
//                        when the window being themed is destroyed or
//                        whenever a WM_THEMECHANGED msg is received 
//                        (followed by an attempt to create a new Theme data 
//                        handle).
//
//  hTheme              - open theme data handle (returned from prior call
//                        to OpenThemeData() API).
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * CloseThemeDataProc)(HTHEME hTheme);

//---------------------------------------------------------------------------
//    functions for basic drawing support 
//---------------------------------------------------------------------------
// The following methods are the theme-aware drawing services.
// Controls/Windows are defined in drawable "parts" by their author: a 
// parent part and 0 or more child parts.  Each of the parts can be 
// described in "states" (ex: disabled, hot, pressed).  
//---------------------------------------------------------------------------
// For the list of all themed classes and the definition of all
// parts and states, see the file "tmschmea.h".
//---------------------------------------------------------------------------
// Each of the below methods takes a "iPartId" param to specify the 
// part and a "iStateId" to specify the state of the part.  
// "iStateId=0" refers to the root part.  "iPartId" = "0" refers to 
// the root class.  
//-----------------------------------------------------------------------
// Note: draw operations are always scaled to fit (and not to exceed)  
// the specified "Rect".
//-----------------------------------------------------------------------

//------------------------------------------------------------------------
//  DrawThemeBackground()   
//                      - draws the theme-specified border and fill for 
//                        the "iPartId" and "iStateId".  This could be 
//                        based on a bitmap file, a border and fill, or 
//                        other image description.  
//
//  hTheme              - theme data handle
//  hdc                 - HDC to draw into
//  iPartId             - part number to draw
//  iStateId            - state number (of the part) to draw
//  pRect               - defines the size/location of the part
//  pClipRect           - optional clipping rect (don't draw outside it)
//------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * DrawThemeBackgroundProc)(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);

#ifndef _UXTHEME_H_
//---------------------------------------------------------------------------
//----- DrawThemeText() flags ----

#define DTT_GRAYED      0x1         // draw a grayed-out string  
#endif

//-------------------------------------------------------------------------
//  DrawThemeText()     - draws the text using the theme-specified 
//                        color and font for the "iPartId" and 
//                        "iStateId".  
//
//  hTheme              - theme data handle
//  hdc                 - HDC to draw into
//  iPartId             - part number to draw
//  iStateId            - state number (of the part) to draw
//  pszText             - actual text to draw
//  dwCharCount         - number of chars to draw (-1 for all)
//  dwTextFlags         - same as DrawText() "uFormat" param
//  dwTextFlags2        - additional drawing options 
//  pRect               - defines the size/location of the part
//-------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * DrawThemeTextProc)(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
    DWORD dwTextFlags2, const RECT *pRect);

//-------------------------------------------------------------------------
//  GetThemeBackgroundContentRect()
//                      - gets the size of the content for the theme-defined 
//                        background.  This is usually the area inside
//                        the borders or Margins.  
//
//      hTheme          - theme data handle
//      hdc             - (optional) device content to be used for drawing
//      iPartId         - part number to draw
//      iStateId        - state number (of the part) to draw
//      pBoundingRect   - the outer RECT of the part being drawn
//      pContentRect    - RECT to receive the content area
//-------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeBackgroundContentRectProc)(HTHEME hTheme, OPTIONAL HDC hdc, 
    int iPartId, int iStateId,  const RECT *pBoundingRect, 
    OUT RECT *pContentRect);

//-------------------------------------------------------------------------
//  GetThemeBackgroundExtent() - calculates the size/location of the theme-
//                               specified background based on the 
//                               "pContentRect".
//
//      hTheme          - theme data handle
//      hdc             - (optional) device content to be used for drawing
//      iPartId         - part number to draw
//      iStateId        - state number (of the part) to draw
//      pContentRect    - RECT that defines the content area
//      pBoundingRect   - RECT to receive the overall size/location of part
//-------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeBackgroundExtentProc)(HTHEME hTheme, OPTIONAL HDC hdc,
    int iPartId, int iStateId, const RECT *pContentRect, 
    OUT RECT *pExtentRect);

#ifndef _UXTHEME_H_
//-------------------------------------------------------------------------
typedef enum THEMESIZE
{
    TS_MIN,             // minimum size
    TS_TRUE,            // size without stretching
    TS_DRAW,            // size that theme mgr will use to draw part
};
#endif
//-------------------------------------------------------------------------
//  GetThemePartSize() - returns the specified size of the theme part
//
//  hTheme              - theme data handle
//  hdc                 - HDC to select font into & measure against
//  iPartId             - part number to retrieve size for
//  iStateId            - state number (of the part) 
//  prc                 - (optional) rect for part drawing destination
//  eSize               - the type of size to be retreived
//  psz                 - receives the specified size of the part 
//-------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemePartSizeProc)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
    OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz);

//-------------------------------------------------------------------------
//  GetThemeTextExtent() - calculates the size/location of the specified 
//                         text when rendered in the Theme Font. 
//
//  hTheme              - theme data handle
//  hdc                 - HDC to select font & measure into
//  iPartId             - part number to draw
//  iStateId            - state number (of the part) 
//  pszText             - the text to be measured
//  dwCharCount         - number of chars to draw (-1 for all)
//  dwTextFlags         - same as DrawText() "uFormat" param
//  pszBoundingRect     - optional: to control layout of text
//  pszExtentRect       - receives the RECT for text size/location
//-------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeTextExtentProc)(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
    DWORD dwTextFlags, OPTIONAL const RECT *pBoundingRect, 
    OUT RECT *pExtentRect);

//-------------------------------------------------------------------------
//  GetThemeTextMetrics()
//                      - returns info about the theme-specified font
//                        for the part/state passed in.
//
//  hTheme              - theme data handle
//  hdc                 - optional: HDC for screen context
//  iPartId             - part number to draw
//  iStateId            - state number (of the part) 
//  ptm                 - receives the font info
//-------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeTextMetricsProc)(HTHEME hTheme, OPTIONAL HDC hdc, 
    int iPartId, int iStateId, OUT TEXTMETRIC* ptm);

//-------------------------------------------------------------------------
//  GetThemeBackgroundRegion()
//                      - computes the region for a regular or partially 
//                        transparent theme-specified background that is 
//                        bound by the specified "pRect".
//                        If the rectangle is empty, sets the HRGN to NULL 
//                        and return S_FALSE.
//
//  hTheme              - theme data handle
//  hdc                 - optional HDC to draw into (DPI scaling)
//  iPartId             - part number to draw
//  iStateId            - state number (of the part) 
//  pRect               - the RECT used to draw the part
//  pRegion             - receives handle to calculated region
//-------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeBackgroundRegionProc)(HTHEME hTheme, OPTIONAL HDC hdc,  
    int iPartId, int iStateId, const RECT *pRect, OUT HRGN *pRegion);

#ifndef _UXTHEME_H_
//-------------------------------------------------------------------------
//----- HitTestThemeBackground, HitTestThemeBackgroundRegion flags ----

//  Theme background segment hit test flag (default). possible return values are:
//  HTCLIENT: hit test succeeded in the middle background segment
//  HTTOP, HTLEFT, HTTOPLEFT, etc:  // hit test succeeded in the the respective theme background segment.
#define HTTB_BACKGROUNDSEG          0x0000  

//  Fixed border hit test option.  possible return values are:
//  HTCLIENT: hit test succeeded in the middle background segment
//  HTBORDER: hit test succeeded in any other background segment
#define HTTB_FIXEDBORDER            0x0002  // Return code may be either HTCLIENT or HTBORDER. 

//  Caption hit test option.  Possible return values are:
//  HTCAPTION: hit test succeeded in the top, top left, or top right background segments
//  HTNOWHERE or another return code, depending on absence or presence of accompanying flags, resp.
#define HTTB_CAPTION                0x0004  

//  Resizing border hit test flags.  Possible return values are:
//  HTCLIENT: hit test succeeded in middle background segment
//  HTTOP, HTTOPLEFT, HTLEFT, HTRIGHT, etc:    hit test succeeded in the respective system resizing zone
//  HTBORDER: hit test failed in middle segment and resizing zones, but succeeded in a background border segment
#define HTTB_RESIZINGBORDER_LEFT    0x0010  // Hit test left resizing border, 
#define HTTB_RESIZINGBORDER_TOP     0x0020  // Hit test top resizing border
#define HTTB_RESIZINGBORDER_RIGHT   0x0040  // Hit test right resizing border
#define HTTB_RESIZINGBORDER_BOTTOM  0x0080  // Hit test bottom resizing border

#define HTTB_RESIZINGBORDER         (HTTB_RESIZINGBORDER_LEFT|HTTB_RESIZINGBORDER_TOP|\
                                     HTTB_RESIZINGBORDER_RIGHT|HTTB_RESIZINGBORDER_BOTTOM)

// Resizing border is specified as a template, not just window edges.
// This option is mutually exclusive with HTTB_SYSTEMSIZINGWIDTH; HTTB_SIZINGTEMPLATE takes precedence  
#define HTTB_SIZINGTEMPLATE      0x0100

// Use system resizing border width rather than theme content margins.   
// This option is mutually exclusive with HTTB_SIZINGTEMPLATE, which takes precedence.
#define HTTB_SYSTEMSIZINGMARGINS 0x0200
#endif

//-------------------------------------------------------------------------
//  HitTestThemeBackground()
//                      - returns a HitTestCode (a subset of the values 
//                        returned by WM_NCHITTEST) for the point "ptTest" 
//                        within the theme-specified background
//                        (bound by pRect).  "pRect" and "ptTest" should 
//                        both be in the same coordinate system 
//                        (client, screen, etc).
//
//      hTheme          - theme data handle
//      hdc             - HDC to draw into
//      iPartId         - part number to test against
//      iStateId        - state number (of the part) 
//      pRect           - the RECT used to draw the part
//      hrgn            - optional region to use; must be in same coordinates as
//                      -    pRect and pTest.
//      ptTest          - the hit point to be tested
//      dwOptions       - HTTB_xxx constants
//      pwHitTestCode   - receives the returned hit test code - one of:
//  
//                        HTNOWHERE, HTLEFT, HTTOPLEFT, HTBOTTOMLEFT,
//                        HTRIGHT, HTTOPRIGHT, HTBOTTOMRIGHT,
//                        HTTOP, HTBOTTOM, HTCLIENT
//-------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * HitTestThemeBackgroundProc)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, DWORD dwOptions, const RECT *pRect, OPTIONAL HRGN hrgn, 
    POINT ptTest, OUT WORD *pwHitTestCode);

//------------------------------------------------------------------------
//  DrawThemeEdge()     - Similar to the DrawEdge() API, but uses part colors
//                        and is high-DPI aware
//  hTheme              - theme data handle
//  hdc                 - HDC to draw into
//  iPartId             - part number to draw
//  iStateId            - state number of part
//  pDestRect           - the RECT used to draw the line(s)
//  uEdge               - Same as DrawEdge() API
//  uFlags              - Same as DrawEdge() API
//  pContentRect        - Receives the interior rect if (uFlags & BF_ADJUST)
//------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * DrawThemeEdgeProc)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
                       const RECT *pDestRect, UINT uEdge, UINT uFlags, OPTIONAL OUT RECT *pContentRect);

//------------------------------------------------------------------------
//  DrawThemeIcon()     - draws an image within an imagelist based on 
//                        a (possible) theme-defined effect. 
//
//  hTheme              - theme data handle
//  hdc                 - HDC to draw into
//  iPartId             - part number to draw
//  iStateId            - state number of part
//  pRect               - the RECT to draw the image within
//  himl                - handle to IMAGELIST 
//  iImageIndex         - index into IMAGELIST (which icon to draw)
//------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * DrawThemeIconProc)(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);

//---------------------------------------------------------------------------
//  IsThemePartDefined() - returns TRUE if the theme has defined parameters
//                         for the specified "iPartId" and "iStateId".
//
//  hTheme              - theme data handle
//  iPartId             - part number to find definition for
//  iStateId            - state number of part
//---------------------------------------------------------------------------
typedef BOOL (STDAPICALLTYPE * IsThemePartDefinedProc)(HTHEME hTheme, int iPartId, 
    int iStateId);

//---------------------------------------------------------------------------
//  IsThemeBackgroundPartiallyTransparent()
//                      - returns TRUE if the theme specified background for 
//                        the part/state has transparent pieces or 
//                        alpha-blended pieces.
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//---------------------------------------------------------------------------
typedef BOOL (STDAPICALLTYPE * IsThemeBackgroundPartiallyTransparentProc)(HTHEME hTheme, 
    int iPartId, int iStateId);

//---------------------------------------------------------------------------
//    lower-level theme information services
//---------------------------------------------------------------------------
// The following methods are getter routines for each of the Theme Data types.
// Controls/Windows are defined in drawable "parts" by their author: a 
// parent part and 0 or more child parts.  Each of the parts can be 
// described in "states" (ex: disabled, hot, pressed).  
//---------------------------------------------------------------------------
// Each of the below methods takes a "iPartId" param to specify the 
// part and a "iStateId" to specify the state of the part.  
// "iStateId=0" refers to the root part.  "iPartId" = "0" refers to 
// the root class.  
//-----------------------------------------------------------------------
// Each method also take a "iPropId" param because multiple instances of 
// the same primitive type can be defined in the theme schema.
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
//  GetThemeColor()     - Get the value for the specified COLOR property
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  pColor              - receives the value of the property
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeColorProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT COLORREF *pColor);

//-----------------------------------------------------------------------
//  GetThemeMetric()    - Get the value for the specified metric/size
//                        property
//
//  hTheme              - theme data handle
//  hdc                 - (optional) hdc to be drawn into (DPI scaling)
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  piVal               - receives the value of the property
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeMetricProc)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, int iPropId, OUT int *piVal);

//-----------------------------------------------------------------------
//  GetThemeString()    - Get the value for the specified string property
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  pszBuff             - receives the string property value
//  cchMaxBuffChars     - max. number of chars allowed in pszBuff
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeStringProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars);

//-----------------------------------------------------------------------
//  GetThemeBool()      - Get the value for the specified BOOL property
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  pfVal               - receives the value of the property
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeBoolProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT BOOL *pfVal);

//-----------------------------------------------------------------------
//  GetThemeInt()       - Get the value for the specified int property
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  piVal               - receives the value of the property
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeIntProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT int *piVal);

//-----------------------------------------------------------------------
//  GetThemeEnumValue() - Get the value for the specified ENUM property
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  piVal               - receives the value of the enum (cast to int*)
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeEnumValueProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT int *piVal);

//-----------------------------------------------------------------------
//  GetThemePosition()  - Get the value for the specified position
//                        property
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  pPoint              - receives the value of the position property
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemePositionProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT POINT *pPoint);

//-----------------------------------------------------------------------
//  GetThemeFont()      - Get the value for the specified font property
//
//  hTheme              - theme data handle
//  hdc                 - (optional) hdc to be drawn to (DPI scaling)
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  pFont               - receives the value of the LOGFONT property
//                        (scaled for the current logical screen dpi) 
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeFontProc)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, int iPropId, OUT LOGFONT *pFont);

//-----------------------------------------------------------------------
//  GetThemeRect()      - Get the value for the specified RECT property
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to get the value for
//  pRect               - receives the value of the RECT property
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeRectProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT RECT *pRect);

//-----------------------------------------------------------------------
#ifndef _UXTHEME_H_
typedef struct _MARGINS
{
    int cxLeftWidth;      // width of left border that retains its size
    int cxRightWidth;     // width of right border that retains its size
    int cyTopHeight;      // height of top border that retains its size
    int cyBottomHeight;   // height of bottom border that retains its size
} MARGINS, *PMARGINS;
#endif

//-----------------------------------------------------------------------
//  GetThemeMargins()   - Get the value for the specified MARGINS property
//
//      hTheme          - theme data handle
//      hdc             - (optional) hdc to be used for drawing
//      iPartId         - part number 
//      iStateId        - state number of part
//      iPropId         - the property number to get the value for
//      prc             - RECT for area to be drawn into
//      pMargins        - receives the value of the MARGINS property
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeMarginsProc)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, int iPropId, OPTIONAL RECT *prc, OUT MARGINS *pMargins);

//-----------------------------------------------------------------------
#ifndef _UXTHEME_H_
#define MAX_INTLIST_COUNT 10

typedef struct _INTLIST
{
    int iValueCount;      // number of values in iValues
    int iValues[MAX_INTLIST_COUNT];
} INTLIST, *PINTLIST;
#endif

//-----------------------------------------------------------------------
//  GetThemeIntList()   - Get the value for the specified INTLIST struct
//
//      hTheme          - theme data handle
//      iPartId         - part number 
//      iStateId        - state number of part
//      iPropId         - the property number to get the value for
//      pIntList        - receives the value of the INTLIST property
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeIntListProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT INTLIST *pIntList);

//-----------------------------------------------------------------------
#ifndef _UXTHEME_H_
typedef enum PROPERTYORIGIN
{
    PO_STATE,           // property was found in the state section
    PO_PART,            // property was found in the part section
    PO_CLASS,           // property was found in the class section
    PO_GLOBAL,          // property was found in [globals] section
    PO_NOTFOUND         // property was not found
};
#endif

//-----------------------------------------------------------------------
//  GetThemePropertyOrigin()
//                      - searches for the specified theme property
//                        and sets "pOrigin" to indicate where it was 
//                        found (or not found)
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to search for
//  pOrigin             - receives the value of the property origin
//-----------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemePropertyOriginProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT enum PROPERTYORIGIN *pOrigin);

//---------------------------------------------------------------------------
//  SetWindowTheme()
//                      - redirects an existing Window to use a different 
//                        section of the current theme information than its 
//                        class normally asks for.
//
//  hwnd                - the handle of the window (cannot be NULL)
//
//  pszSubAppName       - app (group) name to use in place of the calling
//                        app's name.  If NULL, the actual calling app 
//                        name will be used.
//
//  pszSubIdList        - semicolon separated list of class Id names to 
//                        use in place of actual list passed by the 
//                        window's class.  if NULL, the id list from the 
//                        calling class is used.
//---------------------------------------------------------------------------
// The Theme Manager will remember the "pszSubAppName" and the
// "pszSubIdList" associations thru the lifetime of the window (even 
// if themes are subsequently changed).  The window is sent a 
// "WM_THEMECHANGED" msg at the end of this call, so that the new
// theme can be found and applied.
//---------------------------------------------------------------------------
// When "pszSubAppName" or "pszSubIdList" are NULL, the Theme Manager 
// removes the previously remember association.  To turn off theme-ing for 
// the specified window, you can pass an empty string (L"") so it 
// won't match any section entries.
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * SetWindowThemeProc)(HWND hwnd, LPCWSTR pszSubAppName, 
    LPCWSTR pszSubIdList);

//---------------------------------------------------------------------------
//  GetThemeFilename()  - Get the value for the specified FILENAME property.
//
//  hTheme              - theme data handle
//  iPartId             - part number 
//  iStateId            - state number of part
//  iPropId             - the property number to search for
//  pszThemeFileName    - output buffer to receive the filename
//  cchMaxBuffChars     - the size of the return buffer, in chars
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeFilenameProc)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT LPWSTR pszThemeFileName, int cchMaxBuffChars);

//---------------------------------------------------------------------------
//  GetThemeSysColor()  - Get the value of the specified System color. 
//
//  hTheme              - the theme data handle.  if non-NULL, will return
//                        color from [SysMetrics] section of theme.
//                        if NULL, will return the global system color.
//
//  iColorId            - the system color index defined in winuser.h
//---------------------------------------------------------------------------
typedef COLORREF (STDAPICALLTYPE * GetThemeSysColorProc)(HTHEME hTheme, int iColorId);

//---------------------------------------------------------------------------
//  GetThemeSysColorBrush() 
//                      - Get the brush for the specified System color. 
//
//  hTheme              - the theme data handle.  if non-NULL, will return
//                        brush matching color from [SysMetrics] section of 
//                        theme.  if NULL, will return the brush matching
//                        global system color.
//
//  iColorId            - the system color index defined in winuser.h
//---------------------------------------------------------------------------
typedef HBRUSH (STDAPICALLTYPE * GetThemeSysColorBrushProc)(HTHEME hTheme, int iColorId);

//---------------------------------------------------------------------------
//  GetThemeSysBool()   - Get the boolean value of specified System metric. 
//
//  hTheme              - the theme data handle.  if non-NULL, will return
//                        BOOL from [SysMetrics] section of theme.
//                        if NULL, will return the specified system boolean.
//
//  iBoolId             - the TMT_XXX BOOL number (first BOOL
//                        is TMT_FLATMENUS)
//---------------------------------------------------------------------------
typedef BOOL (STDAPICALLTYPE * GetThemeSysBoolProc)(HTHEME hTheme, int iBoolId);

//---------------------------------------------------------------------------
//  GetThemeSysSize()   - Get the value of the specified System size metric. 
//                        (scaled for the current logical screen dpi) 
//
//  hTheme              - the theme data handle.  if non-NULL, will return
//                        size from [SysMetrics] section of theme.
//                        if NULL, will return the global system metric.
//
//  iSizeId             - the following values are supported when 
//                        hTheme is non-NULL:
//
//                          SM_CXBORDER   (border width)
//                          SM_CXVSCROLL  (scrollbar width)
//                          SM_CYHSCROLL  (scrollbar height)
//                          SM_CXSIZE     (caption width)
//                          SM_CYSIZE     (caption height)
//                          SM_CXSMSIZE   (small caption width)
//                          SM_CYSMSIZE   (small caption height)
//                          SM_CXMENUSIZE (menubar width)
//                          SM_CYMENUSIZE (menubar height)
//
//                        when hTheme is NULL, iSizeId is passed directly
//                        to the GetSystemMetrics() function
//---------------------------------------------------------------------------
typedef int (STDAPICALLTYPE * GetThemeSysSizeProc)(HTHEME hTheme, int iSizeId);

//---------------------------------------------------------------------------
//  GetThemeSysFont()   - Get the LOGFONT for the specified System font. 
//
//  hTheme              - the theme data handle.  if non-NULL, will return
//                        font from [SysMetrics] section of theme.
//                        if NULL, will return the specified system font.
//
//  iFontId             - the TMT_XXX font number (first font
//                        is TMT_CAPTIONFONT)
//
//  plf                 - ptr to LOGFONT to receive the font value.
//                        (scaled for the current logical screen dpi) 
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeSysFontProc)(HTHEME hTheme, int iFontId, OUT LOGFONT *plf);

//---------------------------------------------------------------------------
//  GetThemeSysString() - Get the value of specified System string metric. 
//
//  hTheme              - the theme data handle (required)
//
//  iStringId           - must be one of the following values:
//
//                          TMT_CSSNAME
//                          TMT_XMLNAME
//
//  pszStringBuff       - the buffer to receive the string value
//
//  cchMaxStringChars   - max. number of chars that pszStringBuff can hold
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeSysStringProc)(HTHEME hTheme, int iStringId, 
    OUT LPWSTR pszStringBuff, int cchMaxStringChars);

//---------------------------------------------------------------------------
//  GetThemeSysInt() - Get the value of specified System int.
//
//  hTheme              - the theme data handle (required)
//
//  iIntId              - must be one of the following values:
//
//                          TMT_DPIX
//                          TMT_DPIY
//                          TMT_MINCOLORDEPTH
//
//  piValue             - ptr to int to receive value
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetThemeSysIntProc)(HTHEME hTheme, int iIntId, int *piValue);

//---------------------------------------------------------------------------
//  IsThemeActive()     - can be used to test if a system theme is active
//                        for the current user session.  
//
//                        use the API "IsAppThemed()" to test if a theme is
//                        active for the calling process.
//---------------------------------------------------------------------------
typedef BOOL (STDAPICALLTYPE * IsThemeActiveProc)();

//---------------------------------------------------------------------------
//  IsAppThemed()       - returns TRUE if a theme is active and available to
//                        the current process
//---------------------------------------------------------------------------
typedef BOOL (STDAPICALLTYPE * IsAppThemedProc)();


//---------------------------------------------------------------------------
//  GetWindowTheme()    - if window is themed, returns its most recent
//                        HTHEME from OpenThemeData() - otherwise, returns
//                        NULL.
//
//      hwnd            - the window to get the HTHEME of
//---------------------------------------------------------------------------
typedef HTHEME (STDAPICALLTYPE * GetWindowThemeProc)(HWND hwnd);


//---------------------------------------------------------------------------
//  EnableThemeDialogTexture() 
//
//  - Enables/disables dialog background theme.  This method can be used to 
//    tailor dialog compatibility with child windows and controls that 
//    may or may not coordinate the rendering of their client area backgrounds 
//    with that of their parent dialog in a manner that supports seamless 
//    background texturing.
//
//      hdlg         - the window handle of the target dialog
//      dwFlags      - ETDT_ENABLE to enable the theme-defined dialog background texturing,
//                     ETDT_DISABLE to disable background texturing,
//                     ETDT_ENABLETAB to enable the theme-defined background 
//                          texturing using the Tab texture
//---------------------------------------------------------------------------

#ifndef _UXTHEME_H_
#define ETDT_DISABLE        0x00000001
#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE  | ETDT_USETABTEXTURE)
#endif

typedef HRESULT (STDAPICALLTYPE * EnableThemeDialogTextureProc)(HWND hwnd, DWORD dwFlags);


//---------------------------------------------------------------------------
//  IsThemeDialogTextureEnabled() 
//
//  - Reports whether the dialog supports background texturing.
//
//      hdlg         - the window handle of the target dialog
//---------------------------------------------------------------------------
typedef BOOL (STDAPICALLTYPE * IsThemeDialogTextureEnabledProc)(HWND hwnd);


//---------------------------------------------------------------------------
//---- flags to control theming within an app ----

#ifndef _UXTHEME_H_
#define STAP_ALLOW_NONCLIENT    (1 << 0)
#define STAP_ALLOW_CONTROLS     (1 << 1)
#define STAP_ALLOW_WEBCONTENT   (1 << 2)
#endif

//---------------------------------------------------------------------------
//  GetThemeAppProperties()
//                      - returns the app property flags that control theming
//---------------------------------------------------------------------------
typedef DWORD (STDAPICALLTYPE * GetThemeAppPropertiesProc)();

//---------------------------------------------------------------------------
//  SetThemeAppProperties()
//                      - sets the flags that control theming within the app
//
//      dwFlags         - the flag values to be set
//---------------------------------------------------------------------------
typedef void (STDAPICALLTYPE * SetThemeAppPropertiesProc)(DWORD dwFlags);

//---------------------------------------------------------------------------
//  GetCurrentThemeName()
//                      - Get the name of the current theme in-use.  
//                        Optionally, return the ColorScheme name and the 
//                        Size name of the theme.
//
//  pszThemeFileName    - receives the theme path & filename
//  cchMaxNameChars     - max chars allowed in pszNameBuff
//
//  pszColorBuff        - (optional) receives the canonical color scheme name
//                        (not the display name)
//  cchMaxColorChars    - max chars allowed in pszColorBuff
//
//  pszSizeBuff         - (optional) receives the canonical size name 
//                        (not the display name)
//  cchMaxSizeChars     - max chars allowed in pszSizeBuff
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * GetCurrentThemeNameProc)(
    OUT LPWSTR pszThemeFileName, int cchMaxNameChars, 
    OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars,
    OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars);

//---------------------------------------------------------------------------
//  GetThemeDocumentationProperty()
//                      - Get the value for the specified property name from 
//                        the [documentation] section of the themes.ini file 
//                        for the specified theme.  If the property has been 
//                        localized in the theme files string table, the 
//                        localized version of the property value is returned. 
//
//  pszThemeFileName    - filename of the theme file to query
//  pszPropertyName     - name of the string property to retreive a value for
//  pszValueBuff        - receives the property string value
//  cchMaxValChars      - max chars allowed in pszValueBuff
//---------------------------------------------------------------------------
#ifndef _UXTHEME_H_
#define SZ_THDOCPROP_DISPLAYNAME                L"DisplayName"
#define SZ_THDOCPROP_CANONICALNAME              L"ThemeName"
#define SZ_THDOCPROP_TOOLTIP                    L"ToolTip"
#define SZ_THDOCPROP_AUTHOR                     L"author"
#endif

typedef HRESULT (STDAPICALLTYPE * GetThemeDocumentationPropertyProc)(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName, OUT LPWSTR pszValueBuff, int cchMaxValChars);

//---------------------------------------------------------------------------
//  Theme API Error Handling
//
//      All functions in the Theme API not returning an HRESULT (HRESULT (STDAPICALLTYPE *_) 
//      use the WIN32 function "SetLastError()" to record any call failures.  
//
//      To retreive the error code of the last failure on the
//      current thread for these type of API's, use the WIN32 function 
//      "GetLastError()".
//
//      All Theme API error codes (HRESULT's and GetLastError() values)
//      should be normal win32 errors which can be formatted into
//      strings using the Win32 API FormatMessage().
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// DrawThemeParentBackground()
//                      - used by partially-transparent or alpha-blended
//                        child controls to draw the part of their parent
//                        that they appear in front of.
//
//  hwnd                - handle of the child control

//  hdc                 - hdc of the child control

//  prc                 - (optional) rect that defines the area to be 
//                        drawn (CHILD coordinates)
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * DrawThemeParentBackgroundProc)(HWND hwnd, HDC hdc, OPTIONAL RECT* prc);

//---------------------------------------------------------------------------
//  EnableTheming()     - enables or disables themeing for the current user
//                        in the current and future sessions.
//
//  fEnable             - if FALSE, disable theming & turn themes off.
//                      - if TRUE, enable themeing and, if user previously
//                        had a theme active, make it active now.
//---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * EnableThemingProc)(BOOL fEnable);

#ifndef _UXTHEME_H_
//------------------------------------------------------------------------
//---- bits used in dwFlags of DTBGOPTS ----
#define DTBG_CLIPRECT        0x00000001   // rcClip has been specified
#define DTBG_DRAWSOLID       0x00000002   // draw transparent/alpha images as solid
#define DTBG_OMITBORDER      0x00000004   // don't draw border of part
#define DTBG_OMITCONTENT     0x00000008   // don't draw content area of part

#define DTBG_COMPUTINGREGION 0x00000010   // TRUE if calling to compute region

#define DTBG_MIRRORDC        0x00000020   // assume the hdc is mirrorred and
                                          // flip images as appropriate (currently 
                                          // only supported for bgtype=imagefile)
//------------------------------------------------------------------------
typedef struct _DTBGOPTS
{
    DWORD dwSize;           // size of the struct
    DWORD dwFlags;          // which options have been specified
    RECT rcClip;            // clipping rectangle
}
DTBGOPTS, *PDTBGOPTS;
#endif

//------------------------------------------------------------------------
//  DrawThemeBackgroundEx()   
//                      - draws the theme-specified border and fill for 
//                        the "iPartId" and "iStateId".  This could be 
//                        based on a bitmap file, a border and fill, or 
//                        other image description.  NOTE: This will be
//                        merged back into DrawThemeBackground() after 
//                        BETA 2.
//
//  hTheme              - theme data handle
//  hdc                 - HDC to draw into
//  iPartId             - part number to draw
//  iStateId            - state number (of the part) to draw
//  pRect               - defines the size/location of the part
//  pOptions            - ptr to optional params
//------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE * DrawThemeBackgroundExProc)(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions);

class uxtheme_handle;

class uxtheme_ptr
{
public:
	operator uxtheme_handle * () {assert(m_uxtheme); return m_uxtheme;}
	uxtheme_handle * operator -> () const {assert(m_uxtheme); return m_uxtheme;}
	void release();
	bool is_valid()const;
	bool load();

	uxtheme_ptr();
	uxtheme_ptr(const uxtheme_ptr & p_source);
	~uxtheme_ptr();

	uxtheme_ptr & operator = (const uxtheme_ptr & p_source) {load(); return *this;}
private:
	uxtheme_handle * m_uxtheme;
};

typedef uxtheme_ptr uxtheme_api_ptr;

class uxtheme_handle// : public pfc::refcounted_object_root
{
	HINSTANCE inst_uxtheme;
	OpenThemeDataProc pOpenThemeData;
	CloseThemeDataProc pCloseThemeData;
	DrawThemeBackgroundProc pDrawThemeBackground;
	DrawThemeTextProc pDrawThemeText;
	GetThemeBackgroundContentRectProc pGetThemeBackgroundContentRect;
	GetThemeBackgroundExtentProc pGetThemeBackgroundExtent;
	GetThemePartSizeProc pGetThemePartSize;
	GetThemeTextExtentProc pGetThemeTextExtent;
	GetThemeTextMetricsProc pGetThemeTextMetrics;
	GetThemeBackgroundRegionProc pGetThemeBackgroundRegion;
	HitTestThemeBackgroundProc pHitTestThemeBackground;
	DrawThemeEdgeProc pDrawThemeEdge;
	DrawThemeIconProc pDrawThemeIcon;
	IsThemePartDefinedProc pIsThemePartDefined;
	IsThemeBackgroundPartiallyTransparentProc pIsThemeBackgroundPartiallyTransparent;
	GetThemeColorProc pGetThemeColor;
	GetThemeMetricProc pGetThemeMetric;
	GetThemeStringProc pGetThemeString;
	GetThemeBoolProc pGetThemeBool;
	GetThemeIntProc pGetThemeInt;
	GetThemeEnumValueProc pGetThemeEnumValue;
	GetThemePositionProc pGetThemePosition;
	GetThemeFontProc pGetThemeFont;
	GetThemeRectProc pGetThemeRect;
	GetThemeMarginsProc pGetThemeMargins;
	GetThemeIntListProc pGetThemeIntList;
	GetThemePropertyOriginProc pGetThemePropertyOrigin;
	SetWindowThemeProc pSetWindowTheme;
	GetThemeFilenameProc pGetThemeFilename;
	GetThemeSysColorProc pGetThemeSysColor;
	GetThemeSysColorBrushProc pGetThemeSysColorBrush;
	GetThemeSysBoolProc pGetThemeSysBool;
	GetThemeSysSizeProc pGetThemeSysSize;
	GetThemeSysFontProc pGetThemeSysFont;
	GetThemeSysStringProc pGetThemeSysString;
	GetThemeSysIntProc pGetThemeSysInt;
	IsThemeActiveProc pIsThemeActive;
	IsAppThemedProc pIsAppThemed;
	GetWindowThemeProc pGetWindowTheme;
	EnableThemeDialogTextureProc pEnableThemeDialogTexture;
	IsThemeDialogTextureEnabledProc pIsThemeDialogTextureEnabled;
	GetThemeAppPropertiesProc pGetThemeAppProperties;
	SetThemeAppPropertiesProc pSetThemeAppProperties;
	GetCurrentThemeNameProc pGetCurrentThemeName;
	GetThemeDocumentationPropertyProc pGetThemeDocumentationProperty;
	DrawThemeParentBackgroundProc pDrawThemeParentBackground;
	EnableThemingProc pEnableTheming;
	DrawThemeBackgroundExProc pDrawThemeBackgroundEx;
public:
	uxtheme_handle(HINSTANCE inst);
	~uxtheme_handle();

	static bool g_create(uxtheme_ptr & p_out);
	HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
	HRESULT CloseThemeData(HTHEME hTheme);
	HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
	HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
		int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
		DWORD dwTextFlags2, const RECT *pRect);
	HRESULT GetThemeBackgroundContentRect(HTHEME hTheme, OPTIONAL HDC hdc, 
		int iPartId, int iStateId,  const RECT *pBoundingRect, 
		OUT RECT *pContentRect);
	HRESULT GetThemeBackgroundExtent(HTHEME hTheme, OPTIONAL HDC hdc,
		int iPartId, int iStateId, const RECT *pContentRect, 
		OUT RECT *pExtentRect);
	HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
		OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz);
	HRESULT GetThemeTextExtent(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
		DWORD dwTextFlags, OPTIONAL const RECT *pBoundingRect, 
		OUT RECT *pExtentRect);
	HRESULT GetThemeTextMetrics(HTHEME hTheme, OPTIONAL HDC hdc, 
		int iPartId, int iStateId, OUT TEXTMETRIC* ptm);
	HRESULT GetThemeBackgroundRegion(HTHEME hTheme, OPTIONAL HDC hdc,  
		int iPartId, int iStateId, const RECT *pRect, OUT HRGN *pRegion);
	HRESULT HitTestThemeBackground(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
		int iStateId, DWORD dwOptions, const RECT *pRect, OPTIONAL HRGN hrgn, 
		POINT ptTest, OUT WORD *pwHitTestCode);
	HRESULT DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
		const RECT *pDestRect, UINT uEdge, UINT uFlags, OPTIONAL OUT RECT *pContentRect);
	HRESULT DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, 
		int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);
	BOOL IsThemePartDefined(HTHEME hTheme, int iPartId, 
		int iStateId);
	BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, 
		int iPartId, int iStateId);
	HRESULT GetThemeColor(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT COLORREF *pColor);
	HRESULT GetThemeMetric(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
		int iStateId, int iPropId, OUT int *piVal);
	HRESULT GetThemeString(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars);
	HRESULT GetThemeBool(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT BOOL *pfVal);
	HRESULT GetThemeInt(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT int *piVal);
	HRESULT GetThemeEnumValue(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT int *piVal);
	HRESULT GetThemePosition(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT POINT *pPoint);
	HRESULT GetThemeFont(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
		int iStateId, int iPropId, OUT LOGFONT *pFont);
	HRESULT GetThemeRect(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT RECT *pRect);
	HRESULT GetThemeMargins(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
		int iStateId, int iPropId, OPTIONAL RECT *prc, OUT MARGINS *pMargins);
	HRESULT GetThemeIntList(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT INTLIST *pIntList);
	HRESULT GetThemePropertyOrigin(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT enum PROPERTYORIGIN *pOrigin);
	HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
		LPCWSTR pszSubIdList);
	HRESULT GetThemeFilename(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId, OUT LPWSTR pszThemeFileName, int cchMaxBuffChars);
	COLORREF GetThemeSysColor(HTHEME hTheme, int iColorId);
	HBRUSH GetThemeSysColorBrush(HTHEME hTheme, int iColorId);
	BOOL GetThemeSysBool(HTHEME hTheme, int iBoolId);
	int GetThemeSysSize(HTHEME hTheme, int iSizeId);
	HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId, OUT LOGFONT *plf);
	HRESULT GetThemeSysString(HTHEME hTheme, int iStringId, 
		OUT LPWSTR pszStringBuff, int cchMaxStringChars);
	HRESULT GetThemeSysInt(HTHEME hTheme, int iIntId, int *piValue);
	BOOL IsThemeActive();
	BOOL IsAppThemed();
	HTHEME GetWindowTheme(HWND hwnd);
	HRESULT EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);
	BOOL IsThemeDialogTextureEnabled(HWND hwnd);
	DWORD GetThemeAppProperties();
	void SetThemeAppProperties(DWORD dwFlags);
	HRESULT GetCurrentThemeName(
		OUT LPWSTR pszThemeFileName, int cchMaxNameChars, 
		OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars,
		OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars);
	HRESULT GetThemeDocumentationProperty(LPCWSTR pszThemeName,
		LPCWSTR pszPropertyName, OUT LPWSTR pszValueBuff, int cchMaxValChars);
	HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc, OPTIONAL RECT* prc);
	HRESULT EnableTheming(BOOL fEnable);
	HRESULT DrawThemeBackgroundEx(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions);
};

#endif