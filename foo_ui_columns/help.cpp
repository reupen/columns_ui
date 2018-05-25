#include "stdafx.h"

#include "help.h"
#include "urls.h"

namespace cui::help {

void open_colour_script_help(HWND wnd)
{
    helpers::open_web_page(wnd, urls::colour_script_help.data());
}

void open_global_variables_help(HWND wnd)
{
    helpers::open_web_page(wnd, urls::global_variables_help.data());
}

} // namespace cui::help
