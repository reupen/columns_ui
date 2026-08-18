#include "pch.h"
#include "buttons.h"
#include "dark_mode_dialog.h"

namespace cui::toolbars::buttons {

std::tuple<bool, CommandPickerData> CommandPickerDialog::open_modal(HWND wnd)
{
    dark::DialogDarkModeConfig dark_mode_config{
        .button_ids = {IDOK, IDCANCEL}, .list_box_ids = {IDC_GROUP, IDC_ITEM, IDC_COMMAND}};

    const auto dialog_result = modal_dialog_box(
        IDD_BUTTON_COMMAND_PICKER, dark_mode_config, wnd,
        [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); }, false);

    return {dialog_result > 0, m_data};
}

bool CommandPickerDialog::process_dynamic_main_menu_node_commands(
    const GUID& id, const mainmenu_node::ptr& ptr_node, std::list<std::string> name_parts, bool b_root)
{
    if (ptr_node.is_valid()) {
        pfc::string8 name_part;
        uint32_t flags;
        ptr_node->get_display(name_part, flags);

        switch (ptr_node->get_type()) {
        case mainmenu_node::type_command: {
            name_parts.emplace_back(name_part);

            auto command = std::make_unique<CommandData>();
            command->id = id;
            command->subcommand_id = ptr_node->get_guid();

            mmh::StringAdaptor adapted_description(command->description);
            ptr_node->get_description(adapted_description);

            command->path = mmh::to_utf16(mmh::join(name_parts, "/"));

            m_commands.emplace_back(std::move(command));
        }
            return true;
        case mainmenu_node::type_group: {
            if (!b_root)
                name_parts.emplace_back(name_part);

            for (size_t i = 0, count = ptr_node->get_children_count(); i < count; i++) {
                mainmenu_node::ptr ptr_child = ptr_node->get_child(i);
                process_dynamic_main_menu_node_commands(id, ptr_child, name_parts, false);
            }
        }
            return true;
        default:
            return false;
        }
    }
    return false;
}

bool CommandPickerDialog::process_dynamic_context_menu_node_commands(
    const GUID& id, std::list<std::string> name_parts, contextmenu_item_node* node)
{
    if (!node)
        return false;

    pfc::string8 name = menu_helpers::get_context_menu_node_name(node);

    if (!name.is_empty())
        name_parts.emplace_back(name);

    if (node->get_type() == contextmenu_item_node::TYPE_POPUP) {
        const auto child_count = node->get_children_count();

        for (size_t child = 0; child < child_count; child++) {
            contextmenu_item_node* child_node = node->get_child(child);
            process_dynamic_context_menu_node_commands(id, name_parts, child_node);
        }
        return true;
    }

    if (node->get_type() == contextmenu_item_node::TYPE_COMMAND && node->get_guid() != GUID{}) {
        auto command = std::make_unique<CommandData>();
        command->id = id;
        command->path = mmh::to_utf16(mmh::join(name_parts, "/"));
        command->subcommand_id = node->get_guid();

        mmh::StringAdaptor adapted_description(command->description);
        node->get_description(adapted_description);

        m_commands.emplace_back(std::move(command));
        return true;
    }

    return false;
}

void CommandPickerDialog::collect_commands()
{
    m_commands.clear();

    switch (m_data.group) {
    case TYPE_MENU_ITEM_CONTEXT:
        for (auto&& items : contextmenu_item::enumerate()) {
            for (const auto item_index : ranges::views::iota(0u, items->get_num_items())) {
                pfc::ptrholder_t node(items->instantiate_item(
                    item_index, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list));

                const auto id = items->get_item_guid(item_index);

                std::list<std::string> name_parts;
                menu_helpers::get_context_menu_item_parent_names(items, name_parts);

                if (node.is_valid() && process_dynamic_context_menu_node_commands(id, name_parts, node.get_ptr()))
                    continue;

                auto command = std::make_unique<CommandData>();
                command->id = id;

                std::string name;
                mmh::StringAdaptor adapted_name(name);
                items->get_item_name(item_index, adapted_name);
                name_parts.emplace_back(name);
                command->path = mmh::to_utf16(mmh::join(name_parts, "/"));

                mmh::StringAdaptor adapted_description(command->description);
                items->get_item_description(item_index, adapted_description);

                m_commands.emplace_back(std::move(command));
            }
        }
        break;
    case TYPE_MENU_ITEM_MAIN:
        for (auto&& commands_service : mainmenu_commands::enumerate()) {
            mainmenu_commands_v2::ptr commands_service_v2;
            commands_service_v2 &= commands_service;

            for (auto item_index : ranges::views::iota(0u, commands_service->get_command_count())) {
                const auto id = commands_service->get_command(item_index);

                pfc::string8 name;
                commands_service->get_name(item_index, name);
                std::list<std::string> name_parts{name.get_ptr()};

                auto parent_id = commands_service->get_parent();
                while (parent_id != GUID{}) {
                    pfc::string8 parent_name;
                    if (menu_helpers::maingroupname_from_guid(parent_id, parent_name, parent_id))
                        name_parts.emplace_front(parent_name);
                }

                if (commands_service_v2.is_valid() && commands_service_v2->is_command_dynamic(item_index)) {
                    mainmenu_node::ptr ptr_node = commands_service_v2->dynamic_instantiate(item_index);
                    process_dynamic_main_menu_node_commands(id, ptr_node, name_parts, true);
                } else {
                    auto command = std::make_unique<CommandData>();
                    command->id = id;
                    command->path = mmh::to_utf16(mmh::join(name_parts, "/"));

                    mmh::StringAdaptor adapted_description(command->description);
                    commands_service->get_description(item_index, adapted_description);
                    m_commands.emplace_back(std::move(command));
                }
            }
        }
        break;
    case TYPE_BUTTON:
        for (auto&& button_service : uie::button::enumerate()) {
            uie::custom_button::ptr custom_button_service;

            if (button_service->get_guid_type() == uie::BUTTON_GUID_BUTTON
                && (custom_button_service &= button_service)) {
                auto command = std::make_unique<CommandData>();
                command->id = button_service->get_item_guid();

                mmh::StringAdaptor adapted_description(command->description);
                custom_button_service->get_description(adapted_description);

                std::string name;
                mmh::StringAdaptor adapted_name(name);
                custom_button_service->get_name(adapted_name);
                command->path = mmh::to_utf16(name);

                m_commands.emplace_back(std::move(command));
            }
        }
        break;
    case TYPE_SEPARATOR:
        break;
    }
}

void CommandPickerDialog::populate_command_list() const
{
    ListBox_ResetContent(m_command_list_wnd);
    SetWindowRedraw(m_command_list_wnd, FALSE);

    for (const auto& command : m_commands) {
        const auto index = ListBox_AddString(m_command_list_wnd, command->path.c_str());

        if (index == LB_ERR || index == LB_ERRSPACE)
            continue;

        ListBox_SetItemData(m_command_list_wnd, index, reinterpret_cast<LPARAM>(command.get()));

        if (m_data.guid != GUID{} && command->id == m_data.guid && command->subcommand_id == m_data.subcommand) {
            ListBox_SetCurSel(m_command_list_wnd, index);
            update_description();
        }
    }

    SetWindowRedraw(m_command_list_wnd, TRUE);
}

void CommandPickerDialog::collect_commands_and_populate_command_list()
{
    collect_commands();
    populate_command_list();
}

void CommandPickerDialog::update_description() const
{
    LRESULT p_command = SendMessage(m_command_list_wnd, LB_GETCURSEL, 0, 0);
    if (p_command != LB_ERR) {
        LRESULT p_data = SendMessage(m_command_list_wnd, LB_GETITEMDATA, p_command, 0);
        if (p_data != LB_ERR)
            uSendDlgItemMessageText(
                m_wnd, IDC_DESC, WM_SETTEXT, 0, reinterpret_cast<CommandData*>(p_data)->description.c_str());
        else
            SetWindowTextW(GetDlgItem(m_wnd, IDC_DESC), L"");
    } else {
        SetWindowTextW(GetDlgItem(m_wnd, IDC_DESC), L"");
    }
}

void CommandPickerDialog::initialise(HWND wnd)
{
    m_wnd = wnd;
    m_command_group_wnd = GetDlgItem(wnd, IDC_GROUP);
    m_item_group = GetDlgItem(wnd, IDC_ITEM);
    m_command_list_wnd = GetDlgItem(wnd, IDC_COMMAND);

    SendMessage(m_command_group_wnd, LB_ADDSTRING, 0, (LPARAM) _T("Separator"));
    SendMessage(m_command_group_wnd, LB_ADDSTRING, 0, (LPARAM) _T("Buttons"));
    SendMessage(m_command_group_wnd, LB_ADDSTRING, 0, (LPARAM) _T("Context menu items"));
    SendMessage(m_command_group_wnd, LB_ADDSTRING, 0, (LPARAM) _T("Main menu items"));

    SendMessage(m_item_group, LB_ADDSTRING, 0, (LPARAM) _T("None"));
    SendMessage(m_item_group, LB_ADDSTRING, 0, (LPARAM) _T("Now playing item"));
    SendMessage(m_item_group, LB_ADDSTRING, 0, (LPARAM) _T("Current playlist selection"));
    SendMessage(m_item_group, LB_ADDSTRING, 0, (LPARAM) _T("Active selection"));

    SendMessage(m_command_group_wnd, LB_SETCURSEL, m_data.group, 0);
}

void CommandPickerDialog::deinitialise(HWND wnd)
{
    SendMessage(m_command_group_wnd, LB_RESETCONTENT, 0, 0);
    SendMessage(m_item_group, LB_RESETCONTENT, 0, 0);
    SendMessage(m_command_list_wnd, LB_RESETCONTENT, 0, 0);
    m_commands.clear();
}

INT_PTR CommandPickerDialog::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        initialise(wnd);
        collect_commands_and_populate_command_list();
        SendMessage(m_item_group, LB_SETCURSEL, static_cast<WPARAM>(m_data.filter), 0);
        return TRUE;
    }
    case WM_DESTROY:
        deinitialise(wnd);
        return TRUE;
    case WM_COMMAND:
        switch (wp) {
        case IDC_GROUP | (LBN_SELCHANGE << 16):
            m_data.group = ListBox_GetCurSel(m_command_group_wnd);
            m_data.guid = {};
            m_data.subcommand = {};
            collect_commands_and_populate_command_list();
            return TRUE;
        case IDC_ITEM | (LBN_SELCHANGE << 16): {
            const auto p_filter = ListBox_GetCurSel(m_item_group);
            if (p_filter != LB_ERR)
                m_data.filter = p_filter;
            return TRUE;
        }
        case IDC_COMMAND | (LBN_SELCHANGE << 16): {
            m_data.guid = {};
            m_data.subcommand = {};

            const auto p_command = ListBox_GetCurSel(m_command_list_wnd);
            if (p_command != LB_ERR) {
                LRESULT ret = SendMessage(m_command_list_wnd, LB_GETITEMDATA, p_command, 0);
                auto* p_data = (CommandData*)ret;
                if (ret != LB_ERR) {
                    m_data.guid = p_data->id;
                    m_data.subcommand = p_data->subcommand_id;
                }
            }
            update_description();
            return TRUE;
        }
        case IDCANCEL: {
            EndDialog(wnd, 0);
            return TRUE;
        }
        case IDOK: {
            EndDialog(wnd, 1);
            return TRUE;
        }
        }
        break;
    }
    return FALSE;
}

} // namespace cui::toolbars::buttons
