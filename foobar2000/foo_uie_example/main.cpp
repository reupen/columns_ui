/**
* \file main.cpp
*
* \brief Example panel component
*
* This component is an example of a very simple multiple instance panel that does not take keyboard input
*/

#include "../SDK/foobar2000.h"
#include "../helpers/helpers.h"
#include <commctrl.h>
#include <windowsx.h>

#include "../columns_ui-sdk/ui_extension.h"

/** Declare some information about our component */
DECLARE_COMPONENT_VERSION("Example Columns UI Panel",
"0.1",
"compiled: " __DATE__ "\n"
"with Panel API version: " UI_EXTENSION_VERSION
);

/** Our window class */
class example_window : public uie::container_ui_extension
{
public:
	example_window();
	~example_window();

	virtual const GUID & get_extension_guid() const;
	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;
	unsigned get_type () const;

private:
	/** Our window procedure */
	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	virtual class_data & get_class_data()const; 
	virtual void get_menu_items (uie::menu_hook_t & p_hook);

	static const GUID g_extension_guid;
	
	/** Our child window */
	HWND wnd_static;
};

class menu_node_close : public ui_extension::menu_node_command_t
{
	service_ptr_t<example_window> p_this;
public:
	virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags) const
	{
		p_out = "Close";
		p_displayflags= 0;
		return true;
	}
	virtual bool get_description(pfc::string_base & p_out) const
	{
		return false;
	}
	virtual void execute()
	{
		HWND wnd = p_this->get_wnd();
		uie::window_host_ptr p_host = p_this->get_host();
		uie::window_ptr(p_this)->destroy_window();
		p_host->relinquish_ownership(wnd);
	}
	menu_node_close(example_window * wnd) : p_this(wnd) {};
};

void example_window::get_menu_items (uie::menu_hook_t & p_hook) 
{
	p_hook.add_node(new menu_node_close(this));
};

example_window::example_window() : wnd_static(NULL)
{}

example_window::~example_window()
{}

LRESULT example_window::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{

	switch(msg)
	{
	case WM_CREATE:
		{
			RECT rc;
			GetClientRect(wnd, &rc);

			/** Create a static window, with text "Example" */
			wnd_static = CreateWindowEx(0, WC_STATIC, _T("Example panel"),
				WS_CHILD | WS_VISIBLE, 0, 0, rc.right, rc.bottom,
				wnd, HMENU(0), core_api::get_my_instance(), NULL);
		}
		break;
	case WM_SIZE:
		/** Apparently, the static control sucks */
		RedrawWindow(wnd_static, 0, 0, RDW_INVALIDATE|RDW_ERASE);
		/** Reposition our child window */
		SetWindowPos(wnd_static, 0, 0, 0, LOWORD(lp), HIWORD(lp), SWP_NOZORDER);
		break;
	case WM_DESTROY:
		/** DefWindowProc will destroy our child window. Set our window handle to NULL now. */
		wnd_static=NULL;
		break;
	}
	return DefWindowProc(wnd, msg, wp, lp);
}

example_window::class_data & example_window::get_class_data() const 
{
	__implement_get_class_data(_T("{79F574F1-DC70-4f3f-8155-384B00AE0679}"), true);
}

const GUID & example_window::get_extension_guid() const
{
	return g_extension_guid;
}

void example_window::get_name(pfc::string_base & out)const
{
	out.set_string("Example");
}
void example_window::get_category(pfc::string_base & out)const
{
	out.set_string("Panels");
}
unsigned example_window::get_type() const{return uie::type_panel;}

/** Do not use the same GUID! */
// {79F574F1-DC70-4f3f-8155-384B00AE0679}
const GUID example_window::g_extension_guid = 
{ 0x79f574f1, 0xdc70, 0x4f3f, { 0x81, 0x55, 0x38, 0x4b, 0x0, 0xae, 0x6, 0x79 } };

uie::window_factory<example_window> g_example_window_factory;