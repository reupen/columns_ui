#ifndef _COLUMNS_EXTERN_H_
#define _COLUMNS_EXTERN_H_

extern HWND g_main_window,
	g_tooltip,
	g_rebar,
	g_status
	;


extern bool g_playing;

extern HICON g_icon;
extern bool 
	ui_initialising,
	g_minimised ,
	g_icon_created;

extern HFONT g_font;

extern pfc::string8 statusbartext;

extern HIMAGELIST  g_imagelist;

#endif