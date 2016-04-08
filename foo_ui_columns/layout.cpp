#include "stdafx.h"


// {755971A7-109B-41dc-BED9-5A05CC07C905}
static const GUID g_guid_layout = 
{ 0x755971a7, 0x109b, 0x41dc, { 0xbe, 0xd9, 0x5a, 0x5, 0xcc, 0x7, 0xc9, 0x5 } };

cfg_layout_t cfg_layout(g_guid_layout);


class window_host_layout : public ui_extension::window_host
{
public:

	virtual const GUID & get_host_guid()const
	{
		// {DA9A1375-A411-48a9-AF74-4AC29FF9BE9C}
		static const GUID ret = 
		{ 0xda9a1375, 0xa411, 0x48a9, { 0xaf, 0x74, 0x4a, 0xc2, 0x9f, 0xf9, 0xbe, 0x9c } };
		return ret;
	}

	virtual void on_size_limit_change(HWND wnd, unsigned flags)
	{
	};

	
	virtual bool override_status_text_create(service_ptr_t<ui_status_text_override> & p_out)
	{
		static_api_ptr_t<ui_control> api;
		return api->override_status_text_create(p_out);
	}

	virtual unsigned is_resize_supported(HWND wnd)const
	{
		return false;
	}

	virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height)
	{
		bool rv = false;
		return rv;
	}
	virtual bool is_visible(HWND wnd) const
	{
		bool rv = IsWindowVisible(g_layout_window.get_wnd()) != 0;
		return  rv;
	}
	virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility)const
	{
		bool rv = false;
		return  rv;
	}
	virtual bool set_window_visibility(HWND wnd, bool visibility)
	{
		bool rv = false;
		return rv;
	}

	virtual void relinquish_ownership(HWND wnd)
	{
		g_layout_window.relinquish_child();
	}
	
};

ui_extension::window_host_factory_single<window_host_layout> g_window_host_layout_factory;

layout_window::layout_window() : m_child_guid(columns_ui::panels::guid_playlist_view_v2), m_layout_editing_active(false), m_child_wnd(nullptr) {};
bool layout_window::set_focus()
{
	return __set_focus_recur(m_child);
}


void layout_window::show_window()
{
	ShowWindow(m_child_wnd, SW_SHOWNORMAL);
	ShowWindow(get_wnd(), SW_SHOWNORMAL);
}

bool layout_window::__set_focus_recur(const uie::window_ptr & p_wnd)
{
	service_ptr_t<uie::playlist_window> p_playlist_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		if (p_wnd->service_query_t(p_playlist_wnd))// && (GetWindowLongPtr(p_playlist_wnd->get_wnd(), GWL_STYLE) & WS_VISIBLE)) //we cheat: IsWindowVisible checks parent/main win visibility as well
		{ 
			p_playlist_wnd->set_focus(); 
			return true; 
		}
		else if (p_wnd->service_query_t(p_splitter_wnd))
		{
			unsigned n, count;
			count = p_splitter_wnd->get_panel_count();
			for (n=0; n<count; n++)
			{
				uie::splitter_item_ptr temp;
				p_splitter_wnd->get_panel(n, temp);
				if (temp.is_valid() && __set_focus_recur(temp->get_window_ptr()))
					return true;
			}
		}
	}
	return false;
}

void layout_window::show_menu_access_keys()
{
	return __show_menu_access_keys_recur(m_child);
}


void layout_window::__show_menu_access_keys_recur(const uie::window_ptr & p_wnd)
{
	service_ptr_t<uie::menu_window> p_menu_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		if (p_wnd->service_query_t(p_menu_wnd))
		{ 
			p_menu_wnd->show_accelerators();
		}
		else if (p_wnd->service_query_t(p_splitter_wnd))
		{
			unsigned n, count;
			count = p_splitter_wnd->get_panel_count();
			for (n=0; n<count; n++)
			{
				uie::splitter_item_ptr temp;
				p_splitter_wnd->get_panel(n, temp);
				if (temp.is_valid())
					__show_menu_access_keys_recur(temp->get_window_ptr());
			}
		}
	}
}

void layout_window::hide_menu_access_keys()
{
	__hide_menu_access_keys_recur(m_child);
}

bool layout_window::on_menu_char (unsigned short c)
{
	return __on_menu_char_recur(m_child, c);
}

bool layout_window::__on_menu_char_recur (const uie::window_ptr & p_wnd, unsigned short c)
{
	service_ptr_t<uie::menu_window> p_menu_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		if (p_wnd->service_query_t(p_menu_wnd))
		{ 
			if (p_menu_wnd->on_menuchar(c))
			return true; 
		}
		else if (p_wnd->service_query_t(p_splitter_wnd))
		{
			unsigned n, count;
			count = p_splitter_wnd->get_panel_count();
			for (n=0; n<count; n++)
			{
				uie::splitter_item_ptr temp;
				p_splitter_wnd->get_panel(n, temp);
				if (temp.is_valid() && __on_menu_char_recur(temp->get_window_ptr(), c))
					return true;
			}
		}
	}
	return false;
}

bool layout_window::set_menu_focus ()
{
	return __set_menu_focus_recur(m_child);
}

bool layout_window::__set_menu_focus_recur (const uie::window_ptr & p_wnd)
{
	bool ret = false;
	service_ptr_t<uie::menu_window> p_menu_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		if (p_wnd->service_query_t(p_menu_wnd))
		{ 
			if (!ret)
			{
				p_menu_wnd->set_focus();
				ret = true; 
			}
			else
			{
				p_menu_wnd->hide_accelerators();
			}
		}
		else if (p_wnd->service_query_t(p_splitter_wnd))
		{
			unsigned n, count;
			count = p_splitter_wnd->get_panel_count();
			for (n=0; n<count; n++)
			{
				uie::splitter_item_ptr temp;
				p_splitter_wnd->get_panel(n, temp);
				if (temp.is_valid())
				{
					if (!ret) ret = __set_menu_focus_recur(temp->get_window_ptr());
					else __hide_menu_access_keys_recur(temp->get_window_ptr());

				}
			}
		}
	}
	return ret;
}

void layout_window::set_layout_editing_active(bool b_val)
{
	if (b_val)
	{
		if (!m_layout_editing_active)
			enter_layout_editing_mode();
		m_layout_editing_active = true;
	}
	else
	{
		if (m_layout_editing_active)
			exit_layout_editing_mode();
		m_layout_editing_active = false;
	}
}
bool layout_window::get_layout_editing_active()
{
	return m_layout_editing_active;
}


void layout_window::enter_layout_editing_mode()
{
	if (get_wnd())
	{
		message_hook_manager::register_hook(message_hook_manager::type_get_message, this);
		message_hook_manager::register_hook(message_hook_manager::type_mouse, this);
	}
	//__enter_layout_editing_mode_recur(m_child);
}

void layout_window::exit_layout_editing_mode()
{
	message_hook_manager::deregister_hook(message_hook_manager::type_get_message, this);
	message_hook_manager::deregister_hook(message_hook_manager::type_mouse, this);
	//__exit_layout_editing_mode_recur(m_child);
}

bool layout_window::is_menu_focused()
{
	return __is_menu_focused_recur(m_child);
}

bool layout_window::__is_menu_focused_recur (const uie::window_ptr & p_wnd)
{
	service_ptr_t<uie::menu_window> p_menu_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		if (p_wnd->service_query_t(p_menu_wnd))
		{ 
			if (p_menu_wnd->is_menu_focused()) return true;
		}
		else if (p_wnd->service_query_t(p_splitter_wnd))
		{
			unsigned n, count;
			count = p_splitter_wnd->get_panel_count();
			for (n=0; n<count; n++)
			{
				uie::splitter_item_ptr temp;
				p_splitter_wnd->get_panel(n, temp);
				if (temp.is_valid())
				{
					if (__is_menu_focused_recur(temp->get_window_ptr()))
						return true;

				}
			}
		}
	}
	return false;
}

HWND layout_window::get_previous_menu_focus_window() const
{
	HWND ret = nullptr;
	__get_previous_menu_focus_window_recur(m_child, ret);
	return ret;
}

bool layout_window::__get_previous_menu_focus_window_recur(const uie::window_ptr & p_wnd, HWND & wnd_previous) const
{
	service_ptr_t<uie::menu_window_v2> p_menu_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		if (p_wnd->service_query_t(p_menu_wnd))
		{ 
			if (p_menu_wnd->is_menu_focused() ) {
				wnd_previous = p_menu_wnd->get_previous_focus_window();
				return true;
			}
		}
		else if (p_wnd->service_query_t(p_splitter_wnd))
		{
			unsigned n, count;
			count = p_splitter_wnd->get_panel_count();
			for (n=0; n<count; n++)
			{
				uie::splitter_item_ptr temp;
				p_splitter_wnd->get_panel(n, temp);
				if (temp.is_valid())
				{
					if (__get_previous_menu_focus_window_recur(temp->get_window_ptr(), wnd_previous))
						return true;

				}
			}
		}
	}
	return false;
}

void layout_window::__hide_menu_access_keys_recur(const uie::window_ptr & p_wnd)
{
	service_ptr_t<uie::menu_window> p_menu_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		if (p_wnd->service_query_t(p_menu_wnd))
		{ 
			p_menu_wnd->hide_accelerators();
		}
		else if (p_wnd->service_query_t(p_splitter_wnd))
		{
			unsigned n, count;
			count = p_splitter_wnd->get_panel_count();
			for (n=0; n<count; n++)
			{
				uie::splitter_item_ptr temp;
				p_splitter_wnd->get_panel(n, temp);
				if (temp.is_valid())
					__hide_menu_access_keys_recur(temp->get_window_ptr());
			}
		}
	}
}

void layout_window::get_child (uie::splitter_item_ptr & p_out)
{
	p_out = new uie::splitter_item_simple_t;
	p_out->set_panel_guid(m_child_guid);
	if (m_child.is_valid())
	{
		stream_writer_memblock conf;
		abort_callback_dummy p_abort;
		m_child->get_config(&conf, p_abort);
		p_out->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());
	}
	else
		p_out->set_panel_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size());
}
void layout_window::set_child (const uie::splitter_item_t * item)
{
	if (get_wnd())
	{
		SendMessage(get_wnd(), WM_SETREDRAW, FALSE, 0);
		destroy_child();
	}
	m_child_guid = item->get_panel_guid();
	item->get_panel_config_to_array(m_child_data, true);
	if (get_wnd())
	{
		create_child();
		show_window();
		SendMessage(get_wnd(), WM_SETREDRAW, TRUE, 0);
		RedrawWindow(get_wnd(), 0, 0, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN|RDW_FRAME|RDW_ERASE);
	}
}

void __get_panel_list_recur(const uie::window_ptr & p_wnd, pfc::list_base_t<GUID> & p_out)
{
	service_ptr_t<uie::menu_window> p_menu_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		p_out.add_item(p_wnd->get_extension_guid());
		if (p_wnd->service_query_t(p_splitter_wnd))
		{
			unsigned n, count;
			count = p_splitter_wnd->get_panel_count();
			for (n=0; n<count; n++)
			{
				uie::splitter_item_ptr temp;
				p_splitter_wnd->get_panel(n, temp);
				if (temp.is_valid())
				{
					p_out.add_item(temp->get_panel_guid());
					uie::window_ptr ptr = temp->get_window_ptr();
					if (!ptr.is_valid())
						if (uie::window::create_by_guid(temp->get_panel_guid(), ptr))
						{
							stream_writer_memblock w;
							temp->get_panel_config(&w);
							try {
								abort_callback_dummy p_abort;
								ptr->set_config_from_ptr(w.m_data.get_ptr(), w.m_data.get_size(), p_abort);
							} catch (const exception_io &) {};
						}
						else throw cui::fcl::exception_missing_panel();
					__get_panel_list_recur(ptr, p_out);
				}
			}
		}
	}
}

bool layout_window::import_config_to_object(stream_reader * p_reader, t_size psize, t_uint32 mode, cfg_layout_t::preset & p_out, pfc::list_base_t<GUID> & panels, abort_callback & p_abort)
{
	//uie::splitter_item_ptr item = new uie::splitter_item_simple_t;
	GUID guid;
	pfc::string8 name;
	p_reader->read_lendian_t(guid, p_abort);
	p_reader->read_string(name, p_abort);

	pfc::array_t<t_uint8> data, conf;
	t_uint32 size;
	p_reader->read_lendian_t(size, p_abort);
	data.set_size(size);
	p_reader->read(data.get_ptr(), size, p_abort);

	panels.add_item(guid);
	
	p_out.m_guid = guid;
	p_out.m_name = name;

	if (mode == cui::fcl::type_public)
	{
		uie::window_ptr wnd;
		if (uie::window::create_by_guid(guid, wnd))
		{
			try {
				wnd->import_config_from_ptr(data.get_ptr(), data.get_size(), p_abort);
			} catch (const exception_io &) {};
			wnd->get_config_to_array(conf, p_abort);
			__get_panel_list_recur(wnd, panels);
		}
		else return false;

		p_out.m_val = conf;
	}
	else
	{
		p_out.m_val = data;
		uie::window_ptr wnd;
		if (uie::window::create_by_guid(guid, wnd))
		{
			try {
			wnd->set_config_from_ptr(data.get_ptr(), data.get_size(), p_abort);
			} catch (const exception_io &) {};
			__get_panel_list_recur(wnd, panels);
		}
		else return false;
	}
	return true;

	//item->set_panel_guid(guid);
	//item->set_panel_config(&stream_reader_memblock_ref(conf.get_ptr(), conf.get_size()), conf.get_size());
	//p_out.set(item);
}

void layout_window::export_config(stream_writer * p_out, t_uint32 mode, pfc::list_base_t<GUID> & panels, abort_callback & p_abort)
{
	enum {stream_version = 0};
	p_out->write_lendian_t((t_uint32)stream_version, p_abort);
	t_size i, count = cfg_layout.get_presets().get_count();
	p_out->write_lendian_t(cfg_layout.get_active(), p_abort);
	p_out->write_lendian_t(count, p_abort);
	for (i=0; i<count; i++)
	{
		uie::splitter_item_ptr item;
		cfg_layout.get_preset(i, item);
		pfc::string8 name;
		cfg_layout.get_preset_name(i, name);
		try 
		{
			if (mode == cui::fcl::type_public)
			{
				p_out->write_lendian_t(item->get_panel_guid(), p_abort);
				p_out->write_string(name, p_abort);
				uie::window_ptr ptr;
				if (!uie::window::create_by_guid(item->get_panel_guid(), ptr))
					throw cui::fcl::exception_missing_panel();
				panels.add_item(item->get_panel_guid());
				{
					stream_writer_memblock writer;
					stream_writer_memblock data;
					item->get_panel_config(&data);
					try {
						ptr->set_config_from_ptr(data.m_data.get_ptr(), data.m_data.get_size(), p_abort);
					} catch (const exception_io &) {};
					__get_panel_list_recur(ptr, panels);
					ptr->export_config(&writer, p_abort);
					p_out->write_lendian_t((t_uint32)writer.m_data.get_size(), p_abort);
					p_out->write(writer.m_data.get_ptr(), (t_uint32)writer.m_data.get_size(), p_abort);
				}
			}
			else
			{
				p_out->write_lendian_t(item->get_panel_guid(), p_abort);
				p_out->write_string(name, p_abort);
				panels.add_item(item->get_panel_guid());
				stream_writer_memblock writer;
				item->get_panel_config(&writer);

				uie::window_ptr ptr;
				if (!uie::window::create_by_guid(item->get_panel_guid(), ptr))
					throw cui::fcl::exception_missing_panel();
				{
					try {
					ptr->set_config_from_ptr(writer.m_data.get_ptr(), writer.m_data.get_size(), p_abort);
					} catch (const exception_io &) {};
					__get_panel_list_recur(ptr, panels);
				}

				p_out->write_lendian_t((t_uint32)writer.m_data.get_size(), p_abort);
				p_out->write(writer.m_data.get_ptr(), (t_uint32)writer.m_data.get_size(), p_abort);
			}
		}
		catch (const pfc::exception & ex)
		{
			pfc::string_formatter formatter;
			throw pfc::exception(formatter << "Error exporting layout preset \"" << name << "\" - " << ex.what());
		}
	}
}

void layout_window::create_child()
{
	RECT rc;
	GetClientRect(get_wnd(), &rc);
	
	if (uie::window::create_by_guid(m_child_guid, m_child))
	{
		try {
			abort_callback_dummy p_abort;
			m_child->set_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size(), p_abort);
		} catch (const exception_io & ex)
		{
			console::formatter formatter;
			formatter << "Error setting panel config: " << ex.what();
		}
		if (m_child_wnd = m_child->create_or_transfer_window(get_wnd(), uie::window_host_ptr(&g_window_host_layout_factory.get_static_instance()), ui_helpers::window_position_t(rc)))
		{
			SetWindowLongPtr(m_child_wnd, GWL_STYLE, GetWindowLongPtr(m_child_wnd, GWL_STYLE)|WS_CLIPSIBLINGS);
		}
	}
}
void layout_window::destroy_child()
{
	if (m_child_wnd && m_child.is_valid())
	{
		abort_callback_dummy p_abort;
		m_child->get_config_to_array(m_child_data, p_abort, true);
		m_child->destroy_window();
		m_child.release();
	}
}

void layout_window::relinquish_child()
{
	m_child_wnd = NULL;
	m_child.release();
	m_child_data.set_size(0);
}


void layout_window::refresh_child ()
{
	uie::splitter_item_ptr item;
	cfg_layout.get_active_preset_for_use(item);
	if (item.is_valid())
	{
		m_child_guid = item->get_panel_guid();
		item->get_panel_config_to_array(m_child_data, true);
	}
	else
	{
		m_child_guid = columns_ui::panels::guid_playlist_view_v2;
		m_child_data.set_size(0);
	}
}
void layout_window::run_live_edit_base_delayed(POINT pt)
{
	PostMessage(get_wnd(), MSG_EDIT_PANEL, (int)pt.x, (int)pt.y);
}

void layout_window::run_live_edit_base_delayed_v2(HWND wnd, POINT pt, pfc::list_t<uie::window::ptr> & p_hierarchy)
{
	m_live_edit_data.m_hierarchy = p_hierarchy;
	m_live_edit_data.m_wnd = wnd;
	m_live_edit_data.m_point = pt;
	PostMessage(get_wnd(), MSG_EDIT_PANEL_V2, NULL, NULL);
}

///don't pass smartptrs by reference as they may be nuked when destroying stuff
void layout_window::run_live_edit_base(POINT pt_menu)
{
	if (!m_trans_fill.get_wnd())
	{
		RECT rc;
		GetRelativeRect(m_child_wnd, HWND_DESKTOP, &rc);
		HWND wnd_over = m_trans_fill.create(get_wnd(), 0, ui_helpers::window_position_t(rc));
		WindowEnum_t WindowEnum(GetAncestor(get_wnd(), GA_ROOT));
		WindowEnum.run();
		t_size count_owned = WindowEnum.m_wnd_list.get_count();
		if (count_owned)
			SetWindowPos(wnd_over, WindowEnum.m_wnd_list[count_owned-1], 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
		ShowWindow(wnd_over, SW_SHOWNOACTIVATE);

		HMENU menu = CreatePopupMenu();
		HMENU menu_change = CreatePopupMenu();
		uie::window_info_list_simple panels;
		g_get_panel_list(panels, uie::window_host_ptr(&g_window_host_layout_factory.get_static_instance()));
		enum {ID_CLOSE= 1, ID_CHANGE_BASE = 2};

		uie::splitter_window_ptr p_splitter;
		if (m_child.is_valid())
			m_child->service_query_t(p_splitter);

		g_append_menu_panels(menu_change, panels, ID_CHANGE_BASE);
		pfc::string8 temp;
		if (m_child.is_valid())
			m_child->get_name(temp);
		uAppendMenu(menu, MF_STRING|MF_GRAYED, (UINT_PTR)0, temp);
		uAppendMenu(menu, MF_MENUBREAK, (UINT_PTR)0, NULL);

		const UINT_PTR ID_ADD_BASE = ID_CHANGE_BASE + panels.get_count();
		const UINT_PTR ID_CHANGE_SPLITTER_BASE = ID_ADD_BASE + panels.get_count();
		if (p_splitter.is_valid())
		{
			if (p_splitter->get_panel_count() < p_splitter->get_maximum_panel_count())
			{
				HMENU menu_add = CreatePopupMenu();
				g_append_menu_panels(menu_add, panels, ID_ADD_BASE);
				AppendMenu(menu, MF_STRING|MF_POPUP, (UINT_PTR)menu_add, L"Add panel");
			}
			HMENU menu_change = CreatePopupMenu();
			g_append_menu_splitters(menu_change, panels, ID_CHANGE_SPLITTER_BASE);
			AppendMenu(menu, MF_STRING|MF_POPUP, (UINT_PTR)menu_change, L"Change splitter");
		}

		AppendMenu(menu, MF_STRING|MF_POPUP, (UINT_PTR)menu_change, L"Change panel");

		int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt_menu.x,pt_menu.y,0,get_wnd(),0);
		m_trans_fill.destroy();
		{
			{
				if (cmd)
				{
					if (cmd >=ID_CHANGE_BASE && cmd < panels.get_count()+ID_CHANGE_BASE)
					{
						t_size panel_index = cmd - ID_CHANGE_BASE;
						uie::splitter_item_ptr si = new uie::splitter_item_simple_t;
						si->set_panel_guid(panels[panel_index].guid);
						set_child(si.get_ptr());
					}
					else if (cmd >=ID_ADD_BASE && cmd < panels.get_count()+ID_ADD_BASE)
					{
						t_size panel_index = cmd - ID_ADD_BASE;
						uie::splitter_item_ptr si = new uie::splitter_item_simple_t;
						si->set_panel_guid(panels[panel_index].guid);
						p_splitter->add_panel(si.get_ptr());
					}
					else if (cmd >=ID_CHANGE_SPLITTER_BASE && cmd < panels.get_count()+ID_CHANGE_SPLITTER_BASE)
					{
						t_size panel_index = cmd - ID_CHANGE_SPLITTER_BASE;

						uie::window_ptr window;
						service_ptr_t<uie::splitter_window> splitter;
						if (uie::window::create_by_guid(panels[panel_index].guid, window) && window->service_query_t(splitter))
						{
							unsigned n, count = min (p_splitter->get_panel_count(), splitter->get_maximum_panel_count());
							if (count == p_splitter->get_panel_count() || MessageBox(get_wnd(), _T("The number of child items will not fit in the selected splitter type. Continue?"), _T("Warning"), MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
							{
								for (n=0; n<count; n++)
								{
									uie::splitter_item_ptr ptr;
									p_splitter->get_panel(n, ptr);
									splitter->add_panel(ptr.get_ptr());
								}
								uie::splitter_item_ptr newsi;
								get_child(newsi);

								stream_writer_memblock conf;
								try {
									abort_callback_dummy p_abort;
									splitter->get_config(&conf, p_abort);
								}
								catch (const pfc::exception &) {};
								newsi->set_panel_guid(panels[panel_index].guid);
								newsi->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());

								set_child(newsi.get_ptr());
							}
						}

					}
				}
			}
		}
		DestroyMenu(menu);
	}
}

class panel_list_t : public pfc::list_t<uie::window::ptr>
{
public:
	panel_list_t()
	{
		service_enum_t<ui_extension::window> e;
		uie::window_ptr l;

		while (e.next(l))
		{
			add_item(l);
		}
	};
};

void g_get_panels_info(const pfc::list_t<uie::window::ptr> & p_panels, uie::window_info_list_simple & p_out)
{
	t_size i, count = p_panels.get_count();

	for (i=0;i<count; i++)
	{
		uie::window_info_simple info;
		uie::window::ptr l = p_panels[i];

			l->get_name(info.name);
			l->get_category(info.category);
			info.guid = l->get_extension_guid();
			info.prefer_multiple_instances = l->get_prefer_multiple_instances();
			info.type = l->get_type();
			p_out.add_item(info);
	}

	p_out.sort();
}

void layout_window::run_live_edit_base_v2(const live_edit_data_t & p_data)
{
	if (!m_trans_fill.get_wnd())
	{
		t_size hierarchy_count = p_data.m_hierarchy.get_count();
		if (hierarchy_count ==0)
			throw pfc::exception_bug_check();

		uie::window::ptr p_window = p_data.m_hierarchy[hierarchy_count-1];
		uie::splitter_window_ptr p_container, p_splitter;
		uie::splitter_window_v2_ptr p_container_v2;

		if (p_window.is_valid())
			p_window->service_query_t(p_splitter);
		if (hierarchy_count >= 2)
			p_data.m_hierarchy[hierarchy_count-2]->service_query_t(p_container);
		if (p_container.is_valid())
			p_container->service_query_t(p_container_v2);

		RECT rc;
		GetRelativeRect(p_window->get_wnd(), HWND_DESKTOP, &rc);
		HWND wnd_over = m_trans_fill.create(get_wnd(), 0, ui_helpers::window_position_t(rc));
		WindowEnum_t WindowEnum(GetAncestor(get_wnd(), GA_ROOT));
		WindowEnum.run();
		t_size count_owned = WindowEnum.m_wnd_list.get_count();
		if (count_owned)
			SetWindowPos(wnd_over, WindowEnum.m_wnd_list[count_owned-1], 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
		ShowWindow(wnd_over, SW_SHOWNOACTIVATE);

		HMENU menu = CreatePopupMenu();
		panel_list_t panel_list;
		pfc::list_t<uie::window::ptr> supported_panels(panel_list);
		bit_array_bittable mask_remove(supported_panels.get_count());
		if (p_container_v2.is_valid())
			p_container_v2->get_supported_panels(supported_panels, mask_remove);
		supported_panels.remove_mask(mask_remove);

		uie::window_info_list_simple panels;
		g_get_panels_info(supported_panels, panels);
		enum {ID_CLOSE= 1, ID_SHOW_CAPTION, ID_LOCKED, ID_CHANGE_BASE};


		pfc::string8 temp;
		p_window->get_name(temp);
		uAppendMenu(menu, MF_STRING|MF_GRAYED, (UINT_PTR)0, temp);
		//uAppendMenu(menu, MF_MENUBREAK, (UINT_PTR)0, NULL);

		const UINT_PTR ID_PARENT_ADD_BASE = ID_CHANGE_BASE + panels.get_count();
		const UINT_PTR ID_CHANGE_BASE_SPLITTER_BASE = ID_PARENT_ADD_BASE + panels.get_count();

		const UINT_PTR ID_CHANGE_SPLITTER_BASE = ID_CHANGE_BASE_SPLITTER_BASE + panels.get_count();
		const UINT_PTR ID_ADD_BASE = ID_CHANGE_SPLITTER_BASE + panels.get_count();

		if (hierarchy_count==1)
		{
			{
				HMENU menu_change = CreatePopupMenu();
				g_append_menu_panels(menu_change, panels, ID_CHANGE_BASE);
				AppendMenu(menu, MF_STRING|MF_POPUP, (UINT_PTR)menu_change, L"Change panel");
			}
			if (p_splitter.is_valid())
			{
				HMENU menu_change = CreatePopupMenu();
				g_append_menu_splitters(menu_change, panels, ID_CHANGE_BASE_SPLITTER_BASE);
				AppendMenu(menu, MF_STRING|MF_POPUP, (UINT_PTR)menu_change, L"Change splitter");
			}
		}

		{
			if (p_splitter.is_valid())
			{
				if (p_splitter->get_panel_count() < p_splitter->get_maximum_panel_count())
				{
					HMENU menu_add = CreatePopupMenu();
					g_append_menu_panels(menu_add, panels, ID_ADD_BASE);
					AppendMenu(menu, MF_STRING|MF_POPUP, (UINT_PTR)menu_add, L"Add panel");
				}
				if (p_container.is_valid())
				{
					HMENU menu_change = CreatePopupMenu();
					g_append_menu_splitters(menu_change, panels, ID_CHANGE_SPLITTER_BASE);
					AppendMenu(menu, MF_STRING|MF_POPUP, (UINT_PTR)menu_change, L"Change splitter");
				}
			}
		}
		t_size index = pfc_infinite;

		if (p_container.is_valid())
		{
			
			if (p_container->find_by_ptr(p_data.m_hierarchy[hierarchy_count-1], index))
			{
				if (p_container->get_config_item_supported(index, uie::splitter_window::bool_show_caption))
				{
					bool b_val;
					p_container->get_config_item(index, uie::splitter_window::bool_show_caption, b_val);
					AppendMenu(menu, MF_STRING|(b_val ? MF_CHECKED : NULL), ID_SHOW_CAPTION, L"Show caption");
				}
				if (p_container->get_config_item_supported(index, uie::splitter_window::bool_locked))
				{
					bool b_val;
					p_container->get_config_item(index, uie::splitter_window::bool_locked, b_val);
					AppendMenu(menu, MF_STRING|(b_val ? MF_CHECKED : NULL), ID_LOCKED, L"Locked");
				}
				AppendMenu(menu, MF_STRING, ID_CLOSE, L"Close");
			}
			uAppendMenu(menu, MF_MENUBREAK, (UINT_PTR)0, NULL);
			p_container->get_name(temp);
			uAppendMenu(menu, MF_STRING|MF_GRAYED, (UINT_PTR)0, temp);
			if (p_container->get_panel_count() < p_container->get_maximum_panel_count())
			{
				HMENU menu_add = CreatePopupMenu();
				g_append_menu_panels(menu_add, panels, ID_PARENT_ADD_BASE);
				AppendMenu(menu, MF_STRING|MF_POPUP, (UINT_PTR)menu_add, L"Add panel");
			}
		}


		int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,p_data.m_point.x,p_data.m_point.y,0,get_wnd(),0);
		m_trans_fill.destroy();
		{
			{
				if (cmd)
				{
					abort_callback_dummy p_abort;
					if (cmd == ID_CLOSE)
					{
						p_container->remove_panel(p_window);
					}
					else if (cmd == ID_SHOW_CAPTION)
					{
						bool b_old;
						p_container->get_config_item(index, uie::splitter_window::bool_show_caption, b_old);
						p_container->set_config_item_t(index, uie::splitter_window::bool_show_caption, !b_old, p_abort);
					}
					else if (cmd == ID_LOCKED)
					{
						bool b_old;
						p_container->get_config_item(index, uie::splitter_window::bool_locked, b_old);
						p_container->set_config_item_t(index, uie::splitter_window::bool_locked, !b_old, p_abort);
					}
					else if (cmd >=ID_CHANGE_BASE && cmd < panels.get_count()+ID_CHANGE_BASE)
					{
						t_size panel_index = cmd - ID_CHANGE_BASE;
						uie::splitter_item_ptr si = new uie::splitter_item_simple_t;
						si->set_panel_guid(panels[panel_index].guid);
						set_child(si.get_ptr());
					}
					else if (cmd >=ID_PARENT_ADD_BASE && cmd < panels.get_count()+ID_PARENT_ADD_BASE)
					{
						t_size panel_index = cmd - ID_PARENT_ADD_BASE;
						uie::splitter_item_ptr si = new uie::splitter_item_simple_t;
						si->set_panel_guid(panels[panel_index].guid);
						p_container->add_panel(si.get_ptr());
					}
					else if (cmd >=ID_CHANGE_BASE_SPLITTER_BASE && cmd < panels.get_count()+ID_CHANGE_BASE_SPLITTER_BASE)
					{
						t_size panel_index = cmd - ID_CHANGE_BASE_SPLITTER_BASE;

						uie::window_ptr window;
						service_ptr_t<uie::splitter_window> splitter;
						if (uie::window::create_by_guid(panels[panel_index].guid, window) && window->service_query_t(splitter))
						{
							unsigned n, count = min (p_splitter->get_panel_count(), splitter->get_maximum_panel_count());
							if (count == p_splitter->get_panel_count() || MessageBox(get_wnd(), _T("The number of child items will not fit in the selected splitter type. Continue?"), _T("Warning"), MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
							{
								for (n=0; n<count; n++)
								{
									uie::splitter_item_ptr ptr;
									p_splitter->get_panel(n, ptr);
									splitter->add_panel(ptr.get_ptr());
								}
								uie::splitter_item_ptr newsi;
								get_child(newsi);

								stream_writer_memblock conf;
								try {splitter->get_config(&conf, p_abort);}
								catch (const pfc::exception &) {};
								newsi->set_panel_guid(panels[panel_index].guid);
								newsi->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());

								set_child(newsi.get_ptr());
							}
						}

					}
					else if (cmd >=ID_ADD_BASE && cmd < panels.get_count()+ID_ADD_BASE)
					{
						t_size panel_index = cmd - ID_ADD_BASE;
						uie::splitter_item_ptr si = new uie::splitter_item_simple_t;
						si->set_panel_guid(panels[panel_index].guid);
						p_splitter->add_panel(si.get_ptr());
					}
					else if (cmd >=ID_CHANGE_SPLITTER_BASE && cmd < panels.get_count()+ID_CHANGE_SPLITTER_BASE)
					{
						t_size panel_index = cmd - ID_CHANGE_SPLITTER_BASE;

						uie::window_ptr window;
						service_ptr_t<uie::splitter_window> splitter;
						if (uie::window::create_by_guid(panels[panel_index].guid, window) && window->service_query_t(splitter))
						{
							unsigned n, count = min (p_splitter->get_panel_count(), splitter->get_maximum_panel_count());
							if (index != pfc_infinite && ( count == p_splitter->get_panel_count() || MessageBox(p_data.m_wnd, _T("The number of child items will not fit in the selected splitter type. Continue?"), _T("Warning"), MB_YESNO|MB_ICONEXCLAMATION) == IDYES))
							{
								for (n=0; n<count; n++)
								{
									uie::splitter_item_ptr ptr;
									p_splitter->get_panel(n, ptr);
									splitter->add_panel(ptr.get_ptr());
								}
								uie::splitter_item_ptr newsi;
								p_container->get_panel(index, newsi);

								stream_writer_memblock conf;
								try {splitter->get_config(&conf, p_abort);}
								catch (const pfc::exception &) {};
								newsi->set_panel_guid(panels[panel_index].guid);
								newsi->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());

								p_container->replace_panel(index, newsi.get_ptr());
							}
						}

					}
				}
			}
		}
		DestroyMenu(menu);
	}
}

bool layout_window::on_hooked_message(message_hook_manager::t_message_hook_type p_type, int code, WPARAM wp, LPARAM lp)
{
	if (p_type == message_hook_manager::type_get_message)
	{
		MSG * lpmsg = (LPMSG)lp;
		if (lpmsg->message == WM_CONTEXTMENU)
		{
			if (lpmsg->hwnd == get_wnd() || IsChild(get_wnd(), lpmsg->hwnd))
			{
				uie::splitter_window_v2_ptr sw2;
				if (m_child.is_valid())
				{
					m_child->service_query_t(sw2);

					RECT rc;
					GetRelativeRect(lpmsg->hwnd, HWND_DESKTOP, &rc);

					POINT pt = {rc.left + RECT_CX(rc)/2,rc.top + RECT_CY(rc)/2};

					pfc::list_t<uie::window::ptr> hierarchy;
					if (!sw2.is_valid() || sw2->is_point_ours(lpmsg->hwnd, pt, hierarchy))
					{
						if (!sw2.is_valid())
							hierarchy.add_item(m_child);
						HWND wnd_panel = NULL;
						if (hierarchy.get_count())
						{
							wnd_panel = hierarchy[hierarchy.get_count()-1]->get_wnd();

							GetRelativeRect(wnd_panel, HWND_DESKTOP, &rc);
							POINT pt = {rc.left + RECT_CX(rc)/2,rc.top + RECT_CY(rc)/2};

							run_live_edit_base_delayed_v2(wnd_panel, pt, hierarchy);
						}

						lpmsg->message = WM_NULL;
						lpmsg->lParam = NULL;
						lpmsg->wParam = NULL;
						lpmsg->hwnd = NULL;
					}
				}
			}
		}
		return false;
	}
	else
		if (p_type == message_hook_manager::type_mouse)
		{
			MOUSEHOOKSTRUCT * lpmhs = (LPMOUSEHOOKSTRUCT)lp;
			if (lpmhs->hwnd == get_wnd() || IsChild(get_wnd(), lpmhs->hwnd))
			{
				uie::splitter_window_v2_ptr sw2;
				if (m_child.is_valid())
				{
					m_child->service_query_t(sw2);
					if (wp == WM_RBUTTONDOWN || wp == WM_RBUTTONUP)
					{
						pfc::list_t<uie::window::ptr> hierarchy;
						if (!sw2.is_valid() || sw2->is_point_ours(lpmhs->hwnd, lpmhs->pt, hierarchy))
						{
							if (wp == WM_RBUTTONUP)
							{
								if (!sw2.is_valid())
									hierarchy.add_item(m_child);
								if (!m_trans_fill.get_wnd())
								{
									POINT pt = lpmhs->pt;
									run_live_edit_base_delayed_v2(lpmhs->hwnd, pt, hierarchy);
								}
							}
							else if (wp == WM_RBUTTONDOWN)
								SendMessage(lpmhs->hwnd, WM_CANCELMODE, NULL, NULL);
							return true;
						}
					}
				}
			}
			return false;
		}
	return false;
}

LRESULT layout_window::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		refresh_child();
		create_child();
		break;
	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
			if (!(lpwp->flags & SWP_NOSIZE))
			{
				if (m_child_wnd)
					SetWindowPos(m_child_wnd, NULL, 0, 0, lpwp->cx, lpwp->cy, SWP_NOZORDER);
			}
		}
		break;
	case WM_SIZE:
		break;
	case WM_SETFOCUS:
		PostMessage(wnd, MSG_LAYOUT_SET_FOCUS, 0, 0);
		break;
	case MSG_EDIT_PANEL:
		{
			POINT pt = {int(wp), int(lp)};
			run_live_edit_base(pt);
		}
		break;
	case MSG_EDIT_PANEL_V2:
		{
			run_live_edit_base_v2(m_live_edit_data);
			m_live_edit_data.reset();
		}
		break;
	case MSG_LAYOUT_SET_FOCUS:
		set_focus();
		break;
#if 0
	case WM_CONTEXTMENU:
		{
			if (m_layout_editing_active)
			{
				POINT pt = {GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
				if (pt.x == -1 && pt.y == -1)
				{
					RECT rc;
					GetRelativeRect(m_child_wnd, HWND_DESKTOP, &rc);

					pt.x = rc.left + RECT_CX(rc)/2;
					pt.y = rc.top + RECT_CY(rc)/2;
				}
				run_live_edit_base(pt);
			}
		}
		return 0;
#endif
	case WM_DESTROY:
		destroy_child();
		message_hook_manager::deregister_hook(message_hook_manager::type_get_message, this);
		message_hook_manager::deregister_hook(message_hook_manager::type_mouse, this);
		break;
	}
	return DefWindowProc(wnd, msg, wp, lp);
}
