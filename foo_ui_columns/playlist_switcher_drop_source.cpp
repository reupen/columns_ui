#include "stdafx.h"
#include "playlist_switcher_v2.h"

namespace cui::panels::playlist_switcher {

bool PlaylistSwitcher::do_drag_drop(WPARAM wp)
{
    bit_array_bittable mask(get_item_count());
    get_selection_state(mask);

    playlist_dataobject_desc_impl data;
    data.set_from_playlist_manager(mask);

    pfc::com_ptr_t<IDataObject> pDataObject = ole_interaction_v2::get()->create_dataobject(data);

    if (pDataObject.is_valid()) {
        DWORD blah = DROPEFFECT_NONE;
        m_dragging = true;
        m_DataObject = pDataObject.get_ptr();
        HRESULT hr = uih::ole::do_drag_drop(
            get_wnd(), wp, pDataObject.get_ptr(), DROPEFFECT_COPY | DROPEFFECT_MOVE, DROPEFFECT_COPY, &blah);
        m_DataObject.reset();
        m_dragging = false;
    }
    return true;
}

HRESULT STDMETHODCALLTYPE PlaylistSwitcher::DropSource::QueryInterface(REFIID iid, void** ppvObject)
{
    if (ppvObject == nullptr)
        return E_INVALIDARG;
    *ppvObject = nullptr;
    if (iid == IID_IUnknown) {
        AddRef();
        *ppvObject = (IUnknown*)this;
        return S_OK;
    }
    if (iid == IID_IDropSource) {
        AddRef();
        *ppvObject = (IDropSource*)this;
        return S_OK;
    }
    return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE PlaylistSwitcher::DropSource::AddRef()
{
    return InterlockedIncrement(&refcount);
}
ULONG STDMETHODCALLTYPE PlaylistSwitcher::DropSource::Release()
{
    LONG rv = InterlockedDecrement(&refcount);
    if (!rv) {
        delete this;
    }
    return rv;
}

HRESULT STDMETHODCALLTYPE PlaylistSwitcher::DropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if (fEscapePressed || ((m_initial_key_state & MK_LBUTTON) && (grfKeyState & MK_RBUTTON))) {
        return DRAGDROP_S_CANCEL;
    }
    if (((m_initial_key_state & MK_LBUTTON) && !(grfKeyState & MK_LBUTTON))
        || ((m_initial_key_state & MK_RBUTTON) && !(grfKeyState & MK_RBUTTON))) {
        return DRAGDROP_S_DROP;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PlaylistSwitcher::DropSource::GiveFeedback(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}

PlaylistSwitcher::DropSource::DropSource(PlaylistSwitcher* p_window, DWORD initial_key_state)
    : refcount(0)
    , m_window(p_window)
    , m_initial_key_state(initial_key_state)
{
}

} // namespace cui::panels::playlist_switcher
