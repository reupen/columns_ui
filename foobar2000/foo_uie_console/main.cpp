/**
* \file main.cpp
*
* \brief Console panel component
*
* This component is an example of a multiple instance panel that takes keyboard input
*
* It demonstrates the following relevant techniques:
* - Subclassing the child control to process keyboard shortcuts
* - Setting the font of the child window
* - Keeping a list of active windows and updating them from a callback (in this case designed
*   such that the callback may come from any thread)
* - That's about it ?
*/

#include "../SDK/foobar2000.h"
#include "../helpers/helpers.h"
#include <commctrl.h>
#include <windowsx.h>

#include "../columns_ui-sdk/ui_extension.h"

/** Declare some component information */
DECLARE_COMPONENT_VERSION("Console panel",
"0.4",
"compiled: " __DATE__ "\n"
"with Panel API version: " UI_EXTENSION_VERSION

);

#define IDC_EDIT 1001

enum {MSG_UPDATE = WM_USER + 2};

/** \brief The maximum number of message we cache/display */
const t_size maximum_messages = 200;

struct create_guid : public GUID
{
	create_guid(t_uint32 p_data1, t_uint16 p_data2, t_uint16 p_data3, t_uint8 p_data41, t_uint8 p_data42, t_uint8 p_data43, t_uint8 p_data44, t_uint8 p_data45, t_uint8 p_data46, t_uint8 p_data47, t_uint8 p_data48) 
	{
		Data1 = p_data1;
		Data2 = p_data2;
		Data3 = p_data3;
		Data4[0] = p_data41;
		Data4[1] = p_data42;
		Data4[2] = p_data43;
		Data4[3] = p_data44;
		Data4[4] = p_data45;
		Data4[5] = p_data46;
		Data4[6] = p_data47;
		Data4[7] = p_data48;
	}
};

cfg_int cfg_frame(create_guid(0x05550547,0xbf98,0x088c,0xbe,0x0e,0x24,0x95,0xe4,0x9b,0x88,0xc7),2);

// {26059FEB-488B-4ce1-824E-4DF113B4558E}
static const GUID g_guid_console_font = 
{ 0x26059feb, 0x488b, 0x4ce1, { 0x82, 0x4e, 0x4d, 0xf1, 0x13, 0xb4, 0x55, 0x8e } };

class console_window : public ui_extension::container_ui_extension
{
	WNDPROC m_editproc;
protected:
	HWND wnd_edit;
public:
	static pfc::ptr_list_t<console_window> list_wnd;
	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	LRESULT on_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static LRESULT WINAPI hook_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	class t_message: public pfc::refcounted_object_root
	{
	public:
		typedef pfc::refcounted_object_ptr_t<t_message> ptr;

		SYSTEMTIME m_time;
		pfc::string_simple m_message;
		t_message(const t_message & p_source)
			: m_time(p_source.m_time), m_message(p_source.m_message)
		{};
		t_message(const char * p_message, t_size p_message_length)
			: m_message(p_message, p_message_length)
		{
			GetLocalTime(&m_time);
		}
		t_message()
		{
			memset (&m_time, 0, sizeof(SYSTEMTIME));
		};
	};

	console_window();

	static void g_update_all_fonts();

	~console_window();

	static const GUID extension_guid;

	virtual const GUID & get_extension_guid() const
	{
		return extension_guid;
	}

	//static const TCHAR * g_get_popup_class_name() {return _T("[popup] {89A3759F-348A-4e3f-BF43-3D16BC059186}");}

	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;

	unsigned get_type () const
	{
		/** In this case we are only of type type_panel */
		return ui_extension::type_panel;
	}

	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data(_T("{89A3759F-348A-4e3f-BF43-3D16BC059186}"), false);
	}

	static critical_section sync;

	static void update_all_window_frames();
	static void g_on_message_received(const char * ptr, t_size len) //from any thread
	{
		insync(sync);
		pfc::string8 buffer;
		/**  Sort out line break messes */
		{
			const char * start = ptr;
			const char * pos = ptr;
			while (t_size(pos-ptr)<len && *pos)
			{
				while (t_size(pos-ptr+1)<len && pos[1] && pos[0] != '\n')
					pos++;
				{
					if (pos[0] == '\n')
						buffer.add_string(start, pos-start-( (pos>ptr && (pos[-1]) == '\r') ?1:0));
					else
						buffer.add_string(start, pos+1-start);
					buffer.add_byte('\r');
					buffer.add_byte('\n');
					//if ((pos-ptr)<len && *pos)
					{
						start=pos+1;
						pos++;
					}
				}
			}
		}
		if (g_messages.add_item(new t_message(buffer.get_ptr(), buffer.get_length())) == maximum_messages)
			/** Inefficient, yes. Fix one day */
			g_messages.remove_by_idx(0);

		/** Post a notification to all instances of the panel to update their display */
		unsigned n , count = g_notify_list.get_count();
		for (n=0;n<count;n++)
		{
			HWND wnd = g_notify_list[n];
			if (wnd)
				PostMessage(wnd, MSG_UPDATE, 0, 0);
		}
	};
	static void g_clear()
	{
		insync(sync);

		/** Clear all messages */
		g_messages.remove_all();

		/** Post a notification to all instances of the panel to update their display */
		unsigned n , count = g_notify_list.get_count();
		for (n=0;n<count;n++)
		{
			HWND wnd = g_notify_list[n];
			if (wnd)
				PostMessage(wnd, MSG_UPDATE, 0, 0);
		}
	};

	void update_content();
	void update_content_throttled();

	virtual bool have_config_popup(){return false;}
	virtual bool show_config_popup(HWND wnd_parent){return false;};
private:
	LARGE_INTEGER m_time_last_update;
	bool m_timer_active;
	static HFONT g_font;
	static pfc::list_t<t_message::ptr> g_messages;
	static pfc::list_t<HWND> g_notify_list;
};

pfc::list_t<HWND> console_window::g_notify_list;
pfc::ptr_list_t<console_window> console_window::list_wnd;
HFONT console_window::g_font = 0;
pfc::list_t<console_window::t_message::ptr> console_window::g_messages;
critical_section console_window::sync;

void console_window::g_update_all_fonts()
{
	if (g_font!=0)
	{
		unsigned n, count = console_window::list_wnd.get_count();
		for (n=0; n<count; n++)
		{
			HWND wnd = console_window::list_wnd[n]->wnd_edit;
			if (wnd) SendMessage(wnd,WM_SETFONT,(WPARAM)0,MAKELPARAM(0,0));
		}
		DeleteObject(g_font);
	}

	g_font = cui::fonts::helper(g_guid_console_font).get_font();

	unsigned n, count = console_window::list_wnd.get_count();
	for (n=0; n<count; n++)
	{
		HWND wnd = console_window::list_wnd[n]->wnd_edit;
		if (wnd) 
		{
			SendMessage(wnd,WM_SETFONT,(WPARAM)g_font,MAKELPARAM(TRUE,0));
		}
	}
}



console_window::console_window() : wnd_edit(NULL), m_editproc(NULL), m_timer_active(false)
{
	memset(&m_time_last_update, 0, sizeof(m_time_last_update));
}

console_window::~console_window()
{
}

void console_window::update_all_window_frames()
{
	unsigned n, count = list_wnd.get_count();
	long flags = 0;
	if (cfg_frame == 1) flags |= WS_EX_CLIENTEDGE;
	if (cfg_frame == 2) flags |= WS_EX_STATICEDGE;

	for (n=0; n<count; n++)
	{
		HWND wnd = list_wnd[n]->wnd_edit;
		if (wnd)
		{
			SetWindowLongPtr(wnd, GWL_EXSTYLE, flags);
			SetWindowPos(wnd,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
		}
	}
}

void console_window::update_content()
{
	insync(sync);
	pfc::string8_fastalloc buffer;
	buffer.prealloc(1024);
	unsigned n, count = g_messages.get_count();
	for (n=0; n<count; n++)
	{
		buffer << "[" << pfc::format_int(g_messages[n]->m_time.wHour, 2)
			<< ":" << pfc::format_int(g_messages[n]->m_time.wMinute, 2)
			<< ":" << pfc::format_int(g_messages[n]->m_time.wSecond, 2)
			<< "] " << g_messages[n]->m_message;
#if 0				
		buffer.add_string(pfc::string_printf("[%02u:%02u:%02u] ",(unsigned)g_messages[n].m_time.wHour
			,(unsigned)g_messages[n].m_time.wMinute
			,(unsigned)g_messages[n].m_time.wSecond));
		buffer.add_string(g_messages[n].m_message);
		//if (n != count-1)
		//	buffer.add_string("\r\n",2);
#endif
	}
	uSetWindowText(wnd_edit, buffer);
	LONG_PTR len = SendMessage(wnd_edit, EM_GETLINECOUNT , 0, 0);
	SendMessage(wnd_edit, EM_LINESCROLL , 0, len);
	QueryPerformanceCounter(&m_time_last_update);
}

void console_window::update_content_throttled()
{
	if (!m_timer_active)
	{
	
		LARGE_INTEGER current = {0}, freq = {0};
		QueryPerformanceCounter(&current);
		QueryPerformanceFrequency(&freq);
		t_uint64 tenth = 5;
		if (m_time_last_update.QuadPart)
		{
			tenth = (current.QuadPart - m_time_last_update.QuadPart) / (freq.QuadPart / 100);
		}
		if ( tenth < 25 )
		{
			SetTimer(get_wnd(), 667, 250 - t_uint32(tenth)*10, NULL);
			m_timer_active = true;
		}
		else update_content();
	}
}

LRESULT console_window::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{

	switch(msg)
	{
	case WM_CREATE:
		{
			/**
			* Store a pointer to ourselve in this list, used for global notifications (in the main thread)
			* which updates instances of our panel.
			*/
			list_wnd.add_item(this);
			{
				insync(sync);
				/** Store a window handle in this list, used in global notifications (in any thread) which
				* updates the panels */
				g_notify_list.add_item(wnd);
			}

			long flags = 0;
			if (cfg_frame == 1) flags |= WS_EX_CLIENTEDGE;
			else if (cfg_frame == 2) flags |= WS_EX_STATICEDGE;

			/** Create our edit window */
			wnd_edit = CreateWindowEx(flags, WC_EDIT, _T(""),
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY | ES_MULTILINE, 0, 0, 0, 0,
				wnd, HMENU(IDC_EDIT), core_api::get_my_instance(), NULL);

			if (wnd_edit)
			{
				if (g_font)
				{
					/** Nth, n>1, instance; use exisiting font handle */
					SendMessage(wnd_edit,WM_SETFONT,(WPARAM)g_font,MAKELPARAM(0,0));
				}
				else
					/** First window - create the font handle */
					g_update_all_fonts();

				/** Store a pointer to ourself in the user data field of the edit window */
				SetWindowLongPtr(wnd_edit,GWL_USERDATA,(LPARAM)(this));
				/** Subclass the edit window */
				m_editproc = (WNDPROC)SetWindowLongPtr(wnd_edit,GWL_WNDPROC,(LPARAM)(hook_proc));

				SendMessage(wnd, MSG_UPDATE, 0, 0);
			}
		}
		break;
	case WM_TIMER:
		if (wp == 667)
		{
			KillTimer(wnd, 667);
			m_timer_active = false;
			update_content();
			return 0;
		}
		break;
	/** Update the edit window's text */
	case MSG_UPDATE:
		{
			update_content_throttled();
		}
		break;
	case WM_GETMINMAXINFO:
		break;
	case WM_SIZE:
		/** Reposition the edit window. */
		SetWindowPos(wnd_edit, 0, 0, 0, LOWORD(lp), HIWORD(lp), SWP_NOZORDER);
		break;
	case WM_ERASEBKGND:
		return FALSE;
	case WM_DESTROY:
		{
			wnd_edit=0;
			list_wnd.remove_item(this);
			SendMessage(wnd_edit,WM_SETFONT,NULL,MAKELPARAM(0,0));
			if (list_wnd.get_count() == 0)
			{
				DeleteFont(g_font);
				g_font = 0;
			}
			{
				insync(sync);
				g_notify_list.remove_item(wnd);
			}
		}
		break;
	}
	return DefWindowProc(wnd, msg, wp, lp);
}

LRESULT WINAPI console_window::hook_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	console_window * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<console_window*>(GetWindowLongPtr(wnd,GWL_USERDATA));

	rv = p_this ? p_this->on_hook(wnd,msg,wp,lp) : DefWindowProc(wnd, msg, wp, lp);

	return rv;
}

LRESULT console_window::on_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{

	switch(msg)
	{
	case WM_KEYDOWN:
		/**
		* It's possible to assign right, left, up and down keys to keyboard shortcuts. But we would rather 
		* let the edit control proces those.
		*/
		if (get_host()->get_keyboard_shortcuts_enabled() && wp != VK_LEFT && wp != VK_RIGHT && wp != VK_UP && wp != VK_DOWN && g_process_keydown_keyboard_shortcuts(wp)) 
		{
			return 0;
		}
		else if (wp == VK_TAB)
		{
			ui_extension::window::g_on_tab(wnd);
			return 0;
		}
		break;
	case WM_SYSKEYDOWN:
		if (get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp))
			return 0;
		break;
	case WM_GETDLGCODE:
		break;
	case WM_CONTEXTMENU:
		if (wnd == (HWND)wp)
		{
			enum {ID_COPY=1, ID_CLEAR=2};

			POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
			if (pt.x == -1 && pt.x == -1)
			{
				RECT rc;
				GetRelativeRect(wnd, HWND_DESKTOP, &rc);
				pt.x = rc.left + (rc.right-rc.left)/2;
				pt.y = rc.top + (rc.bottom-rc.top)/2;
			}

			HMENU menu = CreatePopupMenu();
			DWORD start = 0, end = 0;
			SendMessage(wnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
			AppendMenu(menu, MF_STRING|(start != end ? NULL: MF_DISABLED), ID_COPY, L"&Copy");
			AppendMenu(menu, MF_SEPARATOR, NULL, NULL);
			AppendMenu(menu, MF_STRING, ID_CLEAR, L"C&lear");

			int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);

			DestroyMenu(menu);
			
			if (cmd)
			{
				if (cmd == ID_COPY)
					SendMessage(wnd, WM_COPY, NULL, NULL);
				else if (cmd == ID_CLEAR)
					g_clear();
			}
			return 0;
		}
		break;
	}
	return CallWindowProc(m_editproc,wnd,msg,wp,lp);
}

void console_window::get_name(pfc::string_base & out)const
{
	out.set_string("Console");
}
void console_window::get_category(pfc::string_base & out)const
{
	out.set_string("Panels");
}

/**
* This is the unique GUID identifying our panel. You should not re-use this one
* and generate your own using GUIDGEN.
*/
const GUID console_window::extension_guid = 
{ 0x3c85d0a9, 0x19d5, 0x4144, { 0xbc, 0xc2, 0x94, 0x9a, 0xb7, 0x64, 0x23, 0x3a } };

ui_extension::window_factory<console_window> blah;

class console_receiver_impl : public console_receiver
{
	/**
	* We assume that this function may be called from any thread.
	*
	* However, in most callbacks you would want to use, you can assume calls
	* come from the main thread.
	*
	* Check the documentation of the callback to find out if this is true for
	* the callback you wish to use.
	*/
	virtual void print(const char * p_message,unsigned p_message_length)
	{
		console_window::g_on_message_received(p_message, p_message_length);
	}
};

service_factory_single_t<console_receiver_impl> g_menu;

	class font_client_console : public cui::fonts::client
	{
	public:
		virtual const GUID & get_client_guid() const
		{
			return g_guid_console_font;
		}
		virtual void get_name (pfc::string_base & p_out) const
		{
			p_out = "Console";
		}

		virtual cui::fonts::font_type_t get_default_font_type() const
		{
			return cui::fonts::font_type_labels;
		}

		virtual void on_font_changed() const 
		{
			console_window::g_update_all_fonts();
			
		}
	};

	font_client_console::factory<font_client_console> g_font_client_console;
