#pragma once

/**
* Various helpers used to construct menu item paths for display. Used by buttons toolbar.
*/

#include "foo_ui_columns.h"

namespace menu_helpers
{
	void contextpath_from_guid(const GUID & p_guid, const GUID & p_subcommand, pfc::string_base & p_out, bool b_short = false);
	bool maingroupname_from_guid(const GUID & p_guid, pfc::string_base & p_out, GUID & parentout);
	bool mainmenunode_subguid_to_path(const mainmenu_node::ptr & ptr_node, const GUID & p_subguid, pfc::string8 & p_out, bool b_is_root = false);
	void mainpath_from_guid(const GUID & p_guid, const GUID & p_subguid, pfc::string_base & p_out, bool b_short = false);
};
