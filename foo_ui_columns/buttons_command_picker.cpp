#include "stdafx.h"
#include "buttons.h"

bool CommandPickerData::__populate_mainmenu_dynamic_recur(
    CommandData& data, const mainmenu_node::ptr& ptr_node, std::list<std::string> name_parts, bool b_root)
{
    if (ptr_node.is_valid()) {
        pfc::string8 name_part;
        t_uint32 flags;
        ptr_node->get_display(name_part, flags);

        switch (ptr_node->get_type()) {
        case mainmenu_node::type_command: {
            name_parts.emplace_back(name_part);

            auto p_data = std::make_unique<CommandData>(data);
            p_data->m_subcommand = ptr_node->get_guid();
            ptr_node->get_description(p_data->m_desc);

            auto& data_item = m_data.emplace_back(std::move(p_data));

            auto path = mmh::join(name_parts, "/");
            unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, path.c_str());
            SendMessage(wnd_command, LB_SETITEMDATA, idx, reinterpret_cast<LPARAM>(data_item.get()));
        }
            return true;
        case mainmenu_node::type_group: {
            if (!b_root)
                name_parts.emplace_back(name_part);

            for (t_size i = 0, count = ptr_node->get_children_count(); i < count; i++) {
                mainmenu_node::ptr ptr_child = ptr_node->get_child(i);
                __populate_mainmenu_dynamic_recur(data, ptr_child, name_parts, false);
            }
        }
            return true;
        default:
            return false;
        }
    }
    return false;
}
bool CommandPickerData::__populate_commands_recur(
    CommandData& data, std::list<std::string> name_parts, contextmenu_item_node* p_node, bool b_root)
{
    if (p_node) {
        pfc::string8 name = menu_helpers::get_context_menu_node_name(p_node);
        if (!name.is_empty())
            name_parts.emplace_back(name);

        if (p_node->get_type() == contextmenu_item_node::TYPE_POPUP) {
            const unsigned child_count = p_node->get_children_count();

            for (unsigned child = 0; child < child_count; child++) {
                contextmenu_item_node* p_child = p_node->get_child(child);
                __populate_commands_recur(data, name_parts, p_child, false);
            }
            return true;
        }
        if (p_node->get_type() == contextmenu_item_node::TYPE_COMMAND && p_node->get_guid() != GUID{}) {
            auto p_data = std::make_unique<CommandData>(data);
            p_data->m_subcommand = p_node->get_guid();
            p_node->get_description(p_data->m_desc);

            auto& data_item = m_data.emplace_back(std::move(p_data));

            const auto path = mmh::join(name_parts, "/");
            unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, path.c_str());
            SendMessage(wnd_command, LB_SETITEMDATA, idx, reinterpret_cast<LPARAM>(data_item.get()));
            return true;
        }
    }
    return false;
}

void CommandPickerData::populate_commands()
{
    SendMessage(wnd_command, LB_RESETCONTENT, 0, 0);
    m_data.clear();
    SendMessage(wnd_command, WM_SETREDRAW, FALSE, 0);
    if (m_group == 2) {
        service_enum_t<contextmenu_item> e;
        service_ptr_t<contextmenu_item> ptr;

        unsigned p_item_index = 0; //,n=0;
        while (e.next(ptr)) {
            {
                unsigned p_service_item_count = ptr->get_num_items();
                for (unsigned p_service_item_index = 0; p_service_item_index < p_service_item_count;
                     p_service_item_index++) {
                    pfc::ptrholder_t<contextmenu_item_node_root> p_node(ptr->instantiate_item(
                        p_service_item_index, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list));

                    CommandData data;
                    data.m_guid = ptr->get_item_guid(p_service_item_index);

                    std::list<std::string> name_parts;
                    menu_helpers::get_context_menu_item_parent_names(ptr, name_parts);

                    if (p_node.is_valid() && __populate_commands_recur(data, name_parts, p_node.get_ptr(), true)) {
                    } else {
                        pfc::string8 name;
                        ptr->get_item_name(p_service_item_index, name);

                        name_parts.emplace_back(name);

                        auto p_data = std::make_unique<CommandData>(data);
                        ptr->get_item_description(p_service_item_index, p_data->m_desc);
                        auto& data_item = m_data.emplace_back(std::move(p_data));

                        const auto path = mmh::join(name_parts, "/");
                        unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, path.c_str());
                        SendMessage(wnd_command, LB_SETITEMDATA, idx, reinterpret_cast<LPARAM>(data_item.get()));
                        //                            n++;
                    }
                }
            }
        }
    } else if (m_group == 3) {
        service_enum_t<mainmenu_commands> e;
        service_ptr_t<mainmenu_commands> ptr;

        unsigned p_item_index = 0; //,n=0;
        while (e.next(ptr)) {
            service_ptr_t<mainmenu_commands_v2> ptr_v2;
            ptr->service_query_t(ptr_v2);
            {
                unsigned p_service_item_count = ptr->get_command_count();
                for (unsigned p_service_item_index = 0; p_service_item_index < p_service_item_count;
                     p_service_item_index++) {
                    CommandData data;
                    data.m_guid = ptr->get_command(p_service_item_index);

                    pfc::string8 name;
                    ptr->get_name(p_service_item_index, name);
                    std::list<std::string> name_parts{name.get_ptr()};

                    {
                        GUID parent = ptr->get_parent();
                        while (parent != pfc::guid_null) {
                            pfc::string8 parentname;
                            if (menu_helpers::maingroupname_from_guid(parent, parentname, parent))
                                name_parts.emplace_front(parentname);
                        }
                    }

                    if (ptr_v2.is_valid() && ptr_v2->is_command_dynamic(p_service_item_index)) {
                        mainmenu_node::ptr ptr_node = ptr_v2->dynamic_instantiate(p_service_item_index);
                        __populate_mainmenu_dynamic_recur(data, ptr_node, name_parts, true);
                    } else {
                        auto p_data = std::make_unique<CommandData>(data);
                        ptr->get_description(p_service_item_index, p_data->m_desc);
                        auto& data_item = m_data.emplace_back(std::move(p_data));

                        auto path = mmh::join(name_parts, "/");
                        unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, path.c_str());
                        SendMessage(wnd_command, LB_SETITEMDATA, idx, reinterpret_cast<LPARAM>(data_item.get()));
                    }
                }
            }
        }
    } else if (m_group == 1) {
        service_enum_t<uie::button> e;
        service_ptr_t<uie::button> ptr;
        while (e.next(ptr)) {
            service_ptr_t<uie::custom_button> p_button;
            if (ptr->get_guid_type() == uie::BUTTON_GUID_BUTTON && ptr->service_query_t(p_button)) {
                auto p_data = std::make_unique<CommandData>();
                p_data->m_guid = ptr->get_item_guid();
                p_button->get_description(p_data->m_desc);
                auto& data_item = m_data.emplace_back(std::move(p_data));
                pfc::string8 temp;
                p_button->get_name(temp);
                unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, temp);
                SendMessage(wnd_command, LB_SETITEMDATA, idx, reinterpret_cast<LPARAM>(data_item.get()));
            }
        }
    }
    unsigned count = SendMessage(wnd_command, LB_GETCOUNT, 0, 0);
    for (unsigned n = 0; n < count; n++) {
        LRESULT ret = SendMessage(wnd_command, LB_GETITEMDATA, n, 0);
        CommandData* p_data = ((CommandData*)ret);

        if (ret != LB_ERR && p_data->m_guid == m_guid && p_data->m_subcommand == m_subcommand) {
            SendMessage(wnd_command, LB_SETCURSEL, n, 0);
            update_description();
            break;
        }
    }
    SendMessage(wnd_command, WM_SETREDRAW, TRUE, 0);
}
void CommandPickerData::update_description()
{
    LRESULT p_command = SendMessage(wnd_command, LB_GETCURSEL, 0, 0);
    if (p_command != LB_ERR) {
        LRESULT p_data = SendMessage(wnd_command, LB_GETITEMDATA, p_command, 0);
        if (p_data != LB_ERR)
            uSendDlgItemMessageText(m_wnd, IDC_DESC, WM_SETTEXT, 0, ((CommandData*)p_data)->m_desc);
        else
            uSendDlgItemMessageText(m_wnd, IDC_DESC, WM_SETTEXT, 0, "");
    } else
        uSendDlgItemMessageText(m_wnd, IDC_DESC, WM_SETTEXT, 0, "");
}

void CommandPickerData::set_data(const CommandPickerParam& p_data)
{
    m_group = p_data.m_group;
    m_guid = p_data.m_guid;
    m_subcommand = p_data.m_subcommand;
    m_filter = p_data.m_filter;
}
void CommandPickerData::get_data(CommandPickerParam& p_data) const
{
    p_data.m_group = m_group;
    p_data.m_guid = m_guid;
    p_data.m_subcommand = m_subcommand;
    p_data.m_filter = m_filter;
}
void CommandPickerData::initialise(HWND wnd)
{
    m_wnd = wnd;
    wnd_group = GetDlgItem(wnd, IDC_GROUP);
    wnd_filter = GetDlgItem(wnd, IDC_ITEM);
    wnd_command = GetDlgItem(wnd, IDC_COMMAND);

    SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM) _T("Separator"));
    SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM) _T("Buttons"));
    SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM) _T("Context menu items"));
    SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM) _T("Main menu items"));

    SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM) _T("None"));
    SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM) _T("Now playing item"));
    SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM) _T("Current playlist selection"));
    SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM) _T("Active selection"));

    SendMessage(wnd_group, LB_SETCURSEL, m_group, 0);
}
void CommandPickerData::deinitialise(HWND wnd)
{
    SendMessage(wnd_group, LB_RESETCONTENT, 0, 0);
    SendMessage(wnd_filter, LB_RESETCONTENT, 0, 0);
    SendMessage(wnd_command, LB_RESETCONTENT, 0, 0);
    m_data.clear();
}
BOOL CommandPickerData::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_scope.initialize(FindOwningPopup(wnd));
        initialise(wnd);
        populate_commands();
        SendMessage(wnd_filter, LB_SETCURSEL, (WPARAM)m_filter, 0);
    }
        return TRUE;
    case WM_DESTROY:
        deinitialise(wnd);
        return TRUE;
    case WM_ERASEBKGND:
        SetWindowLongPtr(wnd, DWLP_MSGRESULT, TRUE);
        return TRUE;
    case WM_PAINT:
        uih::handle_modern_background_paint(wnd, GetDlgItem(wnd, IDOK));
        return TRUE;
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
        SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
        return (BOOL)GetSysColorBrush(COLOR_WINDOW);
    case WM_COMMAND:
        switch (wp) {
        case IDC_GROUP | (LBN_SELCHANGE << 16):
            m_group = SendMessage(wnd_group, LB_GETCURSEL, 0, 0);
            m_guid = {};
            m_subcommand = {};
            populate_commands();
            return TRUE;
        case IDC_ITEM | (LBN_SELCHANGE << 16): {
            LRESULT p_filter = SendMessage(wnd_filter, LB_GETCURSEL, 0, 0);
            if (p_filter != LB_ERR)
                m_filter = p_filter;
        }
            return TRUE;
        case IDC_COMMAND | (LBN_SELCHANGE << 16): {
            m_guid = {};
            m_subcommand = {};

            LRESULT p_command = SendMessage(wnd_command, LB_GETCURSEL, 0, 0);
            if (p_command != LB_ERR) {
                LRESULT ret = SendMessage(wnd_command, LB_GETITEMDATA, p_command, 0);
                auto* p_data = (CommandData*)ret;
                if (ret != LB_ERR) {
                    m_guid = p_data->m_guid;
                    m_subcommand = p_data->m_subcommand;
                }
            }
            update_description();
        }
            return TRUE;
        case IDCANCEL: {
            EndDialog(wnd, 0);
        }
            return TRUE;
        case IDOK: {
            EndDialog(wnd, 1);
        }
            return TRUE;
        }
        break;
    }
    return FALSE;
}
