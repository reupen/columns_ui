#include "pch.h"

#include "vis_spectrum_renderer.h"

namespace cui::toolbars::spectrum_analyser {

namespace {

class FrameTimeAverager {
public:
    void add_frame(double frame_time)
    {
        m_sum -= m_frame_times[m_index];
        m_frame_times[m_index] = frame_time;
        m_sum += frame_time;

        m_index = (m_index + 1) % num_frames;

        if (m_count < num_frames)
            ++m_count;
    }

    double get_average() const
    {
        if (m_count == 0)
            return 0.;

        return m_sum / m_count;
    }

private:
    static constexpr size_t num_frames{25};

    std::array<double, num_frames> m_frame_times{};
    double m_sum{0.};
    size_t m_index{};
    size_t m_count{};
};

class FrameTimeMinimum {
public:
    void add_frame_time(double frame_time)
    {
        const double evicted = m_frame_times[m_index];
        m_frame_times[m_index] = frame_time;
        m_index = (m_index + 1) % num_frames;

        if (m_count < num_frames)
            ++m_count;

        if (frame_time <= m_minimum)
            m_minimum = frame_time;
        else if (evicted == m_minimum)
            update_minimum();
    }

    double get_minimum() const { return m_minimum; }

private:
    void update_minimum()
    {
        m_minimum = m_frame_times[0];

        for (size_t i{1}; i < m_count; ++i)
            m_minimum = std::min(m_minimum, m_frame_times[i]);
    }

    static constexpr size_t num_frames{10};

    std::array<double, num_frames> m_frame_times{};
    double m_minimum{std::numeric_limits<double>::max()};
    size_t m_index{};
    size_t m_count{};
};

constexpr auto get_fft_bins(size_t fft_size, size_t x_index, size_t x_count, unsigned sample_rate, bool is_log)
{
    float start_bin{};
    float end_bin{};

    if (is_log) {
        constexpr auto start_freq = 50.f;
        constexpr auto end_freq = 22'050.f;
        constexpr auto freq_ratio = end_freq / start_freq;
        const auto nyquist_freq = static_cast<float>(sample_rate) / 2.f;

        const auto normalised_x_start = static_cast<float>(x_index) / static_cast<float>(x_count);
        const auto normalised_x_end = static_cast<float>(x_index + 1) / static_cast<float>(x_count);

        const auto start = start_freq * std::pow(freq_ratio, normalised_x_start) / nyquist_freq;
        const auto end = start_freq * std::pow(freq_ratio, normalised_x_end) / nyquist_freq;

        start_bin = start * fft_size;
        end_bin = end * fft_size;
    } else {
        start_bin = static_cast<float>(fft_size) * (static_cast<float>(x_index) / x_count);
        end_bin = static_cast<float>(fft_size) * (static_cast<float>(x_index + 1) / x_count);
    }

    const size_t source_start
        = static_cast<size_t>(std::clamp(static_cast<int>(std::lround(start_bin)), 0, static_cast<int>(fft_size) - 1));
    size_t source_end
        = static_cast<size_t>(std::clamp(static_cast<int>(std::lround(end_bin)), 0, static_cast<int>(fft_size) - 1));

    if (source_end > source_start)
        --source_end;

    return std::make_tuple(source_start, source_end);
}

constexpr int calculate_y_position(audio_sample value, int y_count)
{
    constexpr auto min_db = -80.f;
    constexpr auto max_db = 0.f;

    const auto db = [&] {
        if (value <= 0)
            return min_db;

        return std::clamp(20.f * log10(gsl::narrow_cast<float>(value)), min_db, max_db);
    }();

    const auto percentage = (db - min_db) / (max_db - min_db);
    return std::clamp(static_cast<int>(std::lround(percentage * y_count)), 0, y_count);
}

} // namespace

void SpectrumAnalyserRenderer::configure(
    Mode mode, Scale horizontal_scale, COLORREF foreground_colour, COLORREF background_colour)
{
    assert(!m_render_thread);

    m_mode = mode;
    m_horizontal_scale = horizontal_scale;
    m_foreground_colour = foreground_colour;
    m_background_colour = background_colour;
}

void SpectrumAnalyserRenderer::start()
{
    if (m_render_thread && !m_render_thread->get_stop_source().stop_requested())
        return;

    m_render_thread.reset();
    m_stop_event.ResetEvent();
    m_dib_dc.reset();
    reset_dib();

    m_render_thread = std::jthread([this](std::stop_token stop_token) {
        mmh::set_thread_description(GetCurrentThread(), L"[Columns UI] Spectrum analyser renderer");

        GdiSetBatchLimit(1);

        uih::dcomp::DcompApi dcomp_api;
        wil::com_ptr<IDXGIFactory2> dxgi_factory;
        wil::com_ptr<IDXGIOutput> primary_output;

        FrameTimeAverager frame_time_averager;
        FrameTimeMinimum vblank_time_minimum;

        if (!dcomp_api.has_wait_for_composition_clock() && mmh::is_windows_8_or_newer()) {
            try {
                dxgi_factory = uih::dxgi::create_dxgi_factory();
                primary_output = uih::dxgi::get_primary_output(dxgi_factory.get());
            } catch (...) {
                LOG_CAUGHT_EXCEPTION();
                return;
            }
        }

        auto last_vblank_time_point = std::chrono::steady_clock::now() - 16ms;
        auto last_client_width = m_client_width.load(std::memory_order::relaxed);
        auto last_client_height = m_client_height.load(std::memory_order::relaxed);

        const auto dc = wil::GetDC(m_wnd);

        while (!stop_token.stop_requested()) {
            if (dxgi_factory && !dxgi_factory->IsCurrent()) {
                try {
                    dxgi_factory = uih::dxgi::create_dxgi_factory();
                    primary_output = uih::dxgi::get_primary_output(dxgi_factory.get());
                } catch (...) {
                    LOG_CAUGHT_EXCEPTION();
                    return;
                }
            }

            double vblank_time_ms{};

            auto wait_for_vblank = [&] {
                if (dcomp_api.has_wait_for_composition_clock()) {
                    const auto event = m_stop_event.get();
                    [[maybe_unused]] const auto dcomp_status
                        = dcomp_api.wait_for_composition_clock(m_stop_event ? 1 : 0, &event, 50);
#ifdef _DEBUG
                    LOG_IF_NTSTATUS_FAILED(dcomp_status);
#endif
                } else if (primary_output) {
                    [[maybe_unused]] const auto hr = primary_output->WaitForVBlank();
#ifdef _DEBUG
                    LOG_IF_FAILED(hr);
#endif
                } else {
                    [[maybe_unused]] const auto hr = DwmFlush();
#ifdef _DEBUG
                    LOG_IF_FAILED(hr);
#endif
                }

                const auto vblank_time_point = std::chrono::steady_clock::now();
                vblank_time_ms = (vblank_time_point - last_vblank_time_point) / 1.ms;

                if (vblank_time_ms > 1.)
                    vblank_time_minimum.add_frame_time(vblank_time_ms);

                last_vblank_time_point = vblank_time_point;
            };

            wait_for_vblank();

            // Possibly this could happen in scenarios like the display being off or RDP being in use
            const auto need_to_sleep = vblank_time_ms < 1.;

            if (stop_token.stop_requested())
                return;

            if (!need_to_sleep) {
                // Throttle if the average frame time is more than 50% of the minimum time between vblanks
                // (only likely for a massive window)
                const auto num_frames_to_skip = std::max(
                    0, static_cast<int>(frame_time_averager.get_average() * 2 / vblank_time_minimum.get_minimum()));

                for (int index{}; index < num_frames_to_skip; ++index)
                    wait_for_vblank();
            }

            const auto frame_start = std::chrono::steady_clock::now();

            double time{};
            m_stream->get_absolute_time(time);

            constexpr auto fft_size = 4096u;
            const auto is_active = m_stream->get_spectrum_absolute(m_chunk, time, fft_size);

            if (!is_active)
                m_chunk.reset();

            const auto client_width = m_client_width.load(std::memory_order::relaxed);
            const auto client_height = m_client_height.load(std::memory_order::relaxed);

            if (client_width != last_client_width || client_height != last_client_height) {
                last_client_width = client_width;
                last_client_height = client_height;
                reset_dib();
            }

            if (!m_dib)
                make_dib(client_width, client_height);

            if (is_active)
                render(dc.get());

            if (stop_token.stop_requested())
                return;

            const auto frame_time = (std::chrono::steady_clock::now() - frame_start) / 1.ms;
            frame_time_averager.add_frame(frame_time);

            // Will typically sleep for longer in practice (as the usual Windows timer resolution is 15.6ms)
            if (need_to_sleep)
                Sleep(15);
        }
    });
}

void SpectrumAnalyserRenderer::request_stop()
{
    if (m_render_thread) {
        m_render_thread->request_stop();
        m_stop_event.SetEvent();
    }
}

void SpectrumAnalyserRenderer::stop()
{
    m_render_thread.reset();
}

void SpectrumAnalyserRenderer::handle_paint(HDC dc, const RECT& update_rect, bool is_stopped)
{
    if (m_render_thread && !m_render_thread->get_stop_source().stop_requested())
        return;

    m_render_thread.reset();

    const auto client_width = m_client_width.load(std::memory_order::relaxed);
    const auto client_height = m_client_height.load(std::memory_order::relaxed);

    if (is_stopped) {
        SetDCBrushColor(dc, m_background_colour);
        auto _ = wil::SelectObject(dc, GetStockObject(DC_BRUSH));
        PatBlt(dc, update_rect.left, update_rect.top, wil::rect_width(update_rect), wil::rect_height(update_rect),
            PATCOPY);
        return;
    }

    if (client_width != m_dib_width || client_height != m_dib_height)
        reset_dib();

    if (!m_dib)
        make_dib(client_width, client_height);

    render(dc);
}

void SpectrumAnalyserRenderer::resize()
{
    RECT client_rect{};
    GetClientRect(m_wnd, &client_rect);

    m_client_width.store(std::max<int>(0, client_rect.right - client_rect.left), std::memory_order::relaxed);
    m_client_height.store(std::max<int>(0, client_rect.bottom - client_rect.top), std::memory_order::relaxed);
}

void SpectrumAnalyserRenderer::make_dib(int width, int height)
{
    assert(!(m_dib_bits || m_dib));

    if (width <= 0 || height <= 0)
        return;

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = -height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    m_dib.reset(CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &m_dib_bits, nullptr, 0));

    if (!m_dib || !m_dib_bits)
        return;

    m_dib_width = width;
    m_dib_height = height;
    m_stride = width * 4;

    paint_background();
}

void SpectrumAnalyserRenderer::reset_dib()
{
    m_dib_dc_select_object.reset();
    m_dib_dc.reset();
    m_dib.reset();
    m_dib_bits = nullptr;
    m_dib_width = 0;
    m_dib_height = 0;
    m_stride = 0;
}

void SpectrumAnalyserRenderer::render_dib(HDC dc)
{
    if (!m_dib_dc || !m_dib_dc_select_object) {
        if (!m_dib_dc)
            m_dib_dc.reset(CreateCompatibleDC(dc));

        m_dib_dc_select_object = wil::SelectObject(m_dib_dc.get(), m_dib.get());
    }

    BitBlt(dc, 0, 0, m_dib_width, m_dib_height, m_dib_dc.get(), 0, 0, SRCCOPY);
}

void SpectrumAnalyserRenderer::paint_background() const
{
    if (!m_dib_bits)
        return;

    fill_rect(0, 0, m_dib_width, m_dib_height, m_background_colour);
}

void SpectrumAnalyserRenderer::fill_rect(int left, int top, int right, int bottom, COLORREF colour) const
{
    assert(m_dib_bits);
    assert(left >= 0);
    assert(top >= 0);
    assert(right <= m_dib_width);
    assert(bottom <= m_dib_height);

    if (!m_dib_bits)
        return;

    if (left < 0)
        left = 0;

    if (top < 0)
        top = 0;

    if (right > m_dib_width)
        right = m_dib_width;

    if (bottom > m_dib_height)
        bottom = m_dib_height;

    if (right <= left || bottom <= top)
        return;

    const auto rgb_colour = RGB(GetBValue(colour), GetGValue(colour), GetRValue(colour));
    const auto data = static_cast<uint8_t*>(m_dib_bits);

    const int width = right - left;

    const auto stride_32 = m_stride / 4;
    auto* row = reinterpret_cast<uint32_t*>(data) + top * stride_32 + left;

    // Use fast paths for small widths (has a noticeable performance benefit,
    // particularly for non-bars mode)
    switch (width) {
    case 1:
        for (int y = top; y < bottom; ++y) {
            *row = rgb_colour;
            row += stride_32;
        }
        break;
    case 2:
        for (int y = top; y < bottom; ++y) {
            row[0] = rgb_colour;
            row[1] = rgb_colour;
            row += stride_32;
        }
        break;
    case 3:
        for (int y = top; y < bottom; ++y) {
            row[0] = rgb_colour;
            row[1] = rgb_colour;
            row[2] = rgb_colour;
            row += stride_32;
        }
        break;
    default:
        for (int y = top; y < bottom; ++y) {
            for (int x{}; x < width; ++x)
                row[x] = rgb_colour;

            row += stride_32;
        }
        break;
    }
}

void SpectrumAnalyserRenderer::render(HDC dc)
{
    if (!(m_dib && m_dib_bits))
        return;

    auto _ = wil::scope_exit([&] {
        if (!IsWindowVisible(m_wnd))
            return;

        render_dib(dc);
    });

    paint_background();

    if (m_chunk.is_empty())
        return;

    const auto channel_count = m_chunk.get_channels();

    // foo_input_sacd interferes with the visualisation API and can cause wrong channel counts
    if (channel_count != 1) {
#ifdef _DEBUG
        console::print("Columns UI – visualisation chunk with incorrect channel count received");
#endif
        return;
    }

    const auto data = m_chunk.get_data();
    const auto sample_count = m_chunk.get_sample_count();
    const auto sample_rate = m_chunk.get_sample_rate();

    if (m_mode == Mode::Bars) {
        const int num_bars = m_dib_width / m_bar_width;

        if (num_bars <= 0)
            return;

        for (const auto bar_index : std::ranges::views::iota(0, num_bars)) {
            const auto [start_bin, end_bin] = get_fft_bins(
                sample_count, bar_index, num_bars, sample_rate, m_horizontal_scale == Scale::Logarithmic);

            audio_sample value{};
            for (const auto bin_index : std::ranges::views::iota(start_bin, end_bin + 1)) {
                const auto bin_value = data[bin_index];
                value = std::max(value, bin_value);
            }

            const auto y_pos = calculate_y_position(value, (m_dib_height) / 2);

            const auto left = 1 + bar_index * m_bar_width;
            const auto right = left + m_bar_width - m_bar_gap;
            const auto bottom = m_dib_height - 1;
            const auto top = m_dib_height - y_pos * 2;

            for (int y = bottom; y > top; y -= 2)
                fill_rect(left, y - 1, right, y, m_foreground_colour);
        }
        return;
    }

    for (int x = 0; x < m_dib_width; x++) {
        const auto [start_bin, end_bin]
            = get_fft_bins(sample_count, x, m_dib_width, sample_rate, m_horizontal_scale == Scale::Logarithmic);

        audio_sample value{};

        for (const auto bin_index : std::ranges::views::iota(start_bin, end_bin + 1)) {
            const auto bin_value = data[bin_index];
            value = std::max(value, bin_value);
        }

        const auto y_pos = calculate_y_position(value, m_dib_height);

        const auto left = x;
        const auto right = x + 1;
        const auto bottom = m_dib_height;
        const auto top = m_dib_height - y_pos;

        fill_rect(left, top, right, bottom, m_foreground_colour);
    }
}

} // namespace cui::toolbars::spectrum_analyser
