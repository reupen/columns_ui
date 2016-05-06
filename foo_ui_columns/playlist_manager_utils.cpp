#include "stdafx.h"
#include "playlist_manager_utils.h"

playlist_format_name_t::titleformat_hook_playlist_t::titleformat_hook_playlist_t(unsigned p_index, const char * p_name, t_size p_playing) 
			: m_name(p_name), m_index(p_index), m_lock_name_initialised(false), 
			m_length_initialised(false), m_filesize_initialised(false), m_filesize(NULL), m_playing(p_playing == p_index)
		{
			m_metadb_api = static_api_ptr_t<metadb>().get_ptr();
			m_api = static_api_ptr_t<playlist_manager_v3>().get_ptr();
			m_locked = m_api->playlist_lock_is_present(p_index);
			m_size = m_api->playlist_get_item_count(p_index);
			m_active = m_api->get_active_playlist() == p_index;
		};

playlist_format_name_t::playlist_format_name_t(unsigned p_index, const char * src, t_size p_playing)
{
	if (cfg_playlist_switcher_use_tagz)
	{
		service_ptr_t<titleformat_object> to_temp;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_temp,cfg_playlist_switcher_tagz);
		to_temp->run(&titleformat_hook_playlist_t(p_index, src, p_playing), *this, NULL);
	}
	else set_string(src);
}

bool playlist_format_name_t::titleformat_hook_playlist_t::process_field(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,bool & p_found_flag)
{
	p_found_flag = false;
	if (!stricmp_utf8_ex(p_name,p_name_length,"title",pfc_infinite))
	{
		p_out->write(titleformat_inputtypes::unknown, m_name,pfc_infinite);
		p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"is_locked",pfc_infinite))
	{
		if (m_locked)
		{
			p_out->write(titleformat_inputtypes::unknown, "1",pfc_infinite);
			p_found_flag = true;
			return true;
		}
		return false;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"is_active",pfc_infinite))
	{
		if (m_active)
		{
			p_out->write(titleformat_inputtypes::unknown, "1",pfc_infinite);
			p_found_flag = true;
			return true;
		}
		return false;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"size",pfc_infinite))
	{
		p_out->write_int(titleformat_inputtypes::unknown, m_size);
		p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"is_playing",pfc_infinite))
	{
		if (m_playing)
		{
			p_out->write(titleformat_inputtypes::unknown, "1",pfc_infinite);
			p_found_flag = true;
			return true;
		}
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"lock_name",pfc_infinite))
	{
		if (m_locked)
		{
			if (!m_lock_name_initialised)
			{
				m_api->playlist_lock_query_name(m_index, m_lock_name);
				m_lock_name_initialised=true;
			}
			p_out->write(titleformat_inputtypes::unknown, m_lock_name, pfc_infinite);
			p_found_flag = true;
			return true;
		}
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"length",pfc_infinite))
	{
		if (!m_length_initialised)
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> list;
			list.set_count(m_size);
			m_api->playlist_get_all_items(m_index, list);

			m_length = pfc::format_time_ex(list.calc_total_duration(), 0);
			m_length_initialised=true;
		}
		p_out->write(titleformat_inputtypes::unknown, m_length.get_ptr(), pfc_infinite);
		p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"filesize_raw",pfc_infinite))
	{
		initialise_filesize();
		p_out->write(titleformat_inputtypes::unknown, pfc::format_uint(m_filesize).get_ptr(), pfc_infinite);
		p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"filesize",pfc_infinite))
	{
		initialise_filesize();
		mmh::format_file_size str(m_filesize);
		p_out->write(titleformat_inputtypes::unknown, str.get_ptr(), pfc_infinite);
		p_found_flag = true;
		return true;
	}
	return false;
}

namespace playlist_manager_utils
{
	bool check_clipboard()
	{
		try 
		{
			static_api_ptr_t<ole_interaction> api;
			pfc::com_ptr_t<IDataObject> pDO;
			if (SUCCEEDED(OleGetClipboard(pDO.receive_ptr())))
			{
				HRESULT hr = api->check_dataobject_playlists(pDO);
				if (SUCCEEDED(hr))
					return true;
			}
		}
		catch (const exception_service_not_found &) {};
		return false;
	}
	bool cut(const bit_array & mask)	
	{
		try 
		{
			static_api_ptr_t<playlist_manager> m_playlist_api;
			static_api_ptr_t<ole_interaction> api;
			playlist_dataobject_desc_impl data;
			data.set_from_playlist_manager(mask);
			pfc::com_ptr_t<IDataObject> pDO = api->create_dataobject(data);
			if (pDO.is_valid())
			{
				if (SUCCEEDED(OleSetClipboard(pDO.get_ptr())))
				{
					m_playlist_api->remove_playlists(mask);
					if (m_playlist_api->get_active_playlist() == pfc_infinite && m_playlist_api->get_playlist_count())
						m_playlist_api->set_active_playlist(0);
				}
			}
			return true;
		}
		catch (const exception_service_not_found &) {};
		return false;
	}
	bool cut(const pfc::list_base_const_t<t_size> & indices)	
	{
		static_api_ptr_t<playlist_manager> m_playlist_api;
		t_size i, count = indices.get_count(), playlist_count=m_playlist_api->get_playlist_count();
		bit_array_bittable mask(playlist_count);
		for (i=0; i<count; i++)
		{
			if (indices[i]<playlist_count)
				mask.set(indices[i], true);
		}
		return cut(mask);
	};
	bool copy(const bit_array & mask)	
	{
		try 
		{
			static_api_ptr_t<playlist_manager> m_playlist_api;
			static_api_ptr_t<ole_interaction> api;
			playlist_dataobject_desc_impl data;
			data.set_from_playlist_manager(mask);
			pfc::com_ptr_t<IDataObject> pDO = api->create_dataobject(data);
			if (pDO.is_valid())
			{
				if (SUCCEEDED(OleSetClipboard(pDO.get_ptr())))
				{
				}
			}
			return true;
		}
		catch (const exception_service_not_found &) {};
		return false;
	}
	bool copy(const pfc::list_base_const_t<t_size> & indices)	
	{
		static_api_ptr_t<playlist_manager> m_playlist_api;
		t_size i, count = indices.get_count(), playlist_count=m_playlist_api->get_playlist_count();
		bit_array_bittable mask(playlist_count);
		for (i=0; i<count; i++)
		{
			if (indices[i]<playlist_count)
				mask.set(indices[i], true);
		}
		return copy(mask);
	};
	bool paste(HWND wnd, t_size index_insert)	
	{
		try 
		{
			static_api_ptr_t<playlist_manager> m_playlist_api;
			static_api_ptr_t<ole_interaction> api;
			pfc::com_ptr_t<IDataObject> pDO;
			if (SUCCEEDED(OleGetClipboard(pDO.receive_ptr())))
			{
				playlist_dataobject_desc_impl data;
				HRESULT hr = api->check_dataobject_playlists(pDO);
				if (SUCCEEDED(hr))
				{
					hr = api->parse_dataobject_playlists(pDO, data);
					if (SUCCEEDED(hr))
					{
						t_size plcount = m_playlist_api->get_playlist_count();
						if (index_insert > plcount)
							index_insert = plcount;
						t_size i, count = data.get_entry_count();
						for (i=0; i<count; i++)
						{
							pfc::string8 name;
							metadb_handle_list handles;
							data.get_entry_name(i, name);
							data.get_entry_content(i, handles);
							index_insert = m_playlist_api->create_playlist(name, pfc_infinite, index_insert);
							m_playlist_api->playlist_insert_items(index_insert, 0, handles, bit_array_false());
							index_insert++;
						}
					}
				}
			}
			return true;
		}
		catch (const exception_service_not_found &) {};
		return false;
	};
}
