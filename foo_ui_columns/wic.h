#pragma once

namespace cui::wic {

class wic_error : public std::exception {
public:
    using std::exception::exception;
};

struct BitmapData {
    unsigned width{};
    unsigned height{};
    unsigned stride{};
    std::vector<uint8_t> data;
};

void check_hresult(HRESULT hr);
wil::com_ptr<IWICBitmapSource> create_bitmap_source_from_path(const char* path);
wil::com_ptr<IWICBitmapSource> create_bitmap_source_from_bitmap_data(const BitmapData& bitmap_data);
wil::unique_hbitmap create_hbitmap_from_bitmap_source(const wil::com_ptr<IWICBitmapSource>& source);
wil::com_ptr<IWICBitmapSource> resize_bitmap_source(
    const wil::com_ptr<IWICBitmapSource>& original_bitmap, int width, int height);
wil::unique_hbitmap resize_hbitmap(HBITMAP original_bitmap, int width, int height);

BitmapData decode_image_data(const void* data, size_t size);

} // namespace cui::wic
