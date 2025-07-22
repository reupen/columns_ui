#include "pch.h"

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

} // namespace cui::utils
