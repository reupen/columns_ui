#ifndef _COLUMNS_UI_PLHOOK_H_
#define _COLUMNS_UI_PLHOOK_H_

#include "foo_ui_columns.h"
#include "callback.h"
#include "extern.h"

LRESULT WINAPI TabHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
//LRESULT WINAPI PlistHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

#endif