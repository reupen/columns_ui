#include "foobar2000.h"

void modeless_dialog_manager::g_add(HWND p_wnd)
{
	service_enum_t<modeless_dialog_manager> e;
	service_ptr_t<modeless_dialog_manager> ptr;
	if (e.first(ptr)) do {
		ptr->add(p_wnd);
	} while(e.next(ptr));
}

void modeless_dialog_manager::g_remove(HWND p_wnd)
{
	service_enum_t<modeless_dialog_manager> e;
	service_ptr_t<modeless_dialog_manager> ptr;
	if (e.first(ptr)) do {
		ptr->remove(p_wnd);
	} while(e.next(ptr));
}