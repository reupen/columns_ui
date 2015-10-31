#include "stdafx.h"

void do_playlist(node_ptr const & src, bool replace, bool b_new)
{
	if (src.is_valid() && src->get_entries().get_count()>0)
	{
		if (b_new) src->create_new_playlist();
		else src->send_to_playlist(replace);
		if (replace && cfg_autoplay)
		{
			static_api_ptr_t<playlist_manager>()->reset_playing_playlist();
			static_api_ptr_t<play_control>()->play_start();
		}
	}
}

void do_autosend_playlist(node_ptr const & src, string_base & view, bool b_play)
{
	if (src.is_valid())
	{
		static_api_ptr_t<playlist_manager> api;
		string8 playlist_name;

		class titleformat_hook_view : public titleformat_hook
		{
			const char * m_view;
		public:
			virtual bool process_field(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, bool & p_found_flag)
			{
				p_found_flag = false;
				if (m_view)
				{
					if (!stricmp_utf8_ex(p_name, p_name_length, "_view", 5))
					{
						p_out->write(titleformat_inputtypes::meta, m_view, pfc_infinite);
						p_found_flag = true;
						return true;
					}
				}
				return false;
			}
			virtual bool process_function(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag)
			{
				p_found_flag = false;
				return false;
			}
			inline titleformat_hook_view(const char * p_view) : m_view(p_view)
			{};
		};

		static_api_ptr_t<titleformat_compiler>()->run(&titleformat_hook_view(view), playlist_name, cfg_playlist_name);
		//playlist_name.fix_filename_chars(); //for find playlist NOT RELEVANT FOR 0.9
		unsigned idx = api->find_or_create_playlist(playlist_name, pfc_infinite);
		api->playlist_undo_backup(idx);
		api->playlist_clear(idx);
		if (cfg_add_items_use_core_sort) api->playlist_add_items_filter(idx, src->get_entries(), cfg_add_items_select != 0);
		else
		{
			src->sort_entries();
			api->playlist_add_items(idx, src->get_entries(), bit_array_val(cfg_add_items_select != 0));
		}
		api->set_active_playlist(idx);
		if (b_play && cfg_autoplay)
		{
			api->reset_playing_playlist();
			static_api_ptr_t<play_control>()->play_start();
		}
	}
}
