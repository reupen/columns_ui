#include "pch.h"

#include "tab_fonts.h"

namespace cui::prefs {

namespace {

cfg_guid last_selected_element_id{{0x467c3e73, 0x80ff, 0x4ded, {0xa8, 0x27, 0x57, 0x3c, 0xfa, 0x39, 0x1b, 0x8f}}, {}};

}

bool TabFonts::is_active() const
{
    return m_wnd != nullptr;
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
    if (m_direct_write_font_picker)
        if (const auto result = m_direct_write_font_picker->handle_message(wnd, msg, wp, lp); result)
            return *result;

    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_mode_combobox = GetDlgItem(wnd, IDC_FONT_MODE);
        m_element_combobox = GetDlgItem(wnd, IDC_FONT_ELEMENT);

        FontsClientList::g_get_list(m_fonts_client_list);

        ComboBox_AddString(m_element_combobox, L"Common (list items)");
        ComboBox_AddString(m_element_combobox, L"Common (labels)");

        std::optional<int> last_selected_element_index;

        for (auto&& client : m_fonts_client_list) {
            const auto index
                = ComboBox_AddString(m_element_combobox, mmh::to_utf16(mmh::to_string_view(client.m_name)).c_str());

            if (last_selected_element_id != GUID{} && last_selected_element_id == client.m_guid && index != CB_ERR)
                last_selected_element_index = index;
        }

        if (last_selected_element_index)
            ComboBox_SetCurSel(m_element_combobox, *last_selected_element_index);
        else if (last_selected_element_id == fonts::labels_font_id)
            ComboBox_SetCurSel(m_element_combobox, 1);
        else
            ComboBox_SetCurSel(m_element_combobox, 0);

        m_direct_write_font_picker = utils::DirectWriteFontPicker(false);
        m_direct_write_font_picker->on_font_changed([this](auto& font_description) {
            m_element_ptr->font_description = font_description;
            on_font_changed();
        });
        m_direct_write_font_picker->handle_message(wnd, msg, wp, lp);

        on_element_selected();
        break;
    }
    case WM_DESTROY: {
        m_direct_write_font_picker.reset();
        m_fonts_client_list.remove_all();
        m_element_ptr.reset();
        m_element_api.release();
        m_wnd = nullptr;
        m_element_combobox = nullptr;
        m_mode_combobox = nullptr;
        break;
    }
    case WM_COMMAND:
        switch (wp) {
        case IDC_FONT_MODE | (CBN_SELCHANGE << 16): {
            const int idx = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            m_element_ptr->font_mode
                = static_cast<fonts::FontMode>(ComboBox_GetItemData(reinterpret_cast<HWND>(lp), idx));
            restore_font_selection_state();
            enable_or_disable_font_selection();
            on_font_changed();
            break;
        }
        case IDC_FONT_ELEMENT | (CBN_SELCHANGE << 16):
            on_element_selected();
            last_selected_element_id = m_element_ptr ? m_element_ptr->guid : GUID{};
            return 0;
        }
        break;
    }
    return 0;
}

void TabFonts::on_font_changed()
{
    if (m_element_api.is_valid()) {
        g_font_manager_data.dispatch_client_font_changed(m_element_api);
        return;
    }

    const auto index_element = ComboBox_GetCurSel(m_element_combobox);
    if (index_element > 1)
        return;

    g_font_manager_data.dispatch_common_font_changed(1 << index_element);

    for (auto&& client : m_fonts_client_list) {
        const auto p_data = g_font_manager_data.find_by_id(client.m_guid);
        if ((index_element == 0 && p_data->font_mode == fonts::FontMode::CommonItems)
            || (index_element == 1 && p_data->font_mode == fonts::FontMode::CommonLabels))
            g_font_manager_data.dispatch_client_font_changed(client.m_ptr);
    }
}

void TabFonts::on_element_selected()
{
    m_element_api.reset();
    m_element_ptr.reset();

    if (const auto index = ComboBox_GetCurSel(m_element_combobox); index != CB_ERR) {
        if (index == 0)
            m_element_ptr = g_font_manager_data.m_common_items_entry;
        else if (index == 1)
            m_element_ptr = g_font_manager_data.m_common_labels_entry;
        else if (index >= 2) {
            m_element_api = m_fonts_client_list[index - 2].m_ptr;
            m_element_ptr = g_font_manager_data.find_by_id(m_fonts_client_list[index - 2].m_guid);
        }
    }

    update_mode_combobox();
    restore_font_selection_state();
    enable_or_disable_font_selection();
}

void TabFonts::restore_font_selection_state()
{
    const auto font_description = g_font_manager_data.resolve_font_description(m_element_ptr);
    m_direct_write_font_picker->set_font_description(std::move(font_description));
}

void TabFonts::enable_or_disable_font_selection() const
{
    const auto is_custom_mode = m_element_ptr->font_mode == fonts::FontMode::Custom;
    m_direct_write_font_picker->set_font_selection_allowed(is_custom_mode);
}

FontManagerData::entry_ptr_t TabFonts::get_current_resolved_entry() const
{
    auto entry = m_element_ptr;

    if (entry->font_mode == fonts::FontMode::CommonItems)
        return g_font_manager_data.m_common_items_entry;

    if (entry->font_mode == fonts::FontMode::CommonLabels)
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
        ComboBox_SetItemData(m_mode_combobox, index, cui::fonts::FontMode::System);
    } else {
        index = ComboBox_AddString(m_mode_combobox, L"Common (list items)");
        ComboBox_SetItemData(m_mode_combobox, index, cui::fonts::FontMode::CommonItems);
        index = ComboBox_AddString(m_mode_combobox, L"Common (labels)");
        ComboBox_SetItemData(m_mode_combobox, index, cui::fonts::FontMode::CommonLabels);
    }
    index = ComboBox_AddString(m_mode_combobox, L"Custom");
    ComboBox_SetItemData(m_mode_combobox, index, cui::fonts::FontMode::Custom);

    ComboBox_SetCurSel(
        m_mode_combobox, uih::combo_box_find_item_by_data(m_mode_combobox, WI_EnumValue(m_element_ptr->font_mode)));
}

} // namespace cui::prefs
