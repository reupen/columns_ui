#include "foo_ui_columns.h"

cfg_guid cfg_default_playlist(create_guid(0x68527c89,0xb0f7,0xf653,0x00,0x53,0x8c,0xeb,0x47,0xe7,0xa3,0xb3), playlist_view::extension_guid);

// {942C36A4-4E28-4cea-9644-F223C9A838EC}
const GUID g_guid_playlist_switcher_tabs_font = 
{ 0x942c36a4, 0x4e28, 0x4cea, { 0x96, 0x44, 0xf2, 0x23, 0xc9, 0xa8, 0x38, 0xec } };

void remove_playlist_helper(t_size index)
{
	static_api_ptr_t<playlist_manager> api;
	if (index == api->get_active_playlist())
	{
		if (index && index + 1 == api->get_playlist_count())
			api->set_active_playlist(index-1);
		//else
		//	api->set_active_playlist(index);
	}
	api->remove_playlist_switch(index);
}

class playlists_tabs_extension : public uie::container_ui_extension_t<ui_helpers::container_window, uie::splitter_window_v2>, public playlist_callback
{
public:
	//enum { MSG_EDIT_PANEL =  WM_USER+2};
	class window_host_impl : public ui_extension::window_host
	{
	public:

		virtual unsigned get_supported_types()const
		{
			return ui_extension::type_panel;
		}

		virtual unsigned is_resize_supported(HWND wnd)const
		{
			//We won't support ui_extension::size_width since we can't reliably detect multiline tab shit
			return (m_this->get_host()->is_resize_supported(m_this->get_wnd()) & ui_extension::size_height);
		}

		virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height)
		{
			if (flags == ui_extension::size_height && is_resize_supported(wnd))
			{
				if (m_this->wnd_tabs)
				{
				RECT rc;
				GetWindowRect(m_this->m_child_wnd, &rc);
				MapWindowPoints(HWND_DESKTOP, m_this->get_wnd(), (LPPOINT)&rc, 2);
				rc.bottom = rc.top + height;
				m_this->adjust_rect(TRUE, &rc);
				//We would expect rc.top and rc.left to be 0.
				return m_this->get_host()->request_resize(m_this->get_wnd(), flags, rc.right, rc.bottom);
				}
				else
					return m_this->get_host()->request_resize(m_this->get_wnd(), flags, width, height);
			}
			return false;
		}
		virtual bool is_visible(HWND wnd)const
		{
			return true;
		}
		virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility)const
		{
			return false;
		}
		virtual bool set_window_visibility(HWND wnd, bool visibility)
		{
			return false;
		}

		virtual bool get_keyboard_shortcuts_enabled()const {return m_this->get_host()->get_keyboard_shortcuts_enabled();}

		virtual bool get_show_shortcuts()const
		{
			return m_this->get_host()->get_keyboard_shortcuts_enabled();
		}

		virtual void on_size_limit_change(HWND wnd, unsigned flags)
		{
			m_this->on_child_position_change();
		};

		virtual const GUID & get_host_guid()const
		{
			// {20789B52-4998-43ae-9B20-CCFD3BFBEEBD}
			static const GUID guid = 
			{ 0x20789b52, 0x4998, 0x43ae, { 0x9b, 0x20, 0xcc, 0xfd, 0x3b, 0xfb, 0xee, 0xbd } };
			return guid;
		}

		virtual bool override_status_text_create(service_ptr_t<ui_status_text_override> & p_out)
		{
			static_api_ptr_t<ui_control> api;
			return m_this->get_host()->override_status_text_create(p_out);
		}

		virtual void relinquish_ownership(HWND wnd)
		{
			m_this->m_child_wnd=0;
			m_this->m_host.release();
			m_this->m_child.release();
			m_this->reset_size_limits();
		};
		void set_this(playlists_tabs_extension * ptr)
		{
			m_this = ptr;
		}
	private:
		service_ptr_t<playlists_tabs_extension> m_this;

	};
private:

	service_ptr_t<ui_status_text_override> m_status_override;

	WNDPROC tabproc;

	bool m_dragging;
	unsigned m_dragging_idx;
	RECT m_dragging_rect;

	bool m_playlist_switched;
	bool m_switch_timer;
	unsigned m_switch_playlist;
	bool initialised;

	t_int32 m_mousewheel_delta;
	UINT_PTR ID_CUSTOM_BASE;

	service_ptr_t<contextmenu_manager> p_manager;

	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data_ex(_T("{ABB72D0D-DBF0-4bba-8C68-3357EBE07A4D}"), _T(""), false, 0, WS_CHILD|WS_CLIPCHILDREN, WS_EX_CONTROLPARENT, CS_DBLCLKS);
	}

public:
	static pfc::ptr_list_t<playlists_tabs_extension> list_wnd;

	HWND wnd_tabs;
	LRESULT WINAPI hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static LRESULT WINAPI main_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	playlists_tabs_extension();

	~playlists_tabs_extension();

	class playlists_tabs_drop_target : public IDropTarget
	{
		bool m_last_rmb ;
		long drop_ref_count;
		POINTL last_over; 
		service_ptr_t<playlists_tabs_extension> p_list;
		pfc::com_ptr_t<IDataObject> m_DataObject;
		mmh::comptr_t<IDropTargetHelper> m_DropTargetHelper;
	public:
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR *ppvObject);
		virtual ULONG STDMETHODCALLTYPE   AddRef();
		virtual ULONG STDMETHODCALLTYPE   Release();
		virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
		virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
		virtual HRESULT STDMETHODCALLTYPE DragLeave( void);
		virtual HRESULT STDMETHODCALLTYPE Drop( IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
		playlists_tabs_drop_target(playlists_tabs_extension * p_wnd);
	};

	virtual void FB2KAPI on_items_removing(unsigned p_playlist,const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count){};//called before actually removing them
	virtual void FB2KAPI on_items_removed(unsigned p_playlist,const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count){};

	void on_playlist_activate(unsigned p_old,unsigned p_new)
	{
		if (wnd_tabs)
		{
			TabCtrl_SetCurSel(wnd_tabs, p_new);
		}
	}

	void on_playlists_reorder(const unsigned * p_order,unsigned p_count)
	{
		if (wnd_tabs)
		{
			static_api_ptr_t<playlist_manager> playlist_api;
			unsigned n;
			int sel = playlist_api->get_active_playlist();
			
			for (n=0;n<p_count;n++)
			{
				if (n != (unsigned)p_order[n])
				{
					pfc::string8 temp, temp2;
					playlist_api->playlist_get_name(n, temp);
					
					uTabCtrl_InsertItemText(wnd_tabs, n, temp, false);
				}
			}
			TabCtrl_SetCurSel(wnd_tabs, sel);
		}


	}
	void on_playlist_created(unsigned p_index,const char * p_name,unsigned p_name_len)
	{
		if (wnd_tabs)
		{
			uTabCtrl_InsertItemText(wnd_tabs, p_index, pfc::string8(p_name,p_name_len));
			set_styles();
			if (cfg_tabs_multiline) on_size();
		}
		else if (create_tabs()) on_size();
	}
	void on_playlists_removed(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count)
	{
		bool need_move = false;

		if (create_tabs()) need_move = true;
		else 
			if (wnd_tabs)
			{
				unsigned n = p_old_count;
				for (;n>0;n--)
				{
					if (p_mask[n-1])
						TabCtrl_DeleteItem(wnd_tabs, n-1);
				}
				set_styles();
				if (cfg_tabs_multiline) on_size();
				//RedrawWindow(wnd_tabs, NULL, NULL, RDW_ERASE|RDW_INVALIDATE);
			}

			if (need_move) on_size();

	}
	void on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len)
	{
		if (wnd_tabs)
		{
			uTabCtrl_InsertItemText(wnd_tabs, p_index, pfc::string8(p_new_name, p_new_name_len), false);
			if (cfg_tabs_multiline) on_size();
		}
	}

	void on_items_added(unsigned int,unsigned int,const pfc::list_base_const_t<metadb_handle_ptr> &,const bit_array &){};
	void on_items_reordered(unsigned int,const unsigned int *,unsigned int){};
	void on_items_selection_change(unsigned int,const bit_array &,const bit_array &){};
	void on_item_focus_change(unsigned int,unsigned int,unsigned int){};
	void on_items_modified(unsigned int,const bit_array &){};
	void on_items_modified_fromplayback(unsigned int,const bit_array &,play_control::t_display_level){};
	void on_items_replaced(unsigned int,const bit_array &,const pfc::list_base_const_t<t_on_items_replaced_entry> &){};
	void on_item_ensure_visible(unsigned int,unsigned int){};
	void on_playlists_removing(const bit_array &,unsigned int,unsigned int){};
	void on_default_format_changed(void){};
	void on_playback_order_changed(unsigned int){};
	void on_playlist_locked(unsigned int,bool){};

	void kill_switch_timer();

	void switch_to_playlist_delayed2(unsigned idx);

	static const GUID extension_guid;

	virtual const GUID & get_extension_guid() const;

	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;
	virtual bool get_short_name(pfc::string_base & out)const;

	void set_styles(bool visible = true)
	{
		if (wnd_tabs)
		{
			long flags = WS_CHILD |  TCS_HOTTRACK | TCS_TABS | ((cfg_tabs_multiline && (TabCtrl_GetItemCount(wnd_tabs) > 1)) ? TCS_MULTILINE|TCS_RIGHTJUSTIFY  : TCS_SINGLELINE) |(visible ? WS_VISIBLE : 0)|WS_CLIPSIBLINGS |WS_TABSTOP |0;

			if (uGetWindowLong(wnd_tabs, GWL_STYLE) != flags)
				uSetWindowLong(wnd_tabs, GWL_STYLE, flags);
		}
	}

	void reset_size_limits()
	{
		memset(&mmi, 0, sizeof(mmi));
		if (m_child_wnd)
		{
			mmi.ptMinTrackSize.x = 0;
			mmi.ptMinTrackSize.y = 0;
			mmi.ptMaxTrackSize.x = MAXLONG;
			mmi.ptMaxTrackSize.y = MAXLONG;
			SendMessage(m_child_wnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
			if (mmi.ptMinTrackSize.x != 0 || mmi.ptMinTrackSize.y != 0 || mmi.ptMaxTrackSize.x != MAXLONG || mmi.ptMaxTrackSize.y != MAXLONG)
			{
				RECT rc_min = {0, 0, mmi.ptMinTrackSize.x, mmi.ptMinTrackSize.y};
				RECT rc_max = {0, 0, mmi.ptMaxTrackSize.x, mmi.ptMaxTrackSize.y};
				if (wnd_tabs)
				{
					adjust_rect(TRUE, &rc_min);
					adjust_rect(TRUE, &rc_max);
				}
				mmi.ptMinTrackSize.x = rc_min.right - rc_min.left;
				mmi.ptMinTrackSize.y = rc_min.bottom - rc_min.top;
				mmi.ptMaxTrackSize.x = rc_max.right - rc_max.left;
				mmi.ptMaxTrackSize.y = rc_max.bottom - rc_max.top;
			}
			else
			{
				if (wnd_tabs)
				{
					RECT rc, rc_child;
					GetWindowRect(wnd_tabs, &rc);
					MapWindowPoints(HWND_DESKTOP, get_wnd(), (LPPOINT)&rc, 2);
					rc_child = rc;
					adjust_rect(FALSE, &rc_child);
					mmi.ptMinTrackSize.x = rc_child.left - rc.left + rc.right - rc_child.right;
					mmi.ptMinTrackSize.y = rc_child.top - rc.top + rc.bottom - rc_child.bottom;
				}
				mmi.ptMaxTrackSize.x = MAXLONG;
				mmi.ptMaxTrackSize.y = MAXLONG;
			}
		}
		else
		{
			if (wnd_tabs)
			{
				RECT rc_tabs, rc_child;
				GetWindowRect(wnd_tabs, &rc_tabs);
				MapWindowPoints(HWND_DESKTOP, get_wnd(), (LPPOINT)&rc_tabs, 2);
				rc_child = rc_tabs;
				adjust_rect(FALSE, &rc_child);
				mmi.ptMinTrackSize.x = rc_child.left - rc_tabs.left + rc_tabs.right - rc_child.right;
				mmi.ptMinTrackSize.y = rc_child.top - rc_tabs.top + rc_tabs.bottom - rc_child.bottom;
				mmi.ptMaxTrackSize.y = mmi.ptMinTrackSize.y;
				mmi.ptMaxTrackSize.x = MAXLONG;
			}
			else
			{
				mmi.ptMaxTrackSize.x = MAXLONG;
				mmi.ptMaxTrackSize.y = MAXLONG;
			}
		}
	}

	void on_size()
	{
		RECT rc;
		GetWindowRect(get_wnd(), &rc);
		on_size(rc.right-rc.left, rc.bottom-rc.top);
	}

	void adjust_rect(bool b_larger, RECT * rc)
	{
		if (b_larger)
		{
			RECT rc_child = *rc;
			TabCtrl_AdjustRect(wnd_tabs, FALSE, &rc_child);
			rc_child.top = rc->top + 2;
			TabCtrl_AdjustRect(wnd_tabs, TRUE, &rc_child);
			*rc = rc_child;
		}
		else
		{
			RECT rc_tabs;
			rc_tabs = *rc;
			TabCtrl_AdjustRect(wnd_tabs, FALSE, &rc_tabs);
			rc->top = rc_tabs.top - 2;
		}
	}

	void on_size(unsigned cx, unsigned cy)
	{
		if (wnd_tabs)
		{
			SetWindowPos(wnd_tabs, 0, 0, 0, cx, cy, SWP_NOZORDER);
			RECT rc = {0,0,cx,cy};
			adjust_rect(FALSE, &rc);
			if (m_child_wnd)
				SetWindowPos(m_child_wnd, 0, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_NOZORDER);
			unsigned old_top = m_child_top; //prevent braindead (partial) recursion
			m_child_top = rc.top; 
			if (rc.top != old_top)
			{
				//PostMessage(get_wnd(), WM_USER+3,0,0);
				on_child_position_change();
				//console::formatter() << "old top: " << old_top << " new top: " << rc.top;
			}
		}
		else if (m_child_wnd)
		{
			SetWindowPos(m_child_wnd, 0, 0, 0, cx, cy, SWP_NOZORDER);
		}
	}


	virtual unsigned get_type  () const{return ui_extension::type_layout|uie::type_splitter;};
	static void on_font_change();
	bool create_tabs();

	void create_child();
	void destroy_child();

	virtual bool is_point_ours(HWND wnd_point, const POINT & pt_screen, pfc::list_base_t<uie::window::ptr> & p_hierarchy);
	virtual void get_supported_panels(const pfc::list_base_const_t<uie::window::ptr> & p_windows, bit_array_var & p_mask_unsupported);

	virtual void insert_panel(unsigned index, const uie::splitter_item_t *  p_item)
	{
		if (index==0 && m_child_guid == pfc::guid_null)
		{
			if (initialised)
				destroy_child();

			m_child_data.set_size(0);
			stream_writer_memblock_ref w(m_child_data);
			p_item->get_panel_config(&w);
			m_child_guid = p_item->get_panel_guid();

			if (initialised)
				create_child();
		}
	};
	virtual void remove_panel(unsigned index)
	{
		if (index==0 && m_child_guid != pfc::guid_null)
		{
			if (initialised)
				destroy_child();

			m_child_guid = pfc::guid_null;
			m_child_data.set_size(0);

			if (initialised)
				create_child();
		}
	};
	virtual void replace_panel(unsigned index, const uie::splitter_item_t * p_item)
	{
		if (index==0 && m_child_guid != pfc::guid_null)
		{
			if (initialised)
				destroy_child();

			m_child_data.set_size(0);
			stream_writer_memblock_ref w(m_child_data);
			p_item->get_panel_config(&w);
			m_child_guid = p_item->get_panel_guid();

			if (initialised)
				create_child();
		}
	};
	virtual unsigned get_panel_count()const
	{
		return m_child_guid != pfc::guid_null ? 1 : 0;
	};
	virtual unsigned get_maximum_panel_count()const{return 1;};
	virtual uie::splitter_item_t * get_panel(unsigned index)const
	{
		uie::splitter_item_simple_t * ptr = new uie::splitter_item_simple_t;
		ptr->set_panel_guid(m_child_guid);
		ptr->set_panel_config(&stream_reader_memblock_ref( m_child_data.get_ptr(), m_child_data.get_size()), m_child_data.get_size());
		if (index == 0 && m_child_guid != pfc::guid_null)
		{
			if (m_child_wnd && m_child.is_valid())
				ptr->set_window_ptr(m_child);
			//else
			//	p_out.set_window_ptr(uie::window_ptr(NULL));
		}
		return ptr;
	};

#if 0
	virtual void get_children(pfc::list_base_t<uie::extension_data_ptr> & p_out)const
	{
		if (m_child_guid != pfc::guid_null)
		{
			uie::extension_data_ptr temp = new(std::nothrow) uie::extension_data_impl;
			temp->set_guid(m_child_guid);
			temp->set_data(m_child_data, m_child_data.get_size());
			if (m_child_wnd && m_child.is_valid()) temp->set_window_ptr(m_child);
			p_out.add_item(temp);
		}
	}; 

	virtual void set_children(const pfc::list_base_const_t<uie::extension_data_ptr> & p_out)
	{
		if (initialised)
			destroy_child();
		if (p_out.get_count())
		{
			m_child_data.set_size(0);
			stream_writer_memblock_ref w(m_child_data);
			p_out[0]->get_data(&w);
			m_child_guid = p_out[0]->get_guid();
		}
		else
		{
			m_child_guid = pfc::guid_null;
			m_child_data.set_size(0);
		}
		if (initialised)
			create_child();
	}; 
#endif
	/**
	 * Recursively search your windows
	 * e.g.
	 * service_ptr_t<uie::playlist_window> p_playlist_wnd;
	 * service_ptr_t<uie::splitter_window> p_splitter_wnd;
	 * for(n;n<count;n++)
	 *  if (m_children[n]->m_wnd->service_query_t(p_playlist_wnd))
	 *  { p_playlist_wnd->set_focus(); return true; }
	 *  else if (m_children[n]->m_wnd->service_query_t(p_splitter_wnd) && p_splitter_wnd->focus_playlist_window())
	 *   return true;
	 *
	 * \return count
	 */
#if 0
	virtual bool focus_playlist_window()const
	{
		service_ptr_t<uie::playlist_window> p_playlist_wnd;
		service_ptr_t<uie::splitter_window> p_splitter_wnd;
		if (m_child.is_valid() && m_child_wnd)
		{
			if (m_child->service_query_t(p_playlist_wnd))
			{
				p_playlist_wnd->set_focus(); 
				return true; 
			}
			else if (m_child->service_query_t(p_splitter_wnd) && p_splitter_wnd->focus_playlist_window())
				return true;
		}
		return false;
	}
#endif
	virtual void import_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort) 
	{
		if (p_size)
		{
			p_reader->read_lendian_t(m_child_guid, p_abort);
			unsigned size = 0;
			p_reader->read_lendian_t(size, p_abort);
			m_child_data.set_size(0);
			//m_child_data.set_size(size);
			pfc::array_t<t_uint8> data;
			data.set_size(size);
			p_reader->read(data.get_ptr(), size, p_abort);
			uie::window_ptr ptr;
			if (uie::window::create_by_guid(m_child_guid, ptr))
			{
				try {
					ptr->import_config(&stream_reader_memblock_ref(data.get_ptr(), data.get_size()), data.get_size(), p_abort);
				} catch (const exception_io &) {};
				m_child_data.set_size(0);
				ptr->get_config(&stream_writer_memblock_ref(m_child_data), p_abort);
			}
			//p_reader->read(m_child_data.get_ptr(), size, p_abort);
		}
	}
	virtual void export_config(stream_writer * p_writer, abort_callback & p_abort) const 
	{
		p_writer->write_lendian_t(m_child_guid, p_abort);
		uie::window_ptr ptr = m_child;
		if (!ptr.is_valid() && m_child_guid != pfc::guid_null)
		{
			if (uie::window::create_by_guid(m_child_guid, ptr))
			{
				try {
				ptr->set_config(&stream_reader_memblock_ref(m_child_data.get_ptr(), m_child_data.get_size()),m_child_data.get_size(),abort_callback_impl());
				} catch (const exception_io &) {};
			}
			else throw cui::fcl::exception_missing_panel();
		}
		pfc::array_t<t_uint8> data;
		stream_writer_memblock_ref w(data);
		if (ptr.is_valid())
			ptr->export_config(&w,abort_callback_impl());
		p_writer->write_lendian_t(data.get_size(), p_abort);
		p_writer->write(data.get_ptr(), data.get_size(), p_abort);
	}

	virtual void set_config(stream_reader * config, t_size p_size, abort_callback & p_abort)
	{
		if (p_size)
		{
			config->read_lendian_t(m_child_guid, p_abort);
			unsigned size = 0;
			config->read_lendian_t(size, p_abort);
			m_child_data.set_size(0);
			m_child_data.set_size(size);
			config->read(m_child_data.get_ptr(), size, p_abort);
		}
	};

	virtual void get_config(stream_writer * out, abort_callback & p_abort) const
	{
		out->write_lendian_t(m_child_guid, p_abort);
		if (m_child.is_valid())
		{
			pfc::array_t<t_uint8> data;
			stream_writer_memblock_ref w(data);
			m_child->get_config(&w,abort_callback_impl());
			out->write_lendian_t(data.get_size(), p_abort);
			out->write(data.get_ptr(), data.get_size(), p_abort);
		}
		else
		{
			out->write_lendian_t(m_child_data.get_size(), p_abort);
			out->write(m_child_data.get_ptr(), m_child_data.get_size(), p_abort);
		}
	};

	virtual bool have_config_popup(unsigned p_index) const
	{
		if (p_index == 0 && m_child_guid != pfc::guid_null)
		{
			uie::window_ptr p_window = m_child;
			if (!p_window.is_valid())
				uie::window::create_by_guid(m_child_guid, p_window);
			if (p_window.is_valid())
				return p_window->have_config_popup();
		}
		return false;
	}

	virtual bool show_config_popup(unsigned p_index, HWND wnd_parent)
	{
		if (p_index == 0 && m_child_guid != pfc::guid_null)
		{
			uie::window_ptr p_window = m_child;
			if (!p_window.is_valid())
			{
				if (uie::window::create_by_guid(m_child_guid, p_window))
				{
					p_window->set_config(&stream_reader_memblock_ref(m_child_data.get_ptr(), m_child_data.get_size()),m_child_data.get_size(),abort_callback_impl());
				}
			}
			if (p_window.is_valid())
			{
				bool rv =  p_window->show_config_popup(wnd_parent);
				if (rv)
				{
					m_child_data.set_size(0);
					p_window->get_config(&stream_writer_memblock_ref(m_child_data), abort_callback_impl());
				}
			}
		}
		return false;
	}

	void on_child_position_change()
	{
		reset_size_limits();
		get_host()->on_size_limit_change(get_wnd(), uie::size_limit_maximum_height|uie::size_limit_maximum_width|uie::size_limit_minimum_height|uie::size_limit_minimum_width);
		//on_size();
	}

private:
	static HFONT g_font;

	GUID m_child_guid;
	pfc::array_t<t_uint8> m_child_data;
	service_ptr_t<window_host_impl> m_host;
	ui_extension::window_ptr m_child;
	HWND m_child_wnd;
	HWND m_host_wnd;

	unsigned m_child_top;

	MINMAXINFO mmi;
};

ui_extension::window_factory<playlists_tabs_extension> blah;
ui_extension::window_host_factory<playlists_tabs_extension::window_host_impl> g_tab_host;

HFONT playlists_tabs_extension::g_font
 = 0;

void playlists_tabs_extension::get_supported_panels(const pfc::list_base_const_t<uie::window::ptr> & p_windows, bit_array_var & p_mask_unsupported)
{
	service_ptr_t<service_base> temp;
	g_tab_host.instance_create(temp);
	uie::window_host_ptr ptr;
	if (temp->service_query_t(ptr))
		(static_cast<playlists_tabs_extension::window_host_impl*>(ptr.get_ptr()))->set_this(this);
	t_size i, count = p_windows.get_count();
	for(i=0;i<count;i++)
		p_mask_unsupported.set(i, !p_windows[i]->is_available(ptr));
}

bool playlists_tabs_extension::is_point_ours(HWND wnd_point, const POINT & pt_screen, pfc::list_base_t<uie::window::ptr> & p_hierarchy)
{
	if (wnd_point == get_wnd() || IsChild(get_wnd(), wnd_point))
	{
		if (wnd_point == get_wnd() || wnd_point == wnd_tabs)
		{
			p_hierarchy.add_item(this);
			return true;
		}
		else
		{
			{
				uie::splitter_window_v2_ptr sptr;
				if (m_child.is_valid())
				{
					if (m_child->service_query_t(sptr))
					{
						pfc::list_t<uie::window::ptr> temp;
						temp.add_item(this);
						if (sptr->is_point_ours(wnd_point, pt_screen, temp))
						{
							p_hierarchy.add_items(temp);
							return true;
						}
					}
					else if (wnd_point == m_child_wnd || IsChild(m_child_wnd , wnd_point))
					{
						p_hierarchy.add_item(this);
						p_hierarchy.add_item(m_child);
						return true;
					}
				}
			}
		}
	}
	return false;
};

void playlists_tabs_extension::on_font_change()
{
	if (g_font!=0)
	{
		unsigned n, count = list_wnd.get_count();
		for (n=0; n<count; n++)
		{
			HWND wnd = list_wnd[n]->wnd_tabs;
			if (wnd) uSendMessage(wnd,WM_SETFONT,(WPARAM)0,MAKELPARAM(0,0));
		}
		DeleteObject(g_font);
	}

	g_font = static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_playlist_switcher_tabs_font);

	unsigned n, count = list_wnd.get_count();
	for (n=0; n<count; n++)
	{
		HWND wnd = list_wnd[n]->wnd_tabs;
		if (wnd) 
		{
			uSendMessage(wnd,WM_SETFONT,(WPARAM)g_font,MAKELPARAM(1,0));
			list_wnd[n]->on_size();
			list_wnd[n]->on_child_position_change();
		}
	}
}



pfc::ptr_list_t<playlists_tabs_extension> playlists_tabs_extension::list_wnd;


#if 0

class callback_manager_tabs : public playlist_callback
{
public:

	virtual void on_items_added(unsigned p_playlist,unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection){};
	virtual void on_items_reordered(unsigned p_playlist,const unsigned * order,unsigned count){};
	virtual void on_items_removing(unsigned p_playlist,const bit_array & mask){};
	virtual void on_items_removed(unsigned p_playlist,const bit_array & mask){};
	virtual void on_items_selection_change(unsigned p_playlist,const bit_array & affected,const bit_array & state){};
	virtual void on_item_focus_change(unsigned p_playlist,unsigned from,unsigned to){};
	virtual void on_items_modified(unsigned p_playlist,const bit_array & p_mask){};
	virtual void on_items_replaced(unsigned p_playlist,const bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data){};

	virtual void on_item_ensure_visible(unsigned p_playlist,unsigned idx){};

	virtual void on_playlist_activate(unsigned p_old,unsigned p_new);
	virtual void on_playlist_created(unsigned p_index,const char * p_name,unsigned p_name_len);
	virtual void on_playlists_reorder(const unsigned * p_order,unsigned p_count);
	virtual void on_playlists_removing(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count){};
	virtual void on_playlists_removed(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count);
	virtual void on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len);

	virtual void on_default_format_changed() {};
	virtual void on_playback_order_changed(unsigned p_new_index) {};
	virtual void on_playlist_locked(unsigned p_playlist,bool p_locked) {};

};


void callback_manager_tabs::on_playlist_activate(unsigned p_old,unsigned p_new)
{
	unsigned n, count = playlists_tabs_extension::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_extension::list_wnd[n]->on_playlist_activate(p_old,p_new);
}

void callback_manager_tabs::on_playlist_created(unsigned p_index,const char * p_name,unsigned p_name_len)
{
	unsigned n, count = playlists_tabs_extension::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_extension::list_wnd[n]->on_playlist_created(p_index,p_name,p_name_len);
}

void callback_manager_tabs::on_playlists_reorder(const unsigned * p_order,unsigned p_count)
{
	unsigned n, count = playlists_tabs_extension::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_extension::list_wnd[n]->on_playlists_reorder(p_order, p_count);
}

void callback_manager_tabs::on_playlists_removed(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count)
{
	unsigned n, count = playlists_tabs_extension::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_extension::list_wnd[n]->on_playlists_removed(p_mask, p_old_count, p_new_count);
}

void callback_manager_tabs::on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len)
{
	unsigned n, count = playlists_tabs_extension::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_extension::list_wnd[n]->on_playlist_renamed(p_index,p_new_name,p_new_name_len);
}

static service_factory_single_t<playlist_callback,callback_manager_tabs> hgf;

#endif


void playlists_tabs_extension::kill_switch_timer()
{
	if (m_switch_timer)
	{
		m_switch_timer = 0;
		KillTimer(get_wnd(), SWITCH_TIMER_ID);
	}
}

void playlists_tabs_extension::switch_to_playlist_delayed2(unsigned idx)
{
	//if have a timer already and idxs re same dont bother
	if (!(m_switch_timer && idx == m_switch_playlist))
	{
		if (m_switch_timer) kill_switch_timer();
		m_switch_playlist = idx;
		m_playlist_switched = false;
		m_switch_timer = (SetTimer(get_wnd(), SWITCH_TIMER_ID, cfg_autoswitch_delay, 0) !=0);
	}
}


playlists_tabs_extension::playlists_tabs_extension() : m_dragging(false), m_dragging_idx(0), m_playlist_switched(false), 
m_switch_timer(false), m_switch_playlist(0), initialised(false), wnd_tabs(0),
m_child_guid(/*cfg_default_playlist*/pfc::guid_null), m_child_wnd(0), m_host_wnd(0), m_child_top(0),
ID_CUSTOM_BASE(NULL), m_mousewheel_delta(0)
{
	//reset_size_limits();
	memset(&mmi, 0, sizeof(mmi));
	mmi.ptMaxTrackSize.x = MAXLONG;
	mmi.ptMaxTrackSize.y = MAXLONG;
	m_dragging_rect.left=0;
	m_dragging_rect.top=0;
	m_dragging_rect.right=0;
	m_dragging_rect.bottom=0;
};


playlists_tabs_extension::~playlists_tabs_extension()
{
}

LRESULT WINAPI playlists_tabs_extension::main_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	playlists_tabs_extension * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<playlists_tabs_extension*>(uGetWindowLong(wnd,GWL_USERDATA));

	rv = p_this ? p_this->hook(wnd,msg,wp,lp) : uDefWindowProc(wnd, msg, wp, lp);;

	return rv;
}

void playlists_tabs_extension::create_child()
{
	destroy_child();
	if (m_host.is_valid())
	{
		ui_extension::window::create_by_guid(m_child_guid, m_child);
		if (m_child.is_valid())
		{
			try {
			m_child->set_config(&stream_reader_memblock_ref(m_child_data.get_ptr(), m_child_data.get_size()),m_child_data.get_size(),abort_callback_impl());
			} catch (const exception_io &) {};
			m_child_wnd = m_child->create_or_transfer_window(m_host_wnd, ui_extension::window_host_ptr(m_host.get_ptr()));
			if (m_child_wnd)
			{
				//ShowWindow(m_child_wnd, SW_SHOWNORMAL);
				SetWindowPos(m_child_wnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			}
		}
	}
	reset_size_limits();
	on_size();
	if (IsWindowVisible(get_wnd()))
	{
		get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
		ShowWindow(m_child_wnd, SW_SHOWNORMAL);
	}
}

void playlists_tabs_extension::destroy_child()
{
	if (m_child.is_valid())
	{
		m_child->destroy_window();
		m_child_wnd=0;
		m_child.release();
		reset_size_limits();
		if (IsWindowVisible(get_wnd()))
			get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
	}
}

bool playlists_tabs_extension::create_tabs()
{
	bool rv = false;
	bool force_close = false;

	static_api_ptr_t<playlist_manager> playlist_api;

	if (cfg_pl_autohide)
	{
		force_close = (playlist_api->get_playlist_count() <= 1);
	}

	if (wnd_tabs && force_close) {DestroyWindow(wnd_tabs); wnd_tabs = 0; rv = true;}
	else if (!wnd_tabs && !force_close) 
	{
		int i, t = playlist_api->get_playlist_count();
		pfc::string8 temp;

		int x = 0;
		int y = 0;
		int cx = 0;//bah
		int cy = 0;

		wnd_tabs = CreateWindowEx(0, WC_TABCONTROL, _T("Playlist switcher"),
			WS_CHILD |  WS_TABSTOP | TCS_HOTTRACK | TCS_TABS | (t>1 ? TCS_MULTILINE : 0) | (1 ? WS_VISIBLE : 0), x, y, cx, cy,
			m_host_wnd, HMENU(5002), core_api::get_my_instance(), NULL);

		if (wnd_tabs)
		{
			uSetWindowLong(wnd_tabs,GWL_USERDATA,(LPARAM)(this));

			if (g_font)
			{
				uSendMessage(wnd_tabs,WM_SETFONT,(WPARAM)g_font,MAKELPARAM(0,0));
			}
			else
				on_font_change();

			SetWindowPos(wnd_tabs,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

			tabproc = (WNDPROC)uSetWindowLong(wnd_tabs,GWL_WNDPROC,(LPARAM)main_hook);

			pfc::string8 temp2;
			for (i = 0; i < t; i++)
			{
				playlist_api->playlist_get_name(i, temp);
				uTabCtrl_InsertItemText(wnd_tabs, i, temp);
			}

			TabCtrl_SetCurSel(wnd_tabs, playlist_api->get_active_playlist());
			set_styles();

			RECT rc;
			GetWindowRect(wnd_tabs, &rc);
			adjust_rect(FALSE, &rc);
			m_child_top = rc.top;

			rv = true;
		}

	}
	if (rv) 
	{
		reset_size_limits();
		get_host()->on_size_limit_change(get_wnd(), uie::size_limit_maximum_height|uie::size_limit_maximum_width|uie::size_limit_minimum_height|uie::size_limit_minimum_width);
		on_size();
	}
	return rv;
}

enum {ID_SWITCH = 1, ID_REMOVE, ID_RENAME, ID_NEW, ID_SAVE, ID_SAVE_ALL, ID_LOAD, ID_UP, ID_DOWN, ID_CUT, ID_COPY, ID_PASTE, ID_AUTOPLAYLIST, ID_RECYCLER_CLEAR, ID_RECYCLER_BASE};

LRESULT playlists_tabs_extension::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch (msg)
	{
	case WM_NCCREATE:
		{
			m_host_wnd=wnd;
			break;
		}
	case WM_CREATE:
		{
			initialised=true;
			list_wnd.add_item(this);
			pfc::com_ptr_t< playlists_tabs_drop_target > m_drop_target = new playlists_tabs_drop_target(this);
			RegisterDragDrop(wnd, m_drop_target.get_ptr());

			create_tabs();

			service_ptr_t<service_base> p_temp;

			g_tab_host.instance_create(p_temp);

			//Well simple reinterpret_cast without this mess should work fine but this is "correct"
			m_host = static_cast<window_host_impl*>(p_temp.get_ptr());
			if (m_host.is_valid())
			{
				m_host->set_this(this);
				create_child();
			}
			static_api_ptr_t<playlist_manager>()->register_callback(this, playlist_callback::flag_all);
			break;
		}

	case WM_SHOWWINDOW:
		{
			if (wp == TRUE && lp == NULL && !IsWindowVisible(m_child_wnd))
			{
				ShowWindow(m_child_wnd, SW_SHOWNORMAL);
			}
			break;
		}
	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
			if (!(lpwp->flags & SWP_NOSIZE))
			{
				on_size(lpwp->cx, lpwp->cy);
			}
		}
		break;
	/*case WM_USER+3:
		on_child_position_change();
		break;*/
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = LPMINMAXINFO(lp);
			*lpmmi = mmi;
		}
		return 0;
	case WM_DESTROY:
		{
			static_api_ptr_t<playlist_manager>()->unregister_callback(this);
			destroy_child();
			m_host.release();
			RevokeDragDrop(wnd);
			if (wnd_tabs)
				DestroyWindow(wnd_tabs);
			wnd_tabs=0;
			list_wnd.remove_item(this);
			if (!list_wnd.get_count())
			{
				uSendMessage(wnd, WM_SETFONT, 0, 0);
				if (g_font) DeleteObject(g_font);
				g_font=0;
			}
			initialised=false;
		}
		break;
	case WM_NCDESTROY:
		{
			m_host_wnd=0;
		}
		break;
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONUP:
		{
			if (wnd_tabs) 
			{

				POINT temp;
				temp.x = GET_X_LPARAM(lp);
				temp.y = GET_Y_LPARAM(lp);

				HWND wnd_hit = ChildWindowFromPointEx(wnd, temp, CWP_SKIPINVISIBLE);

				if (wnd_hit == wnd_tabs)
				{
					TCHITTESTINFO hittest;
					hittest.pt.x = temp.x;
					hittest.pt.y = temp.y;
					int idx = TabCtrl_HitTest(wnd_tabs, &hittest);

					if (idx < 0) 
					{
						static_api_ptr_t<playlist_manager> playlist_api;
						unsigned new_idx = playlist_api->create_playlist("Untitled",12,playlist_api->get_playlist_count());
						playlist_api->set_active_playlist(new_idx);
						return 0;
					}
				}
			}
		}
		break;
	case WM_TIMER:
		if (wp == SWITCH_TIMER_ID)
		{
			m_switch_timer = 0;
			KillTimer(wnd, SWITCH_TIMER_ID);
			if (!m_playlist_switched)
			{
				static_api_ptr_t<playlist_manager> playlist_api;
				if (m_switch_playlist < playlist_api->get_playlist_count())
					playlist_api->set_active_playlist(m_switch_playlist);

				m_playlist_switched = true;
			}
			return 0;
		}
		break;
	case WM_MENUSELECT:
		{
			if (HIWORD(wp) & MF_POPUP)
			{
				m_status_override.release();
			}
			else 
			{
				if (p_manager.is_valid())
				{
					unsigned id = LOWORD(wp);

					bool set = true;

					pfc::string8 blah;

					if (id >= ID_CUSTOM_BASE)
					{
						::contextmenu_node * node = p_manager->find_by_id(id - ID_CUSTOM_BASE);
						if (node) set = node->get_description(blah);
					}
					else if (id == ID_SWITCH)
					{
						blah = "Activates this playlist.";
					}
					else if (id == ID_REMOVE)
					{
						blah = "Removes this playlist.";
					}
					else if (id == ID_RENAME)
					{
						blah = "Renames this playlist.";
					}
					else if (id == ID_NEW)
					{
						blah = "Creates a new playlist.";
					}
					else if (id == ID_LOAD)
					{
						blah = "Loads an existing playlist from a file.";
					}
					else if (id == ID_SAVE_ALL)
					{
						blah = "Saves all playlists to individual files.";
					}
					else if (id == ID_SAVE)
					{
						blah = "Saves this playlist to a file.";
					}
					else if (id == ID_UP)
					{
						blah = "Moves this playlist up one position.";
					}
					else if (id == ID_DOWN)
					{
						blah = "Moves this playlist down one position.";
					}
					else if (id == ID_COPY)
					{
						blah = "Copies the selected items to the Clipboard.";
					}
					else if (id == ID_CUT)
					{
						blah = "Removes the selected items and copies them to the Clipboard.";
					}
					else if (id == ID_PASTE)
					{
						blah = "Inserts the items you have copied or cut to the selected location.";
					}
					else if (id == ID_AUTOPLAYLIST)
					{
						blah = "Open autoplaylist properties.";
					}
					else
						set = false;

					service_ptr_t<ui_status_text_override> p_status_override;

					if (set)
					{
						get_host()->override_status_text_create(p_status_override);

						if (p_status_override.is_valid())
						{
							p_status_override->override_text(blah);
						}
					}
					m_status_override = p_status_override;
				}
			}
			break;
		}
	case WM_CONTEXTMENU:
		if (wnd_tabs)
		{
			uie::window_ptr p_this_temp = this;
			POINT pt = {(short)LOWORD(lp),(short)HIWORD(lp)};
			int old_idx=0;
			unsigned idx=0;

			bool b_keyb_invoked = pt.x==-1 && pt.y == -1;
			
			POINT pt_client = pt;
			ScreenToClient(wnd_tabs,&pt_client);

			RECT rc;
			if (m_child_wnd)
			{
				GetClientRect(m_child_wnd, &rc);
				MapWindowPoints(m_child_wnd, wnd_tabs, (LPPOINT)&rc, 2);
			}
			if (!b_keyb_invoked && m_child_wnd && m_child.is_valid() && PtInRect(&rc, pt_client))
			{
				pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> menu_hook_t = new ui_extension::menu_hook_impl;
				m_child->get_menu_items(*menu_hook_t.get_ptr());
				HMENU menu = CreatePopupMenu();
				menu_hook_t->win32_build_menu(menu, 1, pfc_infinite);
				int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);
				menu_hook_t->execute_by_id(cmd);
				DestroyMenu(menu);
			}
			else
			{
				if (b_keyb_invoked)
				{
					idx = TabCtrl_GetCurSel(wnd_tabs);
					RECT rc_sel;
					if (TabCtrl_GetItemRect(wnd_tabs, idx, &rc_sel))
					{
						MapWindowPoints(wnd_tabs, HWND_DESKTOP, (LPPOINT)&rc_sel, 2);
						pt.x = rc_sel.left+(rc_sel.right-rc_sel.left)/2;
						pt.y = rc_sel.top+(rc_sel.bottom-rc_sel.top)/2;
					}
					else GetMessagePos(&pt);

				}
				else
				{
					TCHITTESTINFO hittest;
					hittest.pt.x = pt_client.x;
					hittest.pt.y = pt_client.y;
					idx = TabCtrl_HitTest(wnd_tabs, &hittest);
				}


				static_api_ptr_t<playlist_manager_v3> playlist_api;

				unsigned num = playlist_api->get_playlist_count(), active = playlist_api->get_active_playlist();
				bool b_index_valid = idx<num;
			
				metadb_handle_list_t<pfc::alloc_fast_aggressive> data;

				static_api_ptr_t<autoplaylist_manager> autoplaylist_api;
				autoplaylist_client_v2::ptr autoplaylist;

				try
				{
					autoplaylist_client::ptr ptr = autoplaylist_api->query_client(idx);
					ptr->service_query_t(autoplaylist);
				} catch (pfc::exception const &) {};

				HMENU menu = CreatePopupMenu();

				playlist_position_reference_tracker position_tracker(false);
				position_tracker.m_playlist = idx;

				if (b_index_valid)
				{
					if (active != idx)
						AppendMenu(menu,MF_STRING ,ID_SWITCH,_T("Activate"));
					AppendMenu(menu,MF_STRING,ID_RENAME,_T("Rename..."));
					AppendMenu(menu,MF_STRING,ID_REMOVE,_T("Remove"));
					if (idx>0)
						AppendMenu(menu,MF_STRING,ID_UP,_T("Move left"));
					if (idx+1<num)
						AppendMenu(menu,MF_STRING,ID_DOWN,_T("Move right"));
					if (autoplaylist.is_valid() && autoplaylist->show_ui_available())
					{
						AppendMenu(menu,MF_SEPARATOR,0,0);

						pfc::string8 name;
						autoplaylist->get_display_name(name);
						name << " properties";

						AppendMenu(menu,MF_STRING,ID_AUTOPLAYLIST,uT(name));
					}
					AppendMenu(menu,MF_SEPARATOR,0,0);

					AppendMenu(menu,MF_STRING,ID_CUT,L"Cut");
					AppendMenu(menu,MF_STRING,ID_COPY,L"Copy");
					if (playlist_manager_utils::check_clipboard())
						AppendMenu(menu,MF_STRING,ID_PASTE,L"Paste");
					AppendMenu(menu,MF_SEPARATOR,0,NULL);
				}

				AppendMenu(menu,MF_STRING,ID_NEW,_T("New"));
				AppendMenu(menu,MF_STRING,ID_LOAD,_T("Load..."));
				if (b_index_valid)
				{
					AppendMenu(menu,MF_STRING,ID_SAVE,_T("Save as..."));
				}

				if (num)
					AppendMenu(menu,MF_STRING,ID_SAVE_ALL,_T("Save all as..."));
				pfc::array_t<t_size> recycler_ids;
				{
					t_size recycler_count = playlist_api->recycler_get_count();
					if (recycler_count)
					{
						recycler_ids.set_count(recycler_count);
						HMENU recycler_popup = CreatePopupMenu();
						pfc::string8_fast_aggressive temp;
						t_size i;
						for (i=0; i<recycler_count; i++)
						{
							playlist_api->recycler_get_name(i, temp);
							recycler_ids[i] = playlist_api->recycler_get_id(i); //Menu Message Loop !
							uAppendMenu(recycler_popup, MF_STRING, ID_RECYCLER_BASE+i, temp);
						}
						AppendMenu(recycler_popup,MF_SEPARATOR,0,0);
						AppendMenu(recycler_popup, MF_STRING, ID_RECYCLER_CLEAR, _T("Clear"));
						AppendMenu(menu, MF_POPUP, (UINT_PTR)recycler_popup, _T("History"));
					}
					ID_CUSTOM_BASE = ID_RECYCLER_BASE + recycler_count;
				}
				if (b_index_valid)
				{

					data.prealloc(playlist_api->playlist_get_item_count(idx));
					playlist_api->playlist_get_all_items(idx, data);

					MENUITEMINFO mi;
					memset(&mi, 0, sizeof(mi));
					mi.cbSize = sizeof(MENUITEMINFO);
					mi.fMask = MIIM_STATE;
					mi.fState = MFS_DEFAULT;

					SetMenuItemInfo(menu, (active != idx) ? ID_SWITCH : ID_RENAME, FALSE, &mi);
				}


				if (data.get_count() >0)
				{
					uAppendMenu(menu,MF_SEPARATOR,0,0);

					HMENU submenu = CreatePopupMenu();

					contextmenu_manager::g_create(p_manager);
					if (p_manager.is_valid())
					{
						p_manager->init_context(data,0);

						p_manager->win32_build_menu(submenu,ID_CUSTOM_BASE,-1);
					}
					AppendMenu(menu,MF_POPUP,(UINT_PTR)submenu,_T("Items"));
				}
				menu_helpers::win32_auto_mnemonics(menu);

				int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);

				m_status_override.release();

				DestroyMenu(menu);


				num = playlist_api->get_playlist_count();
				active = playlist_api->get_active_playlist();
				b_index_valid = idx<num;

				if (cmd)
				{
					if (cmd>=ID_CUSTOM_BASE)
					{
						if (p_manager.is_valid())
						{
							p_manager->execute_by_id(cmd - ID_CUSTOM_BASE);
						}
					}
					else if (cmd >= ID_RECYCLER_BASE)
					{
						if (cmd - ID_RECYCLER_BASE < recycler_ids.get_count())
							playlist_api->recycler_restore_by_id(recycler_ids[cmd - ID_RECYCLER_BASE]);
					}
					else
					{
						switch(cmd)
						{
						case ID_AUTOPLAYLIST:
							if (autoplaylist.is_valid())
								autoplaylist->show_ui(position_tracker.m_playlist);
							break;
						case ID_RECYCLER_CLEAR:
							playlist_api->recycler_purge(bit_array_true());
							break;
						case ID_CUT:
							if (b_index_valid) playlist_manager_utils::cut(pfc::list_single_ref_t<t_size>(idx));
							break;
						case ID_COPY:
							if (b_index_valid) playlist_manager_utils::copy(pfc::list_single_ref_t<t_size>(idx));
							break;
						case ID_PASTE:
							if (b_index_valid) playlist_manager_utils::paste(wnd, idx+1);
							break;
						case ID_SWITCH:
							if (b_index_valid)
							{
								playlist_api->set_active_playlist(idx);
								old_idx = idx;							
							}
							break;
						case ID_REMOVE:
							if (b_index_valid) remove_playlist_helper(idx);								
							break;
						case ID_RENAME:
							if (b_index_valid)
							{
								pfc::string8 temp;
								if (playlist_api->playlist_get_name(idx,temp))
								{
									if (g_rename(&temp,wnd))
									{//fucko: dialogobx has a messgeloop, someone might have called switcher api funcs in the meanwhile
										//			idx = ((HWND)wp == wnd_tabs) ? idx : uSendMessage(g_plist,LB_GETCURSEL,0,0);
										num = playlist_api->get_playlist_count();
										if ((signed)idx>=0 && idx<num)
										{
											playlist_api->playlist_rename(idx,temp,-1);
										}
									}
								}
							}							
							break;
						case ID_NEW:
							{
								metadb_handle_list data;
								playlist_api->playlist_add_items(playlist_api->create_playlist(pfc::string8("Untitled"),-1,playlist_api->get_playlist_count()),data, bit_array_false());
							}
							break;
						case ID_SAVE:
							{
								pfc::string8 name;
								playlist_api->playlist_get_name(idx, name);
								g_save_playlist(wnd, data, name);
							}
							break;
						case ID_LOAD:
							{
								standard_commands::main_load_playlist();
							}
							break;
						case ID_SAVE_ALL:
							{
								standard_commands::main_save_all_playlists();
							}
							break;
						case ID_UP:
							if (idx>0)
							{
								order_helper order(num);
								order.swap(idx,idx-1);
								playlist_api->reorder(order.get_ptr(),num);
							}
							break;
						case ID_DOWN:
							if (idx+1<num)
							{
								order_helper order(num);
								order.swap(idx,idx+1);
								playlist_api->reorder(order.get_ptr(),num);
							}
							break;
						}
					}
				}
				p_manager.release();
				ID_CUSTOM_BASE = NULL;
				data.remove_all();
			}
		} 
		return 0;
	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lp)->idFrom)
			{
			case 5002:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
					{
						static_api_ptr_t<playlist_manager>()->set_active_playlist(TabCtrl_GetCurSel(((LPNMHDR)lp)->hwndFrom));
					}
					break;
				}
				break;
			}
		}
		break;
	}
	return uDefWindowProc(wnd, msg, wp, lp);
}

const GUID & playlists_tabs_extension::get_extension_guid() const
{
	return extension_guid;
}

void playlists_tabs_extension::get_name(pfc::string_base & out)const
{
	out.set_string("Playlist tabs");
}
void playlists_tabs_extension::get_category(pfc::string_base & out)const
{
	out.set_string("Splitters");
}
bool playlists_tabs_extension::get_short_name(pfc::string_base & out)const
{
	out.set_string("Playlist tabs");
	return true;
}

// {ABB72D0D-DBF0-4bba-8C68-3357EBE07A4D}
const GUID playlists_tabs_extension::extension_guid = 
{ 0xabb72d0d, 0xdbf0, 0x4bba, { 0x8c, 0x68, 0x33, 0x57, 0xeb, 0xe0, 0x7a, 0x4d } };




LRESULT WINAPI playlists_tabs_extension::hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		{
			if (wp != VK_LEFT && wp != VK_RIGHT && get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp)) return 0;
			else if (wp == VK_TAB)
			{
				ui_extension::window::g_on_tab(wnd);
				//return 0;
			}
			SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
		}
		break;
	case WM_SYSKEYDOWN:
		if (get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp)) return 0;
		break;
	case WM_LBUTTONDOWN:
		{
			{
				TCHITTESTINFO hittest;
				hittest.pt.x = GET_X_LPARAM(lp);
				hittest.pt.y = GET_Y_LPARAM(lp);
				int idx = TabCtrl_HitTest(wnd_tabs, &hittest);
				if (idx>=0)
				{
					static_api_ptr_t<playlist_manager> playlist_api;
					//if (cfg_playlists_shift_lmb && (wp & MK_SHIFT)) remove_playlist_helper(idx);
					//else 
						if (cfg_drag_pl)
					{
						SetCapture(wnd);
						m_dragging = true;
						m_dragging_idx = idx;
						TabCtrl_GetItemRect(wnd, idx, &m_dragging_rect);
					}
				}
			}
		}
		break;
	case WM_MOUSEMOVE:
		if (m_dragging && (wp & MK_LBUTTON))
		{
			TCHITTESTINFO hittest;
			hittest.pt.x = GET_X_LPARAM(lp);
			hittest.pt.y = GET_Y_LPARAM(lp);
			int idx = TabCtrl_HitTest(wnd_tabs, &hittest);
			if (idx>=0 && !PtInRect(&m_dragging_rect, hittest.pt))
			{
				int cur_idx = m_dragging_idx;
				static_api_ptr_t<playlist_manager> playlist_api;
				int count = playlist_api->get_playlist_count();

				int n = cur_idx;
				order_helper order(count);
				if (n < idx)
				{
					while (n<idx && n < count)
					{
						order.swap(n,n+1);
						n++;
					}
				}
				else if (n > idx)
				{
					while (n>idx && n > 0)
					{
						order.swap(n,n-1);
						n--;
					}
				}
				if (n != cur_idx) 
				{
					TabCtrl_GetItemRect(wnd, n, &m_dragging_rect);
					playlist_api->reorder(order.get_ptr(),count);
					m_dragging_idx = n;
				}
			}
		}
		else m_dragging = false;

		break;
	case WM_LBUTTONUP:
		if (m_dragging)
		{
			m_dragging = false;
			ReleaseCapture();
		}
		break;
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONUP:
		{
			if (cfg_mclick || 1 || cfg_plm_rename) //in actuality we dont get messages when mouse not on tab here.
			{
				TCHITTESTINFO hittest;
				hittest.pt.x = GET_X_LPARAM(lp);
				hittest.pt.y = GET_Y_LPARAM(lp);
				int idx = TabCtrl_HitTest(wnd_tabs, &hittest);
				static_api_ptr_t<playlist_manager> playlist_api;
				if (idx >= 0) 
				{
					if (cfg_mclick && msg == WM_MBUTTONUP) {remove_playlist_helper(idx);}
					if (cfg_plm_rename && msg == WM_LBUTTONDBLCLK) {rename_playlist(idx);}
				}
				else if (1) 
				{
					unsigned new_idx = playlist_api->create_playlist(pfc::string8("Untitled"),pfc_infinite,playlist_api->get_playlist_count());
					playlist_api->set_active_playlist(new_idx);
				}
			}
		}
		break;
	case WM_MOUSEWHEEL:
		if ((GetWindowLongPtr(wnd, GWL_STYLE) & TCS_MULTILINE) == NULL)
		{
			//unsigned scroll_lines = GetNumScrollLines();

			HWND wnd_child = GetWindow(wnd, GW_CHILD);
			WCHAR str_class [129];
			memset(str_class, 0, sizeof(str_class));
			if (wnd_child && RealGetWindowClass (wnd_child, str_class, tabsize(str_class)-1) && !wcscmp(str_class, UPDOWN_CLASS) && IsWindowVisible(wnd_child))
			{

				INT min = NULL, max = NULL, index=NULL;
				BOOL err = FALSE;
				SendMessage(wnd_child, UDM_GETRANGE32, (WPARAM)&min, (LPARAM)&max);
				index = SendMessage(wnd_child, UDM_GETPOS32, (WPARAM)NULL, (LPARAM)&err);

				//if (!err)
				{
					if (max)
					{
						int zDelta = short(HIWORD(wp));

						//int delta = MulDiv(zDelta, scroll_lines, 120);
						m_mousewheel_delta += zDelta;
						int scroll_lines = 1;//GetNumScrollLines();
						//if (scroll_lines == -1)
							//scroll_lines = count;

						if (m_mousewheel_delta*scroll_lines >= WHEEL_DELTA)
						{
							if (index>min)
							{
								SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, index-1), NULL);
								SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
								SendMessage(wnd_child, UDM_SETPOS32, NULL, index-1);
							}
							m_mousewheel_delta=0;
						}
						else if (m_mousewheel_delta*scroll_lines <= -WHEEL_DELTA)
						{
							if (index+1<=max)
							{
								SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, index+1), NULL);
								SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
								SendMessage(wnd_child, UDM_SETPOS32, NULL, index+1);
							}
							m_mousewheel_delta=0;
						}
					}
				}

				return 0;
			}
		}
		break;
	}
	return uCallWindowProc(tabproc,wnd,msg,wp,lp);
}

HRESULT STDMETHODCALLTYPE playlists_tabs_extension::playlists_tabs_drop_target::QueryInterface(REFIID riid, LPVOID FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	*ppvObject = NULL;
	if (riid == IID_IUnknown) {AddRef();*ppvObject = (IUnknown*)this;return S_OK;}
	else if (riid == IID_IDropTarget) {AddRef();*ppvObject = (IDropTarget*)this;return S_OK;}
	else return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE   playlists_tabs_extension::playlists_tabs_drop_target::AddRef()
{
	return InterlockedIncrement(&drop_ref_count);
}
ULONG STDMETHODCALLTYPE   playlists_tabs_extension::playlists_tabs_drop_target::Release()
{
	LONG rv = InterlockedDecrement(&drop_ref_count);
	if (!rv)
	{
#ifdef _DEBUG
		OutputDebugString(_T("deleting playlists_tabs_extension"));
#endif
		delete this;
	}
	return rv;
}

HRESULT STDMETHODCALLTYPE playlists_tabs_extension::playlists_tabs_drop_target::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	POINT pt = {ptl.x, ptl.y};
	if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragEnter(p_list->get_wnd(), pDataObj, &pt, *pdwEffect);
	m_DataObject = pDataObj;

	last_over.x = 0;
	last_over.y = 0;
	m_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

	if (ui_drop_item_callback::g_is_accepted_type(pDataObj, pdwEffect))
	{
		return S_OK; 	
	}
	else if (static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check(pDataObj))
	{
		*pdwEffect = DROPEFFECT_COPY;
		return S_OK; 	
	}
	return S_FALSE; 	
}


HRESULT STDMETHODCALLTYPE playlists_tabs_extension::playlists_tabs_drop_target::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
		POINT pt = {ptl.x, ptl.y};
		if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragOver(&pt, *pdwEffect);

	*pdwEffect = DROPEFFECT_COPY;
	m_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

	if (last_over.x != pt.x || last_over.y != pt.y)
	{

		static_api_ptr_t<playlist_manager> playlist_api;
		POINT pti,ptm;
		pti.y = pt.y;
		pti.x = pt.x;

		ptm = pti;
		ScreenToClient(p_list->get_wnd(), &ptm);

		HWND wnd = ChildWindowFromPointEx(p_list->get_wnd(), ptm, CWP_SKIPINVISIBLE);

		//	RECT plist;

		//	GetWindowRect(g_plist, &plist);
		//	RECT tabs;

		//	GetWindowRect(g_tab, &tabs);

		if (p_list->wnd_tabs)
		{

			POINT pttab;
			pttab = pti;


			if (wnd== p_list->wnd_tabs && ScreenToClient(p_list->wnd_tabs,&pttab))
			{
				TCHITTESTINFO hittest;
				hittest.pt.x = pttab.x;
				hittest.pt.y = pttab.y;
				int idx = TabCtrl_HitTest(p_list->wnd_tabs, &hittest);
				int old = playlist_api->get_active_playlist();
				if (cfg_drag_autoswitch && idx >= 0 && old != idx) p_list->switch_to_playlist_delayed2(idx);//playlist_switcher::get()->set_active_playlist(idx);
				else p_list->kill_switch_timer();

				if (idx != -1)
				{
					pfc::string8 name;
					static_api_ptr_t<playlist_manager>()->playlist_get_name(idx, name);
					mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_COPY, "Add to %1", name);
				}
				else
					mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_COPY, "Add to new playlist", "");
			}
			else
				mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_COPY, "Add to new playlist", "");

		}
		if ((!p_list->wnd_tabs || wnd != p_list->wnd_tabs))  p_list->kill_switch_timer();

		last_over = ptl;
	}

	return S_OK; 



}

HRESULT STDMETHODCALLTYPE playlists_tabs_extension::playlists_tabs_drop_target::DragLeave( void)
{
	if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragLeave();
	last_over.x = 0;
	last_over.y = 0;
	p_list->kill_switch_timer();
	mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");
	m_DataObject.release();

	return S_OK;		
}

HRESULT STDMETHODCALLTYPE playlists_tabs_extension::playlists_tabs_drop_target::Drop( IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{ 
	POINT pt = {ptl.x, ptl.y};
	if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->Drop(pDataObj, &pt, *pdwEffect);

	p_list->kill_switch_timer();

	static_api_ptr_t<playlist_manager> playlist_api;

	POINT pti,ptm;
	pti.y = pt.y;
	pti.x = pt.x;

	ptm = pti;
	ScreenToClient(p_list->get_wnd(), &ptm);

	HWND wnd = ChildWindowFromPointEx(p_list->get_wnd(), ptm, CWP_SKIPINVISIBLE);

	if (wnd)
	{

		bool process = !ui_drop_item_callback::g_on_drop(pDataObj);

		bool send_new_playlist = false;

		if (process && m_last_rmb)
		{
			process = false;
			enum {ID_DROP = 1, ID_NEW_PLAYLIST, ID_CANCEL };

			HMENU menu = CreatePopupMenu();

			uAppendMenu(menu,(MF_STRING),ID_DROP,"&Add files here");
			uAppendMenu(menu,(MF_STRING),ID_NEW_PLAYLIST,"&Add files to new playlist");
			uAppendMenu(menu,MF_SEPARATOR,0,0);
			uAppendMenu(menu,MF_STRING,ID_CANCEL,"&Cancel");

			int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,p_list->get_wnd(),0);
			DestroyMenu(menu);

			if (cmd)
			{
				switch(cmd)
				{
				case ID_DROP:
					process = true;						
					break;
				case ID_NEW_PLAYLIST:
					process = true;						
					send_new_playlist = true;						
					break;
				}
			}
		}

		if (process)
		{
			metadb_handle_list data;
			static_api_ptr_t<playlist_incoming_item_filter> incoming_api;
			incoming_api->process_dropped_files(pDataObj, data,true,p_list->get_wnd());

			POINT pttab,ptpl;
			pttab = pti;
			ptpl = pti;


			int idx = -1;
			//	if ((g_tab && wnd == g_tab) || g_plist && wnd == g_plist)


			//	bool processed = false;
			t_size target_index = playlist_api->get_active_playlist();


			if (p_list->wnd_tabs && wnd == p_list->wnd_tabs && !send_new_playlist)
			{
				RECT tabs;

				GetWindowRect(p_list->wnd_tabs, &tabs);
				if (ScreenToClient(p_list->wnd_tabs,&pttab))
				{
					TCHITTESTINFO hittest;
					hittest.pt.x = pttab.x;
					hittest.pt.y = pttab.y;
					int idx = TabCtrl_HitTest(p_list->wnd_tabs, &hittest);
					int old = playlist_api->get_active_playlist();
					if (idx < 0) 
					{
						send_new_playlist = true;
					}
					else target_index = idx;
				}

			}


			if (send_new_playlist)
			{
				pfc::string8 playlist_name("Untitled");

				bool named= false;

				if (1 ||1)
				{
					FORMATETC   fe;
					STGMEDIUM   sm;
					HRESULT     hr = E_FAIL;

					//					memset(&sm, 0, sizeof(0));

					fe.cfFormat = CF_HDROP;
					fe.ptd = NULL;
					fe.dwAspect = DVASPECT_CONTENT;  
					fe.lindex = -1;
					fe.tymed = TYMED_HGLOBAL;       

					// User has dropped on us. Get the data from drag source
					hr = pDataObj->GetData(&fe, &sm);
					if(SUCCEEDED(hr))
					{

						// Display the data and release it.
						pfc::string8 temp;

						unsigned int /*n,*/t = uDragQueryFileCount((HDROP)sm.hGlobal);
						if (t==1)
						{
							{
								uDragQueryFile((HDROP)sm.hGlobal, 0, temp);
								if (uGetFileAttributes(temp) & FILE_ATTRIBUTE_DIRECTORY) 
								{
									playlist_name.set_string(pfc::string_filename_ext(temp));
									named=true;
								}
								else
								{
									playlist_name.set_string(pfc::string_filename(temp));
									named=true;
#if 0
									pfc::string_extension ext(temp);

									service_enum_t<playlist_loader> e;
									service_ptr_t<playlist_loader> l;
									if (e.first(l))
										do
										{
											if (!strcmp(l->get_extension(),ext))
											{
												playlist_name.set_string(pfc::string_filename(temp));
												named=true;
												l.release();
												break;
											}
											l.release();
										} while (e.next(l));
#endif
								}

							}
						}

						ReleaseStgMedium(&sm);
					}
				}

				unsigned new_idx;

				if (named && cfg_replace_drop_underscores) playlist_name.replace_char('_',' ',0);
				if (!named && cfg_pgen_tf) new_idx = playlist_api->create_playlist(string_pn(data, cfg_pgenstring),pfc_infinite,playlist_api->get_playlist_count());

				else  new_idx= playlist_api->create_playlist(playlist_name,pfc_infinite, playlist_api->get_playlist_count());

				playlist_api->playlist_add_items(new_idx, data, bit_array_false());
				if (main_window::config_get_activate_target_playlist_on_dropped_items())
					playlist_api->set_active_playlist(new_idx);

			}
			else
			{
				playlist_api->playlist_clear_selection(target_index);
				playlist_api->playlist_insert_items(target_index, idx, data, bit_array_true());
				if (main_window::config_get_activate_target_playlist_on_dropped_items())
					playlist_api->set_active_playlist(target_index);
			}

			data.remove_all();
		}
	}
	m_DataObject.release();

	return S_OK;		
}
playlists_tabs_extension::playlists_tabs_drop_target::playlists_tabs_drop_target(playlists_tabs_extension * p_wnd) : drop_ref_count(0), p_list(p_wnd), m_last_rmb(false)
{
	last_over.x=0; last_over.y=0;
	m_DropTargetHelper.instantiate(CLSID_DragDropHelper, NULL, CLSCTX_INPROC);
}

void g_on_autohide_tabs_change()
{
	unsigned n, count = playlists_tabs_extension::list_wnd.get_count();
	for (n=0; n<count; n++)
	{
		playlists_tabs_extension::list_wnd[n]->create_tabs();
	}
}

void g_on_multiline_tabs_change()
{
	unsigned n, count = playlists_tabs_extension::list_wnd.get_count();
	for (n=0; n<count; n++)
	{
		playlists_tabs_extension * p_tabs = playlists_tabs_extension::list_wnd[n];
		p_tabs->set_styles();
		p_tabs->on_size();
	}
}

void g_on_tabs_font_change()
{
	playlists_tabs_extension::on_font_change();
}

class font_client_switcher_tabs : public cui::fonts::client
{
public:
	virtual const GUID & get_client_guid() const
	{
		return g_guid_playlist_switcher_tabs_font;
	}
	virtual void get_name (pfc::string_base & p_out) const
	{
		p_out = "Playlist Tabs";
	}

	virtual cui::fonts::font_type_t get_default_font_type() const
	{
		return cui::fonts::font_type_labels;
	}

	virtual void on_font_changed() const 
	{
		playlists_tabs_extension::on_font_change();
	}
};

font_client_switcher_tabs::factory<font_client_switcher_tabs> g_font_client_switcher_tabs;
