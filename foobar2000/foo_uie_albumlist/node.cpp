#include "stdafx.h"

void node::sort_children()
{
	pfc::list_t<pfc::string_simple_t<WCHAR> > sortdata;
	mmh::permutation_t permutation(children.get_count());
	sortdata.set_size(children.get_count());
	t_size n;
	for (n = 0; n<children.get_count(); n++)
		sortdata[n] = pfc::stringcvt::string_wide_from_utf8(children[n]->value);
	mmh::g_sort_get_permutation_qsort(sortdata, permutation, sortproc, false);
	children.reorder(permutation.get_ptr());
	for (n = 0; n<children.get_count(); n++)
		children[n]->sort_children();
}

void node::sort_entries()//for contextmenu
{
	if (!m_sorted)
	{
		//			entries.sort_by_pointer_remove_duplicates();
		if (b_bydir) entries.sort_by_path();
		/*else if (cfg_sorttree)
		{
		entries.sort_by_format(cfg_sort_order,0);
		}*/
		else
		{
			string8 temp = p_dbe->get_hierarchy();
			temp += "|%path%";
			entries.sort_by_format(temp, 0);
		}
		m_sorted = true;
	}
}

void node::create_new_playlist()
{
	static_api_ptr_t<playlist_manager> api;
	string8 name = value;
	if (name.is_empty()) name = "All music";
	unsigned idx = api->create_playlist(name, pfc_infinite, pfc_infinite);
	if (idx != pfc_infinite)
	{
		api->set_active_playlist(idx);
		send_to_playlist(true);
	}
}

void node::send_to_playlist(bool replace)
{
	static_api_ptr_t<playlist_manager> api;
	const bool select = !!cfg_add_items_select;
	api->activeplaylist_undo_backup();
	if (replace) api->activeplaylist_clear();
	else if (select) api->activeplaylist_clear_selection();
	if (cfg_add_items_use_core_sort) api->activeplaylist_add_items_filter(entries, select);
	else
	{
		sort_entries();
		api->activeplaylist_add_items(entries, bit_array_val(select));
	}

	if (select && !replace)
	{
		unsigned num = api->activeplaylist_get_item_count();
		if (num > 0)
		{
			api->activeplaylist_set_focus_item(num - 1);
		}
	}
}

void node::remove_from_db() const
{
	static_api_ptr_t<library_manager> api;
	api->remove_items(entries);
}

node::node(const char * p_value, unsigned p_value_len, album_list_window * dbe) : p_dbe(dbe), m_ti(NULL), m_label_dirty(false)
{
	if (p_value && p_value_len > 0)
	{
		value.set_string(p_value, p_value_len);
	}
	m_sorted = false;
}


void node::remove_entries(pfc::bit_array & mask)
{
	entries.remove_mask(mask);
}

void node::set_data(const list_base_const_t<metadb_handle_ptr>& p_data, bool b_keep_existing)
{
	if (!b_keep_existing)
		entries.remove_all();
	entries.add_items(p_data);
	m_sorted = false;
}

node_ptr node::find_or_add_child(const char * p_value, unsigned p_value_len, bool b_find, bool & b_new)
{
	if (!b_find)
		return add_child_v2(p_value, p_value_len);
	if (p_value_len == 0 || *p_value == 0)
	{
		p_value = "?"; p_value_len = 1;
	}
	t_size index = 0;
	b_new = true;
	if (/*b_find && */children.bsearch_t(g_compare_name, uTS(p_value, p_value_len), index))
	{
		b_new = false;
	}
	else
		children.insert_item(new node(/*this,*/p_value, p_value_len, p_dbe), index);
	return children[index];
}

node_ptr node::add_child_v2(const char * p_value, unsigned p_value_len)
{
	if (p_value_len == 0 || *p_value == 0)
	{
		p_value = "?"; p_value_len = 1;
	}
	node * temp = new node(/*this,*/p_value, p_value_len, p_dbe);
	children.add_item(temp);
	return temp;
}

void node::mark_all_labels_dirty()
{
	m_label_dirty = true;
	t_size i = children.get_count();
	for (; i; i--)
	{
		children[i - 1]->mark_all_labels_dirty();
	}
}

void node::purge_empty_children(HWND wnd)
{
	t_size i = children.get_count(), index_first_removed = pfc_infinite;
	for (; i; i--)
	{
		if (!children[i - 1]->get_entries().get_count())
		{
			if (p_dbe && p_dbe->p_selection == children[i - 1])
				p_dbe->p_selection = NULL;
			TreeView_DeleteItem(wnd, children[i - 1]->m_ti);
			children.remove_by_idx(i - 1);
			index_first_removed = i - 1;
		}
	}
	if (index_first_removed != pfc_infinite && cfg_show_numbers)
		m_label_dirty = true;

	if (index_first_removed != pfc_infinite && cfg_show_numbers2)
	{
		t_size count = children.get_count();
		for (i = index_first_removed; i<count; i++)
		{
			children[i]->m_label_dirty = true;
		}
	}
}
