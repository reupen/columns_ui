#include "stdafx.h"
#include "filter.h"

namespace filter_panel {

    bool filter_panel_t::notify_before_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, bool b_source_mouse)
    {
        if (!m_field_data.m_use_script && m_field_data.m_fields.get_count() && column == 0 && indices.get_count() == 1 && indices[0] != 0)
            return true;
        return false;
    };
    bool filter_panel_t::notify_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, pfc::string_base & p_text, t_size & p_flags, mmh::ComPtr<IUnknown> & pAutocompleteEntries)
    {
        t_size indices_count = indices.get_count();
        if (!m_field_data.m_use_script && m_field_data.m_fields.get_count() && indices_count == 1 && indices[0] < m_nodes.get_count())
        {
            m_edit_handles = m_nodes[indices[0]].m_handles;

            m_edit_fields = m_field_data.m_fields;

            p_text = (m_edit_previous_value = pfc::stringcvt::string_utf8_from_wide(m_nodes[indices[0]].m_value));

            return true;
        }
        return false;
    };
    void filter_panel_t::notify_save_inline_edit(const char * value)
    {
        static_api_ptr_t<metadb_io_v2> tagger_api;
        {
            metadb_handle_list ptrs(m_edit_handles);
            pfc::list_t<file_info_impl> infos;
            pfc::list_t<bool> mask;
            pfc::list_t<const file_info *> infos_ptr;
            t_size i, count = ptrs.get_count();
            mask.set_count(count);
            infos.set_count(count);
            //infos.set_count(count);
            for (i = 0; i<count; i++)
            {
                assert(ptrs[i].is_valid());
                mask[i] = !ptrs[i]->get_info(infos[i]);
                infos_ptr.add_item(&infos[i]);
                if (!mask[i])
                {
                    bool b_remove = true;
                    t_size j, jcount = m_edit_fields.get_count();
                    for (j = 0; j<jcount; j++)
                    {
                        t_size field_index = infos[i].meta_find(m_edit_fields[j]);
                        if (field_index != pfc_infinite)
                        {
                            t_size field_count = infos[i].meta_enum_value_count(field_index);
                            t_size k;
                            bool b_found = false;
                            for (k = 0; k<field_count; k++)
                            {
                                const char * ptr = infos[i].meta_enum_value(field_index, k);
                                if (((!ptr && m_edit_previous_value.is_empty()) || !stricmp_utf8(m_edit_previous_value, ptr)) && strcmp(value, ptr))
                                {
                                    infos[i].meta_modify_value(field_index, k, value);
                                    b_remove = false;
                                }
                            }
                        }
                    }
                    mask[i] = b_remove;
                }
            }
            infos_ptr.remove_mask(mask.get_ptr());
            ptrs.remove_mask(mask.get_ptr());

            {
                service_ptr_t<file_info_filter_impl>  filter = new service_impl_t<file_info_filter_impl>(ptrs, infos_ptr);
                tagger_api->update_info_async(ptrs, filter, GetAncestor(get_wnd(), GA_ROOT), metadb_io_v2::op_flag_no_errors | metadb_io_v2::op_flag_background | metadb_io_v2::op_flag_delay_ui, nullptr);
            }
        }
    };
    void filter_panel_t::notify_exit_inline_edit()
    {
        m_edit_fields.remove_all();
        m_edit_handles.remove_all();
        m_edit_previous_value.reset();
    };

}