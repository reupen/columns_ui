#include "pch.h"

#include "artwork_decoder.h"

namespace cui::artwork_panel {

namespace {

wil::com_ptr<IWICColorContext> get_bitmap_source_colour_context(const wil::com_ptr<IWICImagingFactory>& imaging_factory,
    const wil::com_ptr<IWICBitmapFrameDecode>& bitmap_frame_decode)
{
    wil::com_ptr<IWICColorContext> wic_colour_context;
    THROW_IF_FAILED(imaging_factory->CreateColorContext(&wic_colour_context));

    unsigned colour_context_count{};
    // Not supported on Wine
    if (const auto hr = LOG_IF_FAILED(
            bitmap_frame_decode->GetColorContexts(1, wic_colour_context.addressof(), &colour_context_count));
        FAILED(hr))
        return {};

    if (colour_context_count > 0)
        return wic_colour_context;

    return {};
}

auto get_bitmap_source_pixel_format_info(const wil::com_ptr<IWICImagingFactory>& imaging_factory,
    const wil::com_ptr<IWICBitmapFrameDecode>& bitmap_frame_decode)
{
    WICPixelFormatGUID source_pixel_format{};
    THROW_IF_FAILED(bitmap_frame_decode->GetPixelFormat(&source_pixel_format));

    wil::com_ptr<IWICComponentInfo> wic_component_info;
    THROW_IF_FAILED(imaging_factory->CreateComponentInfo(source_pixel_format, &wic_component_info));

    return wic_component_info.query<IWICPixelFormatInfo2>();
}

} // namespace

void ArtworkDecoder::decode(
    wil::com_ptr<ID2D1DeviceContext> d2d_render_target, album_art_data_ptr data, std::function<void()> on_complete)
{
    reset();
    m_current_task = std::make_shared<ArtworkDecoderTask>(std::move(on_complete));

    m_current_task->future = std::async(std::launch::async,
        [this, data, d2d_render_target{std::move(d2d_render_target)},
            stop_token{m_current_task->stop_source.get_token()}, task{std::weak_ptr{m_current_task}}]() noexcept {
            TRACK_CALL_TEXT("cui::artwork_panel::ArtworkDecoder::async_task");

            wil::com_ptr<ID2D1Bitmap> d2d_bitmap;
            wil::com_ptr<ID2D1ColorContext> d2d_colour_context;
            bool is_float{};

            try {
                if (stop_token.stop_requested())
                    return;

                auto _ = wil::CoInitializeEx(COINIT_MULTITHREADED);
                const auto imaging_factory = wic::create_factory();
                const auto decoder = wic::create_decoder_from_data(data->get_ptr(), data->get_size(), imaging_factory);

                if (stop_token.stop_requested())
                    return;

                wil::com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
                THROW_IF_FAILED(decoder->GetFrame(0, &bitmap_frame_decode));

                const auto wic_colour_context = get_bitmap_source_colour_context(imaging_factory, bitmap_frame_decode);
                const auto pixel_format_info
                    = get_bitmap_source_pixel_format_info(imaging_factory, bitmap_frame_decode);

                WICPixelFormatNumericRepresentation pixel_format_numeric_representation{};
                THROW_IF_FAILED(pixel_format_info->GetNumericRepresentation(&pixel_format_numeric_representation));

                unsigned bpp{};
                THROW_IF_FAILED(pixel_format_info->GetBitsPerPixel(&bpp));

                is_float = pixel_format_numeric_representation == WICPixelFormatNumericRepresentationFloat;

                const auto target_pixel_format = [&] {
                    if (is_float)
                        return GUID_WICPixelFormat64bppPRGBAHalf;

                    if (bpp > 32)
                        return GUID_WICPixelFormat64bppPRGBA;

                    return GUID_WICPixelFormat32bppPRGBA;
                }();

                wil::com_ptr<IWICBitmapSource> wic_bitmap;
                THROW_IF_FAILED(WICConvertBitmapSource(target_pixel_format, bitmap_frame_decode.get(), &wic_bitmap));

                if (stop_token.stop_requested())
                    return;

                uint32_t width{};
                uint32_t height{};

                THROW_IF_FAILED(wic_bitmap->GetSize(&width, &height));

                const auto max_bitmap_size = d2d_render_target->GetMaximumBitmapSize();
                const auto needs_rescaling = width > max_bitmap_size || height > max_bitmap_size;

                if (needs_rescaling) {
                    const auto scaling_factor
                        = gsl::narrow_cast<float>(max_bitmap_size) / gsl::narrow_cast<float>(std::max(width, height));
                    const auto new_width = std::min(max_bitmap_size,
                        gsl::narrow_cast<uint32_t>(gsl::narrow_cast<float>(width) * scaling_factor + .5f));
                    const auto new_height = std::min(max_bitmap_size,
                        gsl::narrow_cast<uint32_t>(gsl::narrow_cast<float>(height) * scaling_factor + .5f));

                    const auto message = fmt::format("Artwork panel – image with size {0}×{1} exceeds maximum "
                                                     "size of {2}×{2}, image will be pre-scaled to {3}×{4}",
                        width, height, max_bitmap_size, new_width, new_height);
                    console::print(message.c_str());

                    wic_bitmap = wic::resize_bitmap_source(wic_bitmap, new_width, new_height, imaging_factory);

                    if (stop_token.stop_requested())
                        return;
                }

                HRESULT hr{};

                // Not implemented on Wine
                if (wic_colour_context) {
                    LOG_IF_FAILED(d2d_render_target->CreateColorContextFromWicColorContext(
                        wic_colour_context.get(), &d2d_colour_context));
                } else {
                    LOG_IF_FAILED(d2d_render_target->CreateColorContext(
                        is_float ? D2D1_COLOR_SPACE_SCRGB : D2D1_COLOR_SPACE_SRGB, nullptr, 0, &d2d_colour_context));
                }

                hr = d2d_render_target->CreateBitmapFromWicBitmap(wic_bitmap.get(), nullptr, &d2d_bitmap);

                if (hr != E_NOTIMPL || needs_rescaling)
                    THROW_IF_FAILED(hr);

                if (stop_token.stop_requested())
                    return;

                if (hr == E_NOTIMPL) {
                    // For lossy images, the Microsoft JXL codec does not support some interface or
                    // method that D2D expects (but making a copy of the bitmap and trying again works fine).
                    // (ID2D1DeviceContext2::CreateImageSourceFromWic() doesn't have this problem.)
                    wil::com_ptr<IWICBitmap> wic_bitmap_copy;
                    THROW_IF_FAILED(imaging_factory->CreateBitmapFromSource(
                        wic_bitmap.get(), WICBitmapCacheOnDemand, &wic_bitmap_copy));

                    THROW_IF_FAILED(
                        d2d_render_target->CreateBitmapFromWicBitmap(wic_bitmap_copy.get(), nullptr, &d2d_bitmap));
                }

                if (stop_token.stop_requested())
                    return;
            } catch (const std::exception& ex) {
                console::print("Artwork panel – loading image failed: ", ex.what());
            }

            fb2k::inMainThread(
                [this, d2d_bitmap{std::move(d2d_bitmap)}, d2d_colour_context{std::move(d2d_colour_context)}, is_float,
                    stop_token{std::move(stop_token)}, task{std::move(task)}]() {
                    const auto locked_task = task.lock();

                    if (stop_token.stop_requested()) {
                        if (locked_task)
                            std::erase(m_aborting_tasks, locked_task);
                        return;
                    }

                    assert(m_current_task == locked_task);
                    m_current_task.reset();
                    m_decoded_image = std::move(d2d_bitmap);
                    m_colour_context = std::move(d2d_colour_context);
                    m_is_float = is_float;

                    if (locked_task)
                        locked_task->on_complete();
                });
        });
}

} // namespace cui::artwork_panel
