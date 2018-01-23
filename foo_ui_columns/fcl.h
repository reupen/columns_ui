#ifndef _COLUMNS_FCL_H_
#define _COLUMNS_FCL_H_

/*!
 * \file fcl.h
 *
 * \author musicmusic
 * \date March 2015
 *
 * Contains functions for importing and exporting FCL (Columns UI layout) files
 */

void g_export_layout(HWND wnd, pfc::string8 path = {}, bool is_quiet = false);
void g_import_layout(HWND wnd);
void g_import_layout(HWND wnd, const char* path, bool quiet = false);

namespace cui {
namespace fcl {
namespace groups {
extern const GUID titles_playlist_view, titles_common;
}
} // namespace fcl
} // namespace cui

#endif