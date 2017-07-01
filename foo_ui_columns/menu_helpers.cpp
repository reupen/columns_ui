#include "stdafx.h"
#include "menu_helpers.h"


bool operator==(const menu_item_identifier & p1, const menu_item_identifier & p2)
{
    return p1.m_command == p2.m_command && p1.m_subcommand == p2.m_subcommand;
}

bool operator!=(const menu_item_identifier & p1, const menu_item_identifier & p2)
{
    return !(p1 == p2);
}

menu_item_cache::menu_item_cache()
{
    service_enum_t<mainmenu_commands> e;
    service_ptr_t<mainmenu_commands> ptr;

    unsigned p_service_item_index;
    while (e.next(ptr))
    {
        //if (ptr->get_type() == menu_item::TYPE_MAIN)
        {
            unsigned p_service_item_count = ptr->get_command_count();
            for (p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++)
            {
                menu_item_info info;

                info.m_command = ptr->get_command(p_service_item_index);

                pfc::string8 name, full;
                ptr->get_name(p_service_item_index, name);
                {
                    pfc::list_t<pfc::string8> levels;
                    GUID parent = ptr->get_parent();
                    while (parent != pfc::guid_null)
                    {
                        pfc::string8 parentname;
                        if (menu_helpers::maingroupname_from_guid(GUID(parent), parentname, parent))
                            levels.insert_item(parentname, 0);
                    }
                    unsigned i, count = levels.get_count();
                    for (i = 0; i < count; i++)
                    {
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
                    auto  p_info = new menu_item_info(info);
                    ptr->get_description(p_service_item_index, p_info->m_desc);
                    p_info->m_name = full;

                    m_data.add_item(p_info);
                }


            }
        }
    }
}

const menu_item_cache::menu_item_info & menu_item_cache::get_item(unsigned n) const
{
    return *m_data[n];
}

namespace menu_helpers
{
    bool __contextpath_from_guid_recur(contextmenu_item_node * p_node, const GUID & p_subcommand, pfc::string_base & p_out, bool b_short, bool b_root)
    {
        if (p_node)
        {
            if (p_node->get_type() == contextmenu_item_node::TYPE_POPUP)
            {
                pfc::string8 subname, temp = p_out;
                unsigned dummy;
                p_node->get_display_data(subname, dummy, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list);
                if (temp.get_length() && temp.get_ptr()[temp.get_length() - 1] != '/')
                    temp.add_byte('/');
                temp << subname;
                unsigned child, child_count = p_node->get_children_count();
                for (child = 0; child<child_count; child++)
                {
                    contextmenu_item_node * p_child = p_node->get_child(child);
                    if (__contextpath_from_guid_recur(p_child, p_subcommand, temp, b_short, false))
                    {
                        p_out = temp;
                        return true;
                    }
                }
            }
            else if (p_node->get_type() == contextmenu_item_node::TYPE_COMMAND && !b_root)
            {
                if (p_node->get_guid() == p_subcommand)
                {
                    pfc::string8 subname;
                    unsigned dummy;
                    p_node->get_display_data(subname, dummy, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list);
                    if (!b_short)
                        p_out.add_byte('/');
                    else
                        p_out.reset();
                    p_out.add_string(subname);
                    return true;
                }
            }
        }
        return false;
    }
    void contextpath_from_guid(const GUID & p_guid, const GUID & p_subcommand, pfc::string_base & p_out, bool b_short)
    {
        p_out.reset();
        service_enum_t<contextmenu_item> e;
        service_ptr_t<contextmenu_item> ptr;

        unsigned p_service_item_index;
        while (e.next(ptr))
        {
            unsigned p_service_item_count = ptr->get_num_items();
            for (p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++)
            {
                if (p_guid == ptr->get_item_guid(p_service_item_index))
                {
                    pfc::string8 name;
                    ptr->get_item_name(p_service_item_index, name);
                    if (!b_short)
                    {
                        ptr->get_item_default_path(p_service_item_index, p_out);
                        if (p_out.get_length() && p_out[p_out.get_length() - 1] != '/')
                            p_out.add_byte('/');
                    }
                    p_out.add_string(name);

                    if (p_subcommand != pfc::guid_null)
                    {
                        pfc::ptrholder_t<contextmenu_item_node_root> p_node(ptr->instantiate_item(p_service_item_index, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list));

                        if (p_node.is_valid())
                            if (__contextpath_from_guid_recur(p_node.get_ptr(), p_subcommand, p_out, b_short, true))
                                return;
                    }
                }

            }
        }
    }
    bool maingroupname_from_guid(const GUID & p_guid, pfc::string_base & p_out, GUID & parentout)
    {
        p_out.reset();
        parentout = pfc::guid_null;
        {
            service_enum_t<mainmenu_group> e;
            service_ptr_t<mainmenu_group> ptr;
            service_ptr_t<mainmenu_group_popup> ptrp;

            //unsigned p_service_item_index;
            while (e.next(ptr))
            {
                {
                    if (ptr->get_guid() == p_guid)
                    {
                        parentout = ptr->get_parent();
                        if (ptr->service_query_t(ptrp))
                        {
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
    bool mainmenunode_subguid_to_path(const mainmenu_node::ptr & ptr_node, const GUID & p_subguid, pfc::string8 & p_out, bool b_is_root)
    {
        if (ptr_node.is_valid())
        {
            switch (ptr_node->get_type())
            {
            case mainmenu_node::type_command:
            {
                if (p_subguid == ptr_node->get_guid())
                {
                    t_uint32 flags;
                    ptr_node->get_display(p_out, flags);
                    return true;
                }
            }
            return false;
            case mainmenu_node::type_group:
            {
                mainmenu_node::ptr ptr_child;
                for (t_size i = 0, count = ptr_node->get_children_count(); i<count; i++)
                {
                    ptr_child = ptr_node->get_child(i);
                    pfc::string8 name;
                    if (mainmenunode_subguid_to_path(ptr_child, p_subguid, name))
                    {
                        if (b_is_root)
                            p_out = name;
                        else
                        {
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
            };
        }
        return false;
    }
    void mainpath_from_guid(const GUID & p_guid, const GUID & p_subguid, pfc::string_base & p_out, bool b_short)
    {
        p_out.reset();
        service_enum_t<mainmenu_commands> e;
        service_ptr_t<mainmenu_commands> ptr;

        unsigned p_service_item_index;
        while (e.next(ptr))
        {
            service_ptr_t<mainmenu_commands_v2> ptr_v2;
            ptr->service_query_t(ptr_v2);
            unsigned p_service_item_count = ptr->get_command_count();
            for (p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++)
            {
                if (p_guid == ptr->get_command(p_service_item_index))
                {
                    pfc::string8 name;
                    ptr->get_name(p_service_item_index, name);
                    if (p_subguid != pfc::guid_null && ptr_v2.is_valid() && ptr_v2->is_command_dynamic(p_service_item_index))
                    {
                        pfc::string8 name_sub;
                        mainmenu_node::ptr ptr_node = ptr_v2->dynamic_instantiate(p_service_item_index);
                        mainmenunode_subguid_to_path(ptr_node, p_subguid, name_sub, true);
                        name << "/" << name_sub;
                    }
                    if (!b_short)
                    {
                        pfc::list_t<pfc::string8> levels;
                        GUID parent = ptr->get_parent();
                        while (parent != pfc::guid_null)
                        {
                            pfc::string8 parentname;
                            if (maingroupname_from_guid(GUID(parent), parentname, parent))
                                levels.insert_item(parentname, 0);
                        }
                        unsigned i, count = levels.get_count();
                        for (i = 0; i<count; i++)
                        {
                            p_out.add_string(levels[i]);
                            p_out.add_byte('/');

                        }
                    }
                    p_out.add_string(name);
                }

            }
        }
    }
};
