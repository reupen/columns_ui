#include "stdafx.h"

cfg_struct_t<LOGFONT> cfg_editor_font(create_guid(0xd429d322, 0xd236, 0x7356, 0x33, 0x25, 0x4b, 0x67, 0xc5, 0xd4, 0x50, 0x3e), get_menu_font());
cfg_int cfg_import_titles(create_guid(0xcd062463, 0x488f, 0xc7ec, 0x56, 0xf2, 0x90, 0x7f, 0x0a, 0xfe, 0x77, 0xda), 1);
cfg_int cfg_export_titles(create_guid(0x96094997, 0xbf50, 0x202d, 0x98, 0x01, 0xfc, 0x02, 0xf3, 0x94, 0x30, 0x63), 0);
cfg_int cfg_child(create_guid(0xf20b83d0, 0x5890, 0xba6f, 0xe8, 0x62, 0x69, 0x30, 0xe2, 0x6b, 0xc8, 0x1c), 0);
cfg_int cfg_child_panels(create_guid(0x1a8d8760, 0x4f60, 0x4800, 0x93, 0x81, 0x32, 0x32, 0x66, 0xa0, 0x6c, 0xff), 0);
cfg_int cfg_child_playlist(create_guid(0xbc6c99d4, 0x51c1, 0xf76e, 0x10, 0x9c, 0x62, 0x92, 0x92, 0xbd, 0xbd, 0xb2), 0);




void update_vis_host_frames();

pvt::preferences_tab_impl g_tab_grouping;

static preferences_tab * g_tabs[] =
{
	g_get_tab_main(),
	//&g_tab_layout,
	g_get_tab_layout(),
	//&g_tab_display,
	g_get_tab_status(),
	g_get_tab_sys(),
	g_get_tab_filter(),
	g_get_tab_artwork(),
};

static preferences_tab * g_tabs_panels[] =
{
	g_get_tab_playlist(),
	//&g_tab_playlist_colours,
	g_get_tab_playlist_dd(),
};

static preferences_tab * g_tabs_playlist_view[] =
{
	g_get_tab_display2(),
	&g_tab_grouping,
	g_get_tab_columns_v3(),
	g_get_tab_global(),
};

class config_host : public preferences_page
{
public:
	static HWND child;
private:
	static void destroy_child()
	{
		if (child) {
			ShowWindow(child, SW_HIDE);
			DestroyWindow(child);
			child = nullptr;
		}
	}

	static void make_child(HWND wnd)
	{
		destroy_child();

		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

		RECT tab;

		GetWindowRect(wnd_tab, &tab);
		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);

		TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

		unsigned count = tabsize(g_tabs);
		if (cfg_child >= count) cfg_child = 0;

		if (cfg_child < count && cfg_child >= 0)
		{
			child = g_tabs[cfg_child]->create(wnd);
		}

		//SetWindowPos(wnd_tab,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		if (child)
		{
			EnableThemeDialogTexture(child, ETDT_ENABLETAB);
		}

		SetWindowPos(child, 0, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
		SetWindowPos(wnd_tab, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		ShowWindow(child, SW_SHOWNORMAL);
		//UpdateWindow(child);
	}

	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{
			HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
			//SendMessage(wnd_tab, TCM_SETMINTABWIDTH, 0, 35);
			unsigned n, count = tabsize(g_tabs);
			for (n = 0; n < count; n++)
			{
				uTabCtrl_InsertItemText(wnd_tab, n, g_tabs[n]->get_name());
			}
			TabCtrl_SetCurSel(wnd_tab, cfg_child);
			make_child(wnd);
		}
		break;
		case WM_DESTROY:
			break;
		case WM_WINDOWPOSCHANGED:
			{
				auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);
				// Temporary workaround for various bugs that occur due to foobar2000 1.0+ 
				// having a dislike for destroying preference pages
				if (lpwp->flags & SWP_HIDEWINDOW) {
					destroy_child();
				}
				else if (lpwp->flags & SWP_SHOWWINDOW && !child) {
					make_child(wnd);
				}
			}
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom)
			{
			case IDC_TAB1:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
				{
					cfg_child = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
					make_child(wnd);
				}
				break;
				}
				break;
			}
			break;


		case WM_PARENTNOTIFY:
			switch (wp)
			{
			case WM_DESTROY:
			{
				if (child && (HWND)lp == child) child = 0;
			}
			break;
			}
			break;
		}
		return 0;
	}

public:
	HWND create(HWND parent)
	{
		return uCreateDialog(IDD_HOST, parent, ConfigProc);
	}
	const char * get_name() { return "Columns UI"; }
	const char * get_parent_name() { return "Display"; }

	static const GUID guid;

	virtual GUID get_guid()
	{
		return guid;
	}
	virtual GUID get_parent_guid() { return preferences_page::guid_display; }
	virtual bool reset_query()
	{
		return false;
	}
	virtual void reset()
	{
	};
	virtual bool get_help_url(pfc::string_base & p_out)
	{
		if (!(cfg_child < tabsize(g_tabs) && g_tabs[cfg_child]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}

};

// {DF6B9443-DCC5-4647-8F8C-D685BF25BD09}
const GUID config_host::guid =
{ 0xdf6b9443, 0xdcc5, 0x4647, { 0x8f, 0x8c, 0xd6, 0x85, 0xbf, 0x25, 0xbd, 0x9 } };

void g_show_artwork_settings()
{
	cfg_child = 5;
	static_api_ptr_t<ui_control>()->show_preferences(config_host::guid);
}
class config_panels : public preferences_page
{
public:
	static HWND child;
private:


	virtual bool get_help_url(pfc::string_base & p_out)
	{
		if (!(cfg_child_panels < tabsize(g_tabs_panels) && g_tabs_panels[cfg_child_panels]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}

	static void destroy_child()
	{
		if (child) {
			ShowWindow(child, SW_HIDE);
			DestroyWindow(child);
			child = nullptr;
		}
	}

	static void make_child(HWND wnd)
	{
		destroy_child();

		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

		RECT tab;

		GetWindowRect(wnd_tab, &tab);
		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);

		TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

		unsigned count = tabsize(g_tabs_panels);
		if (cfg_child_panels >= count) cfg_child_panels = 0;

		if (cfg_child_panels < count && cfg_child_panels >= 0)
		{
			child = g_tabs_panels[cfg_child_panels]->create(wnd);
		}

		//SetWindowPos(child,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		if (child)
		{
			{
				EnableThemeDialogTexture(child, ETDT_ENABLETAB);
			}
		}

		SetWindowPos(child, 0, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
		SetWindowPos(wnd_tab, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		ShowWindow(child, SW_SHOWNORMAL);
		//UpdateWindow(child);

		//if (wnd_destroy)
		//	DestroyWindow(wnd_destroy);
	}

	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{

			HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
			unsigned n, count = tabsize(g_tabs_panels);


			for (n = 0; n < count; n++)
			{
				uTabCtrl_InsertItemText(wnd_tab, n, g_tabs_panels[n]->get_name());
			}

			TabCtrl_SetCurSel(wnd_tab, cfg_child_panels);


			make_child(wnd);

		}

		break;
		case WM_DESTROY:
			break;
		case WM_WINDOWPOSCHANGED:
			{
				auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);
				// Temporary workaround for various bugs that occur due to foobar2000 1.0+ 
				// having a dislike for destroying preference pages
				if (lpwp->flags & SWP_HIDEWINDOW) {
					destroy_child();
				}
				else if (lpwp->flags & SWP_SHOWWINDOW && !child) {
					make_child(wnd);
				}
			}
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom)
			{
			case IDC_TAB1:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
				{
					cfg_child_panels = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
					make_child(wnd);
				}
				break;
				}
				break;
			}
			break;


		case WM_PARENTNOTIFY:
			switch (wp)
			{
			case WM_DESTROY:
			{
				if (child && (HWND)lp == child) child = 0;
			}
			break;
			}
			break;
		}
		return 0;
	}




public:
	HWND create(HWND parent)
	{
		return uCreateDialog(IDD_HOST, parent, ConfigProc);
	}
	const char * get_name() { return "Playlist switcher"; }

	static const GUID guid;

	virtual GUID get_guid()
	{
		return guid;
	}
	virtual GUID get_parent_guid() { return config_host::guid; }
	virtual bool reset_query()	{ return false; }
	virtual void reset() {};

};

// {779F2FA6-3B76-4829-9E02-2E579CA510BF}
const GUID config_panels::guid =
{ 0x779f2fa6, 0x3b76, 0x4829, { 0x9e, 0x2, 0x2e, 0x57, 0x9c, 0xa5, 0x10, 0xbf } };

class config_playlist : public preferences_page
{
public:
	static HWND child;
private:

	virtual bool get_help_url(pfc::string_base & p_out)
	{
		if (!(cfg_child_playlist < tabsize(g_tabs_playlist_view) && g_tabs_playlist_view[cfg_child_playlist]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}
	static void destroy_child()
	{
		if (child) {
			ShowWindow(child, SW_HIDE);
			DestroyWindow(child);
			child = nullptr;
		}
	}

	static void make_child(HWND wnd)
	{
		destroy_child();

		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

		RECT tab;

		GetWindowRect(wnd_tab, &tab);
		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);

		TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

		unsigned count = tabsize(g_tabs_playlist_view);
		if (cfg_child_playlist >= count) cfg_child_playlist = 0;

		if (cfg_child_playlist < count && cfg_child_playlist >= 0)
		{
			child = g_tabs_playlist_view[cfg_child_playlist]->create(wnd);
		}

		//SetWindowPos(child,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		if (child)
		{
			{
				EnableThemeDialogTexture(child, ETDT_ENABLETAB);
			}
		}

		SetWindowPos(child, 0, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
		SetWindowPos(wnd_tab, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		ShowWindow(child, SW_SHOWNORMAL);
		//UpdateWindow(child);
	}

	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{

			HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
			unsigned n, count = tabsize(g_tabs_playlist_view);


			for (n = 0; n < count; n++)
			{
				uTabCtrl_InsertItemText(wnd_tab, n, g_tabs_playlist_view[n]->get_name());
			}

			TabCtrl_SetCurSel(wnd_tab, cfg_child_playlist);


			make_child(wnd);

		}

		break;
		case WM_DESTROY:
			break;
		case WM_WINDOWPOSCHANGED:
			{
				// Temporary workaround for various bugs that occur due to foobar2000 1.0+ 
				// having a dislike for destroying preference pages
				auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);
				if (lpwp->flags & SWP_HIDEWINDOW) {
					destroy_child();
				}
				else if (lpwp->flags & SWP_SHOWWINDOW && !child) {
					make_child(wnd);
				}
			}
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom)
			{
			case IDC_TAB1:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
				{
					cfg_child_playlist = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
					make_child(wnd);
				}
				break;
				}
				break;
			}
			break;


		case WM_PARENTNOTIFY:
			switch (wp)
			{
			case WM_DESTROY:
			{
				if (child && (HWND)lp == child) child = 0;
			}
			break;
			}
			break;
		}
		return 0;
	}




public:
	HWND create(HWND parent)
	{
		return uCreateDialog(IDD_HOST, parent, ConfigProc);
	}
	const char * get_name() { return "Playlist view"; }
	static const GUID guid;

	virtual GUID get_guid()
	{
		return guid;
	}
	virtual GUID get_parent_guid() { return config_host::guid; }
	virtual bool reset_query()	{ return false; }
	virtual void reset() {};

};


// {B8CA5FC9-7463-48e8-9879-0D9517F3E7A9}
const GUID config_playlist::guid =
{ 0xb8ca5fc9, 0x7463, 0x48e8, { 0x98, 0x79, 0xd, 0x95, 0x17, 0xf3, 0xe7, 0xa9 } };

namespace columns
{
	const GUID & config_get_playlist_view_guid()
	{
		return config_playlist::guid;
	}
	const GUID & config_get_main_guid()
	{
		return config_host::guid;
	}
};


/*bool config_ui::initialising = false;
bool config_ui2::initialised = false;
bool config_ui3::initialised = false;*/
HWND config_host::child = 0;
HWND config_panels::child = 0;
HWND config_playlist::child = 0;
//config_tab_item * config_host::config_tab = 0;
//bool tab_display::initialised = 0;

//ptr_list_autofree_t<char> config_host::tab_sys:: status_items;
//bool tab_columns::initialising = 0;
//WNDPROC tab_columns::editproc = 0;

//WNDPROC config_extra::editproc;

/*static service_factory_single_t<config,config_ui3> blah;
static service_factory_single_t<config,config_ui2> foo39;
static service_factory_single_t<config,config_ui> foo3;
static service_factory_single_t<config,config_extra> foog3;
*/

static preferences_page_factory_t<config_host> foog43;

static preferences_page_factory_t<config_panels> foog4;
static preferences_page_factory_t<config_playlist> foog3;


void g_set_tab(const char * name)
{
	unsigned n, count = tabsize(g_tabs_playlist_view);

	for (n = 0; n < count; n++)
	{
		if (!strcmp(g_tabs_playlist_view[n]->get_name(), name))
		{
			cfg_child_playlist = n;
			break;
		}
	}
}