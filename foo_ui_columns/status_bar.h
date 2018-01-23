#pragma once

/*!
 * \file status_bar.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Portions of the status bar code
 */

#include "volume.h"

void status_set_menu(bool on);

namespace status_bar {
extern bool b_lock;
extern unsigned u_length_pos;
extern unsigned u_lock_pos;
extern unsigned u_vol_pos;
extern HICON icon_lock;
extern HTHEME thm_status;

extern volume_popup_t volume_popup_window;

HICON get_icon();
void destroy_icon();
void destroy_theme_handle();
void create_theme_handle();
void destroy_status_window();

extern pfc::string8 menudesc;

}; // namespace status_bar
