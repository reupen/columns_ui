#include "stdafx.h"
#include "menu_helpers.h"

bool operator==(const MenuItemIdentifier& p1, const MenuItemIdentifier& p2)
{
    return p1.m_command == p2.m_command && p1.m_subcommand == p2.m_subcommand;
}

bool operator!=(const MenuItemIdentifier& p1, const MenuItemIdentifier& p2)
{
    return !(p1 == p2);
}

MenuItemCache::MenuItemCache()
{
    service_enum_t<mainmenu_commands> e;
    service_ptr_t<mainmenu_commands> ptr;

    while (e.next(ptr)) {
        // if (ptr->get_type() == menu_item::TYPE_MAIN)
        {
            unsigned p_service_item_count = ptr->get_command_count();
            for (unsigned p_service_item_index = 0; p_service_item_index < p_service_item_count;
                 p_service_item_index++) {
                MenuItemInfo info;

                info.m_command = ptr->get_command(p_service_item_index);

                pfc::string8 name, full;
                ptr->get_name(p_service_item_index, name);
                {
                    pfc::list_t<pfc::string8> levels;
                    GUID parent = ptr->get_parent();
                    while (parent != pfc::guid_null) {
                        pfc::string8 parentname;
                        if (menu_helpers::maingroupname_from_guid(GUID(parent), parentname, parent))
                            levels.insert_item(parentname, 0);
                    }
                    unsigned count = levels.get_count();
                    for (unsigned i = 0; i < count; i++) {
                        full.add_string(levels[i]);
                        full.add_byte('/');
                    }
                }
                full.add_string(name);

                /*if (p_node.is_valid() && p_node->get_type() == menu_item_node::TYPE_POPUP)
                {
                unsigned child, child_count = p_node->get_children_count();
                for (child=0;child<child_count;child++)
                {
                menu_item_node * p_child = p_node->get_child(child);
                if (p_child->get_type() == menu_item_node::TYPE_COMMAND)
                {
                pfc::string8 subfull = full,subname;
                unsigned dummy;
                p_child->get_display_data(subname, dummy, metadb_handle_list(), pfc::guid_null);
                subfull.add_byte('/');
                subfull.add_string(subname);

                menu_item_info * p_info = new(std::nothrow) menu_item_info (info);
                p_info->m_subcommand = p_child->get_guid();
                p_child->get_description(p_info->m_desc);
                p_info->m_name = subfull;

                m_data.add_item(p_info);
                }
                }
                }
                else*/
                {
                    auto p_info = new MenuItemInfo(info);
                    ptr->get_description(p_service_item_index, p_info->m_desc);
                    p_info->m_name = full;

                    m_data.add_item(p_info);
                }
            }
        }
    }
}

const MenuItemCache::MenuItemInfo& MenuItemCache::get_item(unsigned n) const
{
    return *m_data[n];
}

namespace menu_helpers {
auto get_context_menu_node_name(contextmenu_item_node* p_node)
{
    pfc::string8 name;
    unsigned _;
    p_node->get_display_data(name, _, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list);
    return name;
}

bool __contextpath_from_guid_recur(
    contextmenu_item_node* p_node, const GUID& p_subcommand, std::list<std::string>& p_out, bool b_root)
{
    if (!p_node)
        return false;

    if (p_node->get_type() == contextmenu_item_node::type_command && p_node->get_guid() == p_subcommand) {
        auto subname = get_context_menu_node_name(p_node);
        p_out.emplace_back(subname);
        return true;
    }

    if (p_node->get_type() == contextmenu_item_node::type_group) {
        auto subname = get_context_menu_node_name(p_node);
        const auto has_name = !subname.is_empty();
        if (has_name)
            p_out.emplace_back(subname);

        const auto child_count = p_node->get_children_count();
        for (unsigned child = 0; child < child_count; child++) {
            const auto p_child = p_node->get_child(child);

            if (__contextpath_from_guid_recur(p_child, p_subcommand, p_out, false))
                return true;
        }

        if (has_name)
            p_out.erase(--p_out.end());
    }
    return false;
}

void get_context_menu_item_parent_names(const contextmenu_item::ptr& menu_item, std::list<std::string>& names)
{
    cui::fcl::service_list_auto_t<contextmenu_group> groups;
    GUID parent_id = menu_item->get_parent_();
    contextmenu_group::ptr group;

    while (parent_id != contextmenu_groups::root && groups.find_by_guid(parent_id, group)) {
        contextmenu_group_popup::ptr popup_parent;
        if (group->service_query_t(popup_parent)) {
            pfc::string8 name;
            popup_parent->get_name(name);
            names.emplace_front(name.c_str());
        }
        parent_id = group->get_parent();
    }
}

auto get_context_menu_item_name_parts(GUID p_guid, GUID p_subcommand)
{
    std::list<std::string> item_parts;
    contextmenu_item::ptr menu_item;
    size_t menu_item_index{};

    if (!menu_item_resolver::g_resolve_context_command(p_guid, menu_item, menu_item_index)) {
        item_parts.emplace_back("Unknown command");
        return item_parts;
    }

    get_context_menu_item_parent_names(menu_item, item_parts);

    if (p_subcommand == GUID{}) {
        pfc::string8 part_name;
        menu_item->get_item_name(menu_item_index, part_name);
        item_parts.emplace_front(part_name.c_str());
    } else {
        pfc::ptrholder_t<contextmenu_item_node_root> p_node = menu_item->instantiate_item(
            menu_item_index, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list);

        if (!__contextpath_from_guid_recur(p_node.get_ptr(), p_subcommand, item_parts, true))
            item_parts.emplace_back("Unknown command");
    }

    return item_parts;
}

void contextpath_from_guid(GUID p_guid, GUID p_subcommand, pfc::string_base& p_out, bool b_short)
{
    auto name_parts = get_context_menu_item_name_parts(p_guid, p_subcommand);

    if (b_short) {
        p_out = (*--name_parts.end()).data();
        return;
    }

    p_out.reset();

    for (auto&& part : name_parts) {
        if (!p_out.is_empty())
            p_out << "/";
        p_out << part.data();
    }
}

bool maingroupname_from_guid(const GUID& p_guid, pfc::string_base& p_out, GUID& parentout)
{
    p_out.reset();
    parentout = pfc::guid_null;
    {
        service_enum_t<mainmenu_group> e;
        service_ptr_t<mainmenu_group> ptr;
        service_ptr_t<mainmenu_group_popup> ptrp;

        // unsigned p_service_item_index;
        while (e.next(ptr)) {
            {
                if (ptr->get_guid() == p_guid) {
                    parentout = ptr->get_parent();
                    if (ptr->service_query_t(ptrp)) {
                        ptrp->get_display_string(p_out);
                        return true;
                    }
                    return false;
                }
            }
        }
    }
    return false;
}
bool mainmenunode_subguid_to_path(
    const mainmenu_node::ptr& ptr_node, const GUID& p_subguid, pfc::string8& p_out, bool b_is_root)
{
    if (ptr_node.is_valid()) {
        switch (ptr_node->get_type()) {
        case mainmenu_node::type_command: {
            if (p_subguid == ptr_node->get_guid()) {
                t_uint32 flags;
                ptr_node->get_display(p_out, flags);
                return true;
            }
        }
            return false;
        case mainmenu_node::type_group: {
            for (t_size i = 0, count = ptr_node->get_children_count(); i < count; i++) {
                mainmenu_node::ptr ptr_child = ptr_node->get_child(i);
                pfc::string8 name;
                if (mainmenunode_subguid_to_path(ptr_child, p_subguid, name)) {
                    if (b_is_root)
                        p_out = name;
                    else {
                        t_uint32 flags;
                        ptr_node->get_display(p_out, flags);
                        p_out << "/" << name;
                    }
                    return true;
                }
            }
        }
            return false;
        default:
            return false;
        }
    }
    return false;
}
void mainpath_from_guid(const GUID& p_guid, const GUID& p_subguid, pfc::string_base& p_out, bool b_short)
{
    p_out.reset();
    service_enum_t<mainmenu_commands> e;
    service_ptr_t<mainmenu_commands> ptr;

    while (e.next(ptr)) {
        service_ptr_t<mainmenu_commands_v2> ptr_v2;
        ptr->service_query_t(ptr_v2);
        unsigned p_service_item_count = ptr->get_command_count();
        for (unsigned p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++) {
            if (p_guid == ptr->get_command(p_service_item_index)) {
                pfc::string8 name;
                ptr->get_name(p_service_item_index, name);
                if (p_subguid != pfc::guid_null && ptr_v2.is_valid()
                    && ptr_v2->is_command_dynamic(p_service_item_index)) {
                    pfc::string8 name_sub;
                    mainmenu_node::ptr ptr_node = ptr_v2->dynamic_instantiate(p_service_item_index);
                    mainmenunode_subguid_to_path(ptr_node, p_subguid, name_sub, true);
                    name << "/" << name_sub;
                }
                if (!b_short) {
                    pfc::list_t<pfc::string8> levels;
                    GUID parent = ptr->get_parent();
                    while (parent != pfc::guid_null) {
                        pfc::string8 parentname;
                        if (maingroupname_from_guid(GUID(parent), parentname, parent))
                            levels.insert_item(parentname, 0);
                    }
                    unsigned count = levels.get_count();
                    for (unsigned i = 0; i < count; i++) {
                        p_out.add_string(levels[i]);
                        p_out.add_byte('/');
                    }
                }
                p_out.add_string(name);
            }
        }
    }
}
}; // namespace menu_helpers
