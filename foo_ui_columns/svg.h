#pragma once

namespace cui::svg {

void ensure_available();
bool is_available();
wil::unique_hbitmap render_to_hbitmap(int width, int height, const void* data, size_t size,
    svg_services::ScalingMode scaling_mode, svg_services::PixelFormat = svg_services::PixelFormat::BGRA);

} // namespace cui::svg
