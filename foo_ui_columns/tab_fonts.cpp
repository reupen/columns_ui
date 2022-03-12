#include "pch.h"

#include "tab_fonts.h"

namespace {

std::string format_font_description(const LOGFONT& lf)
{
    const auto dpi = uih::get_system_dpi_cached().cy;
    const auto pt = -MulDiv(lf.lfHeight, 72, dpi);
    const auto face = pfc::stringcvt::string_utf8_from_wide(lf.lfFaceName, std::size(lf.lfFaceName));

    return fmt::format(
        "{} {}pt{}{}", face.get_ptr(), pt, lf.lfWeight == FW_BOLD ? " bold"sv : ""sv, lf.lfItalic ? " italic"sv : ""sv);
}

} // namespace

bool TabFonts::is_active()
{
    return m_wnd != nullptr;
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

void TabFonts::apply() {}

INT_PTR TabFonts::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_wnd_colours_mode = GetDlgItem(wnd, IDC_COLOURS_MODE);
        m_wnd_colours_element = GetDlgItem(wnd, IDC_COLOURS_ELEMENT);

        FontsClientList::g_get_list(m_fonts_client_list);

        ComboBox_AddString(m_wnd_colours_element, L"Common (list items)");
        ComboBox_AddString(m_wnd_colours_element, L"Common (labels)");

        size_t count = m_fonts_client_list.get_count();
        for (size_t i = 0; i < count; i++)
            ComboBox_AddString(
                m_wnd_colours_element, pfc::stringcvt::string_os_from_utf8(m_fonts_client_list[i].m_name));

        ComboBox_SetCurSel(m_wnd_colours_element, 0);
        m_element_ptr = g_font_manager_data.m_common_items_entry;

        update_mode_combobox();
        update_font_desc();
        update_change();

        refresh_me(wnd);
    } break;
    case WM_DESTROY: {
        m_wnd = nullptr;
        m_fonts_client_list.remove_all();
        m_element_ptr.reset();
        m_element_api.release();
    } break;
    case WM_COMMAND:
        switch (wp) {
        case IDC_CHANGE_FONT: {
            LOGFONT lf;
            get_font(lf);

            if (auto font_description = cui::fonts::select_font(wnd, lf); font_description) {
                m_element_ptr->font_description = *font_description;
                update_font_desc();
                on_font_changed();
            }
        } break;
        case IDC_COLOURS_MODE | (CBN_SELCHANGE << 16): {
            int idx = ComboBox_GetCurSel((HWND)lp);
            m_element_ptr->font_mode = (cui::fonts::font_mode_t)ComboBox_GetItemData((HWND)lp, idx);
            update_font_desc();
            update_change();
            on_font_changed();
        } break;
        case IDC_COLOURS_ELEMENT | (CBN_SELCHANGE << 16): {
            int idx = ComboBox_GetCurSel((HWND)lp);
            m_element_api.release();
            if (idx != -1) {
                if (idx == 0)
                    m_element_ptr = g_font_manager_data.m_common_items_entry;
                else if (idx == 1)
                    m_element_ptr = g_font_manager_data.m_common_labels_entry;
                else if (idx >= 2) {
                    m_element_api = m_fonts_client_list[idx - 2].m_ptr;
                    g_font_manager_data.find_by_guid(m_fonts_client_list[idx - 2].m_guid, m_element_ptr);
                }
            }
            update_mode_combobox();
            update_font_desc();
            update_change();
        }
            return 0;
        }
        break;
    }
    return 0;
}

void TabFonts::on_font_changed()
{
    if (m_element_api.is_valid())
        m_element_api->on_font_changed();
    else {
        const auto index_element = ComboBox_GetCurSel(m_wnd_colours_element);
        if (index_element <= 1) {
            g_font_manager_data.g_on_common_font_changed(1 << index_element);
            size_t count = m_fonts_client_list.get_count();
            for (size_t i = 0; i < count; i++) {
                FontManagerData::entry_ptr_t p_data;
                g_font_manager_data.find_by_guid(m_fonts_client_list[i].m_guid, p_data);
                if (index_element == 0 && p_data->font_mode == cui::fonts::font_mode_common_items) {
                    m_fonts_client_list[i].m_ptr->on_font_changed();
                } else if (index_element == 1 && p_data->font_mode == cui::fonts::font_mode_common_labels)
                    m_fonts_client_list[i].m_ptr->on_font_changed();
            }
        }
    }
}

void TabFonts::update_font_desc()
{
    LOGFONT lf;
    get_font(lf);
    uSetWindowText(GetDlgItem(m_wnd, IDC_FONT_DESC), format_font_description(lf).c_str());
}

void TabFonts::update_change()
{
    EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_FONT), m_element_ptr->font_mode == cui::fonts::font_mode_custom);
}

void TabFonts::get_font(LOGFONT& lf)
{
    size_t index_element = ComboBox_GetCurSel(m_wnd_colours_element);
    if (index_element <= 1)
        fb2k::std_api_get<cui::fonts::manager>()->get_font(cui::fonts::font_type_t(index_element), lf);
    else
        fb2k::std_api_get<cui::fonts::manager>()->get_font(m_element_api->get_client_guid(), lf);
}

void TabFonts::update_mode_combobox()
{
    ComboBox_ResetContent(m_wnd_colours_mode);
    size_t index;
    size_t index_element = ComboBox_GetCurSel(m_wnd_colours_element);
    if (index_element <= 1) {
        index = ComboBox_AddString(m_wnd_colours_mode, L"System");
        ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_system);
    } else {
        index = ComboBox_AddString(m_wnd_colours_mode, L"Common (list items)");
        ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_common_items);
        index = ComboBox_AddString(m_wnd_colours_mode, L"Common (labels)");
        ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_common_labels);
    }
    index = ComboBox_AddString(m_wnd_colours_mode, L"Custom");
    ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_custom);

    ComboBox_SetCurSel(
        m_wnd_colours_mode, uih::combo_box_find_item_by_data(m_wnd_colours_mode, m_element_ptr->font_mode));
}

void TabFonts::refresh_me(HWND wnd)
{
    initialising = true;
    initialising = false;
}
