#ifndef _COLUMNS_SPLITTER_H_
#define _COLUMNS_SPLITTER_H_

class WindowEnum_t
{
	static BOOL CALLBACK g_EnumWindowsProc(HWND wnd, LPARAM lp)
	{
		return ((WindowEnum_t*)lp)->EnumWindowsProc(wnd);
	}
	BOOL EnumWindowsProc(HWND wnd)
	{
		if (GetWindow(wnd, GW_OWNER) == m_wnd_owner && IsWindowVisible(wnd))
			m_wnd_list.add_item(wnd);
		return TRUE;
	}
	HWND m_wnd_owner;
public:
	void run()
	{
		EnumWindows(&g_EnumWindowsProc, (LPARAM)this);
	}
	pfc::list_t<HWND, pfc::alloc_fast> m_wnd_list;
	WindowEnum_t(HWND wnd_owner) : m_wnd_owner(wnd_owner) {};
};

void g_get_panel_list(uie::window_info_list_simple & p_out, uie::window_host_ptr & p_host);
void g_append_menu_panels(HMENU menu, const uie::window_info_list_simple & panels, UINT base);
void g_append_menu_splitters(HMENU menu, const uie::window_info_list_simple & panels, UINT base);
void g_run_live_edit_contextmenu(HWND wnd, POINT pt_menu, window_transparent_fill & p_overlay, const RECT & rc_overlay, uie::window_ptr ptr, uie::splitter_window_ptr p_container, t_size index, uie::window_host_ptr & p_host);

#endif
