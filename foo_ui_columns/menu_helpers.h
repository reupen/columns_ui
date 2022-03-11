#pragma once

#include "pch.h"

namespace menu_helpers {
pfc::string8 get_context_menu_node_name(contextmenu_item_node* p_node);
void get_context_menu_item_parent_names(const contextmenu_item::ptr& menu_item, std::list<std::string>& names);

std::string contextpath_from_guid(GUID p_guid, GUID p_subcommand, bool short_form = false);
bool maingroupname_from_guid(GUID p_guid, pfc::string_base& p_out, GUID& parentout);
bool mainmenunode_subguid_to_path(
    const mainmenu_node::ptr& ptr_node, const GUID& p_subguid, pfc::string8& p_out, bool b_is_root = false);
std::string mainpath_from_guid(GUID p_guid, GUID p_subguid, bool b_short = false);
} // namespace menu_helpers

struct MenuItemIdentifier {
    GUID m_command{};
    GUID m_subcommand{};
};

bool operator==(const MenuItemIdentifier& p1, const MenuItemIdentifier& p2);
bool operator!=(const MenuItemIdentifier& p1, const MenuItemIdentifier& p2);

class MenuItemInfo : public MenuItemIdentifier {
public:
    pfc::string8 m_name;
    pfc::string8 m_desc;
};
namespace cui::helpers {

std::vector<MenuItemInfo> get_main_menu_items();

inline bool execute_main_menu_command(MenuItemIdentifier command)
{
    if (command.m_subcommand != GUID{})
        return mainmenu_commands::g_execute_dynamic(command.m_command, command.m_subcommand);

    return mainmenu_commands::g_execute(command.m_command);
}
} // namespace cui::helpers
