#ifndef _COLUMNS_FCS_H_
#define _COLUMNS_FCS_H_

/*!
 * \file fcs.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Functions for importing deprecated FCS files. Export functions no longer used.
 */

bool g_import(const char * path);
bool g_export(const char * path);

void import(HWND wnd);
void _export(HWND wnd);

#endif