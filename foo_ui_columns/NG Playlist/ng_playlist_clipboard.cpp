#include "stdafx.h"

namespace playlist_utils
{
	bool check_clipboard()
	{
		try 
		{
			static_api_ptr_t<ole_interaction> api;
			pfc::com_ptr_t<IDataObject> pDO;
			if (SUCCEEDED(OleGetClipboard(pDO.receive_ptr())))
			{
				bool b_native; DWORD dummy  = DROPEFFECT_COPY;
				HRESULT hr = api->check_dataobject(pDO, dummy, b_native);
				if (SUCCEEDED(hr))
					return b_native;
			}
		}
		catch (const exception_service_not_found &) {};
		return false;
	}
	bool cut()	
	{
		try 
		{
			static_api_ptr_t<playlist_manager> m_playlist_api;
			static_api_ptr_t<ole_interaction> api;
			metadb_handle_list data;
			m_playlist_api->activeplaylist_undo_backup();
			m_playlist_api->activeplaylist_get_selected_items(data);
			pfc::com_ptr_t<IDataObject> pDO = api->create_dataobject(data);
			if (pDO.is_valid())
			{
				if (SUCCEEDED(OleSetClipboard(pDO.get_ptr())))
					m_playlist_api->activeplaylist_remove_selection();
			}
			return true;
		}
		catch (const exception_service_not_found &) {};
		return false;
	};
	bool copy()	
	{
		try 
		{
			static_api_ptr_t<playlist_manager> m_playlist_api;
			static_api_ptr_t<ole_interaction> api;
			metadb_handle_list data;
			m_playlist_api->activeplaylist_get_selected_items(data);
			pfc::com_ptr_t<IDataObject> pDO = api->create_dataobject(data);
			if (pDO.is_valid())
			{
				OleSetClipboard(pDO.get_ptr());
			}
			return true;
		}
		catch (const exception_service_not_found &) {};
		return false;
	};
	bool paste(HWND wnd)	
	{
		try 
		{
			static_api_ptr_t<playlist_manager> m_playlist_api;
			static_api_ptr_t<ole_interaction> api;
			pfc::com_ptr_t<IDataObject> pDO;
			if (SUCCEEDED(OleGetClipboard(pDO.receive_ptr())))
			{
				dropped_files_data_impl data;
				metadb_handle_list handles;
				bool b_native; DWORD dummy  = DROPEFFECT_COPY;
				HRESULT hr = api->check_dataobject(pDO, dummy, b_native);
				if (SUCCEEDED(hr))
				{
					hr = api->parse_dataobject(pDO, data);
					if (SUCCEEDED(hr))
					{
						m_playlist_api->activeplaylist_undo_backup();
						t_size index = m_playlist_api->activeplaylist_get_focus_item();
						if (index != pfc_infinite) index ++;
						data.to_handles(handles, b_native, GetAncestor(wnd, GA_ROOT));
						m_playlist_api->activeplaylist_clear_selection();
						m_playlist_api->activeplaylist_insert_items(index, handles, bit_array_true());

						//OleSetClipboard(NULL);
					}
				}
			}
			return true;
		}
		catch (const exception_service_not_found &) {};
		return false;
	};

}
