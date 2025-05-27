#include "pch.h"

#include "artwork_resizer.h"

namespace cui::artwork_panel {
namespace {
std::tuple<int, int> calculate_scaled_size(
    int bitmap_width, int bitmap_height, int client_width, int client_height, bool preserve_aspect_ratio)
{
    int scaled_width = client_width;
    int scaled_height = client_height;

    const double source_aspect_ratio = gsl::narrow_cast<double>(bitmap_width) / gsl::narrow_cast<double>(bitmap_height);
    const double client_aspect_ratio = gsl::narrow_cast<double>(client_width) / gsl::narrow_cast<double>(client_height);

    if (preserve_aspect_ratio) {
        if (client_aspect_ratio < source_aspect_ratio)
            scaled_height
                = gsl::narrow_cast<unsigned>(floor(gsl::narrow_cast<double>(client_width) / source_aspect_ratio));
        else if (client_aspect_ratio > source_aspect_ratio)
            scaled_width
                = gsl::narrow_cast<unsigned>(floor(gsl::narrow_cast<double>(client_height) * source_aspect_ratio));
    }

    if (((client_height - scaled_height) % 2) != 0)
        ++scaled_height;

    if (((client_width - scaled_width) % 2) != 0)
        ++scaled_width;

    return {scaled_width, scaled_height};
}

wil::shared_hbitmap draw_image(Gdiplus::Bitmap& bitmap, const int client_width, const int client_height,
    const int scaled_width, const int scaled_height, const COLORREF background_colour, bool low_quality = false)
{
    if (bitmap.GetWidth() < 2 || bitmap.GetHeight() < 2)
        return nullptr;

    const auto dc = wil::GetDC(nullptr);
    const wil::unique_hdc memory_dc(CreateCompatibleDC(dc.get()));

    wil::unique_hbitmap scaled_bitmap(CreateCompatibleBitmap(dc.get(), client_width, client_height));

    auto _ = wil::SelectObject(memory_dc.get(), scaled_bitmap.get());

    Gdiplus::Graphics graphics(memory_dc.get());

    Gdiplus::SolidBrush brush(
        Gdiplus::Color(GetRValue(background_colour), GetGValue(background_colour), GetBValue(background_colour)));
    graphics.FillRectangle(&brush, 0, 0, client_width, client_height);

    const auto draw_x = (client_width - scaled_width) / 2;
    const auto draw_y = (client_height - scaled_height) / 2;

    if (bitmap.GetWidth() == scaled_width && bitmap.GetHeight() == scaled_height) {
        graphics.DrawImage(&bitmap, draw_x, draw_y);
    } else {
        graphics.SetPixelOffsetMode(
            low_quality ? Gdiplus::PixelOffsetModeHighSpeed : Gdiplus::PixelOffsetModeHighQuality);
        graphics.SetInterpolationMode(
            low_quality ? Gdiplus::InterpolationModeLowQuality : Gdiplus::InterpolationModeHighQualityBicubic);
        Gdiplus::Rect dest_rect(draw_x, draw_y, scaled_width, scaled_height);
        graphics.SetClip(dest_rect);
        Gdiplus::ImageAttributes image_attributes;
        image_attributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);

        graphics.DrawImage(
            &bitmap, dest_rect, 0, 0, bitmap.GetWidth(), bitmap.GetHeight(), Gdiplus::UnitPixel, &image_attributes);
    }

    return scaled_bitmap;
}

} // namespace

wil::shared_hbitmap ArtworkResizer::s_scale_image(const wil::com_ptr<IWICBitmapSource>& bitmap, int width, int height,
    bool preserve_aspect_ratio, COLORREF background_colour, const std::stop_token& stop_token, bool low_quality)
{
    uint32_t bitmap_unsigned_width{};
    uint32_t bitmap_unsigned_height{};
    THROW_IF_FAILED(bitmap->GetSize(&bitmap_unsigned_width, &bitmap_unsigned_height));

    const auto bitmap_width = gsl::narrow<int>(bitmap_unsigned_width);
    const auto bitmap_height = gsl::narrow<int>(bitmap_unsigned_height);

    auto [scaled_width, scaled_height]
        = calculate_scaled_size(bitmap_width, bitmap_height, width, height, preserve_aspect_ratio);

    const auto imaging_factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

    wil::com_ptr<IWICBitmapScaler> bitmap_scaler;
    THROW_IF_FAILED(imaging_factory->CreateBitmapScaler(&bitmap_scaler));

    try {
        THROW_IF_FAILED(bitmap_scaler->Initialize(bitmap.get(), gsl::narrow<unsigned>(scaled_width),
            gsl::narrow<unsigned>(scaled_height),
            low_quality ? WICBitmapInterpolationModeFant : WICBitmapInterpolationModeHighQualityCubic));
    } catch (const wil::ResultException&) {
#if _DEBUG
        LOG_CAUGHT_EXCEPTION();
#endif
        bitmap_scaler.reset();
    }

    if (stop_token.stop_requested())
        return nullptr;

    wic::BitmapData decoded_data;

    // The WIC JPEG decoder really doesn't like multiple threads decoding the image at once.
    // So that is prevented using a lock.
    {
        std::scoped_lock _(s_wic_mutex);

        if (stop_token.stop_requested())
            return nullptr;

        decoded_data = wic::decode_bitmap_source(bitmap_scaler ? bitmap_scaler : bitmap);
    }

    if (stop_token.stop_requested())
        return nullptr;

    auto gdip_bitmap = gdip::create_bitmap_from_wic_data(decoded_data);

    if (stop_token.stop_requested())
        return nullptr;

    auto ret
        = draw_image(*gdip_bitmap.get(), width, height, scaled_width, scaled_height, background_colour, low_quality);

    return ret;
}

void ArtworkResizer::resize(
    wil::com_ptr<IWICBitmapSource> bitmap, ArtworkResizerOptions opts, std::function<void()> on_complete)
{
    abort();
    m_current_task = std::make_shared<ArtworkResizerTask>(std::move(on_complete));

    m_current_task->future = std::async(std::launch::async,
        [this, bitmap, opts, stop_token{m_current_task->stop_source.get_token()},
            task{std::weak_ptr{m_current_task}}]() noexcept {
            TRACK_CALL_TEXT("cui::artwork_panel::ArtworkResizer::async_task");

            if (stop_token.stop_requested())
                return;

            wil::shared_hbitmap scaled_bitmap;

            try {
                auto _ = wil::CoInitializeEx(COINIT_MULTITHREADED);
                scaled_bitmap = s_scale_image(
                    bitmap, opts.width, opts.height, opts.preserve_aspect_ratio, opts.background_colour, stop_token);
            }
            CATCH_LOG()

            if (stop_token.stop_requested())
                return;

            fb2k::inMainThread([this, scaled_bitmap{std::move(scaled_bitmap)}, stop_token{std::move(stop_token)},
                                   task{std::move(task)}]() mutable {
                const auto locked_task = task.lock();

                if (stop_token.stop_requested()) {
                    if (locked_task)
                        std::erase(m_aborting_tasks, locked_task);
                    return;
                }

                assert(m_current_task == locked_task);
                m_current_task.reset();
                m_scaled_bitmap = std::move(scaled_bitmap);

                if (locked_task)
                    locked_task->on_complete();
            });
        });
}

} // namespace cui::artwork_panel
