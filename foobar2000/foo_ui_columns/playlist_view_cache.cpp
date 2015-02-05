#include "foo_ui_columns.h"

playlist_view_cache playlist_view::g_cache;

void playlist_view_cache::on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len)
{
	flush_columns(p_index);
	flush(p_index);
}

#if 0
struct colour_map
{
	unsigned byte_index;
	COLORREF colour;
};

class titleformat_text_filter_impl_colour : public titleformat_text_filter
{
public:
	titleformat_text_filter_impl_colour() : m_length(0) {};
	virtual void on_new_field () {};
	virtual void write (titleformat_text_out * p_out,const char * p_data,unsigned p_data_length){};
	unsigned get_position () {return m_length;}
private:
	unsigned m_length;
};


class titleformat_hook_colourc : public titleformat_hook
{
	const SYSTEMTIME * p_st;
	pfc::array_t<char> year,month,day,dayofweek,hour,julian;
public:
	virtual bool process_field(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,bool & p_found_flag);
	virtual bool process_function(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag);
	inline titleformat_hook_colourc(const SYSTEMTIME * st = 0) : p_st(st), year(0),month(0),day(0),dayofweek(0),hour(0),julian(0) 
	{
	};
};
#endif

void playlist_view_cache::on_items_added(unsigned p_playlist,unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection)
{
	flush(p_playlist);
	flush_sort(p_playlist);
//	playlist_cache * p_cache = get_item(p_playlist);
//	p_cache->insert_items_repeat(0, p_data.get_count(), start);
}

void playlist_view_cache::on_items_reordered(unsigned p_playlist,const unsigned * order,unsigned count)
{
//	playlist_cache * p_cache = get_item(p_playlist);
//	assert(p_cache->get_count() == count);
//	p_cache->reorder(order);
	flush_sort(p_playlist);
	unsigned n,start=0;
	for(n=0;n<count;n++)
	{
		start=n;
		while (n<count && order[n]!=n)
		{
			n++;
		}
		if (n>start) on_items_change(p_playlist, bit_array_range(start, n));
	}
}

void playlist_view_cache::on_items_removed(unsigned p_playlist,const bit_array & mask)
{
	flush(p_playlist);
	flush_sort(p_playlist);
//	playlist_cache * p_cache = get_item(p_playlist);
//	p_cache->remove_mask(mask);
}

void playlist_view_cache::on_items_change(unsigned p_playlist, const bit_array & p_mask)
{
	playlist_cache * p_cache = get_item(p_playlist);
	unsigned n, count = p_cache->get_count();
	for(n=0; n<count;n++)
	{
		if (p_mask[n])
		{
			playlist_entry_ui * entry = p_cache->get_item(n);
			if (entry) 
			{
				delete entry;
				p_cache->replace_item(n, 0);
			}
		}
	}
}

void playlist_view_cache::on_items_modified(unsigned p_playlist, const bit_array & p_mask)
{
	flush_sort(p_playlist);
	on_items_change(p_playlist, p_mask);
}

void playlist_view_cache::on_items_modified_fromplayback(unsigned p_playlist,const bit_array & p_mask,play_control::t_display_level p_level)
{
	on_items_change(p_playlist, p_mask);
}


void process_colour_string(const char * src, colourinfo & out);

bool playlist_cache::is_valid(unsigned idx)
{
	if (idx >= 0 && idx < get_count())
	{
		return true;
	}
	return false;
}

bool playlist_view_cache::is_in_playlist(unsigned playlist, unsigned idx)
{
	unsigned count = static_api_ptr_t<playlist_manager>()->playlist_get_item_count(playlist);
	return (idx >= 0 && idx < count);
}

void playlist_view_cache::get_display_name(unsigned playlist, unsigned idx, int col, pfc::string_base & out)
{	
	playlist_cache * p_cache = get_item(playlist);
	if (idx >= 0 && idx < p_cache->get_count())
	{
		if (!p_cache->get_item(idx))
		{
			if (!update_item(playlist, idx)) {out.set_string("Error"); return;}
		}
		p_cache->get_item(idx)->get_item(col)->get_display(out); return;
	}
	out.set_string("Internal error - invalid playlist entry!");
}

COLORREF playlist_view_cache::get_colour(unsigned playlist, unsigned idx, int col, colour_type colour)
{	
	playlist_cache * p_cache = get_item(playlist);
	if (idx >= 0 && idx < p_cache->get_count())
	{
		if (!p_cache->get_item(idx))
		{
			if (!update_item(playlist, idx)) return 0x000000FF;
		}
		return p_cache->get_item(idx)->get_item(col)->get_colour(colour);
	}
	return 0x000000FF;
}

void playlist_view_cache::get_colour(unsigned playlist, unsigned idx, int col, colourinfo & out)
{	
	playlist_cache * p_cache = get_item(playlist);
	if (idx >= 0 && idx < p_cache->get_count())
	{
		if (!p_cache->get_item(idx))
		{
			if (!update_item(playlist, idx)) return;
		}
		p_cache->get_item(idx)->get_item(col)->get_colour(out);
	}
}

void playlist_cache::delete_all()
{	
//	if (synctest) console::error("oh dear 3");

	int n,t=get_count();
	for(n=0; n<t; n++)
	{
		playlist_entry_ui * entry = get_item(n);
		if (entry) {delete entry;}
	}
	
	remove_all();
}

/*
void playlist_cache::rebuild_all()
{	
	delete_all();
	
	int n,count = static_api_ptr_t<playlist_manager>()->activeplaylist_get_item_count();
	for (n=0;n<count;n++)
		add_item(n);
}
*/
playlist_cache::~playlist_cache()
{
	delete_all();
}








void playlist_view_cache::make_extra(unsigned playlist, unsigned idx, global_variable_list & p_out, const SYSTEMTIME * st, bool b_legacy) const
{
	static_api_ptr_t<playlist_manager> playlist_api;
		
	pfc::string8_fast_aggressive extra_formatted;
	extra_formatted.prealloc(512);
	
	if (!g_to_global.is_valid())
	{
		static_api_ptr_t<titleformat_compiler>()->compile_safe(g_to_global, cfg_globalstring);
	}
	
	//watchout, might not be active playlist
	playlist_api->playlist_item_format_title(playlist, idx, &titleformat_hook_splitter_pt3(&titleformat_hook_set_global<true,false>(p_out,b_legacy), st ? &titleformat_hook_date(st) : 0, &titleformat_hook_playlist_name()), extra_formatted, g_to_global, 0, play_control::display_level_all);
	
	
	//	if (map_codes) extra_formatted.replace_char(3, 6);
	
	if (b_legacy)
	{
		const char * ptr = extra_formatted;
		
		const char * start = ptr;
		
		while(*ptr)
		{
			start = ptr;
			while (*ptr && *ptr != '\x07') ptr++;
			if (ptr > start)
			{
				const char * p_equal = strchr_n(start+1, '=', ptr-start-1);
				if (p_equal)
				{
					if (ptr-p_equal-1)
						p_out.add_item(start,p_equal-start,p_equal+1,ptr-p_equal-1);
//					console::info(pfc::string_simple(start,p_equal-start));
				}
			}
			while (*ptr && *ptr == '\x07') ptr++;
		}
	}

}






bool playlist_view_cache::update_item(unsigned playlist, unsigned idx)
{
	bool rv = false;
	playlist_cache * p_cache = get_item(playlist);
	if (idx >= 0 && idx < p_cache->get_count())
	{
		column_list_cref_t columns = playlist_view::g_get_columns();
		const bit_array & p_mask = get_columns_mask(playlist);

		playlist_entry_ui * entry = p_cache->get_item(idx);

		static_api_ptr_t<playlist_manager> playlist_api;

		if (!entry) 
		{
			entry = new(std::nothrow) playlist_entry_ui; 
			if (!entry) return false;
			
			unsigned e=columns.get_count();
			entry->add_display_items(column_get_active_count(playlist));
			p_cache->replace_item(idx, entry);
		}

		bool date = cfg_playlist_date != 0;

		SYSTEMTIME st;
		if (date) GetLocalTime(&st);

		bool global = (cfg_global != 0);
		bool b_legacy = cfg_oldglobal != 0;

		global_variable_list p_vars;

		if (global)
		{
			make_extra(playlist, idx, p_vars, date ? &st : 0, b_legacy);
		}
	
		int s,e=columns.get_count(),i=0;
		pfc::string8_fast_aggressive colour,display,temp;
		colour.prealloc(512);
		display.prealloc(512);
		temp.prealloc(512);

		cui::colours::helper p_helper(appearance_client_pv_impl::g_guid);

		colourinfo colours_global(
			p_helper.get_colour(cui::colours::colour_text),
			p_helper.get_colour(cui::colours::colour_selection_text),
			p_helper.get_colour(cui::colours::colour_background),
			p_helper.get_colour(cui::colours::colour_selection_background),
			p_helper.get_colour(cui::colours::colour_inactive_selection_text),
			p_helper.get_colour(cui::colours::colour_inactive_selection_background)
			);

		service_ptr_t<titleformat_object> to_display;
		service_ptr_t<titleformat_object> to_colour;
		service_ptr_t<titleformat_object> to_global_style;

		bool colour_global_av = false;
		for (s=0;s<e;s++)
		{
			if (p_mask[s])
			{

				playlist_view::g_get_columns()[s]->get_to_display(to_display);
				//playlist_view::g_get_titleformat_object(s, STRING_DISPLAY, to_display);
				if (to_display.is_valid())
					playlist_api->playlist_item_format_title(playlist, idx, &titleformat_hook_splitter_pt3(global ? &titleformat_hook_set_global<false,true>(p_vars, b_legacy) :0, date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), display, to_display, 0, play_control::display_level_all);

				colourinfo col_item(
					p_helper.get_colour(cui::colours::colour_text),
					p_helper.get_colour(cui::colours::colour_selection_text),
					p_helper.get_colour(cui::colours::colour_background),
					p_helper.get_colour(cui::colours::colour_selection_background),
					p_helper.get_colour(cui::colours::colour_inactive_selection_text),
					p_helper.get_colour(cui::colours::colour_inactive_selection_background)
					);

				playlist_view::g_get_columns()[s]->get_to_colour(to_colour);
				//playlist_view::g_get_titleformat_object(s, STRING_COLOUR, to_colour);

				bool b_custom = columns[s]->use_custom_colour;

				if (!cfg_oldglobal || !b_custom)
				{
					if (!colour_global_av)
					{
						playlist_view::g_get_global_style_titleformat_object(to_global_style);
						if (to_global_style.is_valid())
							playlist_api->playlist_item_format_title(playlist, idx, 
								&titleformat_hook_splitter_pt3(&titleformat_hook_style(colours_global),global ? &titleformat_hook_set_global<false,true>(p_vars, b_legacy) : 0,date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), colour, to_global_style, 0, play_control::display_level_all);

						if (cfg_oldglobal && !colour.is_empty())
							process_colour_string(colour, colours_global);
						colour_global_av = true;
					}
					col_item = colours_global;
				}
				if (b_custom)
				{
					if (to_colour.is_valid())
						playlist_api->playlist_item_format_title(playlist, idx, 
							&titleformat_hook_splitter_pt3(&titleformat_hook_style(col_item),global ? &titleformat_hook_set_global<false,true>(p_vars, b_legacy) : 0,date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()) 
							, colour, to_colour, 0, play_control::display_level_all);
					
					
					if (cfg_oldglobal && !colour.is_empty())
						process_colour_string(colour, col_item);
				}

				entry->set_display_item(i++, display, col_item);
			}


		}
		rv = true;
	}
	return rv;
}

const bit_array & playlist_view_cache::get_columns_mask(unsigned playlist)
{
	playlist_cache * p_cache = get_item(playlist);
	if (!p_cache->m_active_columns_valid)
	{
		update_active_columns(playlist);
	}
	return p_cache->m_active_columns;
}

unsigned playlist_view_cache::column_active_to_actual(unsigned playlist, unsigned column)
{
	const bit_array & p_array = get_columns_mask(playlist);
	unsigned n, rv=0, count = playlist_view::g_get_columns().get_count();
	for (n=0; n<count;n++)
	{
		if (p_array[n]) 
		{
			if (rv++ == column) return n;
		}
	}
	return rv;
}

unsigned playlist_view_cache::column_get_active_count(unsigned playlist)
{
	const bit_array & p_array = get_columns_mask(playlist);
	unsigned n, rv=0, count = playlist_view::g_get_columns().get_count();
	for (n=0; n<count;n++)
	{
		if (p_array[n]) rv++;
	}
	return rv;
}

void playlist_view_cache::update_active_columns(unsigned playlist)
{
	pfc::string8 playlist_name;
	static_api_ptr_t<playlist_manager> playlist_api;

	playlist_api->playlist_get_name(playlist, playlist_name);
		
	column_list_cref_t columns = playlist_view::g_get_columns();
	playlist_cache * p_cache = get_item(playlist);

	int s,e=columns.get_count();
	p_cache->m_active_columns.resize(e);
	for (s=0;s<e;s++)
	{
		bool b_valid = false;
		if (columns[s]->show)
		{
			switch(columns[s]->filter_type)
			{
			case FILTER_NONE:
				{
					b_valid = true;
					break;
				}
			case FILTER_SHOW:
				{
					if (wildcard_helper::test(playlist_name,columns[s]->filter,true))
						b_valid = true;
				}
				break;
			case FILTER_HIDE:
				{
					if (!wildcard_helper::test(playlist_name,columns[s]->filter,true))
						b_valid = true;
				}
				break;
			}
		}
		p_cache->m_active_columns.set(s, b_valid);
	}
	p_cache->m_active_columns_valid = true;
}

void playlist_view::g_load_columns()
{
	columns.set_entries_copy(g_columns);
}

void playlist_view::g_reset_columns()
{
	g_load_columns();
	g_cache.flush_all();
}


unsigned int playlist_view::g_columns_get_total_width()
{	
	const bit_array & p_mask = g_cache.active_get_columns_mask();
	unsigned w=0,n,t = columns.get_count();
	for (n=0;n<t;n++) if (p_mask[n]) w += columns[n]->width;
	return w;
}

unsigned int playlist_view::g_columns_get_total_parts()
{	
	const bit_array & p_mask = g_cache.active_get_columns_mask();
	unsigned w=0,n,t = columns.get_count();
	for (n=0;n<t;n++) if (p_mask[n]) w += columns[n]->parts;
	return w;
}

unsigned playlist_view::g_columns_get_width(unsigned column)
{	
	return columns[g_cache.active_column_active_to_actual(column)]->width;
}

unsigned playlist_view::get_columns_total_width() const
{	
	pfc::array_t<int, pfc::alloc_fast_aggressive> widths;
	widths.prealloc(g_cache.active_column_get_active_count());
	return get_column_widths(widths);
}

unsigned playlist_view::get_column_width(unsigned column1) const
{	
	pfc::array_t<int, pfc::alloc_fast_aggressive> widths;
	widths.prealloc(g_cache.active_column_get_active_count());
	get_column_widths(widths);
//	unsigned column = g_cache.active_column_active_to_actual(column1);
	int rv = 0;
	if (column1 >= 0 && column1 < widths.get_size()) rv = widths[column1];

	return rv;
}



void playlist_view_cache::flush(unsigned playlist)
{
	playlist_cache * p_cache = get_item(playlist);
	p_cache->delete_all();
	
	unsigned n,count = static_api_ptr_t<playlist_manager>()->playlist_get_item_count(playlist);
	for (n=0;n<count;n++)
		p_cache->add_item(n);
}

void playlist_view_cache::flush_columns(unsigned playlist)
{
	//could change so doesnt do anythng if not using playlist filters
	playlist_cache * p_cache = get_item(playlist);
	p_cache->m_active_columns.resize(0);
	p_cache->m_active_columns_valid = 0;
	flush_sort(playlist);
}

void playlist_view_cache::flush_sort(unsigned playlist)
{
	playlist_cache * p_cache = get_item(playlist);
	p_cache->m_sorted = false;
}

bool playlist_view_cache::get_playlist_sort(unsigned playlist, unsigned & idx, bool * p_descending)
{
	playlist_cache * p_cache = get_item(playlist);
	if (p_descending) *p_descending = p_cache->m_sorted_descending;
	idx = p_cache->m_sorted_column;
	return p_cache->m_sorted;
}

void playlist_view_cache::set_playlist_sort(unsigned playlist, unsigned column, bool descending)
{
	playlist_cache * p_cache = get_item(playlist);
	p_cache->m_sorted_descending = descending;
	p_cache->m_sorted = true;
	p_cache->m_sorted_column = column;
}

struct sort_info
{
	HANDLE text;
	int index;
	sort_info(const char * p_text,int p_idx)
		: text(uSortStringCreate(p_text)), index(p_idx)
	{}
	~sort_info() {uSortStringFree(text);}
};

template<class T>
class sort_info_callback_base : public pfc::list_base_t<T>
{
public:
	class asc_sort_callback : public sort_callback
	{
		virtual int compare(const T& p_item1,const T& p_item2)
		{
			int temp = uSortStringCompare(p_item1->text,p_item2->text);
			if (temp==0) temp = (p_item1->index > p_item2->index ? 1 : -1);
			return temp;
		}
	};
	class desc_sort_callback : public sort_callback
	{
		virtual int compare(const T &elem1, const T &elem2 )
		{
			int temp = uSortStringCompare(elem1->text,elem2->text)*-1;
			if (temp==0) temp = (elem1->index > elem2->index ? 1 : -1); //we could swap -1 and 1 for desc, but then, apart from not making any sense, same sort repeated would yield diff results
			return temp;
		}
	};
};



void playlist_view::g_remove_sort()
{
	bool b_sorted;
	unsigned idx;
	b_sorted = g_cache.active_get_playlist_sort(idx);

	if (b_sorted)
	{
		unsigned n, pcount = playlist_view::list_playlist.get_count();
		for (n=0; n<pcount; n++)
		{
			playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
			if (p_playlist->wnd_header)
			{
				HDITEM hdi;
				memset(&hdi, 0, sizeof(hdi));

				hdi.mask = HDI_FORMAT;
				Header_GetItem(p_playlist->wnd_header, idx, &hdi);

				/* FIX */
				if (hdi.fmt & HDF_IMAGE) hdi.fmt ^= HDF_IMAGE;
				if (hdi.fmt & HDF_BITMAP_ON_RIGHT) hdi.fmt ^= HDF_BITMAP_ON_RIGHT;
				if (hdi.fmt & 0x0200) hdi.fmt ^= 0x0200;
				if (hdi.fmt & 0x0400) hdi.fmt ^= 0x0400;

			//	hdi.fmt ^= HDF_IMAGE|HDF_BITMAP_ON_RIGHT|0x0200|0x0400;
				Header_SetItem(p_playlist->wnd_header, idx, &hdi);
			}
		}
	}
}

void playlist_view::g_set_sort( unsigned column, bool descending, bool selection_only)
{
	static_api_ptr_t<playlist_manager> playlist_api;

	unsigned active_playlist = playlist_api->get_active_playlist();
	if (active_playlist != -1 && (!playlist_api->playlist_lock_is_present(active_playlist) || !(playlist_api->playlist_lock_get_filter_mask(active_playlist) & playlist_lock::filter_reorder)))
	{

		unsigned act_column = g_cache.active_column_active_to_actual(column);

	//	const columns_class & columns = playlist_view::g_get_columns();

		unsigned n,count = playlist_api->activeplaylist_get_item_count();

		pfc::array_t<unsigned> order;
		
		order.set_size(count);

		pfc::ptr_list_t<sort_info> data;

		pfc::string8_fast_aggressive temp;
		pfc::string8_fast_aggressive temp2;
		temp.prealloc(512);

		pfc::string8 spec;
		bool custom_sort = columns[act_column]->use_custom_sort;

		/*if (custom_sort) */spec = custom_sort ? columns[act_column]->sort_spec : columns[act_column]->spec;
		//columns.get_string(act_column, spec, custom_sort ? STRING_SORT : STRING_DISPLAY);


		global_variable_list extra_items;
		bool extra = (cfg_global_sort != 0);
		bool b_legacy = cfg_oldglobal != 0;

		bit_array_bittable mask(count);
		if (selection_only) playlist_api->activeplaylist_get_selection_mask(mask);

		service_ptr_t<titleformat_object> to_sort;

		static_api_ptr_t<titleformat_compiler> titleformat_api;
		titleformat_api->compile_safe(to_sort, spec);
		bool date = cfg_playlist_date != 0;
		SYSTEMTIME st;
		if (date) GetLocalTime(&st);

		for(n=0;n<count;n++)
		{

			if (!selection_only || mask[n] == true)
			{

		//		if (custom_sort)
				{

				if (extra)
				{
					extra_items.delete_all();
					g_cache.active_make_extra(n, extra_items, date ? &st : 0, b_legacy);
				}

		//		g_oper->format_title(n, temp,spec,(extra ? extra_items.get_ptr() : 0));

				playlist_api->activeplaylist_item_format_title(n, &titleformat_hook_splitter_pt3(extra ? &titleformat_hook_set_global<false,true>(extra_items, b_legacy) : 0 , date ? &titleformat_hook_date(&st) : 0, &titleformat_hook_playlist_name()), temp, to_sort, 0, play_control::display_level_none);
				}
		/*		else
				{
					g_playlist_entries.get_display_name(n, column, temp);
				}*/

				const char * ptr = temp.get_ptr();
				if (strchr(ptr,3))
				{
					titleformat_compiler::remove_color_marks(ptr,temp2);
					ptr = temp2;
				}

				data.add_item(new(std::nothrow) sort_info(ptr,n));
			}

		}

		if (descending)
		{
			data.sort(sort_info_callback_base<sort_info*>::desc_sort_callback());
		}
		else
		{
			sort_info_callback_base<sort_info*>::asc_sort_callback cc;
			data.sort(cc);
		}

		unsigned new_count = data.get_count(), i=0;

		for(n=0;n<count;n++)
		{
			if (!selection_only || mask[n] == true)
			{
				order[n]=data[/*descending ? new_count - 1 - i :*/i]->index; //or we could use diff sortproc
				i++;
			}
			else
			{
				order[n] = n;
			}
		}

		data.delete_all();

		unsigned window_count = playlist_view::list_playlist.get_count();
		for (n=0; n<window_count; n++)
		{
			playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
			uSendMessage(p_playlist->wnd_playlist, WM_SETREDRAW, FALSE, 0);
		}
		
		playlist_api->activeplaylist_reorder_items(order.get_ptr(), count);
		
		for (n=0; n<window_count; n++)
		{
			playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
			uSendMessage(p_playlist->wnd_playlist, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(p_playlist->wnd_playlist, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
		}
		if (!selection_only)
		{
			g_remove_sort();//change so only if diff idx
			g_cache.active_set_playlist_sort(column, descending);
			g_update_sort();
		}
	}
}

void playlist_view::g_update_sort()
{
	if (cfg_show_sort_arrows)
	{
		unsigned column;
		bool descending;
		bool b_sorted = g_cache.active_get_playlist_sort(column, &descending);
		if (b_sorted)
		{
			unsigned n, pcount = list_playlist.get_count();
			for (n=0; n<pcount; n++)
			{
				playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
				if (p_playlist->wnd_header)
				{
					HDITEM hdi;
					memset(&hdi, 0, sizeof(hdi));
					
					hdi.mask = HDI_FORMAT;
					Header_GetItem(p_playlist->wnd_header, column, &hdi);
					
					if (is_winxp_or_newer())
						hdi.fmt |= HDF_STRING | (descending ? 0x0200 : 0x0400);
					else
					{
						if (!g_imagelist) create_image_list();
						if (!Header_GetImageList(p_playlist->wnd_header))
							Header_SetImageList(p_playlist->wnd_header, g_imagelist);
						hdi.mask |= HDI_IMAGE;
						hdi.fmt |= HDF_STRING|HDF_IMAGE|( (hdi.fmt & HDF_RIGHT ) ? 0 : HDF_BITMAP_ON_RIGHT);
						if (descending)
						{
							hdi.iImage = 0;
						}
						else
						{
							hdi.iImage = 1;
						}
					}
					
					Header_SetItem(p_playlist->wnd_header, column, &hdi);
				}
			}
		}
	}
}

void playlist_view_cache::flush_all(bool b_flush_columns)
{	
	unsigned n,count = get_count();
	for (n=0;n<count;n++)
	{
		flush(n);
		if (b_flush_columns) flush_columns(n);
	}
}

void playlist_view_cache::enable()
{
	m_active = true;
	rebuild();
}

void playlist_view_cache::rebuild()
{
	delete_all();

	static_api_ptr_t<playlist_manager> playlist_api;

	unsigned n,pcount = playlist_api->get_playlist_count();
	for (n=0; n<pcount; n++)
	{
		playlist_cache * p_cache = new(std::nothrow) playlist_cache;
		unsigned i,count = playlist_api->playlist_get_item_count(n);
		for (i=0;i<count;i++)
			p_cache->add_item(i);
		add_item(p_cache);
	}

	active_playlist = playlist_api->get_active_playlist();
}

void playlist_view_cache::disable()
{
	m_active = false;
	delete_all();
}

template <bool set, bool get>
bool titleformat_hook_set_global<set, get>::process_field(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, bool & p_found_flag)
{
	p_found_flag = false;
	if (b_legacy && p_name_length > 1 && p_name[0] == '_')
	{
		const char * ptr = p_vars.find_by_name(p_name + 1, p_name_length - 1);
		if (ptr)
		{
			p_out->write(titleformat_inputtypes::unknown, ptr, pfc_infinite);
			p_found_flag = true;
			return true;
		}
	}
	return false;
}
template <bool set, bool get>
bool titleformat_hook_set_global<set, get>::process_function(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag)
{
	p_found_flag = false;
	if (set && !stricmp_utf8_ex(p_name, p_name_length, "set_global", pfc_infinite))
	{
		switch (p_params->get_param_count())
		{
		case 2:
		{
			const char * name;
			const char * value;
			unsigned name_length, value_length;
			p_params->get_param(0, name, name_length);
			p_params->get_param(1, value, value_length);
			p_vars.add_item(name, name_length, value, value_length);
			p_found_flag = true;
			return true;
		}
		default:
			return false;
		}
	}
	else if (get && !stricmp_utf8_ex(p_name, p_name_length, "get_global", pfc_infinite))
	{
		switch (p_params->get_param_count())
		{
		case 1:
		{
			const char * name;
			unsigned name_length;
			p_params->get_param(0, name, name_length);
			const char * ptr = p_vars.find_by_name(name, name_length);
			if (ptr)
			{
				p_out->write(titleformat_inputtypes::unknown, ptr, pfc_infinite);
				p_found_flag = true;
			}
			else p_out->write(titleformat_inputtypes::unknown, "[unknown variable]", pfc_infinite);
			return true;
		}
		default:
			return false;
		}
	}
	else return false;
}

