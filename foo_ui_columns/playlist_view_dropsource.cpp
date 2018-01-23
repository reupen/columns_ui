#include "stdafx.h"
#include "playlist_view.h"

HRESULT STDMETHODCALLTYPE IDropSource_playlist::QueryInterface(REFIID iid, void** ppvObject)
{
    if (ppvObject == nullptr)
        return E_INVALIDARG;
    *ppvObject = nullptr;
    if (iid == IID_IUnknown) {
        AddRef();
        *ppvObject = (IUnknown*)this;
        return S_OK;
    } else if (iid == IID_IDropSource) {
        AddRef();
        *ppvObject = (IDropSource*)this;
        return S_OK;
    } else
        return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE IDropSource_playlist::AddRef()
{
    return InterlockedIncrement(&refcount);
}

ULONG STDMETHODCALLTYPE IDropSource_playlist::Release()
{
    LONG rv = InterlockedDecrement(&refcount);
    if (!rv) {
#ifdef _DEBUG
        OutputDebugString(_T("deleting IDropSource_playlist"));
#endif
        delete this;
    }
    return rv;
}

HRESULT STDMETHODCALLTYPE IDropSource_playlist::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if (fEscapePressed || (p_playlist->m_rmb_is_dragging && grfKeyState & MK_LBUTTON)
        || (!p_playlist->m_rmb_is_dragging && ((grfKeyState & MK_RBUTTON) || !(grfKeyState & MK_CONTROL)))) {
        return DRAGDROP_S_CANCEL;
    } else if ((p_playlist->m_rmb_is_dragging && !(grfKeyState & MK_RBUTTON))
        || (!p_playlist->m_rmb_is_dragging && !(grfKeyState & MK_LBUTTON))) {
        return DRAGDROP_S_DROP;
    } else
        return S_OK;
}

HRESULT STDMETHODCALLTYPE IDropSource_playlist::GiveFeedback(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}

IDropSource_playlist::IDropSource_playlist(playlist_view* playlist) : refcount(0), p_playlist(playlist) {}
