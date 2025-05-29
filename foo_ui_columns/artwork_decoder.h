#pragma once

namespace cui::artwork_panel {

class ArtworkDecoderTask {
public:
    using Ptr = std::shared_ptr<ArtworkDecoderTask>;

    std::function<void()> on_complete;
    std::stop_source stop_source;
    std::future<void> future;
};

class ArtworkDecoder {
public:
    ~ArtworkDecoder() { abort(); }

    void decode(wil::com_ptr<ID2D1HwndRenderTarget> d2d_render_target, album_art_data_ptr data,
        std::function<void()> on_complete);

    void abort()
    {
        if (m_current_task) {
            m_current_task->stop_source.request_stop();
            m_aborting_tasks.emplace_back(std::move(m_current_task));
        }
    }

    void reset()
    {
        abort();
        m_decoded_image.reset();
    }

    void shut_down()
    {
        reset();
        m_aborting_tasks.clear();
    }

    bool has_image() const { return static_cast<bool>(m_decoded_image); }

    wil::com_ptr<ID2D1Bitmap> get_image() { return m_decoded_image; }

    ArtworkDecoderTask::Ptr m_current_task;
    std::vector<ArtworkDecoderTask::Ptr> m_aborting_tasks;
    wil::com_ptr<ID2D1Bitmap> m_decoded_image;
};

} // namespace cui::artwork_panel
