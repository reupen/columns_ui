#ifndef _COLUMNS_FCL_H_
#define _COLUMNS_FCL_H_

void g_export_layout(HWND wnd);
//void g_export_layout(HWND wnd, const char * path);
void g_import_layout(HWND wnd);
void g_import_layout(HWND wnd, const char * path);

namespace cui {	namespace fcl { namespace groups {
	extern const GUID titles_playlist_view, titles_common;
	} } }

#endif