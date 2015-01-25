#include "foo_ui_columns.h"

class playlists_tabs_test2 : public uie::container_ui_extension_t<ui_helpers::container_window, uie::window>
{
public:
private:
	WNDPROC tabproc;

	bool m_dragging;
	unsigned m_dragging_idx;
	RECT m_dragging_rect;

	bool m_playlist_switched;
	bool m_switch_timer;
	unsigned m_switch_playlist;
	bool initialised;

	service_ptr_t<contextmenu_manager> p_manager;

	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data_ex(_T("{ABB72D0D-DBF0-4bba-8C68-3357EBE07A4Dxx}"), _T(""), false, 0, WS_CHILD|WS_CLIPCHILDREN, WS_EX_CONTROLPARENT, CS_DBLCLKS);
	}

public:
	static pfc::ptr_list_t<playlists_tabs_test2> list_wnd;

	HWND wnd_tabs;
	LRESULT WINAPI hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static LRESULT WINAPI main_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	playlists_tabs_test2();

	~playlists_tabs_test2();

	static const GUID extension_guid;

	virtual const GUID & get_extension_guid() const;

	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;
	virtual bool get_short_name(pfc::string_base & out)const;

	void set_styles(bool visible = true)
	{
		if (wnd_tabs)
		{
			long flags = WS_CHILD |  TCS_HOTTRACK | TCS_TABS | ((cfg_tabs_multiline && (0 && TabCtrl_GetItemCount(wnd_tabs) > 1)) ? TCS_MULTILINE|TCS_RIGHTJUSTIFY  : TCS_SINGLELINE) |(visible ? WS_VISIBLE : 0)|WS_CLIPSIBLINGS |WS_TABSTOP |0;

			if (uGetWindowLong(wnd_tabs, GWL_STYLE) != flags)
				uSetWindowLong(wnd_tabs, GWL_STYLE, flags);
		}
	}

	void reset_size_limits()
	{
		TRACK_CALL_TEXT("playlist_tabs::reset_size_limits");
		console::print(uGetCallStackPath());
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
		TRACK_CALL_TEXT("playlist_tabs::on_size");
		console::print(uGetCallStackPath());
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
				console::formatter() << "old top: " << old_top << " new top: " << rc.top;
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

	virtual void import_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort) 
	{
		if (p_size)
		{
			p_reader->read_lendian_t(m_child_guid, p_abort);
			unsigned size = 0;
			p_reader->read_lendian_t(size, p_abort);
			m_child_data.set_size(0);
			m_child_data.set_size(size);
			p_reader->read(m_child_data.get_ptr(), size, p_abort);
		}
	}
	virtual void export_config(stream_writer * p_writer, abort_callback & p_abort) const 
	{
		p_writer->write_lendian_t(m_child_guid, p_abort);
		uie::window_ptr ptr = m_child;
		if (!ptr.is_valid())
		{
			if (uie::window::create_by_guid(m_child_guid, ptr))
			{
				try {
				ptr->set_config(&stream_reader_memblock_ref(m_child_data.get_ptr(), m_child_data.get_size()),m_child_data.get_size(),abort_callback_impl());
				} catch (const exception_io &) {};
			}
			else throw pfc::exception("panel missing");
		}
		pfc::array_t<t_uint8> data;
		stream_writer_memblock_ref w(data);
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
	ui_extension::window_ptr m_child;
	HWND m_child_wnd;
	HWND m_host_wnd;

	unsigned m_child_top;

	MINMAXINFO mmi;
};

ui_extension::window_factory<playlists_tabs_test2> blah;

HFONT playlists_tabs_test2::g_font
 = 0;

void playlists_tabs_test2::on_font_change()
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

	g_font = CreateFontIndirect(&(LOGFONT)cfg_tab_font);

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



pfc::ptr_list_t<playlists_tabs_test2> playlists_tabs_test2::list_wnd;


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
	unsigned n, count = playlists_tabs_test2::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_test2::list_wnd[n]->on_playlist_activate(p_old,p_new);
}

void callback_manager_tabs::on_playlist_created(unsigned p_index,const char * p_name,unsigned p_name_len)
{
	unsigned n, count = playlists_tabs_test2::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_test2::list_wnd[n]->on_playlist_created(p_index,p_name,p_name_len);
}

void callback_manager_tabs::on_playlists_reorder(const unsigned * p_order,unsigned p_count)
{
	unsigned n, count = playlists_tabs_test2::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_test2::list_wnd[n]->on_playlists_reorder(p_order, p_count);
}

void callback_manager_tabs::on_playlists_removed(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count)
{
	unsigned n, count = playlists_tabs_test2::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_test2::list_wnd[n]->on_playlists_removed(p_mask, p_old_count, p_new_count);
}

void callback_manager_tabs::on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len)
{
	unsigned n, count = playlists_tabs_test2::list_wnd.get_count();
	for (n=0; n<count; n++)
		playlists_tabs_test2::list_wnd[n]->on_playlist_renamed(p_index,p_new_name,p_new_name_len);
}

static service_factory_single_t<playlist_callback,callback_manager_tabs> hgf;

#endif




playlists_tabs_test2::playlists_tabs_test2() : m_dragging(false), m_dragging_idx(0), m_playlist_switched(false), 
m_switch_timer(false), m_switch_playlist(0), initialised(false), wnd_tabs(0),
m_child_guid(/*cfg_default_playlist*/pfc::guid_null), m_child_wnd(0), m_host_wnd(0), m_child_top(0)
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

playlists_tabs_test2::~playlists_tabs_test2()
{
}

LRESULT WINAPI playlists_tabs_test2::main_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	playlists_tabs_test2 * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<playlists_tabs_test2*>(uGetWindowLong(wnd,GWL_USERDATA));

	rv = p_this ? p_this->hook(wnd,msg,wp,lp) : uDefWindowProc(wnd, msg, wp, lp);;

	return rv;
}

bool playlists_tabs_test2::create_tabs()
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
			WS_CHILD |  WS_TABSTOP | TCS_HOTTRACK | TCS_TABS | (t>1 && 0 ? TCS_MULTILINE : 0) | (1 ? WS_VISIBLE : 0), x, y, cx, cy,
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

			uTabCtrl_InsertItemText(wnd_tabs, 0, "Console");
			uTabCtrl_InsertItemText(wnd_tabs, 1, "Console");

			TabCtrl_SetCurSel(wnd_tabs, 1);
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

LRESULT playlists_tabs_test2::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
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

			create_tabs();
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
	case WM_SIZE:
		{
			on_size(LOWORD(lp), HIWORD(lp));
		}
		break;
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = LPMINMAXINFO(lp);
			*lpmmi = mmi;
		}
		return 0;
	case WM_DESTROY:
		{
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
	}
	return uDefWindowProc(wnd, msg, wp, lp);
}

const GUID & playlists_tabs_test2::get_extension_guid() const
{
	return extension_guid;
}

void playlists_tabs_test2::get_name(pfc::string_base & out)const
{
	out.set_string("Playlist tabs (TEST)");
}
void playlists_tabs_test2::get_category(pfc::string_base & out)const
{
	out.set_string("Splitters");
}
bool playlists_tabs_test2::get_short_name(pfc::string_base & out)const
{
	out.set_string("Playlist tabs");
	return true;
}

// {ABB72D0D-DBF0-4bba-8C68-3357EBE07A4D}
const GUID playlists_tabs_test2::extension_guid = 
{ 0xabb72d0d, 0xdbf0, 0x4bba, { 0x8c, 0x68, 0x33, 0x57, 0xeb, 0xe0, 0x7d, 0x4f } };




LRESULT WINAPI playlists_tabs_test2::hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		{
			if (get_host()->get_keyboard_shortcuts_enabled() && static_api_ptr_t<keyboard_shortcut_manager>()->on_keydown_auto(wp)) return 0;
			else if (wp == VK_TAB)
			{
				ui_extension::window::g_on_tab(wnd);
				return 0;
			}
		}
		break;
	case WM_SYSKEYDOWN:
		if (get_host()->get_keyboard_shortcuts_enabled() && static_api_ptr_t<keyboard_shortcut_manager>()->on_keydown_auto(wp)) return 0;
		break;
	}
	return uCallWindowProc(tabproc,wnd,msg,wp,lp);
}

