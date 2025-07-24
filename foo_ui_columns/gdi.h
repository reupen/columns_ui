#pragma once

namespace cui::gdi {

wil::unique_hbitmap create_hbitmap_from_32bpp_data(
    int width, int height, const void* data, size_t size, std::optional<int> stride = {});

}
