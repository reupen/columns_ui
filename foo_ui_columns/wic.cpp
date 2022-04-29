#include "pch.h"

#include "wic.h"

namespace cui::wic {

namespace {

wil::com_ptr_t<IWICBitmapDecoder> create_decoder_from_path(std::string_view path)
{
    const auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    pfc::stringcvt::string_wide_from_utf8 utf16_path(path.data(), path.size());

    wil::com_ptr_t<IWICBitmapDecoder> bitmap_decoder;
    check_hresult(imaging_factory->CreateDecoderFromFilename(
        utf16_path.get_ptr(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmap_decoder));

    return bitmap_decoder;
}

wil::com_ptr_t<IWICBitmapDecoder> create_decoder_from_data(const void* data, size_t size)
{
    const auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    wil::com_ptr_t<IStream> stream;
    stream.attach(SHCreateMemStream(static_cast<const BYTE*>(data), gsl::narrow<UINT>(size)));

    wil::com_ptr_t<IWICBitmapDecoder> bitmap_decoder;
    check_hresult(imaging_factory->CreateDecoderFromStream(
        stream.get(), nullptr, WICDecodeMetadataCacheOnDemand, &bitmap_decoder));

    return bitmap_decoder;
}

wil::com_ptr_t<IWICBitmap> create_bitmap_from_hbitmap(HBITMAP bitmap)
{
    const auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    wil::com_ptr_t<IWICBitmap> wic_bitmap;
    check_hresult(imaging_factory->CreateBitmapFromHBITMAP(bitmap, nullptr, WICBitmapUseAlpha, &wic_bitmap));

    return wic_bitmap;
}

wil::com_ptr_t<IWICBitmapSource> get_image_frame(const wil::com_ptr_t<IWICBitmapDecoder>& bitmap_decoder)
{
    wil::com_ptr_t<IWICBitmapFrameDecode> bitmap_frame_decode;
    check_hresult(bitmap_decoder->GetFrame(0, &bitmap_frame_decode));

    wil::com_ptr_t<IWICBitmapSource> converted_bitmap;
    check_hresult(WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, bitmap_frame_decode.get(), &converted_bitmap));

    return converted_bitmap;
}

BitmapData decode_image(const wil::com_ptr_t<IWICBitmapSource>& bitmap_source)
{
    BitmapData image_data{};
    check_hresult(bitmap_source->GetSize(&image_data.width, &image_data.height));

    constexpr unsigned pixel_size = 4;
    const unsigned row_size = image_data.width * pixel_size - 1;
    const unsigned remainder = row_size % pixel_size;
    image_data.stride = row_size + pixel_size - remainder;

    image_data.data.resize(image_data.stride * image_data.height);
    check_hresult(bitmap_source->CopyPixels(
        nullptr, image_data.stride, gsl::narrow<UINT>(image_data.data.size()), image_data.data.data()));

    return image_data;
}

} // namespace

void check_hresult(HRESULT hr)
{
    pfc::string8 message;
    if (FAILED(hr))
        throw wic_error(message << "WIC error: " << format_win32_error(hr));
}

wil::com_ptr_t<IWICBitmapSource> resize_bitmap_source(
    const wil::com_ptr_t<IWICBitmapSource>& original_bitmap, int width, int height)
{
    const auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    wil::com_ptr_t<IWICBitmapScaler> bitmap_scaler;
    check_hresult(imaging_factory->CreateBitmapScaler(&bitmap_scaler));

    try {
        check_hresult(bitmap_scaler->Initialize(original_bitmap.get(), gsl::narrow<unsigned>(width),
            gsl::narrow<unsigned>(height), WICBitmapInterpolationModeHighQualityCubic));
    } catch (const wic_error&) {
        check_hresult(bitmap_scaler->Initialize(original_bitmap.get(), gsl::narrow<unsigned>(width),
            gsl::narrow<unsigned>(height), WICBitmapInterpolationModeFant));
    }

    return bitmap_scaler;
}

wil::unique_hbitmap resize_hbitmap(HBITMAP hbitmap, int width, int height)
{
    wil::com_ptr_t<IWICBitmapSource> bitmap_source = create_bitmap_from_hbitmap(hbitmap);
    bitmap_source = resize_bitmap_source(bitmap_source, width, height);
    return create_hbitmap_from_bitmap_source(bitmap_source);
}

wil::com_ptr_t<IWICBitmapSource> create_bitmap_source_from_path(const char* path)
{
    const auto decoder = create_decoder_from_path(path);
    return get_image_frame(decoder);
}

wil::unique_hbitmap create_hbitmap_from_bitmap_source(const wil::com_ptr_t<IWICBitmapSource>& source)
{
    const auto bitmap_data = decode_image(source);

    BITMAPINFOHEADER bmi{};

    bmi.biSize = sizeof(bmi);
    bmi.biWidth = gsl::narrow<long>(bitmap_data.width);
    bmi.biHeight = -gsl::narrow<long>(bitmap_data.height);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biCompression = BI_RGB;
    bmi.biClrUsed = 0;
    bmi.biClrImportant = 0;

    std::array<uint8_t, sizeof(BITMAPINFOHEADER)> bm_data{};

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
