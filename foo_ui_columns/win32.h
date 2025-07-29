#pragma once

namespace cui::win32 {

bool check_windows_10_build(DWORD build_number);
bool is_windows_11_rtm_or_newer();

std::wstring get_display_device_key(HMONITOR monitor);

} // namespace cui::win32
