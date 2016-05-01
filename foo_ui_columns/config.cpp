#include "stdafx.h"

class config_host_generic : public preferences_page {
public:
	config_host_generic(const char* p_name, preferences_tab*const * const p_tabs, size_t p_tab_count, const GUID& p_guid, const GUID& p_parent_guid, cfg_int* const p_active_tab)
		: m_child(nullptr), m_name(p_name), m_guid(p_guid), m_parent_guid(p_parent_guid), m_tabs(p_tabs), m_tab_count(p_tab_count), m_active_tab(*p_active_tab) {}

	HWND create(HWND parent) override
	{
		return uCreateDialog(IDD_HOST, parent, g_on_message, reinterpret_cast<LPARAM>(this));
	}

	const char* get_name() override
	{
		return m_name;
	}

	GUID get_guid() override
	{
		return m_guid;
	}

	GUID get_parent_guid() override
	{
		return m_parent_guid;
	}

	bool reset_query() override
	{
		return false;
	}

	void reset() override { };

	bool get_help_url(pfc::string_base& p_out) override
	{
		if (!(m_active_tab < (int)m_tab_count && m_tabs[m_active_tab]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}

private:
	void destroy_child()
	{
		if (m_child) {
			ShowWindow(m_child, SW_HIDE);
			DestroyWindow(m_child);
			m_child = nullptr;
		}
	}

	void make_child(HWND wnd)
	{
		destroy_child();

		HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

		RECT tab;

		GetWindowRect(wnd_tab, &tab);
		MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);

		TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

		if (m_active_tab >= (int)m_tab_count)
			m_active_tab = 0;

		if (m_active_tab < (int)m_tab_count && m_active_tab >= 0) {
			m_child = m_tabs[m_active_tab]->create(wnd);
		}

		//SetWindowPos(wnd_tab,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		if (m_child) {
			EnableThemeDialogTexture(m_child, ETDT_ENABLETAB);
		}

		SetWindowPos(m_child, 0, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
		SetWindowPos(wnd_tab, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		ShowWindow(m_child, SW_SHOWNORMAL);
		//UpdateWindow(child);
	}

	static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		config_host_generic* p_instance;
		if (msg == WM_INITDIALOG) {
			p_instance = reinterpret_cast<config_host_generic*>(lp);
			SetWindowLongPtr(wnd, DWLP_USER, lp);
		} else
			p_instance = reinterpret_cast<config_host_generic*>(GetWindowLongPtr(wnd, DWLP_USER));
		return p_instance ? p_instance->on_message(wnd, msg, wp, lp) : FALSE;
	}

	BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg) {
			case WM_INITDIALOG:
				{
					HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
					//SendMessage(wnd_tab, TCM_SETMINTABWIDTH, 0, 35);
					unsigned n, count = m_tab_count;
					for (n = 0; n < count; n++) {
						uTabCtrl_InsertItemText(wnd_tab, n, m_tabs[n]->get_name());
					}
					TabCtrl_SetCurSel(wnd_tab, m_active_tab);
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
					} else if (lpwp->flags & SWP_SHOWWINDOW && !m_child) {
						make_child(wnd);
					}
				}
				break;
			case WM_NOTIFY:
				switch (((LPNMHDR)lp)->idFrom) {
					case IDC_TAB1:
						switch (((LPNMHDR)lp)->code) {
							case TCN_SELCHANGE:
								{
									m_active_tab = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
									make_child(wnd);
								}
								break;
						}
						break;
				}
				break;


			case WM_PARENTNOTIFY:
				switch (wp) {
					case WM_DESTROY:
						{
							if (m_child && (HWND)lp == m_child)
								m_child = 0;
						}
						break;
				}
				break;
		}
		return 0;
	}

	HWND m_child;
	const char* m_name;
	const GUID &m_guid, &m_parent_guid;
	preferences_tab*const * const m_tabs;
	const size_t m_tab_count;
	cfg_int& m_active_tab;
};


cfg_struct_t<LOGFONT> cfg_editor_font(create_guid(0xd429d322, 0xd236, 0x7356, 0x33, 0x25, 0x4b, 0x67, 0xc5, 0xd4, 0x50, 0x3e), get_menu_font());
cfg_int cfg_import_titles(create_guid(0xcd062463, 0x488f, 0xc7ec, 0x56, 0xf2, 0x90, 0x7f, 0x0a, 0xfe, 0x77, 0xda), 1);
cfg_int cfg_export_titles(create_guid(0x96094997, 0xbf50, 0x202d, 0x98, 0x01, 0xfc, 0x02, 0xf3, 0x94, 0x30, 0x63), 0);
cfg_int cfg_child(create_guid(0xf20b83d0, 0x5890, 0xba6f, 0xe8, 0x62, 0x69, 0x30, 0xe2, 0x6b, 0xc8, 0x1c), 0);
cfg_int cfg_child_panels(create_guid(0x1a8d8760, 0x4f60, 0x4800, 0x93, 0x81, 0x32, 0x32, 0x66, 0xa0, 0x6c, 0xff), 0);
cfg_int cfg_child_playlist(create_guid(0xbc6c99d4, 0x51c1, 0xf76e, 0x10, 0x9c, 0x62, 0x92, 0x92, 0xbd, 0xbd, 0xb2), 0);

// {E57A430E-51BB-4FCC-B0BC-9D228B891A17}
static const GUID guid_filters_page_child =
{0xe57a430e, 0x51bb, 0x4fcc,{0xb0, 0xbc, 0x9d, 0x22, 0x8b, 0x89, 0x1a, 0x17}};

cfg_int cfg_child_filters(guid_filters_page_child, 0);

pvt::preferences_tab_impl g_tab_grouping;

static preferences_tab* g_tabs[] =
{
	g_get_tab_main(),
	//&g_tab_layout,
	g_get_tab_layout(),
	//&g_tab_display,
	g_get_tab_status(),
	g_get_tab_sys(),
	g_get_tab_artwork(),
};

static preferences_tab* g_tabs_filters[] =
{
	g_tab_filter_misc,
	g_tab_filter_fields
};

static preferences_tab* g_tabs_panels[] =
{
	g_get_tab_playlist(),
	//&g_tab_playlist_colours,
	g_get_tab_playlist_dd(),
};

static preferences_tab* g_tabs_playlist_view[] =
{
	g_get_tab_display2(),
	&g_tab_grouping,
	g_get_tab_columns_v3(),
	g_get_tab_global(),
};

// {DF6B9443-DCC5-4647-8F8C-D685BF25BD09}
const GUID guid_config_host =
{0xdf6b9443, 0xdcc5, 0x4647, {0x8f, 0x8c, 0xd6, 0x85, 0xbf, 0x25, 0xbd, 0x9}};

void g_show_artwork_settings()
{
	cfg_child = 5;
	static_api_ptr_t<ui_control>()->show_preferences(guid_config_host);
}

// {779F2FA6-3B76-4829-9E02-2E579CA510BF}
const GUID guid_playlist_switcher_page =
{0x779f2fa6, 0x3b76, 0x4829, {0x9e, 0x2, 0x2e, 0x57, 0x9c, 0xa5, 0x10, 0xbf}};

// {B8CA5FC9-7463-48e8-9879-0D9517F3E7A9}
const GUID guid_playlist_view_page =
{0xb8ca5fc9, 0x7463, 0x48e8, {0x98, 0x79, 0xd, 0x95, 0x17, 0xf3, 0xe7, 0xa9}};

// {71A480E2-9007-4315-8DF3-81636C740AAD}
static const GUID guid_filters_page =
{0x71a480e2, 0x9007, 0x4315,{0x8d, 0xf3, 0x81, 0x63, 0x6c, 0x74, 0xa, 0xad}};

namespace columns {
	const GUID& config_get_playlist_view_guid()
	{
		return guid_playlist_view_page;
	}

	const GUID& config_get_main_guid()
	{
		return guid_config_host;
	}
};


static service_factory_single_t<config_host_generic> main_page("Columns UI", g_tabs, tabsize(g_tabs), guid_config_host, preferences_page::guid_display, &cfg_child);
static service_factory_single_t<config_host_generic> playlist_view_page("Playlist view", g_tabs_playlist_view, tabsize(g_tabs_playlist_view), guid_playlist_view_page, guid_config_host, &cfg_child_playlist);
static service_factory_single_t<config_host_generic> playlist_switcher_page("Playlist switcher", g_tabs_panels, tabsize(g_tabs_panels), guid_playlist_switcher_page, guid_config_host, &cfg_child_panels);
static service_factory_single_t<config_host_generic> filters_page("Filters", g_tabs_filters, tabsize(g_tabs_filters), guid_filters_page, guid_config_host, &cfg_child_filters);


void g_set_tab(const char* name)
{
	unsigned n, count = tabsize(g_tabs_playlist_view);

	for (n = 0; n < count; n++) {
		if (!strcmp(g_tabs_playlist_view[n]->get_name(), name)) {
			cfg_child_playlist = n;
			break;
		}
	}
}
