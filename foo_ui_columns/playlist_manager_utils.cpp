#include "stdafx.h"
#include "playlist_manager_utils.h"
#include "rename_dialog.h"

namespace playlist_manager_utils {

void rename_playlist(size_t index, HWND wnd_parent)
{
    static_api_ptr_t<playlist_manager> playlist_api;
    pfc::string8 current_name;

    if (!playlist_api->playlist_get_name(index, current_name))
        return;

    pfc::string8 title;
    title << "Rename playlist: " << current_name;

    // Note: The playlist could be moved etc. while the dialog is displayed
    playlist_position_reference_tracker playlist_position_tracker(false);
    playlist_position_tracker.m_playlist = index;

    const auto new_name = cui::helpers::show_rename_dialog_box(wnd_parent, title, current_name);

    if (new_name && playlist_position_tracker.m_playlist != (std::numeric_limits<size_t>::max)())
        playlist_api->playlist_rename(playlist_position_tracker.m_playlist, *new_name, new_name->length());
}

bool check_clipboard()
{
    static_api_ptr_t<ole_interaction> api;
    pfc::com_ptr_t<IDataObject> pDO;

    HRESULT hr = OleGetClipboard(pDO.receive_ptr());
    if (FAILED(hr))
        return false;

    hr = api->check_dataobject_playlists(pDO);
    return SUCCEEDED(hr);
}
bool cut(const pfc::bit_array& mask)
{
    static_api_ptr_t<playlist_manager> m_playlist_api;
    static_api_ptr_t<ole_interaction> api;
    playlist_dataobject_desc_impl data;

    data.set_from_playlist_manager(mask);
    pfc::com_ptr_t<IDataObject> pDO = api->create_dataobject(data);

    HRESULT hr = OleSetClipboard(pDO.get_ptr());
    if (FAILED(hr))
        return false;

    m_playlist_api->remove_playlists(mask);
    if (m_playlist_api->get_active_playlist() == pfc_infinite && m_playlist_api->get_playlist_count())
        m_playlist_api->set_active_playlist(0);

    return true;
}
bool cut(const pfc::list_base_const_t<t_size>& indices)
{
    static_api_ptr_t<playlist_manager> m_playlist_api;
    t_size count = indices.get_count();
    t_size playlist_count = m_playlist_api->get_playlist_count();
    pfc::bit_array_bittable mask(playlist_count);
    for (t_size i = 0; i < count; i++) {
        if (indices[i] < playlist_count)
            mask.set(indices[i], true);
    }
    return cut(mask);
};
bool copy(const pfc::bit_array& mask)
{
    static_api_ptr_t<playlist_manager> m_playlist_api;
    static_api_ptr_t<ole_interaction> api;
    playlist_dataobject_desc_impl data;

    data.set_from_playlist_manager(mask);
    pfc::com_ptr_t<IDataObject> pDO = api->create_dataobject(data);

    HRESULT hr = OleSetClipboard(pDO.get_ptr());
    return SUCCEEDED(hr);
}
bool copy(const pfc::list_base_const_t<t_size>& indices)
{
    static_api_ptr_t<playlist_manager> m_playlist_api;
    t_size count = indices.get_count();
    t_size playlist_count = m_playlist_api->get_playlist_count();
    pfc::bit_array_bittable mask(playlist_count);
    for (t_size i = 0; i < count; i++) {
        if (indices[i] < playlist_count)
            mask.set(indices[i], true);
    }
    return copy(mask);
};
bool paste(HWND wnd, t_size index_insert)
{
    static_api_ptr_t<playlist_manager> m_playlist_api;
    static_api_ptr_t<ole_interaction> api;
    pfc::com_ptr_t<IDataObject> pDO;

    HRESULT hr = OleGetClipboard(pDO.receive_ptr());
    if (FAILED(hr))
        return false;

    playlist_dataobject_desc_impl data;
    hr = api->check_dataobject_playlists(pDO);
    if (FAILED(hr))
        return false;

    hr = api->parse_dataobject_playlists(pDO, data);
    if (FAILED(hr))
        return false;

    t_size plcount = m_playlist_api->get_playlist_count();
    if (index_insert > plcount)
        index_insert = plcount;

    t_size count = data.get_entry_count();
    for (t_size i = 0; i < count; i++) {
        pfc::string8 name;
        metadb_handle_list handles;
        data.get_entry_name(i, name);
        data.get_entry_content(i, handles);
        index_insert = m_playlist_api->create_playlist(name, pfc_infinite, index_insert);
        m_playlist_api->playlist_insert_items(index_insert, 0, handles, pfc::bit_array_false());
        index_insert++;
    }
    return true;
};
} // namespace playlist_manager_utils
