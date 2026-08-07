#pragma once

namespace cui::win32 {

bool is_windows_11_rtm_or_newer();

std::wstring get_display_device_key(HMONITOR monitor);

void add_window_styles(HWND wnd, DWORD styles_to_add);
void remove_window_styles(HWND wnd, DWORD styles_to_remove);

void handle_tab_key(HWND wnd);

HWND find_window_for_menu(HMENU menu);

} // namespace cui::win32
