#include "pch.h"
#include "mw_drop_target.h"

#include "main_window.h"
#include "rebar.h"
#include "status_pane.h"

extern HWND g_status;

bool g_last_rmb = false;

MainWindowDropTarget::MainWindowDropTarget()
{
    m_DropTargetHelper = wil::CoCreateInstanceNoThrow<IDropTargetHelper>(CLSID_DragDropHelper);
}

bool MainWindowDropTarget::check_window_allowed(HWND wnd)
{
    return wnd
        && (wnd == cui::main_window.get_wnd() || wnd == cui::rebar::g_rebar
            || (cui::rebar::g_rebar && IsChild(cui::rebar::g_rebar, wnd))
            || wnd == cui::status_pane::g_status_pane.get_wnd()
            || (cui::status_pane::g_status_pane.get_wnd() && IsChild(cui::status_pane::g_status_pane.get_wnd(), wnd))
            || wnd == g_status);
}

HRESULT STDMETHODCALLTYPE MainWindowDropTarget::Drop(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    m_DataObject.reset();

    POINT pt = {ptl.x, ptl.y};

    if (m_DropTargetHelper)
        m_DropTargetHelper->Drop(pDataObj, &pt, *pdwEffect);

    const auto playlist_api = playlist_manager::get();
    const auto incoming_api = playlist_incoming_item_filter::get();

    HWND wnd = WindowFromPoint(pt);
    bool is_allowed_window = check_window_allowed(wnd);

    *pdwEffect = is_allowed_window ? DROPEFFECT_COPY : DROPEFFECT_NONE;

    if (ui_drop_item_callback::g_on_drop(pDataObj) || !is_allowed_window)
        return S_OK;

    if (g_last_rmb) {
        enum {
            ID_DROP = 1,
            ID_CANCEL
        };

        HMENU menu = CreatePopupMenu();

        uAppendMenu(menu, (MF_STRING), ID_DROP, "&Add files here");
        uAppendMenu(menu, MF_SEPARATOR, 0, nullptr);
        uAppendMenu(menu, MF_STRING, ID_CANCEL, "&Cancel");

        int cmd = TrackPopupMenu(
            menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, cui::main_window.get_wnd(), nullptr);
        DestroyMenu(menu);

        switch (cmd) {
        case ID_DROP:
            break;
        default:
            return S_OK;
        }
    }

    metadb_handle_list data;

    incoming_api->process_dropped_files(pDataObj, data, true, cui::main_window.get_wnd());

    playlist_api->activeplaylist_undo_backup();
    playlist_api->activeplaylist_clear_selection();
    playlist_api->activeplaylist_add_items(data, bit_array_true());

    return S_OK;
}

HRESULT STDMETHODCALLTYPE MainWindowDropTarget::DragLeave()
{
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragLeave();
    uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_INVALID, "", "");

    m_DataObject.reset();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MainWindowDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    POINT pt = {ptl.x, ptl.y};
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragOver(&pt, *pdwEffect);

    g_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

    HWND wnd = WindowFromPoint(pt);
    bool is_allowed_window = check_window_allowed(wnd);

    if (ui_drop_item_callback::g_is_accepted_type(m_DataObject.get(), pdwEffect))
        return S_OK;

    if (is_allowed_window && playlist_incoming_item_filter::get()->process_dropped_files_check(m_DataObject.get())) {
        *pdwEffect = DROPEFFECT_COPY;

        pfc::string8 name;
        playlist_manager::get()->activeplaylist_get_name(name);
        uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_COPY, "Add to %1", name);
    } else {
        *pdwEffect = DROPEFFECT_NONE;
        uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_INVALID, "", "");
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE MainWindowDropTarget::DragEnter(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    POINT pt = {ptl.x, ptl.y};
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragEnter(cui::main_window.get_wnd(), pDataObj, &pt, *pdwEffect);

    m_DataObject = pDataObj;

    g_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

    HWND wnd = WindowFromPoint(pt);

    if (ui_drop_item_callback::g_is_accepted_type(pDataObj, pdwEffect))
        return S_OK;

    const auto is_allowed_window = check_window_allowed(wnd);
    if (is_allowed_window && playlist_incoming_item_filter::get()->process_dropped_files_check(pDataObj)) {
        *pdwEffect = DROPEFFECT_COPY;
        pfc::string8 name;
        playlist_manager::get()->activeplaylist_get_name(name);
        uih::ole::set_drop_description(pDataObj, DROPIMAGE_COPY, "Add to %1", name);
    } else {
        *pdwEffect = DROPEFFECT_NONE;
        uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_INVALID, "", "");
    }

    return S_OK;
}

ULONG STDMETHODCALLTYPE MainWindowDropTarget::Release()
{
    LONG rv = InterlockedDecrement(&drop_ref_count);
    if (!rv) {
#ifdef _DEBUG
        OutputDebugString(_T("deleting drop_handler_interface"));
#endif
        delete this;
    }
    return rv;
}

ULONG STDMETHODCALLTYPE MainWindowDropTarget::AddRef()
{
    return InterlockedIncrement(&drop_ref_count);
}

HRESULT STDMETHODCALLTYPE MainWindowDropTarget::QueryInterface(REFIID riid, LPVOID FAR* ppvObject)
{
    if (ppvObject == nullptr)
        return E_INVALIDARG;
    *ppvObject = nullptr;
    if (riid == IID_IUnknown) {
        AddRef();
        *ppvObject = (IUnknown*)this;
        return S_OK;
    }
    if (riid == IID_IDropTarget) {
        AddRef();
        *ppvObject = (IDropTarget*)this;
        return S_OK;
    }
    return E_NOINTERFACE;
}
