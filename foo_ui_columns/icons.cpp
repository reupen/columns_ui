#include "pch.h"

#include "icons.h"

namespace cui::icons {

namespace {

const std::unordered_set<int> standard_sizes{16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 256};

}

bool is_standard_size(int width, int height)
{
    return width == height && standard_sizes.contains(width);
}

bool use_svg_icon(int width, int height)
{
    return svg::is_available() && !is_standard_size(width, height);
}

wil::unique_hbitmap render_svg(IconConfig icon_config, int width, int height, svg_services::PixelFormat pixel_format)
{
    const auto [data, size] = resource_utils::get_resource_data(icon_config.svg_id(), L"SVG");
    return svg::render_to_hbitmap(width, height, data, size, svg_services::ScalingMode::Stretch, pixel_format);
}

wil::unique_hicon load_icon(IconConfig icon_config, int width, int height)
{
    return wil::unique_hicon(static_cast<HICON>(LoadImage(
        wil::GetModuleInstanceHandle(), MAKEINTRESOURCE(icon_config.icon_id()), IMAGE_ICON, width, height, NULL)));
}

} // namespace cui::icons
