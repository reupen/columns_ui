#include "stdafx.h"
#include "playlist_view.h"
#include "main_window.h"

cfg_int cfg_pv_use_system_frame(create_guid(0x99bbcbcb,0xd5c4,0x2122,0x0b,0x5c,0xd8,0x01,0x33,0x63,0xaa,0x36),0),
cfg_pv_text_colour(create_guid(0x2a1aeb89,0xd278,0xa73e,0xe4,0x12,0x22,0xb2,0xe0,0x23,0x80,0x62),get_default_colour(colours::COLOUR_TEXT)),
cfg_pv_selected_text_colour(create_guid(0xc8f9907d,0x675b,0x89a6,0xb0,0xda,0xfc,0xea,0xa3,0x35,0x79,0x4a),get_default_colour(colours::COLOUR_SELECTED_TEXT)),
cfg_pv_selected_back(create_guid(0x70f6c48c,0xdee7,0xecaf,0xcf,0xff,0xca,0x90,0xf8,0x18,0x58,0x57),get_default_colour(colours::COLOUR_SELECTED_BACK)),
cfg_pv_selceted_back_no_focus(create_guid(0x0327b009,0xdcf2,0x3a97,0x24,0x52,0x71,0x0d,0x14,0xe4,0xc5,0xce),get_default_colour(colours::COLOUR_SELECTED_BACK_NO_FOCUS)),
cfg_pv_use_custom_colours(create_guid(0x214156c5,0x17cb,0x58f7,0xf1,0x5e,0x50,0x50,0x95,0x23,0x59,0x42),0);

// {224ED777-DF52-4985-B70B-C518186DE8BE}
static const GUID guid_pv_selected_text_no_focus = 
{ 0x224ed777, 0xdf52, 0x4985, { 0xb7, 0xb, 0xc5, 0x18, 0x18, 0x6d, 0xe8, 0xbe } };

cfg_int cfg_pv_selected_text_no_focus(guid_pv_selected_text_no_focus,get_default_colour(colours::COLOUR_SELECTED_TEXT_NO_FOCUS));

service_ptr_t<titleformat_object> g_to_global;
service_ptr_t<titleformat_object> g_to_global_colour;

column_list_t playlist_view::columns;


pfc::ptr_list_t<playlist_view> playlist_view::list_playlist;

playlist_view::playlist_view()
	: wnd_playlist(nullptr), wnd_header(nullptr),
	initialised(false), drawing_enabled(false), dragged(true), drag_type(0),
	dragitem(0), dragstartitem(0), last_idx(-1), last_column(-1), g_shift_item_start(0), 
	scroll_item_offset(0), horizontal_offset(0),	g_dragging(false),	g_drag_lmb(false),
	g_dragging1(false), MENU_A_BASE(1), MENU_B_BASE(0), m_shown(false), m_edit_changed(false)
//#ifdef INLINE_EDIT
	, m_wnd_edit(nullptr),	m_edit_index(-1), m_edit_column(-1),
	m_prev_sel(false),
	m_no_next_edit(false),
	m_edit_timer(false), m_inline_edit_proc(nullptr), m_edit_save(true), m_edit_saving(false), m_theme(nullptr),
	m_always_show_focus(false), m_prevent_wm_char_processing(false)
//#endif
{
	drag_start.x = 0;
	drag_start.y = 0;
	drag_start_lmb.x = 0;
	drag_start_lmb.y = 0;

	tooltip.left=0;
	tooltip.top=0;
	tooltip.right=0;
	tooltip.bottom=0;
};

playlist_view::~playlist_view()
= default;

// {F20BED8F-225B-46c3-9FC7-454CEDB6CDAD}
GUID playlist_view::extension_guid = 
{ 0xf20bed8f, 0x225b, 0x46c3, { 0x9f, 0xc7, 0x45, 0x4c, 0xed, 0xb6, 0xcd, 0xad } };

const GUID & playlist_view::get_extension_guid() const
{
	return extension_guid;
}

void playlist_view::get_name(pfc::string_base & out)const
{
	out.set_string("Columns Playlist");
}

bool playlist_view::get_short_name(pfc::string_base & out)const
{
	out.set_string("Playlist");
	return true;
}

void playlist_view::get_category(pfc::string_base & out)const
{
	out.set_string("Playlist views");
}



bool playlist_view::is_item_clipped(int idx, int col)
{
	//	if (idx_rel + scroll_item_offset < playlist_api->activeplaylist_get_item_count() && idx_rel + scroll_item_offset >=0 && col >= 0 
	HDC hdc = GetDC(wnd_playlist);
	if (!hdc) return false;
	pfc::string8 text;
	g_cache.active_get_display_name(idx, col,text);
	SelectObject(hdc, g_font);
	int width = ui_helpers::get_text_width_color(hdc, text, text.length());
	ReleaseDC(wnd_playlist, hdc);
	unsigned col_width = get_column_width(col);

	//	console::info(pfc::string_printf("%i %i",width+4, col_width));

	return (width+2+(columns[col]->align == ALIGN_LEFT ? 2 : columns[col]->align == ALIGN_RIGHT ? 1 : 0) > col_width);//we use 3 for the spacing, 1 for column divider
	//however, this is going be wrong for centred columns ...
}


bool playlist_view::ensure_visible(int idx, bool check)
{
	//	assert(0);
	//	assert(!IsIconic(wnd_playlist) || IsWindowVisible(wnd_playlist));
	bool rv = true;
	RECT rect;
	//	if (g_minimised) rect = g_client_rect; else rect = get_playlist_rect();

	get_playlist_rect(&rect);
	int item_height=0;
	item_height = get_item_height();

	int items = ((rect.bottom-rect.top)/item_height);

	//	console::info(pfc::string_printf("%i %i %i %i : %i",rect,item_height));

	rv = !( idx < (scroll_item_offset) || idx > (items + scroll_item_offset - 1));

	if (!rv && !check)
	{
		SCROLLINFO scroll;
		memset(&scroll, 0, sizeof(SCROLLINFO));
		scroll.fMask = SIF_POS;
		scroll.nPos = idx - ((items-1)/2);
		scroll.cbSize = sizeof(SCROLLINFO);
		scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &scroll, true);

		RedrawWindow(wnd_playlist,nullptr,nullptr,RDW_INVALIDATE|RDW_UPDATENOW);
	}

	return rv;
}

unsigned int playlist_view::calculate_header_height()
{
	unsigned rv = 0;
	if (wnd_header)
	{
		HFONT font = (HFONT)SendMessage(wnd_header, WM_GETFONT, 0, 0);
		rv = uGetFontHeight(font) + 5;
	}
	return rv;
}

int playlist_view::get_header_height()
{
	int rv = 0;
	if (wnd_header)
	{
		RECT rect;
		GetWindowRect(wnd_header, &rect);
		rv =  (rect.bottom-rect.top);
	}
	return rv;
}

int playlist_view::get_item_height()
{
	int rv = 1;
	if (g_font)
	{
		//	if (!g_font) return 1;
		rv = uGetFontHeight(g_font) + settings::playlist_view_item_padding;
		if (rv < 1) rv = 1;
	}
	return rv;
}

LRESULT playlist_view::CreateToolTip(const char * text)
{
	if (g_tooltip) {DestroyWindow(g_tooltip); g_tooltip=nullptr;}

	DLLVERSIONINFO2 dvi;
	bool b_comctl_6 = SUCCEEDED(uih::GetComCtl32Version(dvi)) && dvi.info1.dwMajorVersion >= 6;

	g_tooltip = CreateWindowEx(b_comctl_6?WS_EX_TRANSPARENT:0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_NOPREFIX ,		
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, wnd_playlist, nullptr, core_api::get_my_instance(), nullptr);

	//	toolproc = (WNDPROC)SetWindowLongPtr(g_tooltip,GWLP_WNDPROC,(LPARAM)(TooltipHook));


	//	SendMessage(g_tooltip, CCM_SETVERSION, (WPARAM) COMCTL32_VERSION, 0);

	//SetWindowPos(g_tooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	RECT rect;
	GetClientRect (wnd_playlist, &rect);

	uTOOLINFO ti;
	memset(&ti,0,sizeof(ti));

	ti.cbSize = sizeof(uTOOLINFO);
	ti.uFlags = TTF_TRANSPARENT|TTF_SUBCLASS;//TTF_SUBCLASS
	ti.hwnd = wnd_playlist;
	ti.hinst = core_api::get_my_instance();
	ti.uId = ID_PLAYLIST_TOOLTIP;
	ti.lpszText = const_cast<char *>(text);
	ti.rect = rect;

	return uToolTip_AddTool(g_tooltip, &ti);
}


unsigned playlist_view::get_last_viewable_item()
{
	unsigned rv;
	RECT rect;
	get_playlist_rect(&rect);
	int item_height = get_item_height();
	int items = ((rect.bottom-rect.top)/item_height);

	static_api_ptr_t<playlist_manager> playlist_api;
	int total = playlist_api->activeplaylist_get_item_count();
	rv = items + scroll_item_offset - 1;
	if (rv >= total) rv = total-1; 

	return rv;
}

COLORREF playlist_view::g_get_default_colour(colours::t_colours col)
{
	if (cfg_pv_use_custom_colours!=1) return get_default_colour(col, cfg_pv_use_custom_colours ==2);
	switch (col)
	{
	case colours::COLOUR_TEXT:
		return cfg_pv_text_colour;
		break;
	case colours::COLOUR_SELECTED_TEXT:
		return cfg_pv_selected_text_colour;
		break;
	case colours::COLOUR_BACK:
		return cfg_back;
		break;
	case colours::COLOUR_SELECTED_BACK:
		return cfg_pv_selected_back;
		break;
	case colours::COLOUR_FRAME:
		return cfg_focus;
		break;
	case colours::COLOUR_SELECTED_BACK_NO_FOCUS:
		return cfg_pv_selceted_back_no_focus;
		break;
	case colours::COLOUR_SELECTED_TEXT_NO_FOCUS:
		return cfg_pv_selected_text_no_focus;
		break;
	}
	return 0xFF;
}

COLORREF get_default_theme_colour(HTHEME thm, colours::t_colours index)
{
	if (!thm || !IsAppThemed() || !IsThemeActive())
		return get_default_colour(index);
	switch (index)
	{
	case colours::COLOUR_TEXT:
		return GetThemeSysColor(thm, COLOR_WINDOWTEXT);
	case colours::COLOUR_SELECTED_TEXT:
		return GetThemeSysColor(thm, COLOR_HIGHLIGHTTEXT);
	case colours::COLOUR_BACK:
		return GetThemeSysColor(thm, COLOR_WINDOW);
	case colours::COLOUR_SELECTED_BACK:
		return GetThemeSysColor(thm, COLOR_HIGHLIGHT);
	case colours::COLOUR_FRAME:
		return GetThemeSysColor(thm, COLOR_WINDOWFRAME);
	case colours::COLOUR_SELECTED_BACK_NO_FOCUS:
		return GetThemeSysColor(thm, COLOR_BTNFACE);
	case colours::COLOUR_SELECTED_TEXT_NO_FOCUS:
		return GetThemeSysColor(thm, COLOR_BTNTEXT);
	default:
		return 0x0000FF;
	}
}

COLORREF playlist_view::get_default_colour_v2(colours::t_colours col)
{
	if (cfg_pv_use_custom_colours==0)
		return get_default_colour(col);
	else if (cfg_pv_use_custom_colours==2)
		return get_default_theme_colour(m_theme, col);
	else
	{
		switch (col)
		{
		case colours::COLOUR_TEXT:
			return cfg_pv_text_colour;
			break;
		case colours::COLOUR_SELECTED_TEXT:
			return cfg_pv_selected_text_colour;
			break;
		case colours::COLOUR_BACK:
			return cfg_back;
			break;
		case colours::COLOUR_SELECTED_BACK:
			return cfg_pv_selected_back;
			break;
		case colours::COLOUR_FRAME:
			return cfg_focus;
			break;
		case colours::COLOUR_SELECTED_BACK_NO_FOCUS:
			return cfg_pv_selceted_back_no_focus;
			break;
		case colours::COLOUR_SELECTED_TEXT_NO_FOCUS:
			return cfg_pv_selected_text_no_focus;
			break;
		}
	}
	return 0xFF;
}

void playlist_view::get_playlist_rect(RECT * out)
{
	int item_height = get_item_height();
	int header_height = 0;
	if (wnd_header)
	{
		RECT rc_header;
		GetWindowRect(wnd_header, &rc_header);
		header_height = rc_header.bottom-rc_header.top;
	}
	GetClientRect(wnd_playlist,out);

	if (out->bottom - out->top > header_height)
		out->top += header_height;
	else
		out->top = out->bottom;

}





unsigned playlist_view::get_column_widths(pfc::array_t<int, pfc::alloc_fast_aggressive> & out) const
{
	const bit_array & p_mask = g_cache.active_get_columns_mask();
	unsigned n,t = columns.get_count(),nw=0,i;

	unsigned ac = 0;
	for (n=0;n<t;n++) if  (p_mask[n]) ac++;

	out.set_size(ac);

	RECT hd;
	SetRectEmpty(&hd);
	
	if (cfg_nohscroll && wnd_playlist && GetClientRect(wnd_playlist, &hd))
	{

		int tw=0,total_parts=0;

		pfc::array_t<unsigned> columns_parts;
		
		columns_parts.set_size(t);

		out.fill(0);

		for (n=0,i=0;n<t;n++)
		if (p_mask[n])
		{
			tw += columns[n]->width;
			unsigned part = columns[n]->parts;
			total_parts += part;
			columns_parts[n] = part;
			i++;
		}

		int excess = hd.right-hd.left-tw;

		bool first_pass = true;

		while ( (excess && total_parts) || first_pass)
		{
			first_pass= false;
			int parts = total_parts;


			for (n=0,i=0;n<t;n++)
			if (p_mask[n])
			{
				int part = columns_parts[n];
				int e = ((parts && part) ?  MulDiv(part,(excess),parts) : 0);
				int f = columns[n]->width;

				parts -= part;
				excess -= e;
				if (e < f*-1)
				{
					e = f*-1;
					total_parts -= columns_parts[n];
					columns_parts[n]=0;
				}
				int w = f + e;

				out[i++] += w;
				nw += w;
			}
		}
	}
	else
	{
		for (n=0,i=0;n<t;n++) 
		{
			if (p_mask[n])
			{
				out[i] = columns[n]->width;
				nw += out[i++];
			}
		}
	}
	return nw;
}



void playlist_view::on_playlist_activate(unsigned p_old,unsigned p_new)
{
	//if (!cfg_nohscroll) g_save_columns();
	//	g_reset_columns();

	unsigned n, count = playlist_view::list_playlist.get_count();
	for (n=0; n<count; n++)
	{
		playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);

		t_local_cache::t_local_cache_entry * p_cache = nullptr; 
		if (p_playlist->m_cache.get_entry(p_old, p_cache))
		{
			p_cache->set_last_position(p_playlist->scroll_item_offset);
		}

		p_playlist->update_scrollbar(true);
		if (p_playlist->wnd_header)
		{
			p_playlist->rebuild_header();			
			p_playlist->move_header(true, false);
		}

		if (p_playlist->m_cache.get_entry(p_new, p_cache))
		{
			unsigned temp;
			if (p_cache->get_last_position(temp))
			{
				SCROLLINFO scroll;
				memset(&scroll, 0, sizeof(SCROLLINFO));
				scroll.fMask = SIF_POS;
				scroll.nPos = temp;
				scroll.cbSize = sizeof(SCROLLINFO);
				p_playlist->scroll_item_offset = SetScrollInfo(p_playlist->wnd_playlist, SB_VERT, &scroll, true);
			}
			else
				p_playlist->ensure_visible(static_api_ptr_t<playlist_manager>()->playlist_get_focus_item(p_new));
		}

		SendMessage(p_playlist->wnd_playlist, WM_SETREDRAW, TRUE, 0);		
		RedrawWindow(p_playlist->wnd_playlist,nullptr,nullptr,RDW_INVALIDATE|RDW_UPDATENOW);
	}
}

class playlist_callback_playlist : public playlist_callback_static
{
public:

	void on_items_added(unsigned p_index,unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection) override//inside any of these methods, you can call IPlaylist APIs to get exact info about what happened (but only methods that read playlist state, not those that modify it)
	{
		//		console::info(pfc::string_printf("on_items_added: %i %i",p_index,start));
		if (playlist_view::g_get_cache().is_active())
		{
			bool b_active = p_index == playlist_view::g_get_cache().get_active_playlist();
			if (b_active)
				playlist_view::g_remove_sort();
			playlist_view::g_get_cache().on_items_added(p_index, start, p_data, p_selection);
			if (b_active)
			{
				unsigned n, count = playlist_view::list_playlist.get_count();
				for (n=0; n<count; n++)
				{
					playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
					if (p_playlist->wnd_playlist)
					{
						//					g_playlist_entries.rebuild_all();
						p_playlist->update_scrollbar();
						if (p_playlist->drawing_enabled) RedrawWindow(p_playlist->wnd_playlist,nullptr,nullptr,RDW_INVALIDATE|RDW_UPDATENOW);
					}
				}
			}
		}
	}
	void on_items_reordered(unsigned p_index,const unsigned * order,unsigned count) override
	{
		//		console::info(pfc::string_printf("on_items_reorder: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
		{
			bool b_active = p_index == playlist_view::g_get_cache().get_active_playlist();
			if (b_active)
				playlist_view::g_remove_sort();
			playlist_view::g_get_cache().on_items_reordered(p_index, order, count);
			if (b_active)
			{
				int n,start=0;
				for(n=0;n<count;n++)
				{
					start=n;
					while (n<count && order[n]!=n)
					{
						//					g_playlist_entries.mark_out_of_date(n);
						n++;
					}
					if (n>start)
					{
						unsigned nn, pcount = playlist_view::list_playlist.get_count();
						for (nn=0; nn<pcount; nn++)
						{
							playlist_view * p_playlist = playlist_view::list_playlist.get_item(nn);
							if (p_playlist->wnd_playlist)
								p_playlist->draw_items_wrapper(start,n-start);
						}
					}
				}
			}
		}
	}
	//changes selection too; doesnt actually change set of items that are selected or item having focus, just changes their order
	void FB2KAPI on_items_removing(unsigned p_playlist,const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count) override{};//called before actually removing them
	void FB2KAPI on_items_removed(unsigned p_index,const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count) override
	{
		//		console::info(pfc::string_printf("on_items_removed: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
		{
			bool b_active = p_index == playlist_view::g_get_cache().get_active_playlist();
			if (b_active)
				playlist_view::g_remove_sort();
			playlist_view::g_get_cache().on_items_removed(p_index, p_mask);
			if (b_active)
			{
				unsigned n, count = playlist_view::list_playlist.get_count();
				for (n=0; n<count; n++)
				{
					playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
					if (p_playlist->wnd_playlist)
					{
						//					g_playlist_entries.rebuild_all();
						p_playlist->update_scrollbar();
						if (p_playlist->drawing_enabled) RedrawWindow(p_playlist->wnd_playlist,nullptr,nullptr,RDW_INVALIDATE|RDW_UPDATENOW);
					}
				}
			}
		}
	}

	void on_items_selection_change(unsigned p_index,const bit_array & affected,const bit_array & state) override
	{
		//		console::info(pfc::string_printf("on_items_selection_change: %i",p_index));
		//		if (playlist_view::g_get_cache().is_active())
		{
			if (p_index == playlist_view::g_get_cache().get_active_playlist())
			{
				unsigned n, pcount = playlist_view::list_playlist.get_count();
				for (n=0; n<pcount; n++)
				{
					playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
					if (p_playlist->wnd_playlist)
					{
						int n,start=0;
						unsigned count = static_api_ptr_t<playlist_manager>()->activeplaylist_get_item_count();
						for(n=0;n<count;n++)
						{
							//	if (before[n]!=after[n]) draw_items_wrapper(n);
							start=n;
							while (n<count && affected[n])
							{
								n++;
							}
							if (n>start)
							{
								p_playlist->draw_items_wrapper(start,n-start);
							}
						}
					}
				}
			}
		}
	}
	void on_item_focus_change(unsigned p_index,unsigned from,unsigned to) override
	{
		//		console::info(pfc::string_printf("on_item_focus_change: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
		{
			if (p_index == playlist_view::g_get_cache().get_active_playlist())
			{
				unsigned n, count = playlist_view::list_playlist.get_count();
				for (n=0; n<count; n++)
				{
					playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
					if (p_playlist->wnd_playlist)
					{
						p_playlist->draw_items_wrapper(from);
						p_playlist->draw_items_wrapper(to);
					}
				}
			}
		}
	}

	void FB2KAPI on_items_modified_fromplayback(unsigned p_playlist,const bit_array & p_mask,play_control::t_display_level p_level) override
	{
		on_items_modified(p_playlist, p_mask, true, p_level);
	}

	void on_items_modified(unsigned p_index,const bit_array & p_mask) override
	{
		on_items_modified(p_index, p_mask, false);
	}

	void on_items_modified(unsigned p_index,const bit_array & p_mask, bool b_playback, playback_control::t_display_level p_level = playback_control::display_level_none)
	{
		//		console::info(pfc::string_printf("on_items_modified: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
		{
			bool b_active = p_index == playlist_view::g_get_cache().get_active_playlist();
			if (b_active && !b_playback)
				playlist_view::g_remove_sort();
			if (b_playback)
				playlist_view::g_get_cache().on_items_modified_fromplayback(p_index, p_mask, p_level);
			else
				playlist_view::g_get_cache().on_items_modified(p_index, p_mask);
			if (b_active)
			{
				unsigned n, count = playlist_view::list_playlist.get_count();
				for (n=0; n<count; n++)
				{
					playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
					if (p_playlist->wnd_playlist)
					{
						unsigned n,start=0, count = playlist_view::g_get_cache().playlist_get_count(p_index);
						for (n=0; n<count; n++)
						{
							start=n;
							while (n<count && p_mask[n])
							{
								n++;
							}
							if (n>start)
							{
								p_playlist->draw_items_wrapper(start,n-start);
							}
						}
						//					g_playlist_entries.mark_out_of_date(idx);
						//						p_playlist->draw_items_wrapper(idx);
					}
				}
			}
		}
	}

	void on_items_replaced(unsigned p_playlist,const bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data) override
	{
		//		console::info(pfc::string_printf("on_items_replaced: %i",p_playlist));
		on_items_modified(p_playlist, p_mask);
	}

	void on_item_ensure_visible(unsigned p_index,unsigned idx) override
	{
		//		console::info(pfc::string_printf("on_item_item_visible: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
		{
			if (p_index == playlist_view::g_get_cache().get_active_playlist())
			{
				unsigned n, count = playlist_view::list_playlist.get_count();
				for (n=0; n<count; n++)
				{
					playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
					if (p_playlist->wnd_playlist)
					{
						p_playlist->ensure_visible(idx);			
					}
				}
			}
		}
	}

	void on_playlist_activate(unsigned p_old,unsigned p_new) override
	{
		//		console::info(pfc::string_printf("on_playlist_activate: %i",p_new));
		if (playlist_view::g_get_cache().is_active())
		{
			playlist_view::g_get_cache().on_playlist_activate(p_old, p_new);
			//		playlist_view::g_remove_sort();
			playlist_view::on_playlist_activate(p_old, p_new);
			if (p_new != pfc_infinite)
				playlist_view::g_update_sort();
		}
	};
	void on_playlist_created(unsigned p_index,const char * p_name,unsigned p_name_len) override
	{
		//		console::info(pfc::string_printf("on_playlist_created: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
			playlist_view::g_get_cache().on_playlist_created(p_index, p_name, p_name_len);
	};
	void on_playlists_reorder(const unsigned * p_order,unsigned p_count) override
	{
		if (playlist_view::g_get_cache().is_active())
		{
			playlist_view::g_get_cache().on_playlists_reorder(p_order, p_count);
		}
	};
	void on_playlists_removing(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count) override
	{
		//		console::info(pfc::string_printf("on_playlist_removing"));
	};
	void on_playlists_removed(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count) override
	{
		//		console::info(pfc::string_printf("on_playlist_removed:"));
		if (playlist_view::g_get_cache().is_active())
		{
			playlist_view::g_get_cache().on_playlists_removed(p_mask, p_old_count, p_new_count);
		}
	};
	void on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len) override
	{
		//		console::info(pfc::string_printf("on_playlist_renamed: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
		{
			playlist_view::g_get_cache().on_playlist_renamed(p_index, p_new_name, p_new_name_len);
			if (p_index == playlist_view::g_get_cache().get_active_playlist())
				playlist_view::on_playlist_activate(p_index, p_index);
		}
	};

	void on_default_format_changed() override
	{
		//		console::info(pfc::string_printf("on_default_format_changed:"));
	};
	void on_playback_order_changed(unsigned p_new_index) override
	{
		//		console::info(pfc::string_printf("on_playback_order_changed: %i",p_new_index));
	};
	void on_playlist_locked(unsigned p_playlist,bool p_locked) override
	{
		//		console::info(pfc::string_printf("on_playlist_locked: %i",p_playlist));
	};

	unsigned get_flags() override { return playlist_callback::flag_all;}

};

static service_factory_single_t<playlist_callback_playlist> asdf3;


ui_extension::window_factory<playlist_view> blah;

void playlist_view::g_get_global_style_titleformat_object(service_ptr_t<titleformat_object> & p_out)
{
	if (!g_to_global_colour.is_valid()) 
		static_api_ptr_t<titleformat_compiler>()->compile_safe(g_to_global_colour, cfg_colour);
	p_out = g_to_global_colour;
}

playlist_view::class_data & playlist_view::get_class_data() const
{
	DWORD flags = 0;
	if (cfg_frame == 1) flags |= WS_EX_CLIENTEDGE;
	if (cfg_frame == 2) flags |= WS_EX_STATICEDGE;

	__implement_get_class_data_ex(_T("{F20BED8F-225B-46c3-9FC7-454CEDB6CDAD}"), _T(""), false, 0, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP, flags, CS_DBLCLKS | CS_HREDRAW);
}

void playlist_view::g_on_playback_follows_cursor_change(bool b_val)
{
	unsigned n, count = playlist_view::list_playlist.get_count();
	for (n = 0; n < count; n++)
	{
		playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
		p_playlist->m_always_show_focus = b_val;
		RedrawWindow(p_playlist->wnd_playlist, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void playlist_view::update_all_windows(HWND wnd_header_skip /*= 0*/)
{
	unsigned n, count = playlist_view::list_playlist.get_count();
	for (n = 0; n < count; n++)
	{
		playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
		if (p_playlist->wnd_header && wnd_header_skip != p_playlist->wnd_header)
		{
			p_playlist->rebuild_header();
		}
		p_playlist->update_scrollbar(true);
		RedrawWindow(p_playlist->wnd_playlist, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void playlist_view::g_on_columns_size_change(const playlist_view * p_skip /*= NULL*/)
{
	if (g_cache.is_active())
		columns.set_widths(g_columns);
	unsigned n, count = playlist_view::list_playlist.get_count();
	for (n = 0; n < count; n++)
	{
		playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);
		if (p_playlist != p_skip && p_playlist->wnd_header)
		{
			p_playlist->rebuild_header();
		}
		RedrawWindow(p_playlist->wnd_playlist, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void refresh_all_playlist_views()
{
	if (playlist_view::g_get_cache().is_active())
	{
		g_to_global.release();
		g_to_global_colour.release();
		playlist_view::g_reset_columns();
		unsigned m, pcount = playlist_view::list_playlist.get_count();
		for (m = 0; m < pcount; m++)
		{
			playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
			p_playlist->create_header();
			if (p_playlist->wnd_header)
				p_playlist->move_header();
		}
		playlist_view::update_all_windows();
	}
}

// {0CF29D60-1262-4f55-A6E1-BC4AE6579D19}
const GUID appearance_client_pv_impl::g_guid = 
{ 0xcf29d60, 0x1262, 0x4f55, { 0xa6, 0xe1, 0xbc, 0x4a, 0xe6, 0x57, 0x9d, 0x19 } };

appearance_client_pv_impl::factory<appearance_client_pv_impl> g_appearance_client_pv_impl;