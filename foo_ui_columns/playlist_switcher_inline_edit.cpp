#include "pch.h"
#include "playlist_switcher_v2.h"

namespace cui::panels::playlist_switcher {

bool PlaylistSwitcher::notify_before_create_inline_edit(
    const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse)
{
    return column == 0 && indices.get_count() == 1;
}
bool PlaylistSwitcher::notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
    pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries)
{
    size_t indices_count = indices.get_count();
    if (indices_count == 1 && indices[0] < m_playlist_api->get_playlist_count()) {
        m_edit_playlist = std::make_shared<playlist_position_reference_tracker>(false);
        m_edit_playlist->m_playlist = indices[0];
        m_playlist_api->playlist_get_name(indices[0], p_text);
        return true;
    }
    return false;
}
void PlaylistSwitcher::notify_save_inline_edit(const char* value)
{
    if (m_edit_playlist && m_edit_playlist->m_playlist != pfc_infinite) {
        pfc::string8 current;
        m_playlist_api->playlist_get_name(m_edit_playlist->m_playlist, current);
        if (strcmp(current, value) != 0) {
            m_playlist_api->playlist_rename(m_edit_playlist->m_playlist, value, pfc_infinite);
        }
    }
    m_edit_playlist.reset();
}

} // namespace cui::panels::playlist_switcher
