#include "stdafx.h"
#include "item_properties.h"

void fields_list_view_t::notify_save_inline_edit(const char * value)
{
	if (m_edit_index < m_fields.get_count())
	{
		(m_edit_column ? m_fields[m_edit_index].m_name : m_fields[m_edit_index].m_name_friendly) = value;
		pfc::list_t<t_list_view::t_item_insert_sized<2, 0>> items;
		items.set_count(1);
		items[0].m_subitems[0] = m_fields[m_edit_index].m_name_friendly;
		items[0].m_subitems[1] = m_fields[m_edit_index].m_name;
		replace_items(m_edit_index, items);
	}
	m_edit_column = pfc_infinite;
	m_edit_index = pfc_infinite;
}

bool fields_list_view_t::notify_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, pfc::string_base & p_text, t_size & p_flags, mmh::comptr_t<IUnknown> & pAutocompleteEntries)
{
	t_size indices_count = indices.get_count();
	if (indices_count == 1 && indices[0] < m_fields.get_count())
	{
		m_edit_index = indices[0];
		m_edit_column = column;

		p_text = m_edit_column ? m_fields[m_edit_index].m_name : m_fields[m_edit_index].m_name_friendly;

		if (m_edit_column == 1)
			p_flags = inline_edit_uppercase;

		return true;
	}
	return false;
}

bool fields_list_view_t::notify_before_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, bool b_source_mouse)
{
	if (column <= 1 && indices.get_count() == 1)
		return true;
	return false;
}

void fields_list_view_t::notify_on_create()
{
	set_single_selection(true);
	pfc::list_t<t_column> columns;
	columns.set_count(2);
	columns[0].m_title = "Name";
	columns[0].m_size = 150;
	columns[1].m_title = "Field";
	columns[1].m_size = 150;
	t_list_view::set_columns(columns);

	t_size count = m_fields.get_count();
	pfc::list_t<t_list_view::t_item_insert> items;
	get_insert_items(0, count, items);
	insert_items(0, count, items.get_ptr());
}

void fields_list_view_t::get_insert_items(t_size base, t_size count, pfc::list_t<t_list_view::t_item_insert> & items)
{
	t_size i;
	items.set_count(count);
	for (i = 0; i < count; i++)
	{
		items[i].m_subitems.set_size(2);
		items[i].m_subitems[0] = m_fields[base + i].m_name_friendly;
		items[i].m_subitems[1] = m_fields[base + i].m_name;
	}
}

fields_list_view_t::fields_list_view_t(pfc::list_t<field_t> & p_fields) : m_edit_index(pfc_infinite), m_edit_column(pfc_infinite), m_fields(p_fields)
{

}
