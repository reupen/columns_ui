#include "stdafx.h"

HRESULT IDropSource_albumlist::QueryInterface(REFIID iid, void ** ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	*ppvObject = NULL;
	if (iid == IID_IUnknown) { AddRef(); *ppvObject = (IUnknown*)this; return S_OK; }
	else if (iid == IID_IDropSource) { AddRef(); *ppvObject = (IDropSource*)this; return S_OK; }
	else return E_NOINTERFACE;
}

ULONG IDropSource_albumlist::AddRef()
{
	return InterlockedIncrement(&refcount);
}

ULONG IDropSource_albumlist::Release()
{
	ULONG ret = InterlockedDecrement(&refcount);
	if (!ret) delete this;
	return ret;
}

HRESULT IDropSource_albumlist::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if (fEscapePressed || (grfKeyState&MK_RBUTTON)) return DRAGDROP_S_CANCEL;
	else if (!(grfKeyState&MK_LBUTTON))
	{
		if (wnd)
		{
			DWORD pts = GetMessagePos();
			POINT pt = { (short)LOWORD(pts),(short)HIWORD(pts) };
			HWND pwnd = WindowFromPoint(pt);
			if (pwnd == wnd || GetParent(pwnd) == wnd) return DRAGDROP_S_CANCEL;
			else return DRAGDROP_S_DROP;
		}
		else return DRAGDROP_S_CANCEL;
	}
	else return S_OK;
}

HRESULT IDropSource_albumlist::GiveFeedback(DWORD dwEffect)
{
	if (wnd)
	{
		DWORD pts = GetMessagePos();
		POINT pt = { (short)LOWORD(pts),(short)HIWORD(pts) };
		HWND pwnd = WindowFromPoint(pt);
		if (pwnd == wnd || GetParent(pwnd) == wnd)
		{
			SetCursor(LoadCursor(0, MAKEINTRESOURCE(IDC_NO)));
			return S_OK;
		}
	}

	return DRAGDROP_S_USEDEFAULTCURSORS;
}
