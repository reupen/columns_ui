#include "pch.h"

#include "artwork_decoder.h"

namespace cui::artwork_panel {

void ArtworkDecoder::decode(
    wil::com_ptr<ID2D1HwndRenderTarget> d2d_render_target, album_art_data_ptr data, std::function<void()> on_complete)
{
    reset();
    m_current_task = std::make_shared<ArtworkDecoderTask>(std::move(on_complete));

    m_current_task->future = std::async(std::launch::async,
        [this, data, d2d_render_target{std::move(d2d_render_target)},
            stop_token{m_current_task->stop_source.get_token()}, task{std::weak_ptr{m_current_task}}]() noexcept {
            TRACK_CALL_TEXT("cui::artwork_panel::ArtworkDecoder::async_task");

            wil::com_ptr_t<ID2D1Bitmap> d2d_bitmap;

            try {
                if (stop_token.stop_requested())
                    return;

                auto _ = wil::CoInitializeEx(COINIT_MULTITHREADED);
                const auto imaging_factory = wic::create_factory();
                const auto decoder = wic::create_decoder_from_data(data->get_ptr(), data->get_size(), imaging_factory);

                if (stop_token.stop_requested())
                    return;

                auto wic_bitmap = wic::get_image_frame(decoder, GUID_WICPixelFormat32bppPBGRA);

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

                    const auto message = fmt::format(u8"Artwork panel – image with size {0}×{1} exceeds maximum "
                                                     u8"size of {2}×{2}, image will be pre-scaled to {3}×{4}",
                        width, height, max_bitmap_size, new_width, new_height);
                    console::print(reinterpret_cast<const char*>(message.c_str()));

                    wic_bitmap = wic::resize_bitmap_source(wic_bitmap, new_width, new_height, imaging_factory);

                    if (stop_token.stop_requested())
                        return;
                }

                const auto hr = d2d_render_target->CreateBitmapFromWicBitmap(wic_bitmap.get(), nullptr, &d2d_bitmap);

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
                console::print(u8"Artwork panel – loading image failed: "_pcc, ex.what());
                return;
            }

            fb2k::inMainThread(
                [this, d2d_bitmap{std::move(d2d_bitmap)}, stop_token{std::move(stop_token)}, task{std::move(task)}]() {
                    const auto locked_task = task.lock();

                    if (stop_token.stop_requested()) {
                        if (locked_task)
                            std::erase(m_aborting_tasks, locked_task);
                        return;
                    }

                    assert(m_current_task == locked_task);
                    m_current_task.reset();
                    m_decoded_image = std::move(d2d_bitmap);

                    if (locked_task)
                        locked_task->on_complete();
                });
        });
}

} // namespace cui::artwork_panel
