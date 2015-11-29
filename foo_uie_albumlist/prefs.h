#pragma once

class preferences_tab
{
public:
	virtual HWND create(HWND wnd) = 0;
	virtual const char * get_name() = 0;
};


class tab_general : public preferences_tab
{
	bool m_initialised;
	HWND m_wnd;
public:
	bool is_active() { return m_wnd != 0; }
	void refresh_views();
	static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
public:
	virtual HWND create(HWND wnd) { return uCreateDialog(IDD_CONFIG, wnd, g_on_message, (LPARAM)this); }
	virtual const char * get_name() { return "General"; }

	tab_general() : m_initialised(false), m_wnd(NULL) {};

};

class tab_advanced : public preferences_tab
{
	static bool initialised;

	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
public:
	virtual HWND create(HWND wnd) { return uCreateDialog(IDD_ADVANCED, wnd, ConfigProc); }
	virtual const char * get_name() { return "Advanced"; }

};

class config_albumlist : public preferences_page
{

	static HWND child;

	static void make_child(HWND wnd);

	static BOOL CALLBACK ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
public:
	HWND create(HWND parent)
	{
		return uCreateDialog(IDD_HOST, parent, ConfigProc);
	}
	const char * get_name() { return "Album List Panel"; }
	virtual GUID get_guid()
	{
		return g_guid_preferences_album_list_panel;
	}
	virtual GUID get_parent_guid() { return preferences_page::guid_media_library; }
	virtual bool reset_query()
	{
		return false;
	}
	virtual void reset()
	{
	};
};

extern tab_general g_config_general;

