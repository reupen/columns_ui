#pragma once

#include "buttons_button.h"

namespace cui::toolbars::buttons {

struct CommandPickerData {
    GUID guid{};
    GUID subcommand{};
    int group{TYPE_SEPARATOR};
    int filter{FILTER_ACTIVE_SELECTION};
};

class CommandPickerDialog {
public:
    CommandPickerDialog(CommandPickerData data = {}) : m_data(std::move(data)) {}

    std::tuple<bool, CommandPickerData> open_modal(HWND wnd);

private:
    class CommandData {
    public:
        GUID id{};
        GUID subcommand_id{};
        std::wstring path;
        std::string description;
    };

    void collect_commands();
    bool process_dynamic_main_menu_node_commands(
        const GUID& id, const mainmenu_node::ptr& ptr_node, std::list<std::string> name_parts, bool b_root);
    bool process_dynamic_context_menu_node_commands(
        const GUID& id, std::list<std::string> name_parts, contextmenu_item_node* node);

    void populate_command_list() const;
    void collect_commands_and_populate_command_list();

    void update_description() const;
    void initialise(HWND wnd);
    void deinitialise(HWND wnd);
    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    std::vector<std::unique_ptr<CommandData>> m_commands;
    HWND m_wnd{};
    HWND m_command_group_wnd{};
    HWND m_item_group{};
    HWND m_command_list_wnd{};
    HWND m_search_edit{};
    std::wstring m_search_string;
    CommandPickerData m_data;
};

} // namespace cui::toolbars::buttons
