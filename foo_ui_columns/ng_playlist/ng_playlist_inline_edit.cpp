#include "pch.h"

#include "file_info_utils.h"
#include "ng_playlist.h"

namespace cui::panels::playlist_view {

bool PlaylistView::notify_before_create_inline_edit(
    const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse)
{
    return (!b_source_mouse || main_window::config_get_inline_metafield_edit_mode() != main_window::mode_disabled)
        && column < m_edit_fields.get_count() && strlen(m_edit_fields[column]);
}

bool PlaylistView::notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
    pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries)
{
    const size_t indices_count = indices.get_count();
    m_edit_handles.remove_all();
    m_edit_handles.set_count(indices_count);

    std::vector<std::string_view> values;

    m_edit_field = m_edit_fields[column];

    for (size_t i = 0; i < indices_count; i++) {
        if (!m_playlist_api->activeplaylist_get_item_handle(m_edit_handles[i], indices[i]))
            return false;
    }

    helpers::EditMetadataFieldValueAggregator aggregator;

    for (const auto& track : m_edit_handles) {
        const auto info_container = track->get_info_ref();
        aggregator.process_file_info(m_edit_field, &info_container->info());
    }

    const auto joined = mmh::join(aggregator.m_values, "; "s);

    if (aggregator.m_mixed_values) {
        p_text = "«mixed values» ";
        p_text += joined.c_str();

        if (aggregator.m_truncated)
            p_text += "; …";
    } else {
        p_text = joined.c_str();
    }

    if (m_library_autocomplete_v2.is_empty() && m_library_autocomplete_v1.is_empty()) {
        if (!library_meta_autocomplete_v2::tryGet(m_library_autocomplete_v2)) {
            m_library_autocomplete_v1 = library_meta_autocomplete::get();
        }
    }

    p_flags |= inline_edit_autocomplete;
    pfc::com_ptr_t<IUnknown> local_autocomplete_entries;

    if (m_library_autocomplete_v2.is_valid())
        m_library_autocomplete_v2->get_value_list_async(m_edit_field, local_autocomplete_entries);
    else
        m_library_autocomplete_v1->get_value_list(m_edit_field, local_autocomplete_entries);

    autocomplete_entries.attach(local_autocomplete_entries.detach());

    return true;
}

void PlaylistView::notify_save_inline_edit(const char* value)
{
    auto values = helpers::split_meta_value(value);
    const auto filter
        = fb2k::service_new<helpers::SingleFieldFileInfoFilter>(m_edit_field.get_ptr(), std::move(values));

    metadb_io_v2::get()->update_info_async(m_edit_handles, filter, GetAncestor(get_wnd(), GA_ROOT),
        metadb_io_v2::op_flag_no_errors | metadb_io_v2::op_flag_background | metadb_io_v2::op_flag_delay_ui, nullptr);
}

void PlaylistView::notify_exit_inline_edit()
{
    m_edit_field.reset();
    m_edit_handles.remove_all();
}

} // namespace cui::panels::playlist_view
