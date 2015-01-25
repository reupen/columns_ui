#include "stdafx.h"

void t_list_view::get_selection_state(bit_array_var & out)
{
	storage_get_selection_state(out);
}

void t_list_view::set_selection_state(const bit_array & p_affected,const bit_array & p_status, bool b_notify, bool b_update_display, notification_source_t p_notification_source) 
{
	bit_array_bittable p_changed(get_item_count());
	if (storage_set_selection_state(p_affected,p_status, &p_changed))
	{
		invalidate_items(p_changed, b_update_display);
		//RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE|(b_update_display?RDW_UPDATENOW:0));
		if (b_notify)
			notify_on_selection_change(p_changed, p_status, p_notification_source);
	}
}

t_size t_list_view::get_focus_item() {t_size ret = storage_get_focus_item(); if (ret >= get_item_count()) ret = pfc_infinite; return ret;}

void t_list_view::set_focus_item(t_size index, bool b_notify, bool b_update_display) 
{
	t_size old = storage_get_focus_item();
	if (old != index)
	{
		storage_set_focus_item(index);
		on_focus_change(old, index, b_update_display);
		if (b_notify)
			notify_on_focus_item_change(index);
	}
};

bool t_list_view::get_item_selected(t_size index)
{
	return storage_get_item_selected(index);
}

t_size t_list_view::get_selection_count(t_size max)
{
	return storage_get_selection_count(max);
}


t_size t_list_view::storage_get_selection_count(t_size max)
{
	t_size i, count = m_items.get_count(), ret = 0;;
	for (i=0; i<count; i++)
	{
		if (get_item_selected(i))
			ret++;
		if (ret == max)
			break;
	}
	return ret;

}

void t_list_view::set_item_selected(t_size index, bool b_state)
{
	if (b_state)
		set_selection_state(bit_array_one(index), bit_array_one(index));
	else
		set_selection_state(bit_array_one(index), bit_array_false());
}
void t_list_view::set_item_selected_single(t_size index, bool b_notify, notification_source_t p_notification_source)
{
	if (index < m_items.get_count())
	{
		set_selection_state(bit_array_true(), bit_array_one(index), b_notify, false, p_notification_source);
		set_focus_item(index, b_notify, false);
		UpdateWindow(get_wnd());
		//ensure_visible(index);
	}
}

//DEFAULT STORAGE
t_size t_list_view::storage_get_focus_item() {return m_focus_index;}

void t_list_view::storage_set_focus_item(t_size index) 
{
	m_focus_index = index;
};

void t_list_view::storage_get_selection_state(bit_array_var & out) //storage
{
	t_size i, count = m_items.get_count();
	for (i=0; i<count; i++)
		out.set(i,m_items[i]->m_selected);
}

bool t_list_view::storage_set_selection_state(const bit_array & p_affected,const bit_array & p_status, bit_array_var * p_changed) //storage, returns hint if sel actually changed
{
	bool b_changed = false;
	t_size i, count = m_items.get_count();
	for (i=0; i<count; i++)
	{
		if (p_affected[i] && p_status[i] != get_item_selected(i))
		{
			b_changed = true;
			m_items[i]->m_selected = p_status[i];
			if (p_changed) p_changed->set(i, true);
		}
	}
	return b_changed;
}

bool t_list_view::storage_get_item_selected(t_size index)
{
	return m_items[index]->m_selected;
}
