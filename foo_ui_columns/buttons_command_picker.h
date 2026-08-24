#pragma once

#include "buttons_button.h"
#include "core_dark_list_view.h"
#include "window_resize_helper.h"

namespace cui::toolbars::buttons {

constexpr bool does_group_use_tracks(int group)
{
    return group == TYPE_BUTTON || group == TYPE_MENU_ITEM_CONTEXT;
}

struct CommandPickerData {
    GUID guid{};
    GUID subcommand{};
    int group{TYPE_SEPARATOR};
    int filter{FILTER_ACTIVE_SELECTION};

    int cleaned_filter() const { return does_group_use_tracks(group) ? filter : FILTER_NONE; }
};

class CommandPickerDialog {
public:
    CommandPickerDialog(CommandPickerData data) : m_data(std::move(data)) {}
    CommandPickerDialog();

    std::tuple<bool, CommandPickerData> open_modal(HWND wnd);

private:
    class CommandData {
    public:
        GUID id{};
        GUID subcommand_id{};
        std::string description;

        void set_path(std::string path)
        {
            path_utf8 = std::move(path);
            path_utf16 = mmh::to_utf16(path_utf8);
        }

        wil::zstring_view get_path_utf8() { return path_utf8; }

        wil::zwstring_view get_path_utf16() { return path_utf16; }

    private:
        std::string path_utf8;
        std::wstring path_utf16;
    };

    void collect_commands();
    bool process_dynamic_main_menu_node_commands(
        const GUID& id, const mainmenu_node::ptr& ptr_node, std::list<std::string> name_parts, bool b_root);
    bool process_dynamic_context_menu_node_commands(
        const GUID& id, std::list<std::string> name_parts, contextmenu_item_node* node);

    void populate_command_list();
    void collect_commands_and_populate_command_list();

    void populate_item_group_list();

    void update_description() const;
    void initialise(HWND wnd);
    void deinitialise(HWND wnd);
    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    std::vector<std::shared_ptr<CommandData>> m_commands;
    std::vector<std::shared_ptr<CommandData>> m_filtered_commands;
    HWND m_wnd{};
    std::optional<utils::WindowResizeHelper> m_resize_helper;
    helpers::CoreDarkListView m_command_group_list_view;
    helpers::CoreDarkListView m_item_group_list_view;
    helpers::CoreDarkListView m_commands_list_view;
    HWND m_search_edit{};
    std::wstring m_search_string;
    CommandPickerData m_data;
};

} // namespace cui::toolbars::buttons
