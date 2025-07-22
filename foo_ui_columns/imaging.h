#pragma once

namespace cui::utils {

std::tuple<int, int> calculate_scaled_image_size(int image_width, int image_height, int target_width, int target_height,
    bool preserve_aspect_ratio, bool ensure_even_margins);

}
