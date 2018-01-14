#include "stdafx.h"
#include "status_pane.h"
#include "mw_drop_target.h"

bool g_last_rmb = false;

drop_handler_interface::drop_handler_interface() : drop_ref_count(0)
{
    last_over.x = 0; last_over.y = 0;
    m_DropTargetHelper.instantiate(CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER);
}

bool drop_handler_interface::check_window_allowed(HWND wnd)
{
    return wnd && (wnd == g_main_window || wnd == g_rebar || (g_rebar && IsChild(g_rebar, wnd)) || wnd == g_status_pane.get_wnd() 
        || (g_status_pane.get_wnd() && IsChild(g_status_pane.get_wnd(), wnd)) || wnd == g_status);
}

HRESULT STDMETHODCALLTYPE drop_handler_interface::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    POINT pt = { ptl.x, ptl.y };
    if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->Drop(pDataObj, &pt, *pdwEffect);

    static_api_ptr_t<playlist_manager> playlist_api;
    static_api_ptr_t<playlist_incoming_item_filter> incoming_api;

    HWND wnd = WindowFromPoint(pt);
    bool is_allowed_window = check_window_allowed(wnd);

    *pdwEffect = is_allowed_window ? DROPEFFECT_COPY : DROPEFFECT_NONE;

    bool process = !ui_drop_item_callback::g_on_drop(pDataObj) && is_allowed_window;

    if (process && g_last_rmb)
    {
        process = false;
        enum { ID_DROP = 1, ID_CANCEL };

        HMENU menu = CreatePopupMenu();

        uAppendMenu(menu, (MF_STRING), ID_DROP, "&Add files here");
        uAppendMenu(menu, MF_SEPARATOR, 0, nullptr);
        uAppendMenu(menu, MF_STRING, ID_CANCEL, "&Cancel");

        int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, g_main_window, nullptr);
        DestroyMenu(menu);

        if (cmd)
        {
            switch (cmd)
            {
            case ID_DROP:
                process = true;
                break;
            }
        }
    }

    if (process)
    {
        metadb_handle_list data;

        incoming_api->process_dropped_files(pDataObj, data, true, g_main_window);

        bool send_new_playlist = false;

        int idx = -1;

        playlist_api->activeplaylist_undo_backup();
        playlist_api->activeplaylist_clear_selection();
        playlist_api->activeplaylist_insert_items(idx, data, pfc::bit_array_true());

        data.remove_all();
    }


    if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragLeave();
    uih::ole::set_drop_description(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");

    m_DataObject.release();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE drop_handler_interface::DragLeave()
{
    if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragLeave();
    uih::ole::set_drop_description(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");

    last_over.x = 0;
    last_over.y = 0;
    m_DataObject.release();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE drop_handler_interface::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    POINT pt = { ptl.x, ptl.y };
    if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragOver(&pt, *pdwEffect);

    g_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

    HWND wnd = WindowFromPoint(pt);
    bool is_allowed_window = check_window_allowed(wnd);
    bool uid_handled = ui_drop_item_callback::g_is_accepted_type(m_DataObject, pdwEffect);

    //if (last_over.x != pt.x || last_over.y != pt.y)
    if (!uid_handled)
    {
        if (is_allowed_window && static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check(m_DataObject))
        {
            *pdwEffect = DROPEFFECT_COPY;

            pfc::string8 name;
            static_api_ptr_t<playlist_manager>()->activeplaylist_get_name(name);
            uih::ole::set_drop_description(m_DataObject, DROPIMAGE_COPY, "Add to %1", name);
        }
        else
        {
            *pdwEffect = DROPEFFECT_NONE;
            uih::ole::set_drop_description(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");
        }
    }

    last_over = ptl;


    return S_OK;
}

HRESULT STDMETHODCALLTYPE drop_handler_interface::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    POINT pt = { ptl.x, ptl.y };
    if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragEnter(g_main_window, pDataObj, &pt, *pdwEffect);

    m_DataObject = pDataObj;

    last_over.x = 0;
    last_over.y = 0;
    g_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

    HWND wnd = WindowFromPoint(pt);
    bool is_allowed_window = check_window_allowed(wnd);
    bool uid_handled = ui_drop_item_callback::g_is_accepted_type(pDataObj, pdwEffect);

    if (!uid_handled)
    {
        if (is_allowed_window && static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check(pDataObj))
        {
            *pdwEffect = DROPEFFECT_COPY;
            pfc::string8 name;
            static_api_ptr_t<playlist_manager>()->activeplaylist_get_name(name);
            uih::ole::set_drop_description(pDataObj, DROPIMAGE_COPY, "Add to %1", name);
        }
        else
        {
            *pdwEffect = DROPEFFECT_NONE;
            uih::ole::set_drop_description(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");
        }
    }
    return S_OK; //??
}

ULONG STDMETHODCALLTYPE drop_handler_interface::Release()
{
    LONG rv = InterlockedDecrement(&drop_ref_count);
    if (!rv)
    {
#ifdef _DEBUG
        OutputDebugString(_T("deleting drop_handler_interface"));
#endif
        delete this;
    }
    return rv;
}

ULONG STDMETHODCALLTYPE drop_handler_interface::AddRef()
{
    return InterlockedIncrement(&drop_ref_count);
}

HRESULT STDMETHODCALLTYPE drop_handler_interface::QueryInterface(REFIID riid, LPVOID FAR *ppvObject)
{
    if (ppvObject == nullptr) return E_INVALIDARG;
    *ppvObject = nullptr;
    if (riid == IID_IUnknown) { AddRef(); *ppvObject = (IUnknown*)this; return S_OK; }
    else if (riid == IID_IDropTarget) { AddRef(); *ppvObject = (IDropTarget*)this; return S_OK; }
    else return E_NOINTERFACE;
}

