#pragma once

/*!
 * \file menu_helpers.h
 *
 * \author musicmusic
 * \date March 2015
 *
 * Various helpers used to construct menu item paths for display. Used by buttons toolbar.
 */

#include "stdafx.h"
#include "common.h"

namespace menu_helpers {
void contextpath_from_guid(const GUID& p_guid, const GUID& p_subcommand, pfc::string_base& p_out, bool b_short = false);
bool maingroupname_from_guid(const GUID& p_guid, pfc::string_base& p_out, GUID& parentout);
bool mainmenunode_subguid_to_path(
    const mainmenu_node::ptr& ptr_node, const GUID& p_subguid, pfc::string8& p_out, bool b_is_root = false);
void mainpath_from_guid(const GUID& p_guid, const GUID& p_subguid, pfc::string_base& p_out, bool b_short = false);
}; // namespace menu_helpers

struct menu_item_identifier {
    GUID m_command{};
    GUID m_subcommand{};
};

bool operator==(const menu_item_identifier& p1, const menu_item_identifier& p2);
bool operator!=(const menu_item_identifier& p1, const menu_item_identifier& p2);

class menu_item_cache {
    class menu_item_info : public menu_item_identifier {
    public:
        pfc::string8 m_name;
        pfc::string8 m_desc;
    };

public:
    menu_item_cache();
    const menu_item_info& get_item(unsigned n) const;
    unsigned get_count() { return m_data.get_count(); }

private:
    pfc::ptr_list_t<menu_item_info> m_data;
};
