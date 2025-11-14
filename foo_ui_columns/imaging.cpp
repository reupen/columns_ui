#include "pch.h"

#include "d2d_utils.h"
#include "wcs.h"

namespace cui::utils {

std::tuple<int, int> calculate_scaled_image_size(int image_width, int image_height, int target_width, int target_height,
    bool preserve_aspect_ratio, bool ensure_even_margins)
{
    int scaled_width = target_width;
    int scaled_height = target_height;

    const double source_aspect_ratio = gsl::narrow_cast<double>(image_width) / gsl::narrow_cast<double>(image_height);
    const double client_aspect_ratio = gsl::narrow_cast<double>(target_width) / gsl::narrow_cast<double>(target_height);

    if (preserve_aspect_ratio) {
        if (client_aspect_ratio < source_aspect_ratio)
            scaled_height
                = gsl::narrow_cast<unsigned>(floor(gsl::narrow_cast<double>(target_width) / source_aspect_ratio));
        else if (client_aspect_ratio > source_aspect_ratio)
            scaled_width
                = gsl::narrow_cast<unsigned>(floor(gsl::narrow_cast<double>(target_height) * source_aspect_ratio));
    }

    if (ensure_even_margins) {
        if (((target_height - scaled_height) % 2) != 0)
            ++scaled_height;

        if (((target_width - scaled_width) % 2) != 0)
            ++scaled_width;
    }

    return {scaled_width, scaled_height};
}

wil::com_ptr<ID2D1ColorContext> create_d2d_colour_context_for_display_profile(
    const wil::com_ptr<ID2D1DeviceContext>& d2d_device_context, const std::wstring& display_profile_name)
{
    wil::com_ptr<ID2D1ColorContext> d2d_colour_context;

    const auto profile = wcs::get_display_colour_profile(display_profile_name);

    if (profile.empty())
        return {};

    try {
        THROW_IF_FAILED(d2d_device_context->CreateColorContext(
            D2D1_COLOR_SPACE_CUSTOM, profile.data(), gsl::narrow<uint32_t>(profile.size()), &d2d_colour_context));
    } catch (const wil::ResultException& ex) {
        if (uih::d2d::is_device_reset_error(ex.GetErrorCode()))
            throw;

        wcs::mark_display_colour_profile_as_bad(display_profile_name);

        const auto error_message = mmh::to_utf8(mmh::win32::format_error(ex.GetErrorCode()));
        const auto log_message = fmt::format("Columns UI – failed to create colour context from display colour profile "
                                             "\"{}\": {}. This display profile will not be used.",
            mmh::to_utf8(display_profile_name), error_message);
        console::print(log_message.c_str());
    }

    return d2d_colour_context;
}

} // namespace cui::utils
