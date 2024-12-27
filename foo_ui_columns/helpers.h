#pragma once

void g_ui_selection_manager_register_callback_no_now_playing_fallback(ui_selection_callback* p_callback);
bool g_ui_selection_manager_is_now_playing_fallback();

void g_compare_file_with_bytes(
    const service_ptr_t<file>& p1, const pfc::array_t<uint8_t>& p2, bool& b_same, abort_callback& p_abort);

HBITMAP LoadMonoBitmap(WORD uid, COLORREF cr_btntext);
BOOL uDrawPanelTitle(HDC dc, const RECT* rc_clip, const char* text, int len, bool is_font_vertical, bool is_dark);

namespace cui::helpers {

class WindowEnum_t {
    static BOOL CALLBACK g_EnumWindowsProc(HWND wnd, LPARAM lp) noexcept
    {
        return ((WindowEnum_t*)lp)->EnumWindowsProc(wnd);
    }
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
    explicit WindowEnum_t(HWND wnd_owner) : m_wnd_owner(wnd_owner) {}
};

std::vector<HWND> get_child_windows(HWND wnd, std::function<bool(HWND)> filter = nullptr);
pfc::string8 get_last_win32_error_message();
bool open_web_page(HWND wnd, const wchar_t* url);
void clip_minmaxinfo(MINMAXINFO& mmi);
void handle_tabs_ctrl_tab(MSG* msg, HWND wnd_container, HWND wnd_tabs);

} // namespace cui::helpers
