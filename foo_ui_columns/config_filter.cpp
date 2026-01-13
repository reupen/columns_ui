#include "pch.h"
#include "filter.h"
#include "filter_config_var.h"
#include "config.h"
#include "core_dark_list_view.h"
#include "dark_mode_dialog.h"
#include "list_view_drop_target.h"
#include "permutation_utils.h"

namespace {

void get_list_view_insert_items(size_t base, size_t count, pfc::list_t<uih::ListView::InsertItem>& items)
{
    items.set_count(count);

    for (auto index : ranges::views::iota(size_t{}, count)) {
        items[index].m_subitems.resize(2);
        items[index].m_subitems[0] = cui::panels::filter::cfg_field_list[base + index].m_name;
        items[index].m_subitems[1] = cui::panels::filter::cfg_field_list[base + index].m_field;
    }
}

} // namespace

class FieldList : public cui::helpers::CoreDarkListView {
public:
    size_t m_edit_index, m_edit_column;
    FieldList() : m_edit_index(pfc_infinite), m_edit_column(pfc_infinite) {}

    void move_item(size_t old_index, size_t new_index)
    {
        auto permutation = cui::utils::create_shift_item_permutation(old_index, new_index, get_item_count());
        cui::panels::filter::cfg_field_list.reorder(permutation.data());
        cui::panels::filter::FilterPanel::s_on_fields_reordered(permutation, old_index, new_index);

        const auto first_affected = std::min(old_index, new_index);
        const auto last_affected = std::max(old_index, new_index);

        pfc::list_t<InsertItem> items;
        get_list_view_insert_items(first_affected, last_affected - first_affected + 1, items);
        replace_items(first_affected, items);
    }

    void execute_default_action(size_t index, size_t column, bool b_keyboard, bool b_ctrl) override
    {
        activate_inline_editing(index, column);
    }

    void notify_on_create() override
    {
        RECT rc{};
        GetClientRect(get_wnd(), &rc);
        const auto client_width = rc.right - rc.left;

        set_selection_mode(SelectionMode::SingleRelaxed);
        set_columns({{"Name", client_width / 3}, {"Field", client_width * 2 / 3}});

        wil::com_ptr drop_target(new cui::utils::SimpleListViewDropTarget(
            this, [this](size_t old_index, size_t new_index) { move_item(old_index, new_index); }));

        RegisterDragDrop(get_wnd(), drop_target.get());
    }

    void notify_on_destroy() override { RevokeDragDrop(get_wnd()); }

    void move_selection(int delta) override
    {
        const auto selection_index = fbh::as_optional(get_selected_item_single());

        if (!selection_index)
            return;

        const auto new_index = std::clamp(gsl::narrow<ptrdiff_t>(*selection_index) + delta, ptrdiff_t{},
            gsl::narrow<ptrdiff_t>(get_item_count() - 1));

        move_item(*selection_index, gsl::narrow<size_t>(new_index));
        set_item_selected_single(new_index, false);
        ensure_visible(new_index);
    }

    bool do_drag_drop(WPARAM wp) override
    {
        DWORD drop_effect{DROPEFFECT_NONE};
        const auto data_object = cui::utils::create_simple_list_view_data_object(get_wnd());
        LOG_IF_FAILED(uih::ole::do_drag_drop(get_wnd(), wp, data_object.get(), DROPEFFECT_MOVE, NULL, &drop_effect));
        return true;
    }

    bool notify_before_create_inline_edit(
        const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse) override
    {
        return column <= 1 && indices.get_count() == 1;
    }
    bool notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
        pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries) override
    {
        size_t indices_count = indices.get_count();
        if (indices_count == 1 && indices[0] < cui::panels::filter::cfg_field_list.get_count()) {
            m_edit_index = indices[0];
            m_edit_column = column;

            p_text = m_edit_column ? cui::panels::filter::cfg_field_list[m_edit_index].m_field
                                   : cui::panels::filter::cfg_field_list[m_edit_index].m_name;

            return true;
        }
        return false;
    }
    void notify_save_inline_edit(const char* value) override
    {
        if (m_edit_index < cui::panels::filter::cfg_field_list.get_count()) {
            pfc::string8& dest = m_edit_column ? cui::panels::filter::cfg_field_list[m_edit_index].m_field
                                               : cui::panels::filter::cfg_field_list[m_edit_index].m_name;
            cui::panels::filter::Field field_old = cui::panels::filter::cfg_field_list[m_edit_index];
            if (strcmp(dest, value) != 0) {
                pfc::string8 valueReal = value;
                if (m_edit_column == 0)
                    cui::panels::filter::cfg_field_list.fix_name(valueReal);
                dest = valueReal;
                pfc::list_t<SizedInsertItem<2, 0>> items;
                items.set_count(1);
                {
                    items[0].m_subitems[0] = cui::panels::filter::cfg_field_list[m_edit_index].m_name;
                    items[0].m_subitems[1] = cui::panels::filter::cfg_field_list[m_edit_index].m_field;
                }
                replace_items(m_edit_index, items);
                if (m_edit_column == 0)
                    cui::panels::filter::FilterPanel::g_on_field_title_change(field_old.m_name, valueReal);
                else
                    cui::panels::filter::FilterPanel::g_on_field_query_change(
                        cui::panels::filter::cfg_field_list[m_edit_index]);
            }
        }
        m_edit_column = pfc_infinite;
        m_edit_index = pfc_infinite;
    }
};

static class TabFilterFields : public PreferencesTab {
    FieldList m_field_list;

public:
    TabFilterFields() = default;

    void refresh_me(HWND wnd)
    {
        m_initialising = true;

        m_field_list.remove_items(bit_array_true());
        pfc::list_t<uih::ListView::InsertItem> items;
        size_t count = cui::panels::filter::cfg_field_list.get_count();
        get_list_view_insert_items(0, count, items);
        m_field_list.insert_items(0, items.get_count(), items.get_ptr());

        m_initialising = false;
    }

    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            {
                LOGFONT font{};
                GetObject(GetWindowFont(wnd), sizeof(font), &font);
                m_field_list.set_font_from_log_font(font);
                HWND wnd_fields = m_field_list.create(wnd, uih::WindowPosition(7, 30, 313, 213), true);
                SetWindowPos(wnd_fields, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                ShowWindow(wnd_fields, SW_SHOWNORMAL);
            }
            refresh_me(wnd);
        } break;
        case WM_DESTROY: {
            // if (m_changed)
            //    filter_panel::g_on_fields_change();
            m_field_list.destroy();
        } break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_FILTER_HELP: {
                const char* text = "You can use either enter field names (for remappings, separate multiple field "
                                   "names by a semi-colon) or titleformatting scripts to specify fields. "
                                   "For example, \"Album Artist;Artist\" or \"%album artist%\".\r\n\r\n"
                                   "Only the former format supports multiple values per field and is compatible with "
                                   "inline metadata editing.";
                cui::dark::modeless_info_box(wnd, "Filter Field Help", text, uih::InfoBoxType::Neutral);
            } break;
            case IDC_UP: {
                if (m_field_list.get_selection_count(2) == 1) {
                    size_t index = 0;
                    size_t count = m_field_list.get_item_count();
                    while (!m_field_list.get_item_selected(index) && index < count)
                        index++;
                    if (index && cui::panels::filter::cfg_field_list.get_count()) {
                        cui::panels::filter::cfg_field_list.swap_items(index, index - 1);
                        cui::panels::filter::FilterPanel::s_on_fields_swapped(index, index - 1);

                        pfc::list_t<uih::ListView::InsertItem> items;
                        get_list_view_insert_items(index - 1, 2, items);
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
                    if (index + 1 < count && index + 1 < cui::panels::filter::cfg_field_list.get_count()) {
                        cui::panels::filter::cfg_field_list.swap_items(index, index + 1);
                        cui::panels::filter::FilterPanel::s_on_fields_swapped(index, index + 1);

                        pfc::list_t<uih::ListView::InsertItem> items;
                        get_list_view_insert_items(index, 2, items);
                        m_field_list.replace_items(index, items);
                        m_field_list.set_item_selected_single(index + 1);
                    }
                }
            } break;
            case IDC_NEW: {
                cui::panels::filter::Field temp;
                temp.m_name = "<enter name here>";
                temp.m_field = "<enter field here>";
                size_t index = cui::panels::filter::cfg_field_list.add_item(temp);
                cui::panels::filter::FilterPanel::s_on_new_field(temp);

                pfc::list_t<uih::ListView::InsertItem> items;
                get_list_view_insert_items(index, 1, items);
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
                    if (index < count && index < cui::panels::filter::cfg_field_list.get_count()) {
                        cui::panels::filter::cfg_field_list.remove_by_idx(index);
                        m_field_list.remove_item(index);
                        cui::panels::filter::FilterPanel::s_on_field_removed(index);
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
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_FILTER_FIELDS,
            [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Fields"; }

private:
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
    bool m_initialising{false};
} g_tab_filter_fields;

static class TabFilterAppearance : public PreferencesTab {
public:
    TabFilterAppearance() = default;

    void on_init_dialog(HWND wnd)
    {
        pfc::vartoggle_t<bool> initialising_toggle(m_initialising, true);

        const auto wnd_edge_style = GetDlgItem(wnd, IDC_EDGESTYLE);
        const auto edge_style_options = {L"None", L"Sunken", L"Grey"};
        for (auto&& option : edge_style_options)
            ComboBox_AddString(wnd_edge_style, option);

        ComboBox_SetCurSel(wnd_edge_style, cui::panels::filter::cfg_edgestyle);

        uih::enhance_edit_control(wnd, IDC_PADDING);
        SendDlgItemMessage(wnd, IDC_SPINPADDING, UDM_SETRANGE32, -100, 100);
        SendDlgItemMessage(wnd, IDC_SPINPADDING, UDM_SETPOS32, 0, cui::panels::filter::cfg_vertical_item_padding);

        const auto wnd_show_column_titles = GetDlgItem(wnd, IDC_FILTERS_SHOW_COLUMN_TITLES);
        Button_SetCheck(
            wnd_show_column_titles, cui::panels::filter::cfg_show_column_titles ? BST_CHECKED : BST_UNCHECKED);

        const auto wnd_show_sort_indicators = GetDlgItem(wnd, IDC_FILTERS_SHOW_SORT_INDICATORS);
        Button_SetCheck(
            wnd_show_sort_indicators, cui::panels::filter::cfg_show_sort_indicators ? BST_CHECKED : BST_UNCHECKED);
    }

    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            on_init_dialog(wnd);
            break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_FILTERS_SHOW_COLUMN_TITLES:
                cui::panels::filter::cfg_show_column_titles
                    = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                cui::panels::filter::FilterPanel::g_on_show_column_titles_change();
                break;
            case IDC_FILTERS_SHOW_SORT_INDICATORS:
                cui::panels::filter::cfg_show_sort_indicators
                    = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                cui::panels::filter::FilterPanel::g_on_show_sort_indicators_change();
                break;
            case IDC_PADDING | EN_CHANGE << 16:
                if (!m_initialising) {
                    cui::panels::filter::cfg_vertical_item_padding
                        = strtol(uGetWindowText(reinterpret_cast<HWND>(lp)).get_ptr(), nullptr, 10);
                    cui::panels::filter::FilterPanel::g_on_vertical_item_padding_change();
                }
                break;
            case IDC_EDGESTYLE | CBN_SELCHANGE << 16:
                cui::panels::filter::cfg_edgestyle = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
                cui::panels::filter::FilterPanel::g_on_edgestyle_change();
                break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_FILTER_APPEARANCE,
            [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Appearance"; }

private:
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
    bool m_initialising{false};
} g_tab_filter_appearance;

static class TabFilterBehaviour : public PreferencesTab {
public:
    TabFilterBehaviour() = default;

    void on_init_dialog(HWND wnd)
    {
        pfc::vartoggle_t<bool> initialising_toggle(m_initialising, true);

        const auto double_click_action_options = {L"Send to autosend playlist", L"Send to autosend playlist and play",
            L"Send to playlist", L"Send to playlist and play", L"Add to active playlist"};
        const auto wnd_double_click_action = GetDlgItem(wnd, IDC_DBLCLK);
        for (auto&& option : double_click_action_options)
            ComboBox_AddString(wnd_double_click_action, option);
        ComboBox_SetCurSel(wnd_double_click_action, cui::panels::filter::cfg_doubleclickaction);

        const auto middle_click_action_options
            = {L"None", L"Send to autosend playlist", L"Send to autosend playlist and play", L"Send to playlist",
                L"Send to playlist and play", L"Add to active playlist"};

        const auto wnd_middle_click_action = uGetDlgItem(wnd, IDC_MIDDLE);
        for (auto&& option : middle_click_action_options)
            ComboBox_AddString(wnd_middle_click_action, option);
        ComboBox_SetCurSel(wnd_middle_click_action, cui::panels::filter::cfg_middleclickaction);

        const auto wnd_precedence = uGetDlgItem(wnd, IDC_PRECEDENCE);
        ComboBox_AddString(wnd_precedence, L"By position in splitter");
        ComboBox_AddString(wnd_precedence, L"By field order in field list");
        ComboBox_SetCurSel(wnd_precedence, cui::panels::filter::cfg_orderedbysplitters ? 0 : 1);

        const auto wnd_sort = GetDlgItem(wnd, IDC_SORT);
        Button_SetCheck(wnd_sort, cui::panels::filter::cfg_sort ? BST_CHECKED : BST_UNCHECKED);

        const auto wnd_sort_string = GetDlgItem(wnd, IDC_SORT_STRING);
        uih::enhance_edit_control(wnd_sort_string);
        uSetWindowText(wnd_sort_string, cui::panels::filter::cfg_sort_string);

        const auto wnd_autosend_reverse_sort = GetDlgItem(wnd, IDC_REVERSE_SORT_TRACKS);
        Button_SetCheck(
            wnd_autosend_reverse_sort, cui::panels::filter::cfg_reverse_sort_tracks ? BST_CHECKED : BST_UNCHECKED);

        const auto wnd_autosend = GetDlgItem(wnd, IDC_AUTOSEND);
        Button_SetCheck(wnd_autosend, cui::panels::filter::cfg_autosend ? BST_CHECKED : BST_UNCHECKED);

        const auto wnd_showempty = GetDlgItem(wnd, IDC_SHOWEMPTY);
        Button_SetCheck(wnd_showempty, cui::panels::filter::cfg_showemptyitems ? BST_CHECKED : BST_UNCHECKED);

        const auto wnd_allow_sorting = GetDlgItem(wnd, IDC_FILTERS_ALLOW_SORTING);
        Button_SetCheck(wnd_allow_sorting, cui::panels::filter::cfg_allow_sorting ? BST_CHECKED : BST_UNCHECKED);
    }

    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            on_init_dialog(wnd);
            break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_SORT:
                cui::panels::filter::cfg_sort = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                break;
            case IDC_AUTOSEND:
                cui::panels::filter::cfg_autosend = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                break;
            case IDC_REVERSE_SORT_TRACKS:
                cui::panels::filter::cfg_reverse_sort_tracks
                    = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                break;
            case IDC_FILTERS_ALLOW_SORTING:
                cui::panels::filter::cfg_allow_sorting = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                cui::panels::filter::FilterPanel::g_on_allow_sorting_change();
                break;
            case IDC_SHOWEMPTY:
                cui::panels::filter::cfg_showemptyitems = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                cui::panels::filter::FilterPanel::g_on_showemptyitems_change(cui::panels::filter::cfg_showemptyitems);
                break;
            case IDC_SORT_STRING | EN_CHANGE << 16:
                cui::panels::filter::cfg_sort_string = uGetWindowText(reinterpret_cast<HWND>(lp));
                break;
            case IDC_PRECEDENCE | CBN_SELCHANGE << 16:
                cui::panels::filter::cfg_orderedbysplitters = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp)) == 0;
                cui::panels::filter::FilterPanel::g_on_orderedbysplitters_change();
                break;
            case IDC_MIDDLE | CBN_SELCHANGE << 16:
                cui::panels::filter::cfg_middleclickaction = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
                break;
            case IDC_DBLCLK | CBN_SELCHANGE << 16:
                cui::panels::filter::cfg_doubleclickaction = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
                break;
            }
        }
        return 0;
    }
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_FILTER_BEHAVIOUR,
            [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Behaviour"; }

private:
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
    bool m_initialising{false};
} g_tab_filter_behaviour;

PreferencesTab* g_tabs_filters[] = {&g_tab_filter_behaviour, &g_tab_filter_appearance, &g_tab_filter_fields};

cfg_int cfg_child_filters({0xe57a430e, 0x51bb, 0x4fcc, {0xb0, 0xbc, 0x9d, 0x22, 0x8b, 0x89, 0x1a, 0x17}}, 0);

constexpr GUID guid_filters_page = {0x71a480e2, 0x9007, 0x4315, {0x8d, 0xf3, 0x81, 0x63, 0x6c, 0x74, 0xa, 0xad}};

service_factory_single_t<PreferencesTabsHost> page_filters(
    "Filters", g_tabs_filters, guid_filters_page, g_guid_columns_ui_preferences_page, cfg_child_filters);
