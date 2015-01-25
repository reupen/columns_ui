#include "stdafx.h"

namespace {
	class titleformat_hook_impl : public titleformat_hook {
	public:
		titleformat_hook_impl(pfc::list_base_const_t<metadb_handle_ptr> const & p_items,titleformat_hook * p_hook) : m_hook(p_hook) {
			m_infos.set_size(p_items.get_count());
			for(t_size n = 0; n < p_items.get_count(); n++) {
				if (!p_items[n]->get_info_locked(m_infos[n])) m_infos[n] = NULL;
			}
		}

		bool process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag) {
			if (m_hook != NULL) {
				if (m_hook->process_field(p_out,p_name,p_name_length,p_found_flag)) return true;
			}
			if (process_own_field(p_out,p_name,p_name_length,p_found_flag)) return true;
			for(t_size n = 0; n < m_infos.get_size(); n++) {
				if (m_infos[n] != NULL) {
					if (titleformat_hook_impl_file_info(make_playable_location("",0),m_infos[n]).process_field(p_out,p_name,p_name_length,p_found_flag))
						return true;
				}
			}
			return false;
		}
		bool process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) {
			if (m_hook != NULL) {
				if (m_hook->process_function(p_out,p_name,p_name_length,p_params,p_found_flag)) return true;
			}
			for(t_size n = 0; n < m_infos.get_size(); n++) {
				if (m_infos[n] != NULL) {
					if (titleformat_hook_impl_file_info(make_playable_location("",0),m_infos[n]).process_function(p_out,p_name,p_name_length,p_params,p_found_flag))
						return true;
				}
			}
			return false;
		}
	private:
		bool process_own_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag) {
			if (stricmp_utf8_ex(p_name,p_name_length,"multiitem",infinite) == 0) {
				if (m_infos.get_size() > 1) {
					p_out->write_int(titleformat_inputtypes::unknown,1);
					p_found_flag = true;
				} else {
					p_out->write_int(titleformat_inputtypes::unknown,0);
					p_found_flag = false;
				}
				return true;
			} else {
				return false;
			}
		}

		in_metadb_sync m_lock;
		titleformat_hook * m_hook;
		pfc::array_t<const file_info*> m_infos;
	};
	
}

void format_title_group(pfc::list_base_const_t<metadb_handle_ptr> const & p_items,titleformat_hook * p_hook,pfc::string_base & p_out,service_ptr_t<titleformat_object> p_script,titleformat_text_filter * p_filter) {
	p_script->run(&titleformat_hook_impl(p_items,p_hook),p_out,p_filter);
}