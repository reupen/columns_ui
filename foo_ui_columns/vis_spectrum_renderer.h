#pragma once

#include "vis_spectrum.h"

namespace cui::toolbars::spectrum_analyser {

class SpectrumAnalyserRenderer {
public:
    SpectrumAnalyserRenderer(HWND wnd) : m_wnd(wnd)
    {
        m_stop_event.create();

        resize();

        visualisation_manager::get()->create_stream(m_stream, visualisation_manager::KStreamFlagNewFFT);
        m_stream->set_channel_mode(visualisation_stream_v2::channel_mode_mono);
    }

    ~SpectrumAnalyserRenderer() { m_render_thread.reset(); }

    void configure(Mode mode, int bar_width, Scale horizontal_scale, uint32_t fft_size, float min_frequency,
        float max_frequency, bool smooth_values, COLORREF foreground_colour, COLORREF background_colour);
    void start();
    void request_stop();
    void stop();
    void handle_paint(HDC dc, const RECT& update_rect, bool is_stopped);
    void resize();

private:
    void make_dib(int width, int height);
    void reset_dib();
    void paint_background();
    void fill_rect(int left, int top, int right, int bottom, COLORREF colour) const;
    void render(HDC dc);
    void render_dib(HDC dc);

    HWND m_wnd{};
    audio_chunk_impl m_chunk{};

    int m_bar_width{2_spx};
    int m_bar_gap{1_spx};

    Mode m_mode{};
    Scale m_horizontal_scale{};
    uint32_t m_fft_size{default_fft_size};
    float m_min_frequency{default_min_frequency};
    float m_max_frequency{default_max_frequency};
    bool m_smooth_values{};
    COLORREF m_foreground_colour{};
    COLORREF m_background_colour{};

    wil::unique_event_nothrow m_stop_event;
    wil::unique_hdc m_dib_dc;
    wil::unique_select_object m_dib_dc_select_object;
    wil::unique_hbitmap m_dib;
    void* m_dib_bits{};
    int m_stride{};
    std::atomic<int> m_client_width{};
    std::atomic<int> m_client_height{};
    int m_dib_width{};
    int m_dib_height{};
    int m_dib_min_filled_y{};

    visualisation_stream_v2::ptr m_stream;
    std::optional<std::jthread> m_render_thread;
};

} // namespace cui::toolbars::spectrum_analyser
