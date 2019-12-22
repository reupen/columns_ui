#pragma once

namespace cui::wic {

struct BitmapData {
    unsigned width{};
    unsigned height{};
    unsigned stride{};
    std::vector<uint8_t> data;
};

wil::unique_hbitmap create_hbitmap_from_path(const char* path);
BitmapData decode_image_data(const void* data, size_t size);

} // namespace cui::wic
