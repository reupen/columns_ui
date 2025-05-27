#include "pch.h"

#include "artwork_decoder.h"

namespace cui::artwork_panel {

void ArtworkDecoder::decode(album_art_data_ptr data, std::function<void()> on_complete)
{
    reset();
    m_current_task = std::make_shared<ArtworkDecoderTask>(std::move(on_complete));

    m_current_task->future = std::async(std::launch::async,
        [this, data, stop_token{m_current_task->stop_source.get_token()},
            task{std::weak_ptr{m_current_task}}]() noexcept {
            TRACK_CALL_TEXT("cui::artwork_panel::ArtworkDecoder::async_task");

            wil::com_ptr<IWICBitmapSource> bitmap;

            try {
                if (stop_token.stop_requested())
                    return;

                const auto decoder = wic::create_decoder_from_data(data->get_ptr(), data->get_size());

                bitmap = wic::get_image_frame(decoder);

                if (stop_token.stop_requested())
                    return;

            } catch (const std::exception& ex) {
                console::print(u8"Artwork panel – loading image failed: "_pcc, ex.what());
                return;
            }

            fb2k::inMainThread(
                [this, bitmap{std::move(bitmap)}, stop_token{std::move(stop_token)}, task{std::move(task)}]() {
                    const auto locked_task = task.lock();

                    if (stop_token.stop_requested()) {
                        if (locked_task)
                            std::erase(m_aborting_tasks, locked_task);
                        return;
                    }

                    assert(m_current_task == locked_task);
                    m_current_task.reset();
                    m_decoded_image = std::move(bitmap);

                    if (locked_task)
                        locked_task->on_complete();
                });
        });
}

} // namespace cui::artwork_panel
