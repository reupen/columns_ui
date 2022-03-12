#include "pch.h"
#include "vis_spectrum.h"

#include "vis_gen_host.h"

namespace cui::toolbars::spectrum_analyser {

enum {
    MODE_STANDARD,
    MODE_BARS,
};

enum {
    scale_linear,
    scale_logarithmic,
};

constexpr GUID scale_config_id = {0xdfa4e08c, 0x325f, 0x4b32, {0x91, 0xeb, 0xcd, 0x9f, 0xd5, 0xd0, 0xad, 0x14}};
constexpr GUID vertical_scale_config_id = {0x3323c764, 0x875a, 0x4464, {0xac, 0x8e, 0xbb, 0x13, 0xe, 0x21, 0x5a, 0x4c}};

cfg_int cfg_vis_mode(GUID{0x3341d306, 0xf8b6, 0x6c60, {0xbd, 0x7e, 0xe4, 0xc5, 0xab, 0x51, 0xf3, 0xdd}}, MODE_BARS);
cfg_int cfg_scale(scale_config_id, scale_logarithmic);
cfg_int cfg_vertical_scale(vertical_scale_config_id, scale_logarithmic);

class SpectrumAnalyserVisualisation
    : public ui_extension::visualisation
    , public play_callback {
    ui_extension::visualisation_host_ptr p_host;

public:
    static void s_flush_brushes();
    static void s_refresh_all(bool include_inactive = false);

    bool b_active{false};
    unsigned mode;
    short m_bar_width{3};
    short m_bar_gap{1};

    uint32_t m_scale;
    uint32_t m_vertical_scale;

    SpectrumAnalyserVisualisation();

    ~SpectrumAnalyserVisualisation();

    static const GUID extension_guid;

    const GUID& get_extension_guid() const override { return extension_guid; }

    void get_name(pfc::string_base& out) const override;

    void set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort) override;
    void enable(const ui_extension::visualisation_host_ptr& p_host) override;
    void paint_background(HDC dc, const RECT* rc_client) override;
    void disable() override;

    void clear() { refresh(nullptr); }

    bool have_config_popup() const override { return true; }

    bool show_config_popup(HWND wnd_parent) override;

    void get_config(stream_writer* data, abort_callback& p_abort) const override;

    static void g_register_stream(SpectrumAnalyserVisualisation* p_ext)
    {
        if (!s_active_instances.have_item(p_ext)) {
            if (s_active_instances.add_item(p_ext) == 0) {
                s_create_timer();
            }
        }
    }

    static void g_deregister_stream(SpectrumAnalyserVisualisation* p_ext, bool b_paused = false)
    {
        s_active_instances.remove_item(p_ext);
        if (!s_active_instances.get_count()) {
            s_destroy_timer();
        }
        if (!b_paused) {
            if (p_ext->b_active)
                p_ext->clear();
        }
    }

    static bool g_is_stream_active(SpectrumAnalyserVisualisation* p_ext) { return s_active_instances.have_item(p_ext); }

    friend class SpectrumAnalyserConfigData;

private:
    inline static UINT_PTR g_timer_refcount{};
    inline static UINT_PTR g_timer{};
    inline static visualisation_stream::ptr g_stream;
    inline static pfc::ptr_list_t<SpectrumAnalyserVisualisation> s_instances;
    inline static pfc::ptr_list_t<SpectrumAnalyserVisualisation> s_active_instances;
    inline static wil::unique_hbrush s_foreground_brush;
    inline static wil::unique_hbrush s_background_brush;

    static void CALLBACK g_timer_proc(HWND wnd, UINT msg, UINT_PTR id_event, DWORD time);

    void refresh(const audio_chunk* p_chunk);
    static void s_create_timer()
    {
        if (!g_timer) {
            g_timer = SetTimer(nullptr, NULL, 25, g_timer_proc);
            s_refresh_all();
        }
    }

    static void s_destroy_timer()
    {
        if (g_timer) {
            KillTimer(nullptr, g_timer);
            g_timer = NULL;
        }
    }

    void FB2KAPI on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void FB2KAPI on_playback_new_track(metadb_handle_ptr p_track) override { g_register_stream(this); }
    void FB2KAPI on_playback_stop(play_control::t_stop_reason p_reason) override
    {
        const auto should_clear = p_reason != play_control::stop_reason_shutting_down;
        g_deregister_stream(this, !should_clear);

        if (should_clear)
            s_flush_brushes();
    }
    void FB2KAPI on_playback_seek(double p_time) override {}
    void FB2KAPI on_playback_pause(bool p_state) override
    {
        if (p_state)
            g_deregister_stream(this, true);
        else
            g_register_stream(this);
    }
    void FB2KAPI on_playback_edited(metadb_handle_ptr p_track) override {}
    void FB2KAPI on_playback_dynamic_info(const file_info& p_info) override {}
    void FB2KAPI on_playback_dynamic_info_track(const file_info& p_info) override {}
    void FB2KAPI on_playback_time(double p_time) override {}
    void FB2KAPI on_volume_change(float p_new_val) override {}
};

SpectrumAnalyserVisualisation::SpectrumAnalyserVisualisation()
    : mode(cfg_vis_mode)
    , m_scale(cfg_scale)
    , m_vertical_scale(cfg_vertical_scale)
{
}

SpectrumAnalyserVisualisation::~SpectrumAnalyserVisualisation() = default;

void SpectrumAnalyserVisualisation::s_flush_brushes()
{
    s_foreground_brush.reset();
    s_background_brush.reset();
}

void SpectrumAnalyserVisualisation::paint_background(HDC dc, const RECT* rc_client)
{
    colours::helper colours(colour_client_id);

    if (!s_background_brush)
        s_background_brush.reset(CreateSolidBrush(colours.get_colour(cui::colours::colour_background)));

    FillRect(dc, rc_client, s_background_brush.get());
}

void SpectrumAnalyserVisualisation::enable(const ui_extension::visualisation_host_ptr& p_vis_host)
{
    p_host = p_vis_host;
    b_active = true;

    m_bar_width = uih::scale_dpi_value(3);
    m_bar_gap = uih::scale_dpi_value(1);

    if (s_instances.add_item(this) == 0) {
        visualisation_manager::get()->create_stream(g_stream, visualisation_manager::KStreamFlagNewFFT);
        visualisation_stream_v2::ptr p_stream_v2;
        if (g_stream->service_query_t(p_stream_v2))
            p_stream_v2->set_channel_mode(visualisation_stream_v2::channel_mode_mono);
    }

    play_callback_manager::get()->register_callback(
        this, flag_on_playback_new_track | flag_on_playback_stop | flag_on_playback_pause, false);
    if (play_control::get()->is_playing())
        g_register_stream(this);
}

void SpectrumAnalyserVisualisation::disable()
{
    b_active = false;

    s_instances.remove_item(this);

    play_callback_manager::get()->unregister_callback(this);
    if (play_control::get()->is_playing())
        g_deregister_stream(this);

    if (!s_instances.get_count()) {
        g_stream.release();
        s_flush_brushes();
    }

    p_host.release();
}

class SpectrumAnalyserConfigData {
public:
    modal_dialog_scope m_scope;
    unsigned mode;
    uint32_t m_scale;
    uint32_t m_vertical_scale;
    SpectrumAnalyserVisualisation* ptr;
    unsigned frame;
    bool b_show_frame;
    SpectrumAnalyserConfigData(unsigned p_mode, uint32_t scale, uint32_t vertical_scale,
        SpectrumAnalyserVisualisation* p_spec, bool p_show_frame = false, unsigned p_frame = 0)
        : mode(p_mode)
        , m_scale(scale)
        , m_vertical_scale(vertical_scale)
        , ptr(p_spec)
        , frame(p_frame)
        , b_show_frame(p_show_frame)
    {
    }
    SpectrumAnalyserConfigData(const SpectrumAnalyserConfigData&) = delete;
    SpectrumAnalyserConfigData& operator=(const SpectrumAnalyserConfigData&) = delete;
    SpectrumAnalyserConfigData(SpectrumAnalyserConfigData&&) = delete;
    SpectrumAnalyserConfigData& operator=(SpectrumAnalyserConfigData&&) = delete;
    ~SpectrumAnalyserConfigData() {}
};

static INT_PTR CALLBACK SpectrumPopupProc(SpectrumAnalyserConfigData& state, HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        state.m_scope.initialize(FindOwningPopup(wnd));
        SendDlgItemMessage(wnd, IDC_BARS, BM_SETCHECK, state.ptr->mode == MODE_BARS, 0);
        HWND wnd_combo = GetDlgItem(wnd, IDC_FRAME_COMBO);
        EnableWindow(wnd_combo, state.b_show_frame);

        if (state.b_show_frame) {
            ComboBox_AddString(wnd_combo, _T("None"));
            ComboBox_AddString(wnd_combo, _T("Sunken"));
            ComboBox_AddString(wnd_combo, _T("Grey"));
            ComboBox_SetCurSel(wnd_combo, state.frame);
        }

        wnd_combo = GetDlgItem(wnd, IDC_SCALE);
        ComboBox_AddString(wnd_combo, _T("Linear"));
        ComboBox_AddString(wnd_combo, _T("Logarithmic"));
        ComboBox_SetCurSel(wnd_combo, state.m_scale);

        wnd_combo = GetDlgItem(wnd, IDC_VERTICAL_SCALE);
        ComboBox_AddString(wnd_combo, _T("Linear"));
        ComboBox_AddString(wnd_combo, _T("Logarithmic"));
        ComboBox_SetCurSel(wnd_combo, state.m_vertical_scale);
        return TRUE;
    }
    case WM_ERASEBKGND:
        SetWindowLongPtr(wnd, DWLP_MSGRESULT, TRUE);
        return TRUE;
    case WM_PAINT:
        uih::handle_modern_background_paint(wnd, GetDlgItem(wnd, IDOK));
        return TRUE;
    case WM_CTLCOLORSTATIC:
        return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_3DHIGHLIGHT));
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
            state.m_scale = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            break;
        case IDC_VERTICAL_SCALE | (CBN_SELCHANGE << 16):
            state.m_vertical_scale = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
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
    SpectrumAnalyserConfigData param(mode, m_scale, m_vertical_scale, this);
    const auto dialog_result = uih::modal_dialog_box(IDD_SPECTRUM_ANALYSER_OPTIONS, wnd_parent,
        [&param](auto&&... args) { return SpectrumPopupProc(param, std::forward<decltype(args)>(args)...); });

    if (dialog_result > 0) {
        mode = param.mode;
        cfg_vis_mode = param.mode;
        m_scale = param.m_scale;
        cfg_scale = param.m_scale;
        m_vertical_scale = param.m_vertical_scale;
        cfg_vertical_scale = param.m_vertical_scale;
        if (b_active) {
            clear();
        }
        return true;
    }
    return false;
}

void CALLBACK SpectrumAnalyserVisualisation::g_timer_proc(HWND wnd, UINT msg, UINT_PTR id_event, DWORD time)
{
    s_refresh_all();
}

void g_scale_value(
    size_t source_count, size_t index, size_t dest_count, size_t& source_start, size_t& source_end, bool b_log)
{
    double start;
    double end;
    if (b_log) {
        static constexpr auto power = 500;
        static const double exp0 = pow(power, 0);
        static const double exp1 = pow(power, 1);
        start = (double)source_count * (pow(power, double(index) / (double)dest_count) - exp0) / (exp1 - exp0);
        end = (double)source_count * (pow(power, double(index + 1) / (double)dest_count) - exp0) / (exp1 - exp0);
    } else {
        start = (double)source_count * ((double)(index) / (double)dest_count);
        end = (double)source_count * ((double)(index + 1) / (double)dest_count);
    }
    source_start = pfc::rint32(start);
    source_end = pfc::rint32(end);
    if (source_end > source_start)
        --source_end;
}

int g_scale_value_single(double val, int count, bool b_log)
{
    double val_trans;
    if (b_log) {
        constexpr auto minimum_value = -4;
        double log_val = val > 0 ? log10(val) : minimum_value;
        if (log_val < minimum_value)
            log_val = minimum_value;
        val_trans = count * (log_val + -minimum_value) / -minimum_value;
    } else {
        double start = (double)count * val;
        val_trans = start;
    }
    return std::clamp(pfc::rint32(val_trans), 0, count);
}

void SpectrumAnalyserVisualisation::refresh(const audio_chunk* p_chunk)
{
    ui_extension::visualisation_host::painter_ptr ps;
    p_host->create_painter(ps);

    HDC dc = ps->get_device_context();
    const RECT* rc_client = ps->get_area();
    {
        paint_background(dc, rc_client);

        if (g_is_stream_active(this) && p_chunk) {
            colours::helper colours(colour_client_id);

            if (!s_foreground_brush)
                s_foreground_brush.reset(CreateSolidBrush(colours.get_colour(colours::colour_text)));

            if (mode == MODE_BARS) {
                int totalbars = rc_client->right / m_bar_width;
                if (totalbars) {
                    const audio_sample* p_data = p_chunk->get_data();
                    size_t sample_count = p_chunk->get_sample_count();
                    size_t channel_count = p_chunk->get_channels();
                    for (int i = 0; i < totalbars; i++) {
                        double val = 0;
                        size_t starti;
                        size_t endi;
                        g_scale_value(sample_count, i, totalbars, starti, endi, m_scale == scale_logarithmic);
                        for (size_t j = starti; j <= endi; j++) {
                            if (j < sample_count) {
                                double sample_val = 0;
                                for (size_t k = 0; k < channel_count; k++)
                                    sample_val += p_data[j * channel_count + k];
                                sample_val *= 1.0 / channel_count;
                                val = std::max(val, sample_val);
                            }
                        }

                        RECT r;
                        r.left = 1 + i * m_bar_width;
                        r.right = r.left + m_bar_width - m_bar_gap;
                        r.bottom = rc_client->bottom ? rc_client->bottom - 1 : 0;
                        r.top = rc_client->bottom
                            - g_scale_value_single(
                                  val, (rc_client->bottom + 1) / 2, m_vertical_scale == scale_logarithmic)
                                * 2;
                        if (r.bottom > r.top)
                            FillRect(dc, &r, s_foreground_brush.get());
                    }
                    for (int j = rc_client->bottom; j > rc_client->top; j -= 2) {
                        RECT rc = {0, j - 1, rc_client->right, j};
                        FillRect(dc, &rc, s_background_brush.get());
                    }
                }
            } else {
                const audio_sample* p_data = p_chunk->get_data();
                size_t sample_count = p_chunk->get_sample_count();
                size_t channel_count = p_chunk->get_channels();
                for (int i = 0; i < rc_client->right; i++) {
                    double val = 0;
                    size_t starti;
                    size_t endi;
                    g_scale_value(sample_count, i, rc_client->right, starti, endi, m_scale == scale_logarithmic);
                    for (size_t j = starti; j <= endi; j++) {
                        if (j < sample_count) {
                            double sample_val = 0;
                            for (size_t k = 0; k < channel_count; k++)
                                sample_val += p_data[j * channel_count + k];
                            sample_val *= 1.0 / channel_count;
                            val = std::max(val, sample_val);
                        }
                    }
                    RECT r;
                    r.left = i;
                    r.right = i + 1;
                    r.bottom = rc_client->bottom;
                    r.top = rc_client->bottom
                        - g_scale_value_single(val, rc_client->bottom, m_vertical_scale == scale_logarithmic);
                    if (r.bottom > r.top)
                        FillRect(dc, &r, s_foreground_brush.get());
                }
            }
        }
    }
    ps.release();
}

void SpectrumAnalyserVisualisation::s_refresh_all(bool include_inactive)
{
    bool is_active{};
    audio_chunk_impl p_chunk;

    if (g_stream.is_valid()) {
        double time{};
        g_stream->get_absolute_time(time);

        constexpr auto fft_size = 4096u;
        is_active = g_stream->get_spectrum_absolute(p_chunk, time, fft_size);
    }

    const auto& instances = include_inactive ? s_instances : s_active_instances;

    for (const auto instance : instances) {
        instance->refresh(is_active ? &p_chunk : nullptr);
    }
}

void SpectrumAnalyserVisualisation::get_name(pfc::string_base& out) const
{
    out.set_string("Spectrum analyser");
}

void SpectrumAnalyserVisualisation::set_config(stream_reader* r, size_t p_size, abort_callback& p_abort)
{
    if (p_size) {
        r->read_lendian_t<COLORREF>(p_abort);
        r->read_lendian_t<COLORREF>(p_abort);
        r->read_lendian_t(mode, p_abort);
        try {
            r->read_lendian_t(m_scale, p_abort);
            r->read_lendian_t(m_vertical_scale, p_abort);
        } catch (const exception_io_data_truncation&) {
        }
    }
}

void SpectrumAnalyserVisualisation::get_config(stream_writer* data, abort_callback& p_abort) const
{
    cui::colours::helper colours(colour_client_id);

    data->write_lendian_t(colours.get_colour(colours::colour_text), p_abort);
    data->write_lendian_t(colours.get_colour(colours::colour_background), p_abort);
    data->write_lendian_t(mode, p_abort);
    data->write_lendian_t(m_scale, p_abort);
    data->write_lendian_t(m_vertical_scale, p_abort);
}

// {D947777C-94C7-409a-B02C-9B0EB9E374FA}
const GUID SpectrumAnalyserVisualisation::extension_guid
    = {0xd947777c, 0x94c7, 0x409a, {0xb0, 0x2c, 0x9b, 0xe, 0xb9, 0xe3, 0x74, 0xfa}};

ui_extension::visualisation_factory<SpectrumAnalyserVisualisation> blah;

class SpectrumAnalyserVisualisationPanel : public VisualisationPanel {
    const GUID& get_visualisation_guid() const override { return SpectrumAnalyserVisualisation::extension_guid; }
    const GUID& get_extension_guid() const override { return SpectrumAnalyserVisualisation::extension_guid; }
    void get_menu_items(ui_extension::menu_hook_t& p_hook) override
    {
        p_hook.add_node(uie::menu_node_ptr(new uie::menu_node_configure(this)));
    }
    void set_config(stream_reader* r, size_t p_size, abort_callback& p_abort) override
    {
        if (p_size) {
            uint32_t m_frame;
            r->read_lendian_t(m_frame, p_abort);
            {
                set_frame_style(m_frame);
                unsigned size = 0;
                r->read_lendian_t(size, p_abort);
                pfc::array_t<uint8_t> m_data;
                m_data.set_size(size);
                r->read(m_data.get_ptr(), size, p_abort);
                set_vis_data(m_data.get_ptr(), m_data.get_size());
            }
        }
    }
    void get_config(stream_writer* data, abort_callback& p_abort) const override
    {
        pfc::array_t<uint8_t> m_data;
        data->write_lendian_t(get_frame_style(), p_abort);
        get_vis_data(m_data);
        data->write_lendian_t(gsl::narrow<uint32_t>(m_data.get_size()), p_abort);
        data->write(m_data.get_ptr(), m_data.get_size(), p_abort);
    }
    bool have_config_popup() const override { return true; }
    bool show_config_popup(HWND wnd_parent) override
    {
        uie::visualisation_ptr p_vis;
        service_ptr_t<SpectrumAnalyserVisualisation> p_this;
        get_vis_ptr(p_vis);
        if (p_vis.is_valid())
            p_this = static_cast<SpectrumAnalyserVisualisation*>(p_vis.get_ptr());

        service_ptr_t<SpectrumAnalyserVisualisation> p_temp = p_this;
        if (!p_temp.is_valid())
            uie::visualization::create_by_guid(
                get_visualisation_guid(), reinterpret_cast<uie::visualisation_ptr&>(p_temp));

        pfc::array_t<uint8_t> m_data;
        if (!p_temp->b_active) {
            try {
                abort_callback_dummy p_abort;
                get_vis_data(m_data);
                p_temp->set_config_from_ptr(m_data.get_ptr(), m_data.get_size(), p_abort);
            } catch (const exception_io&) {
            }
        }

        SpectrumAnalyserConfigData param(
            p_temp->mode, p_temp->m_scale, p_temp->m_vertical_scale, p_temp.get_ptr(), true, get_frame_style());

        const auto dialog_result = uih::modal_dialog_box(IDD_SPECTRUM_ANALYSER_OPTIONS, wnd_parent,
            [&param](auto&&... args) { return SpectrumPopupProc(param, std::forward<decltype(args)>(args)...); });

        if (dialog_result > 0) {
            p_temp->mode = param.mode;
            cfg_vis_mode = param.mode;
            p_temp->m_scale = param.m_scale;
            cfg_scale = param.m_scale;
            p_temp->m_vertical_scale = param.m_vertical_scale;
            cfg_vertical_scale = param.m_vertical_scale;
            set_frame_style(param.frame);
            cfg_vis_edge = param.frame;

            if (p_temp->b_active) {
                p_temp->clear();
            } else {
                try {
                    abort_callback_dummy p_abort;
                    p_temp->get_config_to_array(m_data, p_abort, true);
                    set_vis_data(m_data.get_ptr(), m_data.get_size());
                } catch (pfc::exception&) {
                }
            }
            return true;
        }
        return false;
    }
};

ui_extension::window_factory<SpectrumAnalyserVisualisationPanel> blahg;

class ColourClient : public colours::client {
    const GUID& get_client_guid() const override { return colour_client_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Spectrum analyser"; }
    uint32_t get_supported_colours() const override
    {
        return colours::colour_flag_text | colours::colour_flag_background;
    }
    uint32_t get_supported_bools() const override { return colours::bool_flag_dark_mode_enabled; }
    bool get_themes_supported() const override { return false; }
    void on_bool_changed(uint32_t mask) const override{};
    void on_colour_changed(uint32_t mask) const override
    {
        SpectrumAnalyserVisualisation::s_flush_brushes();
        SpectrumAnalyserVisualisation::s_refresh_all(true);
    }
};

namespace {
ColourClient::factory<ColourClient> _colour_client;
}

} // namespace cui::toolbars::spectrum_analyser
