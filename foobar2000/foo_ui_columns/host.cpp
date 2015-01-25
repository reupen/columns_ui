#include "foo_ui_columns.h"

#if 0

// {340A35DD-937A-4098-83B1-362CEB99DA80}
static const GUID guid_host = 
{ 0x340a35dd, 0x937a, 0x4098, { 0x83, 0xb1, 0x36, 0x2c, 0xeb, 0x99, 0xda, 0x80 } };

extension * object_ptr::query_extension() const
{
	object * ptr = get_ptr();
	if (ptr && ptr->get_type() == TYPE_EXTENSION)
		return static_cast<extension*>(ptr);
	return 0;
}
extension_list * object_ptr::query_extension_list() const
{
	object * ptr = get_ptr();
	if (ptr && ptr->get_type() == TYPE_EXTENSION_LIST)
		return static_cast<extension_list*>(ptr);
	return 0;
}


cfg_host_t cfg_host(guid_host);

__forceinline int GetCaptionHeight()
{
	HFONT font = uCreateMenuFont();
	int rv = uGetFontHeight(font);
	DeleteFont(font);
	rv+=9;
	return rv;
}


bool is_winnt()
{
	static OSVERSIONINFO ov;
	static bool blah = false;

	if (!blah)
	{
		ov.dwOSVersionInfoSize = sizeof(ov);
		GetVersionEx(&ov);
	}

	switch(ov.dwPlatformId)
	{
	default:
	case VER_PLATFORM_WIN32_WINDOWS:
	case VER_PLATFORM_WIN32s:
		return false;
	case VER_PLATFORM_WIN32_NT:
		return true;
	}
}

void object::set_area(int x, int y, int width, int height)
{
	rc_area.left = x;
	rc_area.top = y;
	rc_area.right = x+width;
	rc_area.bottom = y+height;
}

void object::on_size()
{
	HDWP dwp = BeginDeferWindowPos(get_child_count());
	dwp = on_size(dwp);
	EndDeferWindowPos(dwp);
}

bool extension::have_extension(const GUID & p_guid)
{
	return (guid == p_guid) != 0;
}

void extension::destroy()
	{
		if (p_ext.is_valid())
		{
			config.set_size(0);
			stream_writer_memblock_ref blah(config);
			try{
			p_ext->get_config(&blah, abort_callback_impl());
			} catch (pfc::exception e) {};
			p_ext->destroy_window();
			wnd_panel=0;
			DestroyWindow(wnd_host);
			wnd_host=0;
			p_ext.release();
		}
	}

	unsigned extension::get_child_count()
	{
		return 1;
	}
	HDWP extension::on_size(HDWP dwp)
	{
		return wnd_host ? DeferWindowPos(dwp, wnd_host, 0, rc_area.left, rc_area.top, rc_area.right-rc_area.left, rc_area.bottom-rc_area.top, SWP_NOZORDER) : dwp;
	}
#if 0
	void extension::setup_tree(HWND wnd_tree, HTREEITEM ti_parent)
	{
		pfc::string8 sz_text;
		if (p_ext.is_valid())
		{
			p_ext->get_name(sz_text);
		}
		else
		{
//			refcounted_ptr_t<ui_extension> ext;
			ui_extension::window::create_by_guid(guid, p_ext);
			if (p_ext.is_valid())
			{
				p_ext->set_config(&stream_reader_memblock_ref(config, config.get_size()));
				p_ext->get_name(sz_text);
//				ext.release();
			}
			else sz_text.set_string("Unknown extension");
		}

		uTVINSERTSTRUCT is;
		memset(&is,0,sizeof(is));
		is.hParent = ti_parent;
		is.hInsertAfter = TVI_LAST;
		is.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
		is.item.pszText = const_cast<char*>(sz_text.get_ptr());
		is.item.state = TVIS_EXPANDED;
		is.item.stateMask = TVIS_EXPANDED;
		is.item.lParam = (LPARAM)static_cast<object*>(this);
		HTREEITEM item = uTreeView_InsertItem(wnd_tree,&is);

		if (p_ext.is_valid())
		{
			service_ptr_t<ui_extension::splitter_window> p_sw;
			if (p_ext->service_query_t(p_sw))
			{
				pfc::list_t<ui_extension::extension_data_impl> childs;
				p_sw->get_children(childs);
				unsigned n, count = childs.get_count();
				for (n=0; n<count; n++)
				{
					ui_extension::window_ptr e;
					ui_extension::window::create_by_guid(childs[n].get_guid(), e);
					pfc::string8 sz;
					if (e.is_valid())
						e->get_name(sz);
					else
						sz = "Unknown extension";

					uTVINSERTSTRUCT is;
					memset(&is,0,sizeof(is));
					is.hParent = item;
					is.hInsertAfter = TVI_LAST;
					is.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
					is.item.pszText = const_cast<char*>(sz.get_ptr());
					is.item.state = TVIS_EXPANDED;
					is.item.stateMask = TVIS_EXPANDED;
					is.item.lParam = 0;
					HTREEITEM item = uTreeView_InsertItem(wnd_tree,&is);
				}
			}
		}

	}
#endif
	bool extension::on_size_limit_change(HWND wnd, unsigned flags)
	{
		bool rv = false;
		if (wnd == wnd_panel)
		{
			if (p_ext.is_valid())
			{
				MINMAXINFO mmi;
				memset(&mmi, 0, sizeof(MINMAXINFO));
				mmi.ptMaxTrackSize.x = MAXLONG;
				mmi.ptMaxTrackSize.y = MAXLONG;
				uSendMessage(wnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
				min_width = mmi.ptMinTrackSize.x;
				min_height = mmi.ptMinTrackSize.y;
				max_height = mmi.ptMaxTrackSize.y;
				max_width = mmi.ptMaxTrackSize.x;
			}
			rv = true;
		}
		return rv;
	}

	bool extension::is_visible(HWND wnd, bool & rv)
	{
		bool handled = false;
		if (wnd == wnd_panel)
		{
			rv = !hidden;
			handled = true;
		}
		return handled;
	}

	bool extension::is_visibility_modifiable(HWND wnd, bool desired_visibility, bool & rv)
	{
		bool handled = false;
		if (wnd == wnd_panel)
		{
			rv = true;
			handled = true;
		}
		return handled;
	}

	bool extension::set_window_visibility(HWND wnd, bool visibility, bool & rv)
	{
		bool handled = false;
		if (wnd == wnd_panel)
		{
			rv = !hidden;
			handled = true;
		}
		return handled;
	}

	bool extension::relinquish_ownership(HWND wnd)
	{
		bool handled = false;
		if (wnd == wnd_panel)
		{
			destroy();
			handled = true;
		}
		return handled;
	}

bool focus_playlist_recur(const uie::window_ptr & p_wnd)
{
	service_ptr_t<uie::playlist_window> p_playlist_wnd;
	service_ptr_t<uie::splitter_window> p_splitter_wnd;
	if (p_wnd.is_valid())
	{
		if (p_wnd->service_query_t(p_playlist_wnd))
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
				if (temp.is_valid() && focus_playlist_recur(temp->get_window_ptr()))
					return true;
			}
		}
	}
	return false;
}

bool extension::focus_playlist_window()
{
	if (p_ext.is_valid() && wnd_panel)
	{
		return focus_playlist_recur(p_ext);
	}
	return false;
}


	void extension_list::write(stream_writer * out, abort_callback & p_abort)
	{
//		DEBUG_TRACK_CALL_TEXT(extension_list::write);
			unsigned num = m_objects.get_count();
			DWORD type = TYPE_EXTENSION_LIST;
			out->write_lendian_t(type, p_abort);
			out->write_lendian_t(orientation, p_abort);
			out->write_lendian_t(autohide, p_abort);
			out->write_lendian_t(height, p_abort);
			out->write_lendian_t(locked, p_abort);
			out->write_lendian_t(hidden, p_abort);
			out->write_lendian_t(num, p_abort);
			unsigned n;
			for (n=0;n<num;n++)
			{
				m_objects.get_item(n)->write(out, p_abort);
			}

	}
	void extension_list::read(stream_reader*t, abort_callback & p_abort)
	{
		//DEBUG_TRACK_CALL_TEXT(extension_list::read);
			t_uint32 num, n, temp;
			t->read_lendian_t(temp, p_abort);
			orientation = temp;

			t->read_lendian_t(autohide, p_abort);

			t->read_lendian_t(temp, p_abort);
			height = temp;

			t->read_lendian_t(locked, p_abort);

			t->read_lendian_t(hidden, p_abort);

			t->read_lendian_t(num, p_abort);

			for (n=0; n<num; n++)
			{

				t_uint32 type;
				t->read_lendian_t(type, p_abort);
				if (type==TYPE_EXTENSION)
				{
					extension * item = new extension(this);
					try{ (item)->read(t, p_abort);}
					catch (pfc::exception e)
					{
						delete item;
						item=0;
						throw e;
					}
					m_objects.add_item(object_ptr(item));
				}
				else if (type==TYPE_EXTENSION_LIST)
				{
					extension_list * item = new extension_list(this);
					try{ (item)->read(t, p_abort);}
					catch (pfc::exception e)
					{
						delete item;
						item=0;
						throw e;
					}
					m_objects.add_item(object_ptr(item));
				}
			}
	}
#if 0
	void extension_list::setup_tree(HWND wnd_tree, HTREEITEM ti_parent)
	{

		uTVINSERTSTRUCT is;
		memset(&is,0,sizeof(is));
		is.hParent = ti_parent;
		is.hInsertAfter = TVI_LAST;
		is.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
		is.item.pszText = orientation == HORZ ? "Splitter" : "Splitter";
		is.item.state = TVIS_EXPANDED;
		is.item.stateMask = TVIS_EXPANDED;
		is.item.lParam = (LPARAM)static_cast<object*>(this);
		HTREEITEM item = uTreeView_InsertItem(wnd_tree,&is);

		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			p_obj->setup_tree(wnd_tree, item);
/*			switch (p_obj->get_type())
			{
			case TYPE_EXTENSION:
				{
					extension * item = static_cast<extension*>(p_obj);
						p_obj->copy(*item);
						out.add_item(item);
					}
				}
				break;
			case TYPE_EXTENSION_LIST:
				{
					extension_list * item = static_cast<extension_list*>(p_obj);;
					if (item)
					{
						p_obj->copy(*item);
						out.add_item(item);
					}
				}
				break;
			}*/
		}

	}
#endif
	bool extension_list::on_size_limit_change(HWND wnd, unsigned flags)
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			if (p_obj->on_size_limit_change(wnd, flags))
			{
				if (!p_obj_parent.is_valid()) on_size();
				return true;
			}
		}
		return false;
	}

	void extension_list::show_window()
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			p_obj->show_window();
		}
	}

	bool extension_list::focus_playlist_window()
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			if (p_obj->focus_playlist_window())
			{
				return true;
			}
		}
		return false;
	}

bool extension_list::have_extension(const GUID & p_guid)
{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			if (p_obj->have_extension(p_guid))
			{
				return true;
			}
		}
		return false;
}

	bool extension_list::request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height, bool & rv)
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			if (p_obj->request_resize(wnd, flags, width, height, rv))
			{
	//			if (flags & (orientation == VERT ? ui_extension::size_height : ui_extension::size_width))
				{
	//				override_size(idx, height-g_sidebar_window->panels[idx]->height);
				}
				
			}
		}
		return false;
	}

	bool extension_list::is_visible(HWND wnd, bool & rv)
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			if (p_obj->is_visible(wnd, rv))
				return false;
		}
		return false;
	}

	bool extension_list::is_visibility_modifiable(HWND wnd, bool desired_visibility, bool & rv)
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			if (p_obj->is_visibility_modifiable(wnd, desired_visibility, rv))
				return false;
		}
		return false;
	}

	bool extension_list::set_window_visibility(HWND wnd, bool visibility, bool & rv)
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			if (p_obj->set_window_visibility(wnd, visibility, rv))
				return false;
		}
		return false;
	}

	bool extension_list::relinquish_ownership(HWND wnd)
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			if (p_obj->relinquish_ownership(wnd))
			{
				m_objects.remove_by_idx(n);
				return false;
			}
		}
		return false;
	}

bool extension_list::find_by_divider_pt(POINT & pt, object_ptr & p_out, object_ptr & p_obj_next)
{
	if (!PtInRect(&rc_area, pt)) return 0;
	
	unsigned n, count = m_objects.get_count(),height_culm=0;;
	for (n=0; n<count;n++)
	{
		object_ptr p_obj = m_objects.get_item(n);
		
		if (PtInRect(&p_obj->rc_area, pt)) return p_obj->find_by_divider_pt(pt, p_out, p_obj_next);
		
//		RECT rc_divider(p_obj->rc_area);
		bool in_divider = false;
		if (orientation == VERT)
		{
//			rc_divider.top =  rc_divider.bottom;
//			rc_divider.bottom += (n == count-1) ? 0 :2;
			in_divider = (pt.y >= p_obj->rc_area.bottom) && (pt.y < (p_obj->rc_area.bottom + ((n == count-1) ? 0 :2)));
		}
		else
		{
			in_divider = pt.x >= p_obj->rc_area.right && pt.x < (p_obj->rc_area.right + ((n == count-1) ? 0 :2));
//			rc_divider.left =  rc_divider.right;
//			rc_divider.right += (n == count-1) ? 0 :2;
		}
		RECT rc;
		if (p_obj->p_obj_parent.is_valid())
		{
			rc = p_obj->p_obj_parent->rc_area;
			//rc.right++;
			//rc.bottom++;
		}
		if (in_divider && (!p_obj->p_obj_parent.is_valid() || PtInRect(&rc, pt))) 
		{
			if (p_obj->p_obj_parent.is_valid() && n < count-1) p_obj_next = m_objects.get_item(n+1);
			p_out = p_obj;
			return true;
		}
		
	}
	return false;
}

int extension_list::override_size(unsigned idx, int delta)
{

	unsigned count = m_objects.get_count();
	if (count)
	{
		on_size(0, true);
		if (idx + 1 < count)
		{
			unsigned n=0;

			unsigned the_caption_height = GetCaptionHeight();
			pfc::array_t<min_max_info> minmax;
			minmax.set_size(count);
			memset(minmax.get_ptr(), 0, minmax.get_size()*sizeof(min_max_info));

			//minmax.fill(0);

			for (n=0; n<count; n++)
			{
				object_ptr p_obj = m_objects.get_item(n);
				extension_ptr p_eobj = p_obj;

				size_limits limits;

				p_obj->get_size_limits(limits);

				unsigned caption_height =(p_eobj.is_valid() && p_eobj->show_caption && p_obj->orientation == orientation) ? the_caption_height : 0;
				//unsigned min_height = p_obj->hidden ? 0 :  ( (orientation == VERT ? limits.min_height : limits.min_width) );
				//unsigned max_height = p_obj->hidden ? 0 : ( (orientation == VERT ? limits.max_height : limits.max_width) );
				unsigned min_height = ( (orientation == VERT ? limits.min_height : limits.min_width) );
				unsigned max_height = ( (orientation == VERT ? limits.max_height : limits.max_width) );

				{
					//if (min_height < (unsigned)(0-caption_height)) min_height += caption_height;
					//if (max_height < (unsigned)(0-caption_height)) max_height += caption_height;
				}

				minmax[n].min_height = min_height;
				minmax[n].max_height = max_height;
				minmax[n].height = p_obj->hidden ? caption_height : p_obj->height;
			}


			bool is_up = delta < 0;//new_height < panels[panel]->height;
			bool is_down = delta > 0;//new_height > panels[panel]->height;

		
			if (is_up /*&& !panels[panel]->locked*/)
			{

				unsigned diff_abs = 0, diff_avail = abs(delta);

				unsigned n = idx+1;
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

				n = idx+1;
				unsigned obtained =0;
				while (n>0 && obtained < diff_abs)
				{
					n--;
					//					if (!panels[n]->locked)
					{
						unsigned height = (diff_abs-obtained > minmax[n].height ? 0 : minmax[n].height-(diff_abs-obtained));
						object_ptr p_obj = m_objects.get_item(n);
						extension_ptr p_eobj = p_obj;

						//unsigned caption_height =(p_eobj.is_valid() && p_eobj->show_caption) ? the_caption_height : 0;
						
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
						if (!p_obj->hidden) p_obj->height = height;
						
					}
				}
				n=idx;
				unsigned obtained2 = obtained;

				while (n < count-1 && obtained2 )
				{
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

					object_ptr p_obj = m_objects.get_item(n);
					
					obtained2 -= height - minmax[n].height;
					minmax[n].height = height;
					if (!p_obj->hidden) p_obj->height = height;
				}
				return (abs(delta)-obtained);
				
				
			}
			else if (is_down /*&& !panels[panel]->locked*/)
			{
				unsigned diff_abs = 0, diff_avail = abs(delta);

				n = idx+1;
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

						
						diff_abs += height - minmax[n].height;
					}
				}
				n = idx;
				unsigned obtained =0;
				while (n < count-1 && obtained < diff_abs)
				{
					n++;
	//				if (!panels[n]->locked)
					{
						unsigned height = (diff_abs-obtained > minmax[n].height ? 0 : minmax[n].height-(diff_abs-obtained));
						
						object_ptr p_obj = m_objects.get_item(n);
						extension_ptr p_eobj = p_obj;

						//unsigned caption_height =(p_eobj.is_valid() && p_eobj->show_caption) ? the_caption_height : 0;
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
						if (!p_obj->hidden) p_obj->height = height;
						
					}
				}
				n=idx+1;
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
					object_ptr p_obj = m_objects.get_item(n);
					if (!p_obj->hidden) p_obj->height = height;
				}
				return 0-(abs(delta)-obtained);
				
			}
			
		}
	}
	return 0;
}


bool extension::request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height, bool & rv)
{
	bool handled = false;
	if (wnd == wnd_panel)
	{
		
		if (p_ext.is_valid() && p_obj_parent.is_valid() && p_obj_parent->get_type()==TYPE_EXTENSION_LIST)
		{
			if (!(flags & (orientation == VERT ? ui_extension::size_width : ui_extension::size_height)))
			{
				handled = true;
				rv = true;
			}
			rv = false;
		}
		rv = true;
	}
	return handled;
}

void extension::get_size_limits( size_limits & p_out)
{
	unsigned the_caption_height = show_caption ? GetCaptionHeight() : 0;
	if (hidden)
	{
		p_out.min_height = orientation == VERT ? the_caption_height : 0;
		p_out.max_height = orientation == VERT ? the_caption_height : max_height;
		p_out.min_width = orientation == VERT ? 0 : the_caption_height;
		p_out.max_width = orientation == VERT ? max_width : the_caption_height;
	}
	else
	{
		p_out.min_height = min_height;
		p_out.max_height = max_height;
		p_out.min_width = min_width;
		p_out.max_width = max_width;

		if (show_caption)
		{
			if (orientation == VERT) 
			{
				p_out.min_height += the_caption_height;
				if (p_out.max_height < infinite-the_caption_height) p_out.max_height += the_caption_height;
				else p_out.max_height = infinite;
			}
			else
			{
				p_out.min_width += the_caption_height;
				if (p_out.max_width < infinite-the_caption_height) p_out.max_width += the_caption_height;
				else p_out.max_width = infinite;
			}
		}
	}
	//console::info(pfc::string_printf("extension : %x %x %x %x",p_out));
}

void extension_list::get_size_limits( size_limits & p_out)
{
	p_out.min_height =0;
	p_out.max_height = orientation == VERT ? 0 : infinite;
	p_out.min_width = 0;
	p_out.max_width = orientation == VERT ? infinite : 0;

	unsigned n, count = m_objects.get_count();
	//	unsigned the_caption_height = GetCaptionHeight();

	for (n=0; n<count; n++)
	{
		object_ptr p_obj = m_objects.get_item(n);
		extension_ptr p_eobj =p_obj; 
		size_limits limits;
		p_obj->get_size_limits(limits);
		if (orientation == VERT)
		{
			p_out.min_height += limits.min_height+2;
			p_out.min_width = max (limits.min_width, p_out.min_width);
			if (p_out.max_height <= infinite - limits.max_height && p_out.max_height + limits.max_height <= infinite-2)
			{
				p_out.max_height += limits.max_height+2;
			}
			else
			{
				p_out.max_height = infinite;
			}
			p_out.max_width = min (limits.max_width, p_out.max_width);
		}
		else
		{
			p_out.min_width += limits.min_width+2;
			p_out.min_height = max (limits.min_height, p_out.min_height);
			if (p_out.max_width <= infinite - limits.max_width && p_out.max_width + limits.max_width <= infinite-2)
			{
				p_out.max_width += limits.max_width+2;
			}
			else 
			{
				p_out.max_width = infinite;
			}
			p_out.max_height = min (limits.max_height, p_out.max_height);
		}
	}
	if (orientation == VERT)
		p_out.max_width = max (p_out.max_width, p_out.min_width);
	else
		p_out.max_height = max (p_out.max_height, p_out.min_height);

	bool b_hidden = false;

	{
		object_ptr p_obj = this;
		while (p_obj.is_valid())
		{
			if (p_obj->hidden)
			{
				b_hidden = true;
				break;
			}
			p_obj = p_obj->p_obj_parent;
		}
	}

	if (b_hidden)
	{
		p_out.min_height = 0;
		if (orientation == HORZ) p_out.max_height = 0;
		p_out.min_width = 0;
		if (orientation == VERT) p_out.max_width = 0;
	}
	//size_limits
	//console::info(pfc::string_printf("%i %i %i %i",p_out));
}

void cfg_host_t::get_data_raw(stream_writer * out, abort_callback & p_abort)
{
	if (g_host_window)
	{
		stream_writer_memblock_ref config(val);
		val.set_size(0);
		g_host_window->get_config(&config, p_abort);
	}
	out->write(val.get_ptr(),val.get_size(), p_abort);
}

void cfg_host_t::set_data_raw(stream_reader * p_reader, unsigned p_sizehint, abort_callback & p_abort)
{
	if (p_sizehint)
		stream_to_mem_block(p_reader, val, p_abort, p_sizehint, true);
}

void cfg_host_t::reset()
{
	val.set_size(0);
	GUID guid_pv = { 0xf20bed8f, 0x225b, 0x46c3, { 0x9f, 0xc7, 0x45, 0x4c, 0xed, 0xb6, 0xcd, 0xad } };
	extension_ptr p_pv = new(std::nothrow) extension(0, guid_pv);
	p_pv->show_caption = false;

	stream_writer_memblock_ref data(val);
	p_pv->write(&data, abort_callback_impl());
}



void host_window::on_size_limit_change(HWND wnd, unsigned flags)
{
	if (p_obj_base.is_valid() && p_obj_base->on_size_limit_change(wnd, flags))
	{
		on_size();
	}
}

bool host_window::request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height)
{
	bool rv;
	if (p_obj_base.is_valid() && p_obj_base->request_resize(wnd, flags, width, height, rv))
	{
		//				override_size(idx, height-g_sidebar_window->panels[idx]->height);
		//on_size();
	}
	return rv;
}

bool host_window::is_visible(HWND wnd)const
{
	bool rv;
	if (p_obj_base.is_valid() && p_obj_base->is_visible(wnd, rv))
	{
	}
	return rv;
}

bool host_window::is_visibility_modifiable(HWND wnd, bool desired_visibility)const
{
	bool rv;
	if (p_obj_base.is_valid() && p_obj_base->is_visibility_modifiable(wnd, desired_visibility, rv))
	{
	}
	return rv;
}

bool host_window::set_window_visibility(HWND wnd, bool visibility)
{
	bool rv;
	if (p_obj_base.is_valid() && p_obj_base->set_window_visibility(wnd, visibility, rv))
	{
	}
	return rv;
}

void host_window::relinquish_ownership(HWND wnd)
{
	if (p_obj_base.is_valid() && p_obj_base->relinquish_ownership(wnd))
	{
		p_obj_base.release();
	}
}

void host_window::set_config(stream_reader*t, abort_callback & p_abort)
{
		if (p_obj_base.is_valid())
		{
			p_obj_base.release();
		}
		t_uint32 type;
		t->read_lendian_t(type, p_abort);
		if (type==TYPE_EXTENSION)
		{
			extension * item = new extension;
			try {item->read(t, p_abort);}
			catch (pfc::exception e)
			{
				delete item;
				item=0;
				throw e;
			}
			p_obj_base = item;
		}
		else if (type==TYPE_EXTENSION_LIST)
		{
			extension_list * item = new extension_list;
			try{item->read(t, p_abort);}
			catch (pfc::exception e)
			{
				delete item;
				item=0;
				throw e;
			}
			p_obj_base = item;
		}

}

void extension::write(stream_writer * out, abort_callback & p_abort)
{
		//		uMessageBox(0, show_caption ? "write: 1" : "write: 0", "", 0);
		if (p_ext.is_valid() && wnd_panel)
		{
			try {
			config.set_size(0);
			p_ext->get_config(&stream_writer_memblock_ref(config), p_abort);
			} catch (pfc::exception e) {};
		}
		//		DEBUG_TRACK_CALL_TEXT(extension::write);
		DWORD type = TYPE_EXTENSION;
		out->write_lendian_t(type, p_abort);
		out->write_lendian_t(guid, p_abort);
		out->write_lendian_t(orientation, p_abort);
		out->write_lendian_t(height, p_abort);
		out->write_lendian_t(locked, p_abort);
		out->write_lendian_t(show_caption, p_abort);
		out->write_lendian_t(hidden, p_abort);
		DWORD size = config.get_size();
		out->write_lendian_t(size, p_abort);
		out->write(config.get_ptr(), size, p_abort);
}
void extension::read(stream_reader*r, abort_callback & p_abort)
{
	//		DEBUG_TRACK_CALL_TEXT(extension::read);

		r->read_lendian_t(guid, p_abort);
		r->read_lendian_t(orientation, p_abort);
		r->read_lendian_t(height, p_abort);
		r->read_lendian_t(locked, p_abort);
		r->read_lendian_t(show_caption, p_abort);
		r->read_lendian_t(hidden, p_abort);

		t_uint32 mem_size;
		r->read_lendian_t(mem_size, p_abort);

		if (mem_size)
		{
			config.set_size(mem_size);
			r->read(config.get_ptr(), mem_size, p_abort);
		}

}
void extension::copy(object & out1)
{
	extension & out = static_cast<extension&>(out1);
	out.guid = guid;
	out.height = height;
	out.locked = locked;
	out.show_caption = show_caption;
	out.hidden = hidden;
	out.config.set_size(0);
	out.config.append_fromptr(config.get_ptr(), config.get_size());
}

void extension_list::copy(object & out1)
{
	extension_list & out = static_cast<extension_list&>(out1);
	out.m_objects.remove_all();
	out.orientation = orientation;
	unsigned n, count = m_objects.get_count();
	for (n=0; n<count; n++)
	{
		object_ptr p_obj = m_objects.get_item(n);
		switch (p_obj->get_type())
		{
		case TYPE_EXTENSION:
			{
				extension * item = new(std::nothrow) extension(&out);
				if (item)
				{
					p_obj->copy(*item);
					out.m_objects.add_item(object_ptr(item));
				}
			}
			break;
		case TYPE_EXTENSION_LIST:
			{
				extension_list * item = new(std::nothrow) extension_list(&out);
				if (item)
				{
					p_obj->copy(*item);
					out.m_objects.add_item(object_ptr(item));
				}
			}
			break;
		}
	}
}
void extension_list::set_list(extension_list & in)
{
	m_objects.remove_all();
	orientation = in.orientation;
	unsigned n, count = in.m_objects.get_count();

	for (n=0; n<count; n++)
	{
		switch ((in.m_objects[n])->get_type())
		{
		case TYPE_EXTENSION:
			{
				extension * item = new(std::nothrow) extension(this);
				if (item)
				{
					(in.m_objects[n])->copy(*item);
					m_objects.add_item(object_ptr(item));
				}
			}
			break;
		case TYPE_EXTENSION_LIST:
			{
				extension_list * item = new(std::nothrow) extension_list(this);
				if (item)
				{
					(in.m_objects[n])->copy(*item);
					m_objects.add_item(object_ptr(item));
				}
			}
			break;
		}
	}
}
bool extension_list:: refresh(HWND wnd)
{
	if (autohide) hidden = true;
	unsigned n, count = m_objects.get_count();

	for (n=0; n<count; )
	{
		if (!(m_objects.get_item(n))->refresh(wnd))
		{
			m_objects.remove_by_idx(n);
			count--;
		}
		else n++;
	}
	return true;
}
bool extension_list::move_up(unsigned idx)
{
	unsigned count = m_objects.get_count();
	if (idx > 0 && idx< count)
	{
		order_helper order(count);
		order.swap(idx, idx-1);
		m_objects.reorder(order.get_ptr());
		return true;
	}
	return false;
}
bool extension_list::move_down(unsigned idx)
{
	unsigned count = m_objects.get_count();
	if (idx >= 0 && idx < (count-1))
	{
		order_helper order(count);
		order.swap(idx, idx+1);
		m_objects.reorder(order.get_ptr());
		return true;
	}
	return false;
}
extension * extension_list::find_by_wnd(HWND wnd)
{
	unsigned count = m_objects.get_count(),n;
	extension * rv = 0;
	for (n=0; n<count;n++)
	{
		object_ptr p_obj = m_objects.get_item(n);
		if (p_obj->get_type() == TYPE_EXTENSION_LIST)
		{
			rv = p_obj.query_extension_list()->find_by_wnd(wnd); if (rv) break;
		}
		else if (p_obj->get_type() == TYPE_EXTENSION)
			if (wnd == p_obj.query_extension()->wnd_panel) {rv = (p_obj.query_extension()); break;}
	}
	return rv;
}


void host_window::refresh(bool b_size)
{
	HWND wnd = get_wnd();
	uSendMessage(wnd, WM_SETREDRAW, FALSE, 0);

	p_obj_dragging=0;
	if (p_obj_base.is_valid())
	{
		p_obj_base->destroy();
		p_obj_base.release();
	}
	set_config(&stream_reader_memblock_ref(cfg_host.val.get_ptr(), cfg_host.val.get_size()), abort_callback_impl());
	if (p_obj_base.is_valid()) p_obj_base->refresh(wnd_host);
	on_size();
	if (b_size) show_window();
	uSendMessage(wnd, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(wnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN|RDW_FRAME|RDW_ERASE);
}


host_window::host_window() : wnd_host(0),menu_ext_base(0),p_obj_base(0),is_dragging(false),
p_obj_dragging(0),last_position(0)
{
};
host_window::~host_window() 
{
	//	if (p_obj_base.is_valid()) p_obj_base->release();
};

const TCHAR * host_window::panel_host_class_name = _T("foo_ui_columns_panel_host");

bool host_window::panel_host_class_registered =false;

void host_window::unregister_class()
{
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
	size_limits limits;
};


HDWP extension_list::on_size(HDWP dwp, bool set)
{

	int x = rc_area.left, y = rc_area.top, cx = rc_area.right-rc_area.left, cy = rc_area.bottom-rc_area.top;

	//	HWND wnd = wnd_host;
	RECT rc_client = {x, y, x+cx, y+cy};
	//	GetClientRect(wnd, &rc_client);

	unsigned width = (orientation != VERT ? rc_client.bottom - rc_client.top : rc_client.right-rc_client.left);
	unsigned total_height = (orientation == VERT ? rc_client.bottom - rc_client.top+2 : rc_client.right-rc_client.left+2);

	unsigned n, count = m_objects.get_count(),height_culm=0;


	if (count)
	{
		pfc::array_t<size_info> sizes;
		sizes.set_size(count);

		//sizes.fill(0);
		memset(sizes.get_ptr(), 0, sizes.get_size()*sizeof(size_info));

		//		unsigned the_caption_height = GetCaptionHeight();

		const int total_height = orientation == VERT ? rc_client.bottom - rc_client.top+2 : rc_client.right-rc_client.left+2;

		int available_height = total_height;
		unsigned available_parts=0;

		for (n=0; n<count;n++)
		{
			object_ptr p_obj = m_objects.get_item(n);
			extension_ptr p_eobj = p_obj;

			unsigned height = /*(p_obj->hidden) ? 0 :*/ p_obj->height;
			available_height -= height+2;
			sizes[n].height = height+2;
			sizes[n].parts = ((p_obj->locked /*|| p_obj->hidden*/)) ? 0 : 1;// (p_eobj && (p_eobj->locked || p_eobj->hidden)) ? 0 : 1;
			available_parts+=sizes[n].parts;

			p_obj->get_size_limits(sizes[n].limits);
/*			unsigned caption_height = (p_eobj.is_valid() && p_eobj->show_caption) ? the_caption_height : 0;

			if (p_obj->orientation == VERT)
			{
				if (p_obj->hidden) sizes[n].limits.min_height = 0;
				if (sizes[n].limits.min_height < (unsigned)(-3-caption_height)) sizes[n].limits.min_height += 2+caption_height;
				if (sizes[n].limits.max_height < (unsigned)(-3-caption_height)) sizes[n].limits.max_height += 2+caption_height;
			}
			else
			{
				if (p_obj->hidden) sizes[n].limits.min_width = 0;
				if (sizes[n].limits.min_width < (unsigned)(-3-caption_height)) sizes[n].limits.min_width += 2+caption_height;
				if (sizes[n].limits.max_width < (unsigned)(-3-caption_height)) sizes[n].limits.max_width += 2+caption_height;
			}*/

			if (orientation == VERT)
			{
				if (sizes[n].limits.min_height < infinite-2) sizes[n].limits.min_height += 2;
				else sizes[n].limits.min_height = infinite;
				if (sizes[n].limits.max_height < infinite-2) sizes[n].limits.max_height += 2;
				else sizes[n].limits.max_height = infinite;
			}
			else
			{
				if (sizes[n].limits.min_width < infinite-2) sizes[n].limits.min_width += 2;
				else sizes[n].limits.min_width = infinite;
				if (sizes[n].limits.max_width < infinite-2) sizes[n].limits.max_width += 2;
				else sizes[n].limits.max_width = infinite;
			}

			//if (!sizes[n].parts) 
			//	sizes[n].sized = true;
		}

//		console::error(uStringPrintf("%i %i",available_parts,available_height));

		bool first_pass(true);

		while (first_pass || (available_parts && available_height))
		{
			first_pass=false;

			unsigned parts=available_parts;
			int available_height2=available_height;

			for (n=0; n<count;n++)
			{
				if (!sizes[n].sized)
				{
					object_ptr p_obj = m_objects.get_item(n);
					extension_ptr p_eobj = p_obj;

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

					bool hidden = p_obj->hidden;//p_eobj ? p_eobj->hidden : false;

					height += adjustment;
					unsigned min_height = 0 ? 0 :  ( (orientation == VERT ? sizes[n].limits.min_height : sizes[n].limits.min_width) );
//					unsigned caption_height = (p_eobj.is_valid() && p_eobj->show_caption) ? the_caption_height : 0;
//					if (min_height < (unsigned)(-3-caption_height)) min_height += 2+caption_height;

					unsigned max_height = 0 ? 0 : ((orientation == VERT ? sizes[n].limits.max_height : sizes[n].limits.max_width));
//					if (max_height < (unsigned)(-3-caption_height)) max_height += 2+caption_height;

					if (height < min_height)
					{
//						if (min_height < (total_height - 2 * count +2))
						{
							height =  min_height;
							adjustment = (height-unadjusted);
							sizes[n].sized = true;
						}
					}
					else if (height > max_height)
					{
						height = max_height;
						adjustment = (height-unadjusted);
						sizes[n].sized = true;
					}
					if ((p_obj->locked)/* || hidden*/) sizes[n].sized = true;

	//				if (n==2) console::error(uStringPrintf("max %I, min %i",max_height,min_height));

					if (sizes[n].sized) available_parts-=sizes[n].parts;

					available_height-=(height-unadjusted);
					sizes[n].height = height;



				}
			}
			if (available_height2 == available_height) break; //no more sizing possible
		}


		if (set)
		{
			for (n=0; n<count;n++)
			{
				object_ptr p_obj = m_objects.get_item(n);
		//		extension * p_eobj = p_obj->get_type()==TYPE_EXTENSION ? static_cast<extension*>(p_obj) : 0;
				bool hidden = p_obj->hidden;
				if (!hidden) p_obj->height = sizes[n].height - 2;
			}
		}
		else
		{
			{
				for (n=0; n<count;n++)
				{
					object_ptr p_obj = m_objects.get_item(n);
					extension_ptr p_eobj = p_obj;
					unsigned height = sizes[n].height;


					orientation == HORZ ? p_obj->set_area(x+height_culm, y+0, min(height-2,sizes[n].limits.max_width-2), min(width,sizes[n].limits.max_height)) : p_obj->set_area(x+0, y+height_culm, min(width,sizes[n].limits.max_width), min(height-2,sizes[n].limits.max_height-2));
					dwp = p_obj->on_size(dwp);
					height_culm += height ;
				}
			}
		}
	}
	return dwp;
}

HHOOK hhk_mouse = 0;

class extension_list_pair
{
public:
	extension_list_ptr p_obj1;
	extension_list_ptr p_obj2;
	extension_list_pair(const extension_list_ptr & p1, const extension_list_ptr &p2)
	{
		if (p1.is_valid() && p1->autohide) p_obj1 = p1;
		if (p2.is_valid() && p2->autohide) p_obj2 = p2;
	}
};

class obj_mouse_class : public ptr_list_autodel_t<extension_list_pair>
{
public:
	bool is_valid()
	{
		unsigned n, count = get_count();
		for (n=0; n<count; n++)
		{
			extension_list_pair * item = get_item(n);
			if (item->p_obj1.is_valid() || item->p_obj2.is_valid()) return true;
		}
		return false;
	}
	bool test_area(POINT pt)
	{
		ScreenToClient(g_host_window->wnd_host, &pt);
		unsigned n, count = get_count();
		for (n=0; n<count; n++)
		{
			extension_list_pair * item = get_item(n);
			bool in_area1=false;
			bool in_area2=false;
			if (item->p_obj1.is_valid() && PtInRect(&item->p_obj1->rc_area, pt))
			{
				in_area1 = true;
			}
			if (item->p_obj2.is_valid() && PtInRect(&item->p_obj2->rc_area, pt))
			{
				in_area2 = true;
			}
			if (g_host_window->p_obj_base.is_valid())
			{
				object_ptr p_obj2 = 0;
				object_ptr p_obj1;
				g_host_window->p_obj_base->find_by_divider_pt(pt, p_obj1,p_obj2);
				if (p_obj1.is_valid() && p_obj1.get_ptr() == item->p_obj1.get_ptr()) in_area1 = true;
				if (p_obj2.is_valid() && p_obj2.get_ptr() == item->p_obj2.get_ptr()) in_area2 = true;
				
			}
			if (!(in_area1 || in_area2)) return false;
		}
		return true;
	}
	bool hide(POINT pt)
	{
		ScreenToClient(g_host_window->wnd_host, &pt);
		bool rv = false;
		unsigned n, count = get_count();
		for (n=count; n>0; n--)
		{
			extension_list_pair * item = get_item(n-1);
			bool in_area1=false;
			bool in_area2=false;
			if (item->p_obj1.is_valid() && PtInRect(&item->p_obj1->rc_area, pt))
			{
				in_area1 = true;
			}
			if (item->p_obj2.is_valid() && PtInRect(&item->p_obj2->rc_area, pt))
			{
				in_area2 = true;
			}
			if (g_host_window->p_obj_base.is_valid())
			{
				object_ptr p_obj2 = 0;
				object_ptr p_obj1;
				g_host_window->p_obj_base->find_by_divider_pt(pt, p_obj1,p_obj2);
				if (p_obj1.is_valid() && p_obj1.get_ptr() == item->p_obj1.get_ptr()) in_area1 = true;
				if (p_obj2.is_valid() && p_obj2.get_ptr() == item->p_obj2.get_ptr()) in_area2 = true;
			}

			if (!(in_area1 || in_area2))
			{
				extension_list_ptr p_list = item->p_obj1.is_valid() ? item->p_obj1->p_obj_parent : item->p_obj2->p_obj_parent;
				if (item->p_obj1.is_valid()) item->p_obj1->hidden = true;
				if (item->p_obj2.is_valid()) item->p_obj2->hidden = true;
				p_list->on_size();
				if (p_list->p_obj_parent.is_valid()) p_list->p_obj_parent->on_size();
				delete_by_idx(n-1);
				rv = true;
			}
		}
		return !get_count();
	}
} g_obj_mouse_list;

//extension_list_ptr p_obj_mouse1 =0;
//extension_list_ptr p_obj_mouse2 =0;

LRESULT WINAPI MouseHookProc(int code,WPARAM wp,LPARAM lp)
{
	if (code >= 0)
	{
		if (wp == WM_MOUSEMOVE && g_host_window)
		{
			static HWND wnd_mouse;

			MOUSEHOOKSTRUCT * mhs = (LPMOUSEHOOKSTRUCT)lp;
			if (wnd_mouse != mhs->hwnd)
			{
				if (mhs->hwnd != g_host_window->wnd_host && (g_obj_mouse_list.is_valid()))
				{
					/*
					bool in_area1=false;
					bool in_area2=false;
					POINT pt = mhs->pt;
					ScreenToClient(g_host_window->wnd_host, &pt);
					if (p_obj_mouse1.is_valid() && PtInRect(&p_obj_mouse1->rc_area, pt))
					{
						in_area1 = true;
					}
					if (p_obj_mouse2.is_valid() && PtInRect(&p_obj_mouse2->rc_area, pt))
					{
						in_area2 = true;
					}
					if (g_host_window->p_obj_base.is_valid())
					{
						object_ptr p_obj2 = 0;
						object_ptr p_obj1;
						g_host_window->p_obj_base->find_by_divider_pt(pt, p_obj1,p_obj2);
						if (p_obj1.is_valid() && p_obj1.get_ptr() == p_obj_mouse1.get_ptr()) in_area1 = true;
						if (p_obj2.is_valid() && p_obj2.get_ptr() == p_obj_mouse2.get_ptr()) in_area2 = true;
						
					}*/
					
					if (!g_obj_mouse_list.test_area(mhs->pt))
					{
						wnd_mouse = 0;
						PostMessage(g_host_window->get_wnd(), MSG_AUTOHIDE_END, 0, 0);
					}
				}
				wnd_mouse = mhs->hwnd;
			}
		}
	}
	return CallNextHookEx(hhk_mouse, code, wp, lp);
}

LRESULT host_window::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_CREATE:
		{
			wnd_host = wnd;
			if (!panel_host_class_registered)
			{
				WNDCLASS  wc;
				memset(&wc,0,sizeof(WNDCLASS));
				wc.style          = CS_DBLCLKS;
				wc.hInstance      = core_api::get_my_instance();
				wc.hCursor        = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
				wc.hbrBackground  = (HBRUSH)(COLOR_3DFACE+1);
				
				wc.style          = CS_DBLCLKS;
				wc.lpszClassName  = panel_host_class_name;
				wc.lpfnWndProc    = (WNDPROC)panel_proc;
				wc.cbWndExtra = 4;
				if (RegisterClass(&wc)) panel_host_class_registered = true;
			}
			
			refresh(false);
		}
		break;
	case WM_DESTROY:
		{
			if (p_obj_base.is_valid())
			{
				p_obj_base->destroy();
//				stream_writer_memblock config;
//				p_obj_base->write(&config);

//				cfg_host.set_data_raw(config.m_data, config.m_data.get_size());
			}

			wnd_host = 0;
			
			unregister_class();
		}
		break;
	case WM_LBUTTONDBLCLK:
		if (p_obj_base.is_valid())
		{
//			console::info("dbl");
			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			HWND child = ChildWindowFromPoint(wnd, pt);
			if (child==wnd)
			{
//				console::info("dbl 2");
				object_ptr p_obj;
				object_ptr p_obj_next;
				if (p_obj_base->find_by_divider_pt(pt, p_obj, p_obj_next))
				{
					//console::info("dbl 3");
					//extension_list_ptr p_list = p_obj->p_obj_parent;
					//if (p_list.is_valid())
					{
						//unsigned idx = p_list->m_objects.find_item(p_obj.get_ptr());
						//if (idx != -1)
						{
							//bool idx_plus = idx + 1 < p_list->m_objects.get_count();
							if (p_obj.is_valid() && p_obj->locked && (!p_obj_next.is_valid() || !p_obj_next->locked))
							{
								//console::info("dbl 2");
								//if (!idx_plus || !(p_list->m_objects.get_item(idx+1))->locked)
								{
									p_obj->hidden = !p_obj->hidden;
									p_obj->on_root_size();
								}
							}
							else if (p_obj_next.is_valid() && p_obj_next->locked && (!p_obj.is_valid() || !p_obj->locked))
							{
								p_obj_next->hidden = !p_obj_next->hidden;
								p_obj_next->on_root_size();
							}
						}
					}
				}
			}
		}
		
		break;
	case WM_MOUSEHOVER:
		{
			if (p_obj_base.is_valid())
			{
				object_ptr p_obj, p_obj2;
				POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
				p_obj_base->find_by_divider_pt(pt, p_obj, p_obj2);
					
				if (p_obj.is_valid())
				{
					extension_list_ptr p_list = p_obj->p_obj_parent;
					extension_list_ptr p_obj_list1;
					extension_list_ptr p_obj_list2;
					
					if (p_list.is_valid())
					{
						p_obj_list1 = p_obj;
						p_obj_list2 = p_obj2;
						
						if (p_list.is_valid() && (p_obj_list1.is_valid() && p_obj_list1->autohide && p_obj_list1->hidden)||(p_obj_list2.is_valid() && p_obj_list2->autohide && p_obj_list2->hidden))
						{
							extension_list_ptr p_list = p_obj_list1.is_valid() ? p_obj_list1->p_obj_parent : p_obj_list2->p_obj_parent;
							if (p_obj_list1.is_valid() && p_obj_list1->autohide) p_obj_list1->hidden = false;
							if (p_obj_list2.is_valid() && p_obj_list2->autohide) p_obj_list2->hidden = false;
							if (p_list->p_obj_parent.is_valid()) p_list->p_obj_parent->on_size();
							p_list->on_size();
							g_obj_mouse_list.add_item(new(std::nothrow) extension_list_pair(p_obj_list1, p_obj_list2));
//							if (p_obj_list1.is_valid() && p_obj_list1->autohide) p_obj_mouse1 = p_obj_list1;
//							if (p_obj_list2.is_valid() && p_obj_list2->autohide) p_obj_mouse2 = p_obj_list2;
							if (!hhk_mouse) hhk_mouse = SetWindowsHookEx(WH_MOUSE, MouseHookProc, 0, GetCurrentThreadId());
						}
					}
				}
			}
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
		{
			static LPARAM lp_last;
			static WPARAM wp_last;
			static UINT msg_last;
			static unsigned start_height;
			
			if (p_obj_base.is_valid() &&(msg_last != msg || lp_last != lp || wp_last != wp))
			{
				
				POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
				HWND child = RealChildWindowFromPoint(wnd, pt);
				if (child==wnd)
				{
					object_ptr p_obj, p_obj2;
					if (is_dragging && p_obj_dragging.is_valid())
						p_obj = p_obj_dragging;
					else
						p_obj_base->find_by_divider_pt(pt, p_obj, p_obj2);

					
					if (p_obj.is_valid())
					{ 
						extension_list_ptr p_list = p_obj->p_obj_parent;
						extension_list_ptr p_obj_list1;
						extension_list_ptr p_obj_list2;

						if (p_list.is_valid())
						{
							p_obj_list1 = p_obj;

							if (is_dragging && p_obj_dragging.is_valid())
							{

								unsigned idx = p_list->m_objects.find_item(p_obj.get_ptr());
								if (idx != -1)
								{
									bool idx_plus = idx + 1 < p_list->m_objects.get_count();
									if (idx_plus)
									{
										object_ptr ptemp = p_list->m_objects.get_item(idx+1);
										p_obj_list2 = ptemp;
									}
								}
							}
							else 
							{
								p_obj_list2 = p_obj2;
								//console::info(p_obj2.is_valid() ? "boom" : "moom1");
							}
						}

						if (p_list.is_valid() && msg == WM_MOUSEMOVE && (p_obj_list1.is_valid() && p_obj_list1->autohide )||(p_obj_list2.is_valid() && p_obj_list2->autohide))
						{
							if (cfg_sidebar_use_custom_show_delay && !cfg_sidebar_show_delay)
							{
								if ((p_obj_list1.is_valid() &&p_obj_list1->hidden) || (p_obj_list2.is_valid() &&p_obj_list2->hidden))
								{
									if (p_obj_list1.is_valid() && p_obj_list1->autohide) p_obj_list1->hidden = false;
									if (p_obj_list2.is_valid() && p_obj_list2->autohide) p_obj_list2->hidden = false;
									p_list->on_size();
									g_obj_mouse_list.add_item(new(std::nothrow) extension_list_pair(p_obj_list1, p_obj_list2));
//									if (p_obj_list1.is_valid() && p_obj_list1->autohide) p_obj_mouse1 = p_obj_list1;
//									if (p_obj_list2.is_valid() && p_obj_list2->autohide) p_obj_mouse2 = p_obj_list2;
									if (!hhk_mouse) hhk_mouse = uSetWindowsHookEx(WH_MOUSE, MouseHookProc, 0, GetCurrentThreadId());
								}
							}
							else
							{
								TRACKMOUSEEVENT tme;
								memset (&tme, 0, sizeof(TRACKMOUSEEVENT));
								tme.cbSize = sizeof(TRACKMOUSEEVENT);
								tme.dwFlags = TME_QUERY;
								tme.hwndTrack = wnd;
								_TrackMouseEvent(&tme);
								
								if (!(tme.dwFlags & TME_HOVER))
								{
						//			p_obj_mouse1 = p_obj_list1;
						//			p_obj_mouse2 = p_obj_list2;
									memset (&tme, 0, sizeof(TRACKMOUSEEVENT));
									tme.cbSize = sizeof(TRACKMOUSEEVENT);
									tme.hwndTrack = wnd;
									tme.dwHoverTime = cfg_sidebar_use_custom_show_delay ? cfg_sidebar_show_delay : HOVER_DEFAULT;
									tme.dwFlags = TME_HOVER;
									_TrackMouseEvent(&tme);
								}
							}
						
						}


						SetCursor(uLoadCursor(0, uMAKEINTRESOURCE(p_list.is_valid() && p_list->orientation == HORZ ? IDC_SIZEWE : IDC_SIZENS)));
						
						if (msg == WM_LBUTTONDOWN)
						{
							if (p_list.is_valid()) p_list->on_size(0, true);
							
							p_obj_dragging=p_obj;
							SetCapture(wnd);

							last_position = (p_list.is_valid() && p_list->orientation == VERT  ? pt.y : pt.x) ;
//							start_height = panels[dragging_panel]->height;
							is_dragging=true;
						}
					}
					else
					{
						if (!(wp & MK_LBUTTON)) SetCursor(uLoadCursor(0, uMAKEINTRESOURCE(IDC_ARROW)));
						p_obj_dragging=0;
					}
				}
				
				extension_list_ptr p_list = (p_obj_dragging.is_valid() && p_obj_dragging->p_obj_parent.is_valid() ? p_obj_dragging->p_obj_parent : 0);

				if (is_dragging && p_list.is_valid() && wp & MK_LBUTTON) 
				{

					int new_height = last_position - (p_list->orientation == VERT ? pt.y : pt.x);//pt.y - start_position;
					
				
					if (msg == WM_MOUSEMOVE && wp & MK_LBUTTON && p_obj_dragging.is_valid())
					{
						unsigned idx = p_list->m_objects.find_item(p_obj_dragging.get_ptr());
						if (idx != -1)
						{
							last_position = (p_list->orientation == VERT ? pt.y : pt.x) + p_list->override_size(idx, (p_list->orientation == VERT ? pt.y : pt.x) - last_position);
							p_list->on_size();
						}
					}
				}
			}
			msg_last = msg;
			lp_last = lp;
			wp_last = wp;
			
		}
		break;
	case MSG_AUTOHIDE_END:
		{
			if (g_obj_mouse_list.is_valid()/*p_obj_mouse1.is_valid() || p_obj_mouse2.is_valid()*/)
			{
				if (!cfg_sidebar_hide_delay)
				{
					POINT pt;
					GetMessagePos(&pt);
					g_obj_mouse_list.hide(pt);
//					g_sidebar_autohidden = true;
					/*
					extension_list_ptr p_list = p_obj_mouse1.is_valid() ? p_obj_mouse1->p_obj_parent : p_obj_mouse2->p_obj_parent;
					if (p_obj_mouse1.is_valid()) p_obj_mouse1->hidden = true;
					if (p_obj_mouse2.is_valid()) p_obj_mouse2->hidden = true;
					p_list->on_size();
					p_obj_mouse1.release();
					p_obj_mouse2.release();
					*/
				}
				else
				{
					SetTimer(wnd, HOST_AUTOHIDE_TIMER_ID, cfg_sidebar_hide_delay, 0);
				}
			}
			if (hhk_mouse && !g_obj_mouse_list.get_count()) UnhookWindowsHookEx(hhk_mouse);
			hhk_mouse=0;
		}
		break;
	case WM_TIMER:
		{
			if (wp == HOST_AUTOHIDE_TIMER_ID)
			{
				if (g_obj_mouse_list.is_valid())
				{
					POINT pt;
					GetMessagePos(&pt);

					if (g_obj_mouse_list.hide(pt))
					{
						if (hhk_mouse && !g_obj_mouse_list.get_count())
						{
							UnhookWindowsHookEx(hhk_mouse);
							hhk_mouse=0;
						}
					}

					else
					{
						if (!hhk_mouse && g_obj_mouse_list.get_count())
							hhk_mouse = uSetWindowsHookEx(WH_MOUSE, MouseHookProc, 0, GetCurrentThreadId());
					}
#if 0
					bool in_area1=false;
					bool in_area2=false;

					ScreenToClient(g_host_window->wnd_host, &pt);

					if (p_obj_mouse1.is_valid() && PtInRect(&p_obj_mouse1->rc_area, pt))
					{
						in_area1 = true;
					}
					if (p_obj_mouse2.is_valid() && PtInRect(&p_obj_mouse2->rc_area, pt))
					{
						in_area2 = true;
					}
					if (g_host_window->p_obj_base.is_valid())
					{
						object_ptr p_obj2 = 0;
						object_ptr p_obj1;
						g_host_window->p_obj_base->find_by_divider_pt(pt, p_obj1,p_obj2);
						if (p_obj1.is_valid() && p_obj1 == p_obj_mouse1.get_ptr()) in_area1 = true;
						if (p_obj2.is_valid() && p_obj2 == p_obj_mouse2.get_ptr()) in_area2 = true;
						
					}
					
					if (!in_area1 || !in_area2)
					{
	//					g_sidebar_autohidden = true;
						extension_list_ptr p_list = p_obj_mouse1.is_valid() ? p_obj_mouse1->p_obj_parent : p_obj_mouse2->p_obj_parent;
						if (p_obj_mouse1.is_valid()) p_obj_mouse1->hidden = true;
						if (p_obj_mouse2.is_valid()) p_obj_mouse2->hidden = true;
						p_list->on_size();
						p_obj_mouse1.release();
						p_obj_mouse2.release();
					}
					else
					{
						if (!hhk_mouse)
							hhk_mouse = uSetWindowsHookEx(WH_MOUSE, MouseHookProc, 0, GetCurrentThreadId());
					}
#endif
				}
				KillTimer(wnd, wp);
			}
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
			if (p_obj_base.is_valid() /*&& !(wp == SIZE_MINIMIZED) && lp*/)
			{
				HDWP dwp = BeginDeferWindowPos(p_obj_base->get_child_count());
				p_obj_base->set_area(0, 0, LOWORD(lp), HIWORD(lp));
				dwp = p_obj_base->on_size(dwp);
				EndDeferWindowPos(dwp);
			}
		}
		break;
/*	case WM_MENUSELECT:
		{
			if (HIWORD(wp) & MF_POPUP)
			{
				status_set_menu(false);
			}
			else 
			{
				unsigned id = LOWORD(wp);

				if (menu_ext_base && id < menu_ext_base)
				{

					bool set = false;

					if (id >= IDM_BASE)
					{
						extern pfc::string8 menudesc;
						set=true;
						menudesc.set_string("Toggle this panel; Modifiers: CTRL - Insert panel; SHIFT - Force new panel instance");
					}

					status_set_menu(set);
				}
			}
		}
		break;*/
	}
	return uDefWindowProc(wnd, msg, wp, lp);
}

// {F3E19309-2EA1-457a-A934-8069552765B2}
static const GUID host_guid = 
{ 0xf3e19309, 0x2ea1, 0x457a, { 0xa9, 0x34, 0x80, 0x69, 0x55, 0x27, 0x65, 0xb2 } };

class ui_ext_host : public ui_extension::window_host
{
public:
	virtual void get_name(pfc::string_base & out)const
	{
		out.set_string("Columns UI/Host");
	};

	virtual bool is_available()const
	{
		return false;
	}

	virtual unsigned get_supported_types()const
	{
		return ui_extension::type_toolbar|ui_extension::type_panel|ui_extension::type_layout|ui_extension::type_playlist;
	}

	virtual void insert_extension (const GUID & in, unsigned height, unsigned width)
	{
/*		if (g_sidebar_window)
		{
			extension_info * item = new(std::nothrow) extension_info(in, height);
			if (item)
			{
				unsigned idx = g_sidebar_window->panels.add_item(item);
				g_sidebar_window->refresh_panels();
				g_sidebar_window->size_panels(true);
				g_sidebar_window->override_size(idx, height-g_sidebar_window->panels[idx]->height);
			}
		}*/
	};

	virtual void insert_extension (ui_extension::window_ptr & p_ext, unsigned height, unsigned width)
	{
/*		if (g_sidebar_window)
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
		}*/
	};

	virtual const GUID & get_host_guid()const{return host_guid;}


	virtual void on_size_limit_change(HWND wnd, unsigned flags)
	{
		if (g_host_window) g_host_window->on_size_limit_change(wnd, flags);
	};

	
	virtual bool override_status_text_create(service_ptr_t<ui_status_text_override> & p_out)
	{
		static_api_ptr_t<ui_control> api;
		return api->override_status_text_create(p_out);
	}

	virtual unsigned is_resize_supported(HWND wnd)const
	{
		return 0;//ui_extension::size_height;
	}

	virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height)
	{
		return (g_host_window && g_host_window->request_resize(wnd, flags, width, height));
	}
	virtual bool is_visible(HWND wnd)const
	{
		return (g_host_window && g_host_window->is_visible(wnd));
	}
	virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility)const
	{
		return (g_host_window && g_host_window->is_visibility_modifiable(wnd, desired_visibility));
	}
	virtual bool set_window_visibility(HWND wnd, bool visibility)
	{
		return (g_host_window && g_host_window->set_window_visibility(wnd, visibility));
	}
	void find_by_wnd(HWND wnd)
	{
	}
	virtual void relinquish_ownership(HWND wnd)
	{
		if (g_host_window)  g_host_window->relinquish_ownership(wnd);
	}
	
};

ui_extension::window_host_factory_single<ui_ext_host> g_ui_ext_host;



bool extension::refresh(HWND wnd)
{
	bool rv = false;
	if (!wnd_host)
	{
		//			ui_extension * p_ext = panels[n]->p_ext;
		
		bool b_new = false;
		if (p_ext.is_empty()) 
		{
			ui_extension::window::create_by_guid(guid,p_ext);
			b_new = true;
		}
		
		if (p_ext.is_valid() && p_ext->is_available(ui_extension::window_host_ptr(&g_ui_ext_host.get_static_instance())))
		{
			pfc::string8 name;
			if (!p_ext->get_short_name(name))
				p_ext->get_name(name);
			
			wnd_host = CreateWindowEx(WS_EX_CONTROLPARENT, host_window::panel_host_class_name, pfc::stringcvt::string_os_from_utf8(name),
				WS_CHILD|WS_CLIPCHILDREN, 0, 0, 0, 0, wnd, 0, core_api::get_my_instance(), this);
			
			if (wnd_host)
			{
				if (b_new)
				{
					try {
					p_ext->set_config(&stream_reader_memblock_ref(config.get_ptr(), config.get_size()),config.get_size(),abort_callback_impl());
					}
					catch (exception_io e)
					{
						console::formatter() << "Error setting panel config: " << e.what();
					}
				}
				wnd_panel = p_ext->create_or_transfer_window(wnd_host, ui_extension::window_host_ptr(&g_ui_ext_host.get_static_instance()));
				if (wnd_panel)
				{
					//						uSetWindowLong(wnd_host, GWL_USERDATA, (long)this);
					
					MINMAXINFO mmi;
					memset(&mmi, 0, sizeof(MINMAXINFO));
					mmi.ptMaxTrackSize.x = MAXLONG;
					mmi.ptMaxTrackSize.y = MAXLONG;
					SendMessage(wnd_panel, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
					
					min_height = mmi.ptMinTrackSize.y;
					min_width = mmi.ptMinTrackSize.x;
					max_width = mmi.ptMaxTrackSize.x;
					max_height = mmi.ptMaxTrackSize.y;

//					b_need_show = true;
					
//					ShowWindow(wnd_host, SW_SHOWNORMAL);
//					ShowWindow(wnd_panel, SW_SHOWNORMAL);
					rv = true;
				}
				else
				{
					DestroyWindow(wnd_host);
					p_ext.release();
				}
			}
			else
			{
				p_ext.release();
			}
		}
		else
		{
			p_ext.release();
		}
	}
	return rv;
}

void extension::on_wm_size(int width, int height)
{
	unsigned caption_height = 0;
	HWND wnd_panel = GetWindow(wnd_host, GW_CHILD);//; 
	if (wnd_panel)
	{
		
		if (show_caption)
		{
			if (wnd_panel) 
			{
				caption_height = GetCaptionHeight();
			}
		}
		if (orientation == VERT)
			SetWindowPos(wnd_panel, 0, 0, caption_height, width, height-caption_height, SWP_NOZORDER);
		else
			SetWindowPos(wnd_panel, 0, caption_height, 0, width-caption_height, height, SWP_NOZORDER);
		
		if (caption_height && g_uxtheme.is_valid() && g_uxtheme->IsThemeActive ())
		{
			RECT rc_caption = {0, 0, orientation == VERT ? width:GetCaptionHeight(), orientation == VERT ?GetCaptionHeight():height};//SM_CYSMCAPTION
			RedrawWindow(wnd_host, &rc_caption, 0, RDW_INVALIDATE|RDW_UPDATENOW);
		}

//		if (b_need_show)
//		{
//			ShowWindow(wnd_host, SW_SHOWNORMAL);
//			ShowWindow(wnd_panel, SW_SHOWNORMAL);
//			b_need_show=false;
//		}		
	}
	
}

void extension::show_window()
{
	if (wnd_host) ShowWindow(wnd_host, SW_SHOWNORMAL);
	if (wnd_panel) ShowWindow(wnd_panel, SW_SHOWNORMAL);
}

LRESULT WINAPI extension::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{

	switch(msg)
	{
	case WM_CREATE:
		{
			if (g_uxtheme.is_valid())
			{
				uSetWindowLong(wnd, 0, g_uxtheme->IsThemeActive() ? (LPARAM)g_uxtheme->OpenThemeData(wnd, L"Rebar") : 0);
			}
		}
		break;
	case WM_THEMECHANGED:
		{
			if (g_uxtheme.is_valid())
			{
				HTHEME theme = (HTHEME)uGetWindowLong(wnd, 0);
				if (theme) g_uxtheme->CloseThemeData(theme);
				uSetWindowLong(wnd, 0, g_uxtheme->IsThemeActive() ? (LPARAM)g_uxtheme->OpenThemeData(wnd, L"Rebar") : 0);
			}
		}
		break;
	case WM_DESTROY:
		{
			if (g_uxtheme.is_valid())
			{
				HTHEME theme = (HTHEME)uGetWindowLong(wnd, 0);
				if (theme) g_uxtheme->CloseThemeData(theme);
				uSetWindowLong(wnd, 0, 0);
			}
		}
		break;
	case WM_PAINT:
		{
				
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(wnd, &ps);
				if (show_caption)
				{
				
					RECT rc_client, rc_dummy;
					GetClientRect(wnd, &rc_client);
					
					RECT rc_caption = {0, 0, orientation == VERT ? rc_client.right:GetCaptionHeight(), orientation == VERT ?GetCaptionHeight():rc_client.bottom};//SM_CYSMCAPTION
					
					if (IntersectRect(&rc_dummy, &rc_client, &rc_caption))
					{
						RECT rc_vert = {0, 0, rc_client.bottom, GetCaptionHeight()};
						if (g_uxtheme.is_valid())
						{
							if (g_uxtheme->IsThemeActive())
							{
								bool world = 0;//orientation != VERT;
								//	POINT pt;
/*								if (world)
								{
									//OffsetWindowOrgEx(dc, 0, 0-rc_client.bottom, &pt);

									SetGraphicsMode(dc, GM_ADVANCED);

										XFORM xf;
										xf.eM11 = (FLOAT) 1; 
										xf.eM12 = (FLOAT) 0; 
										xf.eM21 = (FLOAT) 0; 
										xf.eM22 = (FLOAT) 1; 
										xf.eDx  = (FLOAT) 0.0; 
										xf.eDy  = (FLOAT) 0.0; 
										SetWorldTransform(dc, &xf);
									
								}*/
								HTHEME theme = (HTHEME)uGetWindowLong(wnd, 0);
								if (theme)
									g_uxtheme->DrawThemeBackground(theme, dc, 0, 0, 0 ? & rc_vert : &rc_caption, 0);
//								if (world)
//								{
//								ModifyWorldTransform(dc, 0, MWT_IDENTITY);
//								SetGraphicsMode(dc, GM_COMPATIBLE);
//								}

						//		if (world) SetWindowOrgEx(dc, pt.x, pt.y, NULL);
							}
						}
						
						//			DrawCaption(wnd, dc, &rc_caption, DC_TEXT|DC_SMALLCAP|(GetActiveWindow() == g_main_window ? DC_ACTIVE : 0)|DC_GRADIENT);
						pfc::string8 text;
						uGetWindowText(wnd, text);
						if (!g_menu_font) g_menu_font = uCreateMenuFont();
						if (!g_menu_font_vert) g_menu_font_vert = uCreateMenuFont(true);
						
#if 0
#endif
						
						HFONT old = SelectFont(dc, (orientation != VERT) ? g_menu_font_vert : g_menu_font);
						uDrawPanelTitle(dc, false ? &rc_vert : &rc_caption, text, text.length(), !false && orientation != VERT, false);
						SelectFont(dc, old);
						
					}
				
				}
		
			EndPaint(wnd, &ps);
		}
		return 0;
	case WM_SIZE:
		{
//			if (!(wp == SIZE_MINIMIZED))
			on_wm_size(LOWORD(lp), HIWORD(lp));
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			if (p_obj_parent.is_valid()/* && p_obj_parent->get_type() == TYPE_EXTENSION_LIST*/)
			{
				extension_list_ptr p_obj_list = (p_obj_parent);
				
				POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
				if (ChildWindowFromPoint(wnd, pt) == wnd)
				{
					hidden = !hidden;
					while (p_obj_list->p_obj_parent.is_valid())
					{
						p_obj_list = p_obj_list->p_obj_parent;
					}
					p_obj_list->on_size();
				}
			}
		}
		return 0;
	case WM_SYSCOLORCHANGE:
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_SETTINGCHANGE:
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_CONTEXTMENU:
		{
			enum {IDM_CLOSE=1, IDM_MOVE_UP, IDM_MOVE_DOWN, IDM_LOCK, IDM_CAPTION, IDM_BASE};

			
			POINT pt;
			GetMessagePos(&pt);
			
			POINT pt_client = pt;
			
			ScreenToClient(wnd, &pt_client);
			
			HMENU menu = CreatePopupMenu();
			
			unsigned IDM_EXT_BASE=IDM_BASE;
			
			HWND child = RealChildWindowFromPoint(wnd, pt_client);
			
			
			if (p_ext.is_valid()) 
			{
				uAppendMenu(menu,(MF_STRING | (show_caption ? MF_CHECKED : 0) ),IDM_CAPTION,"Show &caption");
				uAppendMenu(menu,(MF_STRING | (locked ? MF_CHECKED : 0) ),IDM_LOCK,"&Lock panel");
				if (object::p_obj_parent.is_valid() && object::p_obj_parent->get_type() == TYPE_EXTENSION_LIST)
				{
					uAppendMenu(menu,(MF_SEPARATOR),0,"");
					uAppendMenu(menu,(MF_STRING),IDM_MOVE_UP,"Move &up");
					uAppendMenu(menu,(MF_STRING),IDM_MOVE_DOWN,"Move &down");
					uAppendMenu(menu,(MF_STRING),IDM_CLOSE,"&Close panel");
				}
			}
			
			void * user_data = 0;
			
			pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> extension_menu_nodes = new ui_extension::menu_hook_impl;

			if (p_ext.is_valid()) 
			{
//				p_ext->build_menu(menu, IDM_EXT_BASE, pt, true, user_data); 
				p_ext->get_menu_items(*extension_menu_nodes.get_ptr()); 
				if (extension_menu_nodes->get_children_count() > 0)
					uAppendMenu(menu,MF_SEPARATOR,0,0);
					
				extension_menu_nodes->win32_build_menu(menu, IDM_EXT_BASE, infinite - IDM_EXT_BASE);
			}
			menu_helpers::win32_auto_mnemonics(menu);
			
//			menu_ext_base = IDM_EXT_BASE;
			
			int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);
			
//			menu_ext_base=0;
			
			if (cmd >= IDM_EXT_BASE)
			{
				extension_menu_nodes->execute_by_id(cmd);
			}
			
			DestroyMenu(menu);
			
			if (cmd == IDM_CLOSE && p_ext.is_valid() && p_obj_parent.is_valid())
			{
				destroy();
				if (p_obj_parent.is_valid() && p_obj_parent->get_type() == TYPE_EXTENSION_LIST)
				{
					extension_list_ptr p_obj_list = ((p_obj_parent));
					p_obj_list->m_objects.remove_item(this);
					p_obj_list->on_size();
				}
			}
			else if (cmd == IDM_MOVE_UP)
			{
				if (p_obj_parent.query_extension_list())
				{
					extension_list_ptr p_obj_list = ((p_obj_parent));
					int item = p_obj_list->m_objects.find_item(this);
					if (item != -1)
					{
						p_obj_list->move_up(item);
						p_obj_list->on_size();
					}
				}
			}
			else if (cmd == IDM_MOVE_DOWN)
			{
				if (p_obj_parent.is_valid() && p_obj_parent->get_type() == TYPE_EXTENSION_LIST)
				{
					extension_list_ptr p_obj_list = ((p_obj_parent));
					int item = p_obj_list->m_objects.find_item(this);
					if (item != -1)
					{
						p_obj_list->move_down(item);
						p_obj_list->on_size();
					}
				}
		//		size_panels();
			}
			else if (cmd == IDM_LOCK)
			{
				if (p_obj_parent.is_valid() && p_obj_parent->get_type() == TYPE_EXTENSION_LIST)
				{
					extension_list_ptr p_obj_list = ((p_obj_parent));
					p_obj_list->on_size(0, true);
				}
			
				locked = locked == 0;
			}
			else if (cmd == IDM_CAPTION)
			{
				show_caption = show_caption == 0;
//				size_panels();
//				g_on_size(panels[panel]->wnd_host);
//				g_on_wm_size();
				if (p_obj_parent.is_valid() && p_obj_parent->get_type() == TYPE_EXTENSION_LIST)
				{
					extension_list_ptr p_obj_list = ((p_obj_parent));
					p_obj_list->on_size();
				}
				g_on_wm_size();
			}
		}
		return 0;
	}
	return uDefWindowProc(wnd, msg, wp, lp);
}

void host_window::show_window()
{
	if (p_obj_base.is_valid()) p_obj_base->show_window();
	if (wnd_host) ShowWindow(wnd_host, SW_SHOWNORMAL);
}

LRESULT WINAPI host_window::panel_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	extension * p_this = 0;
	if(msg == WM_NCCREATE)
	{
		p_this = reinterpret_cast<extension*>(((CREATESTRUCT *)(lp))->lpCreateParams);
		uSetWindowLong(wnd, GWL_USERDATA, (LPARAM)p_this);//store it for future use
	}
	else
		p_this = reinterpret_cast<extension*>(uGetWindowLong(wnd,GWL_USERDATA));

	return p_this ? p_this->on_message(wnd, msg, wp, lp) : uDefWindowProc(wnd, msg, wp, lp);
}





bool host_window::focus_playlist_window()
{
	return (p_obj_base.is_valid() && p_obj_base->focus_playlist_window());
}

#endif