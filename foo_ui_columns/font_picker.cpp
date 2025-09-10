#include "pch.h"

#include "font_picker.h"

#include "dark_mode.h"
#include "dark_mode_active_ui.h"
#include "dark_mode_dialog.h"
#include "string.h"

namespace cui::utils {

namespace {

wchar_t narrow_to_wchar_t(uint32_t value)
{
    return static_cast<wchar_t>(value & 0xff);
}

class ConfigureAxesDialog {
public:
    static bool g_create(HWND wnd, uih::direct_write::FontFamily font_family, uih::direct_write::Font font,
        uih::direct_write::AxisValues axis_values,
        std::function<void(const uih::direct_write::AxisValues&)> on_values_change)
    {
        ConfigureAxesDialog dialog(
            std::move(font_family), std::move(font), std::move(axis_values), std::move(on_values_change));

        const dark::DialogDarkModeConfig dark_mode_config{
            .button_ids = {IDOK, IDCANCEL},
            .combo_box_ids = {IDC_AXIS},
            .edit_ids = {IDC_AXIS_VALUE},
            .spin_ids = {IDC_AXIS_VALUE_SPIN},
        };

        return dark::modal_dialog_box(IDD_FONT_AXES, dark_mode_config, wnd,
                   [&dialog](auto wnd, auto msg, auto wp, auto lp) { return dialog.handle_message(wnd, msg, wp, lp); })
            != 0;
    }

private:
    ConfigureAxesDialog(uih::direct_write::FontFamily font_family, uih::direct_write::Font font,
        uih::direct_write::AxisValues axis_values,
        std::function<void(const uih::direct_write::AxisValues&)> on_values_change)
        : m_font_family(std::move(font_family))
        , m_font(std::move(font))
        , m_axis_values(axis_values)
        , m_initial_axis_values(std::move(axis_values))
        , m_on_values_change(on_values_change)
    {
    }

    void update_axis_value_edit()
    {
        if (!m_axis)
            return;

        const auto tag = WI_EnumValue(m_axis->tag);
        const auto value = m_axis_values.at(tag);

        const auto axis_value_text = fmt::format(L"{:.01f}", value);

        pfc::vartoggle_t _(m_is_updating_axis_value_edit, true);
        SetWindowText(m_axis_value_edit, axis_value_text.c_str());
    }

    void update_axis_value_spin()
    {
        if (!m_axis)
            return;

        const auto tag = WI_EnumValue(m_axis->tag);
        const auto value = m_axis_values.at(tag);

        const auto spin_pos = gsl::narrow_cast<int>(std::round(value * 10.0f));
        SendMessage(m_axis_value_spin, UDM_SETPOS32, 0, spin_pos);
    }

    void on_axis_change()
    {
        const auto index = ComboBox_GetCurSel(m_axis_wnd);

        if (index == CB_ERR) {
            m_axis.reset();
            return;
        }

        m_axis = m_font_family.axes[index];

        const auto spin_min = gsl::narrow_cast<int>(std::round(m_axis->min * 10.0f));
        const auto spin_max = gsl::narrow_cast<int>(std::round(m_axis->max * 10.0f));
        SendMessage(m_axis_value_spin, UDM_SETRANGE32, spin_min, spin_max);

        const auto tag = WI_EnumValue(m_axis->tag);

        if (m_axis_values.contains(tag)) {
            update_axis_value_edit();
            update_axis_value_spin();

            m_spin_step = m_axis->max - m_axis->min >= 100.0f ? 10 : 1;
        }

        const auto axis_range_text = fmt::format(L"{} – {}", m_axis->min, m_axis->max);
        SetWindowText(GetDlgItem(m_wnd, IDC_AXIS_RANGE), axis_range_text.c_str());
    }

    bool on_axis_value_change(float new_value)
    {
        const auto tag = m_axis->tag;
        const auto clamped_new_value = std::clamp(new_value, m_axis->min, m_axis->max);

        if (clamped_new_value == m_axis_values.at(tag))
            return false;

        m_dirty = true;
        m_axis_values.insert_or_assign(tag, clamped_new_value);

        const auto now = std::chrono::steady_clock::now();
        const auto delay = m_last_apply_time
            ? std::make_optional(
                  std::chrono::duration_cast<std::chrono::milliseconds>(0.1s - (now - *m_last_apply_time)).count())
            : std::nullopt;

        if (m_apply_pending)
            return true;

        if (!delay || delay <= 0) {
            m_on_values_change(m_axis_values);
            m_last_apply_time = now;
        } else {
            SetTimer(m_wnd, timer_id, gsl::narrow_cast<UINT>(*delay), nullptr);
            m_apply_pending = true;
        }

        return true;
    }

    INT_PTR handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            m_wnd = wnd;
            m_axis_wnd = GetDlgItem(wnd, IDC_AXIS);
            m_axis_value_edit = GetDlgItem(m_wnd, IDC_AXIS_VALUE);
            m_axis_value_spin = GetDlgItem(m_wnd, IDC_AXIS_VALUE_SPIN);

            uih::enhance_edit_control(m_axis_value_edit);

            for (auto&& axis : m_font_family.axes) {
                const wchar_t name[5] = {narrow_to_wchar_t(axis.tag), narrow_to_wchar_t(axis.tag >> 8),
                    narrow_to_wchar_t(axis.tag >> 16), narrow_to_wchar_t(axis.tag >> 24), 0};

                ComboBox_AddString(m_axis_wnd, name);
            }

            ComboBox_SetCurSel(m_axis_wnd, 0);
            on_axis_change();

            break;
        }
        case WM_COMMAND:
            switch (wp) {
            case IDOK:
                if (m_apply_pending) {
                    m_on_values_change(m_axis_values);
                    m_apply_pending = false;
                }

                EndDialog(wnd, m_dirty);
                break;
            case IDCANCEL:
                if (m_dirty)
                    m_on_values_change(m_initial_axis_values);

                EndDialog(wnd, 0);
                break;
            case IDC_AXIS | (CBN_SELCHANGE << 16):
                on_axis_change();
                break;
            case IDC_AXIS_VALUE | (EN_CHANGE << 16):
                if (!m_axis || m_is_updating_axis_value_edit)
                    break;

                const auto new_value_text = uih::get_window_text(m_axis_value_edit);
                const auto new_value_float = string::safe_stof(new_value_text);

                if (!new_value_float)
                    break;

                if (on_axis_value_change(*new_value_float))
                    update_axis_value_spin();

                break;
            }
            break;
        case WM_NOTIFY: {
            const auto nmhdr = reinterpret_cast<LPNMHDR>(lp);
            switch (nmhdr->idFrom) {
            case IDC_AXIS_VALUE_SPIN:
                switch (nmhdr->code) {
                case UDN_DELTAPOS: {
                    if (!m_axis)
                        break;

                    const auto nmupdown = reinterpret_cast<LPNMUPDOWN>(lp);

                    const auto spin_min = gsl::narrow_cast<int>(std::round(m_axis->min * 10.0f));
                    const auto spin_max = gsl::narrow_cast<int>(std::round(m_axis->max * 10.0f));
                    int new_spin_value{};
                    float new_axis_value{};

                    if (m_axis->is_toggle) {
                        new_spin_value = nmupdown->iDelta > 0 ? spin_max : spin_min;
                        new_axis_value = nmupdown->iDelta > 0 ? m_axis->max : m_axis->min;
                        nmupdown->iDelta = new_spin_value - nmupdown->iPos;
                    } else {
                        nmupdown->iDelta *= m_spin_step;
                        new_spin_value = std::clamp(nmupdown->iPos + nmupdown->iDelta, spin_min, spin_max);
                        new_axis_value = gsl::narrow_cast<float>(new_spin_value) / 10.0f;
                    }

                    if (on_axis_value_change(new_axis_value))
                        update_axis_value_edit();

                    return 0;
                }
                }
                break;
            }
            break;
        }
        case WM_TIMER:
            if (wp != timer_id)
                break;

            KillTimer(wnd, timer_id);
            m_on_values_change(m_axis_values);
            m_last_apply_time = std::chrono::steady_clock::now();
            m_apply_pending = false;
            break;
        }
        return FALSE;
    }

    static constexpr WORD timer_id = 100;

    uih::direct_write::FontFamily m_font_family;
    uih::direct_write::Font m_font;
    uih::direct_write::AxisValues m_axis_values;
    uih::direct_write::AxisValues m_initial_axis_values;
    std::function<void(const uih::direct_write::AxisValues&)> m_on_values_change;
    HWND m_wnd{};
    HWND m_axis_wnd{};
    HWND m_axis_value_edit{};
    HWND m_axis_value_spin{};
    bool m_is_updating_axis_value_edit{};
    int m_spin_step{10};
    std::optional<uih::direct_write::AxisRange> m_axis;
    bool m_dirty{};
    bool m_apply_pending{};
    std::optional<std::chrono::steady_clock::time_point> m_last_apply_time;
};

} // namespace

std::optional<INT_PTR> DirectWriteFontPicker::handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        handle_wm_init_dialog(wnd);
        return {};
    case WM_COMMAND:
        return handle_wm_command(wp, lp);
    case WM_DRAWITEM:
        return handle_wm_draw_item(reinterpret_cast<LPDRAWITEMSTRUCT>(lp));
    case WM_MEASUREITEM:
        return handle_wm_measure_item(reinterpret_cast<LPMEASUREITEMSTRUCT>(lp));
    case WM_NOTIFY:
        return handle_wm_notify(reinterpret_cast<LPNMHDR>(lp));
    }

    return {};
}

void DirectWriteFontPicker::on_font_changed(std::function<void(const fonts::FontDescription&)> on_font_changed)
{
    m_on_font_changed = std::move(on_font_changed);
}

void DirectWriteFontPicker::set_font_description(fonts::FontDescription font_description)
{
    m_font_description = std::move(font_description);

    if (!m_direct_write_context)
        return;

    update_font_size_edit();
    update_font_size_spin();

    const auto& wss = m_font_description->wss;

    if (!wss) {
        ComboBox_SetCurSel(m_font_family_combobox, -1);
        handle_family_change();
        return;
    }

    const auto& typographic_family_name = m_font_description->typographic_family_name;

    const auto resolved_names = m_direct_write_context->resolve_font_names(wss->family_name.c_str(),
        typographic_family_name.c_str(), wss->weight, wss->stretch, wss->style, m_font_description->axis_values);

    if (!resolved_names) {
        ComboBox_SetCurSel(m_font_family_combobox, -1);
        handle_family_change();
        return;
    }

    const auto& [family_name, face_name] = *resolved_names;

    const auto iter = ranges::find_if(
        m_font_families, [&family_name](auto&& family) { return family.display_name() == family_name; });

    ComboBox_SetCurSel(
        m_font_family_combobox, iter != std::end(m_font_families) ? std::distance(m_font_families.begin(), iter) : -1);
    handle_family_change();

    std::optional<size_t> face_index;

    if (!face_name.empty()) {
        const auto face_iter
            = ranges::find_if(m_font_faces, [&face_name](auto&& face) { return face.localised_name == face_name; });

        if (face_iter != m_font_faces.end())
            face_index = std::distance(m_font_faces.begin(), face_iter);
    }

    if (face_index) {
        const auto& font_face = m_font_faces[*face_index];
        if (!font_face.axis_values.empty() && !m_font_description->axis_values.empty()
            && font_face.axis_values != m_font_description->axis_values)
            ComboBox_SetCurSel(m_font_face_combobox, m_font_faces.size());
        else
            ComboBox_SetCurSel(m_font_face_combobox, *face_index);
    } else
        ComboBox_SetCurSel(m_font_face_combobox, -1);

    handle_face_change();
}

void DirectWriteFontPicker::set_font_selection_allowed(bool value) const
{
    const auto enable = value && m_direct_write_context;
    EnableWindow(m_font_family_combobox, enable);
    EnableWindow(m_font_face_combobox, enable);
    EnableWindow(m_font_size_edit, enable);
    EnableWindow(m_font_size_spin, enable);
}

void DirectWriteFontPicker::handle_wm_init_dialog(HWND wnd)
{
    m_wnd = wnd;
    m_configure_axes_button = GetDlgItem(m_wnd, IDC_CONFIGURE_AXES);
    m_font_family_combobox = GetDlgItem(m_wnd, IDC_FONT_FAMILY);
    m_font_face_combobox = GetDlgItem(m_wnd, IDC_FONT_FACE);
    m_font_size_edit = GetDlgItem(m_wnd, IDC_FONT_SIZE);
    m_font_size_spin = GetDlgItem(m_wnd, IDC_FONT_SIZE_SPIN);

    uih::enhance_edit_control(m_font_size_edit);

    SendMessage(m_font_family_combobox, CB_SETITEMHEIGHT, -1, 14_spx);
    SendMessage(m_font_face_combobox, CB_SETITEMHEIGHT, -1, 14_spx);
    SendMessage(m_font_size_spin, UDM_SETRANGE32, 10, 720);

    try {
        m_direct_write_context = uih::direct_write::Context::s_create();
        m_font_families = m_direct_write_context->get_font_families();
    }
    CATCH_LOG()

    m_font_families_text_formats.resize(m_font_families.size());

    for (auto&& family : m_font_families)
        ComboBox_AddString(m_font_family_combobox, family.display_name().c_str());

    if (m_font_families.empty()) {
        dark::modeless_info_box(m_wnd, "Error initialising fonts list",
            "There was an error listing fonts using DirectWrite.", uih::InfoBoxType::Error);
    }
}

std::optional<INT_PTR> DirectWriteFontPicker::handle_wm_command(WPARAM wp, LPARAM lp)
{
    switch (wp) {
    case IDC_CONFIGURE_AXES: {
        if (!m_font_family || !m_font_face || !m_font_description)
            break;

        const auto& axis_values = m_font_description->axis_values.empty() ? m_font_face->get().axis_values
                                                                          : m_font_description->axis_values;
        const auto axes_configured = ConfigureAxesDialog::g_create(
            m_wnd, *m_font_family, *m_font_face, axis_values, [this](const auto& new_axis_values) {
                if (!m_font_description)
                    return;

                m_font_description->axis_values = new_axis_values;

                const auto wss_and_log_font = m_direct_write_context->get_wss_and_logfont_for_axis_values(
                    m_font_description->typographic_family_name.c_str(), new_axis_values);

                if (wss_and_log_font) {
                    m_font_description->wss = std::get<uih::direct_write::WeightStretchStyle>(*wss_and_log_font);
                    m_font_description->log_font = std::get<LOGFONT>(*wss_and_log_font);
                }

                notify_font_changed();
            });

        if (axes_configured)
            ComboBox_SetCurSel(m_font_face_combobox, m_font_faces.size());

        break;
    }
    case IDC_FONT_FAMILY | (CBN_SELCHANGE << 16): {
        if (!m_direct_write_context)
            break;

        handle_family_change();
        store_font_face();
        notify_font_changed();
        return TRUE;
    }
    case IDC_FONT_FACE | (CBN_SELCHANGE << 16): {
        if (!m_direct_write_context)
            break;

        handle_face_change();
        store_font_face();
        notify_font_changed();
        return TRUE;
    }
    case IDC_FONT_SIZE | (EN_CHANGE << 16): {
        if (m_is_updating_font_size_edit || !m_font_description)
            break;

        const auto previous_size = m_font_description->point_size_tenths;
        const auto font_size_text = uih::get_window_text(m_font_size_edit);

        const auto font_size_float = string::safe_stof(font_size_text);

        if (!font_size_float)
            break;

        m_font_description->set_point_size(*font_size_float);

        update_font_size_spin();

        if (m_font_description->point_size_tenths != previous_size)
            notify_font_changed();
        return TRUE;
    }
    }
    return {};
}

std::optional<INT_PTR> DirectWriteFontPicker::handle_wm_notify(LPNMHDR nmhdr)
{
    switch (nmhdr->idFrom) {
    case IDC_FONT_SIZE_SPIN:
        switch (nmhdr->code) {
        case UDN_DELTAPOS: {
            const auto nmupdown = reinterpret_cast<LPNMUPDOWN>(nmhdr);
            nmupdown->iDelta *= 10;

            if (!m_font_description)
                break;

            const auto new_font_size_tenths = std::clamp(nmupdown->iPos + nmupdown->iDelta, 10, 720);
            m_font_description->set_point_size_tenths(new_font_size_tenths);
            update_font_size_edit();

            notify_font_changed();
            return 0;
        }
        }
        break;
    }

    return {};
}

std::optional<INT_PTR> DirectWriteFontPicker::handle_wm_measure_item(LPMEASUREITEMSTRUCT mis)
{
    if (!m_direct_write_context || mis->CtlType != ODT_COMBOBOX
        || (mis->CtlID != IDC_FONT_FAMILY && mis->CtlID != IDC_FONT_FACE))
        return {};

    const auto is_family = mis->CtlID == IDC_FONT_FAMILY;
    const auto index = mis->itemID;
    const auto& text = is_family ? m_font_families[index].display_name() : get_font_face_combobox_item_text(index);

    try {
        const auto& text_format = is_family ? get_family_text_format(index) : get_face_text_format(index);
        mis->itemHeight = text_format.get_minimum_height(text) + 4_spx;
    }
    CATCH_LOG()

    return TRUE;
}

std::optional<INT_PTR> DirectWriteFontPicker::handle_wm_draw_item(LPDRAWITEMSTRUCT dis)
{
    if (!m_direct_write_context || dis->CtlType != ODT_COMBOBOX
        || (dis->CtlID != IDC_FONT_FAMILY && dis->CtlID != IDC_FONT_FACE))
        return {};

    const auto is_family = dis->CtlID == IDC_FONT_FAMILY;
    const auto is_focused = (dis->itemState & ODS_FOCUS) != 0;
    const auto is_edit_box = (dis->itemState & ODS_COMBOBOXEDIT) != 0;
    const auto is_selected = (dis->itemState & ODS_SELECTED) != 0;
    const auto index = dis->itemID;
    const auto is_dark = dark::is_active_ui_dark(m_allow_cui_dark_mode_fallback);
    const auto hide_focus = (SendMessage(dis->hwndItem, WM_QUERYUISTATE, NULL, NULL) & UISF_HIDEFOCUS) != 0;

    const auto background_colour = [&dis, is_dark, is_edit_box, is_selected] {
        if (is_edit_box && is_dark)
            return dark::get_dark_colour(dark::ColourID::ComboBoxEditBackground);

        if (is_selected)
            return GetSysColor(COLOR_HIGHLIGHT);

        return GetBkColor(dis->hDC);
    }();

    uih::BufferedDC buffered_dc(dis->hDC, dis->rcItem);
    FillRect(buffered_dc.get(), &dis->rcItem, wil::unique_hbrush(CreateSolidBrush(background_colour)).get());

    const auto draw_focus_rect = [&] {
        if (is_focused && !hide_focus) {
            DrawFocusRect(buffered_dc.get(), &dis->rcItem);
        }
    };

    if (index == -1) {
        draw_focus_rect();
        return TRUE;
    }

    const auto text_colour = [&dis, is_dark, is_edit_box, is_selected] {
        if (is_edit_box && is_dark)
            return dark::get_dark_colour(dark::ColourID::ComboBoxEditText);

        if (is_selected)
            return GetSysColor(COLOR_HIGHLIGHTTEXT);

        return GetTextColor(dis->hDC);
    }();

    auto& text = is_family ? m_font_families[index].display_name() : get_font_face_combobox_item_text(index);

    if (is_edit_box) {
        auto _ = wil::SelectObject(buffered_dc.get(), GetWindowFont(dis->hwndItem));
        SetBkMode(buffered_dc.get(), TRANSPARENT);
        SetTextColor(buffered_dc.get(), text_colour);
        DrawTextEx(buffered_dc.get(), const_cast<wchar_t*>(text.data()), gsl::narrow<int>(text.length()), &dis->rcItem,
            DT_SINGLELINE | DT_VCENTER, nullptr);
        draw_focus_rect();
        return TRUE;
    }

    const auto max_width = uih::direct_write::px_to_dip(gsl::narrow_cast<float>(wil::rect_width(dis->rcItem)));
    const auto max_height = uih::direct_write::px_to_dip(gsl::narrow_cast<float>(wil::rect_height(dis->rcItem)));

    try {
        const auto& text_format = is_family ? get_family_text_format(index) : get_face_text_format(index);
        const auto text_layout = text_format.create_text_layout(text, max_width, max_height);
        text_layout.render_with_transparent_background(m_wnd, buffered_dc.get(), dis->rcItem, text_colour);
    }
    CATCH_LOG()

    draw_focus_rect();
    return TRUE;
}

const wchar_t* DirectWriteFontPicker::get_font_face_combobox_item_text(uint32_t index) const
{
    if (index < m_font_faces.size())
        return m_font_faces[index].localised_name.c_str();

    return custom_axis_values_label;
}

wil::com_ptr<IDWriteFontFamily> DirectWriteFontPicker::get_icon_font_family() const
{
    LOGFONT log_font{};
    THROW_IF_WIN32_BOOL_FALSE(SystemParametersInfo(SPI_GETICONTITLELOGFONT, 0, &log_font, 0));

    const auto font = m_direct_write_context->create_font(log_font);

    wil::com_ptr<IDWriteFontFamily> font_family;
    THROW_IF_FAILED(font->GetFontFamily(&font_family));

    return font_family;
}

uih::direct_write::TextFormat& DirectWriteFontPicker::get_family_text_format(size_t index)
{
    const auto& [family, _wss_name, _typographic_name, is_symbol_font, _axes] = m_font_families.at(index);
    auto& text_format = m_font_families_text_formats.at(index);

    if (!text_format) {
        text_format = m_direct_write_context->create_text_format(is_symbol_font ? get_icon_font_family() : family,
            DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL,
            uih::direct_write::pt_to_dip(font_dropdown_font_size));
    }

    return *text_format;
}

uih::direct_write::TextFormat& DirectWriteFontPicker::get_face_text_format(size_t index)
{
    const auto& [family, _wss_name, _typographic_name, is_symbol_font, _axes] = m_font_family->get();

    if (index == m_font_faces.size()) {
        if (!m_custom_axis_values_text_format)
            m_custom_axis_values_text_format
                = m_direct_write_context->create_text_format(L"", DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL, uih::direct_write::pt_to_dip(font_dropdown_font_size));

        return *m_custom_axis_values_text_format;
    }

    const auto& font = m_font_faces.at(index);
    auto& text_format = m_font_faces_text_formats.at(index);

    if (!text_format) {
        text_format
            = m_direct_write_context->create_text_format(is_symbol_font ? get_icon_font_family() : family, font.weight,
                font.stretch, font.style, uih::direct_write::pt_to_dip(font_dropdown_font_size), font.axis_values);
    }

    return *text_format;
}

void DirectWriteFontPicker::handle_family_change(bool skip_face_change)
{
    const auto index = ComboBox_GetCurSel(m_font_family_combobox);
    m_font_family = index != CB_ERR ? std::make_optional(std::reference_wrapper(m_font_families[index])) : std::nullopt;

    const auto previous_font_face_name
        = m_font_face ? std::make_optional(m_font_face->get().localised_name) : std::nullopt;
    ComboBox_ResetContent(m_font_face_combobox);
    m_font_face.reset();
    m_font_faces.clear();
    m_font_faces_text_formats.clear();

    if (m_configure_axes_button)
        EnableWindow(m_configure_axes_button, m_font_family && !m_font_family->get().axes.empty());

    if (!m_font_family)
        return;

    try {
        m_font_faces = m_font_family->get().fonts();
        m_font_faces_text_formats.resize(m_font_faces.size());

        for (auto&& font : m_font_faces)
            ComboBox_AddString(m_font_face_combobox, font.localised_name.c_str());

        if (m_configure_axes_button && !m_font_family->get().axes.empty())
            ComboBox_AddString(m_font_face_combobox, custom_axis_values_label);
    }
    CATCH_LOG()

    if (skip_face_change)
        return;

    bool font_face_found{};

    if (previous_font_face_name)
        font_face_found = ComboBox_SelectString(m_font_face_combobox, -1, previous_font_face_name->c_str()) != CB_ERR;

    if (!font_face_found)
        ComboBox_SetCurSel(m_font_face_combobox, 0);

    handle_face_change();
}

void DirectWriteFontPicker::handle_face_change()
{
    const auto index = ComboBox_GetCurSel(m_font_face_combobox);

    if (index == CB_ERR)
        m_font_face.reset();
    else if (gsl::narrow_cast<size_t>(index) < m_font_faces.size())
        m_font_face = std::ref(m_font_faces[index]);
    else if (!m_font_face && !m_font_faces.empty())
        m_font_face = m_font_faces[0];
}

void DirectWriteFontPicker::notify_font_changed() const
{
    if (m_on_font_changed)
        m_on_font_changed(*m_font_description);
}

void DirectWriteFontPicker::store_font_face()
{
    if (!m_font_face || !m_direct_write_context || !m_font_description)
        return;

    uih::direct_write::WeightStretchStyle wss;
    wss.family_name = m_font_family->get().wss_name;
    wss.weight = m_font_face->get().weight;
    wss.stretch = m_font_face->get().stretch;
    wss.style = m_font_face->get().style;

    m_font_description->wss = wss;
    m_font_description->typographic_family_name = m_font_family->get().typographic_name;
    m_font_description->axis_values = m_font_face->get().axis_values;
    m_font_description->log_font = m_direct_write_context->create_log_font(m_font_face->get().font);
    m_font_description->recalculate_log_font_height();
}

void DirectWriteFontPicker::update_font_size_edit()
{
    const auto font_size_tenths = m_font_description->point_size_tenths;
    const auto font_size_float = gsl::narrow_cast<float>(font_size_tenths) / 10.0f;
    const auto font_size_text = fmt::format(L"{:.01f}", font_size_float);

    m_is_updating_font_size_edit = true;
    auto _ = gsl::finally([this] { m_is_updating_font_size_edit = false; });
    SetWindowText(m_font_size_edit, font_size_text.c_str());
}

void DirectWriteFontPicker::update_font_size_spin() const
{
    const auto font_size_tenths = m_font_description->point_size_tenths;
    SendMessage(m_font_size_spin, UDM_SETPOS32, 0, font_size_tenths);
}

} // namespace cui::utils
