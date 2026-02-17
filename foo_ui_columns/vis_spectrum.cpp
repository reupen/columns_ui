#include "pch.h"

#include "vis_spectrum.h"
#include "vis_spectrum_renderer.h"

#include "config_appearance.h"
#include "dark_mode_dialog.h"

namespace cui::toolbars::spectrum_analyser {

namespace {

const dark::DialogDarkModeConfig dark_mode_config{
    .button_ids = {IDOK, IDCANCEL}, .checkbox_ids = {IDC_BARS}, .combo_box_ids = {IDC_FRAME_COMBO, IDC_SCALE}};

// Legacy settings are no longer used (the values are preserved in case Columns UI is downgraded)
cfg_int cfg_legacy_vertical_scale(
    {0x3323c764, 0x875a, 0x4464, {0xac, 0x8e, 0xbb, 0x13, 0xe, 0x21, 0x5a, 0x4c}}, WI_EnumValue(Scale::Logarithmic));

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

constexpr GUID scale_config_id = {0xdfa4e08c, 0x325f, 0x4b32, {0x91, 0xeb, 0xcd, 0x9f, 0xd5, 0xd0, 0xad, 0x14}};

cfg_int cfg_vis_edge(GUID{0x57cd2544, 0xd765, 0xef88, {0x30, 0xce, 0xd9, 0x9b, 0x47, 0xe4, 0x09, 0x94}}, 0);
cfg_uint cfg_vis_mode(
    GUID{0x3341d306, 0xf8b6, 0x6c60, {0xbd, 0x7e, 0xe4, 0xc5, 0xab, 0x51, 0xf3, 0xdd}}, WI_EnumValue(Mode::Bars));
cfg_uint cfg_scale(scale_config_id, WI_EnumValue(Scale::Logarithmic));

class SpectrumAnalyserVisualisation
    : public uie::container_uie_window_v3
    , public play_callback {
public:
    static void s_update_colours();

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

    bool have_config_popup() const override { return true; }

    bool show_config_popup(HWND wnd_parent) override;

    friend class SpectrumAnalyserConfigData;

private:
    void set_frame_style(unsigned value);

    void configure_renderer()
    {
        colours::helper colours(colour_client_id);
        const auto foreground_colour = colours.get_colour(colours::colour_text);
        const auto background_colour = colours.get_colour(colours::colour_background);
        m_renderer->configure(m_mode, m_scale, foreground_colour, background_colour);
    }

    void restart_renderer()
    {
        m_renderer->stop();

        configure_renderer();

        if (m_playback_api->is_playing() && !m_playback_api->is_paused())
            m_renderer->start();
        else
            RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
    }

    void on_playback_starting(playback_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override
    {
        if (!m_playback_api->is_paused())
            m_renderer->start();
    }
    void on_playback_stop(playback_control::t_stop_reason p_reason) noexcept override
    {
        m_renderer->request_stop();

        if (p_reason != playback_control::stop_reason_shutting_down)
            RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
    }
    void on_playback_seek(double p_time) override {}
    void on_playback_pause(bool p_state) noexcept override
    {
        if (p_state)
            m_renderer->request_stop();
        else
            m_renderer->start();
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

    inline static std::vector<SpectrumAnalyserVisualisation*> s_instances;

    std::optional<SpectrumAnalyserRenderer> m_renderer;
    playback_control::ptr m_playback_api;
    bool m_is_active{};
    Mode m_mode{static_cast<Mode>(cfg_vis_mode.get())};
    Scale m_scale{static_cast<Scale>(cfg_scale.get())};
    Scale m_vertical_scale{Scale::Logarithmic};
    int m_frame{cfg_vis_edge};
};

class SpectrumAnalyserConfigData {
public:
    Mode mode{};
    Scale scale{};
    service_ptr_t<SpectrumAnalyserVisualisation> instance{};
    int frame{};
};

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

    m_mode = static_cast<Mode>(reader->read_lendian_t<int32_t>(p_abort));

    try {
        m_scale = static_cast<Scale>(reader->read_lendian_t<int32_t>(p_abort));
        m_vertical_scale = static_cast<Scale>(reader->read_lendian_t<int32_t>(p_abort));
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
    data.write_lendian_t(WI_EnumValue(m_mode), p_abort);
    data.write_lendian_t(WI_EnumValue(m_scale), p_abort);
    data.write_lendian_t(WI_EnumValue(m_vertical_scale), p_abort);

    writer->write_lendian_t(gsl::narrow<uint32_t>(data.m_data.get_size()), p_abort);
    writer->write(data.m_data.get_ptr(), data.m_data.get_size(), p_abort);
}

void SpectrumAnalyserVisualisation::s_update_colours()
{
    for (const auto instance : s_instances)
        instance->restart_renderer();
}

LRESULT SpectrumAnalyserVisualisation::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        m_is_active = true;
        m_playback_api = playback_control::get();
        s_instances.emplace_back(this);

        play_callback_manager::get()->register_callback(
            this, flag_on_playback_new_track | flag_on_playback_stop | flag_on_playback_pause, false);

        m_renderer.emplace(wnd);
        restart_renderer();
        break;
    }
    case WM_DESTROY: {
        m_is_active = false;
        std::erase(s_instances, this);
        play_callback_manager::get()->unregister_callback(this);
        m_renderer.reset();
        m_playback_api.reset();
        break;
    }
    case WM_ERASEBKGND:
        return FALSE;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        const auto dc = wil::BeginPaint(wnd, &ps);
        m_renderer->handle_paint(dc.get(), ps.rcPaint, !m_playback_api->is_playing());
        return 0;
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

        m_renderer->resize();

        if (m_playback_api->is_playing())
            RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE);
        break;
    }
    }

    return DefWindowProc(wnd, msg, wp, lp);
}

INT_PTR CALLBACK SpectrumPopupProc(SpectrumAnalyserConfigData& state, HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        SendDlgItemMessage(wnd, IDC_BARS, BM_SETCHECK, state.mode == Mode::Bars, 0);

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
            state.mode = (Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED ? Mode::Bars : Mode::Standard);
            break;
        case IDC_FRAME_COMBO | (CBN_SELCHANGE << 16):
            state.frame = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            break;
        case IDC_SCALE | (CBN_SELCHANGE << 16):
            state.scale = static_cast<Scale>(ComboBox_GetCurSel(reinterpret_cast<HWND>(lp)));
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
    cfg_vis_mode = WI_EnumValue(param.mode);
    m_scale = param.scale;
    cfg_scale = WI_EnumValue(param.scale);
    set_frame_style(param.frame);
    cfg_vis_edge = param.frame;

    if (m_is_active)
        restart_renderer();

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
    void on_colour_changed(uint32_t mask) const override { SpectrumAnalyserVisualisation::s_update_colours(); }
};

ColourClient::factory<ColourClient> _colour_client;

} // namespace

} // namespace cui::toolbars::spectrum_analyser
