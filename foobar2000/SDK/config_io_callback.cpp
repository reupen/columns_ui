#include "foobar2000.h"

void config_io_callback::g_on_read()
{
	service_enum_t<config_io_callback> e;
	service_ptr_t<config_io_callback> ptr;
	if (e.first(ptr)) do {
		ptr->on_read();
	} while(e.next(ptr));
}

void config_io_callback::g_on_write(bool reset)
{
	service_enum_t<config_io_callback> e;
	service_ptr_t<config_io_callback> ptr;
	if (e.first(ptr)) do {
		ptr->on_write(reset);
	} while(e.next(ptr));
}
