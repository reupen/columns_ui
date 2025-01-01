#include "pch.h"
#include "filter.h"

namespace cui::panels::filter {

bool FilterPanel::notify_before_create_inline_edit(
    const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse)
{
    return !m_field_data.m_use_script && !m_field_data.m_fields.empty() && column == 0 && indices.get_count() == 1
        && indices[0] != 0;
}
bool FilterPanel::notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
    pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries)
{
    size_t indices_count = indices.get_count();
    if (!m_field_data.m_use_script && !m_field_data.m_fields.empty() && indices_count == 1
        && indices[0] < m_nodes.get_count()) {
        m_edit_handles = m_nodes[indices[0]]->m_handles;

        m_edit_fields = m_field_data.m_fields;

        p_text = (m_edit_previous_value = pfc::stringcvt::string_utf8_from_wide(m_nodes[indices[0]]->m_value.c_str()));

        return true;
    }
    return false;
}
void FilterPanel::notify_save_inline_edit(const char* value)
{
    auto apply_filter = [edit_fields = m_edit_fields, edit_previous_value = m_edit_previous_value,
                            value = std::string{value}](trackRef location, t_filestats stats, file_info& info) -> bool {
        bool changed = false;

        for (auto& field : edit_fields) {
            const size_t field_index = info.meta_find(field);

            if (field_index == std::numeric_limits<size_t>::max())
                continue;

            size_t value_count = info.meta_enum_value_count(field_index);

            for (auto value_index : std::ranges::views::iota(size_t{}, value_count)) {
                auto field_value = info.meta_enum_value(field_index, value_index);

                if (!stricmp_utf8(edit_previous_value, field_value) && strcmp(value.c_str(), field_value) != 0) {
                    info.meta_modify_value(field_index, value_index, value.c_str());
                    changed = true;
                }
            }
        }

        return changed;
    };

    auto filter = file_info_filter::create(std::move(apply_filter));

    metadb_io_v2::get()->update_info_async(m_edit_handles, filter, GetAncestor(get_wnd(), GA_ROOT),
        metadb_io_v2::op_flag_no_errors | metadb_io_v2::op_flag_background | metadb_io_v2::op_flag_delay_ui, nullptr);
}
void FilterPanel::notify_exit_inline_edit()
{
    m_edit_fields.clear();
    m_edit_handles.remove_all();
    m_edit_previous_value.reset();
}

} // namespace cui::panels::filter
