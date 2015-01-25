#ifndef _COLUMNS_SEEKBAR2_H_
#define _COLUMNS_SEEKBAR2_H_

#if 0

#include "../SDK/foobar2000.h"

#include "../ui_extension/ui_extension.h"

#include <windows.h>
#include <COMMCTRL.H>
#include <WinUser.h>
#include <windowsx.h>
#include <commctrl.h>

class seekbar_extension : public ui_extension::container_ui_extension
{
	static const char * class_name;

	UINT last_seek_msg;
	WPARAM last_seek_wp;
	LPARAM last_seek_lp;

	HWND wnd_prev_focus;

	WNDPROC seekproc;
	bool initialised;

public:
	static ptr_list_t<seekbar_extension> windows;
	bool is_seeking;

	HWND wnd_seekbar, wnd_tooltip;

	LRESULT WINAPI hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static LRESULT WINAPI main_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	LRESULT WINAPI on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	LRESULT create_tooltip(const char * text, POINT pt);
	LRESULT update_tooltip(const char * text);
	void destroy_tooltip();

	void disable_seek();
	void update_seek();
	void update_seek_pos();
	seekbar_extension();
	~seekbar_extension();
	bool add_ref();
	bool de_ref();

	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data("{89A3759F-348A-4e3f-BF43-3D16BC059186}", true);
	}

	static const GUID extension_guid;

	virtual const GUID & get_extension_guid() const ;

	virtual void get_name(string_base & out)const;
	virtual void get_category(string_base & out)const;

	virtual unsigned get_type  () const;

	static void update_seek_timer();
	static unsigned g_seek_timer;
	static VOID CALLBACK SeekTimerProc(HWND wnd, UINT msg, UINT event, DWORD time);
	static void update_seekbars(bool positions_only = false);
};


#endif

#endif