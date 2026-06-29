#include "pch.h"

#include "vis_spectrum.h"
#include "vis_spectrum_renderer.h"

#include "config_appearance.h"
#include "dark_mode_dialog.h"
#include "string.h"

namespace cui::toolbars::spectrum_analyser {

namespace {

const std::unordered_set valid_fft_sizes{1024u, 2048u, 4096u, 8192u, 16'384u};

uint32_t clean_fft_size(uint32_t fft_size)
{
    return valid_fft_sizes.contains(fft_size) ? fft_size : default_fft_size;
}

int32_t clean_min_frequency(int32_t min_frequency)
{
    return std::clamp(min_frequency, min_allowed_frequency, max_allowed_frequency - 1);
}

int32_t clean_max_frequency(int32_t min_frequency, int32_t max_frequency)
{
    return std::clamp(max_frequency, clean_min_frequency(min_frequency) + 1, max_allowed_frequency);
}

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
cfg_uint cfg_vis_mode(GUID{0x3341d306, 0xf8b6, 0x6c60, {0xbd, 0x7e, 0xe4, 0xc5, 0xab, 0x51, 0xf3, 0xdd}},
    WI_EnumValue(Mode::HatchedBars));
cfg_uint cfg_scale(scale_config_id, WI_EnumValue(Scale::Logarithmic));
cfg_uint cfg_fft_size({0x59ad4dd7, 0x5350, 0x4cdf, {0x97, 0xd3, 0x1a, 0x5e, 0x4f, 0x3b, 0xfc, 0x48}}, default_fft_size);
cfg_int cfg_min_frequency(
    {0x17482311, 0xe67b, 0x42cd, {0x98, 0x7f, 0x69, 0xb9, 0x4d, 0x2d, 0x74, 0x3a}}, default_min_frequency);
cfg_int cfg_max_frequency(
    {0x6e892c2e, 0xbbf7, 0x4904, {0x9f, 0x6c, 0x4e, 0x4d, 0x00, 0x29, 0xa1, 0xc3}}, default_max_frequency);
cfg_bool cfg_smooth_values({0x1517997c, 0xbdce, 0x434b, {0xb3, 0x7b, 0xc7, 0xba, 0xd6, 0xc0, 0xba, 0x81}}, false);
fbh::ConfigInt32DpiAware cfg_bar_width(
    {0xb0f63999, 0xea70, 0x4331, {0x8e, 0x93, 0x9c, 0x70, 0x09, 0x52, 0x46, 0x1a}}, 2);

class SpectrumAnalyserConfigData {
public:
    Mode mode{};
    uih::IntegerAndDpi<int32_t> bar_width{};
    Scale scale{};
    uint32_t fft_size{};
    int32_t min_frequency{};
    int32_t max_frequency{};
    bool smooth_values{};
    int frame{};
};

const dark::DialogDarkModeConfig dark_mode_config{.button_ids = {IDOK, IDCANCEL},
    .checkbox_ids = {IDC_SMOOTH_VALUES},
    .combo_box_ids = {IDC_APPEARANCE, IDC_FRAME_COMBO, IDC_SCALE, IDC_FFT_SIZE},
    .edit_ids = {IDC_MIN_FREQUENCY, IDC_MAX_FREQUENCY},
    .spin_ids = {IDC_BAR_WIDTH_SPIN, IDC_MIN_FREQUENCY_SPIN, IDC_MAX_FREQUENCY_SPIN}};

template <bool IsModal>
class SpectrumAnalyserOptionsDialog {
public:
    operator bool() const { return m_wnd != nullptr; }

    bool open(HWND parent_wnd, const SpectrumAnalyserConfigData& initial_state,
        const service_ptr_t<class SpectrumAnalyserVisualisation>& panel)
    {
        m_initial_state = initial_state;
        m_state = initial_state;

        if constexpr (IsModal) {
            return dark::modal_dialog_box(
                IDD_SPECTRUM_ANALYSER_OPTIONS, dark_mode_config, parent_wnd, [this, panel](auto&&... args) {
                    return handle_options_message(panel, std::forward<decltype(args)>(args)...);
                });

        } else {
            return dark::modeless_dialog_box(IDD_SPECTRUM_ANALYSER_OPTIONS, dark_mode_config, parent_wnd,
                       [this, panel](auto&&... args) {
                           return handle_options_message(panel, std::forward<decltype(args)>(args)...);
                       })
                != nullptr;
        }
    }

    HWND get_wnd() const { return m_wnd; }

private:
    static void set_rendering_options(
        const service_ptr_t<class SpectrumAnalyserVisualisation>& panel, const SpectrumAnalyserConfigData& state);

    INT_PTR handle_options_message(
        const service_ptr_t<class SpectrumAnalyserVisualisation>& panel, HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    HWND m_wnd{};
    SpectrumAnalyserConfigData m_state{};
    SpectrumAnalyserConfigData m_initial_state{};
};

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

    void set_frame_style(unsigned value);
    void set_rendering_options(Mode mode, const uih::IntegerAndDpi<int32_t>& bar_width, Scale scale, uint32_t fft_size,
        int32_t min_frequency, int32_t max_frequency, bool smooth_values);

private:
    void configure_renderer()
    {
        colours::helper colours(colour_client_id);
        const auto foreground_colour = colours.get_colour(colours::colour_text);
        const auto background_colour = colours.get_colour(colours::colour_background);
        m_renderer->configure(m_mode, std::max(1, m_bar_width.get_scaled_value()), m_scale, m_fft_size,
            static_cast<float>(m_min_frequency), static_cast<float>(m_max_frequency), m_smooth_values,
            foreground_colour, background_colour);
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
        p_hook.add_node(uie::menu_node_ptr(new uie::simple_command_menu_node(
            "Options", "Open spectrum analyser options", 0, [this, self{ptr{this}}] { open_options_modeless(); })));
    }

    void set_config(stream_reader* reader, size_t p_size, abort_callback& p_abort) override;
    void get_config(stream_writer* writer, abort_callback& p_abort) const override;

    void open_options_modeless();

    inline static std::vector<SpectrumAnalyserVisualisation*> s_instances;

    SpectrumAnalyserOptionsDialog<true> m_modal_options_dialog;
    SpectrumAnalyserOptionsDialog<false> m_modeless_options_dialog;
    std::optional<SpectrumAnalyserRenderer> m_renderer;
    playback_control::ptr m_playback_api;
    bool m_is_active{};
    Mode m_mode{static_cast<Mode>(cfg_vis_mode.get())};
    uih::IntegerAndDpi<int32_t> m_bar_width{cfg_bar_width.get_raw_value()};
    Scale m_scale{static_cast<Scale>(cfg_scale.get())};
    uint32_t m_fft_size{clean_fft_size(cfg_fft_size.get())};
    int32_t m_min_frequency{clean_min_frequency(cfg_min_frequency.get())};
    int32_t m_max_frequency{clean_max_frequency(cfg_min_frequency.get(), cfg_max_frequency.get())};
    bool m_smooth_values{cfg_smooth_values.get()};
    Scale m_vertical_scale{Scale::Logarithmic};
    int m_frame{cfg_vis_edge};
};

template <bool IsModal>
INT_PTR SpectrumAnalyserOptionsDialog<IsModal>::handle_options_message(
    const service_ptr_t<class SpectrumAnalyserVisualisation>& panel, HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;

        const auto appearance_combo_wnd = GetDlgItem(wnd, IDC_APPEARANCE);
        ComboBox_AddString(appearance_combo_wnd, L"Area");
        ComboBox_AddString(appearance_combo_wnd, L"Hatched bars");
        ComboBox_AddString(appearance_combo_wnd, L"Solid bars");
        ComboBox_SetCurSel(appearance_combo_wnd, WI_EnumValue(m_state.mode));

        const auto wnd_frame_combo = GetDlgItem(wnd, IDC_FRAME_COMBO);
        ComboBox_AddString(wnd_frame_combo, L"None");
        ComboBox_AddString(wnd_frame_combo, L"Sunken");
        ComboBox_AddString(wnd_frame_combo, L"Grey");
        ComboBox_SetCurSel(wnd_frame_combo, m_state.frame);

        const auto wnd_scale_combo = GetDlgItem(wnd, IDC_SCALE);
        ComboBox_AddString(wnd_scale_combo, L"Linear");
        ComboBox_AddString(wnd_scale_combo, L"Logarithmic");
        ComboBox_SetCurSel(wnd_scale_combo, m_state.scale);

        const auto fft_size_wnd = GetDlgItem(wnd, IDC_FFT_SIZE);

        std::locale locale("");

        const std::array fft_size_options = {
            std::make_tuple(1024u, fmt::format(locale, L"{:L} (more responsive)", 1024)),
            std::make_tuple(2048u, fmt::format(locale, L"{:L}", 2048)),
            std::make_tuple(4096u, fmt::format(locale, L"{:L}", 4096)),
            std::make_tuple(8192u, fmt::format(locale, L"{:L}", 8192)),
            std::make_tuple(16'384u, fmt::format(locale, L"{:L} (more resolution)", 16'384)),
        };

        for (auto&& [fft_size, text] : fft_size_options) {
            const auto index = uih::combo_box_add_string_data(fft_size_wnd, text.c_str(), static_cast<int>(fft_size));
            if (m_state.fft_size == fft_size && index != CB_ERR)
                ComboBox_SetCurSel(fft_size_wnd, index);
        }

        const auto bar_width_wnd = GetDlgItem(wnd, IDC_BAR_WIDTH);
        const auto bar_width_spin_wnd = GetDlgItem(wnd, IDC_BAR_WIDTH_SPIN);

        const auto enable_bar_width = m_state.mode == Mode::HatchedBars || m_state.mode == Mode::SolidBars;
        EnableWindow(bar_width_wnd, enable_bar_width);
        EnableWindow(bar_width_spin_wnd, enable_bar_width);

        uih::enhance_edit_control(bar_width_wnd);
        SendMessage(bar_width_spin_wnd, UDM_SETRANGE32, 1, 9999);
        SendMessage(bar_width_spin_wnd, UDM_SETPOS32, 0, m_state.bar_width);

        uih::enhance_edit_control(wnd, IDC_MIN_FREQUENCY);
        const auto min_frequency_spin_wnd = GetDlgItem(wnd, IDC_MIN_FREQUENCY_SPIN);
        SendMessage(min_frequency_spin_wnd, UDM_SETRANGE32, min_allowed_frequency, max_allowed_frequency - 1);
        SendMessage(min_frequency_spin_wnd, UDM_SETPOS32, 0, m_state.min_frequency);

        uih::enhance_edit_control(wnd, IDC_MAX_FREQUENCY);
        const auto max_frequency_spin_wnd = GetDlgItem(wnd, IDC_MAX_FREQUENCY_SPIN);
        SendMessage(max_frequency_spin_wnd, UDM_SETRANGE32, min_allowed_frequency + 1, max_allowed_frequency);
        SendMessage(max_frequency_spin_wnd, UDM_SETPOS32, 0, m_state.max_frequency);

        SendDlgItemMessage(wnd, IDC_SMOOTH_VALUES, BM_SETCHECK, m_state.smooth_values ? BST_CHECKED : BST_UNCHECKED, 0);

        return TRUE;
    }
    case WM_DESTROY:
        m_wnd = nullptr;
        break;
    case WM_COMMAND:
        switch (wp) {
        case IDOK: {
            if (m_state.mode != m_initial_state.mode)
                cfg_vis_mode = WI_EnumValue(m_state.mode);

            if (m_state.bar_width != m_initial_state.bar_width)
                cfg_bar_width = m_state.bar_width;

            if (m_state.scale != m_initial_state.scale)
                cfg_scale = WI_EnumValue(m_state.scale);

            if (const auto cleaned_fft_size = clean_fft_size(m_state.fft_size);
                cleaned_fft_size != m_initial_state.fft_size)
                cfg_fft_size = m_state.fft_size;

            const auto cleaned_min_frequency = clean_min_frequency(m_state.min_frequency);
            if (cleaned_min_frequency != m_initial_state.min_frequency)
                cfg_min_frequency = cleaned_min_frequency;

            if (const auto cleaned_max_frequency = clean_max_frequency(cleaned_min_frequency, m_state.max_frequency);
                cleaned_max_frequency != m_initial_state.max_frequency)
                cfg_max_frequency = cleaned_max_frequency;

            if (m_state.smooth_values != m_initial_state.smooth_values)
                cfg_smooth_values = m_state.smooth_values;

            if constexpr (IsModal) {
                panel->set_frame_style(m_state.frame);
                set_rendering_options(panel, m_state);
                EndDialog(wnd, 1);
            } else {
                DestroyWindow(wnd);
            }
            return TRUE;
        }
        case IDCANCEL: {
            if constexpr (IsModal) {
                EndDialog(wnd, 0);
            } else {
                panel->set_frame_style(m_initial_state.frame);
                set_rendering_options(panel, m_initial_state);

                DestroyWindow(wnd);
            }
            return TRUE;
        }
        case IDC_APPEARANCE | CBN_SELCHANGE << 16: {
            if (const auto index = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp)); index != CB_ERR) {
                m_state.mode = static_cast<Mode>(index);

                const auto enable_bar_width = m_state.mode == Mode::HatchedBars || m_state.mode == Mode::SolidBars;
                EnableWindow(GetDlgItem(wnd, IDC_BAR_WIDTH), enable_bar_width);
                EnableWindow(GetDlgItem(wnd, IDC_BAR_WIDTH_SPIN), enable_bar_width);

                if constexpr (!IsModal)
                    set_rendering_options(panel, m_state);
            }
            break;
        }
        case IDC_FRAME_COMBO | CBN_SELCHANGE << 16:
            if (const auto index = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp)); index != CB_ERR) {
                m_state.frame = index;

                if constexpr (!IsModal)
                    panel->set_frame_style(m_state.frame);
            }
            break;
        case IDC_SCALE | CBN_SELCHANGE << 16:
            if (const auto index = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp)); index != CB_ERR) {
                m_state.scale = static_cast<Scale>(index);

                if constexpr (!IsModal)
                    set_rendering_options(panel, m_state);
            }
            break;
        case IDC_FFT_SIZE | CBN_SELCHANGE << 16: {
            const auto combo_wnd = reinterpret_cast<HWND>(lp);
            const auto index = ComboBox_GetCurSel(combo_wnd);

            if (index != CB_ERR)
                m_state.fft_size = static_cast<uint32_t>(ComboBox_GetItemData(combo_wnd, index));

            if constexpr (!IsModal)
                set_rendering_options(panel, m_state);

            break;
        }
        case IDC_SMOOTH_VALUES:
            m_state.smooth_values = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;

            if constexpr (!IsModal)
                set_rendering_options(panel, m_state);

            break;
        case IDC_BAR_WIDTH | EN_CHANGE << 16: {
            const auto new_value_text = uih::get_window_text(reinterpret_cast<HWND>(lp));

            if (const auto new_value_int = string::parse_int_forgiving(new_value_text))
                m_state.bar_width = std::max(1, *new_value_int);

            if constexpr (!IsModal)
                set_rendering_options(panel, m_state);

            break;
        }
        case IDC_MIN_FREQUENCY | EN_CHANGE << 16: {
            const auto new_value_text = uih::get_window_text(reinterpret_cast<HWND>(lp));

            if (const auto new_value_int = string::parse_int_forgiving(new_value_text))
                m_state.min_frequency = *new_value_int;

            if constexpr (!IsModal)
                set_rendering_options(panel, m_state);

            break;
        }
        case IDC_MAX_FREQUENCY | EN_CHANGE << 16: {
            const auto new_value_text = uih::get_window_text(reinterpret_cast<HWND>(lp));

            if (const auto new_value_int = string::parse_int_forgiving(new_value_text))
                m_state.max_frequency = *new_value_int;

            if constexpr (!IsModal)
                set_rendering_options(panel, m_state);

            break;
        }
        }
    }

    return FALSE;
}

template <bool IsModal>
void SpectrumAnalyserOptionsDialog<IsModal>::set_rendering_options(
    const service_ptr_t<class SpectrumAnalyserVisualisation>& panel, const SpectrumAnalyserConfigData& state)
{
    panel->set_rendering_options(state.mode, state.bar_width, state.scale, state.fft_size, state.min_frequency,
        state.max_frequency, state.smooth_values);
}

void SpectrumAnalyserVisualisation::set_frame_style(unsigned value)
{
    if (m_frame == value)
        return;

    m_frame = value;
    cfg_vis_edge = value;

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

void SpectrumAnalyserVisualisation::set_rendering_options(Mode mode, const uih::IntegerAndDpi<int32_t>& bar_width,
    Scale scale, uint32_t fft_size, int32_t min_frequency, int32_t max_frequency, bool smooth_values)
{
    const auto cleaned_fft_size = clean_fft_size(fft_size);
    const auto cleaned_min_frequency = clean_min_frequency(min_frequency);
    const auto cleaned_max_frequency = clean_max_frequency(cleaned_min_frequency, max_frequency);

    if (mode == m_mode && bar_width == m_bar_width && scale == m_scale && cleaned_fft_size == m_fft_size
        && cleaned_min_frequency == m_min_frequency && cleaned_max_frequency == m_max_frequency
        && smooth_values == m_smooth_values)
        return;

    m_mode = mode;
    m_bar_width = bar_width;
    m_scale = scale;
    m_fft_size = cleaned_fft_size;
    m_min_frequency = cleaned_min_frequency;
    m_max_frequency = cleaned_max_frequency;
    m_smooth_values = smooth_values;

    if (m_is_active)
        restart_renderer();
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
        m_fft_size = clean_fft_size(reader->read_lendian_t<uint32_t>(p_abort));
        m_min_frequency = clean_min_frequency(reader->read_lendian_t<int32_t>(p_abort));
        m_max_frequency = clean_max_frequency(m_min_frequency, reader->read_lendian_t<int32_t>(p_abort));
        m_smooth_values = reader->read_lendian_t<bool>(p_abort);

        const auto value = reader->read_lendian_t<int32_t>(p_abort);
        const auto dpi = reader->read_lendian_t<uint32_t>(p_abort);
        m_bar_width.set(value, dpi);

        m_mode = static_cast<Mode>(reader->read_lendian_t<int32_t>(p_abort));
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
    data.write_lendian_t(WI_EnumValue(m_mode == Mode::SolidBars ? Mode::HatchedBars : m_mode), p_abort);
    data.write_lendian_t(WI_EnumValue(m_scale), p_abort);
    data.write_lendian_t(WI_EnumValue(m_vertical_scale), p_abort);
    data.write_lendian_t(m_fft_size, p_abort);
    data.write_lendian_t(m_min_frequency, p_abort);
    data.write_lendian_t(m_max_frequency, p_abort);
    data.write_lendian_t(m_smooth_values, p_abort);
    data.write_lendian_t(m_bar_width.value, p_abort);
    data.write_lendian_t(m_bar_width.dpi, p_abort);
    data.write_lendian_t(WI_EnumValue(m_mode), p_abort);

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

bool SpectrumAnalyserVisualisation::show_config_popup(HWND wnd_parent)
{
    if (m_modeless_options_dialog || m_modal_options_dialog)
        return false;

    SpectrumAnalyserConfigData param{
        m_mode, m_bar_width, m_scale, m_fft_size, m_min_frequency, m_max_frequency, m_smooth_values, m_frame};

    return m_modal_options_dialog.open(wnd_parent, param, service_ptr_t{this});
}

void SpectrumAnalyserVisualisation::open_options_modeless()
{
    if (m_modal_options_dialog) {
        SetForegroundWindow(m_modal_options_dialog.get_wnd());
        return;
    }

    if (m_modeless_options_dialog) {
        SetForegroundWindow(m_modeless_options_dialog.get_wnd());
        return;
    }

    SpectrumAnalyserConfigData param{
        m_mode, m_bar_width, m_scale, m_fft_size, m_min_frequency, m_max_frequency, m_smooth_values, m_frame};

    m_modeless_options_dialog.open(GetAncestor(get_wnd(), GA_ROOT), param, service_ptr_t{this});
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
