#include "stdafx.h"

bool t_list_view::copy_selected_items_as_text(t_size default_single_item_column)
{
	pfc::string8 text, cleanedText;
	t_size selected_count = get_selection_count(2);
	if (selected_count == 0)
	{
		//return false;
	}
	else if (default_single_item_column != pfc_infinite && selected_count == 1)
	{
		t_size index = get_selected_item_single();
		if (index != pfc_infinite) text = get_item_text(index, default_single_item_column);
	}
	else
	{
		t_size column_count = get_column_count(), item_count = get_item_count();
		bit_array_bittable mask_selected(get_item_count());
		get_selection_state(mask_selected);
		bool b_first = true;
		for (t_size i = 0; i<item_count; i++)
			if (mask_selected[i]) 
			{
				if (!b_first) text << "\r\n";
				b_first = false;
				for (t_size j = 0; j<column_count; j++) 
					text << (j?"\t":"") << get_item_text(i,j);
			}
	}
	titleformat_compiler::remove_color_marks(text, cleanedText);
	uSetClipboardString(cleanedText);
	return selected_count > 0;
}

void t_list_view::set_sort_column (t_size index, bool b_direction)
{
	t_size headerIndex = index;
	if (m_have_indent_column && index != pfc_infinite)
		headerIndex++;
	HDITEM hdi; 
	memset(&hdi, 0, sizeof(HDITEM));

	hdi.mask = HDI_FORMAT;

	{
		int n,t=m_columns.get_count(),i=0;
		for(n = 0; n<t; n++)
		{
			Header_GetItem(m_wnd_header, n, &hdi);
			hdi.fmt &= ~(HDF_SORTUP|HDF_SORTDOWN);
			if (m_show_sort_indicators && n==headerIndex)
				hdi.fmt |= (b_direction ? HDF_SORTDOWN : HDF_SORTUP);
			Header_SetItem(m_wnd_header, n, &hdi);
		}
	}


	m_sort_column_index = index;
	m_sort_direction = b_direction;

	RedrawWindow(m_wnd_header, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
}

void t_list_view::set_autosize (bool b_val) 
{
	m_autosize=b_val;
	if (m_initialised)
	{
		/*if (m_wnd_header)
		{
			LONG_PTR style = GetWindowLongPtr(m_wnd_header, GWL_STYLE);
			SetWindowLongPtr(m_wnd_header, GWL_STYLE, b_val ? (style|0x0800) : (style & ~0x0800) );
		}*/
		update_column_sizes();
		update_header();
		invalidate_all();
		//RedrawWindow(get_wnd(), NULL, NULL, RDW_UPDATENOW|RDW_ALLCHILDREN);
		update_scroll_info();
	}
}
void t_list_view::set_always_show_focus (bool b_val) 
{
	m_always_show_focus=b_val;
	if (m_initialised)
		invalidate_all();
}
void t_list_view::on_size(bool b_update, bool b_update_scroll)
{
	RECT rc;
	GetClientRect(get_wnd(), &rc);
	on_size(RECT_CX(rc), RECT_CY(rc), b_update, b_update_scroll);
}

void t_list_view::on_size(int cxd, int cyd, bool b_update, bool b_update_scroll)
{
	if (!m_sizing)
	{
		pfc::vartoggle_t<bool> toggler(m_sizing, true);
		RECT rc_header;
		get_header_rect(&rc_header);

		RECT rc;
		GetClientRect(get_wnd(), &rc);
		int cx = RECT_CX(rc);
		int cy = RECT_CY(rc);

		t_size old_search_height = get_search_box_height();
		t_size new_search_height = m_search_editbox ? m_item_height+4 : 0;

		if (m_search_editbox)
		{
			SetWindowPos(m_search_editbox, NULL, 0, 0, cx, new_search_height, SWP_NOZORDER);
		}

		t_size new_header_height = calculate_header_height();
		if (new_header_height != RECT_CY(rc_header) && m_wnd_header)
			//Update height because affects scroll info
			SetWindowPos(m_wnd_header, NULL, -m_horizontal_scroll_position, new_search_height, cx+m_horizontal_scroll_position, new_header_height, SWP_NOZORDER);

		if (b_update_scroll)
			update_scroll_info(false);

		if (m_autosize)
			update_column_sizes();
		if (m_autosize)
			update_header();

		GetClientRect(get_wnd(), &rc);
		cx = RECT_CX(rc);
		cy = RECT_CY(rc);

		//Reposition again due to potential vertical scrollbar changes
		if (m_wnd_header)
			SetWindowPos(m_wnd_header, NULL, -m_horizontal_scroll_position, new_search_height, cx+m_horizontal_scroll_position, new_header_height, SWP_NOZORDER);
		
		if (m_search_editbox)
		{
			SetWindowPos(m_search_editbox, NULL, 0, 0, cx, new_search_height, SWP_NOZORDER);
		}

		if (new_header_height != RECT_CY(rc_header))
			RedrawWindow(m_wnd_header, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
		if (m_autosize || new_header_height != RECT_CY(rc_header) || get_search_box_height() != old_search_height)
			invalidate_all(false);
		if (b_update)
		{
			UpdateWindow(get_wnd());
		}
	}
}

void t_list_view::get_items_rect(LPRECT rc)
{
	GetClientRect(get_wnd(), rc);
	rc->top += get_header_height();
	rc->top += get_search_box_height();
	//InflateRect(rc, -1, -1);
	if (rc->bottom < rc->top)
		rc->bottom = rc->top;
}
void t_list_view::get_items_size(LPRECT rc)
{
	GetClientRect(get_wnd(), rc);
	rc->bottom -= get_header_height();
	rc->bottom -= get_search_box_height();
	if (rc->bottom < rc->top)
		rc->bottom = rc->top;
}
void t_list_view::reset_columns()
{
	//assert (m_items.get_count() == 0);
	//m_items.remove_all();
	m_columns.remove_all();
}

/*void t_list_view::add_column(const t_column & col)
{
	m_columns.add_item(col);
}*/
/*void t_list_view::add_item(const t_string_list_const_fast & text, const t_string_list_const_fast & p_groups, t_size size)
{
	insert_item(m_items.get_count(), text, p_groups, size);
}*/
void t_list_view::set_group_count(t_size count, bool b_update_columns)
{
	m_group_count = count;
	if (m_initialised && b_update_columns)
	{
		update_column_sizes();
		build_header();
		//update_header();
	}
}
void t_list_view::process_keydown(int offset, bool alt_down, bool repeat)
{
	int focus = get_focus_item();
	int count = m_items.get_count();

	if (focus == pfc_infinite)
		focus = 0;

	if (count)
	{
		if ((focus + offset) < 0) offset -= (focus + offset);
		if ((focus + offset) >= count) offset = (count-1-focus);

		bool focus_sel = get_item_selected(focus);

		if ((GetKeyState(VK_SHIFT) & KF_UP) && (GetKeyState(VK_CONTROL) & KF_UP))
		{
			//if (!repeat) playlist_api->activeplaylist_undo_backup();
			move_selection(offset);
		}
		else if ((GetKeyState(VK_CONTROL) & KF_UP))
			set_focus_item(focus + offset);
		else if (!m_single_selection && (GetKeyState(VK_SHIFT) & KF_UP))
		{
			t_size start = m_alternate_selection ? focus : m_shift_start;
			bit_array_range array_select(min(start, t_size(focus+offset)), abs(int(start - (focus+offset)))+1);
			set_selection_state(m_alternate_selection ? (bit_array&)array_select : (bit_array&)bit_array_true(), m_alternate_selection && !focus_sel ? (bit_array&)bit_array_not(array_select) : (bit_array&)array_select, true, false);
			set_focus_item(focus+offset, true, false);
			UpdateWindow(get_wnd());
		}
		else
		{
			set_item_selected_single(focus+offset);
		}
	}
}

int t_list_view::get_default_item_height()
{
	int ret = uGetFontHeight(m_font) + m_vertical_item_padding;
	if (ret < 1) ret = 1;
	return ret;
}

int t_list_view::get_default_group_height()
{
	int ret = uGetFontHeight(m_group_font) + m_vertical_item_padding;
	if (ret < 1) ret = 1;
	return ret;
}
void t_list_view::on_focus_change(t_size index_prev, t_size index_new, bool b_update_display)
{
	t_size count = m_items.get_count();
	if (index_prev < count)
		invalidate_items(index_prev, 1, b_update_display);
	if (index_new < count)
		invalidate_items(index_new, 1, b_update_display);
}
void t_list_view::invalidate_all(bool b_update, bool b_children)
{
	RECT rc;
	get_items_rect(&rc);
	RedrawWindow(get_wnd(), b_children || true ? NULL : &rc, NULL, RDW_INVALIDATE|(b_update?RDW_UPDATENOW:0)|(b_children?RDW_ALLCHILDREN:0));
}

void t_list_view::update_items(t_size index, t_size count, bool b_update_display)
{
	t_size i;
	for (i=0;i<count;i++)
		m_items[i+index]->m_subitems.remove_all();
	invalidate_items(index, count, b_update_display);
}

void t_list_view::update_all_items(bool b_update_display)
{
	update_items(0, m_items.get_count(), b_update_display);
}

void t_list_view::invalidate_items(t_size index, t_size count, bool b_update_display)
{
#if 0
		RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE|(b_update_display?RDW_UPDATENOW:0));
#else
	if (count)
	{
		//t_size header_height = get_header_height();
		RECT rc_client;
		get_items_rect(&rc_client);
		t_size groups = get_item_display_group_count(index);
		RECT rc_invalidate = {0, get_item_position(index) - m_scroll_position+rc_client.top - groups*m_group_height, RECT_CX(rc_client), get_item_position(index+count-1) - m_scroll_position + get_item_height(index+count-1) + rc_client.top};
		if (IntersectRect(&rc_invalidate, &rc_client, &rc_invalidate))
		{
			RedrawWindow(get_wnd(), &rc_invalidate, NULL, RDW_INVALIDATE|(b_update_display?RDW_UPDATENOW:0));
		}
	}
#endif
}
void t_list_view::get_item_group(t_size index, t_size level, t_size & index_start, t_size & count)
{
	if (m_group_count == 0)
	{
		index_start = 0;
		count = m_items.get_count();
	}
	else
	{
		t_size end = index, start = index;
		while (m_items[start]->m_groups[level] == m_items[index]->m_groups[level])
		{
			index_start =start;
			if (start == 0)
				break;
			start--;
		}
		while (end < m_items.get_count() && m_items[end]->m_groups[level] == m_items[index]->m_groups[level])
		{
			count = end-index_start+1;
			end++;
		}
	}
}

void t_list_view::set_highlight_item(t_size index)
{
	if (m_highlight_item_index != index)
	{
		m_highlight_item_index = index;
		invalidate_all();
	}
}

void t_list_view::remove_highlight_item()
{
	if (m_highlight_item_index != pfc_infinite)
	{
		m_highlight_item_index = pfc_infinite;
		invalidate_all();
	}
}

void t_list_view::set_highlight_selected_item(t_size index)
{
	if (m_highlight_selected_item_index != index)
	{
		m_highlight_selected_item_index = index;
		invalidate_all();
	}
}

void t_list_view::remove_highlight_selected_item()
{
	if (m_highlight_selected_item_index != pfc_infinite)
	{
		m_highlight_selected_item_index = pfc_infinite;
		invalidate_all();
	}
}

void t_list_view::set_insert_mark(t_size index)
{
	if (m_insert_mark_index != index)
	{
		m_insert_mark_index = index;
		invalidate_all();
	}
}
void t_list_view::remove_insert_mark()
{
	if (m_insert_mark_index != pfc_infinite)
	{
		m_insert_mark_index = pfc_infinite;
		invalidate_all();
	}
}

bool t_list_view::disable_redrawing()
{
	if (IsWindowVisible(get_wnd()))
	{
		SendMessage(get_wnd(), WM_SETREDRAW, FALSE, 0);
		return true;
	}
	return false;
}
void t_list_view::enable_redrawing()
{
	//if (IsWindowVisible(get_wnd()))
	{
		SendMessage(get_wnd(), WM_SETREDRAW, TRUE, 0);
		invalidate_all(true, true);
	}
}

void t_list_view::create_timer_search()
{
	destroy_timer_search();
	if (!m_timer_search)
	{
		SetTimer(get_wnd(), TIMER_END_SEARCH, 1000, NULL);
		m_timer_search = true;
	}
}
void t_list_view::destroy_timer_search()
{
	if (m_timer_search)
	{
		KillTimer(get_wnd(), TIMER_END_SEARCH);
		m_timer_search = false;
	}
}

void t_list_view::on_search_string_change(WCHAR c)
{
	bool b_all_same = true;
	pfc::string8 temp;
	temp.add_char(c);
	m_search_string.add_char(c);
	const char * ptr = m_search_string.get_ptr();
	if (*ptr)
	{
		ptr++;
		while (*ptr)
		{
			if (*ptr != *(ptr-1))
			{
				b_all_same = false;
				break;
			}
			ptr++;
		}
	}

	create_timer_search();
	t_size countk=m_columns.get_count();
	if (countk == 0)
	{
		destroy_timer_search();
		return;
	}

	t_size focus = get_focus_item();
	t_size i=0,count=m_items.get_count();
	if (focus == pfc_infinite || focus > m_items.get_count())
		focus = 0;
	else if (b_all_same)
	{
		if (focus + 1 == count)
			focus = 0;
		else
			focus++;
	}
	for (i=0; i<count; i++)
	{
		t_size j = (i+focus)%count;
		t_item_ptr item = m_items[j];

		const char * p_compare = get_item_text(j,0);
		pfc::string8 compare_noccodes;

		if (strchr(p_compare, 3))
		{
			titleformat_compiler::remove_color_marks(p_compare, compare_noccodes);
			p_compare = compare_noccodes;
		}

		if ((b_all_same && !stricmp_utf8_partial(p_compare, temp)) || !stricmp_utf8_partial(p_compare, m_search_string)) 
		{
			if (!is_visible(j))
			{
				scroll(false, get_item_position(j));
			}
			set_item_selected_single(j);
			break;
		}
	}
}
