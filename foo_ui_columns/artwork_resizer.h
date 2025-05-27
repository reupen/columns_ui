#pragma once

namespace cui::artwork_panel {

class ArtworkResizerTask {
public:
    using Ptr = std::shared_ptr<ArtworkResizerTask>;

    std::function<void()> on_complete;
    std::stop_source stop_source;
    std::future<void> future;
};

class ArtworkResizer {
public:
    struct ArtworkResizerOptions {
        int width;
        int height;
        bool preserve_aspect_ratio;
        COLORREF background_colour;
    };

    static wil::shared_hbitmap s_scale_image(const wil::com_ptr<IWICBitmapSource>& bitmap, int width, int height,
        bool preserve_aspect_ratio, COLORREF background_colour, const std::stop_token& stop_token,
        bool low_quality = false);

    ~ArtworkResizer() { abort(); }

    void resize(wil::com_ptr<IWICBitmapSource> bitmap, ArtworkResizerOptions opts, std::function<void()> on_complete);

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
        m_scaled_bitmap.reset();
    }

    void shut_down()
    {
        reset();
        m_aborting_tasks.clear();
    }

    bool is_running() const { return static_cast<bool>(m_current_task); }
    bool has_bitmap() const { return static_cast<bool>(m_scaled_bitmap); }

    const wil::shared_hbitmap& get_bitmap() { return m_scaled_bitmap; }

    inline static std::mutex s_wic_mutex;
    ArtworkResizerTask::Ptr m_current_task;
    std::vector<ArtworkResizerTask::Ptr> m_aborting_tasks;
    wil::shared_hbitmap m_scaled_bitmap;
};

} // namespace cui::artwork_panel
