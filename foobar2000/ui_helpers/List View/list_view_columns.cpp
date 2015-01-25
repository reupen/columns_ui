#include "stdafx.h"

t_size t_list_view::get_columns_width()
{
	t_size i, count = m_columns.get_count(), ret = 0;
	for (i=0; i<count; i++)
		ret += m_columns[i].m_size;
	return ret;
}
t_size t_list_view::get_columns_display_width()
{
	t_size i, count = m_columns.get_count(), ret = 0;
	for (i=0; i<count; i++)
		ret += m_columns[i].m_display_size;
	return ret;
}
t_size t_list_view::get_column_display_width(t_size index)
{
	t_size ret = 0;
	assert(index < get_column_count());
	if (index < get_column_count())
		ret = m_columns[index].m_display_size;
	return ret;
}
t_size t_list_view::get_column_count()
{
	return m_columns.get_count();
}

ui_helpers::alignment t_list_view::get_column_alignment(t_size index)
{
	ui_helpers::alignment  ret = ui_helpers::ALIGN_LEFT;
	assert(index < get_column_count());
	if (index < get_column_count())
		ret = m_columns[index].m_alignment;
	return ret;
}

void t_list_view::set_columns(const pfc::list_base_const_t<t_column> & columns)
{
	reset_columns();
	m_columns.add_items(columns);
	update_column_sizes();
	if (m_initialised)
	{
		build_header();
		_update_scroll_info_horizontal();
		on_size(false, false);
		UpdateWindow(get_wnd());
	}
}

void t_list_view::set_column_widths(const pfc::list_base_const_t<t_size> & widths)
{
	t_size i, count = m_columns.get_count();
	for (i=0; i<count; i++)
		m_columns[i].m_size = widths[i];
	update_column_sizes();
	if (m_wnd_header)
		SendMessage(m_wnd_header, WM_SETREDRAW, FALSE, NULL);
	update_header();
	if (m_wnd_header)
		SendMessage(m_wnd_header, WM_SETREDRAW, TRUE, NULL);
	invalidate_all(false);
	on_size();
	RedrawWindow(get_wnd(), 0,0,RDW_ALLCHILDREN|RDW_UPDATENOW);
}

void t_list_view::get_column_sizes (pfc::list_t<t_column> & p_out)
{
	//console::formatter() << "get_column_sizes";
	RECT rc;
	get_items_rect(&rc);
	t_size display_width = RECT_CX(rc), width = get_columns_width(), total_weight=0, indent=get_total_indentation();
	if (display_width > indent) display_width -= indent;
	else display_width=0;
	t_size i, count = p_out.get_count();
	//p_out.set_count(count);
	for (i=0; i<count; i++)
		p_out[i].m_display_size = p_out[i].m_size;

	if (m_autosize)
	{
		for (i=0; i<count; i++)
			total_weight += p_out[i].m_autosize_weight;

		pfc::array_t<bool> sized;
		pfc::array_t<t_ssize> deltas;
		sized.set_count(count);
		deltas.set_count(count);
		sized.fill_null();
		deltas.fill_null();
		t_size sized_count = count;
		int width_difference = display_width - width;
		while (width_difference && total_weight && sized_count)
		{
			//console::formatter() << "width_difference: " << width_difference << " total_weight: " << total_weight << " sized_count: " << sized_count;
			t_ssize width_difference_local = width_difference;
			t_size total_weight_local = total_weight;
			for (i=0; i<count; i++)
			{
				if (!sized[i] && total_weight_local)
				{
					deltas[i] = MulDiv(p_out[i].m_autosize_weight, width_difference_local, total_weight_local);
					width_difference_local -= deltas[i];
					total_weight_local -= p_out[i].m_autosize_weight;
				}
			}
			for (i=0; i<count; i++)
			{
				if (!sized[i])
				{
					t_ssize delta = deltas[i];
					//console::formatter() << "col: " << i << " delta: " << delta << " old size: " << p_out[i].m_display_size;
					if ((t_ssize)p_out[i].m_display_size + delta <= 0)
					{
						total_weight -= p_out[i].m_autosize_weight;
						sized[i] = true;
						sized_count--;
						width_difference += p_out[i].m_display_size;
						p_out[i].m_display_size = 0;
					}
					else
					{
						p_out[i].m_display_size += delta;
						width_difference -= delta;
					}
				}
			}
		}
	}
	//else
	//	for (i=0; i<count; i++)
	//		p_out[i] = m_columns[i].m_size;

	//if (count)
	//	p_out[0].m_display_size += indent;
}
void t_list_view::update_column_sizes()
{
	{
		get_column_sizes(m_columns);
	}
}
