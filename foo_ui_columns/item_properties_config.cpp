#include "pch.h"

#include "dark_mode_dialog.h"
#include "item_properties.h"

namespace cui::panels::item_properties {

INT_PTR CALLBACK ItemPropertiesConfig::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        pfc::vartoggle_t<bool> init(m_initialising, true);

        HWND wnd_fields = m_field_list.create(wnd, uih::WindowPosition(14, 17, 240, 150), true);
        SetWindowPos(wnd_fields, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        ShowWindow(wnd_fields, SW_SHOWNORMAL);

        const auto wnd_sections_tree = GetDlgItem(wnd, IDC_INFOSECTIONS);

        const auto current_styles = GetWindowLongPtr(wnd_sections_tree, GWL_STYLE);
        SetWindowLongPtr(wnd_sections_tree, GWL_STYLE, current_styles | TVS_CHECKBOXES);

        for (auto&& [index, section] : ranges::views::reverse(ranges::views::enumerate(g_info_sections))) {
            const auto is_enabled = (m_info_sections_mask & 1 << section.id) != 0;
            pfc::stringcvt::string_wide_from_utf8 utf16_name(section.name);

            TVINSERTSTRUCT tvis{};
            tvis.hParent = TVI_ROOT;
            tvis.hInsertAfter = TVI_FIRST;
            tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
            tvis.item.lParam = gsl::narrow<LPARAM>(index);
            tvis.item.pszText = const_cast<wchar_t*>(utf16_name.get_ptr());
            tvis.item.stateMask = TVIS_STATEIMAGEMASK;
            tvis.item.state = INDEXTOSTATEIMAGEMASK(is_enabled ? 2 : 1);

            TreeView_InsertItem(wnd_sections_tree, &tvis);
        }

        HWND wnd_combo = GetDlgItem(wnd, IDC_EDGESTYLE);
        ComboBox_AddString(wnd_combo, L"None");
        ComboBox_AddString(wnd_combo, L"Sunken");
        ComboBox_AddString(wnd_combo, L"Grey");
        ComboBox_SetCurSel(wnd_combo, m_edge_style);

        Button_SetCheck(GetDlgItem(wnd, IDC_SHOWCOLUMNS), m_show_columns ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(wnd, IDC_SHOWGROUPS), m_show_groups ? BST_CHECKED : BST_UNCHECKED);
    } break;
    case WM_DESTROY: {
        m_field_list.destroy();
    } break;
    case WM_ERASEBKGND:
        SetWindowLongPtr(wnd, DWLP_MSGRESULT, TRUE);
        return TRUE;
    case WM_PAINT:
        uih::handle_modern_background_paint(wnd, GetDlgItem(wnd, IDOK));
        return TRUE;
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
        SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
        return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
    case WM_NOTIFY: {
        const auto lpnm = reinterpret_cast<LPNMHDR>(lp);

        switch (lpnm->idFrom) {
        case IDC_INFOSECTIONS:
            switch (lpnm->code) {
            case TVN_ITEMCHANGED: {
                auto lpnmtvic = reinterpret_cast<NMTVITEMCHANGE*>(lp);

                if (m_initialising || lpnmtvic->lParam < 0 || lpnmtvic->lParam >= std::ssize(g_info_sections))
                    break;

                auto& section = g_info_sections[lpnmtvic->lParam];

                if (lpnmtvic->uStateNew & INDEXTOSTATEIMAGEMASK(2))
                    m_info_sections_mask |= (1 << section.id);
                else
                    m_info_sections_mask &= ~(1 << section.id);
                break;
            }
            }
            break;
        }
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDOK:
            EndDialog(wnd, 1);
            break;
        case IDCANCEL:
            EndDialog(wnd, 0);
            break;
        case IDC_SHOWCOLUMNS:
            m_show_columns = Button_GetCheck((HWND)lp) != 0;
            break;
        case IDC_SHOWGROUPS:
            m_show_groups = Button_GetCheck((HWND)lp) != 0;
            break;
        case IDC_EDGESTYLE:
            switch (HIWORD(wp)) {
            case CBN_SELCHANGE:
                m_edge_style = ComboBox_GetCurSel((HWND)lp);
                break;
            }
            break;
        case IDC_UP: {
            if (m_field_list.get_selection_count(2) == 1) {
                size_t index = 0;
                size_t count = m_field_list.get_item_count();
                while (!m_field_list.get_item_selected(index) && index < count)
                    index++;
                if (index && m_fields.get_count()) {
                    m_fields.swap_items(index, index - 1);

                    pfc::list_t<uih::ListView::InsertItem> items;
                    m_field_list.get_insert_items(index - 1, 2, items);
                    m_field_list.replace_items(index - 1, items);
                    m_field_list.set_item_selected_single(index - 1);
                }
            }
        } break;
        case IDC_DOWN: {
            if (m_field_list.get_selection_count(2) == 1) {
                size_t index = 0;
                size_t count = m_field_list.get_item_count();
                while (!m_field_list.get_item_selected(index) && index < count)
                    index++;
                if (index + 1 < count && index + 1 < m_fields.get_count()) {
                    m_fields.swap_items(index, index + 1);

                    pfc::list_t<uih::ListView::InsertItem> items;
                    m_field_list.get_insert_items(index, 2, items);
                    m_field_list.replace_items(index, items);
                    m_field_list.set_item_selected_single(index + 1);
                }
            }
        } break;
        case IDC_NEW: {
            Field temp;
            temp.m_name_friendly = "<enter name here>";
            temp.m_name = "<ENTER FIELD HERE>";
            size_t index = m_fields.add_item(temp);

            pfc::list_t<uih::ListView::InsertItem> items;
            m_field_list.get_insert_items(index, 1, items);
            m_field_list.insert_items(index, 1, items.get_ptr());
            m_field_list.set_item_selected_single(index);
            SetFocus(m_field_list.get_wnd());
            m_field_list.activate_inline_editing();

        } break;
        case IDC_REMOVE: {
            if (m_field_list.get_selection_count(2) == 1) {
                bit_array_bittable mask(m_field_list.get_item_count());
                m_field_list.get_selection_state(mask);
                // bool b_found = false;
                size_t index = 0;
                size_t count = m_field_list.get_item_count();
                while (index < count) {
                    if (mask[index])
                        break;
                    index++;
                }
                if (index < count && index < m_fields.get_count()) {
                    m_fields.remove_by_idx(index);
                    m_field_list.remove_item(index);
                    size_t new_count = m_field_list.get_item_count();
                    if (new_count) {
                        if (index < new_count)
                            m_field_list.set_item_selected_single(index);
                        else if (index)
                            m_field_list.set_item_selected_single(index - 1);
                    }
                }
            }
        } break;
        }
        break;
    }
    return FALSE;
}

bool ItemPropertiesConfig::run_modal(HWND wnd)
{
    const dark::DialogDarkModeConfig dark_mode_config{
        .button_ids = {IDC_NEW, IDC_REMOVE, IDC_UP, IDC_DOWN, IDOK, IDCANCEL},
        .checkbox_ids = {IDC_SHOWCOLUMNS, IDC_SHOWGROUPS},
        .combo_box_ids = {IDC_EDGESTYLE},
        .tree_view_ids = {IDC_INFOSECTIONS}};
    const auto dialog_result = modal_dialog_box(IDD_ITEM_PROPS_OPTIONS, dark_mode_config, wnd,
        [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    return dialog_result > 0;
}

ItemPropertiesConfig::ItemPropertiesConfig(pfc::list_t<Field> p_fields, uint32_t edge_style,
    uint32_t info_sections_mask, bool b_show_columns, bool b_show_groups)
    : m_fields(std::move(p_fields))
    , m_edge_style(edge_style)
    , m_info_sections_mask(info_sections_mask)
    , m_show_columns(b_show_columns)
    , m_show_groups(b_show_groups)
    , m_initialising(false)
    , m_field_list(m_fields)
{
}

} // namespace cui::panels::item_properties
