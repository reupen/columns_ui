#include "pch.h"
#include "vis_spectrum.h"

#include "config_appearance.h"
#include "dark_mode_dialog.h"
#include "main_window.h"

namespace cui::toolbars::spectrum_analyser {

namespace {

enum {
    MODE_STANDARD,
    MODE_BARS,
};

enum {
    scale_linear,
    scale_logarithmic,
};

const dark::DialogDarkModeConfig dark_mode_config{
    .button_ids = {IDOK, IDCANCEL}, .checkbox_ids = {IDC_BARS}, .combo_box_ids = {IDC_FRAME_COMBO, IDC_SCALE}};

// Legacy settings are no longer used (the values are preserved in case Columns UI is downgraded)
cfg_int cfg_legacy_vertical_scale(
    {0x3323c764, 0x875a, 0x4464, {0xac, 0x8e, 0xbb, 0x13, 0xe, 0x21, 0x5a, 0x4c}}, scale_logarithmic);

cfg_int cfg_legacy_spectrum_analyser_background_colour(
    GUID{0x2bb960d2, 0xb1a8, 0x5741, {0x55, 0xb6, 0x13, 0x3f, 0xb1, 0x80, 0x37, 0x88}}, GetSysColor(COLOR_WINDOW));

cfg_int cfg_legacy_spectrum_analyser_foreground_colour(
    GUID{0x421d3d3f, 0x5289, 0xb1e4, {0x9b, 0x91, 0xab, 0x51, 0xd3, 0xad, 0xbc, 0x4d}}, GetSysColor(COLOR_WINDOWTEXT));

cfg_bool has_migrated_spectrum_analyser_colours(
    {0x2ce47e0, 0xd964, 0x4f16, {0x83, 0x57, 0xd1, 0x1f, 0xb4, 0x43, 0xf2, 0x58}}, false);

void migrate_spectrum_analyser_colours(COLORREF foreground, COLORREF background)
{
    if (has_migrated_spectrum_analyser_colours)
        return;

    has_migrated_spectrum_analyser_colours = true;

    if (main_window::config_get_is_first_run())
        return;

    if (foreground == GetSysColor(COLOR_WINDOWTEXT) && background == GetSysColor(COLOR_WINDOW))
        return;

    auto set_entry_colours = [foreground, background](const colours::Entry::Ptr& entry) {
        entry->colour_set.colour_scheme = colours::ColourSchemeCustom;
        entry->colour_set.background = background;
        entry->colour_set.text = foreground;
    };

    const auto light_entry = g_colour_manager_data.get_entry(colour_client_id, false);
    set_entry_colours(light_entry);

    const auto dark_entry = g_colour_manager_data.get_entry(colour_client_id, true);
    set_entry_colours(dark_entry);
}

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

constexpr GUID scale_config_id = {0xdfa4e08c, 0x325f, 0x4b32, {0x91, 0xeb, 0xcd, 0x9f, 0xd5, 0xd0, 0xad, 0x14}};

cfg_int cfg_vis_edge(GUID{0x57cd2544, 0xd765, 0xef88, {0x30, 0xce, 0xd9, 0x9b, 0x47, 0xe4, 0x09, 0x94}}, 0);
cfg_uint cfg_vis_mode(GUID{0x3341d306, 0xf8b6, 0x6c60, {0xbd, 0x7e, 0xe4, 0xc5, 0xab, 0x51, 0xf3, 0xdd}}, MODE_BARS);
cfg_uint cfg_scale(scale_config_id, scale_logarithmic);

class SpectrumAnalyserVisualisation
    : public uie::container_uie_window_v3
    , public play_callback {
public:
    static void s_update_colours();
    static void s_refresh_all(bool include_inactive = false);

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    uie::container_window_v3_config get_window_config() override
    {
        uie::container_window_v3_config config(L"{ED4F644F-26AB-4aa0-809D-0D8F25352C5F}", false);

        if (m_frame == 1)
            config.extended_window_styles |= WS_EX_CLIENTEDGE;
        else if (m_frame == 2)
            config.extended_window_styles |= WS_EX_STATICEDGE;

        return config;
    }

    static constexpr GUID extension_guid
        = {0xd947777c, 0x94c7, 0x409a, {0xb0, 0x2c, 0x9b, 0xe, 0xb9, 0xe3, 0x74, 0xfa}};

    const GUID& get_extension_guid() const override { return extension_guid; }

    void get_name(pfc::string_base& out) const override { out.set_string("Spectrum analyser"); }
    void get_category(pfc::string_base& out) const override { out = "Visualisations"; }
    unsigned get_type() const override { return ui_extension::type_toolbar | ui_extension::type_panel; }

    void update_colours();

    void paint_background() const;

    void clear() { refresh(nullptr); }

    bool have_config_popup() const override { return true; }

    bool show_config_popup(HWND wnd_parent) override;

    static void s_register_stream(SpectrumAnalyserVisualisation* instance)
    {
        if (!s_active_instances.have_item(instance)) {
            if (s_active_instances.add_item(instance) == 0) {
                s_create_timer();
            }
        }
    }

    static void s_deregister_stream(SpectrumAnalyserVisualisation* instance, bool b_paused = false)
    {
        s_active_instances.remove_item(instance);

        if (s_active_instances.size() == 0) {
            s_destroy_timer();
        }

        if (!b_paused) {
            if (instance->m_is_active)
                instance->clear();
        }
    }

    friend class SpectrumAnalyserConfigData;

private:
    static void s_create_timer();
    static void s_destroy_timer();
    static void CALLBACK s_on_timer(HWND wnd, UINT msg, UINT_PTR id_event, DWORD time) noexcept;

    void make_dib();
    void reset_dib();
    void render_dib(HDC dc);

    void fill_dib_rect(int left, int top, int right, int bottom, COLORREF colour) const;
    void set_frame_style(unsigned value);

    void refresh();
    void refresh(const audio_chunk* chunk);

    void on_playback_starting(playback_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override
    {
        if (!m_playback_api->is_paused())
            s_register_stream(this);
    }
    void on_playback_stop(playback_control::t_stop_reason p_reason) noexcept override
    {
        const auto should_clear = p_reason != playback_control::stop_reason_shutting_down;
        s_deregister_stream(this, !should_clear);
    }
    void on_playback_seek(double p_time) override {}
    void on_playback_pause(bool p_state) noexcept override
    {
        if (p_state)
            s_deregister_stream(this, true);
        else
            s_register_stream(this);
    }
    void on_playback_edited(metadb_handle_ptr p_track) override {}
    void on_playback_dynamic_info(const file_info& p_info) override {}
    void on_playback_dynamic_info_track(const file_info& p_info) override {}
    void on_playback_time(double p_time) override {}
    void on_volume_change(float p_new_val) override {}

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override
    {
        p_hook.add_node(uie::menu_node_ptr(new uie::menu_node_configure(this)));
    }

    void set_config(stream_reader* reader, size_t p_size, abort_callback& p_abort) override;
    void get_config(stream_writer* writer, abort_callback& p_abort) const override;

    inline static UINT_PTR s_timer_id{};
    inline static audio_chunk_impl s_chunk;
    inline static visualisation_stream_v2::ptr s_stream;
    inline static pfc::ptr_list_t<SpectrumAnalyserVisualisation> s_instances;
    inline static pfc::ptr_list_t<SpectrumAnalyserVisualisation> s_active_instances;

    playback_control::ptr m_playback_api;
    bool m_is_active{};
    unsigned m_mode{cfg_vis_mode};
    short m_bar_width{3};
    short m_bar_gap{1};
    uint32_t m_scale{cfg_scale};
    uint32_t m_vertical_scale{scale_logarithmic};
    int m_frame{cfg_vis_edge};
    RECT m_client_rect{};
    COLORREF m_foreground_colour{};
    COLORREF m_background_colour{};
    wil::unique_hdc_window m_dc;
    wil::unique_hdc m_dib_dc;
    wil::unique_select_object m_dib_dc_select_object;
    wil::unique_hbitmap m_dib;
    void* m_dib_bits{};
    int m_stride{};
    int m_width{};
    int m_height{};
};

void SpectrumAnalyserVisualisation::update_colours()
{
    colours::helper colours(colour_client_id);
    m_foreground_colour = colours.get_colour(colours::colour_text);
    m_background_colour = colours.get_colour(colours::colour_background);
}

void SpectrumAnalyserVisualisation::paint_background() const
{
    if (!m_dib_bits)
        return;

    fill_dib_rect(0, 0, m_width, m_height, m_background_colour);
}

class SpectrumAnalyserConfigData {
public:
    unsigned mode{};
    uint32_t scale{};
    service_ptr_t<SpectrumAnalyserVisualisation> instance{};
    int frame{};
};

void SpectrumAnalyserVisualisation::s_create_timer()
{
    if (!s_timer_id) {
        s_timer_id = SetTimer(nullptr, NULL, 25, s_on_timer);
        s_refresh_all();
    }
}

void SpectrumAnalyserVisualisation::s_destroy_timer()
{
    if (s_timer_id) {
        KillTimer(nullptr, s_timer_id);
        s_timer_id = NULL;
    }
}

void CALLBACK SpectrumAnalyserVisualisation::s_on_timer(HWND wnd, UINT msg, UINT_PTR id_event, DWORD time) noexcept
{
    s_refresh_all();
}

void SpectrumAnalyserVisualisation::make_dib()
{
    assert(!(m_dib_bits || m_dib));

    const int width = std::max<int>(0, m_client_rect.right - m_client_rect.left);
    const int height = std::max<int>(0, m_client_rect.bottom - m_client_rect.top);

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

    m_width = width;
    m_height = height;
    m_stride = width * 4;

    paint_background();
}

void SpectrumAnalyserVisualisation::reset_dib()
{
    m_dib_dc_select_object.reset();
    m_dib_dc.reset();
    m_dib.reset();
    m_dib_bits = nullptr;
    m_width = 0;
    m_height = 0;
    m_stride = 0;
}

void SpectrumAnalyserVisualisation::render_dib(HDC dc)
{
    if (!m_dib_dc) {
        m_dib_dc.reset(CreateCompatibleDC(m_dc.get()));
        m_dib_dc_select_object = wil::SelectObject(m_dib_dc.get(), m_dib.get());
    }

    BitBlt(dc, 0, 0, m_width, m_height, m_dib_dc.get(), 0, 0, SRCCOPY);
}

void SpectrumAnalyserVisualisation::fill_dib_rect(int left, int top, int right, int bottom, COLORREF colour) const
{
    assert(m_dib_bits);
    assert(left >= 0);
    assert(top >= 0);
    assert(right <= m_width);
    assert(bottom <= m_height);

    if (!m_dib_bits)
        return;

    if (left < 0)
        left = 0;

    if (top < 0)
        top = 0;

    if (right > m_width)
        right = m_width;

    if (bottom > m_height)
        bottom = m_height;

    if (right <= left || bottom <= top)
        return;

    const auto rgb_colour = RGB(GetBValue(colour), GetGValue(colour), GetRValue(colour));
    const auto data = static_cast<uint8_t*>(m_dib_bits);

    const int width = right - left;

    for (int y = top; y < bottom; ++y) {
        auto* row = reinterpret_cast<uint32_t*>(data + m_stride * y) + left;
        std::fill_n(row, width, rgb_colour);
    }
}

void SpectrumAnalyserVisualisation::set_frame_style(unsigned value)
{
    if (m_frame == value)
        return;

    m_frame = value;

    if (!get_wnd())
        return;

    long flags = WS_EX_CONTROLPARENT;
    if (m_frame == 1)
        flags |= WS_EX_CLIENTEDGE;
    if (m_frame == 2)
        flags |= WS_EX_STATICEDGE;

    SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, flags);
    SetWindowPos(get_wnd(), nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
}

void SpectrumAnalyserVisualisation::refresh()
{
    refresh(m_playback_api->is_playing() && !s_chunk.is_empty() ? &s_chunk : nullptr);
}

void SpectrumAnalyserVisualisation::refresh(const audio_chunk* chunk)
{
    if (!m_dib)
        make_dib();

    if (!(m_dib && m_dib_bits))
        return;

    auto _ = wil::scope_exit([&] {
        if (!IsWindowVisible(get_wnd()))
            return;

        if (!m_dc)
            m_dc = wil::GetDC(get_wnd());

        render_dib(m_dc.get());
        // GdiFlush() is not called as even if the BitBlt() call gets batched, GetMessage() apparently
        // flushes the current batch anyway
    });

    paint_background();

    if (!chunk)
        return;

    const auto channel_count = chunk->get_channels();

    // foo_input_sacd interferes with the visualisation API and can cause wrong channel counts
    if (channel_count != 1) {
#ifdef _DEBUG
        console::print("Columns UI – visualisation chunk with incorrect channel count received");
#endif
        return;
    }

    const auto data = chunk->get_data();
    const auto sample_count = chunk->get_sample_count();
    const auto sample_rate = chunk->get_sample_rate();

    if (m_mode == MODE_BARS) {
        const int num_bars = m_width / m_bar_width;

        if (num_bars <= 0)
            return;

        for (const auto bar_index : std::ranges::views::iota(0, num_bars)) {
            const auto [start_bin, end_bin]
                = get_fft_bins(sample_count, bar_index, num_bars, sample_rate, m_scale == scale_logarithmic);

            audio_sample value{};
            for (const auto bin_index : std::ranges::views::iota(start_bin, end_bin + 1)) {
                const auto bin_value = data[bin_index];
                value = std::max(value, bin_value);
            }

            const auto y_pos = calculate_y_position(value, (m_height) / 2);

            const auto left = 1 + bar_index * m_bar_width;
            const auto right = left + m_bar_width - m_bar_gap;
            const auto bottom = m_height - 1;
            const auto top = m_height - y_pos * 2;

            for (int y = bottom; y > top; y -= 2)
                fill_dib_rect(left, y - 1, right, y, m_foreground_colour);
        }
        return;
    }

    for (int x = 0; x < m_width; x++) {
        const auto [start_bin, end_bin]
            = get_fft_bins(sample_count, x, m_width, sample_rate, m_scale == scale_logarithmic);

        audio_sample value{};

        for (const auto bin_index : std::ranges::views::iota(start_bin, end_bin + 1)) {
            const auto bin_value = data[bin_index];
            value = std::max(value, bin_value);
        }

        const auto y_pos = calculate_y_position(value, m_height);

        const auto left = x;
        const auto right = x + 1;
        const auto bottom = m_height;
        const auto top = m_height - y_pos;

        fill_dib_rect(left, top, right, bottom, m_foreground_colour);
    }
}

void SpectrumAnalyserVisualisation::set_config(stream_reader* reader, size_t p_size, abort_callback& p_abort)
{
    if (!p_size) {
        has_migrated_spectrum_analyser_colours = true;
        return;
    }

    reader->read_lendian_t(m_frame, p_abort);

    [[maybe_unused]] const auto nested_data_size = reader->read_lendian_t<uint32_t>(p_abort);

    if (nested_data_size == 0) {
        has_migrated_spectrum_analyser_colours = true;
        return;
    }

    const auto legacy_foreground = reader->read_lendian_t<COLORREF>(p_abort);
    const auto legacy_background = reader->read_lendian_t<COLORREF>(p_abort);

    migrate_spectrum_analyser_colours(legacy_foreground, legacy_background);

    reader->read_lendian_t(m_mode, p_abort);

    try {
        reader->read_lendian_t(m_scale, p_abort);
        reader->read_lendian_t(m_vertical_scale, p_abort);
    } catch (const exception_io_data_truncation&) {
    }
}

void SpectrumAnalyserVisualisation::get_config(stream_writer* writer, abort_callback& p_abort) const
{
    has_migrated_spectrum_analyser_colours = true;

    writer->write_lendian_t(m_frame, p_abort);

    colours::helper colours(colour_client_id);

    stream_writer_memblock data;
    data.write_lendian_t(colours.get_colour(colours::colour_text), p_abort);
    data.write_lendian_t(colours.get_colour(colours::colour_background), p_abort);
    data.write_lendian_t(m_mode, p_abort);
    data.write_lendian_t(m_scale, p_abort);
    data.write_lendian_t(m_vertical_scale, p_abort);

    writer->write_lendian_t(gsl::narrow<uint32_t>(data.m_data.get_size()), p_abort);
    writer->write(data.m_data.get_ptr(), data.m_data.get_size(), p_abort);
}

void SpectrumAnalyserVisualisation::s_refresh_all(bool include_inactive)
{
    bool is_active{};

    if (s_stream.is_valid()) {
        double time{};
        s_stream->get_absolute_time(time);

        constexpr auto fft_size = 4096u;
        is_active = s_stream->get_spectrum_absolute(s_chunk, time, fft_size);

        if (!is_active)
            s_chunk.reset();
    }

    const auto& instances = include_inactive ? s_instances : s_active_instances;

    for (const auto instance : instances) {
        instance->refresh(is_active ? &s_chunk : nullptr);
    }
}

void SpectrumAnalyserVisualisation::s_update_colours()
{
    for (const auto instance : s_instances)
        instance->update_colours();
}

LRESULT SpectrumAnalyserVisualisation::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        m_is_active = true;

        m_bar_width = 3_spx;
        m_bar_gap = 1_spx;

        m_playback_api = playback_control::get();

        update_colours();

        if (s_instances.add_item(this) == 0) {
            visualisation_manager::get()->create_stream(s_stream, visualisation_manager::KStreamFlagNewFFT);
            s_stream->set_channel_mode(visualisation_stream_v2::channel_mode_mono);
        }

        play_callback_manager::get()->register_callback(
            this, flag_on_playback_new_track | flag_on_playback_stop | flag_on_playback_pause, false);

        if (m_playback_api->is_playing())
            s_register_stream(this);

        break;
    }
    case WM_DESTROY: {
        m_is_active = false;
        s_instances.remove_item(this);
        play_callback_manager::get()->unregister_callback(this);

        if (m_playback_api->is_playing())
            s_deregister_stream(this);

        m_playback_api.reset();

        if (s_instances.size() == 0) {
            s_stream.release();
            s_chunk.reset();
        }

        m_dc.reset();
        reset_dib();
        break;
    }
    case WM_ERASEBKGND:
        return FALSE;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        const auto dc = wil::BeginPaint(wnd, &ps);

        if (!m_dib)
            make_dib();

        render_dib(dc.get());
        break;
    }
    case WM_GETMINMAXINFO: {
        const auto mmi = reinterpret_cast<LPMINMAXINFO>(lp);
        mmi->ptMinTrackSize.x = 50_spx;
        return 0;
    }
    case WM_WINDOWPOSCHANGED: {
        const auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);
        if ((lpwp->flags & SWP_NOSIZE) && !(lpwp->flags & SWP_FRAMECHANGED))
            break;

        GetClientRect(wnd, &m_client_rect);
        reset_dib();
        refresh();
        break;
    }
    }

    return DefWindowProc(wnd, msg, wp, lp);
}

INT_PTR CALLBACK SpectrumPopupProc(SpectrumAnalyserConfigData& state, HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        SendDlgItemMessage(wnd, IDC_BARS, BM_SETCHECK, state.mode == MODE_BARS, 0);

        const auto wnd_frame_combo = GetDlgItem(wnd, IDC_FRAME_COMBO);
        ComboBox_AddString(wnd_frame_combo, L"None");
        ComboBox_AddString(wnd_frame_combo, L"Sunken");
        ComboBox_AddString(wnd_frame_combo, L"Grey");
        ComboBox_SetCurSel(wnd_frame_combo, state.frame);

        const auto wnd_scale_combo = GetDlgItem(wnd, IDC_SCALE);
        ComboBox_AddString(wnd_scale_combo, L"Linear");
        ComboBox_AddString(wnd_scale_combo, L"Logarithmic");
        ComboBox_SetCurSel(wnd_scale_combo, state.scale);

        return TRUE;
    }
    case WM_COMMAND:
        switch (wp) {
        case IDCANCEL:
            EndDialog(wnd, 0);
            return TRUE;
        case IDC_BARS:
            state.mode = (Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED ? MODE_BARS : MODE_STANDARD);
            break;
        case IDC_FRAME_COMBO | (CBN_SELCHANGE << 16):
            state.frame = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            break;
        case IDC_SCALE | (CBN_SELCHANGE << 16):
            state.scale = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            break;
        case IDOK:
            EndDialog(wnd, 1);
            return TRUE;
        default:
            return FALSE;
        }
    default:
        return FALSE;
    }
}

bool SpectrumAnalyserVisualisation::show_config_popup(HWND wnd_parent)
{
    SpectrumAnalyserConfigData param{m_mode, m_scale, this, m_frame};

    if (!dark::modal_dialog_box(IDD_SPECTRUM_ANALYSER_OPTIONS, dark_mode_config, wnd_parent,
            [&param](auto&&... args) { return SpectrumPopupProc(param, std::forward<decltype(args)>(args)...); }))
        return false;

    m_mode = param.mode;
    cfg_vis_mode = param.mode;
    m_scale = param.scale;
    cfg_scale = param.scale;
    set_frame_style(param.frame);
    cfg_vis_edge = param.frame;

    if (m_is_active)
        refresh();

    return true;
}

ui_extension::window_factory<SpectrumAnalyserVisualisation> _factory;

class ColourClient : public colours::client {
    const GUID& get_client_guid() const override { return colour_client_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Spectrum analyser"; }
    uint32_t get_supported_colours() const override
    {
        return colours::colour_flag_text | colours::colour_flag_background;
    }
    uint32_t get_supported_bools() const override { return colours::bool_flag_dark_mode_enabled; }
    bool get_themes_supported() const override { return false; }
    void on_bool_changed(uint32_t mask) const override {}
    void on_colour_changed(uint32_t mask) const override
    {
        SpectrumAnalyserVisualisation::s_update_colours();
        SpectrumAnalyserVisualisation::s_refresh_all(true);
    }
};

ColourClient::factory<ColourClient> _colour_client;

} // namespace

} // namespace cui::toolbars::spectrum_analyser
