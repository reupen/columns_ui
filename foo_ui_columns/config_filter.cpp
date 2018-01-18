#include "stdafx.h"
#include "filter.h"
#include "filter_config_var.h"
#include "config.h"

class t_list_view_filter : public uih::ListView
{
public:
    t_size m_edit_index, m_edit_column;
    t_list_view_filter() : m_edit_index(pfc_infinite), m_edit_column(pfc_infinite) {};

    void execute_default_action(t_size index, t_size column, bool b_keyboard, bool b_ctrl) override
    {
        activate_inline_editing(index, column);
    }
    void notify_on_create() override
    {
        set_single_selection(true);
        pfc::list_t<Column> columns;
        columns.set_count(2);
        columns[0].m_title = "Name";
        columns[0].m_size = 130;
        columns[1].m_title = "Field";
        columns[1].m_size = 250;
        uih::ListView::set_columns(columns);
    };
    bool notify_before_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, bool b_source_mouse) override
    {
        if (column <= 1 && indices.get_count() == 1)
            return true;
        return false;
    };
    bool notify_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, pfc::string_base & p_text, t_size & p_flags, mmh::ComPtr<IUnknown> & pAutocompleteEntries) override
    {
        t_size indices_count = indices.get_count();
        if (indices_count == 1 && indices[0] < filter_panel::cfg_field_list.get_count()) {
            m_edit_index = indices[0];
            m_edit_column = column;

            p_text = m_edit_column ? filter_panel::cfg_field_list[m_edit_index].m_field : filter_panel::cfg_field_list[m_edit_index].m_name;

            return true;
        }
        return false;
    };
    void notify_save_inline_edit(const char * value) override
    {
        if (m_edit_index < filter_panel::cfg_field_list.get_count()) {
            pfc::string8 & dest = m_edit_column ? filter_panel::cfg_field_list[m_edit_index].m_field : filter_panel::cfg_field_list[m_edit_index].m_name;
            filter_panel::field_t field_old = filter_panel::cfg_field_list[m_edit_index];
            if (strcmp(dest, value)) {
                pfc::string8 valueReal = value;
                if (m_edit_column == 0)
                    filter_panel::cfg_field_list.fix_name(valueReal);
                dest = valueReal;
                pfc::list_t<uih::ListView::SizedInsertItem<2, 0>> items;
                items.set_count(1);
                {
                    items[0].m_subitems[0] = filter_panel::cfg_field_list[m_edit_index].m_name;
                    items[0].m_subitems[1] = filter_panel::cfg_field_list[m_edit_index].m_field;
                }
                replace_items(m_edit_index, items);
                if (m_edit_column == 0)
                    filter_panel::filter_panel_t::g_on_field_title_change(field_old.m_name, valueReal);
                else
                    filter_panel::filter_panel_t::g_on_field_query_change(filter_panel::cfg_field_list[m_edit_index]);
            }
        }
        m_edit_column = pfc_infinite;
        m_edit_index = pfc_infinite;
    }
private:
};

static class tab_filter_fields : public preferences_tab
{
    t_list_view_filter m_field_list;
public:
    tab_filter_fields()  = default;

    void get_insert_items(t_size base, t_size count, pfc::list_t<uih::ListView::InsertItem> & items)
    {
        t_size i;
        items.set_count(count);
        for (i=0; i<count; i++)
        {
            items[i].m_subitems.set_size(2);
            items[i].m_subitems[0] = filter_panel::cfg_field_list[base+i].m_name;
            items[i].m_subitems[1] = filter_panel::cfg_field_list[base+i].m_field;
        }
    }

    void refresh_me(HWND wnd)
    {
        initialising = true;    

        m_field_list.remove_items(pfc::bit_array_true());
        pfc::list_t<uih::ListView::InsertItem> items;
        t_size count = filter_panel::cfg_field_list.get_count();
        get_insert_items(0, count, items);
        m_field_list.insert_items(0, items.get_count(), items.get_ptr());

        initialising = false;    
    }


    static BOOL CALLBACK g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
    {
        tab_filter_fields * p_data = nullptr;
        if (msg == WM_INITDIALOG)
        {
            p_data = reinterpret_cast<tab_filter_fields*>(lp);
            SetWindowLongPtr(wnd, DWLP_USER, lp);
        }
        else
            p_data = reinterpret_cast<tab_filter_fields*>(GetWindowLongPtr(wnd, DWLP_USER));
        return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
    }

    BOOL CALLBACK on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
    {

        switch(msg)
        {
        case WM_INITDIALOG:
            {
                {
                    HWND wnd_fields = m_field_list.create(wnd, uih::WindowPosition(20,17,278,180), true);
                    SetWindowPos(wnd_fields, HWND_TOP, 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
                    ShowWindow(wnd_fields, SW_SHOWNORMAL);

                }
                refresh_me(wnd);
            }
            break;
        case WM_DESTROY:
            {
                //if (m_changed)
                //    filter_panel::g_on_fields_change();
                m_field_list.destroy();
            }
            break;
        case WM_COMMAND:
            switch(wp)
            {
            case IDC_FILTER_HELP:
                {
                    const char * text = 
                        "You can use either enter field names (for remappings, separate multiple field names by a semi-colon) or titleformatting scripts to specify fields. "
                        "For example, \"Album Artist;Artist\" or \"%album artist%\".\r\n\r\n"
                        "Only the former format supports multiple values per field and is compatible with inline metadata editing.";
                    fbh::show_info_box(wnd, "Filter Field Help", text);
                }
                break;
            case IDC_UP:
                {
                    if (m_field_list.get_selection_count(2) == 1)
                    {
                        t_size index = 0, count = m_field_list.get_item_count();
                        while (!m_field_list.get_item_selected(index) && index < count) index++;
                        if (index && filter_panel::cfg_field_list.get_count())
                        {
                            filter_panel::cfg_field_list.swap_items(index, index-1);
                            filter_panel::filter_panel_t::g_on_fields_swapped(index, index - 1);

                            pfc::list_t<uih::ListView::InsertItem> items;
                            get_insert_items(index-1, 2, items);
                            m_field_list.replace_items(index-1, items);
                            m_field_list.set_item_selected_single(index-1);
                        }
                    }
                }
                break;
            case IDC_DOWN:
                {
                    if (m_field_list.get_selection_count(2) == 1)
                    {
                        t_size index = 0, count = m_field_list.get_item_count();
                        while (!m_field_list.get_item_selected(index) && index < count) index++;
                        if (index+1 < count && index + 1 < filter_panel::cfg_field_list.get_count())
                        {
                            filter_panel::cfg_field_list.swap_items(index, index+1);
                            filter_panel::filter_panel_t::g_on_fields_swapped(index, index + 1);

                            pfc::list_t<uih::ListView::InsertItem> items;
                            get_insert_items(index, 2, items);
                            m_field_list.replace_items(index, items);
                            m_field_list.set_item_selected_single(index+1);
                        }
                    }
                }
                break;
            case IDC_NEW:
                {
                    filter_panel::field_t temp;
                    temp.m_name = "<enter name here>";
                    temp.m_field = "<enter field here>";
                    t_size index = filter_panel::cfg_field_list.add_item(temp);
                    filter_panel::filter_panel_t::g_on_new_field(temp);

                    pfc::list_t<uih::ListView::InsertItem> items;
                    get_insert_items(index, 1, items);
                    m_field_list.insert_items(index, 1, items.get_ptr());
                    m_field_list.set_item_selected_single(index);
                    SetFocus(m_field_list.get_wnd());
                    m_field_list.activate_inline_editing();

                }
                break;
            case IDC_REMOVE:
                {
                    if (m_field_list.get_selection_count(2) == 1)
                    {
                        pfc::bit_array_bittable mask(m_field_list.get_item_count());
                        m_field_list.get_selection_state(mask);
                        //bool b_found = false;
                        t_size index=0, count=m_field_list.get_item_count();
                        while (index < count)
                        {
                            if (mask[index]) break;
                            index++;
                        }
                        if (index < count && index < filter_panel::cfg_field_list.get_count())
                        {
                            filter_panel::cfg_field_list.remove_by_idx(index);
                            m_field_list.remove_item(index);
                            filter_panel::filter_panel_t::g_on_field_removed(index);
                            t_size new_count = m_field_list.get_item_count();
                            if (new_count)
                            {
                                if (index < new_count)
                                    m_field_list.set_item_selected_single(index);
                                else if (index)
                                    m_field_list.set_item_selected_single(index-1);
                            }
                        }
                    }
                }
                break;
            }
        }
        return 0;
    }
    void apply()
    {
    }
    HWND create(HWND wnd) override {return uCreateDialog(IDD_FILTER_FIELDS,wnd,g_on_message, (LPARAM)this);}
    const char * get_name() override {return "Fields";}
    bool get_help_url(pfc::string_base & p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:filter";
        return true;
    }

private:
    bool initialising{false};
} g_tab_filter_fields_;    


static class tab_filter_misc : public preferences_tab
{
public:
    tab_filter_misc()  = default;

    void refresh_me(HWND wnd)
    {
        initialising = true;
        HWND wnd_sort = GetDlgItem(wnd, IDC_SORT);
        HWND wnd_sort_string = GetDlgItem(wnd, IDC_SORT_STRING);
        HWND wnd_autosend = GetDlgItem(wnd, IDC_AUTOSEND);
        HWND wnd_showempty = GetDlgItem(wnd, IDC_SHOWEMPTY);
        const auto wnd_show_column_titles = GetDlgItem(wnd, IDC_FILTERS_SHOW_COLUMN_TITLES);
        const auto wnd_allow_sorting = GetDlgItem(wnd, IDC_FILTERS_ALLOW_SORTING);
        const auto wnd_show_sort_indicators = GetDlgItem(wnd, IDC_FILTERS_SHOW_SORT_INDICATORS);
        uSetWindowText(wnd_sort_string, filter_panel::cfg_sort_string);
        Button_SetCheck(wnd_sort, filter_panel::cfg_sort ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(wnd_showempty, filter_panel::cfg_showemptyitems ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(wnd_show_column_titles, filter_panel::cfg_show_column_titles ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(wnd_allow_sorting, filter_panel::cfg_allow_sorting ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(wnd_show_sort_indicators, filter_panel::cfg_show_sort_indicators ? BST_CHECKED : BST_UNCHECKED);

        Button_SetCheck(wnd_autosend, filter_panel::cfg_autosend ? BST_CHECKED : BST_UNCHECKED);

        SendDlgItemMessage(wnd, IDC_SPINPADDING, UDM_SETPOS32, 0, filter_panel::cfg_vertical_item_padding);

        initialising = false;
    }


    static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        tab_filter_misc * p_data = nullptr;
        if (msg == WM_INITDIALOG) {
            p_data = reinterpret_cast<tab_filter_misc*>(lp);
            SetWindowLongPtr(wnd, DWLP_USER, lp);
        }
        else
            p_data = reinterpret_cast<tab_filter_misc*>(GetWindowLongPtr(wnd, DWLP_USER));
        return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
    }

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {

        switch (msg) {
            case WM_INITDIALOG:
                {
                    {
                        HWND list = uGetDlgItem(wnd, IDC_DBLCLK);
                        uSendMessageText(list, CB_ADDSTRING, 0, "Send to autosend playlist");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Send to autosend playlist and play");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Send to playlist");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Send to playlist and play");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Add to active playlist");
                        SendMessage(list, CB_SETCURSEL, filter_panel::cfg_doubleclickaction, 0);

                        list = uGetDlgItem(wnd, IDC_MIDDLE);
                        uSendMessageText(list, CB_ADDSTRING, 0, "None");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Send to autosend playlist");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Send to autosend playlist and play");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Send to playlist");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Send to playlist and play");
                        uSendMessageText(list, CB_ADDSTRING, 0, "Add to active playlist");
                        SendMessage(list, CB_SETCURSEL, filter_panel::cfg_middleclickaction, 0);

                        uSendDlgItemMessageText(wnd, IDC_EDGESTYLE, CB_ADDSTRING, 0, "None");
                        uSendDlgItemMessageText(wnd, IDC_EDGESTYLE, CB_ADDSTRING, 0, "Sunken");
                        uSendDlgItemMessageText(wnd, IDC_EDGESTYLE, CB_ADDSTRING, 0, "Grey");
                        uSendDlgItemMessageText(wnd, IDC_EDGESTYLE, CB_SETCURSEL, filter_panel::cfg_edgestyle, nullptr);

                        uSendDlgItemMessageText(wnd, IDC_PRECEDENCE, CB_ADDSTRING, 0, "By position in splitter");
                        uSendDlgItemMessageText(wnd, IDC_PRECEDENCE, CB_ADDSTRING, 0, "By field order in field list");
                        uSendDlgItemMessageText(wnd, IDC_PRECEDENCE, CB_SETCURSEL, filter_panel::cfg_orderedbysplitters ? 0 : 1, nullptr);

                        SendDlgItemMessage(wnd, IDC_SPINPADDING, UDM_SETRANGE32, -100, 100);
                    }
                    refresh_me(wnd);
                }
                break;
            case WM_DESTROY:
                {
                    //if (m_changed)
                    //    filter_panel::g_on_fields_change();
                }
                break;
            case WM_COMMAND:
                switch (wp) {
                    case IDC_SORT:
                        {
                            filter_panel::cfg_sort = (Button_GetCheck((HWND)lp) != BST_UNCHECKED);
                        }
                        break;
                    case IDC_AUTOSEND:
                        filter_panel::cfg_autosend = (Button_GetCheck((HWND)lp) != BST_UNCHECKED);
                        break;
                    case IDC_FILTERS_SHOW_COLUMN_TITLES:
                        filter_panel::cfg_show_column_titles = (Button_GetCheck((HWND)lp) != BST_UNCHECKED);
                        filter_panel::filter_panel_t::g_on_show_column_titles_change();
                        break;
                    case IDC_FILTERS_ALLOW_SORTING:
                        filter_panel::cfg_allow_sorting = (Button_GetCheck((HWND)lp) != BST_UNCHECKED);
                        filter_panel::filter_panel_t::g_on_allow_sorting_change();
                        break;
                    case IDC_FILTERS_SHOW_SORT_INDICATORS:
                        filter_panel::cfg_show_sort_indicators = (Button_GetCheck((HWND)lp) != BST_UNCHECKED);
                        filter_panel::filter_panel_t::g_on_show_sort_indicators_change();
                        break;
                    case IDC_SHOWEMPTY:
                        filter_panel::cfg_showemptyitems = (Button_GetCheck((HWND)lp) != BST_UNCHECKED);
                        filter_panel::filter_panel_t::g_on_showemptyitems_change(filter_panel::cfg_showemptyitems);
                        break;
                    case (EN_CHANGE << 16) | IDC_SORT_STRING:
                        {
                            filter_panel::cfg_sort_string = string_utf8_from_window((HWND)lp);
                        }
                        break;
                    case (EN_CHANGE << 16) | IDC_PADDING:
                        if (!initialising) {
                            filter_panel::cfg_vertical_item_padding = strtol(string_utf8_from_window((HWND)lp).get_ptr(), nullptr, 10);
                            filter_panel::filter_panel_t::g_on_vertical_item_padding_change();
                        }
                        break;
                    case IDC_PRECEDENCE | (CBN_SELCHANGE << 16) :
                        filter_panel::cfg_orderedbysplitters = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0) == 0;
                        filter_panel::filter_panel_t::g_on_orderedbysplitters_change();
                        break;
                    case IDC_MIDDLE | (CBN_SELCHANGE << 16) :
                        filter_panel::cfg_middleclickaction = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
                        break;
                    case IDC_DBLCLK | (CBN_SELCHANGE << 16) :
                        filter_panel::cfg_doubleclickaction = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
                        break;
                    case IDC_EDGESTYLE | (CBN_SELCHANGE << 16) :
                        filter_panel::cfg_edgestyle = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
                        filter_panel::filter_panel_t::g_on_edgestyle_change();
                        break;
                }
        }
        return 0;
    }
    void apply()
    {
    }
    HWND create(HWND wnd) override { return uCreateDialog(IDD_FILTER_MISC, wnd, g_on_message, (LPARAM)this); }
    const char * get_name() override { return "General"; }
    bool get_help_url(pfc::string_base & p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:filter";
        return true;
    }

private:
    bool initialising{false};//, m_changed;
} g_tab_filter_misc_;



preferences_tab* const g_tab_filter_fields = &g_tab_filter_fields_;
preferences_tab* const g_tab_filter_misc = &g_tab_filter_misc_;
