#include "../stdafx.h"
#include "ng_playlist.h"

namespace pvt
{
    bool ng_playlist_view_t::do_drag_drop(WPARAM wp)
    {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
        m_playlist_api->activeplaylist_get_selected_items(data);
        if (data.get_count() > 0)
        {
            static_api_ptr_t<playlist_incoming_item_filter> incoming_api;
            mmh::ComPtr<IDataObject> pDataObject;
            pDataObject.attach(incoming_api->create_dataobject(data));
            if (pDataObject.is_valid())
            {
                //pfc::com_ptr_t<IAsyncOperation> pAsyncOperation;
                //HRESULT hr = pDataObject->QueryInterface(IID_IAsyncOperation, (void**)pAsyncOperation.receive_ptr());
                DWORD blah = DROPEFFECT_NONE;
                {
                    m_dragging = true;
                    m_DataObject = pDataObject;
                    m_dragging_initial_playlist = m_playlist_api->get_active_playlist();
                    HRESULT hr = uih::ole::do_drag_drop(get_wnd(), wp, pDataObject,DROPEFFECT_COPY|DROPEFFECT_MOVE, DROPEFFECT_COPY, &blah);

                    m_dragging=false;
                    m_DataObject.release();
                    m_dragging_initial_playlist = pfc_infinite;

                }
            }
        }
        return true;
    }

    HRESULT STDMETHODCALLTYPE IDropSource_playlist::QueryInterface(REFIID iid,void ** ppvObject)
    {
        if (ppvObject == nullptr) return E_INVALIDARG;
        *ppvObject = nullptr;
        if (iid == IID_IUnknown) {AddRef();*ppvObject = (IUnknown*)this;return S_OK;}
        else if (iid == IID_IDropSource) {AddRef();*ppvObject = (IDropSource*)this;return S_OK;}
        else return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE IDropSource_playlist::AddRef() {return InterlockedIncrement(&refcount);}
    ULONG STDMETHODCALLTYPE IDropSource_playlist::Release()
    {
        LONG rv = InterlockedDecrement(&refcount);
        if (!rv)
        {
            delete this;
        }
        return rv;
    }

    HRESULT STDMETHODCALLTYPE IDropSource_playlist::QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState)
    {
        if (fEscapePressed || ((m_initial_key_state & MK_LBUTTON) && (grfKeyState&MK_RBUTTON)) ) {return DRAGDROP_S_CANCEL;}
        else if ( ((m_initial_key_state & MK_LBUTTON) && !(grfKeyState&MK_LBUTTON ))
            || ((m_initial_key_state & MK_RBUTTON) && !(grfKeyState&MK_RBUTTON )))
        {
            return DRAGDROP_S_DROP;
        }
        else return S_OK;
    }

    HRESULT STDMETHODCALLTYPE IDropSource_playlist::GiveFeedback(DWORD dwEffect)
    {
        return DRAGDROP_S_USEDEFAULTCURSORS;
    }

    IDropSource_playlist::IDropSource_playlist(ng_playlist_view_t * playlist, DWORD initial_key_state) : refcount(0), p_playlist(playlist),
        m_initial_key_state(initial_key_state) {};

}