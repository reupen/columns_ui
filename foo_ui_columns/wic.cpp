#include "pch.h"

#include "wic.h"

#include "gdi.h"

namespace cui::wic {

namespace {

const std::unordered_set auto_colour_space_conversion_pixel_formats{GUID_WICPixelFormat32bppCMYK,
    GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat80bppCMYKAlpha,
    GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf, GUID_WICPixelFormat32bppGrayFixedPoint,
    GUID_WICPixelFormat32bppGrayFloat};

wil::com_ptr<IWICBitmapDecoder> create_decoder_from_path(std::string_view path)
{
    const auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    pfc::stringcvt::string_wide_from_utf8 utf16_path(path.data(), path.size());

    wil::com_ptr<IWICBitmapDecoder> bitmap_decoder;
    check_hresult(imaging_factory->CreateDecoderFromFilename(
        utf16_path.get_ptr(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmap_decoder));

    return bitmap_decoder;
}

wil::com_ptr<IWICBitmap> create_bitmap_from_hbitmap(HBITMAP bitmap)
{
    const auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    wil::com_ptr<IWICBitmap> wic_bitmap;
    check_hresult(imaging_factory->CreateBitmapFromHBITMAP(bitmap, nullptr, WICBitmapUseAlpha, &wic_bitmap));

    return wic_bitmap;
}

BitmapData decode_bitmap_source(const wil::com_ptr<IWICBitmapSource>& bitmap_source)
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
wil::com_ptr<IWICImagingFactory> create_factory()
{
    return wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);
}

wil::com_ptr<IWICBitmapDecoder> create_decoder_from_data(
    const void* data, size_t size, const wil::com_ptr<IWICImagingFactory>& imaging_factory)
{
    wil::com_ptr<IStream> stream;
    stream.attach(SHCreateMemStream(static_cast<const BYTE*>(data), gsl::narrow<UINT>(size)));

    wil::com_ptr<IWICBitmapDecoder> bitmap_decoder;
    check_hresult(imaging_factory->CreateDecoderFromStream(
        stream.get(), nullptr, WICDecodeMetadataCacheOnDemand, &bitmap_decoder));

    return bitmap_decoder;
}

wil::com_ptr<IWICBitmapSource> get_image_frame(
    const wil::com_ptr<IWICBitmapDecoder>& bitmap_decoder, REFWICPixelFormatGUID pixel_format)
{
    wil::com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
    check_hresult(bitmap_decoder->GetFrame(0, &bitmap_frame_decode));

    wil::com_ptr<IWICBitmapSource> converted_bitmap;
    check_hresult(WICConvertBitmapSource(pixel_format, bitmap_frame_decode.get(), &converted_bitmap));

    return converted_bitmap;
}

wil::com_ptr<IWICBitmapSource> resize_bitmap_source(const wil::com_ptr<IWICBitmapSource>& original_bitmap, int width,
    int height, const wil::com_ptr<IWICImagingFactory>& imaging_factory)
{
    wil::com_ptr<IWICBitmapScaler> bitmap_scaler;
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
    wil::com_ptr<IWICBitmapSource> bitmap_source = create_bitmap_from_hbitmap(hbitmap);
    bitmap_source = resize_bitmap_source(bitmap_source, width, height);
    return create_hbitmap_from_bitmap_source(bitmap_source);
}

wil::com_ptr<IWICColorContext> get_bitmap_source_colour_context(const wil::com_ptr<IWICImagingFactory>& imaging_factory,
    const wil::com_ptr<IWICBitmapFrameDecode>& bitmap_frame_decode)
{
    wil::com_ptr<IWICColorContext> wic_colour_context;
    THROW_IF_FAILED(imaging_factory->CreateColorContext(&wic_colour_context));

    unsigned colour_context_count{};
    // Not supported on Wine
    if (const auto hr = bitmap_frame_decode->GetColorContexts(1, wic_colour_context.addressof(), &colour_context_count);
        FAILED(hr)) {
        LOG_HR_IF(hr, hr != WINCODEC_ERR_UNSUPPORTEDOPERATION);
        return {};
    }

    if (colour_context_count > 0)
        return wic_colour_context;

    return {};
}

bool is_auto_colour_space_conversion_pixel_format(REFWICPixelFormatGUID pixel_format)
{
    return auto_colour_space_conversion_pixel_formats.contains(pixel_format);
}

wil::com_ptr<IWICBitmapSource> create_bitmap_source_from_path(const char* path)
{
    const auto decoder = create_decoder_from_path(path);
    return get_image_frame(decoder);
}

wil::com_ptr<IWICBitmapSource> create_bitmap_source_from_bitmap_data(const BitmapData& bitmap_data)
{
    const auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);
    wil::com_ptr<IWICBitmap> bitmap;

    check_hresult(imaging_factory->CreateBitmapFromMemory(bitmap_data.width, bitmap_data.height,
        GUID_WICPixelFormat32bppBGRA, bitmap_data.stride, gsl::narrow<UINT>(bitmap_data.data.size()),
        const_cast<BYTE*>(bitmap_data.data.data()), &bitmap));

    return bitmap;
}

wil::unique_hbitmap create_hbitmap_from_bitmap_source(const wil::com_ptr<IWICBitmapSource>& source)
{
    const auto bitmap_data = decode_bitmap_source(source);

    return gdi::create_hbitmap_from_32bpp_data(gsl::narrow<int>(bitmap_data.width),
        gsl::narrow<int>(bitmap_data.height), bitmap_data.data.data(), bitmap_data.data.size());
}

BitmapData decode_image_data(const void* data, size_t size)
{
    const auto decoder = create_decoder_from_data(data, size);
    const auto converted_bitmap = get_image_frame(decoder);
    return decode_bitmap_source(converted_bitmap);
}

std::optional<uint32_t> get_icc_colour_space_signature(const wil::com_ptr<IWICColorContext>& colour_context)
{
    try {
        WICColorContextType type{};
        THROW_IF_FAILED(colour_context->GetType(&type));

        if (type != WICColorContextProfile)
            return {};

        uint32_t profile_size{};
        THROW_IF_FAILED(colour_context->GetProfileBytes(0, nullptr, &profile_size));

        std::vector<uint8_t> header(profile_size);
        THROW_IF_FAILED(colour_context->GetProfileBytes(profile_size, header.data(), &profile_size));

        if (profile_size < 20)
            return {};

        return pfc::decode_big_endian<uint32_t>(&header[16]);
    }
    CATCH_LOG()

    return {};
}

namespace {

void collect_metadata(const wil::com_ptr<IWICMetadataQueryReader>& reader, const std::wstring& path,
    MetadataCollection& collected_metadata)
{
    wil::com_ptr<IEnumString> enum_string;
    THROW_IF_FAILED(reader->GetEnumerator(&enum_string));

    wil::unique_cotaskmem_string current_string;
    unsigned long read{};
    while (enum_string->Next(1, &current_string, &read) == S_OK) {
        // The AVIF codec seems to have some bug where it doesn’t return S_FALSE
        if (read == 0)
            break;

        const auto full_path = path + current_string.get();

        wil::unique_prop_variant variant;
        THROW_IF_FAILED(reader->GetMetadataByName(current_string.get(), &variant));

        MetadataValue value;

        // Only the variant types that have been encountered with real images
        // are handled here
        switch (variant.vt) {
        case VT_UNKNOWN: {
            wil::com_ptr<IWICMetadataQueryReader> nested_reader;
            wil::try_com_query_to(variant.punkVal, &nested_reader);

            if (nested_reader)
                collect_metadata(nested_reader, full_path, collected_metadata);
            else
                collected_metadata.emplace_back(full_path, L"Unknown object");
            continue;
        }
        case VT_EMPTY:
            value = L"<empty>";
            break;
        case VT_BSTR:
            value = variant.bstrVal;
            break;
        case VT_LPWSTR:
            value = variant.pwszVal;
            break;
        case VT_LPSTR:
            value = mmh::to_utf16(variant.pszVal);
            break;
        case VT_BOOL:
            value = static_cast<uint64_t>(variant.boolVal);
            break;
        case VT_UI1:
            value = static_cast<uint64_t>(variant.bVal);
            break;
        case VT_UI2:
            value = static_cast<uint64_t>(variant.uiVal);
            break;
        case VT_UI4:
            value = static_cast<uint64_t>(variant.ulVal);
            break;
        case VT_UI8:
            value = std::format(L"{} ({}/{})", variant.uhVal.QuadPart, variant.uhVal.LowPart, variant.uhVal.HighPart);
            break;
        case VT_I1:
            value = static_cast<int64_t>(variant.cVal);
            break;
        case VT_I2:
            value = static_cast<int64_t>(variant.iVal);
            break;
        case VT_I4:
            value = static_cast<int64_t>(variant.lVal);
            break;
        case VT_I8:
            value = std::format(L"{} ({}/{})", variant.hVal.QuadPart, variant.hVal.LowPart, variant.hVal.HighPart);
            break;
        case VT_BLOB:
            value = fmt::format(L"<binary data, {} bytes>", variant.blob.cbSize);
            break;
        case VT_VECTOR | VT_UI1:
        case VT_VECTOR | VT_UI2:
        case VT_VECTOR | VT_UI4:
        case VT_VECTOR | VT_UI8:
        case VT_VECTOR | VT_I1:
        case VT_VECTOR | VT_I2:
        case VT_VECTOR | VT_I4:
        case VT_VECTOR | VT_I8:
            // Each vector type has its own union member, but only the element count is
            // being used here
            value = fmt::format(L"<vector, {} integers>", variant.caui.cElems);
            break;
        default:
            value = fmt::format(L"unhandled variant type {}", variant.vt);
            break;
        }

        collected_metadata.emplace_back(full_path, value);
    }
}

} // namespace

MetadataCollection get_image_metadata(const wil::com_ptr<IWICBitmapFrameDecode>& bitmap_frame_decode)
{
    MetadataCollection collected_metadata;

    wil::com_ptr<IWICMetadataQueryReader> metadata_reader;
    THROW_IF_FAILED(bitmap_frame_decode->GetMetadataQueryReader(&metadata_reader));

    wil::com_ptr<IEnumString> strings;
    THROW_IF_FAILED(metadata_reader->GetEnumerator(&strings));

    collect_metadata(metadata_reader, L"", collected_metadata);

    return collected_metadata;
}

} // namespace cui::wic
