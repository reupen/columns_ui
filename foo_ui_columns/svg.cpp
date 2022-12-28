#include "pch.h"

#include "svg.h"
#include "gdi.h"

namespace cui::svg {

svg_services::svg_services::ptr get_api()
{
    svg_services::svg_services::ptr api;

    if (!fb2k::std_api_try_get(api)) {
        throw exception_service_not_found(
            "A compatible version of the SVG services component is required for SVG support.");
    }

    return api;
}

void ensure_available()
{
    get_api();
}

bool is_available()
{
    return static_api_test_t<svg_services::svg_services>();
}

wil::unique_hbitmap render_to_hbitmap(int width, int height, const void* data, size_t size,
    svg_services::ScalingMode scaling_mode, svg_services::PixelFormat pixel_format)
{
    const svg_services::svg_services::ptr svg_api = get_api();

    std::vector<uint8_t> bitmap_data(static_cast<size_t>(width) * static_cast<size_t>(height) * size_t{4});

    const auto svg_document = svg_api->open(data, size);
    svg_document->render(width, height, svg_services::Position::Centred, scaling_mode, pixel_format, bitmap_data.data(),
        bitmap_data.size());

    return gdi::create_hbitmap_from_32bpp_data(width, height, bitmap_data.data(), bitmap_data.size());
}

} // namespace cui::svg
