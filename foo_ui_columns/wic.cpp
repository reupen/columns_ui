#include "stdafx.h"

#include "wic.h"

namespace cui::wic {

void check_hresult(HRESULT hr)
{
    pfc::string8 message;
    if (FAILED(hr))
        throw pfc::exception(message << "WIC error: " << format_win32_error(hr));
}

wil::com_ptr_t<IWICBitmapDecoder> create_decoder_from_data(const void* data, size_t size)
{
    auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    const wil::com_ptr_t<IStream> stream = new mmh::IStreamMemblock(static_cast<const uint8_t*>(data), size);

    wil::com_ptr_t<IWICBitmapDecoder> bitmap_decoder;
    check_hresult(imaging_factory->CreateDecoderFromStream(
        stream.get(), nullptr, WICDecodeMetadataCacheOnDemand, bitmap_decoder.addressof()));

    return bitmap_decoder;
}

wil::com_ptr_t<IWICBitmapDecoder> create_decoder_from_path(std::string_view path)
{
    auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    pfc::stringcvt::string_wide_from_utf8 utf16_path(path.data(), path.size());

    wil::com_ptr_t<IWICBitmapDecoder> bitmap_decoder;
    check_hresult(imaging_factory->CreateDecoderFromFilename(
        utf16_path.get_ptr(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, bitmap_decoder.addressof()));

    return bitmap_decoder;
}

wil::com_ptr_t<IWICBitmapSource> get_image_frame(const wil::com_ptr_t<IWICBitmapDecoder>& bitmap_decoder)
{
    wil::com_ptr_t<IWICBitmapFrameDecode> bitmap_frame_decode;
    check_hresult(bitmap_decoder->GetFrame(0, bitmap_frame_decode.addressof()));

    wil::com_ptr_t<IWICBitmapSource> converted_bitmap;
    check_hresult(
        WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, bitmap_frame_decode.get(), converted_bitmap.addressof()));

    return converted_bitmap;
}

BitmapData decode_image(const wil::com_ptr_t<IWICBitmapSource>& bitmap_source)
{
    BitmapData image_data{};
    check_hresult(bitmap_source->GetSize(&image_data.width, &image_data.height));

    constexpr size_t pixel_size = 4;
    const size_t row_size = image_data.width * pixel_size - 1;
    const size_t remainder = row_size % pixel_size;
    image_data.stride = row_size + pixel_size - remainder;

    image_data.data.resize(image_data.stride * image_data.height);
    check_hresult(bitmap_source->CopyPixels(nullptr, image_data.stride, image_data.data.size(), image_data.data.data()));

    return image_data;
}

wil::unique_hbitmap create_hbitmap_from_path(const char* path)
{
    const auto decoder = create_decoder_from_path(path);
    const auto converted_bitmap = get_image_frame(decoder);
    const auto bitmap_data = decode_image(converted_bitmap);

    BITMAPINFOHEADER bmi{};

    bmi.biSize = sizeof(bmi);
    bmi.biWidth = gsl::narrow<long>(bitmap_data.width);
    bmi.biHeight = -gsl::narrow<long>(bitmap_data.height);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biCompression = BI_RGB;
    bmi.biClrUsed = 0;
    bmi.biClrImportant = 0;

    std::array<t_uint8, sizeof(BITMAPINFOHEADER)> bm_data{};

    auto* bi = reinterpret_cast<BITMAPINFO*>(bm_data.data());
    bi->bmiHeader = bmi;

    void* data{};
    wil::unique_hbitmap hbitmap(CreateDIBSection(nullptr, bi, DIB_RGB_COLORS, &data, nullptr, 0));

    if (data) {
        GdiFlush();
        memcpy(data, bitmap_data.data.data(), bitmap_data.data.size());
    }

    return hbitmap;
}

BitmapData decode_image_data(const void* data, size_t size)
{
    const auto decoder = create_decoder_from_data(data, size);
    const auto converted_bitmap = get_image_frame(decoder);
    return decode_image(converted_bitmap);
}

} // namespace cui::wic
