#pragma once

namespace cui::wcs {

std::wstring get_display_colour_profile_name(const wchar_t* device_key);
std::vector<uint8_t> get_display_colour_profile(const std::wstring& filename);
void reset_colour_profiles();

} // namespace cui::wcs
