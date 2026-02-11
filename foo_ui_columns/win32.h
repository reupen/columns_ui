#pragma once

namespace cui::win32 {

bool is_windows_11_rtm_or_newer();

std::wstring get_display_device_key(HMONITOR monitor);

void add_window_styles(HWND wnd, DWORD styles_to_add);
void remove_window_styles(HWND wnd, DWORD styles_to_remove);

} // namespace cui::win32
