#include "foo_ui_columns.h"

#if 0

class visualization_ext;
class visualisation_host;

const GUID default_vis = 
{ 0xd947777c, 0x94c7, 0x409a, { 0xb0, 0x2c, 0x9b, 0xe, 0xb9, 0xe3, 0x74, 0xfa } };

cfg_guid cfg_vis_host_guid(create_guid(0x7a432cae,0x1a3c,0xc18d,0xfd,0x2d,0x6f,0xbf,0x36,0x9d,0x59,0xd6), default_vis);

class window_visualisation_host : public ui_extension::container_ui_extension
{
	static const char * class_name;
	bool initialised;
	GUID m_guid;
	pfc::array_t<t_uint8> m_data;
	pfc::refcounted_object_ptr_t<class vis_host_interface> m_interface;
	ui_extension::visualisation_ptr p_vis;
	unsigned m_frame;
	HBITMAP bm_display;
	RECT rc_client;

public:
	static pfc::ptr_list_t<window_visualisation_host> list_vis;

	LRESULT WINAPI on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	virtual class_data & get_class_data()const 
	{
		long flags = WS_EX_CONTROLPARENT;
		if (m_frame == 1) flags |= WS_EX_CLIENTEDGE;
		if (m_frame == 2) flags |= WS_EX_STATICEDGE;
		
		__implement_get_class_data_ex(class_name, false, 0, WS_CHILD|WS_CLIPCHILDREN, flags, 0);
	}

	window_visualisation_host();
//	HWND init(HWND parent, const void * data, unsigned size);

	~window_visualisation_host();

	static const GUID extension_guid;

	virtual const GUID & get_extension_guid() const
	{
		return extension_guid;
	}
	void flush_bitmap();
	void make_bitmap(HDC hdc=0);

	virtual bool have_config_popup(){return true;}

	virtual bool show_config_popup(HWND wnd_parent);

	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;
	virtual bool get_short_name(pfc::string_base & out)const
	{
		out = "Simple visualization"; return true;
	}

	virtual unsigned get_type  () const{return ui_extension::type_toolbar|ui_extension::type_panel;};

	virtual void set_config ( stream_reader * config);

	virtual void get_menu_items (ui_extension::menu_hook_t & p_hook);

	virtual void get_config( stream_writer * data);

	friend class vis_host_interface;

};

class vis_host_interface : public ui_extension::visualisation_host
{
//	vis_extension_ptr p_vis;
	pfc::refcounted_object_ptr_t<window_visualisation_host> p_wnd;
public:
	void begin_paint(vis_paint_struct & p_out)
	{
		HWND wnd = p_wnd->get_wnd();
		if(wnd) 
		{
			HDC dc_bmp = CreateCompatibleDC(0);
			if (!p_wnd->bm_display) p_wnd->make_bitmap();
			HGDIOBJ meh = SelectObject(dc_bmp,p_wnd->bm_display);

			p_out.dc = dc_bmp;
			p_out.rc_client = p_wnd->rc_client;
			p_out.reserved = (void*)meh;
		}
	}
	void end_paint(vis_paint_struct & p_in)
	{
		HWND wnd = p_wnd->get_wnd();
		HDC dc = GetDC(wnd);
		BitBlt(dc,0,0,p_wnd->rc_client.right,p_wnd->rc_client.bottom,p_in.dc,0,0,SRCCOPY);
		SelectObject(p_in.dc,(HGDIOBJ)p_in.reserved);
		DeleteDC(p_in.dc);
		ReleaseDC(wnd,dc);
	}
	static bool g_create(pfc::refcounted_object_ptr_t<vis_host_interface> & p_out, window_visualisation_host * wnd) ;

};

ui_extension::visualisation_host_factory<vis_host_interface> g_vis_host_interface;

bool vis_host_interface::g_create(pfc::refcounted_object_ptr_t<vis_host_interface> & p_out, window_visualisation_host * wnd) 
{
	bool rv = false;
	if ( g_vis_host_interface.instance_create((service_ptr_t<service_base>&)p_out))
	{
		rv = true;
		p_out->p_wnd = wnd;
	}
	return rv;
}

const char * window_visualisation_host::class_name = "{5E9B1A6A-6324-456b-AEFD-66D9091C1854}";

pfc::ptr_list_t<window_visualisation_host> window_visualisation_host::list_vis;


window_visualisation_host::window_visualisation_host() : initialised(false), m_guid(cfg_vis_host_guid), m_frame(cfg_vis_edge), bm_display(0)
{
	memset(&rc_client, 0, sizeof(RECT));
};

window_visualisation_host::~window_visualisation_host()
{
}

void window_visualisation_host::make_bitmap(HDC hdc)
{
	if (!bm_display)
	{
//		RECT rc_client;
//		GetClientRect(wnd_vis,&rc_client);

		HWND wnd_vis = get_wnd();
		HDC dc = hdc ? hdc : GetDC(wnd_vis);
		bm_display = CreateCompatibleBitmap(dc, rc_client.right, rc_client.bottom);
		HDC dcm = CreateCompatibleDC(0);
		HGDIOBJ meh = SelectObject(dcm,bm_display);
		p_vis->paint_background(dcm, &rc_client);
		SelectObject(dcm,meh);
		DeleteDC(dcm);

		if (!hdc) ReleaseDC(wnd_vis, dc);
	}
}

void window_visualisation_host::flush_bitmap()
{
	if (bm_display)
	{
		DeleteObject(bm_display);
		bm_display=0;
	}
}


LRESULT WINAPI window_visualisation_host::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	if(msg == WM_CREATE)
	{
		long flags = WS_EX_CONTROLPARENT;
		if (m_frame == 1) flags |= WS_EX_CLIENTEDGE;
		if (m_frame == 2) flags |= WS_EX_STATICEDGE;
			
		uSetWindowLong(wnd, GWL_EXSTYLE, flags);
		SetWindowPos(wnd,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

		list_vis.add_item(this);
		initialised = true;

		ui_extension::visualisation::create_by_guid(m_guid, p_vis);
		if (p_vis.is_valid())
		{
			GetClientRect(wnd, &rc_client);
			vis_host_interface::g_create(m_interface, this);
			p_vis->set_config(&stream_reader_memblock_ref(m_data, m_data.get_size()));
			p_vis->enable(ui_extension::visualisation_host_ptr(m_interface.get_ptr()));
		}
	}
	else if (msg == WM_DESTROY)
	{
		if (p_vis.is_valid())
			p_vis->disable();
		flush_bitmap();
		m_interface.release();
		initialised = false;
		list_vis.remove_item(this);
	}
	else if (msg == WM_PAINT)
	{
		if (p_vis.is_valid())
		{
			RECT rc_paint;
			if (!GetUpdateRect(wnd, &rc_paint, 0))
			{
				rc_paint = rc_client;
			}
			HDC dc = GetDC(wnd);
			HDC dc_bmp = CreateCompatibleDC(0);
			if (!bm_display) make_bitmap(dc);
			HGDIOBJ meh = SelectObject(dc_bmp,bm_display);
			BitBlt(dc,0,0,rc_client.right,rc_client.bottom,dc_bmp,0,0,SRCCOPY);
			SelectObject(dc_bmp,meh);
			DeleteDC(dc_bmp);
			ReleaseDC(wnd, dc);
			ValidateRect(wnd, &rc_paint);
		}
	}
	else if (msg == WM_SIZE)
	{
		GetClientRect(wnd, &rc_client);
		flush_bitmap();
	}
	else if (msg == WM_ERASEBKGND)
	{
		return 0;
	}

	return uDefWindowProc(wnd,msg,wp,lp);
}


void window_visualisation_host::get_name(pfc::string_base & out)const
{
	out.set_string("Simple visualization");
}
void window_visualisation_host::get_category(pfc::string_base & out)const
{
	out.set_string("Visualizations");
}

void window_visualisation_host::set_config(stream_reader * r)
{
	if (r->read_guid_le(m_guid))
	{
		if (r->read_dword_le(m_frame))
		{
//			console::info(pfc::string_printf("read: %u",m_frame));
			unsigned size = 0;
			if (r->read_dword_le(size))
			{
				if (m_data.set_size(size))
				r->read(m_data, size);
			}
		}
	}
}

void window_visualisation_host::get_menu_items (ui_extension::menu_hook_t & p_hook)
{
	ui_extension::menu_node_ptr p_node(new(std::nothrow) uie::menu_node_configure(this));
	p_hook.add_node(p_node);
	if (p_vis.is_valid()) p_vis->get_menu_items(p_hook);
}

void window_visualisation_host::get_config(stream_writer * data)
{
	data->write_guid_le(m_guid);
	if (p_vis.is_valid())
	{
		m_data.set_size(0);
		p_vis->get_config(&stream_writer_memblock_ref(m_data));
	}
//	console::info(pfc::string_printf("write: %u",m_frame));
	data->write_dword_le(m_frame);
	data->write_dword_le(m_data.get_size());
	data->write(m_data, m_data.get_size());
}

// {5E9B1A6A-6324-456b-AEFD-66D9091C1854}
const GUID window_visualisation_host::extension_guid = 
{ 0x5e9b1a6a, 0x6324, 0x456b, { 0xae, 0xfd, 0x66, 0xd9, 0x9, 0x1c, 0x18, 0x54 } };

ui_extension::window_factory<window_visualisation_host> blah;

//does this trigger wm_size??
void update_vis_host_frames()
{
	unsigned n, count = window_visualisation_host::list_vis.get_count();
	long flags = WS_EX_CONTROLPARENT;
	if (cfg_vis_edge == 1) flags |= WS_EX_CLIENTEDGE;
	if (cfg_vis_edge == 2) flags |= WS_EX_STATICEDGE;
	
	for (n=0; n<count; n++)
	{
		HWND wnd = window_visualisation_host::list_vis[n]->get_wnd();
		if (wnd)
		{
			uSetWindowLong(wnd, GWL_EXSTYLE, flags);
			SetWindowPos(wnd,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
		}
	}
}

class vis_info
{
public:
	pfc::string8 name;
	GUID guid;
};

class config_param
{
public:
	GUID m_guid;
	pfc::array_t<t_uint8> m_data;
	ui_extension::visualisation_ptr m_ptr;
	unsigned m_frame;
	ptr_list_autodel_t<vis_info> m_visinfo;
	bool b_change;
};

static BOOL CALLBACK ConfigPopupProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		uSetWindowLong(wnd,DWL_USER,lp);
		{
			config_param * ptr = reinterpret_cast<config_param*>(lp);

			uSendDlgItemMessageText(wnd,IDC_FRAME,CB_ADDSTRING,0,"None");
			uSendDlgItemMessageText(wnd,IDC_FRAME,CB_ADDSTRING,0,"Sunken");
			uSendDlgItemMessageText(wnd,IDC_FRAME,CB_ADDSTRING,0,"Grey");
			uSendDlgItemMessage(wnd,IDC_FRAME,CB_SETCURSEL,ptr->m_frame,0);

			unsigned n, count = ptr->m_visinfo.get_count();

			unsigned sel =-1;

			HWND wnd_combo = GetDlgItem(wnd, IDC_VIS_LIST);

			for (n=0; n<count; n++)
			{
				unsigned idx = uSendDlgItemMessageText(wnd,IDC_VIS_LIST,CB_ADDSTRING,0,ptr->m_visinfo[n]->name);
				uSendMessage(wnd_combo,CB_SETITEMDATA,idx,n);

				if (sel == -1 && ptr->m_visinfo[n]->guid == ptr->m_guid) sel = idx;
				else if (sel != -1 && idx <= sel) sel++;
			}

			if (sel != -1) uSendMessageText(wnd_combo,CB_SETCURSEL,sel,0);
			EnableWindow(GetDlgItem(wnd, IDC_CONFIGURE), ptr->m_ptr.is_valid() && ptr->m_ptr->have_config_popup());
		}
		return TRUE;
	case WM_COMMAND:
		switch(wp)
		{
		case IDCANCEL:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				EndDialog(wnd, ptr->b_change);
			}
			return TRUE;
		case (CBN_SELCHANGE<<16)|IDC_VIS_LIST:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				unsigned idx = uSendMessageText((HWND)lp,CB_GETCURSEL,0,0);
				if (idx != CB_ERR)
				{
					ptr->m_guid = ptr->m_visinfo[idx]->guid;
					ui_extension::visualisation::create_by_guid(ptr->m_guid, ptr->m_ptr);
					EnableWindow(GetDlgItem(wnd, IDC_CONFIGURE), ptr->m_ptr.is_valid() && ptr->m_ptr->have_config_popup());
				}
			}
			break;
		case IDC_CONFIGURE:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				if (ptr->m_ptr.is_valid())
				{
					ptr-> b_change = ptr->m_ptr->show_config_popup(wnd);
					if ( ptr->b_change)
					{	
						ptr->m_data.set_size(0);
						ptr->m_ptr->get_config(&stream_writer_memblock_ref(ptr->m_data));
					}
				}
			}
			break;
		case (CBN_SELCHANGE<<16)|IDC_FRAME:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				ptr->m_frame = uSendMessage((HWND)lp,CB_GETCURSEL,0,0);
			}
			break;
		case IDOK:
			{
				EndDialog(wnd,1);
			}
			return TRUE;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
}

bool window_visualisation_host::show_config_popup(HWND wnd_parent)
{
	config_param param;
	param.m_guid = m_guid;
	param.m_frame = m_frame;
	param. b_change = false;
	if (p_vis.is_valid())
	{
		param.m_ptr = p_vis;
		p_vis->get_config(&stream_writer_memblock_ref(param.m_data));
	}
	else
		ui_extension::visualisation::create_by_guid(m_guid, param.m_ptr);

	service_enum_t<ui_extension::visualisation> e;
	pfc::refcounted_object_ptr_t<ui_extension::visualisation> ptr;
	
	while(e.next(ptr))
	{
		vis_info * p_info = new(std::nothrow) vis_info;
		p_info->guid = ptr->get_extension_guid();
		ptr->get_name(p_info->name);
		param.m_visinfo.add_item(p_info);
	}

	bool rv = !!uDialogBox(IDD_POPUP_VISHOST,wnd_parent,ConfigPopupProc,reinterpret_cast<LPARAM>(&param));
	if (rv)
	{
		bool b_frame_changed = m_frame != param.m_frame;
		bool b_vis_changed = p_vis.get_ptr() != param.m_ptr.get_ptr();
		if (initialised)
		{
			if (p_vis.is_valid() && b_vis_changed) p_vis->disable();
		}
		m_guid = param.m_guid;
		cfg_vis_host_guid = m_guid;
		m_data = param.m_data;
		m_frame = param.m_frame;
		cfg_vis_edge = m_frame;
		if (initialised)
		{
			if (b_vis_changed) p_vis.release();
			if (b_frame_changed)
			{
			long flags = WS_EX_CONTROLPARENT;
			if (m_frame == 1) flags |= WS_EX_CLIENTEDGE;
			if (m_frame == 2) flags |= WS_EX_STATICEDGE;
			
			uSetWindowLong(get_wnd(), GWL_EXSTYLE, flags);
			SetWindowPos(get_wnd(),0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
			}
			if (b_vis_changed)
			{
				ui_extension::visualisation::create_by_guid(m_guid, p_vis);
				if (p_vis.is_valid())
				{
					p_vis->set_config(&stream_reader_memblock_ref(m_data, m_data.get_size()));
					vis_host_interface::g_create(m_interface, this);
					p_vis->enable(ui_extension::visualisation_host_ptr(m_interface.get_ptr()));
				}
			}
		}
	}
	return rv;
}
#endif