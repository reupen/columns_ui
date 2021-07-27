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

namespace cui::status_bar {

extern bool b_lock;
extern unsigned u_length_pos;
extern unsigned u_lock_pos;
extern unsigned u_vol_pos;
extern HICON icon_lock;
extern HTHEME thm_status;
extern pfc::string8 menudesc;
extern pfc::string8 statusbartext;

extern volume_popup_t volume_popup_window;

enum StatusBarPart : uint32_t {
    t_parts_none = 0,
    t_parts_all = 0xffffffff,
    t_part_main = 1 << 0,
    t_part_lock = 1 << 1,
    t_part_length = 1 << 2,
    t_part_volume = 1 << 3
};

void set_part_sizes(unsigned p_parts = t_parts_none);

HICON get_icon();
void regenerate_text();
void update_text(bool is_caller_menu_desc);
void create_window();
void destroy_icon();
void destroy_theme_handle();
void create_theme_handle();
void destroy_window();
void set_show_menu_item_description(bool on);
void on_status_font_change();

} // namespace cui::status_bar
