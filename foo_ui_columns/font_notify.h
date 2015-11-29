#ifndef _COLUMNS_FONT_H_
#define _COLUMNS_FONT_H_

/*!
 * \file font_notify.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Handles font changes for main UI components
 */

#include "stdafx.h"
#include "extern.h"

extern HFONT g_status_font;
extern HFONT g_tab_font;
extern HFONT g_header_font;
extern HFONT g_plist_font;

void on_playlist_font_change();
//void on_show_sidebar_change();
void on_show_toolbars_change();
void on_show_status_change();
void on_show_status_pane_change();
void on_status_font_change();
void on_header_font_change();
//void on_tab_font_change();

void font_cleanup();

void g_get_font_size_next_step (LOGFONT & p_lf, bool up);
void set_font_size(bool up);

#endif