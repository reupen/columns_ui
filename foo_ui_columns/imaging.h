#pragma once

namespace cui::utils {

std::tuple<int, int> calculate_scaled_image_size(int image_width, int image_height, int target_width, int target_height,
    bool preserve_aspect_ratio, bool ensure_even_margins);

wil::com_ptr<ID2D1ColorContext> create_d2d_colour_context_for_display_profile(
    const wil::com_ptr<ID2D1DeviceContext>& d2d_device_context, const std::wstring& display_profile_name);

} // namespace cui::utils
