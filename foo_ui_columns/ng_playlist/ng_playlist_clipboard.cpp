#include "pch.h"
#include "ng_playlist.h"

namespace playlist_utils {

bool check_clipboard()
{
    const auto api = ole_interaction::get();
    wil::com_ptr_t<IDataObject> data_object;

    if (FAILED(OleGetClipboard(&data_object)))
        return false;

    bool is_native{};
    DWORD dummy = DROPEFFECT_COPY;

    return SUCCEEDED(api->check_dataobject(data_object.get(), dummy, is_native));
}

bool cut()
{
    const auto m_playlist_api = playlist_manager::get();
    const auto api = ole_interaction::get();
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
    const auto m_playlist_api = playlist_manager::get();
    const auto api = ole_interaction::get();
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
    const auto playlist_api = playlist_manager::get();
    const auto ole_api = ole_interaction::get();
    wil::com_ptr_t<IDataObject> pDO;

    if (FAILED(OleGetClipboard(&pDO)))
        return false;

    dropped_files_data_impl data;
    metadb_handle_list handles;
    bool is_native;
    DWORD dummy = DROPEFFECT_COPY;
    HRESULT hr = ole_api->check_dataobject(pDO.get(), dummy, is_native);
    if (FAILED(hr))
        return false;

    hr = ole_api->parse_dataobject(pDO.get(), data);
    if (FAILED(hr))
        return false;

    data.to_handles(handles, !is_native, GetAncestor(wnd, GA_ROOT));
    playlist_api->activeplaylist_undo_backup();
    playlist_api->activeplaylist_clear_selection();
    playlist_api->activeplaylist_insert_items(index, handles, bit_array_true());
    return true;
}

} // namespace playlist_utils
