#include "stdafx.h"
#include "item_properties.h"

namespace cui::panels::item_properties {

INT_PTR CALLBACK ItemPropertiesConfig::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        pfc::vartoggle_t<bool> init(m_initialising, true);

        HWND wnd_fields = m_field_list.create(wnd, uih::WindowPosition(21, 17, 226, 150), true);
        SetWindowPos(wnd_fields, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        ShowWindow(wnd_fields, SW_SHOWNORMAL);

        HWND wnd_lv = GetDlgItem(wnd, IDC_INFOSECTIONS);
        ListView_SetExtendedListViewStyleEx(wnd_lv, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
        uih::list_view_set_explorer_theme(wnd_lv);

        RECT rc;
        GetClientRect(wnd_lv, &rc);
        uih::list_view_insert_column_text(wnd_lv, 0, L"", RECT_CX(rc));

        for (auto&& [index, section] : ranges::views::enumerate(g_info_sections)) {
            uih::list_view_insert_item_text(wnd_lv, index, 0, section.name);
            ListView_SetCheckState(wnd_lv, index, (m_info_sections_mask & (1 << section.id)) ? TRUE : FALSE);
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
        auto lpnm = (LPNMHDR)lp;
        switch (lpnm->idFrom) {
        case IDC_INFOSECTIONS:
            switch (lpnm->code) {
            case LVN_ITEMCHANGED: {
                auto lpnmlv = (LPNMLISTVIEW)lp;
                if (!m_initialising && lpnmlv->iItem < gsl::narrow<int>(g_info_sections.size())
                    && (lpnmlv->uChanged & LVIF_STATE)) {
                    m_info_sections_mask = m_info_sections_mask & ~(1 << g_info_sections[lpnmlv->iItem].id);

                    // if (((((UINT)(lpnmlv->uNewState & LVIS_STATEIMAGEMASK )) >> 12) -1))
                    if (ListView_GetCheckState(lpnm->hwndFrom, lpnmlv->iItem))
                        m_info_sections_mask = m_info_sections_mask | (1 << g_info_sections[lpnmlv->iItem].id);
                }
            } break;
            }
            break;
        }
    } break;
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
                t_size index = 0;
                t_size count = m_field_list.get_item_count();
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
                t_size index = 0;
                t_size count = m_field_list.get_item_count();
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
            t_size index = m_fields.add_item(temp);

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
                t_size index = 0;
                t_size count = m_field_list.get_item_count();
                while (index < count) {
                    if (mask[index])
                        break;
                    index++;
                }
                if (index < count && index < m_fields.get_count()) {
                    m_fields.remove_by_idx(index);
                    m_field_list.remove_item(index);
                    t_size new_count = m_field_list.get_item_count();
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
    const auto dialog_result = DialogBoxParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_ITEM_PROPS_OPTIONS), wnd,
        g_DialogProc, reinterpret_cast<LPARAM>(this));
    return dialog_result > 0;
}

ItemPropertiesConfig::ItemPropertiesConfig(pfc::list_t<Field> p_fields, t_size edge_style, t_uint32 info_sections_mask,
    bool b_show_columns, bool b_show_groups)
    : m_fields(std::move(p_fields))
    , m_edge_style(edge_style)
    , m_info_sections_mask(info_sections_mask)
    , m_show_columns(b_show_columns)
    , m_show_groups(b_show_groups)
    , m_initialising(false)
    , m_field_list(m_fields)
{
}

INT_PTR CALLBACK ItemPropertiesConfig::g_DialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ItemPropertiesConfig* p_data = nullptr;
    if (msg == WM_INITDIALOG) {
        p_data = reinterpret_cast<ItemPropertiesConfig*>(lp);
        SetWindowLongPtr(wnd, DWLP_USER, lp);
    } else
        p_data = reinterpret_cast<ItemPropertiesConfig*>(GetWindowLongPtr(wnd, DWLP_USER));
    return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
}

} // namespace cui::panels::item_properties
