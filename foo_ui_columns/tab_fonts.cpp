#include "pch.h"

#include "tab_fonts.h"

#include "dark_mode.h"
#include "dark_mode_active_ui.h"

bool TabFonts::is_active() const
{
    return m_wnd != nullptr;
}

void TabFonts::on_family_change()
{
    const auto index = ComboBox_GetCurSel(m_font_family_combobox);
    m_font_family = index != CB_ERR ? std::make_optional(std::reference_wrapper(m_font_families[index])) : std::nullopt;

    const auto previous_font_face_name = m_font_face ? std::make_optional(m_font_face->localised_name) : std::nullopt;
    ComboBox_ResetContent(m_font_face_combobox);
    m_font_faces_text_formats.clear();

    if (!m_font_family)
        return;

    try {
        m_font_faces = m_font_family->get().fonts();
        m_font_faces_text_formats.resize(m_font_faces.size());

        for (auto&& font : m_font_faces)
            ComboBox_AddString(m_font_face_combobox, font.localised_name.c_str());
    }
    CATCH_LOG()

    bool font_face_found{};

    if (previous_font_face_name) {
        font_face_found = ComboBox_SelectString(m_font_face_combobox, -1, previous_font_face_name->c_str()) != CB_ERR;
    }

    if (!font_face_found) {
        ComboBox_SetCurSel(m_font_face_combobox, 0);
    }

    on_face_change();
}

void TabFonts::on_face_change()
{
    const auto index = ComboBox_GetCurSel(m_font_face_combobox);
    m_font_face = index != CB_ERR ? std::make_optional(m_font_faces[index]) : std::nullopt;
}

void TabFonts::update_font_size_edit()
{
    const auto font_size_tenths = get_current_font_size();
    const auto font_size_float = gsl::narrow_cast<float>(font_size_tenths) / 10.0f;
    const auto font_size_text = fmt::format(L"{:.01f}", font_size_float);

    m_is_updating_font_size_edit = true;
    auto _ = gsl::finally([this] { m_is_updating_font_size_edit = false; });
    SetWindowText(m_font_size_edit, font_size_text.c_str());
}

void TabFonts::update_font_size_spin() const
{
    const auto font_size_tenths = get_current_font_size();
    SendMessage(m_font_size_spin, UDM_SETPOS32, 0, font_size_tenths);
}

void TabFonts::save_font_face() const
{
    if (!m_font_face || !m_direct_write_context)
        return;

    auto& font_description = m_element_ptr->font_description;

    font_description.log_font = m_direct_write_context->create_log_font(m_font_face->font);
    font_description.recalculate_log_font_height();
}

void TabFonts::save_size_edit() const
{
    const auto font_size_text = uih::get_window_text(m_font_size_edit);
    std::optional<float> font_size_float;

    try {
        font_size_float = std::stof(font_size_text);
    } catch (const std::exception&) {
        return;
    }

    auto& font_description = m_element_ptr->font_description;

    font_description.point_size_tenths = gsl::narrow_cast<int>(std::roundf(*font_size_float * 10.0f));
    font_description.recalculate_log_font_height();
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
    const auto& [family, _, is_symbol_font] = m_font_families.at(index);
    auto& text_format = m_font_families_text_formats.at(index);

    if (!text_format) {
        text_format = m_direct_write_context->create_text_format(is_symbol_font ? get_icon_font_family() : family,
            DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            uih::direct_write::pt_to_dip(font_dropdown_font_size));
    }

    return *text_format;
}

uih::direct_write::TextFormat& TabFonts::get_face_text_format(size_t index)
{
    const auto& [family, _family_name, is_symbol_font] = m_font_family->get();
    const auto& [font, _face_name] = m_font_faces.at(index);
    auto& text_format = m_font_faces_text_formats.at(index);

    if (!text_format) {
        text_format = m_direct_write_context->create_text_format(is_symbol_font ? get_icon_font_family() : family,
            font->GetWeight(), font->GetStyle(), font->GetStretch(),
            uih::direct_write::pt_to_dip(font_dropdown_font_size));
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
            ComboBox_AddString(m_font_family_combobox, family.localised_name.c_str());

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
        const auto& text = is_family ? m_font_families[index].localised_name : m_font_faces[index].localised_name;

        try {
            const auto& text_format = is_family ? get_family_text_format(index) : get_face_text_format(index);
            mis->itemHeight = text_format.get_minimum_height(text) + 4_spx;
        }
        CATCH_LOG();

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
                m_element_ptr->font_description.point_size_tenths = new_font_size_tenths;
                m_element_ptr->font_description.recalculate_log_font_height();
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

    if (index == -1) {
        if (is_focused && !hide_focus) {
            DrawFocusRect(buffered_dc.get(), &dis->rcItem);
        }
        return {};
    }

    const auto text_colour = [&dis, is_dark, is_edit_box, is_selected] {
        if (is_edit_box && is_dark)
            return cui::dark::get_dark_colour(cui::dark::ColourID::ComboBoxEditText);

        if (is_selected)
            return GetSysColor(COLOR_HIGHLIGHTTEXT);

        return GetTextColor(dis->hDC);
    }();

    auto& text = is_family ? m_font_families[index].localised_name : m_font_faces[index].localised_name;

    if (is_edit_box) {
        auto _ = wil::SelectObject(buffered_dc.get(), GetWindowFont(dis->hwndItem));
        SetBkMode(buffered_dc.get(), TRANSPARENT);
        SetTextColor(buffered_dc.get(), text_colour);
        DrawTextEx(buffered_dc.get(), text.data(), gsl::narrow<int>(text.length()), &dis->rcItem,
            DT_SINGLELINE | DT_VCENTER, nullptr);
        return {};
    }

    const auto max_width = uih::direct_write::px_to_dip(gsl::narrow_cast<float>(wil::rect_width(dis->rcItem)));
    const auto max_height = uih::direct_write::px_to_dip(gsl::narrow_cast<float>(wil::rect_height(dis->rcItem)));

    try {
        const auto& text_format = is_family ? get_family_text_format(index) : get_face_text_format(index);
        const auto text_layout = text_format.create_text_layout(text, max_width, max_height);
        text_layout.render(buffered_dc.get(), dis->rcItem, text_colour);
    }
    CATCH_LOG()

    if (is_focused && !hide_focus) {
        DrawFocusRect(buffered_dc.get(), &dis->rcItem);
    }

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

    const auto log_font = get_current_log_font();

    try {
        const auto font = m_direct_write_context->create_font(log_font);

        wil::com_ptr_t<IDWriteFontFamily> family;
        THROW_IF_FAILED(font->GetFontFamily(&family));

        wil::com_ptr_t<IDWriteLocalizedStrings> localised_family_names;
        THROW_IF_FAILED(family->GetFamilyNames(&localised_family_names));

        wil::com_ptr_t<IDWriteLocalizedStrings> localised_face_names;
        THROW_IF_FAILED(font->GetFaceNames(&localised_face_names));

        const auto family_name = uih::direct_write::get_localised_string(localised_family_names);
        const auto face_name = uih::direct_write::get_localised_string(localised_face_names);

        ComboBox_SelectString(m_font_family_combobox, -1, family_name.c_str());
        on_family_change();
        ComboBox_SelectString(m_font_face_combobox, -1, face_name.c_str());
    }
    CATCH_LOG()
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

LOGFONT TabFonts::get_current_log_font() const
{
    LOGFONT log_font{};

    const size_t index_element = ComboBox_GetCurSel(m_element_combobox);
    if (index_element <= 1)
        fb2k::std_api_get<cui::fonts::manager>()->get_font(
            static_cast<cui::fonts::font_type_t>(index_element), log_font);
    else
        fb2k::std_api_get<cui::fonts::manager>()->get_font(m_element_api->get_client_guid(), log_font);

    return log_font;
}

int TabFonts::get_current_font_size() const
{
    auto entry = m_element_ptr;
    const auto is_common_items = entry->font_mode == cui::fonts::font_mode_common_items;
    const auto is_common_labels = entry->font_mode == cui::fonts::font_mode_common_labels;

    const auto resolved_entry = [&] {
        if (is_common_items)
            return g_font_manager_data.m_common_items_entry;

        if (is_common_labels)
            return g_font_manager_data.m_common_labels_entry;

        return entry;
    }();

    if (resolved_entry->font_mode == cui::fonts::font_mode_system) {
        const auto system_font = is_common_items ? cui::fonts::get_items_font_for_dpi(USER_DEFAULT_SCREEN_DPI)
                                                 : cui::fonts::get_labels_font_for_dpi(USER_DEFAULT_SCREEN_DPI);

        const auto point_size = system_font.size * 72.0f / gsl::narrow_cast<float>(USER_DEFAULT_SCREEN_DPI);
        return gsl::narrow_cast<int>(std::roundf(point_size * 10.0f));
    }

    return entry->font_description.point_size_tenths;
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
