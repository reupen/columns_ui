#pragma once
#include "wic.h"

namespace cui::artwork_panel {

class ArtworkDecoderTask {
public:
    using Ptr = std::shared_ptr<ArtworkDecoderTask>;

    std::jthread thread;
};

class ArtworkDecoder {
public:
    ~ArtworkDecoder() { shut_down(); }

    void decode(wil::com_ptr<ID2D1DeviceContext> d2d_render_target, bool use_advanced_colour, HMONITOR monitor,
        album_art_data_ptr data, std::function<void()> on_complete);

    void abort()
    {
        if (m_current_task) {
            m_current_task->thread.request_stop();
            m_aborting_tasks.emplace_back(std::move(m_current_task));
        }
    }

    void reset()
    {
        abort();
        m_decoded_image.reset();
        m_display_colour_context.reset();
        m_image_colour_context.reset();
        m_photo_orientation.reset();
        m_is_float.reset();
    }

    void shut_down()
    {
        reset();
        m_aborting_tasks.clear();
    }

    HRESULT get_error_result() const { return m_error_result.value_or(S_OK); }
    wil::com_ptr<ID2D1ColorContext> get_display_colour_context() { return m_display_colour_context; }
    wil::com_ptr<ID2D1Bitmap> get_image() { return m_decoded_image; }
    wil::com_ptr<ID2D1ColorContext> get_image_colour_context() { return m_image_colour_context; }
    bool has_image() const { return static_cast<bool>(m_decoded_image); }
    auto get_photo_orientation() const { return m_photo_orientation; }
    bool is_float() const { return m_is_float.value_or(false); }

    ArtworkDecoderTask::Ptr m_current_task;
    std::vector<ArtworkDecoderTask::Ptr> m_aborting_tasks;
    wil::com_ptr<ID2D1Bitmap> m_decoded_image;
    wil::com_ptr<ID2D1ColorContext> m_display_colour_context;
    wil::com_ptr<ID2D1ColorContext> m_image_colour_context;
    std::optional<wic::PhotoOrientation> m_photo_orientation;
    std::optional<bool> m_is_float{};
    std::optional<HRESULT> m_error_result{};
};

} // namespace cui::artwork_panel
