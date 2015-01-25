#include "foo_ui_columns.h"

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

static unsigned g_day_timer;

VOID CALLBACK on_day_change()
{
	//	static_api_ptr_t<playlist_manager> playlist_api;
	//	unsigned count=playlist_api->get_playlist_count(),n;
	//	for (n=0;n<count;n++)
	playlist_view::g_get_cache().flush_all(false);

	unsigned m, pcount = playlist_view::list_playlist.get_count();
	for (m=0; m<pcount; m++)
	{
		playlist_view * p_playlist = playlist_view::list_playlist.get_item(m);
		if (p_playlist->wnd_playlist)
			RedrawWindow(p_playlist->wnd_playlist, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
	}

	set_day_timer();
}

void kill_day_timer();

class playlist_message_window : public ui_helpers::container_window
{
	long ref_count;
public:
	virtual class_data & get_class_data() const 
	{
		__implement_get_class_data_ex(_T("{13EFE4B7-A679-4e5c-8B98-F24A77667F78}"), _T(""), false, 0, 0, 0, 0);
	}

	virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_CREATE:
			playlist_view::g_load_columns();
			playlist_view::g_get_cache().enable();
			set_day_timer();
			break;
		case WM_TIMECHANGE:
			if (cfg_playlist_date) on_day_change();
			break;
		case WM_TIMER:
			on_day_change();
			break;
		case WM_SYSCOLORCHANGE:
			if (!cfg_pv_use_custom_colours)
			{
				playlist_view::g_reset_columns();
				//playlist_view::update_all_windows();
			}
			break;
		case WM_THEMECHANGED:
			break;
		case WM_DESTROY:
			if (g_font) DeleteObject(g_font); g_font = 0;
			if (g_header_font) DeleteObject(g_header_font); g_header_font = 0;
			g_to_global.release();
			g_to_global_colour.release();
			kill_day_timer();
			playlist_view::g_get_cache().disable();
			playlist_view::g_kill_columns();
			break;

		}
		return uDefWindowProc(wnd, msg, wp, lp);
	}
	void add_ref()
	{
		if (!ref_count++)
		{
			create(0);
		}
	}
	void release()
	{
		if (!--ref_count)
		{
			destroy();
		}
	}
	playlist_message_window() : ref_count(0) {};
};

static playlist_message_window g_playlist_message_window;

void kill_day_timer()
{
	HWND wnd = g_playlist_message_window.get_wnd();
	if (g_day_timer && wnd) {KillTimer(wnd, DAY_TIMER_ID); g_day_timer=0;}
}

void set_day_timer()
{
	HWND wnd = g_playlist_message_window.get_wnd();

	if (g_day_timer) {KillTimer(wnd, DAY_TIMER_ID); g_day_timer=0;}

	if (cfg_playlist_date && g_playlist_message_window.get_wnd())
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		//	unsigned ms=st.wMilliseconds + st.wMinute*60*1000 + st.wSecond*1000 + st.wHour*60*60*1000;
		unsigned ms=/*24**/60*60*1000 - (st.wMilliseconds + ((/*st.wHour*60 + */st.wMinute)*60 + st.wSecond)*1000) ;

		SetTimer(wnd, DAY_TIMER_ID, ms,  0);
	}
}

class IDropSource_playlist : public IDropSource
{
	long refcount;
	service_ptr_t<playlist_view> p_playlist;
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void ** ppvObject)
	{
		if (ppvObject == NULL) return E_INVALIDARG;
		*ppvObject = NULL;
		if (iid == IID_IUnknown) {AddRef();*ppvObject = (IUnknown*)this;return S_OK;}
		else if (iid == IID_IDropSource) {AddRef();*ppvObject = (IDropSource*)this;return S_OK;}
		else return E_NOINTERFACE;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef() {return InterlockedIncrement(&refcount);}
	virtual ULONG STDMETHODCALLTYPE Release()
	{
		LONG rv = InterlockedDecrement(&refcount);
		if (!rv)
		{
#ifdef _DEBUG
			OutputDebugString(_T("deleting IDropSource_playlist"));
#endif
			delete this;
		}
		return rv;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState)
	{
		if (fEscapePressed || (p_playlist->g_dragging1 &&  grfKeyState&MK_LBUTTON) || (!p_playlist->g_dragging1 &&  ( ( grfKeyState&MK_RBUTTON) || !(grfKeyState&MK_CONTROL) ) ) ) {return DRAGDROP_S_CANCEL;}
		else if ((p_playlist->g_dragging1 && !(grfKeyState&MK_RBUTTON)) || (!p_playlist->g_dragging1 && !(grfKeyState&MK_LBUTTON )))
		{
			return DRAGDROP_S_DROP;
		}
		else return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect)
	{
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}

	IDropSource_playlist(playlist_view * playlist) : refcount(0), p_playlist(playlist) {};

};



HRESULT STDMETHODCALLTYPE IDropTarget_playlist::QueryInterface(REFIID riid, LPVOID FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	*ppvObject = NULL;
	if (riid == IID_IUnknown) {AddRef();*ppvObject = (IUnknown*)this;return S_OK;}
	else if (riid == IID_IDropTarget) {AddRef();*ppvObject = (IDropTarget*)this;return S_OK;}
	else return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE   IDropTarget_playlist::AddRef()
{
	return InterlockedIncrement(&drop_ref_count); 
}
ULONG STDMETHODCALLTYPE   IDropTarget_playlist::Release()
{
	LONG rv = InterlockedDecrement(&drop_ref_count); 
	if (!rv)
	{
#ifdef _DEBUG
		OutputDebugString(_T("deleting IDropTarget_playlist"));
#endif
		delete this;
	}
	return rv;
}

HRESULT STDMETHODCALLTYPE IDropTarget_playlist::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

	*pdwEffect = DROPEFFECT_COPY;
	if (ui_drop_item_callback::g_is_accepted_type(pDataObj, pdwEffect) || static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check(pDataObj))
	{
		return S_OK; 	
	}
	return S_FALSE; 	
}


HRESULT STDMETHODCALLTYPE IDropTarget_playlist::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	*pdwEffect = DROPEFFECT_COPY;
	last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

	return S_OK; 



}

HRESULT STDMETHODCALLTYPE IDropTarget_playlist::DragLeave( void)
{
	return S_OK;		
}

HRESULT STDMETHODCALLTYPE IDropTarget_playlist::Drop( IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{ 

	POINT pti;
	pti.y = pt.y;
	pti.x = pt.x;
	if (p_playlist->wnd_playlist)
	{

		bool process = !ui_drop_item_callback::g_on_drop(pDataObj);

		if (process && last_rmb)
		{
			process = false;
			enum {ID_DROP = 1, ID_CANCEL };

			HMENU menu = CreatePopupMenu();

			uAppendMenu(menu,(MF_STRING),ID_DROP,"&Add files here");
			uAppendMenu(menu,MF_SEPARATOR,0,0);
			uAppendMenu(menu,MF_STRING,ID_CANCEL,"&Cancel");

			int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,p_playlist->wnd_playlist,0);
			DestroyMenu(menu);

			if (cmd)
			{
				switch(cmd)
				{
				case ID_DROP:
					process = true;						
					break;
				}
			}
		}

		if (process)
		{
			metadb_handle_list data;

			//console::info(static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check_if_native(pDataObj)?"native":"not very native?");

			static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files(pDataObj, data, true,p_playlist->wnd_playlist);

			int idx = -1;

			if (!cfg_drop_at_end)
			{
				POINT ptt = pti;
				ScreenToClient(p_playlist->wnd_playlist, &ptt);
				idx = p_playlist->hittest_item(ptt.x, ptt.y, false);
			}

			static_api_ptr_t<playlist_manager> playlist_api;
			playlist_api->activeplaylist_undo_backup();
			playlist_api->activeplaylist_clear_selection();
			playlist_api->activeplaylist_insert_items(idx, data, bit_array_true());

			data.remove_all();
		}
	}

	return S_OK;		
}
IDropTarget_playlist::IDropTarget_playlist(playlist_view * playlist) : drop_ref_count(0), last_rmb(false), p_playlist(playlist)
{
}


playlist_view::playlist_view() : wnd_playlist(0),
initialised(false), drawing_enabled(false), dragged(true), drag_type(0),
dragitem(0), dragstartitem(0), last_idx(-1), last_column(-1), g_shift_item_start(0), 
scroll_item_offset(0), horizontal_offset(0),	g_dragging(false),	g_drag_lmb(false),
g_dragging1(false), MENU_A_BASE(1), MENU_B_BASE(0), m_shown(false), m_edit_changed(false)
//#ifdef INLINE_EDIT
, m_wnd_edit(NULL),	m_edit_index(-1), m_edit_column(-1),
	m_prev_sel(false),
	m_no_next_edit(false),
	m_edit_timer(false), m_inline_edit_proc(NULL), m_edit_save(true), m_edit_saving(false), m_theme(NULL),
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
{
}

// {F20BED8F-225B-46c3-9FC7-454CEDB6CDAD}
GUID playlist_view::extension_guid = 
{ 0xf20bed8f, 0x225b, 0x46c3, { 0x9f, 0xc7, 0x45, 0x4c, 0xed, 0xb6, 0xcd, 0xad } };

const GUID & playlist_view::get_extension_guid() const
{
	return extension_guid;
}

/*const char * playlist_view::class_name = "{F20BED8F-225B-46c3-9FC7-454CEDB6CDAD}";
long playlist_view::ref = 0;

bool playlist_view::is_available(ui_extension_host * host) const
{
return true;
}


bool playlist_view::add_ref()
{
if (!class_registered)
{
uWNDCLASS  wc;
memset(&wc,0,sizeof(uWNDCLASS));
wc.style          = CS_DBLCLKS|CS_HREDRAW;
wc.lpfnWndProc    = (WNDPROC)window_proc;
wc.hInstance      = core_api::get_my_instance();
wc.hCursor        = uLoadCursor(NULL, IDC_ARROW);
wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
wc.lpszClassName  = class_name;

class_registered = uRegisterClass(&wc) != 0;
}
if (!ref)
{
g_playlist_entries.refresh_columns();
}
ref++;
return true;
}

bool playlist_view::de_ref()
{
ref--;
if (!ref && class_registered)
{
class_registered = !uUnregisterClass(class_name, core_api::get_my_instance());
}
return true;
}*/

//HWND playlist_view::get_wnd() const {return wnd_playlist;}

/*LRESULT WINAPI playlist_view::window_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
playlist_view * p_this;

if(msg == WM_NCCREATE)
{
p_this = reinterpret_cast<playlist_view *>(((CREATESTRUCT *)(lp))->lpCreateParams);
uSetWindowLong(wnd, GWL_USERDATA, (LPARAM)p_this);
//		if (p_this) p_this->wnd_playlist = wnd;

}
else
p_this = reinterpret_cast<playlist_view*>(uGetWindowLong(wnd,GWL_USERDATA));//if isnt wm_create, retrieve pointer to class

return p_this ? p_this->on_message(wnd, msg, wp, lp) : uDefWindowProc(wnd, msg, wp, lp);
}*/

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

/*HWND playlist_view::init_or_take_ownership(HWND parent, ui_extension_host * host, stream_reader * r)
{
if (wnd_playlist)
{
ShowWindow(wnd_playlist, SW_HIDE);
SetParent(wnd_playlist, parent);
p_host->relinquish_ownership(wnd_vis);
p_host = host;
}
else
{
add_ref();

list_playlist.add_item(this);
initialised = true;

p_host = host;

long flags = 0;
if (cfg_frame == 1) flags |= WS_EX_CLIENTEDGE;
if (cfg_frame == 2) flags |= WS_EX_STATICEDGE;

wnd_playlist = uCreateWindowEx(flags, class_name, "Playlist view",
WS_CHILD | WS_CLIPSIBLINGS| WS_CLIPCHILDREN | WS_TABSTOP| WS_VSCROLL, 0, 0, 0, 0,
parent, 0, core_api::get_my_instance(), this);
}

return wnd_playlist;
}*/

void playlist_view::rebuild_header(bool rebuild)
{

	if (rebuild)
	{
		int n,t=Header_GetItemCount(wnd_header);
		{
			for (n=0;n<t;n++) Header_DeleteItem(wnd_header,0);
		}
	}

	uHDITEM hdi; 
	memset(&hdi, 0, sizeof(HDITEM));

	hdi.mask = (rebuild ? HDI_TEXT | HDI_FORMAT : 0)  | HDI_WIDTH; 
	hdi.fmt = HDF_LEFT | HDF_STRING; 

	pfc::string8 name;

	{
		pfc::array_t<int, pfc::alloc_fast_aggressive> widths;
		get_column_widths(widths);

		const bit_array & p_mask = g_cache.active_get_columns_mask();

		int n,t=columns.get_count(),i=0;//,tw=g_playlist_entries.get_total_width();
		for(n = 0; n<t; n++)
		{
			if (p_mask[n])
			{
				if (rebuild)
				{
					alignment align = columns[n]->align;
					hdi.fmt = HDF_STRING | (align == ALIGN_CENTRE ? HDF_CENTER : (align == ALIGN_RIGHT ? HDF_RIGHT : HDF_LEFT)) ; 
					name = columns[n]->name;
					hdi.cchTextMax = name.length(); 
					hdi.pszText = const_cast<char*>(name.get_ptr());
				}

				hdi.cxy = widths[i];

				uHeader_InsertItem(wnd_header, i++, &hdi, rebuild); 
			}
		}
	}
}

void playlist_view::update_scrollbar(bool redraw)
{
	LONG_PTR old_style = uGetWindowLong(wnd_playlist,GWL_STYLE);
	bool need_move = false;

	static_api_ptr_t<playlist_manager> playlist_api;

	RECT rect;
	get_playlist_rect(&rect);

	int item_height = get_item_height();

	int items = ((rect.bottom-rect.top)/item_height);
	int total = playlist_api->activeplaylist_get_item_count();

	SCROLLINFO info;
	memset(&info, 0, sizeof(SCROLLINFO));
	info.fMask = SIF_POS;
	info.cbSize = sizeof(SCROLLINFO);
	GetScrollInfo(wnd_playlist, SB_HORZ, &info);

	horizontal_offset = info.nPos;

	GetScrollInfo(wnd_playlist, SB_VERT, &info);
	scroll_item_offset = info.nPos;

	SCROLLINFO scroll;
	memset(&scroll,0,  sizeof(SCROLLINFO));
	scroll.fMask = SIF_RANGE|SIF_PAGE|SIF_POS;
	scroll.nMin = 0;
	scroll.nPos = scroll_item_offset;
	scroll.nPage = items;
	scroll.nMax = total - 1;
	scroll.cbSize = sizeof(SCROLLINFO);
	scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &scroll, true);

	//	assert(0);

	bool redraw_playlist = info.nPos != scroll_item_offset;
	int old_vert_pos = info.nPos;

	bool show_vert = (items >= total ? FALSE : TRUE);

	if (((old_style & WS_VSCROLL) != 0) != show_vert)
	{
		ShowScrollBar(wnd_playlist, SB_VERT, show_vert);
		need_move = true;
		//maybe should call get_playlist_rect again ??
	}

	get_playlist_rect(&rect);

	bool show_horiz = FALSE;

	int old_horizontal_offset = horizontal_offset;

	if (cfg_nohscroll) horizontal_offset = 0;
	{
		unsigned totalh = get_columns_total_width();
		scroll.nMin = 0;
		scroll.nPos = horizontal_offset;
		scroll.nMax = cfg_nohscroll ? 0 : totalh - 1;
		scroll.nPage = rect.right - rect.left;
		if (scroll.nPage > scroll.nMax+1)
			scroll.nPage = scroll.nMax+1;
		horizontal_offset = SetScrollInfo(wnd_playlist, SB_HORZ, &scroll, true);//redraw
		if (!cfg_nohscroll)
			show_horiz = (totalh > (rect.right - rect.left)  ? TRUE : FALSE);
	}

	if (((old_style & WS_HSCROLL) != 0) != show_horiz)
	{
		ShowScrollBar(wnd_playlist, SB_HORZ, show_horiz);
	}

	if (old_horizontal_offset != horizontal_offset) need_move= true;


	if (wnd_header && need_move) 
		move_header();

	if (redraw_playlist)
	{

		{
			RECT playlist,rc_redraw;
			get_playlist_rect(&playlist);
			ScrollWindowEx(wnd_playlist, 0, (old_vert_pos-scroll_item_offset) * get_item_height(), &playlist, &playlist, 0, &rc_redraw, 0);
			RedrawWindow(wnd_playlist,&rc_redraw,0,RDW_INVALIDATE|RDW_UPDATENOW);
		}

	}
}

bool playlist_view::draw_items_wrapper(int start_item, int count)
{	
	if (count < 1 || start_item < 0) return false;

	bool rv = false;

	static_api_ptr_t<playlist_manager> playlist_api;

	RECT rect;
	get_playlist_rect(&rect);
	int item_height = get_item_height();

	int total = playlist_api->activeplaylist_get_item_count();

	int visible_first = scroll_item_offset;

	//we minus 1 coz this is a count, last_offset is zero-based index
	int visible_last = scroll_item_offset + (rect.bottom-rect.top)/item_height - 1; 

	//is there a partially obsurced item ??
	if ((visible_last-visible_first +1)*item_height < rect.bottom-rect.top) visible_last++;

	if (visible_last >= total) visible_last = total-1;

	if (start_item+count-1 < visible_first || start_item > visible_last) return false;

	int new_first=start_item;
	int new_count=count;

	if (new_first < visible_first)
	{
		new_count -= visible_first - new_first;
		new_first = visible_first;
	}


	if (new_first+new_count-1 > visible_last)
	{
		new_count -= ((new_first+new_count-1) - visible_last);
	}


	int relative_item = new_first - scroll_item_offset;	
	draw_items(relative_item, new_count);

	return true;
}

bool playlist_view::draw_items(int start_item, int count)
{
	HDC dc = GetDC(wnd_playlist);
	bool ret = draw_items(dc, start_item, count);
	ReleaseDC(wnd_playlist, dc);
	return ret;
}

bool playlist_view::draw_items(HDC dc, int start_item, int count)
{
	//	profiler(draw_items);

	if (!drawing_enabled) return false;

	static_api_ptr_t<playlist_manager> playlist_api;

	HDC hdc_mem=0;
	RECT rect,item,item_area,bk,draw/*,text*/;
	GetClientRect(wnd_playlist,&rect);	
	rect.top+=get_header_height();

	int item_height = get_item_height();

	HBRUSH br=0;
	HBITMAP hbm_mem=0, hbm_old=0; 

	pfc::array_t<int, pfc::alloc_fast_aggressive> widths;
	int total_width = get_column_widths(widths);
	int t = columns.get_count();

	const bit_array & p_mask = g_cache.active_get_columns_mask();

	item_area.left = rect.left;
	item_area.right = rect.right;
	item_area.top = rect.top + (item_height*start_item);
	item_area.bottom = item_area.top + (item_height*count);

	item.left = 0 - horizontal_offset;
	item.right = item.left + total_width;
	item.top = 0;
	item.bottom = item.top + item_height;

	bk.top = 0;
	bk.left = 0;
	bk.bottom = item_height*count;
	bk.right=rect.right-rect.left;

	/*static */pfc::string8_fast_aggressive temp;
	temp.prealloc(512);

	/* edit01 */

	hdc_mem = CreateCompatibleDC(dc);

	COLORREF colourfore = 0xff;

	hbm_mem = CreateCompatibleBitmap(dc, rect.right-rect.left, item_height*count);

	hbm_old = (HBITMAP)SelectObject(hdc_mem, hbm_mem);

	HGDIOBJ font_old = SelectObject(hdc_mem, g_font);

	cui::colours::helper p_helper(appearance_client_pv_impl::g_guid);

	//fill entire area with back colour
	br = CreateSolidBrush(p_helper.get_colour(cui::colours::colour_background));
	FillRect(hdc_mem, &bk, br);
	DeleteObject(br);

	//need checks here because of filling background

	{
		int total = playlist_api->activeplaylist_get_item_count();

		if (start_item + count + scroll_item_offset > total) //end item is NOT inclusive
		{
			count -= (start_item + count + scroll_item_offset) - total;
		}

	}

	int end_item = start_item + count;



	//draw each item

	int n,
		focus = playlist_api->activeplaylist_get_focus_item();

	//	static int pcount;



	t_size playing_index, playing_playlist;
	playlist_api->get_playing_item_location(&playing_playlist, &playing_index);
	bool b_playback = static_api_ptr_t<play_control>()->is_playing();
	if (g_cache.get_active_playlist() != playing_playlist) playing_index = pfc_infinite;
	for (n=start_item;n<end_item;n++)
	{
		bool sel = playlist_api->activeplaylist_is_item_selected(n + scroll_item_offset);
		bool b_focused = GetFocus() == wnd_playlist || IsChild(wnd_playlist, GetFocus());
		bool b_playing = b_playback && playing_index == (n + scroll_item_offset);
		//draw each column of each item
		int theme_state = NULL;
		if (sel)
			theme_state = (b_playing ? LISS_HOTSELECTED : (b_focused ? LISS_SELECTED : LISS_SELECTEDNOTFOCUS));
		else if (b_playing) theme_state = LISS_HOT;
		bool b_themed = m_theme && p_helper.get_themed() && IsThemePartDefined(m_theme, LVP_LISTITEM, theme_state);
		if (b_themed && theme_state)
		{
			if (IsThemeBackgroundPartiallyTransparent(m_theme, LVP_LISTITEM, theme_state))
				DrawThemeParentBackground(get_wnd(), hdc_mem, &item);
			DrawThemeBackground(m_theme, hdc_mem, LVP_LISTITEM, theme_state, &item, NULL);
		}

		//int total = playlist_api->activeplaylist_get_item_count();
		int c,offset=0,i=0;
		for (c = 0; c<t; c++)
		{
			if (p_mask[c])
			{

				draw.left = item.left + offset;
				draw.top = item.top;

				offset += widths[i];

				draw.right = item.left + offset;
				draw.bottom = item.bottom;

				{
					//					profiler_debug(draw_item_get_string);
					g_cache.active_get_display_name(n + scroll_item_offset,i,temp);
				}

				colourinfo colours(0x000000,0x000000,0xFF,0xFF,0,0xFF);
				g_cache.active_get_colour(n + scroll_item_offset,i,colours);

				if (b_themed)
				{
					//COLORREF cr_back= get_default_colour(colours::COLOUR_BACK);
					//colourfore = get_default_colour(colours::COLOUR_TEXT);
					//GetThemeColor(m_theme, LVP_LISTITEM, sel ? (GetFocus() == wnd_playlist ? LIS_SELECTED : LIS_SELECTEDNOTFOCUS) : LIS_NORMAL, TMT_WINDOWTEXT, &colourfore);
					colourfore = GetThemeSysColor(m_theme, sel ? COLOR_BTNTEXT : COLOR_WINDOWTEXT);
					//GetThemeColor(m_theme, LVP_LISTITEM, sel ? (GetFocus() == wnd_playlist ? LIS_SELECTED : LIS_SELECTEDNOTFOCUS) : LIS_NORMAL, TMT_TEXTCOLOR, &colourfore);
					if (!theme_state)
					{
						//GetThemeColor(m_theme, LVP_LISTITEM, LIS_NORMAL, TMT_FILLCOLOR, &colourfore);
						br = CreateSolidBrush(colours.background_colour);
						FillRect(hdc_mem, &draw, br);
					}
				}
				else
				{
					if (sel)
					{   
						/* TEST */

						if (GetFocus() == wnd_playlist)
						{
							colourfore = colours.selected_text_colour;
							br = CreateSolidBrush(colours.selected_background_colour);
						}
						else
						{
							colourfore = colours.selected_text_colour_non_focus;
							br = CreateSolidBrush(colours.selected_background_colour_non_focus);
						}
					}
					else
					{
						colourfore = colours.text_colour;
						br = CreateSolidBrush(colours.background_colour);
					}
					//draw cell background
					//if (!b_themed)
					FillRect(hdc_mem, &draw, br);
				}


				if (br) {DeleteObject(br); br=0;}

				//render text
				//if (b_themed)
				//	DrawThemeText(m_theme, hdc_mem, LVP_LISTITEM, sel ? (GetFocus() == wnd_playlist ? LIS_SELECTED : LIS_SELECTEDNOTFOCUS) : LIS_NORMAL, L"test", 4, 0, 0, &draw);
				//else
					ui_helpers::text_out_colours_tab(hdc_mem, temp, temp.length(),  2, 1,&draw, sel, colourfore, TRUE,true, (cfg_ellipsis != 0), (ui_helpers::alignment)columns[c]->align);


				if (colours.use_frame_left)
				{
					HPEN pen = CreatePen(PS_SOLID, 1, colours.frame_left);
					HPEN pen_old = (HPEN)SelectObject(hdc_mem, pen);

					MoveToEx(hdc_mem, draw.left, draw.top, 0);
					LineTo(hdc_mem, draw.left, draw.bottom);
					SelectObject(hdc_mem, pen_old);
					DeleteObject(pen);
				}
				if (colours.use_frame_top)
				{
					HPEN pen = CreatePen(PS_SOLID, 1, colours.frame_top);
					HPEN pen_old = (HPEN)SelectObject(hdc_mem, pen);

					MoveToEx(hdc_mem, draw.left, draw.top, 0);
					LineTo(hdc_mem, draw.right, draw.top);
					SelectObject(hdc_mem, pen_old);
					DeleteObject(pen);
				}
				if (colours.use_frame_right)
				{
					HPEN pen = CreatePen(PS_SOLID, 1, colours.frame_right);
					HPEN pen_old = (HPEN)SelectObject(hdc_mem, pen);

					MoveToEx(hdc_mem, draw.right-1, draw.top, 0);
					LineTo(hdc_mem, draw.right-1, draw.bottom);
					SelectObject(hdc_mem, pen_old);
					DeleteObject(pen);
				}
				if (colours.use_frame_bottom)
				{
					HPEN pen = CreatePen(PS_SOLID, 1, colours.frame_bottom);
					HPEN pen_old = (HPEN)SelectObject(hdc_mem, pen);

					MoveToEx(hdc_mem, draw.right-1, draw.bottom-1, 0);
					LineTo(hdc_mem, draw.left-1, draw.bottom-1);
					SelectObject(hdc_mem, pen_old);
					DeleteObject(pen);
				}
				i++;
			}
		}



		//draw focus frame
		if ((n + scroll_item_offset) == focus)
		{
			if (m_always_show_focus || ( !(SendMessage(get_wnd(), WM_QUERYUISTATE, NULL, NULL) & UISF_HIDEFOCUS) && b_focused) )
			{
				RECT rc_focus = item;
				if (m_theme && p_helper.get_themed() && IsThemePartDefined(m_theme, LVP_LISTITEM, LISS_SELECTED))
					InflateRect(&rc_focus, -1, -1);
				if (!p_helper.get_bool(cui::colours::bool_use_custom_active_item_frame))
				{
					DrawFocusRect(hdc_mem, &rc_focus);
				}
				else
				{
					br = CreateSolidBrush(p_helper.get_colour(cui::colours::colour_active_item_frame));
					FrameRect(hdc_mem, &rc_focus, br);
					DeleteObject(br);
				}
			}
		}

		item.top += item_height;
		item.bottom += item_height;
	}


	BitBlt(dc,	item_area.left, item_area.top, item_area.right-item_area.left, item_area.bottom-item_area.top,
		hdc_mem, 0, 0, SRCCOPY);

	if (font_old) SelectObject(hdc_mem, font_old);
	SelectObject(hdc_mem, hbm_old);
	DeleteObject(hbm_mem);
	DeleteDC(hdc_mem);

	return true;
}

int playlist_view::hittest_item(int x, int y, bool check_in_column)
{
	RECT playlist;
	get_playlist_rect(&playlist);
	POINT pt = {x,y};

	if (check_in_column && !PtInRect(&playlist, pt) || !check_in_column && (pt.y < playlist.top || pt.y > playlist.bottom)) return -1;

	if (check_in_column && (x + horizontal_offset > get_columns_total_width())) return -1;



	int item_height = get_item_height();
	int idx = ((y - get_header_height()) / item_height) + scroll_item_offset;
	static_api_ptr_t<playlist_manager> playlist_api;

	if( idx >= 0 && idx <playlist_api->activeplaylist_get_item_count())
		return idx;
	else
		return -1;
}

int playlist_view::hittest_item_no_scroll(int x, int y, bool check_in_column)
{

	POINT pt = {x,y};

	if (check_in_column && (x + horizontal_offset > get_columns_total_width())) return -1;



	int item_height = get_item_height();
	int idx = ((y - get_header_height()) / item_height);

	return idx;
}

int playlist_view::hittest_column(int x, long &width)
{
	int column = 0;

	pfc::array_t<int, pfc::alloc_fast_aggressive> widths;

	int tw = get_column_widths(widths);

	int cx=0,
		count = widths.get_size();

	if (x + horizontal_offset > tw || x < 0) return -1;

	while(cx < tw && column + 1 < count && cx < x + horizontal_offset)
	{
		cx += widths[column];
		column++;
	}

	if (cx > x + horizontal_offset && column > 0)
	{
		column--;
		cx -= widths[column];
	}

	width = cx - horizontal_offset;

	return column;
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
	//however, this is gonna be wrong 4 centred columns ...
}

/* no range checks here !*/
void playlist_view::process_keydown(int offset, bool alt_down, bool prevent_redrawing, bool repeat)
{
	static_api_ptr_t<playlist_manager> playlist_api;

	int focus = playlist_api->activeplaylist_get_focus_item();
	int count = playlist_api->activeplaylist_get_item_count();

	//	if (focus < 0) focus =0;
	//	if (focus >= count) focus = count-1;

	//	int alt_offset = offset;

	if ((focus + offset) < 0) offset -= (focus + offset);
	if ((focus + offset) >= count) offset = (count-1-focus);

	bool focus_sel = playlist_api->activeplaylist_is_item_selected(focus);
	if (prevent_redrawing)
		uSendMessage(wnd_playlist, WM_SETREDRAW, FALSE, 0);

	if ((GetKeyState(VK_SHIFT) & KF_UP) && (GetKeyState(VK_CONTROL) & KF_UP))
	{
		if (!repeat) playlist_api->activeplaylist_undo_backup();
		playlist_api->activeplaylist_move_selection(offset);
	}
	else if ((GetKeyState(VK_CONTROL) & KF_UP))
		playlist_api->activeplaylist_set_focus_item(focus + offset);
	else if (GetKeyState(VK_SHIFT) & KF_UP)
	{
		set_sel_range(cfg_alternative_sel ? focus : g_shift_item_start, focus+offset, (cfg_alternative_sel != 0), (cfg_alternative_sel ? !focus_sel : false));
		playlist_api->activeplaylist_set_focus_item(focus+offset);
	}
	else
	{
		//		console::info(pfc::string_printf("%i",focus+offset));
		set_sel_single(focus+offset, false, true, (GetKeyState(VK_SHIFT) & KF_UP) ? false : true);
	}

	if (prevent_redrawing)
	{
		uSendMessage(wnd_playlist, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
	}
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

		RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
	}

	return rv;
}

unsigned int playlist_view::calculate_header_height()
{
	unsigned rv = 0;
	if (wnd_header)
	{
		HFONT font = (HFONT)uSendMessage(wnd_header, WM_GETFONT, 0, 0);
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
		rv = uGetFontHeight(g_font) + cfg_height;
		if (rv < 1) rv = 1;
	}
	return rv;
}

LRESULT playlist_view::CreateToolTip(const char * text)
{
	if (g_tooltip) {DestroyWindow(g_tooltip); g_tooltip=0;}

	DLLVERSIONINFO2 dvi;
	bool b_comctl_6 = SUCCEEDED(win32_helpers::get_comctl32_version(dvi)) && dvi.info1.dwMajorVersion >= 6;

	g_tooltip = CreateWindowEx(b_comctl_6?WS_EX_TRANSPARENT:0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_NOPREFIX ,		
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, wnd_playlist, 0, core_api::get_my_instance(), NULL);

	//	toolproc = (WNDPROC)uSetWindowLong(g_tooltip,GWL_WNDPROC,(LPARAM)(TooltipHook));


	//	uSendMessage(g_tooltip, CCM_SETVERSION, (WPARAM) COMCTL32_VERSION, 0);

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

void playlist_view::move_header(bool redraw, bool update)
{

	RECT rc_playlist, rc_header;
	GetClientRect(wnd_playlist, &rc_playlist);
	GetRelativeRect(wnd_header, wnd_playlist, &rc_header);
	int header_height = calculate_header_height();

	if (rc_header.left != 0-horizontal_offset ||
		rc_header.top != 0 ||
		rc_header.right - rc_header.left != rc_playlist.right-rc_playlist.left + horizontal_offset ||
		rc_header.bottom - rc_header.top != header_height)
	{
		uSendMessage(wnd_header, WM_SETREDRAW, FALSE, 0);
		if (rc_header.bottom - rc_header.top != header_height)
		{
			RECT playlist,redraw;
			get_playlist_rect(&playlist);
			ScrollWindowEx(wnd_playlist, 0, (header_height - rc_header.bottom), &playlist, &playlist, 0, &redraw, 0);
			//			RedrawWindow(wnd_playlist,&redraw,0,RDW_INVALIDATE|RDW_UPDATENOW);
		}
		SetWindowPos(wnd_header, 0, 0-horizontal_offset, 0,rc_playlist.right-rc_playlist.left + horizontal_offset, header_height, SWP_NOZORDER);
		if (cfg_nohscroll && update) rebuild_header(false);
		uSendMessage(wnd_header, WM_SETREDRAW, TRUE, 0);
		if (redraw) RedrawWindow(wnd_header, 0, 0, RDW_UPDATENOW|RDW_INVALIDATE);
	}

}

void playlist_view::create_header(bool visible)
{
	if (wnd_header) {DestroyWindow(wnd_header); wnd_header = 0;}
	if (cfg_header) 
	{
		wnd_header = CreateWindowEx(0, WC_HEADER, _T("Playlist display column titles"),
			WS_CHILD | (visible ? WS_VISIBLE : 0) |  HDS_HOTTRACK |HDS_DRAGDROP| HDS_HORZ | (/*nohscroll ? 0 : */HDS_FULLDRAG) | (cfg_header_hottrack ? HDS_BUTTONS : 0),
			0, 0, 0, 0, wnd_playlist, HMENU(5001), core_api::get_my_instance(), NULL);

		on_header_font_change();
		rebuild_header();		
	}
	if (visible) RedrawWindow(wnd_playlist, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
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

bool playlist_view::process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist, bool keyb)
{
	static_api_ptr_t<keyboard_shortcut_manager> keyboard_api;

	if (msg == WM_SYSKEYDOWN)
	{
		if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp)) 
		{
			return true;
		}
	}
	else if (msg == WM_KEYDOWN)
	{
		if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp)) 
		{
			return true;
		}
		if (wp == VK_TAB)
		{
			uie::window::g_on_tab(GetFocus());
#if 0
			HWND wnd_focus = GetFocus();
			HWND wnd_temp = GetParent(wnd_focus);

			while (GetWindowLong(wnd_temp, GWL_EXSTYLE) & WS_EX_CONTROLPARENT)
			{
				wnd_temp = GetParent(wnd_temp);
			}

			HWND wnd_next = GetNextDlgTabItem(wnd_temp, wnd_focus, (GetAsyncKeyState(VK_SHIFT) & KF_UP) ? TRUE :  FALSE);

			if (wnd_next && wnd_next != wnd_focus) SetFocus(wnd_next);
#endif

		}
	}
	return false;
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

LRESULT playlist_view::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_NCCREATE:
		wnd_playlist = wnd;
		initialised = true;
		list_playlist.add_item(this);
		g_playlist_message_window.add_ref();
		break;
	case WM_CREATE:
		{
			pfc::com_ptr_t<IDropTarget_playlist> IDT_playlist = new IDropTarget_playlist(this);
			RegisterDragDrop(wnd, IDT_playlist.get_ptr());
			if (true)
			{
				m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"ListView") : NULL;
				SetWindowTheme(wnd, L"Explorer", NULL);
			}
			m_always_show_focus = config_object::g_get_data_bool_simple(standard_config_objects::bool_playback_follows_cursor, false);
			on_playlist_font_change();
			create_header(true);
			drawing_enabled=true;
			m_cache.initialise();
		}
		return 0;
	case WM_DESTROY:
		m_edit_save = false;
		exit_inline_edit();
		m_cache.deinitialise();
		RevokeDragDrop(wnd);
		SendMessage(wnd, WM_SETFONT, 0, 0);
		SendMessage(wnd_header, WM_SETFONT, 0, 0);
		{
			if (m_theme) CloseThemeData(m_theme);
			m_theme = NULL;
		}
		m_selection_holder.release();
		break;
	case WM_NCDESTROY:
		g_playlist_message_window.release();
		wnd_playlist = 0;
		initialised = false;
		list_playlist.remove_item(this);
		m_shown = false;
		//		if (!list_playlist.get_count())
		//		{
		//			g_playlist_entries.rebuild_all();
		//		}
		break;
	case WM_THEMECHANGED:
		{
			if (m_theme) CloseThemeData(m_theme);
			m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"ListView") : 0;
		}
		break;
	case WM_SHOWWINDOW:
		if (wp == TRUE && lp == 0 && !m_shown)
		{
			static_api_ptr_t<playlist_manager> playlist_api;
			ensure_visible(playlist_api->activeplaylist_get_focus_item());
			m_shown = true;
		}
		break;
	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
			if (!(lpwp->flags & SWP_NOSIZE))
			{
				on_size(lpwp->cx, lpwp->cy);
			}
		}
		break;
	case WM_ERASEBKGND:
		return TRUE;
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps; 
			HDC dc_paint = BeginPaint(wnd, &ps);

			RECT rc_update, rc_playlist;
			get_playlist_rect(&rc_playlist);


			rc_update=ps.rcPaint;
			if (rc_update.top<rc_playlist.top) rc_update.top=rc_playlist.top;
			if (rc_update.bottom >= rc_update.top)
			{

				int item_height= get_item_height();

				int start_item = (rc_update.top-rc_playlist.top)/item_height;
				int end_item = (rc_update.bottom-rc_playlist.top)/item_height;

				if (((end_item-start_item)+1)*item_height < rc_update.bottom-rc_update.top) end_item++;
				{
					draw_items(dc_paint, start_item, 1+(end_item-start_item));
				}		
			}
			EndPaint(wnd, &ps);
		}
		return 0;
	case WM_SETREDRAW:
		drawing_enabled = (wp != 0);
		return 0;
	case WM_MOUSEACTIVATE:
		if (GetFocus() != wnd)
			m_no_next_edit = true;
		return MA_ACTIVATE;
	case WM_UPDATEUISTATE:
		RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE);
		break;
	case WM_KILLFOCUS:
		RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
		m_selection_holder.release();
		break;
	case WM_SETFOCUS:
		//if (msg == WM_SETFOCUS && (HWND)wp != wnd)
			//m_no_next_edit = true;
		RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
		m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
		m_selection_holder->set_playlist_selection_tracking();
		break;
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		{
			static_api_ptr_t<playlist_manager> playlist_api;
			uie::window_ptr p_this = this;
			//DWORD vk_slash = VkKeyScan('/');
			if (wp == VK_CONTROL) g_drag_lmb = true;
			if (m_prevent_wm_char_processing = process_keydown(msg, lp, wp, true)) return 0;
			else 
			{
				SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
				if (wp == VK_HOME ||wp == VK_DOWN || wp == VK_END || wp == VK_PRIOR || wp == VK_NEXT ||  wp == VK_UP) 
				{
					int focus = playlist_api->activeplaylist_get_focus_item();
					int total = playlist_api->activeplaylist_get_item_count();

					if ((wp == VK_HOME || wp == VK_PRIOR || wp == VK_UP))
					{
						//	if (focus == 0) return 0;
					}
					if ((wp == VK_END || wp == VK_NEXT || wp == VK_DOWN))
					{
						//	if (focus == total - 1) return 0;
					}

					SCROLLINFO si;
					memset(&si, 0, sizeof(si));
					si.cbSize = sizeof(si);

					si.fMask = SIF_PAGE|SIF_POS;
					GetScrollInfo(wnd_playlist, SB_VERT, &si);

					int offset=0;
					int scroll = scroll_item_offset;

					if (wp == VK_HOME)
						scroll = 0;
					else if (wp == VK_PRIOR && focus == scroll_item_offset) 
						scroll -= si.nPage;
					else if (wp == VK_UP)
					{
						if (focus <= scroll_item_offset) 
							scroll = focus -1 ;
						else if (focus > si.nPos + si.nPage - 1) 
							scroll = focus - 1 - si.nPage +1 ;
					}
					else if (wp == VK_DOWN)
					{
						if (focus < scroll_item_offset) 
							scroll = focus + 1;
						else if (focus >= si.nPos + si.nPage - 1) 
							scroll = focus +1 - si.nPage + 1 ;
					}
					else if (wp == VK_END) 
						scroll = total-1;
					else if (wp == VK_NEXT && focus == si.nPos + si.nPage - 1) 
						scroll += si.nPage;

					drawing_enabled = false;

					si.nPos = scroll;
					si.fMask = SIF_POS;
					scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &si, true);

					if (wp == VK_HOME)
						offset = 0-focus;
					else if (wp == VK_PRIOR) 
						offset = scroll_item_offset-focus;
					else if (wp == VK_END) 
						offset = total-focus-1;
					else if (wp == VK_NEXT)
						offset = get_last_viewable_item()-focus;
					else if (wp == VK_DOWN)
						offset = 1;
					else if (wp == VK_UP)
						offset = -1;


					//if (offset) 
						process_keydown(offset,((HIWORD(lp) & KF_ALTDOWN) != 0), drawing_enabled, (HIWORD(lp) & KF_REPEAT) != 0);
					drawing_enabled = true;

					RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);

					return 0;
				}
				else if (wp == VK_SPACE)
				{
					int focus = playlist_api->activeplaylist_get_focus_item();
					set_sel_single(focus, true, false, false);
					return 0;
				}
				else if (wp == VK_RETURN)
				{
					bool ctrl_down = 0!=(GetKeyState(VK_CONTROL) & KF_UP);
					int focus = playlist_api->activeplaylist_get_focus_item();
					unsigned active = playlist_api->get_active_playlist();
					if (ctrl_down)
					{
						if (active != -1 && focus != -1)
							playlist_api->queue_add_item_playlist(active, focus);
					}
					else
					{
						//					playlist_api->set_playing_playlist(active);
						unsigned focus = playlist_api->activeplaylist_get_focus_item();
						//unsigned active = playlist_api->get_active_playlist();
						//playlist_api->playlist_set_playback_cursor(active, focus);
						playlist_api->activeplaylist_execute_default_action(focus);
						//static_api_ptr_t<play_control>()->play_start(play_control::track_command_settrack);
					}
					return 0;
				}
				else if (wp == VK_SHIFT)
				{
					if (!(HIWORD(lp) & KF_REPEAT)) g_shift_item_start = playlist_api->activeplaylist_get_focus_item();
				}
				else if (wp == VK_F2)
				{
					unsigned count = g_get_cache().active_column_get_active_count();
					if (count)
					{
						unsigned focus = playlist_api->activeplaylist_get_focus_item();
						if (focus != pfc_infinite)
						{
							t_size i, pcount = playlist_api->activeplaylist_get_item_count();
							bit_array_bittable sel(pcount);
							playlist_api->activeplaylist_get_selection_mask(sel);

							pfc::list_t<t_size> indices;
							indices.prealloc(32);
							for (i=0; i<pcount; i++)
								if (sel[i]) indices.add_item(i);

							/*t_size start = focus, end = focus;

							if (sel[start] && pcount)
							{
								while (start>0 && sel[start-1]) start--;
								while (end<pcount-1 && sel[end+1]) end++;
							}*/

							unsigned count = g_get_cache().active_column_get_active_count();
							unsigned column;
							for (column=0; column<count; column++)
							{
								if (!g_get_columns()[g_get_cache().active_column_active_to_actual(column)]->edit_field.is_empty())
								{
									//create_inline_edit_v2(start, end-start+1, column);
									create_inline_edit_v2(indices, column);
									break;
								}
							}
						}
					}
				}
				else if (wp == VK_DELETE)
				{
					playlist_api->activeplaylist_undo_backup();
					playlist_api->activeplaylist_remove_selection();
				}
				else if (wp == VK_F3)
				{
					standard_commands::main_playlist_search();
				}
				/*else if (vk_slash != -1 && wp == LOWORD(vk_slash))
				{
					HWND wnd_search = m_searcher.create(wnd);
					on_size();
					ShowWindow(wnd_search, SW_SHOWNORMAL);
					;
				}*/
			}
		}
		break;
	case WM_CHAR:
		if (!m_prevent_wm_char_processing)
		{
			//if (!(HIWORD(lp) & KF_REPEAT))
			{
				if ((GetKeyState(VK_CONTROL) & KF_UP))
				{
					static_api_ptr_t<playlist_manager> playlist_api;
					if (wp == 1) //Ctrl-A
					{
						playlist_api->activeplaylist_set_selection(bit_array_true(), bit_array_true());
						return 0;
					}
					else if (wp == 26) //Ctrl-Z
					{
						playlist_api->activeplaylist_undo_restore();
						return 0;
					}
					else if (wp == 25) //Ctrl-Y
					{
						playlist_api->activeplaylist_redo_restore();
						return 0;
					}
					else if (wp == 24) //Ctrl-X
					{
						playlist_utils::cut();
						return 0;
					}
					else if (wp == 3) //Ctrl-C
					{
						playlist_utils::copy();
						return 0;
					}
					else if (wp == 6) //Ctrl-F
					{
						standard_commands::main_playlist_search();
						return 0;
					}
					else if (wp == 22) //Ctrl-V
					{
						playlist_utils::paste(wnd);
						return 0;
					}
				}	
			}
		}
		break;
	case WM_KEYUP:
		if (process_keydown(msg, lp, wp, true)) return 0;
		break;
	case WM_SYSKEYUP:
		if (process_keydown(msg, lp, wp, true)) return 0;
		break;
	case WM_SYSKEYDOWN:
		{
			uie::window_ptr p_this = this;
			if (m_prevent_wm_char_processing = process_keydown(msg, lp, wp, true)) return 0;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			if (0 && g_tooltip) 
			{
				MSG message;
				memset(&message, 0, sizeof(MSG));
				message.hwnd = wnd;
				message.message = msg;
				message.wParam = wp;
				message.lParam = lp;

				uSendMessage(g_tooltip, TTM_RELAYEVENT, 0, (LPARAM)&message);
			}
			bool b_was_focused = GetFocus() == wnd;
			if (!b_was_focused)
				m_no_next_edit = true;
//#ifdef INLINE_EDIT
			exit_inline_edit();
			//			g_no_next_edit = false;
//#endif
			dragged = false;
			SetFocus(wnd);
			SetCapture(wnd);

			static_api_ptr_t<playlist_manager> playlist_api;
			g_drag_lmb = true;
			int focus = playlist_api->activeplaylist_get_focus_item();

			drag_start_lmb.x = GET_X_LPARAM(lp);
			drag_start_lmb.y = GET_Y_LPARAM(lp);

			int item_height = get_item_height();
			int idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
			//		int idx = ((GET_Y_LPARAM(lp) -get_header_height()) / item_height) + scroll_item_offset;
			//		if( idx >= 0 && idx <playlist_api->activeplaylist_get_item_count()  && GET_X_LPARAM(lp) < g_playlist_entries.get_total_width_actual())

			if( idx >= 0)
			{

				//		playlist_oper * playlist_api = playlist_api;
				//				playlist_api->set_playback_cursor(idx);
//#ifdef INLINE_EDIT
				m_prev_sel = (playlist_api->activeplaylist_is_item_selected(idx) && !m_wnd_edit && (playlist_api->activeplaylist_get_selection_count(2) == 1));
//#endif

				if (!is_visible(idx)) SendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0),0);

				if (wp & MK_CONTROL && wp & MK_SHIFT)
				{
					playlist_api->activeplaylist_move_selection(idx-focus);
					dragged = true;
					drag_type = 0;
				}
				else if (wp & MK_SHIFT)
				{
					drag_type = 2; dragitem = idx,dragstartitem=idx;

					int n=(cfg_alternative_sel ? focus : g_shift_item_start),t = idx;
					bool focus_sel = playlist_api->activeplaylist_is_item_selected(focus);


					set_sel_range(n, t, (cfg_alternative_sel != 0), (cfg_alternative_sel ? !focus_sel : false));
					playlist_api->activeplaylist_set_focus_item(idx);

					dragged = true;

				}
				else if (wp & MK_CONTROL)
				{
					/*			drag_type = 2; dragitem = idx,dragstartitem=idx;

					set_sel_single(idx, false, true, false);

					dragged = true;*/

				}
				else if (playlist_api->activeplaylist_is_item_selected(idx))
				{
					drag_type = 1; dragitem = idx,dragstartitem=idx;
					playlist_api->activeplaylist_undo_backup();
					playlist_api->activeplaylist_set_focus_item(idx);
					dragged = false;
				}
				else
				{
					drag_type = 2; dragitem = idx,dragstartitem=idx;//item irrelevant actually;

					set_sel_single(idx, false, true, true);

					/*			bit_array_bittable mask(playlist_api->activeplaylist_get_item_count());
					//		playlist_api->activeplaylist_is_item_selected_mask(mask);
					int n, t = playlist_api->activeplaylist_get_item_count();
					for (n = 0;n <t;n++) { if (n==idx) mask.set(n, true); else mask.set(n, false); }

					console::info("crap");
					playlist_api->set_sel_mask(mask);
					playlist_api->activeplaylist_set_focus_item(idx);*/

					dragged = false;
				}
			}
			else
			{
				//			console::info("wow");
				//				bit_array_bittable mask(playlist_api->activeplaylist_get_item_count());
				playlist_api->activeplaylist_set_selection(bit_array_true(), bit_array_false());
				dragged = true;
				drag_type = 0;
			}
		}

		break;
	case WM_RBUTTONUP:
		m_no_next_edit = false;
		break;
	case WM_MBUTTONUP:
		{
			m_no_next_edit = false;
			unsigned idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
			playlist_mclick_actions::run(cfg_playlist_middle_action, idx!=-1,idx);
		}
		break;

	case WM_LBUTTONUP:
		{
			if (0 && g_tooltip) 
			{
				MSG message;
				memset(&message, 0, sizeof(MSG));
				message.hwnd = wnd;
				message.message = msg;
				message.wParam = wp;
				message.lParam = lp;

				uSendMessage(g_tooltip, TTM_RELAYEVENT, 0, (LPARAM)&message);
			}
			ReleaseCapture();
			g_drag_lmb = false;
			int idx = hittest_item(GET_X_LPARAM(lp),GET_Y_LPARAM(lp),true);   //((GET_Y_LPARAM(lp) -get_header_height()) / get_item_height()) + scroll_item_offset;
			static_api_ptr_t<playlist_manager> playlist_api;
			if (!dragged) 
			{
				if (wp & MK_CONTROL)
				{
					//			int idx_down = hittest_item(drag_start_lmb.x, drag_start_lmb.y);
					if (idx >= 0) set_sel_single(idx, true, true, false);
				}
				else
				{

					//				int item_height = get_item_height();

					//			int idx = ((GET_Y_LPARAM(lp) - get_header_height()) / item_height) + scroll_item_offset;
					if( idx >= 0 /*&& idx < playlist_api->activeplaylist_get_item_count() && (GET_X_LPARAM(lp) < g_playlist_entries.get_total_width_actual())*/)
					{



						if (!m_no_next_edit && cfg_inline_edit && playlist_api->activeplaylist_is_item_selected(idx) && m_prev_sel /*&& !dragged*/)
						{
							//if (m_no_next_edit && GetCapture() == wnd) ReleaseCapture();

							{
								exit_inline_edit();
								if (main_window::config_get_inline_metafield_edit_mode() != main_window::mode_disabled)
								{
									m_edit_index = idx;
									long width;
									m_edit_column = hittest_column(GET_X_LPARAM(lp), width);
									if (m_edit_column >= 0 && !g_get_columns()[g_get_cache().active_column_active_to_actual(m_edit_column)]->edit_field.is_empty())
									{
										m_edit_timer = (SetTimer(wnd, EDIT_TIMER_ID, GetDoubleClickTime(), 0) != 0);
									}
								}
							}

						}

						int focus = playlist_api->activeplaylist_get_focus_item();
						set_sel_single(focus, false, false,  true);
					}


				}
			}
			dragged = true;
			drag_type = 0;
			dragstartitem=0;
			dragitem=0;
//#ifdef INLINE_EDIT
			m_no_next_edit = false;
//#endif
		}
		break;
	case WM_MOUSEMOVE:
		{
		if (0 && g_tooltip) 
		{
			MSG message;
			memset(&message, 0, sizeof(MSG));
			message.hwnd = wnd;
			message.message = msg;
			message.wParam = wp;
			message.lParam = lp;

			uSendMessage(g_tooltip, TTM_RELAYEVENT, 0, (LPARAM)&message);
		}
		const unsigned cx_drag = (unsigned)abs(GetSystemMetrics(SM_CXDRAG));
		const unsigned cy_drag = (unsigned)abs(GetSystemMetrics(SM_CYDRAG));
		if ( !g_dragging && ((g_dragging1 && wp & MK_RBUTTON  &&( abs(drag_start.x - GET_X_LPARAM(lp)) > cx_drag || abs(drag_start.y - GET_Y_LPARAM(lp)) > cy_drag)) || ( g_drag_lmb &&( wp & MK_LBUTTON) && (wp & MK_CONTROL) && (abs(drag_start_lmb.x - GET_X_LPARAM(lp)) > 3 || abs(drag_start_lmb.y - GET_Y_LPARAM(lp)) > 3))) )
		{
			static_api_ptr_t<playlist_manager> playlist_api;
			metadb_handle_list data;
			playlist_api->activeplaylist_get_selected_items(data);
			if (data.get_count() > 0)
			{
				static_api_ptr_t<playlist_incoming_item_filter> incoming_api;
				IDataObject * pDataObject = incoming_api->create_dataobject(data);
				if (pDataObject)
				{
					//RegisterClipboardFormat(_T("foo_ui_columns");

					if (g_tooltip) {DestroyWindow(g_tooltip); g_tooltip=0;last_idx = -1; last_column = -1;}
					DWORD blah;
					{
						pfc::com_ptr_t<IDropSource_playlist> p_IDropSource_playlist = new IDropSource_playlist(this);
						DoDragDrop(pDataObject,p_IDropSource_playlist.get_ptr(),DROPEFFECT_COPY,&blah);
					}
					pDataObject->Release();
				}
			}
			data.remove_all();
			g_dragging = false;
			g_dragging1 = false;
			g_drag_lmb = false;
			if (wp & MK_LBUTTON)
			{
				dragged = true;
				drag_type = 0;
				dragstartitem=0;
				dragitem=0;
			}
		}




		if (cfg_tooltip && ( GET_Y_LPARAM(lp) > get_header_height()))
		{
			int item_height = get_item_height();
			int idx = hittest_item( GET_X_LPARAM(lp), GET_Y_LPARAM(lp) );
			long cx;
			int column=hittest_column(GET_X_LPARAM(lp), cx);
			//			unsigned act_col = g_cache.active_column_active_to_actual(column);

			if (column >= 0 && idx >= 0)
			{
				if (last_idx != (idx) || last_column != column)
				{
					if (!cfg_tooltips_clipped || is_item_clipped(idx, column))
					{
						pfc::string8 src;
						g_cache.active_get_display_name(idx, column,src);
						pfc::string8 temp;
						titleformat_compiler::remove_color_marks(src, temp);
						temp.replace_char(9, 0x20);
						CreateToolTip(temp);
					}
					else {DestroyWindow(g_tooltip); g_tooltip=0;last_idx = -1; last_column = -1;}

					POINT a;
					a.x = cx + 3;
					a.y = (idx-scroll_item_offset) * item_height + get_header_height();
					ClientToScreen(wnd_playlist, &a);

					tooltip.top = a.y;
					tooltip.bottom = a.y + item_height;
					tooltip.left = a.x;
					tooltip.right = a.x + get_column_width(column);

				}
				last_idx = idx;
				last_column = column;
			}
			else {DestroyWindow(g_tooltip); g_tooltip=0;last_idx = -1; last_column = -1;}
		}


		if (drag_type && (wp & MK_LBUTTON) && !(GetKeyState(VK_SHIFT) & KF_UP) && !(GetKeyState(VK_CONTROL) & KF_UP))
		{
			RECT rc;
			get_playlist_rect(&rc);
			static_api_ptr_t<playlist_manager> playlist_api;

			int total = playlist_api->activeplaylist_get_item_count();

			int item_height = get_item_height();
			int valid_idx = hittest_item(GET_X_LPARAM(lp),GET_Y_LPARAM(lp),false);
			int idx = hittest_item_no_scroll(GET_X_LPARAM(lp),GET_Y_LPARAM(lp),false);
			//    (GET_Y_LPARAM(lp) - get_header_height()) / (item_height);

			int items_count = ((rc.bottom-rc.top)/item_height) + 1;


			if ((idx + scroll_item_offset) != dragitem || GET_Y_LPARAM(lp) < get_header_height()) //(idx + scroll_item_offset) < playlist_api->activeplaylist_get_item_count()
			{
				if (idx >= items_count-1)
				{

					bool need_redrawing = false;

					int focus = playlist_api->activeplaylist_get_focus_item();

					SCROLLINFO si;
					memset(&si, 0, sizeof(si));
					si.cbSize = sizeof(si);
					si.fMask = SIF_POS;
					GetScrollInfo(wnd_playlist, SB_VERT, &si);

					int old_offset = si.nPos;
					si.nPos += 3;

					scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &si, true);

					if (old_offset != scroll_item_offset) need_redrawing = true;

					int t = scroll_item_offset+items_count-2; //n=dragitem,

					if (t > total) t = total-1;


					if (t != dragitem)
					{

						drawing_enabled = false;
						if (drag_type == 1)
							playlist_api->activeplaylist_move_selection((rc.bottom-rc.top)/item_height + scroll_item_offset - focus - 1);
						else if (drag_type == 2) 
						{

							set_sel_range(dragstartitem, t, false);
							playlist_api->activeplaylist_set_focus_item(t);
						}

						dragitem = t;
						drawing_enabled = true;
						need_redrawing = true;

					}
					if (need_redrawing) RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);


				} 
				else if (idx < 0 || GET_Y_LPARAM(lp) < get_header_height() || GET_Y_LPARAM(lp) < 0)
				{


					int focus = playlist_api->activeplaylist_get_focus_item();

					bool need_redrawing = false;

					SCROLLINFO si;
					memset(&si, 0, sizeof(si));
					si.cbSize = sizeof(si);
					si.fMask = SIF_POS;
					GetScrollInfo(wnd_playlist, SB_VERT, &si);
					int old_offset = si.nPos;
					si.nPos -= 3;
					scroll_item_offset = SetScrollInfo(wnd_playlist, SB_VERT, &si, true);

					if (old_offset != scroll_item_offset) need_redrawing = true;

					if (dragitem != scroll_item_offset)
					{
						drawing_enabled = false;
						if (drag_type == 1)
							playlist_api->activeplaylist_move_selection(scroll_item_offset - focus);
						else if (drag_type == 2) 
						{

							set_sel_range(dragstartitem, scroll_item_offset, false);
							playlist_api->activeplaylist_set_focus_item(scroll_item_offset);
							RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
						}

						dragitem = scroll_item_offset;
						drawing_enabled = true;
						need_redrawing = true;
					}

					if (need_redrawing) RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);


				} 
				else
				{
					int focus = playlist_api->activeplaylist_get_focus_item();

					if (drag_type == 1)
						playlist_api->activeplaylist_move_selection(idx + scroll_item_offset - focus);
					else if (drag_type == 2) 
					{
						if (valid_idx >= 0)
						{
							drawing_enabled = false;
							set_sel_range(dragstartitem, valid_idx, false);
							playlist_api->activeplaylist_set_focus_item(valid_idx);
							drawing_enabled = true;
							RedrawWindow(wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
						}

					}

					dragitem = valid_idx;
					dragged = true;
				}
			}

		} else if (!(wp & MK_LBUTTON)) drag_type = 0;
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			int idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), true);

			if (idx >= 0)
			{
//#ifdef INLINE_EDIT
				exit_inline_edit();
				m_no_next_edit = true;
//#endif
				//if (!is_visible(idx)) uSendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0),0);

#if 0
				// DEATH's code
	case WM_LBUTTONDBLCLK:
		{
			int idx = item_from_point((short)HIWORD(lp));
			if (idx>=0 && idx<(int)m_api->activeplaylist_get_item_count())
			{
				m_api->activeplaylist_set_focus_item(idx);
				static_api_ptr_t<play_control>()->play_start(play_control::TRACK_COMMAND_SETTRACK);
			}
		}
		return 0;
#endif
				static_api_ptr_t<playlist_manager> playlist_api;
		//unsigned active = playlist_api->get_active_playlist();
		//				playlist_api->set_playing_playlist(active);
		//playlist_api->playlist_set_playback_cursor(active, idx);
		//playlist_api->queue_flush();
				unsigned focus = playlist_api->activeplaylist_get_focus_item();
				playlist_api->activeplaylist_execute_default_action(focus);

			}
			else if (cfg_playlist_double.get_value().m_command != pfc::guid_null)
			{
				mainmenu_commands::g_execute(cfg_playlist_double.get_value().m_command);
			}

			dragged = true;
		}

		break;
	case WM_RBUTTONDOWN:
		{
			if (wnd_playlist) SetFocus(wnd_playlist);

			g_dragging1 = true;

			drag_start.x = GET_X_LPARAM(lp);
			drag_start.y = GET_Y_LPARAM(lp);

			static_api_ptr_t<playlist_manager> playlist_api;


			//		int item_height = get_item_height();
			//		int idx = ((GET_Y_LPARAM(lp) - get_header_height()) / item_height) + scroll_item_offset;
			int idx = hittest_item(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), true);
			if (idx !=-1 && !is_visible(idx))
				SendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0),0);

			if (idx >= 0 /*&& idx < playlist_api->activeplaylist_get_item_count() && (GET_X_LPARAM(lp) < g_playlist_entries.get_total_width_actual())*/)
			{

				if (!playlist_api->activeplaylist_is_item_selected(idx)	&& !(GetKeyState(VK_CONTROL) & KF_UP))
				{
					set_sel_single(idx, false, false, true);
				}
				playlist_api->activeplaylist_set_focus_item(idx);

			}


		}

		break;
	case WM_MOUSEWHEEL:    
		{//GET_WHEEL_DELTA_WPARAM
			exit_inline_edit();
			if (1 || (wp & MK_CONTROL))
			{

				LONG_PTR style = GetWindowLongPtr(wnd_playlist,GWL_STYLE);
				if (!(style & WS_VSCROLL) || ((wp & MK_CONTROL)&&(style & WS_HSCROLL)))
				{
					if ((style & WS_HSCROLL))
					{
						SCROLLINFO si;
						memset(&si, 0, sizeof(SCROLLINFO));
						si.fMask = SIF_PAGE;
						si.cbSize = sizeof(SCROLLINFO);
						GetScrollInfo(wnd, SB_HORZ, &si);

						int new_pos = horizontal_offset;
						int old_pos = horizontal_offset;

						unsigned scroll_lines = GetNumScrollLines();

						int zDelta = short(HIWORD(wp));

						if (scroll_lines == -1)
						{
							scroll_lines = si.nPage > 1 ? si.nPage-1 : 1;
						}
						else scroll_lines *= 3;

						int delta = MulDiv(zDelta, scroll_lines, 120);

						if (!si.nPage) si.nPage++;

						if (delta < 0 && delta*-1 > si.nPage)
						{
							delta = si.nPage*-1;
							if (delta >1) delta--;
						}
						else if (delta > 0 && delta > si.nPage)
						{
							delta = si.nPage;
							if (delta >1) delta--;
						}

						scroll(scroll_horizontally, scroll_position_delta, -delta);

					}
					return 1;
				}
			}

			SCROLLINFO si;
			memset(&si, 0, sizeof(SCROLLINFO));
			si.fMask = SIF_PAGE;
			si.cbSize = sizeof(SCROLLINFO);
			GetScrollInfo(wnd, SB_VERT, &si);

			int new_pos = scroll_item_offset;
			int old_pos = scroll_item_offset;
			unsigned scroll_lines = GetNumScrollLines();

			int zDelta = short(HIWORD(wp));

			if (scroll_lines == -1)
			{
				scroll_lines = si.nPage > 1 ? si.nPage-1 : 1;
			}

			int delta = MulDiv(zDelta, scroll_lines, 120);

			if (!si.nPage) si.nPage++;

			if (delta < 0 && delta*-1 > si.nPage)
			{
				delta = si.nPage*-1;
				if (delta >1) delta--;
			}
			else if (delta > 0 && delta > si.nPage)
			{
				delta = si.nPage;
				if (delta >1) delta--;
			}

			scroll(scroll_vertically, scroll_position_delta, -delta);
		}
		return 1;		
	case WM_VSCROLL:
		{
			exit_inline_edit();
			scroll(scroll_vertically, scroll_sb, LOWORD(wp));
		}
		return 0;
	case WM_HSCROLL:
		{
			exit_inline_edit();
			scroll(scroll_horizontally, scroll_sb, LOWORD(wp));
		}
		return 0;
	case WM_MENUSELECT:
		{
			if (HIWORD(wp) & MF_POPUP)
			{
				m_status_override.release();
			}
			else 
			{
				if (g_main_menu_a.is_valid() || g_main_menu_b.is_valid())
				{
					unsigned id = LOWORD(wp);

					bool set = false;

					pfc::string8 desc;

					if (g_main_menu_a.is_valid() && id < MENU_B_BASE)
					{
						set = g_main_menu_a->get_description(id - MENU_A_BASE, desc);
					}
					else if (g_main_menu_b.is_valid())
					{
						contextmenu_node * node = g_main_menu_b->find_by_id(id - MENU_B_BASE);
						if (node) set = node->get_description(desc);
					}

					service_ptr_t<ui_status_text_override> p_status_override;

					if (set)
					{
						get_host()->override_status_text_create(p_status_override);

						if (p_status_override.is_valid())
						{
							p_status_override->override_text(desc);
						}
					}
					m_status_override = p_status_override;
				}
			}
		}
		break;
	case WM_CONTEXTMENU:
		{
		uie::window_ptr p_this_temp = this;
		if ((HWND)wp == wnd_header)
		{
			POINT pt = {(short)LOWORD(lp),(short)HIWORD(lp)};
			POINT temp;
			temp.x = pt.x;
			temp.y = pt.y;
			ScreenToClient(wnd_header, &temp);
			HDHITTESTINFO hittest;
			hittest.pt.x = temp.x;
			hittest.pt.y = temp.y;


			uSendMessage(wnd_header, HDM_HITTEST, 0, (LPARAM)&hittest);

			enum {IDM_ASC=1, IDM_DES=2, IDM_SEL_ASC, IDM_SEL_DES, IDM_AUTOSIZE, IDM_PREFS, IDM_EDIT_COLUMN, IDM_CUSTOM_BASE};

			HMENU menu = CreatePopupMenu();
			HMENU selection_menu = CreatePopupMenu();
			if (!(hittest.flags & HHT_NOWHERE)) 
			{
				uAppendMenu(menu,(MF_STRING),IDM_ASC,"&Sort ascending");
				uAppendMenu(menu,(MF_STRING),IDM_DES,"Sort &descending");
				uAppendMenu(selection_menu,(MF_STRING),IDM_SEL_ASC,"Sort a&scending");
				uAppendMenu(selection_menu,(MF_STRING),IDM_SEL_DES,"Sort d&escending");
				uAppendMenu(menu,MF_STRING|MF_POPUP,(UINT)selection_menu,"Se&lection");
				uAppendMenu(menu,(MF_SEPARATOR),0,"");
				uAppendMenu(menu,(MF_STRING),IDM_EDIT_COLUMN,"&Edit this column");
				uAppendMenu(menu,(MF_SEPARATOR),0,"");
				uAppendMenu(menu,(MF_STRING|(cfg_nohscroll ? MF_CHECKED : MF_UNCHECKED)),IDM_AUTOSIZE,"&Auto-sizing columns");
				uAppendMenu(menu,(MF_STRING),IDM_PREFS,"&Preferences");
				uAppendMenu(menu,(MF_SEPARATOR),0,"");

				pfc::string8 playlist_name;
				static_api_ptr_t<playlist_manager> playlist_api;
				playlist_api->activeplaylist_get_name(playlist_name);

				pfc::string8_fast_aggressive filter, name;

				int s,e=columns.get_count();
				for (s=0;s<e;s++)
				{
					bool add = false;
					switch(columns[s]->filter_type)
					{
					case FILTER_NONE:
						{
							add = true;
							break;
						}
					case FILTER_SHOW:
						{
							if (wildcard_helper::test(playlist_name,columns[s]->filter,true))
							{
								add = true;
								/*				g_columns.get_string(s, name, STRING_NAME);
								uAppendMenu(menu,MF_STRING|MF_CHECKED,IDM_CUSTOM_BASE+s,name);*/
							}
						}
						break;
					case FILTER_HIDE:
						{
							if (!wildcard_helper::test(playlist_name,columns[s]->filter,true))
							{
								add = true;
								/*						g_columns.get_string(s, name, STRING_NAME);
								uAppendMenu(menu,MF_STRING|MF_CHECKED,IDM_CUSTOM_BASE+s,name);*/
							}
						}
						break;
					}
					if (add)
					{
						uAppendMenu(menu,MF_STRING|(columns[s]->show ? MF_CHECKED : MF_UNCHECKED),IDM_CUSTOM_BASE+s,columns[s]->name);
					}
				}


			}
			else
			{
				uAppendMenu(menu,(MF_STRING|(cfg_nohscroll ? MF_CHECKED : MF_UNCHECKED)),IDM_AUTOSIZE,"&Auto-sizing columns");
				uAppendMenu(menu,(MF_STRING),IDM_PREFS,"&Preferences");
			}


			menu_helpers::win32_auto_mnemonics(menu);

			int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);
			DestroyMenu(menu);

			if (cmd == IDM_ASC)
			{

				g_set_sort(hittest.iItem, false);
			}
			else if (cmd == IDM_DES)
			{
				g_set_sort(hittest.iItem, true);
			}
			else if (cmd == IDM_SEL_ASC)
			{
				g_set_sort(hittest.iItem, false, true);
			}
			else if (cmd == IDM_SEL_DES)
			{
				g_set_sort(hittest.iItem, true, true);
			}
			else if (cmd == IDM_EDIT_COLUMN)
			{
				g_set_tab("Columns");
				cfg_cur_prefs_col = g_cache.active_column_active_to_actual(hittest.iItem); //get_idx
				static_api_ptr_t<ui_control>()->show_preferences(columns::config_get_playlist_view_guid());
			}
			else if (cmd == IDM_AUTOSIZE)
			{
				cfg_nohscroll = cfg_nohscroll == 0;
				update_all_windows();
				pvt::ng_playlist_view_t::g_on_autosize_change();
			}
			else if (cmd == IDM_PREFS)
			{
				static_api_ptr_t<ui_control>()->show_preferences(columns::config_get_main_guid());
			}
			else if (cmd >= IDM_CUSTOM_BASE)
			{
				if (t_size(cmd - IDM_CUSTOM_BASE) < columns.get_count())
				{
					columns[cmd - IDM_CUSTOM_BASE]->show = !columns[cmd - IDM_CUSTOM_BASE]->show; //g_columns
					//if (!cfg_nohscroll) 
					g_save_columns();
					//g_cache.flush_all();
					g_reset_columns();
					update_all_windows();
					pvt::ng_playlist_view_t::g_on_columns_change();
				}

			}
			return 0;
		}
		else if ((HWND)wp == wnd)
		{
			//DWORD mp = GetMessagePos();
			POINT px,pt = {GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
			static_api_ptr_t<playlist_manager> playlist_api;
			if (playlist_api->activeplaylist_get_selection_count(1) > 0 && 1)
			{
				if (pt.x == -1 && pt.y == -1)
				{
					int focus = playlist_api->activeplaylist_get_focus_item();
					unsigned last = get_last_viewable_item();
					if (focus == -1 || focus < scroll_item_offset || focus > last)
					{
						px.x=0;
						px.y=0;
					}
					else
					{
						RECT rc;
						get_playlist_rect(&rc);
						px.x=0;
						unsigned item_height = get_item_height();
						px.y=(focus-scroll_item_offset)*(item_height) + item_height/2 + rc.top;
					}
					pt = px;
					MapWindowPoints(wnd, HWND_DESKTOP, &pt, 1);
				}
				else
				{
					px = pt;
					ScreenToClient(wnd, &px);
					//int idx = hittest_item(px.x, px.y);
					//if (!is_visible(idx))
					//	SendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0),0);

				}
				//			int idx = hittest_item(px.x, px.y);

				enum { ID_PLAY=1, ID_CUT, ID_COPY, ID_PASTE, ID_SELECTION, ID_CUSTOM_BASE = 0x8000 };
				HMENU menu = CreatePopupMenu();//LoadMenu(core_api::get_my_instance(),MAKEINTRESOURCE(IDR_TREEPOPUP));

				service_ptr_t<mainmenu_manager> p_manager_selection;
				service_ptr_t<contextmenu_manager> p_manager_context;
				p_manager_selection = standard_api_create_t<mainmenu_manager>();
				contextmenu_manager::g_create(p_manager_context);
				if (p_manager_selection.is_valid())
				{
					p_manager_selection->instantiate(mainmenu_groups::edit_part2_selection);
					p_manager_selection->generate_menu_win32(menu,ID_SELECTION,ID_CUSTOM_BASE-ID_SELECTION,standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0);
					if (GetMenuItemCount(menu) > 0) uAppendMenu(menu,MF_SEPARATOR,0,"");
				}

				AppendMenu(menu,MF_STRING,ID_CUT,L"Cut");
				AppendMenu(menu,MF_STRING,ID_COPY,L"Copy");
				if (playlist_utils::check_clipboard())
					AppendMenu(menu,MF_STRING,ID_PASTE,L"Paste");
				AppendMenu(menu,MF_SEPARATOR,0,NULL);
				if (p_manager_context.is_valid())
				{
					const keyboard_shortcut_manager::shortcut_type shortcuts[] = {keyboard_shortcut_manager::TYPE_CONTEXT_PLAYLIST, keyboard_shortcut_manager::TYPE_CONTEXT};
					p_manager_context->set_shortcut_preference(shortcuts, tabsize(shortcuts));
					p_manager_context->init_context_playlist(standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0);

					p_manager_context->win32_build_menu(menu,ID_CUSTOM_BASE,-1);
				}
				menu_helpers::win32_auto_mnemonics(menu);
				MENU_A_BASE = ID_SELECTION;
				MENU_B_BASE = ID_CUSTOM_BASE;

				g_main_menu_a = p_manager_selection;
				g_main_menu_b = p_manager_context;

				int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);
				if (m_status_override.is_valid())
				{
					m_status_override.release();
				}

				DestroyMenu(menu);
				if (cmd)
				{
					if (cmd == ID_CUT)
					{
						playlist_utils::cut();
					}
					else if (cmd == ID_COPY)
					{
						playlist_utils::copy();
					}
					else if (cmd == ID_PASTE)
					{
						playlist_utils::paste(wnd);
					}
					else if (cmd >= ID_SELECTION && cmd<ID_CUSTOM_BASE)
					{
						if (p_manager_selection.is_valid())
						{
							p_manager_selection->execute_command(cmd - ID_SELECTION);
						}
					}
					else if (cmd >= ID_CUSTOM_BASE)
					{
						if (p_manager_context.is_valid())
						{
							p_manager_context->execute_by_id(cmd - ID_CUSTOM_BASE);
						}
					}
				}
				g_main_menu_a.release();
				g_main_menu_b.release();
			}


			//	contextmenu_manager::win32_run_menu_context_playlist(wnd, 0, config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, true) ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0);
		} 
		}
		return 0;

//#ifdef INLINE_EDIT
	case WM_PARENTNOTIFY:
		{
			if (wp == WM_DESTROY)
			{
				if (m_wnd_edit && (HWND)lp == m_wnd_edit) m_wnd_edit = 0;
			}
		}
		break;
	case MSG_KILL_INLINE_EDIT:
		exit_inline_edit();
		return 0;

#if 1
	case WM_COMMAND:
		switch (wp)
		{
		case (EN_CHANGE<<16)|667:
			{
				m_edit_changed = true;
			}
			break;
		}
		break;
#endif

	case WM_TIMER:
		{
			if (wp == EDIT_TIMER_ID)
			{
				create_inline_edit_v2(m_edit_index, m_edit_column);
				if (m_edit_timer)
				{
					KillTimer(wnd_playlist, EDIT_TIMER_ID);
					m_edit_timer = false;
				}
				return 0;
			}

		}
		break;

//#endif
	case WM_NOTIFY:
		switch (((LPNMHDR)lp)->idFrom)
		{
		case ID_PLAYLIST_TOOLTIP:
			switch (((LPNMHDR)lp)->code)
			{
			case TTN_SHOW:

				RECT rc, rc_tt;

				rc = tooltip;
				GetWindowRect(g_tooltip, &rc_tt);

				int offset = MulDiv(get_item_height() - rc_tt.bottom+rc_tt.top, 1, 2);


				rc.top += offset;




				SetWindowPos(g_tooltip,
					NULL,
					rc.left, rc.top,
					0, 0,
					SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
				return TRUE;
			}
			break;
		case 5001:
			switch (((LPNMHDR)lp)->code)
			{
			case HDN_BEGINTRACKA:
			case HDN_BEGINTRACKW:
				{
					return (cfg_nohscroll ? TRUE : FALSE);
				}
			case HDN_ENDDRAG:
				{
					if (((LPNMHEADERA) lp)->iButton == 0)
					{

						if (((LPNMHEADERA) lp)->pitem && (((LPNMHEADERA) lp)->pitem->mask & HDI_ORDER))
						{

							int from = ((LPNMHEADERA) lp)->iItem;
							int to = ((LPNMHEADERA) lp)->pitem->iOrder;
							if (to >= 0 && from != to)
							{
								int act_from = g_cache.active_column_active_to_actual(from), act_to = g_cache.active_column_active_to_actual(to);

								columns.move(act_from,act_to);
								//if (!cfg_nohscroll) 
								g_save_columns();
								g_reset_columns();
								update_all_windows();
								pvt::ng_playlist_view_t::g_on_columns_change();
							}
						}
						else
						{
						}
					}
					return (TRUE);
				}
			case HDN_DIVIDERDBLCLICK:
				if (!cfg_nohscroll)
				{
					static_api_ptr_t<playlist_manager> playlist_api;
					HDC hdc;
					hdc = GetDC(wnd_playlist);
					int size;
					pfc::string8 text;

					SelectObject(hdc, g_font);


					int w=0,n,t = playlist_api->activeplaylist_get_item_count();

					for (n=0; n<t; n++)
					{
						//	playlist_api->format_title(n, text, g_playlist_entries.get_display_spec(((LPNMHEADER)lp)->iItem), NULL);
						g_cache.active_get_display_name(n, ((LPNMHEADER)lp)->iItem,text);
						size = ui_helpers::get_text_width_color(hdc, text, text.length());
						if (size > w) w=size;
					}

					//	g_playlist_entries.get_column(((LPNMHEADER)lp)->iItem)->_set_width(w+5);
					columns[g_cache.active_column_active_to_actual(((LPNMHEADER)lp)->iItem)]->width = w+15;

					ReleaseDC(wnd_playlist, hdc);
					update_all_windows();
					g_save_columns();
					pvt::ng_playlist_view_t::g_on_column_widths_change();
				}

				return 0;
			case HDN_ITEMCLICK:
				{
					bool des = false;

					static_api_ptr_t<playlist_manager> playlist_api;

					unsigned col;
					bool descending;
					bool sorted = g_cache.active_get_playlist_sort(col, &descending);

					if (sorted && col == ((LPNMHEADER)lp)->iItem)
						des = !descending;

					g_set_sort(((LPNMHEADER)lp)->iItem, des /*, playlist_api->activeplaylist_get_selection_count(1) && cfg_sortsel != 0*/);

				}
				break;
			case HDN_ITEMCHANGED:
				{
					if (!cfg_nohscroll)
					{
						if (((LPNMHEADER)lp)->pitem->mask & HDI_WIDTH)
							columns[g_cache.active_column_active_to_actual(((LPNMHEADER)lp)->iItem)]->width =((LPNMHEADER)lp)->pitem->cxy;
						update_all_windows(wnd_header);
						g_save_columns();
						pvt::ng_playlist_view_t::g_on_column_widths_change();
					}
				}
				break;
			}
			break;
		}

	}
	return uDefWindowProc(wnd,msg,wp,lp);
}

#if 0
void playlist_view::save_inline_edit()
{
	static_api_ptr_t<metadb_io> tagger_api;

	if (m_edit_save && !m_edit_saving && !tagger_api->is_busy() && m_edit_item.is_valid())
	{
		//pfc::vartoggle_t<bool>(m_edit_saving, true);
		m_edit_saving=true;
		
		pfc::string8 text;
		uGetWindowText(m_wnd_edit, text);
		file_info_impl info;
		if (m_edit_item->get_info(info))
		{
			const char * ptr = info.meta_get(m_edit_field, 0);
			if ((!ptr  && text.length()) || (ptr &&strcmp(ptr, text)))
			{
				info.meta_set(m_edit_field, text);
				tagger_api->update_info(m_edit_item, info, wnd_playlist, false);
			}
		}
		m_edit_saving=false;
	}
	m_edit_save = true;
}
#endif


void playlist_view::scroll(t_scroll_direction p_direction, t_scroll_type p_type, int p_value)
{
	int nBar = p_direction == scroll_vertically ? SB_VERT : SB_HORZ;

	static_api_ptr_t<playlist_manager> playlist_api;

	int & position = p_direction == scroll_vertically ? scroll_item_offset : horizontal_offset;

	int new_pos = position;
	int old_pos = position;

	SCROLLINFO si;
	memset(&si, 0, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
	GetScrollInfo(wnd_playlist, nBar, &si);
	if (p_type == scroll_sb)
	{

		if (p_value == SB_LINEDOWN && old_pos < si.nMax)
			new_pos = old_pos + 1;
		if (p_value == SB_LINEUP && old_pos > si.nMin)
			new_pos = old_pos - 1;
		if (p_value == SB_PAGEUP) 
			new_pos = old_pos - si.nPage;
		if (p_value == SB_PAGEDOWN) 
			new_pos = old_pos + si.nPage;
		if (p_value == SB_THUMBTRACK) 
			new_pos = si.nTrackPos;
		if (p_value == SB_BOTTOM) 
			new_pos = si.nMax;
		if (p_value == SB_TOP) 
			new_pos = si.nMin;
	}
	else
		new_pos = old_pos + p_value;

	if (new_pos < si.nMin)
		new_pos = si.nMin;
	if (new_pos > si.nMax)
		new_pos = si.nMax;

	if (new_pos != old_pos)
	{

		//evil mouse driver send WM_VSCROLL not WM_MOUSEWHEEL!!!!
		if (g_tooltip) {DestroyWindow(g_tooltip); g_tooltip=0;last_idx = -1; last_column = -1;}

		SCROLLINFO scroll2;
		memset(&scroll2, 0, sizeof(SCROLLINFO));
		scroll2.fMask = SIF_POS;
		scroll2.nPos = new_pos;
		scroll2.cbSize = sizeof(SCROLLINFO);

		position = SetScrollInfo(wnd_playlist, nBar, &scroll2, true);

		if (p_direction == scroll_horizontally)
		{
			move_header(true, false);
		}

		if (drawing_enabled)
		{
			RECT playlist;
			get_playlist_rect(&playlist);
			int dx = (p_direction == scroll_horizontally) ? (si.nPos - position) : 0;
			int dy = (p_direction == scroll_vertically) ? (si.nPos - position) * get_item_height() : 0;
			ScrollWindowEx(wnd_playlist, dx, dy, &playlist, &playlist, 0, 0, SW_INVALIDATE);
			RedrawWindow(wnd_playlist,0,0,RDW_UPDATENOW);
		}
	}
}



template < template<typename> class t_alloc >
unsigned playlist_view::get_column_widths(pfc::array_t<int, t_alloc> & out) const
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

void playlist_view::create_inline_edit_v2(const pfc::list_base_const_t<t_uint32> & indices, unsigned column)
{
	t_size indices_count = indices.get_count();
	if (!indices_count) return;
	t_size indices_spread = indices[indices_count-1] - indices[0] +1;
	if (!(column < g_get_cache().active_column_get_active_count()))
	{
		console::print("internal error - edit column index out of range");
		return;
	}
	static_api_ptr_t<playlist_manager> playlist_api;
	t_size active_count = playlist_api->activeplaylist_get_item_count();
	//if (!(index < active_count))
	//{
	//	console::print("internal error - edit item index out of range");
	//	return;
	//}

	if (m_edit_timer)
	{
		KillTimer(wnd_playlist, EDIT_TIMER_ID);
		m_edit_timer = false;
	}

	t_size median = indices[0] + indices_spread/2;//indices[(indices_count/2)];

	bool start_visible = is_visible(indices[0]);
	bool end_visible = is_visible(indices[indices_count-1]);

	if (!start_visible || !end_visible)
	{
		SCROLLINFO si;
		memset(&si, 0, sizeof(SCROLLINFO));
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
		GetScrollInfo(wnd_playlist, SB_VERT, &si);
		t_size target;
		if (indices_count > si.nPage)
		{
			target = median;
			scroll(scroll_vertically, scroll_position_delta, target - scroll_item_offset-((si.nPage>1?si.nPage-1:0)/2));
		}
		else
		{
			target = indices[0] > scroll_item_offset ? indices[indices_count-1] : indices[0];
		//else target = !start_visible ? indices[0] : indices[indices_count-1];
			scroll(scroll_vertically, scroll_position_delta, target - scroll_item_offset-(target > scroll_item_offset ? (si.nPage>1?si.nPage-1:0) : 0));
		}
	}

	int item_height = get_item_height();

	int x = 0;
	{
		pfc::array_t<int, pfc::alloc_standard> widths;
		get_column_widths(widths);
		unsigned n, count = widths.get_size();
		for (n=0; n<count && n<column; n++)
		{
			x += widths[n];
		}
	}

	RECT rc_playlist, rc_items;
	GetClientRect(wnd_playlist, &rc_playlist);
	get_playlist_rect(&rc_items);

	int font_height = uGetFontHeight(g_font);
	int header_height = get_header_height();

	int y = (indices[0]-scroll_item_offset)*item_height + header_height;
	if (y < header_height) y= header_height;
	int cx = get_column_width(column);
	int cy = min (item_height*indices_spread, rc_items.bottom-rc_items.top);

	if (x-horizontal_offset + cx > rc_playlist.right)
		scroll(scroll_horizontally, scroll_position_delta, x-horizontal_offset + (cx>rc_playlist.right?0:cx-rc_playlist.right));
	else if (x-horizontal_offset < 0)
		scroll(scroll_horizontally, scroll_position_delta, x-horizontal_offset);

	x-=horizontal_offset;

	if (m_wnd_edit)
		save_inline_edit_v2();

	m_edit_field.set_string(pfc::empty_string_t<char>());
	m_edit_items.remove_all();
	m_edit_items.set_count(indices_count);
	//pfc::list_t<bool> mask;

	pfc::array_t<file_info_impl> infos;
	pfc::ptr_list_t<const char> ptrs;
	infos.set_count(indices_count);
	//mask.set_count(indices_count);
	t_size i;

	pfc::string8 meta;
	meta = g_get_columns()[g_get_cache().active_column_active_to_actual(column)]->edit_field;
	m_edit_field = meta;

	bool matching = true;

	for (i=0; i<indices_count; i++)
	{
		if (playlist_api->activeplaylist_get_item_handle(m_edit_items[i],indices[i])) 
		{
			//mask[i] = true;
			m_edit_items[i]->get_info(infos[i]);
			ptrs.add_item(infos[i].meta_get(meta, 0));
		}
		else
		{
			m_edit_save = false;
			exit_inline_edit();
			return;
			//mask[i]=false;
			ptrs.add_item((const char *)NULL);
		}
			//exit_inline_edit();
		if (matching && i>0 && ( (ptrs[i] && ptrs[i-1] && strcmp(ptrs[i], ptrs[i-1])) || ((!ptrs[i] || !ptrs[i-1]) && (ptrs[i] != ptrs[i-1]) )))
			matching = false;
	}

	pfc::string8 text = matching ? (ptrs[0] ? ptrs[0] : "") : "<multiple values>";

	if (!m_wnd_edit)
	{
		m_edit_save = true;
		m_edit_changed = false;
		m_wnd_edit = CreateWindowEx(0, WC_EDIT, pfc::stringcvt::string_os_from_utf8(text).get_ptr(), WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE|ES_LEFT|
			ES_AUTOHSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|WS_BORDER|WS_CLIPCHILDREN, x, 
			y,
			cx, cy, wnd_playlist, HMENU(667),
			core_api::get_my_instance(), 0);

		SetWindowLongPtr(m_wnd_edit,GWL_USERDATA,(LPARAM)(this));
		m_inline_edit_proc = (WNDPROC)SetWindowLongPtr(m_wnd_edit,GWL_WNDPROC,(LPARAM)(g_inline_edit_hook_v2));
		SetWindowPos(m_wnd_edit,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		SendMessage(m_wnd_edit, WM_SETFONT, (WPARAM)g_font, MAKELONG(TRUE,0));
	}
	else
	{
		SendMessage(m_wnd_edit, WM_SETTEXT, 0, (LPARAM)_T(""));
		SetWindowPos(m_wnd_edit, NULL, x, 
			y,
			cx, cy, SWP_NOZORDER);
		//if (ptr)
			uSendMessageText(m_wnd_edit, WM_SETTEXT, 0, text);
	}


	RECT rc;
	rc.left = x+1;
	rc.top = y + (cy-font_height)/2;
	rc.right = x+cx;
	rc.bottom = rc.top + font_height;
	MapWindowPoints(wnd_playlist, m_wnd_edit, (LPPOINT)&rc, 2);


	SendMessage(m_wnd_edit, EM_SETRECT, NULL, (LPARAM)&rc);

	SendMessage(m_wnd_edit, EM_SETSEL, 0, -1);
	SetFocus(m_wnd_edit);

	m_edit_indices.remove_all();
	m_edit_indices.add_items(indices);
	m_edit_column = column;

}

void playlist_view::save_inline_edit_v2()
{
	static_api_ptr_t<metadb_io> tagger_api;

	if (m_edit_save && /*m_edit_changed &&*/ !m_edit_saving && !tagger_api->is_busy() && m_edit_items.get_count())
	{
		//pfc::vartoggle_t<bool>(m_edit_saving, true);
		m_edit_saving=true;
		
		pfc::string8 text;
		uGetWindowText(m_wnd_edit, text);

		if (strcmp(text, "<multiple values>"))
		{
			metadb_handle_list ptrs(m_edit_items);
			pfc::list_t<file_info_impl> infos;
			pfc::list_t<bool> mask;
			pfc::ptr_list_t<file_info> infos_ptr;
			t_size i, count = ptrs.get_count();
			mask.set_count(count);
			infos.set_count(count);
			//infos.set_count(count);
			for (i=0; i<count; i++)
			{
				assert(ptrs[i].is_valid());
				mask[i]= !ptrs[i]->get_info(infos[i]);
				infos_ptr.add_item(&infos[i]);
				if (!mask[i])
				{
					const char * ptr = infos[i].meta_get(m_edit_field, 0);
					if (!(mask[i] = !((!ptr  && text.length()) || (ptr && strcmp(ptr, text)))))
						infos[i].meta_set(m_edit_field, text);
				}
			}
			infos_ptr.remove_mask(mask.get_ptr());
			ptrs.remove_mask(mask.get_ptr());

			{
				{
					tagger_api->update_info_multi(ptrs, infos_ptr, wnd_playlist, false);
				}
			}
		}
		m_edit_saving=false;
	}
	m_edit_save = true;
}

LRESULT WINAPI playlist_view::g_inline_edit_hook_v2(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	playlist_view * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<playlist_view*>(GetWindowLongPtr(wnd,GWL_USERDATA));
	
	rv = p_this ? p_this->on_inline_edit_message_v2(wnd,msg,wp,lp) : DefWindowProc(wnd, msg, wp, lp);;
	
	return rv;
}

LRESULT playlist_view::on_inline_edit_message_v2(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_KILLFOCUS:
		save_inline_edit_v2();
		//if (wp == NULL)
		//	console::print("wp==NULL");
		PostMessage (wnd_playlist, MSG_KILL_INLINE_EDIT, 0, 0);
		break;
	case WM_SETFOCUS:
		break;
	case WM_GETDLGCODE:
		return CallWindowProc(m_inline_edit_proc,wnd,msg,wp,lp)|DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		switch (wp)
		{
		case VK_TAB:
			{
				unsigned count = g_get_cache().active_column_get_active_count();
				t_size indices_count = m_edit_indices.get_count();
				assert(indices_count);
				if (count && indices_count)
				{
					static_api_ptr_t<playlist_manager> api;
					bool back = (GetKeyState(VK_SHIFT) & KF_UP) != 0;
					if (back)
					{
						unsigned column = m_edit_column;
						unsigned playlist_index = m_edit_indices[0];
						t_size playlist_count = api->activeplaylist_get_item_count();
						pfc::string8_fast_aggressive temp;
						bool found = false;
						do
						{
							while (column > 0 && !(found = !g_get_columns()[g_get_cache().active_column_active_to_actual(--column)]->edit_field.is_empty()))
							{
							}
						}
						while(!found && indices_count == 1 && playlist_index > 0 && (column = count) && (playlist_index-- >= 0));

						if (found)
						{
							if (indices_count>1)
								create_inline_edit_v2(pfc::list_t<t_uint32>(m_edit_indices), column);
							else
							create_inline_edit_v2(playlist_index, column);
						}
					}
					else
					{
						unsigned column = m_edit_column+1;
						unsigned playlist_index = m_edit_indices[0];
						t_size playlist_count = api->activeplaylist_get_item_count();
						pfc::string8_fast_aggressive temp;
						bool found = false;
						do
						{
							while (column < count && !(found = !g_get_columns()[g_get_cache().active_column_active_to_actual(column)]->edit_field.is_empty()))
							{
								column++;
							}
						}
						while(!found && indices_count == 1 && ++playlist_index < playlist_count && !(column = 0));

						if (found)
						{
							//console::formatter () << "column: " << column;
							if (indices_count>1)
							create_inline_edit_v2(pfc::list_t<t_uint32>(m_edit_indices), column);
							else
							create_inline_edit_v2(playlist_index, column);
						}
					}
				}
			}
			return 0;
		case VK_ESCAPE:
			m_edit_save = false;
			exit_inline_edit();
			return 0;
		case VK_RETURN:
			if ((GetKeyState(VK_CONTROL) & KF_UP) == 0)
				exit_inline_edit();
				//SetFocus(wnd_playlist);
			return 0;
		}
		break;
	}
	return CallWindowProc(m_inline_edit_proc,wnd,msg,wp,lp);
}


void playlist_view::on_playlist_activate(unsigned p_old,unsigned p_new)
{
	//if (!cfg_nohscroll) g_save_columns();
	//	g_reset_columns();

	unsigned n, count = playlist_view::list_playlist.get_count();
	for (n=0; n<count; n++)
	{
		playlist_view * p_playlist = playlist_view::list_playlist.get_item(n);

		t_local_cache::t_local_cache_entry * p_cache = NULL; 
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

		uSendMessage(p_playlist->wnd_playlist, WM_SETREDRAW, TRUE, 0);		
		RedrawWindow(p_playlist->wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
	}
}

class playlist_callback_playlist : public playlist_callback_static
{
public:

	virtual void on_items_added(unsigned p_index,unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection)//inside any of these methods, you can call IPlaylist APIs to get exact info about what happened (but only methods that read playlist state, not those that modify it)
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
						if (p_playlist->drawing_enabled) RedrawWindow(p_playlist->wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
					}
				}
			}
		}
	}
	virtual void on_items_reordered(unsigned p_index,const unsigned * order,unsigned count)
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
	virtual void FB2KAPI on_items_removing(unsigned p_playlist,const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count){};//called before actually removing them
	virtual void FB2KAPI on_items_removed(unsigned p_index,const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count)
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
						if (p_playlist->drawing_enabled) RedrawWindow(p_playlist->wnd_playlist,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
					}
				}
			}
		}
	}

	virtual void on_items_selection_change(unsigned p_index,const bit_array & affected,const bit_array & state)
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
	virtual void on_item_focus_change(unsigned p_index,unsigned from,unsigned to)
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

	virtual void FB2KAPI on_items_modified_fromplayback(unsigned p_playlist,const bit_array & p_mask,play_control::t_display_level p_level)
	{
		on_items_modified(p_playlist, p_mask, true, p_level);
	}

	virtual void on_items_modified(unsigned p_index,const bit_array & p_mask)
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

	virtual void on_items_replaced(unsigned p_playlist,const bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data)
	{
		//		console::info(pfc::string_printf("on_items_replaced: %i",p_playlist));
		on_items_modified(p_playlist, p_mask);
	}

	virtual void on_item_ensure_visible(unsigned p_index,unsigned idx)
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

	virtual void on_playlist_activate(unsigned p_old,unsigned p_new)
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
	virtual void on_playlist_created(unsigned p_index,const char * p_name,unsigned p_name_len)
	{
		//		console::info(pfc::string_printf("on_playlist_created: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
			playlist_view::g_get_cache().on_playlist_created(p_index, p_name, p_name_len);
	};
	virtual void on_playlists_reorder(const unsigned * p_order,unsigned p_count)
	{
		if (playlist_view::g_get_cache().is_active())
		{
			playlist_view::g_get_cache().on_playlists_reorder(p_order, p_count);
		}
	};
	virtual void on_playlists_removing(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count)
	{
		//		console::info(pfc::string_printf("on_playlist_removing"));
	};
	virtual void on_playlists_removed(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count)
	{
		//		console::info(pfc::string_printf("on_playlist_removed:"));
		if (playlist_view::g_get_cache().is_active())
		{
			playlist_view::g_get_cache().on_playlists_removed(p_mask, p_old_count, p_new_count);
		}
	};
	virtual void on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len)
	{
		//		console::info(pfc::string_printf("on_playlist_renamed: %i",p_index));
		if (playlist_view::g_get_cache().is_active())
		{
			playlist_view::g_get_cache().on_playlist_renamed(p_index, p_new_name, p_new_name_len);
			if (p_index == playlist_view::g_get_cache().get_active_playlist())
				playlist_view::on_playlist_activate(p_index, p_index);
		}
	};

	virtual void on_default_format_changed()
	{
		//		console::info(pfc::string_printf("on_default_format_changed:"));
	};
	virtual void on_playback_order_changed(unsigned p_new_index)
	{
		//		console::info(pfc::string_printf("on_playback_order_changed: %i",p_new_index));
	};
	virtual void on_playlist_locked(unsigned p_playlist,bool p_locked)
	{
		//		console::info(pfc::string_printf("on_playlist_locked: %i",p_playlist));
	};

	unsigned get_flags() { return playlist_callback::flag_all;}

};

static service_factory_single_t<playlist_callback_playlist> asdf3;


ui_extension::window_factory<playlist_view> blah;

void playlist_view::g_get_global_style_titleformat_object(service_ptr_t<titleformat_object> & p_out)
{
	if (!g_to_global_colour.is_valid()) 
		static_api_ptr_t<titleformat_compiler>()->compile_safe(g_to_global_colour, cfg_colour);
	p_out = g_to_global_colour;
}

// {0CF29D60-1262-4f55-A6E1-BC4AE6579D19}
const GUID appearance_client_pv_impl::g_guid = 
{ 0xcf29d60, 0x1262, 0x4f55, { 0xa6, 0xe1, 0xbc, 0x4a, 0xe6, 0x57, 0x9d, 0x19 } };

appearance_client_pv_impl::factory<appearance_client_pv_impl> g_appearance_client_pv_impl;