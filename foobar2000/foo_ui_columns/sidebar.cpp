#include "foo_ui_columns.h"
#if 0

__forceinline int GetCaptionHeight()
{
	HFONT font = uCreateMenuFont();
	int rv = uGetFontHeight(font);
	DeleteFont(font);
	rv+=9;
	return rv;
}
	


void extension_info::write(stream_writer * out, abort_callback & p_abort)
{
		out->write_lendian_t(guid, p_abort);
		out->write_lendian_t(height, p_abort);
		out->write_lendian_t(locked, p_abort);
		out->write_lendian_t(show_caption, p_abort);
		out->write_lendian_t(hidden, p_abort);
		DWORD size = config.get_size();
		out->write_lendian_t(size, p_abort);
		out->write(config.get_ptr(), size, p_abort);
}

void extension_info::read(stream_reader * r, abort_callback & p_abort)
{
		r->read_lendian_t(guid, p_abort);
		r->read_lendian_t(height, p_abort);
		r->read_lendian_t(locked, p_abort);
		r->read_lendian_t(show_caption, p_abort);
		r->read_lendian_t(hidden, p_abort);

		unsigned mem_size;
		r->read_lendian_t(mem_size, p_abort);

		if (mem_size)
		{
			config.set_size(mem_size);
			r->read(config.get_ptr(), mem_size, p_abort);
		}

}
void extension_info::copy(extension_info & out)
{
	out.guid = guid;
	out.height = height;
	out.locked = locked;
	out.show_caption = show_caption;
	out.hidden = hidden;
	if (wnd_panel && p_ext.is_valid())
	{
		try {
		config.set_size(0);
		p_ext->get_config(&stream_writer_memblock_ref(config), abort_callback_impl());
		}
		catch (pfc::exception e)
		{}
	}
	out.config.set_size(0);
	out.config.append_fromptr(config.get_ptr(), config.get_size());
}


void sidebar_info::set_sidebar_info(sidebar_info & in)
{
	delete_all();
	unsigned n, count = in.get_count();
	for (n=0; n<count; n++)
	{
		extension_info * item = new(std::nothrow) extension_info;
		if (item)
		{
			in[n]->copy(*item);
			add_item(item);
		}
	}
}
bool sidebar_info::move_up(unsigned idx)
{
	unsigned count = get_count();
	if (idx > 0 && idx< count)
	{
		order_helper order(count);
		order.swap(idx, idx-1);
		reorder(order.get_ptr());
		return true;
	}
	return false;
}
bool sidebar_info::move_down(unsigned idx)
{
	unsigned count = get_count();
	if (idx >= 0 && idx < (count-1))
	{
		order_helper order(count);
		order.swap(idx, idx+1);
		reorder(order.get_ptr());
		return true;
	}
	return false;
}
extension_info * sidebar_info::find_by_wnd(HWND wnd)
{
	unsigned count = get_count(),n;
	extension_info * rv = 0;
	for (n=0; n<count;n++)
	{
		if (wnd == get_item(n)->wnd_panel) {rv = get_item(n); break;}
	}
	return rv;
}

void cfg_sidebar::get_data_raw(stream_writer * out, abort_callback & p_abort)
{
	extern sidebar_window * g_sidebar_window;
	if (g_sidebar_window)
	{
		entries.set_sidebar_info(g_sidebar_window->panels);
	}
		unsigned n;
		unsigned num = entries.get_count();
		out->write_lendian_t(num, p_abort);
		for(n=0;n<num;n++)
		{
			entries.get_item(n)->write(out, p_abort);
		}
}

void cfg_sidebar::set_data_raw(stream_reader * r, unsigned p_sizehint, abort_callback & p_abort)
{
		entries.delete_all();

		unsigned num;
		r->read_lendian_t(num, p_abort);
		for(;num;num--)
		{
			extension_info * item = new(std::nothrow) extension_info;
			try{ item->read(r, p_abort);}
			catch (pfc::exception e)
			{
				delete item; 
				item = 0;
				throw e;
			}
			entries.add_item(item);
		}
}

void cfg_sidebar::get_sidebar_info(sidebar_info & out)
{
	out.delete_all();
	unsigned n, count = entries.get_count();
	for (n=0; n<count; n++)
	{
		extension_info * item = new(std::nothrow) extension_info;
		if (item)
		{
			entries[n]->copy(*item);
			out.add_item(item);
		}
	}

}
void cfg_sidebar::set_sidebar_info(sidebar_info & in)
{
	entries.delete_all();
	unsigned n, count = in.get_count();
	for (n=0; n<count; n++)
	{
		extension_info * item = new(std::nothrow) extension_info;
		if (item)
		{
			in[n]->copy(*item);
			entries.add_item(item);
		}
	}
}

void cfg_sidebar::reset()
{
	entries.delete_all();
	{
		//playlist switcher
		GUID blah = { 0xc2cf9425, 0x540, 0x4579, { 0xab, 0x3f, 0x13, 0xe2, 0x17, 0x66, 0x3d, 0x9b } };
		extension_info * item = new(std::nothrow) extension_info(blah);
		entries.add_item(item);
	}
}

extern sidebar_window * g_sidebar_window;

// {81502417-26FB-4c2d-93AF-6C85CA0BD08E}
static const GUID sidebar_guid = 
{ 0x81502417, 0x26fb, 0x4c2d, { 0x93, 0xaf, 0x6c, 0x85, 0xca, 0xb, 0xd0, 0x8e } };


class ui_ext_host_sidebar : public ui_extension::window_host
{
public:

	virtual void get_name(pfc::string_base & out)const
	{
		out.set_string("Columns UI/Sidebar");
	};

	virtual bool is_available()const
	{
		return g_sidebar_window != 0;
	}

	virtual unsigned get_supported_types()const
	{
		return ui_extension::type_toolbar|ui_extension::type_panel|ui_extension::type_layout;
	}

	virtual void insert_extension (const GUID & in, unsigned height, unsigned width)
	{
		if (g_sidebar_window)
		{
			extension_info * item = new(std::nothrow) extension_info(in, height);
			if (item)
			{
				unsigned idx = g_sidebar_window->panels.add_item(item);
				g_sidebar_window->refresh_panels();
				g_sidebar_window->size_panels(true);
				g_sidebar_window->override_size(idx, height-g_sidebar_window->panels[idx]->height);
			}
		}
	};

	virtual void insert_extension (ui_extension::window_ptr & p_ext, unsigned height, unsigned width)
	{
		if (g_sidebar_window)
		{
			extension_info * item = new(std::nothrow) extension_info(p_ext->get_extension_guid(), height);
			if (item)
			{
				item->p_ext = p_ext;
				unsigned idx = g_sidebar_window->panels.add_item(item);
				g_sidebar_window->refresh_panels();
				g_sidebar_window->size_panels(true);
				g_sidebar_window->override_size(idx, height-g_sidebar_window->panels[idx]->height);
			}
		}
	};

	virtual const GUID & get_host_guid()const{return sidebar_guid;}

	virtual void on_size_limit_change(HWND wnd, unsigned flags)
	{
		if (g_sidebar_window)
		{
			extension_info * p_ext = g_sidebar_window->panels.find_by_wnd(wnd);
			if (p_ext)
			{
				MINMAXINFO mmi;
				memset(&mmi, 0, sizeof(MINMAXINFO));
				mmi.ptMaxTrackSize.x = MAXLONG;
				mmi.ptMaxTrackSize.y = MAXLONG;
				uSendMessage(wnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
				p_ext->min_width = mmi.ptMinTrackSize.x;
				p_ext->min_height = mmi.ptMinTrackSize.y;
				p_ext->max_height = mmi.ptMaxTrackSize.y;
				p_ext->max_width = mmi.ptMaxTrackSize.x;
				g_sidebar_window->size_panels();
			}
		}
	};

	
	virtual bool override_status_text_create(service_ptr_t<ui_status_text_override> & p_out)
	{
		static_api_ptr_t<ui_control> api;
		return api->override_status_text_create(p_out);
	}

	virtual unsigned is_resize_supported(HWND wnd)const
	{
		return ui_extension::size_height;
	}

	virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height)
	{
		bool rv = false;
		if (g_sidebar_window && !(flags & ui_extension::size_width))	
		{
			if (flags & ui_extension::size_height)
			{
				unsigned idx = 0;
				if (g_sidebar_window->find_panel_by_panel(wnd, idx))
				{
					g_sidebar_window->override_size(idx, height-g_sidebar_window->panels[idx]->height);
					rv = true;
				}
			}
			else rv = true;
		}
		return rv;
	}
	virtual bool is_visible(HWND wnd) const
	{
		bool rv = false;
		extern bool g_sidebar_autohidden;

		if (cfg_scar_hidden || (cfg_autohide && g_sidebar_autohidden))
		{
			rv = false;
		}
		else 
		{
			unsigned idx = 0;
			if (g_sidebar_window->find_panel_by_panel(wnd, idx))
			{
				rv = !g_sidebar_window->panels[idx]->hidden;
			}
		}
		return  rv;
	}
	virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility)const
	{
		bool rv = false;
		extern bool g_sidebar_autohidden;

		if (!(cfg_autohide && g_sidebar_autohidden) || desired_visibility == false)
		{
			if (cfg_scar_hidden && desired_visibility == true)
			{
				rv = true;
			}
			else
			{
				unsigned idx = 0;
				if (g_sidebar_window->find_panel_by_panel(wnd, idx))
				{
					rv = g_sidebar_window->panels[idx]->show_caption;
				}
			}
		}
		return  rv;
	}
	virtual bool set_window_visibility(HWND wnd, bool visibility)
	{
		bool rv = false;
		extern bool g_sidebar_autohidden;
		if (is_visibility_modifiable(wnd, visibility))
		{
			unsigned idx = 0;
			if (g_sidebar_window->find_panel_by_panel(wnd, idx))
			{
				if (cfg_scar_hidden && visibility == true && !(cfg_autohide && g_sidebar_autohidden))
				{
					cfg_scar_hidden = FALSE;
					size_windows();
				}
				g_sidebar_window->panels[idx]->hidden = !visibility;
				g_sidebar_window->size_panels();
				rv = true;
			}
		}
		return rv;
	}

	virtual void relinquish_ownership(HWND wnd)
	{
		if (g_sidebar_window)
		{
			unsigned count = g_sidebar_window->panels.get_count(),n;
			
			for (n=0; n<count;n++)
			{
				extension_info * p_ext = g_sidebar_window->panels.get_item(n);

				if (wnd == p_ext->wnd_panel) 
				{
					if (GetAncestor(wnd, GA_PARENT) == p_ext->wnd_host)
					{
						console::warning("window left by ui extension");
						SetParent(wnd, core_api::get_main_window());
					}
					
					DestroyWindow(p_ext->wnd_host);
					p_ext->wnd_host=0;
					p_ext->p_ext.release();
					g_sidebar_window->panels.delete_by_idx(n);
					g_sidebar_window->size_panels();
					break;
				}
			}
		}
	}
	
};

ui_extension::window_host_factory_single<ui_ext_host_sidebar> g_ui_ext_host_sidebar;

sidebar_window::sidebar_window() : wnd_sidebar(0),menu_ext_base(0) {};
sidebar_window::~sidebar_window() {};

const TCHAR * sidebar_window::class_name = _T("foo_ui_columns_sidebar_class");
const TCHAR * sidebar_window::panel_host_class_name = _T("foo_ui_columns_sidebar_panel_host");

bool sidebar_window::class_registered =false;
bool sidebar_window::panel_host_class_registered =false;

HWND sidebar_window::init(sidebar_info & new_panels, HWND parent)
{
	HWND rv = 0;
	
	panels.set_sidebar_info(new_panels);
	
	if (!class_registered)
	{
		WNDCLASS  wc;
		memset(&wc,0,sizeof(WNDCLASS));
		wc.style          = CS_DBLCLKS;
		wc.lpfnWndProc    = (WNDPROC)sidebar_proc;
		wc.hInstance      = core_api::get_my_instance();
		wc.hCursor        = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		wc.hbrBackground  = (HBRUSH)(0 ? COLOR_MENU+1 : COLOR_3DFACE+1);
		wc.lpszClassName  = class_name;
		
		if (RegisterClass(&wc)) class_registered = true;

		wc.style          = CS_DBLCLKS;
		wc.lpszClassName  = panel_host_class_name;
		wc.lpfnWndProc    = (WNDPROC)panel_proc;
		wc.cbWndExtra = 4;
		if (RegisterClass(&wc)) panel_host_class_registered = true;
	}
	
	if (!wnd_sidebar) 
	{
		rv = wnd_sidebar = CreateWindowEx(WS_EX_CONTROLPARENT, class_name, _T("Sidebar"),
			/*(WS_VISIBLE) */  WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, 0,200, 600,
			parent, 0, core_api::get_my_instance(), this);
	}
	
	return rv;
}

void sidebar_window::destroy()
{
	if (wnd_sidebar)
	{
		DestroyWindow(wnd_sidebar);
		wnd_sidebar =0;
	}
	unregister_class();
	//	delete this;
	//	g_sidebar_window = 0;
}

void sidebar_window::unregister_class()
{
	if (class_registered)
	{
		if (UnregisterClass(class_name, core_api::get_my_instance()))
			class_registered = false;
	}

	if (panel_host_class_registered)
	{
		if (UnregisterClass(panel_host_class_name, core_api::get_my_instance()))
			panel_host_class_registered = false;
	}
}

struct size_info
{
	unsigned height;
	bool sized;
	unsigned parts;
};

void sidebar_window::size_panels(bool set)
{
	HWND wnd = wnd_sidebar;
	RECT rc_client;
	GetClientRect(wnd, &rc_client);
	
	unsigned width = rc_client.right - rc_client.left;
	unsigned total_height = rc_client.bottom - rc_client.top+2;

	unsigned n, count = panels.get_count(),height_culm=0;


	if (count)
	{
		pfc::array_t<size_info> sizes;
		sizes.set_size(count);

		//sizes.fill(0);
		memset(sizes.get_ptr(), 0, sizes.get_size()*sizeof(size_info));

		unsigned the_caption_height = GetCaptionHeight();

		unsigned available_height = rc_client.bottom - rc_client.top+2;
		unsigned available_parts=0;

		for (n=0; n<count;n++)
		{
			unsigned height = panels[n]->hidden ? 0 : panels[n]->height;
			available_height -= height+2;
			sizes[n].height = height+2;
			sizes[n].parts = (panels[n]->locked || panels[n]->hidden) ? 0 : 1;
			available_parts+=sizes[n].parts;
			//if (!sizes[n].parts) 
			//	sizes[n].sized = true;
		}

//		console::error(uStringPrintf("%i %i",available_parts,available_height));

		bool first_pass(true);

		while (first_pass || (available_parts && available_height))
		{
			first_pass=false;

			unsigned parts=available_parts,available_height2=available_height;

			for (n=0; n<count;n++)
			{
				if (!sizes[n].sized)
				{
					unsigned height = sizes[n].height;
					int adjustment = 0;
					{
						adjustment = parts ? MulDiv(available_height2,sizes[n].parts,parts) : 0;
						parts-=sizes[n].parts;
						available_height2-=adjustment;
					}
					//+2
					if ((adjustment < 0 && (height > 2 ? height-2 : 0) < (unsigned)(adjustment*-1)))
					{
						adjustment = (height > 2 ? height-2 : 0) * -1;
						sizes[n].sized = true;
					}
					unsigned unadjusted = height;

					bool hidden = panels[n]->hidden;

					height += adjustment;
					unsigned min_height = hidden ? 0 :  panels[n]->min_height;
					unsigned caption_height = panels[n]->show_caption ? the_caption_height : 0;
					if (min_height < (unsigned)(-3-caption_height)) min_height += 2+caption_height;

					unsigned max_height = hidden ? 0 : panels[n]->max_height;
					if (max_height < (unsigned)(-3-caption_height)) max_height += 2+caption_height;

					if (height < min_height)
					{
						height = min_height;
						adjustment = (height-unadjusted);
						sizes[n].sized = true;
					}
					else if (height > max_height)
					{
						height = max_height;
						adjustment = (height-unadjusted);
						sizes[n].sized = true;
					}
					if (panels[n]->locked || hidden) sizes[n].sized = true;

	//				if (n==2) console::error(uStringPrintf("max %I, min %i",max_height,min_height));

					if (sizes[n].sized) available_parts-=sizes[n].parts;

					available_height-=(height-unadjusted);
					sizes[n].height = height;


				}
			}
		}


		if (set)
		{
			for (n=0; n<count;n++)
			{
				if (!panels[n]->hidden) panels[n]->height = sizes[n].height - 2;
			}
		}
		else
		{
			HDWP dwp = BeginDeferWindowPos((int)count);
			if (dwp)
			{
				for (n=0; n<count;n++)
				{
					if (panels[n]->p_ext.is_valid() && panels[n]->wnd_host)
					{
						unsigned height = sizes[n].height;
						dwp = DeferWindowPos(dwp, panels[n]->wnd_host, 0, 0, height_culm, width, height-2, SWP_NOZORDER/*|SWP_NOREDRAW*/);
						height_culm += height ;
					}
				}
				EndDeferWindowPos(dwp);
/*				for (n=0; n<count;n++)
				{
					if (panels[n]->p_ext && panels[n]->wnd_host) RedrawWindow(panels[n]->wnd_host, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
				}*/
			}
		}
	}
}
/*
		HDWP dwp = BeginDeferWindowPos((int)count);
		if (dwp)
		{
			unsigned total_panel_height=0;
			for (n=0; n<count;n++)
			{
				total_height -= panels[n]->height;
		//		console::info(pfc::string_printf("%i",total_height));
			}
			for (n=0; n<count;n++)
			{
				if (panels[n]->p_ext && panels[n]->wnd)
				{
					unsigned height = panels[n]->height;
					int adjustment = MulDiv(total_height,panels[n]->locked ? 0 : 1,count-n);

		//			console::info(pfc::string_printf("%i %i ",height,adjustment));
					if (adjustment < 0 && height < (unsigned)(adjustment*-1)) adjustment = height * -1;
		//			console::info(pfc::string_printf("%i %i",height,adjustment));
					total_height-=adjustment;
					height += adjustment;

		//			console::info(pfc::string_printf("%i %i",height,height_culm));
					dwp = DeferWindowPos(dwp, panels[n]->wnd, 0, 0, height_culm, width, height-2, SWP_NOZORDER);
					height_culm += height ;
				}
			}
			EndDeferWindowPos(dwp);
		}
*/



bool sidebar_window::check_panel(const GUID & id)
{
	bool rv = false;
	unsigned n, count = panels.get_count();
	for (n=0; n<count; n++)
	{
		if (panels[n]->guid == id)
		{
			rv = true;
			break;
		}
	}
	return rv;
}

bool sidebar_window::find_panel(const GUID & id, unsigned & out)
{
	bool rv = false;
	unsigned n, count = panels.get_count();
	for (n=0; n<count; n++)
	{
		if (panels[n]->guid == id)
		{
			out = n;
			rv = true;
			break;
		}
	}
	return rv;
}

bool sidebar_window::find_panel_by_host(const HWND wnd, unsigned & out)
{
	bool rv = false;
	unsigned n, count = panels.get_count();
	for (n=0; n<count; n++)
	{
		if (panels[n]->wnd_host == wnd)
		{
			out = n;
			rv = true;
			break;
		}
	}
	return rv;
}

bool sidebar_window::find_panel_by_panel(const HWND wnd, unsigned & out)
{
	bool rv = false;
	unsigned n, count = panels.get_count();
	for (n=0; n<count; n++)
	{
		if (panels[n]->wnd_panel == wnd)
		{
			out = n;
			rv = true;
			break;
		}
	}
	return rv;
}



bool sidebar_window::delete_panel(const GUID & id)
{
	unsigned n=0;
	bool rv = find_panel(id, n);
	if (rv) remove_panel(n);
	return rv;
}


struct min_max_info
{
	unsigned min_height;
	unsigned max_height;
	unsigned height;
};

int sidebar_window::override_size(unsigned & panel, int delta)
{

	unsigned count = panels.get_count();
	if (count)
	{
		size_panels(true);
		if (panel + 1 < count)
		{
			unsigned n=0;

			unsigned the_caption_height = GetCaptionHeight();
			pfc::array_t<min_max_info> minmax;
			minmax.set_size(count);

			//minmax.fill(0);
			memset(minmax.get_ptr(), 0, minmax.get_size()*sizeof(min_max_info));

			for (n=0; n<count; n++)
			{
				unsigned caption_height = panels[n]->show_caption ? the_caption_height : 0;
				unsigned min_height = panels[n]->hidden ? 0 : panels[n]->min_height;
				unsigned max_height = panels[n]->hidden ? 0 : panels[n]->max_height;
				
				if (min_height < (unsigned)(0-caption_height)) min_height += caption_height;
				if (max_height < (unsigned)(0-caption_height)) max_height += caption_height;

				minmax[n].min_height = min_height;
				minmax[n].max_height = max_height;
				minmax[n].height = panels[n]->hidden ? caption_height : panels[n]->height;
			}

//			console::info(pfc::string_printf("%u %i %i",panel,new_height,panels[panel]->height));

			bool is_up = delta < 0;//new_height < panels[panel]->height;
			bool is_down = delta > 0;//new_height > panels[panel]->height;

		
			if (is_up /*&& !panels[panel]->locked*/)
			{
/*				{
					unsigned height = panels[panel+1]->height + (panels[panel]->height - new_height);

					unsigned min_height = panels[panel+1]->min_height;
					unsigned max_height = panels[panel+1]->max_height;
					
					if (height < min_height)
					{
						height = min_height;
					}
					else if (height > max_height)
					{
						height = max_height;
					}

					new_height = height - panels[panel+1]->height - panels[panel]->height;

				}*/
	/*			unsigned caption_heighta = panels[panel+1]->show_caption ? the_caption_height : 0;

				unsigned new_height = panels[panel+1]->height - delta;
				
				unsigned min_heighta = panels[panel+1]->min_height;
				if (min_heighta < (unsigned)(0-caption_heighta)) min_heighta += caption_heighta;
				
				unsigned max_heighta = panels[panel+1]->max_height;
				if (max_heighta < (unsigned)(0-caption_heighta)) max_heighta += caption_heighta;
				
				if (new_height < min_heighta)
				{
					new_height = min_heighta;
				}
				else if (new_height > max_heighta) new_height = max_heighta;*/

				unsigned diff_abs = 0, diff_avail = abs(delta);

				unsigned n = panel+1;
				while (n < count && diff_abs < diff_avail)
				{
					{
						unsigned height = minmax[n].height+(diff_avail-diff_abs);//(diff_avail-diff_abs > panels[n]->height ? 0 : panels[n]->height-(diff_avail-diff_abs));
						
						unsigned min_height = minmax[n].min_height;
						unsigned max_height = minmax[n].max_height;
						
						if (height < min_height)
						{
							height = min_height;
						}
						else if (height > max_height)
						{
							height = max_height;
						}
						
						diff_abs += height - minmax[n].height;
					}
					n++;
				}
//				console::info(pfc::string_printf("av %u tot %u",diff_abs,diff_avail));

				n = panel+1;
				unsigned obtained =0;
				while (n>0 && obtained < diff_abs)
				{
					n--;
					//					if (!panels[n]->locked)
					{
						unsigned height = (diff_abs-obtained > minmax[n].height ? 0 : minmax[n].height-(diff_abs-obtained));
						
						unsigned caption_height = panels[n]->show_caption ? the_caption_height : 0;
						
						unsigned min_height = minmax[n].min_height;
						unsigned max_height = minmax[n].max_height;
						
						
						if (height < min_height)
						{
						/*							if (!ctrl_down)
						{
						panels.move_up(panel+1);
						//							if (panel) panel--;
						return 0;
						}
							else*/
							height = min_height;
						}
						else if (height > max_height)
						{
							height = max_height;
						}
						
						obtained += minmax[n].height - height;
						minmax[n].height = height;
						if (!panels[n]->hidden) panels[n]->height = height;
						
					}
				}
				n=panel;
				unsigned obtained2 = obtained;
//				panels[panel+1]->height += obtained;

				while (n < count-1 && obtained2 )
				{
//				console::info(pfc::string_printf("opbtained %u",obtained));
					n++;
					unsigned height = (minmax[n].height);

					unsigned min_height = minmax[n].min_height;
					unsigned max_height = minmax[n].max_height;
					
					height += obtained2;
					
					if (height < min_height)
					{
						height = min_height;
					}
					else if (height > max_height)
					{
						height = max_height;
					}
					
					obtained2 -= height - minmax[n].height;
					minmax[n].height = height;
					if (!panels[n]->hidden) panels[n]->height = height;
				}
				return (abs(delta)-obtained);
				
				
			}
			else if (is_down /*&& !panels[panel]->locked*/)
			{
			/*	unsigned new_height = panels[panel]->height + delta;
				unsigned caption_heighta = panels[panel]->show_caption ? the_caption_height : 0;


				unsigned min_heighta = minmax[panel].min_height;
				
				unsigned max_heighta = minmax[panel].max_height;
				

				if (new_height < panels[panel]->min_height) new_height = panels[panel]->min_height;
				else if (new_height > panels[panel]->max_height) new_height = panels[panel]->max_height;*/

//				int difference = ;
//				unsigned diff_abs = abs(panels[panel]->height - new_height);
				unsigned diff_abs = 0, diff_avail = abs(delta);

				n = panel+1;
				while (n >0 && diff_abs < diff_avail)
				{
					n--;
					{
						unsigned height = minmax[n].height+(diff_avail-diff_abs);//(diff_avail-diff_abs > panels[n]->height ? 0 : panels[n]->height-(diff_avail-diff_abs));
			//			console::info(pfc::string_printf("n: %u, h %u",n,height));
						
						unsigned min_height = minmax[n].min_height;
						unsigned max_height = minmax[n].max_height;
						
						if (height < min_height)
						{
							height = min_height;
						}
						else if (height > max_height)
						{
							height = max_height;
						}

			//			console::info(pfc::string_printf("n: %u, h %u %u",n,panels[n]->height));
						
						diff_abs += height - minmax[n].height;
			//			console::info(pfc::string_printf("n: %u, h %u",n,height));
					}
				}
//				console::info(pfc::string_printf("down av %u tot %u",diff_abs,diff_avail));
				n = panel;
				unsigned obtained =0;
				while (n < count-1 && obtained < diff_abs)
				{
					n++;
	//				if (!panels[n]->locked)
					{
						unsigned height = (diff_abs-obtained > minmax[n].height ? 0 : minmax[n].height-(diff_abs-obtained));
						
						unsigned caption_height = panels[n]->show_caption ? the_caption_height : 0;
						unsigned min_height = minmax[n].min_height;
						unsigned max_height = minmax[n].max_height;

						
						if (height < min_height)
						{
							height = min_height;
						}
						else if (height > max_height)
						{
							height = max_height;
						}
						
						obtained += minmax[n].height - height;
						minmax[n].height = height;
						if (!panels[n]->hidden) panels[n]->height = height;
						
					}
//					if (n+1 < count) n++;
//					else break;
				}
//				panels[panel]->height += obtained;
				n=panel+1;
				unsigned obtained2 = obtained;
				while (n >0 && obtained2)
				{
					n--;
					unsigned height = (minmax[n].height);
					unsigned min_height = minmax[n].min_height;
					unsigned max_height = minmax[n].max_height;
					
					height += obtained2;
					
					if (height < min_height)
					{
						height = min_height;
					}
					else if (height > max_height)
					{
						height = max_height;
					}
					
					obtained2 -= height - minmax[n].height;

					minmax[n].height = height;
					
					if (!panels[n]->hidden) panels[n]->height = height;
				}
				return 0-(abs(delta)-obtained);
				
			}
			
/*			int difference = panels[panel]->height - new_height;
			panels[panel]->height = new_height;
			panels[panel+1]->height += difference;*/
		}
	}
	return 0;
}

void sidebar_window::refresh_panels()
{
	HWND wnd = wnd_sidebar;
	
	RECT rc_client;
	GetClientRect(wnd, &rc_client);
	
	unsigned width = rc_client.right - rc_client.left;
	
	unsigned n, count = panels.get_count(), height_culm=0;
	for (n=0; n<count;)
	{
		if (!panels[n]->wnd_host)
		{
			ui_extension::window_ptr p_ext;
			p_ext = panels[n]->p_ext;

			bool b_new = false;

			if (!p_ext.is_valid()) 
			{
				ui_extension::window::create_by_guid(panels[n]->guid, p_ext);
				b_new = true;
			}
			
			if (p_ext.is_valid() && p_ext->is_available(&g_ui_ext_host_sidebar.get_static_instance()))
			{
				pfc::string8 name;
				if (!p_ext->get_short_name(name))
				p_ext->get_name(name);

				 HWND wnd_host = CreateWindowEx(WS_EX_CONTROLPARENT, panel_host_class_name, pfc::stringcvt::string_os_from_utf8(name),
				 WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, 0, 0, 0, wnd_sidebar, 0, core_api::get_my_instance(), this);

	/*				HWND wnd_host = uCreateWindowEx( WS_EX_TOOLWINDOW|WS_EX_CONTROLPARENT, REBARCLASSNAME, NULL, WS_BORDER |  WS_CHILD | 
				WS_CLIPCHILDREN |  WS_CLIPSIBLINGS |    RBS_DBLCLKTOGGLE | RBS_AUTOSIZE |*//*RBS_VARHEIGHT |*//*
				RBS_VERTICALGRIPPER |  CCS_NODIVIDER |  CCS_NOPARENTALIGN | CCS_VERT | CCS_TOP | CCS_NORESIZE | 0, 0, 0, 0, 0, 
				wnd_sidebar,  (HMENU)0,  core_api::get_my_instance(),  NULL);*/

				if (wnd_host)
				{
					if (b_new)
					{
						try{
						p_ext->set_config(&stream_reader_memblock_ref(panels[n]->config.get_ptr(), panels[n]->config.get_size()),panels[n]->config.get_size(),abort_callback_impl());
						}
						catch (exception_io e)
						{
							console::formatter() << "Error setting panel config: " << e.what();
						}
					}
					HWND wnd_panel = p_ext->create_or_transfer_window(wnd_host, &g_ui_ext_host_sidebar.get_static_instance());
					if (wnd_panel)
					{
			/*			uREBARBANDINFO  rbbi;
						memset(&rbbi,0,sizeof(rbbi));
						rbbi.cbSize       = sizeof(uREBARBANDINFO);

						rbbi.fMask        |=   RBBIM_CHILD | RBBIM_LPARAM | RBBIM_STYLE |  RBBIM_TEXT | 0;
						rbbi.cx           = 100;
						rbbi.fStyle       = RBBS_CHILDEDGE | RBBS_NOGRIPPER | RBBS_VARIABLEHEIGHT |0;
						rbbi.lParam          = n;
						rbbi.hwndChild    = wnd_panel;
						rbbi.lpText    = panels[n]->show_caption ? const_cast<char*>(name.get_ptr()) : "";
						
						uRebar_InsertItem(wnd_host, 0, &rbbi, true);*/


//						uSetWindowLong(wnd_host, GWL_USERDATA, (long)this);

						MINMAXINFO mmi;
						memset(&mmi, 0, sizeof(MINMAXINFO));
						mmi.ptMaxTrackSize.x = MAXLONG;
						mmi.ptMaxTrackSize.y = MAXLONG;
						uSendMessage(wnd_panel, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);

						panels[n]->wnd_host = wnd_host;
						panels[n]->wnd_panel = wnd_panel;
						panels[n]->p_ext = p_ext;
						panels[n]->min_height = mmi.ptMinTrackSize.y;
						panels[n]->min_width = mmi.ptMinTrackSize.x;
						panels[n]->max_width = mmi.ptMaxTrackSize.x;
						panels[n]->max_height = mmi.ptMaxTrackSize.y;
						n++;
					}
					else
					{
						DestroyWindow(wnd_host);
						p_ext.release();
						panels.delete_by_idx(n);
						count--;
					}
				}
				else
				{
					p_ext.release();
					panels.delete_by_idx(n);
					count--;
				}
			}
			else
			{
				p_ext.release();
				panels.delete_by_idx(n);
				count--;
			}
		}
		else n++;
	}
	
	size_panels();
	
	count = panels.get_count();
	for (n=0; n<count;n++)
	{
		ShowWindow(panels[n]->wnd_panel, SW_SHOWNORMAL);
		ShowWindow(panels[n]->wnd_host, SW_SHOWNORMAL);
	}
	RedrawWindow(wnd, 0, 0, RDW_UPDATENOW|RDW_ALLCHILDREN);
}

void sidebar_window::remove_panel(unsigned idx)
{
	if (idx < panels.get_count())
	{
		if (panels[idx]->p_ext.is_valid())
		{
			panels[idx]->p_ext->destroy_window();
			panels[idx]->wnd_panel=0;
			DestroyWindow(panels[idx]->wnd_host);
			panels[idx]->wnd_host=0;
			panels[idx]->p_ext.release();
		}
		panels.delete_by_idx(idx);
		size_panels();
	}
				
}


static unsigned last_position;
static bool is_dragging;
static unsigned dragging_panel;

LRESULT WINAPI sidebar_window::sidebar_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	sidebar_window * p_this;
	LRESULT rv;
	
	if(msg == WM_NCCREATE)
	{
		p_this = (sidebar_window *)((CREATESTRUCT *)(lp))->lpCreateParams;
		uSetWindowLong(wnd, GWL_USERDATA, (LPARAM)p_this);
	}
	else
		p_this = reinterpret_cast<sidebar_window*>(uGetWindowLong(wnd,GWL_USERDATA));
	
	rv = p_this ? p_this->on_message(wnd,msg,wp,lp) : uDefWindowProc(wnd, msg, wp, lp);
	
	return rv;
}

void sidebar_window::on_size(HWND wnd,int width,int height)
{
	sidebar_window * p_sw = reinterpret_cast<sidebar_window*>(uGetWindowLong(wnd,GWL_USERDATA));
	unsigned caption_height = 0;
	HWND wnd_panel = GetWindow(wnd, GW_CHILD);//; 
	if (wnd_panel)
	{
		if (p_sw)
		{
			unsigned panel = 0;
			bool found = p_sw->find_panel_by_host(wnd, panel);
			
			if (found && p_sw->panels[panel]->show_caption)
			{
				if (wnd_panel) 
				{
					caption_height = GetCaptionHeight();
				}
			}
		}
		SetWindowPos(wnd_panel, 0, 0, caption_height, width, height-caption_height, SWP_NOZORDER);
	}
}

LRESULT WINAPI sidebar_window::panel_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	

	sidebar_window * p_this = 0;
	if (msg == WM_NCCREATE)
	{
		p_this = (sidebar_window *)((CREATESTRUCT *)(lp))->lpCreateParams;
		uSetWindowLong(wnd, GWL_USERDATA, (LPARAM)p_this);
	}
	else
		p_this = reinterpret_cast<sidebar_window*>(uGetWindowLong(wnd,GWL_USERDATA));

	if (msg == WM_CREATE)
	{
		if (g_uxtheme.is_valid() && g_uxtheme->IsThemeActive())
		{
			uSetWindowLong(wnd, 0, (LPARAM)g_uxtheme->OpenThemeData(wnd, L"Rebar"));
		}
	}
	else if (msg == WM_THEMECHANGED)
	{
		if (g_uxtheme.is_valid())
		{
			HTHEME theme = (HTHEME)uGetWindowLong(wnd, 0);
			if (theme) g_uxtheme->CloseThemeData(theme);
			uSetWindowLong(wnd, 0, g_uxtheme->IsThemeActive() ? (LPARAM)g_uxtheme->OpenThemeData(wnd, L"Rebar") : 0);
		}
	}
	else if (msg == WM_DESTROY)
	{
		if (g_uxtheme.is_valid())
		{
			HTHEME theme = (HTHEME)uGetWindowLong(wnd, 0);
			if (theme) g_uxtheme->CloseThemeData(theme);
			uSetWindowLong(wnd, 0, 0);
		}
	}
/*	if (msg == WM_ERASEBKGND)
	{
		return TRUE;
	}*/
	else if (msg == WM_PAINT)
	{
		sidebar_window * p_sw = reinterpret_cast<sidebar_window*>(uGetWindowLong(wnd,GWL_USERDATA));

		if (p_sw)
		{
			unsigned panel = 0;
			bool found = p_sw->find_panel_by_host(wnd, panel);

			if (found && p_sw->panels[panel]->show_caption)
			{
			
				
				
				RECT rc_client, rc_dummy;
				GetClientRect(wnd, &rc_client);
				
				RECT rc_caption = {0, 0, rc_client.right, GetCaptionHeight()};//SM_CYSMCAPTION
				
				if (IntersectRect(&rc_dummy, &rc_client, &rc_caption))
				{
					PAINTSTRUCT ps;
					HDC dc = BeginPaint(wnd, &ps);
					if (g_uxtheme.is_valid())
					{
						if (g_uxtheme->IsThemeActive())
						{
							HTHEME theme = (HTHEME)uGetWindowLong(wnd, 0);
							if (theme) g_uxtheme->DrawThemeBackground(theme, dc, 	0, 0, &rc_caption, 0);
						}
					}
					
					//			DrawCaption(wnd, dc, &rc_caption, DC_TEXT|DC_SMALLCAP|(GetActiveWindow() == g_main_window ? DC_ACTIVE : 0)|DC_GRADIENT);
					pfc::string8 text;
					uGetWindowText(wnd, text);
					if (!g_menu_font) g_menu_font = uCreateMenuFont();
					
					HFONT old = SelectFont(dc, g_menu_font);
					uDrawPanelTitle(dc, &rc_caption, text, text.length());
					SelectFont(dc, old);
					
					
					EndPaint(wnd, &ps);
					return 0;
				}
				
		
			}
		}
	}
	else if (msg == WM_ERASEBKGND)
	{
		HDC dc = (HDC)wp;
		
		HWND wnd_parent = GetParent(wnd);
		POINT pt = {0, 0};
		MapWindowPoints(wnd, wnd_parent, &pt, 1);
		OffsetWindowOrgEx(dc, pt.x, pt.y, 0);
		uSendMessage(wnd_parent, WM_ERASEBKGND,wp, 0);
		SetWindowOrgEx(dc, pt.x, pt.y, 0);

		return TRUE;
	}
	else if (msg == WM_SIZE)
	{
		sidebar_window * p_sw = reinterpret_cast<sidebar_window*>(uGetWindowLong(wnd,GWL_USERDATA));
		unsigned caption_height = 0;
		HWND wnd_panel = GetWindow(wnd, GW_CHILD);//; 
		if (wnd_panel)
		{
			if (p_sw)
			{
				unsigned panel = 0;
				bool found = p_sw->find_panel_by_host(wnd, panel);
				
				if (found && p_sw->panels[panel]->show_caption)
				{
					if (wnd_panel) 
					{
						caption_height = GetCaptionHeight();
					}
				}
			}
			SetWindowPos(wnd_panel, 0, 0, caption_height, LOWORD(lp), HIWORD(lp)-caption_height, SWP_NOZORDER);
			if (caption_height && g_uxtheme.is_valid() && g_uxtheme->IsThemeActive())
			{
				RECT rc_caption = {0, 0, LOWORD(lp), caption_height};
				RedrawWindow(wnd, &rc_caption, 0, RDW_INVALIDATE|RDW_UPDATENOW);
			}

		}
		return 0;
	}
	else if (msg == WM_LBUTTONDBLCLK)
	{
		sidebar_window * p_sw = reinterpret_cast<sidebar_window*>(uGetWindowLong(wnd,GWL_USERDATA));
		HWND wnd_panel = GetWindow(wnd, GW_CHILD);
		if (wnd_panel)
		{
			if (p_sw)
			{
				unsigned panel = 0;
				bool found = p_sw->find_panel_by_host(wnd, panel);
				
				if (found)
				{ 
					POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
					if (ChildWindowFromPoint(wnd, pt) == wnd)
					{
				//		p_sw->size_panels(true);
						p_sw->panels[panel]->hidden = !p_sw->panels[panel]->hidden;
				/*		if (panel +1 < p_sw->panels.get_count())
						{
							bool hidden = p_sw->panels[panel]->hidden;
							unsigned caption_height = p_sw->panels[panel]->show_caption ? GetSystemMetrics(SM_CYMENU) : 0;
							unsigned new_height = p_sw->panels[panel+1]->height + ((hidden ? 1 : -1) * p_sw->panels[panel]->height-caption_height);
							unsigned new_height = p_sw->panels[panel+1]->height + ((hidden ? 1 : -1) * p_sw->panels[panel]->height-caption_height);
							p_sw->size_panels(true);
							unsigned int blah = panel;
//							console::info(pfc::string_printf("%u",p_sw->panels[panel]->height));
							p_sw->override_size(blah, new_height-p_sw->panels[panel+1]->height);
						}*/
						p_sw->size_panels();
					}
				}
			}
		}
		return 0;
	}
	else if (msg == WM_SYSCOLORCHANGE || msg == WM_SETTINGCHANGE)
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
	return uDefWindowProc(wnd, msg, wp, lp);
}

LRESULT WINAPI sidebar_window::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_CREATE:
		{
			wnd_sidebar = wnd;
			refresh_panels();
		}
		break;
	case WM_DESTROY:
		{
			unsigned n, count = panels.get_count();
			for (n=0; n<count;n++)
			{
				if (panels[n]->p_ext.is_valid())
				{
					panels[n]->config.set_size(0);
					stream_writer_memblock_ref blah(panels[n]->config);
					try
					{
						panels[n]->p_ext->get_config(&blah, abort_callback_impl());
					}
					catch (pfc::exception e){};
					panels[n]->p_ext->destroy_window();
					panels[n]->wnd_panel=0;
					DestroyWindow(panels[n]->wnd_host);
					panels[n]->wnd_host=0;
					panels[n]->p_ext.release();
					panels[n]->p_ext=0;
				}
			}
//			g_cfg_sidebar.set_sidebar_info(panels);
			
			wnd_sidebar = 0;
			
//			uSetWindowLong(wnd,GWL_USERDATA,0);
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
		{
			static LPARAM lp_last;
			static WPARAM wp_last;
			static UINT msg_last;
			static unsigned start_height;
			
			if (msg_last != msg || lp_last != lp || wp_last != wp)
			{
				
				POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
				HWND child = RealChildWindowFromPoint(wnd, pt);
				if (child==wnd)
				{
					unsigned n, count = panels.get_count(),height_culm=0;;
					for (n=0; n<count;n++)
					{
						if (panels[n]->p_ext.is_valid() && panels[n]->wnd_host)
						{
							RECT rc_panel;
							GetWindowRect(panels[n]->wnd_host, &rc_panel);
							assert(rc_panel.bottom>=rc_panel.top);
							unsigned height = (unsigned)(rc_panel.bottom-rc_panel.top);
							height_culm += height + ( n == count-1 ? 0 :2);
							if (height_culm /*+ height + 2*/ >= pt.y) break;
						}
					}
					
					if (count &&  ((msg == WM_MOUSEMOVE && (wp & MK_LBUTTON) && is_dragging) || (pt.y < height_culm) ))
					{ 
						if (n>=count) n=count-1;
						
						SetCursor(uLoadCursor(0, uMAKEINTRESOURCE(IDC_SIZENS)));
						
						if (msg == WM_LBUTTONDOWN)
						{
							size_panels(true);
					//		RECT rc_panel;
					//		GetWindowRect(panels[dragging_panel+1]->wnd, &rc_panel);

							dragging_panel=n;
							SetCapture(wnd);
							last_position = pt.y /*- height_culm*/;
			//				start_position = pt.y /*- height_culm*/;
							start_height =/* rc_panel.bottom-rc_panel.top;//*/panels[dragging_panel]->height;

				//			console::info(uStringPrintf("%u %u",panels[dragging_panel]->height,rc_panel.bottom-rc_panel.top));
							is_dragging=true;
						}
					}
					else
					{
						if (!(wp & MK_LBUTTON)) SetCursor(uLoadCursor(0, uMAKEINTRESOURCE(IDC_ARROW)));
						dragging_panel=0;
					}
				}
				
				if (is_dragging && wp & MK_LBUTTON) 
				{
					//		RECT rc; GetClientRect(wnd, &rc);
					int new_height = last_position - pt.y;//pt.y - start_position;
					
//					if ((int)(start_height + new_height) < 0) new_height = 0-start_height;
					
					if (msg == WM_MOUSEMOVE && wp & MK_LBUTTON && dragging_panel < panels.get_count())
					{
				//		console::info(pfc::string_printf("%i %i",start_height,new_height));
//						if (new_height > 0)
//							override_size(dragging_panel+1, last_position - pt.y);
//						else
						last_position = pt.y + override_size(dragging_panel,  pt.y - last_position);
						size_panels();
					}
			//		else
			//		last_position = pt.y;
				}
			}
			msg_last = msg;
			lp_last = lp;
			wp_last = wp;
			
		}
		break;
	case WM_LBUTTONUP:
		if (is_dragging)
		{
			is_dragging = false;
			if (GetCapture() == wnd) ReleaseCapture();
			//SetCursor(LoadCursor(0, IDC_ARROW));
		}
		break;
	case WM_SIZE:
		{
			size_panels();
		}
		break;
	case WM_SYSCOLORCHANGE:
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_SETTINGCHANGE:
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_MENUSELECT:
		{
			if (HIWORD(wp) & MF_POPUP)
			{
				m_status_override.release();
			}
			else 
			{
				unsigned id = LOWORD(wp);

				if (menu_ext_base && id < menu_ext_base)
				{
					service_ptr_t<ui_status_text_override> p_status_override;

					if (id >= IDM_BASE)
					{
						static_api_ptr_t<ui_control> api;
						api->override_status_text_create(p_status_override);

						if (p_status_override.is_valid())
						{
							p_status_override->override_text("Toggle this panel; Modifiers: CTRL - Insert panel; SHIFT - Force new panel instance");
						}
					}
					m_status_override = p_status_override;
				}
			}
		}
		break;
/*	case WM_ERASEBKGND:
		{
			if (g_uxtheme)
			{
				RECT rc_client;
				GetClientRect(wnd, &rc_client);
				static HTHEME theme = g_uxtheme->OpenThemeData(wnd, L"ExplorerBar");
				g_uxtheme->DrawThemeBackground(theme, HDC(wp), 		EBP_IEBARMENU, 0, &rc_client, 0);
				return TRUE;
			}
		}
		break;*/
	case WM_CONTEXTMENU:
		{
			
			service_enum_t<ui_extension::window> e;
			ui_extension::window_ptr l;
			
			POINT pt;
			GetMessagePos(&pt);
			
			POINT pt_client = pt;
			
			ScreenToClient(wnd, &pt_client);
			
			HMENU menu = CreatePopupMenu();
			
			unsigned IDM_EXT_BASE=IDM_BASE+1;
			unsigned n;


			
			HWND child = RealChildWindowFromPoint(wnd, pt_client);
			
			ui_extension::window_ptr p_ext;

//			bool on_host = true;

			unsigned count = panels.get_count(), panel = 0;
			for (n=0; n<count;n++)
			{
				if (child == panels[n]->wnd_host)
				{
					p_ext = panels[n]->p_ext; 
					panel = n; 
					
//					on_host = WindowFromPoint(pt) == panels[n]->wnd_host;
					break;
				}
			}
			//		console::error(uStringPrintf("%u %u %u", pt, child));
			
			
			ui_extension::window_info_list_simple moo;

			if (e.first(l))
			do
			{
				if (check_panel(l->get_extension_guid()) || (l->is_available(&g_ui_ext_host_sidebar.get_static_instance()) ))
				{
					ui_extension::window_info_simple info;
				
				l->get_name(info.name);
				l->get_category(info.category);
				info.guid = l->get_extension_guid();
				info.prefer_multiple_instances = l->get_prefer_multiple_instances();
				
				moo.add_item(info);
				
				l.release();
				}
			}while(e.next(l));

			moo.sort();

			unsigned count_exts = moo.get_count();
			HMENU popup;
			for(n=0;n<count_exts;n++)
			{
				if (!n || uStringCompare(moo[n-1].category, moo[n].category))
				{
					if (n) uAppendMenu(menu,MF_STRING|MF_POPUP,(UINT)popup,moo[n-1].category);
					popup = CreatePopupMenu();
				}
				uAppendMenu(popup,(MF_STRING|check_panel(moo[n].guid) ? MF_CHECKED : 0),IDM_BASE+n,moo[n].name);
				if (n == count_exts-1) uAppendMenu(menu,MF_STRING|MF_POPUP,(UINT)popup,moo[n].category);
				IDM_EXT_BASE++;
			}

			if (p_ext.is_valid()) 
			{
				if (moo.get_count()) uAppendMenu(menu,(MF_SEPARATOR),0,"");
				uAppendMenu(menu,(MF_STRING | (panels[panel]->show_caption ? MF_CHECKED : 0) ),IDM_CAPTION,"Show &caption");
				uAppendMenu(menu,(MF_STRING | (panels[panel]->locked ? MF_CHECKED : 0) ),IDM_LOCK,"&Lock panel");
				uAppendMenu(menu,(MF_SEPARATOR),0,"");
				uAppendMenu(menu,(MF_STRING),IDM_MOVE_UP,"Move &up");
				uAppendMenu(menu,(MF_STRING),IDM_MOVE_DOWN,"Move &down");
				uAppendMenu(menu,(MF_STRING),IDM_CLOSE,"&Close panel");
			}

			void * user_data = 0;
			pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> extension_menu_nodes = new ui_extension::menu_hook_impl;
			
			if (p_ext.is_valid() /*&& !on_host*/) 
			{
				p_ext->get_menu_items(*extension_menu_nodes.get_ptr()); 
				if (extension_menu_nodes->get_children_count() > 0)
					uAppendMenu(menu,MF_SEPARATOR,0,0);
					
				extension_menu_nodes->win32_build_menu(menu, IDM_EXT_BASE, infinite - IDM_EXT_BASE);
			}
			menu_helpers::win32_auto_mnemonics(menu);
			
			menu_ext_base = IDM_EXT_BASE;

			int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);

			m_status_override.release();

			menu_ext_base=0;
			
			if (cmd >= IDM_EXT_BASE)
			{
				extension_menu_nodes->execute_by_id(cmd);
			}
			
			DestroyMenu(menu);
			
			if (cmd == IDM_CLOSE && p_ext.is_valid() && panel>=0 && panel <panels.get_count())
			{
				remove_panel(panel);

		/*		if (panels[panel]->p_ext)
				{
					panels[panel]->p_ext->destroy_window(panels[panel]->wnd);
					panels[panel]->wnd=0;
					panels[panel]->p_ext->service_release();
					panels[panel]->p_ext=0;
				}
				panels.delete_by_idx(panel);
				refresh_panels();*/
			}
			else if (cmd == IDM_MOVE_UP)
			{
				panels.move_up(panel);
				size_panels();
			}
			else if (cmd == IDM_MOVE_DOWN)
			{
				panels.move_down(panel);
				size_panels();
			}
			else if (cmd == IDM_LOCK)
			{
				size_panels(true);
/*				RECT rc;
				GetWindowRect(panels[panel]->wnd, &rc);
				panels[panel]->height = rc.bottom - rc.top;*/

				bool locked = panels[panel]->locked;


				panels[panel]->locked = locked == 0;
			}
			else if (cmd == IDM_CAPTION)
			{
				bool show_caption = panels[panel]->show_caption;
				panels[panel]->show_caption = show_caption == 0;
				panels[panel]->hidden = false;
				size_panels();
				g_on_size(panels[panel]->wnd_host);
			}
			else if (cmd > 0 && cmd-IDM_BASE < moo.get_count())
			{
				bool shift_down = (GetAsyncKeyState(VK_SHIFT) & (1 << 31)) != 0;
				bool ctrl_down = (GetAsyncKeyState(VK_CONTROL) & (1 << 31)) != 0;

				if (!shift_down && !moo[cmd-IDM_BASE].prefer_multiple_instances && check_panel(moo[cmd-IDM_BASE].guid))
				{
					unsigned u;
					if (find_panel(moo[cmd-IDM_BASE].guid, u))
						remove_panel(u);
				}
				else
				{
					extension_info * p_ei = new(std::nothrow) extension_info(moo[cmd-IDM_BASE].guid);
					if (p_ei)
					{
						ui_extension::window_ptr p_next;
						ui_extension::window::create_by_guid(p_ei->guid, p_next);
						if (p_next.is_valid())
						{
							DWORD dw_type = p_next->get_type();
							p_ei->show_caption = (dw_type & ui_extension::type_panel) || (dw_type & ui_extension::type_playlist);
							p_next.release();
						}
						if (ctrl_down) panels.insert_item(p_ei, panel);
						else panels.add_item(p_ei);
						refresh_panels();
					}
				}
			}
			return 0;
		}
	}
	return uDefWindowProc(wnd, msg, wp, lp);
}


bool sidebar_window::on_menu_char (unsigned short c)
{
	bool rv;
	unsigned n, count = panels.get_count();

	for (n=0; n<count; n++)
	{
		HWND wnd = panels[n]->wnd_panel;
		ui_extension::window_ptr p_ext = panels[n]->p_ext;
		if (p_ext.is_valid() && wnd)
		{
			service_ptr_t<uie::menu_window> p_menu_ext;
			if (p_ext->service_query_t(p_menu_ext))
			{
				rv = p_menu_ext->on_menuchar(c);
				if (rv) break;
			}
		}
	}
	return rv;

}

void sidebar_window::on_menu_key_down ()
{
	unsigned n, count = panels.get_count();

	for (n=0; n<count; n++)
	{
		HWND wnd = panels[n]->wnd_panel;
		ui_extension::window_ptr p_ext = panels[n]->p_ext;
		if (p_ext.is_valid() && wnd)
		{
			service_ptr_t<uie::menu_window> p_menu_ext;
			if (p_ext->service_query_t(p_menu_ext))
			{
				p_menu_ext->show_accelerators();
			}
		}
	}

}

void sidebar_window::hide_accelerators ()
{
	unsigned n, count = panels.get_count();

	{

		for (n=0; n<count; n++)
		{
			HWND wnd = panels[n]->wnd_panel;
			ui_extension::window_ptr p_ext = panels[n]->p_ext;
			if (p_ext.is_valid() && wnd)
			{
				service_ptr_t<uie::menu_window> p_menu_ext;
				if (p_ext->service_query_t(p_menu_ext))
				{
					p_menu_ext->hide_accelerators();
				}
			}
		}
	}

}

void sidebar_window::on_sys_command_menu_key ()
{
	unsigned n, count = panels.get_count();


	for (n=0; n<count; n++)
	{
		HWND wnd = panels[n]->wnd_panel;
		ui_extension::window_ptr p_ext = panels[n]->p_ext;
		if (p_ext.is_valid() && wnd)
		{
			service_ptr_t<uie::menu_window> p_menu_ext;
			if (p_ext->service_query_t(p_menu_ext))
			{
				p_menu_ext->set_focus();
				break;
			}
		}
	}

}



/*
class initquit_ext : public initquit
{
	virtual void on_init()
	{
	}
	virtual void on_quit()
	{
		if (g_sidebar_window) 
		{
			g_sidebar_window->destroy();
			delete g_sidebar_window;
			g_sidebar_window = 0;
		}
		else sidebar_window::unregister_class();
	}
	virtual void on_system_shutdown() 
	{
		on_quit();
	}
	
};


initquit_factory<initquit_ext> foo6;





class menu_items_columns_ui : public menu_item_main
{
	virtual unsigned get_num_items()
	{
		return 1;
	}
	virtual void enum_item(unsigned n,pfc::string_base & out)
	{
		if (n==0) 	out.set_string("Components/UI Extensions/Create sidebar mockup");
	}
	virtual void perform_command(unsigned n)
	{
		if (n==0) 
		{
			if (g_sidebar_window && !g_sidebar_window->wnd_sidebar)
			{
				delete g_sidebar_window;
				g_sidebar_window = 0;
			}
			if (!g_sidebar_window)
			{
				g_sidebar_window = new(std::nothrow) sidebar_window;
				sidebar_info blah;
				g_cfg_sidebar.get_sidebar_info(blah);
				if (!g_sidebar_window->init(blah))
				{
					delete g_sidebar_window;
					g_sidebar_window = 0;
				}
			}
			else
			{
				SetForegroundWindow(g_sidebar_window->wnd_sidebar);
			}
			
		}
	}
	virtual bool get_description(unsigned n,pfc::string_base & out) {if (n==0) {out.set_string("sidebar"); return true;} return false;}
};

static menu_item_factory<menu_items_columns_ui> foo2;

*/
#endif