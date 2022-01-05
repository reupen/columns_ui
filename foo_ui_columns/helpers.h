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
BOOL uDrawPanelTitle(HDC dc, const RECT* rc_clip, const char* text, int len, bool vert, bool is_dark);

namespace cui::helpers {

class WindowEnum_t {
    static BOOL CALLBACK g_EnumWindowsProc(HWND wnd, LPARAM lp) { return ((WindowEnum_t*)lp)->EnumWindowsProc(wnd); }
    BOOL EnumWindowsProc(HWND wnd)
    {
        if (GetWindow(wnd, GW_OWNER) == m_wnd_owner && IsWindowVisible(wnd))
            m_wnd_list.add_item(wnd);
        return TRUE;
    }
    HWND m_wnd_owner;

public:
    void run() { EnumWindows(&g_EnumWindowsProc, (LPARAM)this); }
    pfc::list_t<HWND, pfc::alloc_fast> m_wnd_list;
    WindowEnum_t(HWND wnd_owner) : m_wnd_owner(wnd_owner){};
};

std::vector<HWND> get_child_windows(HWND wnd, std::function<bool(HWND)> filter = nullptr);
pfc::string8 get_last_win32_error_message();
bool open_web_page(HWND wnd, const wchar_t* url);
void clip_minmaxinfo(MINMAXINFO& mmi);
} // namespace cui::helpers
