#pragma once

/*!
 * \file setup_dialog.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Initial quick set-up dialog box
 */

#include "layout.h"

class setup_dialog_t : public pfc::refcounted_object_root {
    pfc::list_t<cfg_layout_t::preset> m_presets;
    uie::splitter_item_ptr m_previous_layout;
    cui::colours::colour_mode_t m_previous_colour_mode;
    bool m_previous_show_artwork, m_previous_show_grouping;
    using ptr_t = pfc::refcounted_object_ptr_t<setup_dialog_t>;
    ptr_t m_this;
    static BOOL CALLBACK g_SetupDialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    BOOL SetupDialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

public:
    static void g_run();
};
