#include "stdafx.h"
#include "playlist_view.h"

HRESULT STDMETHODCALLTYPE IDropTarget_playlist::QueryInterface(REFIID riid, LPVOID FAR* ppvObject)
{
    if (ppvObject == nullptr)
        return E_INVALIDARG;
    *ppvObject = nullptr;
    if (riid == IID_IUnknown) {
        AddRef();
        *ppvObject = (IUnknown*)this;
        return S_OK;
    } else if (riid == IID_IDropTarget) {
        AddRef();
        *ppvObject = (IDropTarget*)this;
        return S_OK;
    } else
        return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE IDropTarget_playlist::AddRef()
{
    return InterlockedIncrement(&drop_ref_count);
}
ULONG STDMETHODCALLTYPE IDropTarget_playlist::Release()
{
    LONG rv = InterlockedDecrement(&drop_ref_count);
    if (!rv) {
#ifdef _DEBUG
        OutputDebugString(_T("deleting IDropTarget_playlist"));
#endif
        delete this;
    }
    return rv;
}

HRESULT STDMETHODCALLTYPE IDropTarget_playlist::DragEnter(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

    *pdwEffect = DROPEFFECT_COPY;
    if (ui_drop_item_callback::g_is_accepted_type(pDataObj, pdwEffect)
        || static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check(pDataObj)) {
        return S_OK;
    }
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE IDropTarget_playlist::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
    last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE IDropTarget_playlist::DragLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IDropTarget_playlist::Drop(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    POINT pti;
    pti.y = pt.y;
    pti.x = pt.x;
    if (p_playlist->wnd_playlist) {
        bool process = !ui_drop_item_callback::g_on_drop(pDataObj);

        if (process && last_rmb) {
            process = false;
            enum { ID_DROP = 1, ID_CANCEL };

            HMENU menu = CreatePopupMenu();

            uAppendMenu(menu, (MF_STRING), ID_DROP, "&Add files here");
            uAppendMenu(menu, MF_SEPARATOR, 0, nullptr);
            uAppendMenu(menu, MF_STRING, ID_CANCEL, "&Cancel");

            int cmd = TrackPopupMenu(
                menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, p_playlist->wnd_playlist, nullptr);
            DestroyMenu(menu);

            if (cmd) {
                switch (cmd) {
                case ID_DROP:
                    process = true;
                    break;
                }
            }
        }

        if (process) {
            metadb_handle_list data;

            // console::info(static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check_if_native(pDataObj)?"native":"not
            // very native?");

            static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files(
                pDataObj, data, true, p_playlist->wnd_playlist);

            int idx = -1;

            if (!cfg_drop_at_end) {
                POINT ptt = pti;
                ScreenToClient(p_playlist->wnd_playlist, &ptt);
                idx = p_playlist->hittest_item(ptt.x, ptt.y, false);
            }

            static_api_ptr_t<playlist_manager> playlist_api;
            playlist_api->activeplaylist_undo_backup();
            playlist_api->activeplaylist_clear_selection();
            playlist_api->activeplaylist_insert_items(idx, data, pfc::bit_array_true());

            data.remove_all();
        }
    }

    return S_OK;
}
IDropTarget_playlist::IDropTarget_playlist(playlist_view* playlist)
    : drop_ref_count(0), last_rmb(false), p_playlist(playlist)
{
}
