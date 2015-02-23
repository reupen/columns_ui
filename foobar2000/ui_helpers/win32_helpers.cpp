#include "stdafx.h"

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

namespace win32 {
	const RECT rect_null = {0,0,0,0};
}


bool g_test_os_version(DWORD major, DWORD minor)
{
	static OSVERSIONINFO ov;
	static bool blah = false;

	if (!blah)
	{
		ov.dwOSVersionInfoSize = sizeof(ov);
		GetVersionEx(&ov);
	}
	return ov.dwMajorVersion >= major && ov.dwMinorVersion >= minor;
}

bool is_vista_or_newer()
{
	static OSVERSIONINFO ov;
	static bool blah = false;

	if (!blah)
	{
		ov.dwOSVersionInfoSize = sizeof(ov);
		GetVersionEx(&ov);
	}
	return (ov.dwMajorVersion >= 6);
}

void g_set_listview_window_explorer_theme(HWND wnd)
{
	if (g_test_os_version(6,0))
	{
		ListView_SetExtendedListViewStyleEx(wnd, 0x00010000|LVS_EX_FULLROWSELECT, 0x00010000|LVS_EX_FULLROWSELECT);
		if (g_test_os_version(6,1))
		{
			SetWindowTheme(wnd, L"ItemsView", NULL);
			//SetWindowTheme(ListView_GetHeader (wnd), L"ItemsView", NULL);
		}
		else
		{
			SetWindowTheme(wnd, L"Explorer", NULL);
		}
	}
}

bool g_keyboard_cues_enabled()
{
	BOOL a = true;
	SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &a, 0);
	return a != 0;
}

/*void g_show_keyboard_cues(HWND wnd, bool b_focus, bool b_accel)
{
SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_
}*/

#define TVS_EX_UNKNOWN               0x0001 //
#define TVS_EX_MULTISELECT          0x0002
#define TVS_EX_DOUBLEBUFFER         0x0004 //
#define TVS_EX_NOINDENTSTATE        0x0008
#define TVS_EX_RICHTOOLTIP          0x0010 //
#define TVS_EX_AUTOHSCROLL          0x0020 //
#define TVS_EX_FADEINOUTEXPANDOS    0x0040 //
#define TVS_EX_PARTIALCHECKBOXES    0x0080
#define TVS_EX_EXCLUSIONCHECKBOXES  0x0100
#define TVS_EX_DIMMEDCHECKBOXES     0x0200
#define TVS_EX_DRAWIMAGEASYNC       0x0400 //

void g_set_treeview_window_explorer_theme(HWND wnd, bool b_reduce_indent)
{
	if (is_vista_or_newer())
	{
		UINT_PTR stylesex = /*TVS_EX_FADEINOUTEXPANDOS|*/TVS_EX_DOUBLEBUFFER|TVS_EX_AUTOHSCROLL;
		UINT_PTR styles = NULL;//TVS_TRACKSELECT;

		SendMessage(wnd, TV_FIRST + 44, stylesex, stylesex);
		SetWindowTheme(wnd, L"Explorer", NULL);
		SetWindowLongPtr(wnd, GWL_STYLE, (GetWindowLongPtr(wnd, GWL_STYLE) & ~(TVS_HASLINES/*|TVS_NOHSCROLL*/))|styles);
		if (b_reduce_indent)
			TreeView_SetIndent(wnd, 0xa);
	}

}

void g_remove_treeview_window_explorer_theme(HWND wnd)
{
	if (is_vista_or_newer())
	{
		UINT_PTR stylesex = /*TVS_EX_FADEINOUTEXPANDOS|*/TVS_EX_DOUBLEBUFFER;
		UINT_PTR styles = NULL;//TVS_TRACKSELECT;

		SendMessage(wnd, TV_FIRST + 44, stylesex, NULL);
		SetWindowTheme(wnd, L"", NULL);
		SetWindowLongPtr(wnd, GWL_STYLE, (GetWindowLongPtr(wnd, GWL_STYLE)|TVS_HASLINES));
	}

}

int ListView_InsertColumnText(HWND wnd_lv, UINT index, const TCHAR * text, int cx)
{
	LVCOLUMN lvc;
	memset(&lvc, 0, sizeof(LVCOLUMN));
	lvc.mask = LVCF_TEXT|LVCF_WIDTH;

	lvc.pszText = const_cast<TCHAR*>(text);
	lvc.cx = cx;
	return ListView_InsertColumn(wnd_lv, index, &lvc);
}

LRESULT ListView_InsertItemText(HWND wnd_lv, UINT item, UINT subitem, const TCHAR * text, bool b_set, LPARAM lp, int image_index)
{
	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.mask=LVIF_TEXT| (b_set?0:LVIF_PARAM);
	lvi.iItem = item;
	lvi.iSubItem = subitem;
	lvi.pszText = const_cast<TCHAR*>(text);
	lvi.lParam = lp;
	if (image_index != I_IMAGENONE )
	{
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = image_index;
	}
	return b_set ? ListView_SetItem(wnd_lv, &lvi) : ListView_InsertItem(wnd_lv, &lvi);
}

LRESULT ListView_InsertItemText(HWND wnd_lv, UINT item, UINT subitem, const char * text, bool b_set, LPARAM lp, int image_index)
{
	pfc::stringcvt::string_os_from_utf8 wide(text);
	return ListView_InsertItemText(wnd_lv, item, subitem, const_cast<TCHAR*>(wide.get_ptr()), b_set, lp, image_index);
}
HTREEITEM uTreeView_InsertItemSimple(HWND wnd_tree, const char * sz_text, LPARAM data, DWORD state, HTREEITEM ti_parent, HTREEITEM ti_after, bool b_image, UINT image, UINT integral_height)
{
	return uTreeView_InsertItemSimple(wnd_tree, pfc::stringcvt::string_os_from_utf8(sz_text), data, state, ti_parent, ti_after, b_image, image, integral_height);
}

HTREEITEM uTreeView_InsertItemSimple(HWND wnd_tree, const WCHAR * sz_text, LPARAM data, DWORD state, HTREEITEM ti_parent, HTREEITEM ti_after, bool b_image, UINT image, UINT integral_height)
{
	TVINSERTSTRUCT is;
	memset(&is,0,sizeof(is));
	is.hParent = ti_parent;
	is.hInsertAfter = ti_after;
	is.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE|(b_image?TVIF_IMAGE|TVIF_SELECTEDIMAGE:NULL)|(integral_height>1?TVIF_INTEGRAL:NULL);
	is.item.pszText = const_cast<WCHAR*>(sz_text);
	is.item.state = state;
	is.item.stateMask = state;
	is.item.lParam = data;
	is.item.iImage = image;
	is.item.iSelectedImage = image;
	is.itemex.iIntegral = integral_height;
	return TreeView_InsertItem(wnd_tree,&is);
}

t_size uTreeView_GetChildIndex(HWND wnd_tv, HTREEITEM ti)
{
	HTREEITEM item = ti;
	unsigned n=0;
	while (item = TreeView_GetPrevSibling(wnd_tv, item))
		n++;
	return n;
}

namespace win32_helpers
{
	BOOL
		FileTimeToLocalFileTime2(
		__in  CONST FILETIME *lpFileTime,
		__out LPFILETIME lpLocalFileTime
		)
	{
		SYSTEMTIME stUTC, stLocal;
		memset(&stUTC, 0,sizeof(stUTC));
		memset(&stLocal, 0,sizeof(stLocal));

		FileTimeToSystemTime(lpFileTime, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		return SystemTimeToFileTime(&stLocal, lpLocalFileTime);
	}
	FILETIME filetimestamp_to_FileTime(t_filetimestamp time) 
	{
		FILETIME ret;
		ret.dwLowDateTime = (DWORD)(time & 0xFFFFFFFF);
		ret.dwHighDateTime= (DWORD)(time >> 32);
		return ret;
	}
	void format_date(t_filetimestamp time, std::basic_string<TCHAR> & str, bool b_convert_to_local)
	{
		FILETIME ft1 = filetimestamp_to_FileTime(time), ft2 = ft1;
		if (b_convert_to_local)
			FileTimeToLocalFileTime2(&ft1, &ft2);
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft2, &st);
		int size = GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, NULL);
		int size2 = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, NULL);
		pfc::array_t<TCHAR> buf, buf2;
		buf.set_size(size);
		buf2.set_size(size2);
		GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, buf.get_ptr(), size);
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, buf2.get_ptr(), size2);
		str = _T("");
		str += buf.get_ptr();
		str += _T(" ");
		str += buf2.get_ptr();
	}
	HRESULT get_comctl32_version(DLLVERSIONINFO2 & p_dvi)
	{
		static bool have_version = false;
		static HRESULT rv = E_FAIL;

		static DLLVERSIONINFO2 g_dvi;

		if (!have_version)
		{
			HINSTANCE hinstDll = LoadLibrary(_T("comctl32.dll"));

			if(hinstDll)
			{
				DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");


				if(pDllGetVersion)
				{

					memset(&g_dvi, 0, sizeof(DLLVERSIONINFO2));
					g_dvi.info1.cbSize = sizeof(DLLVERSIONINFO2);

					rv = (*pDllGetVersion)(&g_dvi.info1);

					if (FAILED(rv))
					{
						memset(&g_dvi, 0, sizeof(DLLVERSIONINFO));
						g_dvi.info1.cbSize = sizeof(DLLVERSIONINFO);

						rv = (*pDllGetVersion)(&g_dvi.info1);
					}
				}

				FreeLibrary(hinstDll);
			}
			have_version = true;
		}
		p_dvi = g_dvi;
		return rv;
	}

	BOOL ShellNotifyIconSimple(DWORD dwMessage,HWND wnd,UINT id,UINT callbackmsg,HICON icon,
		const char * tip,const char * balloon_title,const char * balloon_msg)
	{
		//param_utf16_from_utf8 wtip(tip), wbtitle(balloon_title), wbmsg(balloon_msg);
		NOTIFYICONDATA nid;
		memset(&nid, 0, sizeof(nid));

		nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		nid.hWnd = wnd;
		nid.uID = id;
		nid.uCallbackMessage = callbackmsg;
		nid.hIcon = icon;
		nid.uFlags = NIF_ICON|NIF_TIP|NIF_MESSAGE|(balloon_msg?NIF_INFO:NULL);
		if (tip)
			wcscpy_s(nid.szTip, pfc::stringcvt::string_wide_from_utf8(tip).get_ptr());
		if (balloon_title)
			wcscpy_s(nid.szInfoTitle, pfc::stringcvt::string_wide_from_utf8(balloon_title).get_ptr());
		if (balloon_msg)
			wcscpy_s(nid.szInfo, pfc::stringcvt::string_wide_from_utf8(balloon_msg).get_ptr());
		nid.uTimeout = 10000;
		nid.dwInfoFlags = NIIF_INFO;

		return Shell_NotifyIcon(dwMessage, &nid);


	}

		BOOL run_action(DWORD action,NOTIFYICONDATA * data)
	{
		if (Shell_NotifyIcon(action,data)) return TRUE;
		if (action==NIM_MODIFY)
		{
			if (Shell_NotifyIcon(NIM_ADD,data)) return TRUE;
		}
		return FALSE;
	}


	BOOL uShellNotifyIcon(DWORD action,HWND wnd,UINT id,UINT version,UINT callbackmsg,HICON icon,const char * tip)
	{
		NOTIFYICONDATA nid;
		memset(&nid,0,sizeof(nid));
		nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		nid.hWnd = wnd;
		nid.uID = id;
		nid.uFlags = 0;
		if (action & NIM_SETVERSION)
			nid.uVersion = version;
		if (callbackmsg)
		{
			nid.uFlags |= NIF_MESSAGE;
			nid.uCallbackMessage = callbackmsg;
		}
		if (icon)
		{
			nid.uFlags |= NIF_ICON;
			nid.hIcon = icon;
		}			
		if (tip)
		{
			nid.uFlags |= NIF_TIP;
			_tcsncpy(nid.szTip,pfc::stringcvt::string_os_from_utf8(tip),tabsize(nid.szTip)-1);
		}

		return run_action(action,&nid);
	}

	BOOL uShellNotifyIconEx(DWORD action,HWND wnd,UINT id,UINT callbackmsg,HICON icon,const char * tip,const char * balloon_title,const char * balloon_msg)
	{

		NOTIFYICONDATA nid;
		memset(&nid,0,sizeof(nid));
		nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		nid.hWnd = wnd;
		nid.uID = id;
		if (callbackmsg)
		{
			nid.uFlags |= NIF_MESSAGE;
			nid.uCallbackMessage = callbackmsg;
		}
		if (icon)
		{
			nid.uFlags |= NIF_ICON;
			nid.hIcon = icon;
		}			
		if (tip)
		{
			nid.uFlags |= NIF_TIP;
			_tcsncpy(nid.szTip,pfc::stringcvt::string_os_from_utf8(tip),tabsize(nid.szTip)-1);
		}

		nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
		//if (balloon_title || balloon_msg)
		{
			nid.uFlags |= NIF_INFO;
			if (balloon_title) _tcsncpy(nid.szInfoTitle,pfc::stringcvt::string_os_from_utf8(balloon_title),tabsize(nid.szInfoTitle)-1);
			if (balloon_msg) _tcsncpy(nid.szInfo,pfc::stringcvt::string_os_from_utf8(balloon_msg),tabsize(nid.szInfo)-1);	
		}
		return run_action(action,reinterpret_cast<NOTIFYICONDATA*>(&nid));


	}
	int ComboBox_AddStringData(HWND wnd, const TCHAR * str, LPARAM data)
	{
		int index = ComboBox_AddString(wnd, str);
		ComboBox_SetItemData(wnd, index, data);
		return index;
	}

	void RegisterShellHookWindowHelper(HWND wnd)
	{
		typedef BOOL(WINAPI * RegisterShellHookWindowProc)(HWND);
		HINSTANCE inst = LoadLibrary(L"user32.dll");
		if (inst)
		{
			RegisterShellHookWindowProc pRegisterShellHookWindow = (RegisterShellHookWindowProc)GetProcAddress(inst, "RegisterShellHookWindow");
			if (pRegisterShellHookWindow)
				pRegisterShellHookWindow(wnd);
			FreeLibrary(inst);
		}
	}

	void DeregisterShellHookWindowHelper(HWND wnd)
	{
		typedef BOOL(WINAPI * DeregisterShellHookWindowProc)(HWND);
		HINSTANCE inst = LoadLibrary(L"user32.dll");
		if (inst)
		{
			DeregisterShellHookWindowProc pDeregisterShellHookWindow = (DeregisterShellHookWindowProc)GetProcAddress(inst, "DeregisterShellHookWindow");
			if (pDeregisterShellHookWindow)
				pDeregisterShellHookWindow(wnd);
			FreeLibrary(inst);
		}
	}

}

namespace ui_helpers
{
	void innerWMPaintModernBackground (HWND wnd, HWND wnd_button) 
	{ 
		PAINTSTRUCT ps; 
		HDC dc = BeginPaint(wnd, &ps); 
		if (dc) 
		{ 
			RECT rc_client, rc_button; 
			GetClientRect(wnd, &rc_client); 
			RECT rc_fill = rc_client; 
			if (wnd_button) 
			{ 
				GetWindowRect(wnd_button, &rc_button); 
				rc_fill.bottom -= RECT_CY(rc_button)+11; 
				rc_fill.bottom -= 11; 
			} 
			FillRect(dc, &rc_fill, GetSysColorBrush(COLOR_WINDOW)); 
			if (wnd_button) 
			{ 
				rc_fill.top=rc_fill.bottom; 
				rc_fill.bottom+=1; 
				FillRect(dc, &rc_fill, GetSysColorBrush(COLOR_3DLIGHT)); 
			} 
			rc_fill.top = rc_fill.bottom; 
			rc_fill.bottom = rc_client.bottom; 
			FillRect(dc, &rc_fill, GetSysColorBrush(COLOR_3DFACE)); 
			EndPaint(wnd, &ps); 
		} 
	} 

}

namespace mmh { namespace ole {

	CLIPFORMAT ClipboardFormatDropDescription()
	{
		static CLIPFORMAT cfRet = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DROPDESCRIPTION);
		return cfRet;
	}

	HRESULT SetBlob(IDataObject *pdtobj, CLIPFORMAT cf, const void *pvBlob, UINT cbBlob)
	{
		HRESULT hr = E_OUTOFMEMORY;
		void *pv = GlobalAlloc(GPTR, cbBlob);
		if (pv)
		{
			CopyMemory(pv, pvBlob, cbBlob);

			FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

			// The STGMEDIUM structure is used to define how to handle a global memory transfer. 
			// This structure includes a flag, tymed, which indicates the medium 
			// to be used, and a union comprising pointers and a handle for getting whichever 
			// medium is specified in tymed.
			STGMEDIUM medium = {0};
			medium.tymed = TYMED_HGLOBAL;
			medium.hGlobal = pv;

			hr = pdtobj->SetData(&fmte, &medium, TRUE);
			if (FAILED(hr))
			{
				GlobalFree(pv);
			}
		}
		return hr;
	}
	HRESULT SetDropDescription(IDataObject *pdtobj, DROPIMAGETYPE dit, const char * msg, const char * insert)
	{
		if (osversion::is_windows_vista_or_newer())
		{
			DROPDESCRIPTION dd;
			dd.type = dit;
			wcscpy_s(dd.szMessage, pfc::stringcvt::string_os_from_utf8(msg).get_ptr());
			wcscpy_s(dd.szInsert, pfc::stringcvt::string_os_from_utf8(insert).get_ptr());
			return SetBlob(pdtobj, ClipboardFormatDropDescription(), &dd, sizeof(dd));
		}
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE IDropSource_Generic::QueryInterface(REFIID iid,void ** ppvObject)
	{
		if (ppvObject == NULL) return E_INVALIDARG;
		*ppvObject = NULL;
		if (iid == IID_IUnknown) {AddRef();*ppvObject = (IUnknown*)this;return S_OK;}
		else if (iid == IID_IDropSource) {AddRef();*ppvObject = (IDropSource*)this;return S_OK;}
		else return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE IDropSource_Generic::AddRef() {return InterlockedIncrement(&refcount);}
	ULONG STDMETHODCALLTYPE IDropSource_Generic::Release()
	{
		LONG rv = InterlockedDecrement(&refcount);
		if (!rv)
		{
			delete this;
		}
		return rv;
	}

	HRESULT STDMETHODCALLTYPE IDropSource_Generic::QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState)
	{
		if (fEscapePressed || ((m_initial_key_state & MK_LBUTTON) && (grfKeyState&MK_RBUTTON)) ) {return DRAGDROP_S_CANCEL;}
		else if ( ((m_initial_key_state & MK_LBUTTON) && !(grfKeyState&MK_LBUTTON ))
			|| ((m_initial_key_state & MK_RBUTTON) && !(grfKeyState&MK_RBUTTON )))
		{
			return DRAGDROP_S_DROP;
		}
		else return S_OK;
	}

	HRESULT STDMETHODCALLTYPE IDropSource_Generic::GiveFeedback(DWORD dwEffect)
	{
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}

//#define FOOBAR2000_NOT_BROKE

	IDropSource_Generic::IDropSource_Generic(HWND wnd, IDataObject * pDataObj, DWORD initial_key_state, bool b_allowdropdescriptiontext)
		: refcount(0), m_initial_key_state(initial_key_state) 
	{
		if (b_allowdropdescriptiontext)
		{
			if (SUCCEEDED(m_DragSourceHelper.instantiate(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER)))
			{
				mmh::comptr_t<IDragSourceHelper2> pDragSourceHelper2 = m_DragSourceHelper;
				if (pDragSourceHelper2.is_valid())
				{
					pDragSourceHelper2->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);
				}
	#if 0
				HDC dc = CreateCompatibleDC(NULL);
				HBITMAP bm = CreateCompatibleBitmap(NULL, 50, 50);
				HBITMAP bm_old = SelectBitmap(dc, bm);
				RECT rc = {0, 0, 50, 50};
				FillRect(dc, &rc, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(RGB(255,0,0))));
				SelectBitmap(dc, bm_old);
				DeleteDC(dc);
				SHDRAGIMAGE shdi;
				shdi.crColorKey = 0;
				shdi.hbmpDragImage = bm;
				shdi.ptOffset.x = 0;
				shdi.ptOffset.y = 0;
				shdi.sizeDragImage.cx = 50;
				shdi.sizeDragImage.cy = 50;
				HRESULT hr = m_DragSourceHelper->InitializeFromBitmap(&shdi, pDataObj);
	#else
				m_DragSourceHelper->InitializeFromWindow(wnd, NULL, pDataObj);
	#endif
			}
		}
//#endif
	};

}}