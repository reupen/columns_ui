#ifndef _COLUMNS_SEEKBAR_H_
#define _COLUMNS_SEEKBAR_H_

#include <windows.h>
#include <COMMCTRL.H>
#include <WinUser.h>
#include <windowsx.h>
#include <commctrl.h>

class seek_bar_extension : public ui_extension::container_ui_extension
{
	bool initialised;
	track_bar_impl m_child;

	class track_bar_host_impl : public track_bar_host
	{
		void on_position_change(unsigned pos, bool b_tracking);
		void get_tooltip_text(unsigned pos, track_bar_string & out);
	} m_track_bar_host;

public:
	static pfc::ptr_list_t<seek_bar_extension> windows;

	HWND wnd_seekbar;

	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	void disable_seek();
	void update_seek();
	void update_seek_pos();
	seek_bar_extension();
	~seek_bar_extension();

	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data(_T("{89A3759F-348A-4e3f-BF43-3D16BC059186}"), true);
	}

	virtual const GUID & get_extension_guid() const ;

	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;

	virtual unsigned get_type  () const;

	static void update_seek_timer();
	static unsigned g_seek_timer;
	static VOID CALLBACK SeekTimerProc(HWND wnd, UINT msg, UINT event, DWORD time);
	static void update_seekbars(bool positions_only = false);
};



#endif