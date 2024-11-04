#include "pch.h"

#include "tab_fonts.h"

#include "dark_mode.h"
#include "dark_mode_active_ui.h"
#include "dark_mode_dialog.h"
#include "string.h"

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

        const cui::dark::DialogDarkModeConfig dark_mode_config{
            .button_ids = {IDOK, IDCANCEL},
            .combo_box_ids = {IDC_AXIS},
            .edit_ids = {IDC_AXIS_VALUE},
            .spin_ids = {IDC_AXIS_VALUE_SPIN},
        };

        return cui::dark::modal_dialog_box(IDD_FONT_AXES, dark_mode_config, wnd,
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
        SendMessage(GetDlgItem(m_wnd, IDC_AXIS_VALUE_SPIN), UDM_SETRANGE32, spin_min, spin_max);

        const auto tag = WI_EnumValue(m_axis->tag);

        if (m_axis_values.contains(tag)) {
            const auto value = m_axis_values.at(tag);

            const auto axis_value_text = fmt::format(L"{:.01f}", value);
            SetWindowText(GetDlgItem(m_wnd, IDC_AXIS_VALUE), axis_value_text.c_str());

            const auto spin_pos = gsl::narrow_cast<int>(std::round(value * 10.0f));
            SendMessage(GetDlgItem(m_wnd, IDC_AXIS_VALUE_SPIN), UDM_SETPOS32, 0, spin_pos);

            m_spin_step = m_axis->max - m_axis->min >= 100.0f ? 10 : 1;
        }

        const auto axis_range_text = fmt::format(L"{} – {}", m_axis->min, m_axis->max);
        SetWindowText(GetDlgItem(m_wnd, IDC_AXIS_RANGE), axis_range_text.c_str());
    }

    INT_PTR handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            m_scope.initialize(wnd);
            m_wnd = wnd;
            m_axis_wnd = GetDlgItem(wnd, IDC_AXIS);

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

                    const auto tag = m_axis->tag;

                    if (new_axis_value == m_axis_values.at(tag))
                        return 0;

                    m_dirty = true;
                    m_axis_values.insert_or_assign(tag, new_axis_value);

                    const auto axis_value_text = fmt::format(L"{:.01f}", new_axis_value);
                    SetWindowText(GetDlgItem(m_wnd, IDC_AXIS_VALUE), axis_value_text.c_str());

                    const auto now = std::chrono::steady_clock::now();
                    const auto delay = m_last_apply_time
                        ? std::make_optional(
                              std::chrono::duration_cast<std::chrono::milliseconds>(0.1s - (now - *m_last_apply_time))
                                  .count())
                        : std::nullopt;

                    if (m_apply_pending)
                        return 0;

                    if (!delay || delay <= 0) {
                        m_on_values_change(m_axis_values);
                        m_last_apply_time = now;
                    } else {
                        SetTimer(wnd, timer_id, gsl::narrow_cast<UINT>(*delay), nullptr);
                        m_apply_pending = true;
                    }

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
    modal_dialog_scope m_scope;
    HWND m_wnd{};
    HWND m_axis_wnd{};
    int m_spin_step{10};
    std::optional<uih::direct_write::AxisRange> m_axis;
    bool m_dirty{};
    bool m_apply_pending{};
    std::optional<std::chrono::steady_clock::time_point> m_last_apply_time;
};

} // namespace

bool TabFonts::is_active() const
{
    return m_wnd != nullptr;
}

void TabFonts::on_family_change(bool skip_face_change)
{
    const auto index = ComboBox_GetCurSel(m_font_family_combobox);
    m_font_family = index != CB_ERR ? std::make_optional(std::reference_wrapper(m_font_families[index])) : std::nullopt;

    const auto previous_font_face_name
        = m_font_face ? std::make_optional(m_font_face->get().localised_name) : std::nullopt;
    ComboBox_ResetContent(m_font_face_combobox);
    m_font_face.reset();
    m_font_faces.clear();
    m_font_faces_text_formats.clear();

    EnableWindow(GetDlgItem(m_wnd, IDC_CONFIGURE_AXES), m_font_family && !m_font_family->get().axes.empty());

    if (!m_font_family)
        return;

    try {
        m_font_faces = m_font_family->get().fonts();
        m_font_faces_text_formats.resize(m_font_faces.size());

        for (auto&& font : m_font_faces)
            ComboBox_AddString(m_font_face_combobox, font.localised_name.c_str());

        if (!m_font_family->get().axes.empty())
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

    on_face_change();
}

void TabFonts::on_face_change()
{
    const auto index = ComboBox_GetCurSel(m_font_face_combobox);

    if (index == CB_ERR)
        m_font_face.reset();
    else if (gsl::narrow_cast<size_t>(index) < m_font_faces.size())
        m_font_face = std::ref(m_font_faces[index]);
    else if (!m_font_face && !m_font_faces.empty())
        m_font_face = m_font_faces[0];
}

void TabFonts::update_font_size_edit()
{
    const auto font_size_tenths = get_current_font_size_tenths();
    const auto font_size_float = gsl::narrow_cast<float>(font_size_tenths) / 10.0f;
    const auto font_size_text = fmt::format(L"{:.01f}", font_size_float);

    m_is_updating_font_size_edit = true;
    auto _ = gsl::finally([this] { m_is_updating_font_size_edit = false; });
    SetWindowText(m_font_size_edit, font_size_text.c_str());
}

void TabFonts::update_font_size_spin() const
{
    const auto font_size_tenths = get_current_font_size_tenths();
    SendMessage(m_font_size_spin, UDM_SETPOS32, 0, font_size_tenths);
}

void TabFonts::save_font_face() const
{
    if (!m_font_face || !m_direct_write_context)
        return;

    auto& font_description = m_element_ptr->font_description;

    uih::direct_write::WeightStretchStyle wss;
    wss.family_name = m_font_family->get().wss_name;
    wss.weight = m_font_face->get().weight;
    wss.stretch = m_font_face->get().stretch;
    wss.style = m_font_face->get().style;

    font_description.wss = wss;
    font_description.typographic_family_name = m_font_family->get().typographic_name;
    font_description.axis_values = m_font_face->get().axis_values;
    font_description.log_font = m_direct_write_context->create_log_font(m_font_face->get().font);
    font_description.recalculate_log_font_height();
}

void TabFonts::save_size_edit() const
{
    const auto font_size_text = uih::get_window_text(m_font_size_edit);

    const auto font_size_float = cui::string::safe_stof(font_size_text);

    if (!font_size_float)
        return;

    auto& font_description = m_element_ptr->font_description;

    font_description.set_point_size(*font_size_float);
}

const wchar_t* TabFonts::get_font_face_combobox_item_text(uint32_t index) const
{
    if (index < m_font_faces.size())
        return m_font_faces[index].localised_name.c_str();

    return custom_axis_values_label;
}

wil::com_ptr_t<IDWriteFontFamily> TabFonts::get_icon_font_family() const
{
    LOGFONT log_font{};
    THROW_IF_WIN32_BOOL_FALSE(SystemParametersInfo(SPI_GETICONTITLELOGFONT, 0, &log_font, 0));

    const auto font = m_direct_write_context->create_font(log_font);

    wil::com_ptr_t<IDWriteFontFamily> font_family;
    THROW_IF_FAILED(font->GetFontFamily(&font_family));

    return font_family;
}

uih::direct_write::TextFormat& TabFonts::get_family_text_format(size_t index)
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

uih::direct_write::TextFormat& TabFonts::get_face_text_format(size_t index)
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

bool TabFonts::get_help_url(pfc::string_base& p_out)
{
    p_out = "http://yuo.be/wiki/columns_ui:config:colours_and_fonts:fonts";
    return true;
}

const char* TabFonts::get_name()
{
    return "Fonts";
}

HWND TabFonts::create(HWND wnd)
{
    return m_helper.create(
        wnd, IDD_PREFS_FONTS, [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
}

INT_PTR TabFonts::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_mode_combobox = GetDlgItem(wnd, IDC_FONT_MODE);
        m_element_combobox = GetDlgItem(wnd, IDC_FONT_ELEMENT);
        m_font_family_combobox = GetDlgItem(wnd, IDC_FONT_FAMILY);
        m_font_face_combobox = GetDlgItem(wnd, IDC_FONT_FACE);
        m_font_size_edit = GetDlgItem(wnd, IDC_FONT_SIZE);
        m_font_size_spin = GetDlgItem(wnd, IDC_FONT_SIZE_SPIN);

        SendMessage(m_font_family_combobox, CB_SETITEMHEIGHT, -1, 14_spx);
        SendMessage(m_font_face_combobox, CB_SETITEMHEIGHT, -1, 14_spx);

        FontsClientList::g_get_list(m_fonts_client_list);

        ComboBox_AddString(m_element_combobox, L"Common (list items)");
        ComboBox_AddString(m_element_combobox, L"Common (labels)");

        for (auto&& client : m_fonts_client_list)
            ComboBox_AddString(m_element_combobox, mmh::to_utf16(mmh::to_string_view(client.m_name)).c_str());

        ComboBox_SetCurSel(m_element_combobox, 0);
        m_element_ptr = g_font_manager_data.m_common_items_entry;

        SendMessage(m_font_size_spin, UDM_SETRANGE32, 10, 720);

        try {
            m_direct_write_context = uih::direct_write::Context::s_create();
            m_font_families = m_direct_write_context->get_font_families();
        }
        CATCH_LOG()

        m_font_families_text_formats.resize(m_font_families.size());
        for (auto&& family : m_font_families)
            ComboBox_AddString(m_font_family_combobox, family.display_name().c_str());

        update_mode_combobox();
        enable_or_disable_font_selection();

        if (m_font_families.empty()) {
            uih::InfoBox::g_run(
                wnd, "Error initialising fonts list", "There was an error listing fonts using DirectWrite.", OIC_ERROR);
            break;
        }

        restore_font_selection_state();
        break;
    }
    case WM_DESTROY: {
        m_fonts_client_list.remove_all();
        m_element_ptr.reset();
        m_element_api.release();
        m_font_family.reset();
        m_font_face.reset();
        m_font_families.clear();
        m_font_faces.clear();
        m_direct_write_context.reset();
        m_font_families_text_formats.clear();
        m_font_faces_text_formats.clear();
        m_wnd = nullptr;
        m_element_combobox = nullptr;
        m_mode_combobox = nullptr;
        m_font_family_combobox = nullptr;
        m_font_face_combobox = nullptr;
        m_font_size_edit = nullptr;
        m_font_size_spin = nullptr;
        break;
    }
    case WM_MEASUREITEM: {
        const auto mis = reinterpret_cast<LPMEASUREITEMSTRUCT>(lp);

        if (!m_direct_write_context || mis->CtlType != ODT_COMBOBOX
            || (mis->CtlID != IDC_FONT_FAMILY && mis->CtlID != IDC_FONT_FACE))
            break;

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
    case WM_DRAWITEM: {
        const auto dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lp);

        if (const auto result = handle_wm_drawitem(dis); result)
            return *result;

        break;
    }
    case WM_NOTIFY: {
        const auto nmhdr = reinterpret_cast<LPNMHDR>(lp);
        switch (nmhdr->idFrom) {
        case IDC_FONT_SIZE_SPIN:
            switch (nmhdr->code) {
            case UDN_DELTAPOS: {
                const auto nmupdown = reinterpret_cast<LPNMUPDOWN>(lp);
                nmupdown->iDelta *= 10;
                const auto new_font_size_tenths = std::clamp(nmupdown->iPos + nmupdown->iDelta, 10, 720);
                m_element_ptr->font_description.set_point_size_tenths(new_font_size_tenths);
                update_font_size_edit();
                on_font_changed();
                return 0;
            }
            }
            break;
        }
        break;
    }
    case WM_COMMAND:
        switch (wp) {
        case IDC_CONFIGURE_AXES: {
            if (!m_font_family || !m_font_face || !m_element_ptr)
                break;

            auto& font_description = m_element_ptr->font_description;
            const auto& axis_values
                = font_description.axis_values.empty() ? m_font_face->get().axis_values : font_description.axis_values;

            const auto axes_configured = ConfigureAxesDialog::g_create(wnd, *m_font_family, *m_font_face, axis_values,
                [&, this, _element{m_element_ptr}](const auto& new_axis_values) {
                    font_description.axis_values = new_axis_values;

                    const auto wss_and_log_font = m_direct_write_context->get_wss_and_logfont_for_axis_values(
                        font_description.typographic_family_name.c_str(), new_axis_values);

                    if (wss_and_log_font) {
                        font_description.wss = std::get<uih::direct_write::WeightStretchStyle>(*wss_and_log_font);
                        font_description.log_font = std::get<LOGFONT>(*wss_and_log_font);
                    }

                    on_font_changed();
                });

            if (axes_configured)
                ComboBox_SetCurSel(m_font_face_combobox, m_font_faces.size());

            break;
        }
        case IDC_FONT_MODE | (CBN_SELCHANGE << 16): {
            const int idx = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            m_element_ptr->font_mode
                = static_cast<cui::fonts::font_mode_t>(ComboBox_GetItemData(reinterpret_cast<HWND>(lp), idx));
            restore_font_selection_state();
            enable_or_disable_font_selection();
            on_font_changed();
            break;
        }
        case IDC_FONT_ELEMENT | (CBN_SELCHANGE << 16): {
            const int idx = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            m_element_api.release();
            if (idx != -1) {
                if (idx == 0)
                    m_element_ptr = g_font_manager_data.m_common_items_entry;
                else if (idx == 1)
                    m_element_ptr = g_font_manager_data.m_common_labels_entry;
                else if (idx >= 2) {
                    m_element_api = m_fonts_client_list[idx - 2].m_ptr;
                    m_element_ptr = g_font_manager_data.find_by_guid(m_fonts_client_list[idx - 2].m_guid);
                }
            }
            update_mode_combobox();
            restore_font_selection_state();
            enable_or_disable_font_selection();
            return 0;
        }
        case IDC_FONT_FAMILY | (CBN_SELCHANGE << 16): {
            if (!m_direct_write_context)
                break;

            on_family_change();
            save_font_face();
            on_font_changed();
            break;
        }
        case IDC_FONT_FACE | (CBN_SELCHANGE << 16): {
            if (!m_direct_write_context)
                break;

            on_face_change();
            save_font_face();
            on_font_changed();
            break;
        }
        case IDC_FONT_SIZE | (EN_CHANGE << 16): {
            if (m_is_updating_font_size_edit)
                break;

            const auto previous_size = m_element_ptr->font_description.point_size_tenths;
            save_size_edit();
            update_font_size_spin();

            if (m_element_ptr->font_description.point_size_tenths != previous_size)
                on_font_changed();

            break;
        }
        }
        break;
    }
    return 0;
}

std::optional<INT_PTR> TabFonts::handle_wm_drawitem(LPDRAWITEMSTRUCT dis)
{
    if (!m_direct_write_context || dis->CtlType != ODT_COMBOBOX
        || (dis->CtlID != IDC_FONT_FAMILY && dis->CtlID != IDC_FONT_FACE))
        return {};

    const auto is_family = dis->CtlID == IDC_FONT_FAMILY;
    const auto is_focused = (dis->itemState & ODS_FOCUS) != 0;
    const auto is_edit_box = (dis->itemState & ODS_COMBOBOXEDIT) != 0;
    const auto is_selected = (dis->itemState & ODS_SELECTED) != 0;
    const auto index = dis->itemID;
    const auto is_dark = cui::dark::is_active_ui_dark();
    const auto hide_focus = (SendMessage(dis->hwndItem, WM_QUERYUISTATE, NULL, NULL) & UISF_HIDEFOCUS) != 0;

    const auto background_colour = [&dis, is_dark, is_edit_box, is_selected] {
        if (is_edit_box && is_dark)
            return cui::dark::get_dark_colour(cui::dark::ColourID::ComboBoxEditBackground);

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
            return cui::dark::get_dark_colour(cui::dark::ColourID::ComboBoxEditText);

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

void TabFonts::on_font_changed()
{
    if (m_element_api.is_valid()) {
        m_element_api->on_font_changed();
        return;
    }

    const auto index_element = ComboBox_GetCurSel(m_element_combobox);
    if (index_element > 1)
        return;

    g_font_manager_data.g_on_common_font_changed(1 << index_element);

    for (auto&& client : m_fonts_client_list) {
        const auto p_data = g_font_manager_data.find_by_guid(client.m_guid);
        if (index_element == 0 && p_data->font_mode == cui::fonts::font_mode_common_items) {
            client.m_ptr->on_font_changed();
        } else if (index_element == 1 && p_data->font_mode == cui::fonts::font_mode_common_labels)
            client.m_ptr->on_font_changed();
    }
}

void TabFonts::restore_font_selection_state()
{
    if (!m_direct_write_context)
        return;

    update_font_size_edit();
    update_font_size_spin();

    const auto font_description = g_font_manager_data.resolve_font_description(m_element_ptr);
    const auto& wss = font_description.wss;

    if (!wss) {
        ComboBox_SetCurSel(m_font_family_combobox, -1);
        on_family_change();
        return;
    }

    const auto& typographic_family_name = font_description.typographic_family_name;

    const auto resolved_names = m_direct_write_context->resolve_font_names(wss->family_name.c_str(),
        typographic_family_name.c_str(), wss->weight, wss->stretch, wss->style, font_description.axis_values);

    if (!resolved_names) {
        return;
    }

    const auto& [family_name, face_name] = *resolved_names;

    const auto iter = ranges::find_if(
        m_font_families, [&family_name](auto&& family) { return family.display_name() == family_name; });

    ComboBox_SetCurSel(
        m_font_family_combobox, iter != std::end(m_font_families) ? std::distance(m_font_families.begin(), iter) : -1);
    on_family_change();

    std::optional<size_t> face_index;

    if (!face_name.empty()) {
        const auto face_iter
            = ranges::find_if(m_font_faces, [&face_name](auto&& face) { return face.localised_name == face_name; });
        face_index = std::distance(m_font_faces.begin(), face_iter);
    }

    if (face_index) {
        const auto& font_face = m_font_faces[*face_index];
        if (!font_face.axis_values.empty() && !font_description.axis_values.empty()
            && font_face.axis_values != font_description.axis_values)
            ComboBox_SetCurSel(m_font_face_combobox, m_font_faces.size());
        else
            ComboBox_SetCurSel(m_font_face_combobox, *face_index);
    } else
        ComboBox_SetCurSel(m_font_face_combobox, -1);

    on_face_change();
}

void TabFonts::enable_or_disable_font_selection() const
{
    const auto is_custom_mode = m_element_ptr->font_mode == cui::fonts::font_mode_custom;
    const auto enable = is_custom_mode && m_direct_write_context;

    EnableWindow(m_font_family_combobox, enable);
    EnableWindow(m_font_face_combobox, enable);
    EnableWindow(m_font_size_edit, enable);
    EnableWindow(m_font_size_spin, enable);
}

int TabFonts::get_current_font_size_tenths() const
{
    const auto font_description = g_font_manager_data.resolve_font_description(m_element_ptr);
    return font_description.point_size_tenths;
}

FontManagerData::entry_ptr_t TabFonts::get_current_resolved_entry() const
{
    auto entry = m_element_ptr;

    if (entry->font_mode == cui::fonts::font_mode_common_items)
        return g_font_manager_data.m_common_items_entry;

    if (entry->font_mode == cui::fonts::font_mode_common_labels)
        return g_font_manager_data.m_common_labels_entry;

    return entry;
}

void TabFonts::update_mode_combobox() const
{
    ComboBox_ResetContent(m_mode_combobox);
    size_t index;
    const size_t index_element = ComboBox_GetCurSel(m_element_combobox);
    if (index_element <= 1) {
        index = ComboBox_AddString(m_mode_combobox, L"System");
        ComboBox_SetItemData(m_mode_combobox, index, cui::fonts::font_mode_system);
    } else {
        index = ComboBox_AddString(m_mode_combobox, L"Common (list items)");
        ComboBox_SetItemData(m_mode_combobox, index, cui::fonts::font_mode_common_items);
        index = ComboBox_AddString(m_mode_combobox, L"Common (labels)");
        ComboBox_SetItemData(m_mode_combobox, index, cui::fonts::font_mode_common_labels);
    }
    index = ComboBox_AddString(m_mode_combobox, L"Custom");
    ComboBox_SetItemData(m_mode_combobox, index, cui::fonts::font_mode_custom);

    ComboBox_SetCurSel(m_mode_combobox, uih::combo_box_find_item_by_data(m_mode_combobox, m_element_ptr->font_mode));
}
