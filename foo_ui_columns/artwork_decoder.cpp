#include "pch.h"

#include "artwork_decoder.h"

#include "wcs.h"
#include "wic.h"
#include "win32.h"

namespace cui::artwork_panel {

namespace {

auto get_bitmap_source_pixel_format_info(
    const wil::com_ptr<IWICImagingFactory>& imaging_factory, const wil::com_ptr<IWICBitmapSource>& bitmap_source)
{
    WICPixelFormatGUID source_pixel_format{};
    THROW_IF_FAILED(bitmap_source->GetPixelFormat(&source_pixel_format));

    wil::com_ptr<IWICComponentInfo> wic_component_info;
    THROW_IF_FAILED(imaging_factory->CreateComponentInfo(source_pixel_format, &wic_component_info));

    return wic_component_info.query<IWICPixelFormatInfo2>();
}

} // namespace

void ArtworkDecoder::decode(wil::com_ptr<ID2D1DeviceContext> d2d_render_target, bool use_advanced_colour,
    HMONITOR monitor, album_art_data_ptr data, std::function<void()> on_complete)
{
    reset();
    m_current_task = std::make_shared<ArtworkDecoderTask>();

    m_current_task->thread = std::jthread([this, data, d2d_render_target{std::move(d2d_render_target)}, monitor,
                                              on_complete{std::move(on_complete)}, task{std::weak_ptr{m_current_task}},
                                              use_advanced_colour](std::stop_token stop_token) noexcept {
        TRACK_CALL_TEXT("cui::artwork_panel::ArtworkDecoder::thread");
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
        (void)mmh::set_thread_description(GetCurrentThread(), L"[Columns UI] Artwork view decoder");

        wil::com_ptr<ID2D1Bitmap> d2d_bitmap;
        wil::com_ptr<ID2D1ColorContext> d2d_image_colour_context;
        wil::com_ptr<ID2D1ColorContext> d2d_display_colour_context;
        bool is_float{};
        std::optional<HRESULT> error_result;

        auto _ = wil::scope_exit([&] {
            fb2k::inMainThread(
                [this, d2d_bitmap{std::move(d2d_bitmap)},
                    d2d_display_colour_context{std::move(d2d_display_colour_context)},
                    d2d_image_colour_context{std::move(d2d_image_colour_context)}, error_result, is_float,
                    on_complete{std::move(on_complete)}, stop_token{std::move(stop_token)}, task{std::move(task)}]() {
                    const auto locked_task = task.lock();

                    if (stop_token.stop_requested()) {
                        if (locked_task)
                            std::erase(m_aborting_tasks, locked_task);
                        return;
                    }

                    assert(m_current_task == locked_task);
                    m_current_task.reset();
                    m_decoded_image = std::move(d2d_bitmap);
                    m_display_colour_context = std::move(d2d_display_colour_context);
                    m_image_colour_context = std::move(d2d_image_colour_context);
                    m_is_float = is_float;
                    m_error_result = error_result;

                    on_complete();
                });
        });

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

            const auto pixel_format_info = get_bitmap_source_pixel_format_info(imaging_factory, bitmap_frame_decode);

            WICPixelFormatNumericRepresentation pixel_format_numeric_representation{};
            THROW_IF_FAILED(pixel_format_info->GetNumericRepresentation(&pixel_format_numeric_representation));

            unsigned bpp{};
            THROW_IF_FAILED(pixel_format_info->GetBitsPerPixel(&bpp));

            const auto wic_colour_context = wic::get_bitmap_source_colour_context(imaging_factory, bitmap_frame_decode);
            is_float = pixel_format_numeric_representation == WICPixelFormatNumericRepresentationFloat;

            const auto target_pixel_format = [&] {
                if (is_float)
                    return use_advanced_colour ? GUID_WICPixelFormat64bppPRGBAHalf : GUID_WICPixelFormat32bppPRGBA;

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
                const auto new_width = std::min(
                    max_bitmap_size, gsl::narrow_cast<uint32_t>(gsl::narrow_cast<float>(width) * scaling_factor + .5f));
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
                    wic_colour_context.get(), &d2d_image_colour_context));
            } else {
                LOG_IF_FAILED(d2d_render_target->CreateColorContext(
                    is_float && use_advanced_colour ? D2D1_COLOR_SPACE_SCRGB : D2D1_COLOR_SPACE_SRGB, nullptr, 0,
                    &d2d_image_colour_context));
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

            if (monitor) {
                const auto display_device_key = win32::get_display_device_key(monitor);
                const auto display_profile_name = wcs::get_display_colour_profile_name(display_device_key.c_str());
                const auto profile = wcs::get_display_colour_profile(display_profile_name);

                if (!profile.empty())
                    LOG_IF_FAILED(d2d_render_target->CreateColorContext(D2D1_COLOR_SPACE_CUSTOM, profile.data(),
                        gsl::narrow<uint32_t>(profile.size()), &d2d_display_colour_context));

                if (!d2d_display_colour_context) {
                    LOG_IF_FAILED(d2d_render_target->CreateColorContext(
                        D2D1_COLOR_SPACE_SRGB, nullptr, 0, &d2d_display_colour_context));
                }
            }

        } catch (const std::exception&) {
            console::print("Artwork panel – loading image failed: ", mmh::get_caught_exception_message().c_str());

            d2d_bitmap.reset();
            d2d_display_colour_context.reset();
            d2d_image_colour_context.reset();
            is_float = false;
            error_result = wil::ResultFromCaughtException();
        }
    });
}

} // namespace cui::artwork_panel
