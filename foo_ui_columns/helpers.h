#pragma once

/*!
 * \file helpers.h
 *
 * \author musicmusic
 * \date March 2015
 *
 * Contains a few odd helper functions
 */

void g_ui_selection_manager_register_callback_no_now_playing_fallback(ui_selection_callback* p_callback);
bool g_ui_selection_manager_is_now_playing_fallback();

void g_compare_file_with_bytes(
    const service_ptr_t<file>& p1, const pfc::array_t<t_uint8>& p2, bool& b_same, abort_callback& p_abort);

HBITMAP LoadMonoBitmap(INT_PTR uid, COLORREF cr_btntext);
