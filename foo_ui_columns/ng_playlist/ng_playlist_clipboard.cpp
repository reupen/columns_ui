#include "../stdafx.h"
#include "ng_playlist.h"

namespace playlist_utils {

bool check_clipboard()
{
    static_api_ptr_t<ole_interaction> api;
    wil::com_ptr_t<IDataObject> pDO;
    if (FAILED(OleGetClipboard(&pDO)))
        return false;

    bool b_native;
    DWORD dummy = DROPEFFECT_COPY;
    if (FAILED(api->check_dataobject(pDO.get(), dummy, b_native)))
        return false;

    return b_native;
}

bool cut()
{
    static_api_ptr_t<playlist_manager> m_playlist_api;
    static_api_ptr_t<ole_interaction> api;
    metadb_handle_list data;
    m_playlist_api->activeplaylist_undo_backup();
    m_playlist_api->activeplaylist_get_selected_items(data);
    pfc::com_ptr_t<IDataObject> pDO = api->create_dataobject(data);
    if (SUCCEEDED(OleSetClipboard(pDO.get_ptr()))) {
        m_playlist_api->activeplaylist_remove_selection();
        return true;
    }
    return false;
}

bool copy()
{
    static_api_ptr_t<playlist_manager> m_playlist_api;
    static_api_ptr_t<ole_interaction> api;
    metadb_handle_list data;
    m_playlist_api->activeplaylist_get_selected_items(data);
    pfc::com_ptr_t<IDataObject> pDO = api->create_dataobject(data);
    return SUCCEEDED(OleSetClipboard(pDO.get_ptr()));
}

bool paste_at_focused_item(HWND wnd)
{
    auto playlist_api = playlist_manager::get();
    size_t index = playlist_api->activeplaylist_get_focus_item();

    if (index != (std::numeric_limits<size_t>::max)())
        index++;
    return paste(wnd, index);
}

bool paste(HWND wnd, size_t index)
{
    static_api_ptr_t<playlist_manager> playlist_api;
    static_api_ptr_t<ole_interaction> ole_api;
    wil::com_ptr_t<IDataObject> pDO;

    if (FAILED(OleGetClipboard(&pDO)))
        return false;

    dropped_files_data_impl data;
    metadb_handle_list handles;
    bool b_native;
    DWORD dummy = DROPEFFECT_COPY;
    HRESULT hr = ole_api->check_dataobject(pDO.get(), dummy, b_native);
    if (FAILED(hr))
        return false;

    hr = ole_api->parse_dataobject(pDO.get(), data);
    if (FAILED(hr))
        return false;

    data.to_handles(handles, b_native, GetAncestor(wnd, GA_ROOT));
    playlist_api->activeplaylist_undo_backup();
    playlist_api->activeplaylist_clear_selection();
    playlist_api->activeplaylist_insert_items(index, handles, bit_array_true());
    return true;
}

} // namespace playlist_utils
