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

    void decode(
        wil::com_ptr<ID2D1DeviceContext> d2d_render_target, album_art_data_ptr data, std::function<void()> on_complete);

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
        m_color_context.reset();
        m_is_float.reset();
    }

    void shut_down()
    {
        reset();
        m_aborting_tasks.clear();
    }

    bool has_image() const { return static_cast<bool>(m_decoded_image); }

    wil::com_ptr<ID2D1Bitmap> get_image() { return m_decoded_image; }
    wil::com_ptr<ID2D1ColorContext> get_color_context() { return m_color_context; }
    bool is_float() const { return m_is_float.value_or(false); }

    ArtworkDecoderTask::Ptr m_current_task;
    std::vector<ArtworkDecoderTask::Ptr> m_aborting_tasks;
    wil::com_ptr<ID2D1Bitmap> m_decoded_image;
    wil::com_ptr<ID2D1ColorContext> m_color_context;
    std::optional<bool> m_is_float{};
};

} // namespace cui::artwork_panel
