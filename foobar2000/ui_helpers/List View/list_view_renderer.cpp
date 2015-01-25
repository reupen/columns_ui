#include "stdafx.h"

const t_size _level_spacing_size = 3;

t_size t_list_view::get_item_indentation()
{
	RECT rc; get_items_rect(&rc); 
	t_size ret = rc.left;
	if (m_group_count)
		ret += get_default_indentation_step()*m_group_count;
	return ret;
}
t_size t_list_view::get_default_indentation_step()
{
	t_size ret = 0;
	if (m_group_level_indentation_enabled)
	{
		HDC dc = GetDC(get_wnd());
		HFONT font_old = SelectFont(dc, m_font);
		const t_size cx_space = ui_helpers::get_text_width(dc, " ", 1);
		SelectFont(dc, font_old);
		ReleaseDC(get_wnd(), dc);
		ret = cx_space*_level_spacing_size;
	}
	return ret;
}

void t_list_view::render_items(HDC dc, const RECT & rc_update, t_size cx)
{
	const t_size level_spacing_size = m_group_level_indentation_enabled ? _level_spacing_size : 0;
	//COLORREF cr_orig = GetTextColor(dc);
	//OffsetWindowOrgEx(dc, m_horizontal_scroll_position, 0, NULL);
	t_size highlight_index = get_highlight_item() ;
	t_size index_focus = get_focus_item();
	HWND wnd_focus = GetFocus();
	bool b_show_focus = (SendMessage(get_wnd(), WM_QUERYUISTATE, NULL, NULL) & UISF_HIDEFOCUS) == 0;
	bool b_window_focused = (wnd_focus == get_wnd()) || IsChild(get_wnd(), wnd_focus);

	render_background(dc, &rc_update);
	RECT rc_items;
	get_items_rect(&rc_items);

	RECT rc_dummy;
	if (rc_update.bottom<=rc_update.top || rc_update.bottom < rc_items.top)
		return;

	t_size i, count = m_items.get_count();
	const t_size cx_space = ui_helpers::get_text_width(dc, " ", 1);
	const t_size item_preindentation = cx_space*level_spacing_size*m_group_count + rc_items.left;
	const t_size item_indentation = item_preindentation + get_group_info_area_total_width();
	cx = get_columns_display_width() + item_indentation;

	bool b_show_group_info_area  = get_show_group_info_area();

	i=get_previous_item((rc_update.top>rc_items.top ? rc_update.top-rc_items.top : 0) + m_scroll_position, true);
	t_size i_start = i;
	t_size i_end = get_previous_item((rc_update.bottom>rc_items.top+1 ? rc_update.bottom-rc_items.top-1 : 0) + m_scroll_position, true);
	for (; i<=i_end,i<count; i++)
	{

		HFONT fnt_old = SelectFont(dc, m_group_font.get());
		t_size item_group_start=NULL, item_group_count=NULL;
		get_item_group(i, m_group_count ? m_group_count-1 : 0, item_group_start, item_group_count);
		
		t_size j, countj = m_items[i]->m_groups.get_count();
		t_size counter = 0;
		for (j=0; j<countj; j++)
		{
			if (!i || m_items[i]->m_groups[j] != m_items[i-1]->m_groups[j])
			{
				t_group_ptr p_group = m_items[i]->m_groups[j];
				t_size y = get_item_position(i)-m_scroll_position - m_group_height*(countj-j) + rc_items.top;
				t_size x = -m_horizontal_scroll_position + rc_items.left;
				//y += counter*m_item_height;
				RECT rc = {x, y, x+cx, y+m_group_height};

				if (rc.top >= rc_update.bottom) 
				{
					//OffsetWindowOrgEx(dc, -m_horizontal_scroll_position, 0, NULL);
					break;// CRUDE 
				}
				render_group(dc, i, j, p_group->m_text, cx_space*level_spacing_size, j, rc);

				counter++;
			}
		}

		SelectFont(dc, fnt_old);

		if (b_show_group_info_area && (i==i_start || i == item_group_start))
		{
			t_size height = max (m_item_height*item_group_count, get_group_info_area_height());
			int gx = 0-m_horizontal_scroll_position+item_preindentation;
			int gy = get_item_position(item_group_start)-m_scroll_position+rc_items.top;
			int gcx = get_group_info_area_width();
			RECT rc_group_info = {gx, gy, gx + gcx, get_item_position(item_group_start)+height-m_scroll_position+rc_items.top};
			if (rc_group_info.top >= rc_update.bottom) 
				break;
			render_group_info(dc, item_group_start, item_group_count, rc_group_info);

			//console::printf("%u %u %u %u; %u %u %u %u",rc_group_info,rc_update);
		}

		bool b_selected = get_item_selected(i) || i == m_highlight_selected_item_index;

		t_item_ptr item = m_items[i];

		RECT rc = {0-m_horizontal_scroll_position+item_indentation, get_item_position(i)-m_scroll_position+rc_items.top, cx-m_horizontal_scroll_position, get_item_position(i)+get_item_height(i)-m_scroll_position+rc_items.top};
		//rc.left += cx_space*level_spacing_size*countj;
		if (rc.top >= rc_update.bottom) 
		{
			//OffsetWindowOrgEx(dc, -m_horizontal_scroll_position, 0, NULL);
			break;// CRUDE 
		}
		if (rc.bottom > rc_update.top)
		{
			render_item(dc, i, 0/*item_indentation*/, b_selected, b_window_focused,
				(m_highlight_item_index == i) || (highlight_index == i), ((b_show_focus && b_window_focused) || m_always_show_focus) && index_focus == i, 
				&rc);
			/*if (i == m_insert_mark_index || i + 1 == m_insert_mark_index)
			{
				gdi_object_t<HPEN>::ptr_t pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
				HPEN pen_old = SelectPen(dc, pen);
				int yPos = i == m_insert_mark_index ? rc.top-counter*m_item_height : rc.bottom-1;
				MoveToEx(dc, item_indentation, yPos, NULL);
				LineTo(dc, rc.right, yPos);
				SelectPen(dc, pen_old);				
			}*/
		}

	}
	/*if (m_search_editbox)
	{
		RECT rc_search;
		get_search_box_rect(&rc_search);
		rc_search.right = rc_search.left;
		rc_search.left = 0;
		if (rc_update.bottom >= rc_search.top)
		{
			FillRect(dc, &rc_search, GetSysColorBrush(COLOR_BTNFACE));
			//render_background(dc, &rc_search);
			ui_helpers::text_out_colours_tab(dc, m_search_label.get_ptr(), m_search_label.get_length(), 0, 2, &rc_search, false, GetSysColor(COLOR_WINDOWTEXT), false, false, false, ui_helpers::ALIGN_LEFT, NULL);
		}
	}*/
	if (m_insert_mark_index != pfc_infinite && (m_insert_mark_index <= count))
	{
		RECT rc_line, rc_dummy;
		rc_line.left = item_indentation;
		int yPos = 0;
		if (count)
		{
			if (m_insert_mark_index == count)
				yPos = get_item_position(count-1) + get_item_height(count-1) - m_scroll_position + rc_items.top - 1;
			else
				yPos = get_item_position(m_insert_mark_index) - m_scroll_position + rc_items.top - 1;
		}
		rc_line.top = yPos;
		rc_line.right = cx;
		rc_line.bottom = yPos + 2;
		if (IntersectRect(&rc_dummy, &rc_line, &rc_update))
		{
			FillRect(dc, &rc_line, GetSysColorBrush(COLOR_WINDOWTEXT));
		}
		/*gdi_object_t<HBRUSH>::ptr_t pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
		HPEN pen_old = SelectPen(dc, pen);
		MoveToEx(dc, item_indentation, yPos, NULL);
		LineTo(dc, cx, yPos);
		SelectPen(dc, pen_old);				*/
	}
	//OffsetWindowOrgEx(dc, -m_horizontal_scroll_position, 0, NULL);
}
	void t_list_view::render_get_colour_data(colour_data_t & p_out)
	{
		p_out.m_themed = true;
		p_out.m_use_custom_active_item_frame = false;
		p_out.m_text = GetSysColor(COLOR_WINDOWTEXT);
		p_out.m_selection_text = GetSysColor(COLOR_HIGHLIGHTTEXT);
		p_out.m_background = GetSysColor(COLOR_WINDOW);
		p_out.m_selection_background = GetSysColor(COLOR_HIGHLIGHT);
		p_out.m_inactive_selection_text = GetSysColor(COLOR_BTNTEXT);
		p_out.m_inactive_selection_background = GetSysColor(COLOR_BTNFACE);
		p_out.m_active_item_frame = GetSysColor(COLOR_WINDOWFRAME);
		p_out.m_group_text = get_group_text_colour_default();
		p_out.m_group_background = p_out.m_background;
	}
	void t_list_view::render_group_line_default(const colour_data_t & p_data, HDC dc, const RECT * rc)
	{
		if (m_theme && IsThemePartDefined(m_theme, LVP_GROUPHEADERLINE, NULL) && SUCCEEDED(DrawThemeBackground(m_theme, dc, LVP_GROUPHEADERLINE, LVGH_OPEN, rc, NULL)))
		{
		}
		else
		{
			COLORREF cr = p_data.m_group_text;//get_group_text_colour_default();
			gdi_object_t<HPEN>::ptr_t pen = CreatePen(PS_SOLID, 1, cr);
			HPEN pen_old = SelectPen(dc, pen);
			MoveToEx(dc, rc->left, rc->top, NULL);
			LineTo(dc, rc->right, rc->top);
			SelectPen(dc, pen_old);
		}
	}
	void t_list_view::render_group_background_default(const colour_data_t & p_data, HDC dc, const RECT * rc)
	{
		FillRect(dc, rc, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(p_data.m_group_background)));
	}
	COLORREF t_list_view::get_group_text_colour_default()
	{
		COLORREF cr = NULL;
		if (!(m_theme && IsThemePartDefined(m_theme, LVP_GROUPHEADER, NULL) && SUCCEEDED(GetThemeColor(m_theme, LVP_GROUPHEADER, LVGH_OPEN, TMT_HEADING1TEXTCOLOR, &cr))))
			cr = GetSysColor(COLOR_WINDOWTEXT);
		return cr;
	}
	bool t_list_view::get_group_text_colour_default(COLORREF & cr)
	{
		cr = NULL;
		if (!(m_theme && IsThemePartDefined(m_theme, LVP_GROUPHEADER, NULL) && SUCCEEDED(GetThemeColor(m_theme, LVP_GROUPHEADER, LVGH_OPEN, TMT_HEADING1TEXTCOLOR, &cr))))
			return false;
		return true;
	}
	void t_list_view::render_group_default(const colour_data_t & p_data, HDC dc, const char * text, t_size indentation, t_size level, const RECT & rc)
	{
		COLORREF cr = p_data.m_group_text;

		unsigned text_width = NULL;

		render_group_background_default(p_data, dc, &rc);
		ui_helpers::text_out_colours_tab(dc, text, strlen(text), 2 + indentation*level, 2, &rc, false, cr, false, false, true, ui_helpers::ALIGN_LEFT, NULL, true, true, &text_width);

		t_size cx = text_width;

		RECT rc_line = {cx+7, rc.top+RECT_CY(rc)/2, rc.right-4, rc.top+RECT_CY(rc)/2+1};

		if (rc_line.right > rc_line.left)
		{
			render_group_line_default(p_data, dc, &rc_line);
		}
	}
	void t_list_view::render_item_default(const colour_data_t & p_data, HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc)
	{
		t_item_ptr item = m_items[index];
		int theme_state = NULL;
		if (b_selected)
			theme_state = (b_highlight ? LISS_HOTSELECTED : (b_window_focused ? LISS_SELECTED : LISS_SELECTEDNOTFOCUS));
		else if (b_highlight)
			theme_state = LISS_HOT;

		//NB Third param of IsThemePartDefined "must be 0". But this works.
		bool b_themed = m_theme && p_data.m_themed && IsThemePartDefined(m_theme, LVP_LISTITEM, theme_state);

		COLORREF cr_text = NULL;
		if (b_themed && theme_state)
		{
			cr_text = GetThemeSysColor(m_theme, b_selected ? COLOR_BTNTEXT : COLOR_WINDOWTEXT);;
			{
				if (IsThemeBackgroundPartiallyTransparent(m_theme, LVP_LISTITEM, theme_state))
					DrawThemeParentBackground(get_wnd(), dc, rc);
				DrawThemeBackground(m_theme, dc, LVP_LISTITEM, theme_state, rc, NULL);
			}
		}
		else
		{
			cr_text = b_selected ? (b_window_focused ? p_data.m_selection_text :p_data.m_inactive_selection_text) : p_data.m_text;
			FillRect(dc, rc, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(b_selected ? (b_window_focused ? p_data.m_selection_background : p_data.m_inactive_selection_background) : p_data.m_background)));
		}
		RECT rc_subitem = *rc;
		t_size k, countk=m_columns.get_count();

		for (k=0; k<countk; k++)
		{
			rc_subitem.right = rc_subitem.left + m_columns[k].m_display_size;
			ui_helpers::text_out_colours_tab(dc, get_item_text(index,k), strlen(get_item_text(index,k)), 1+(k==0?indentation:0), 3, &rc_subitem, b_selected, cr_text, true, true, true, m_columns[k].m_alignment);
			rc_subitem.left = rc_subitem.right;
		}
		if (b_focused)
		{
			RECT rc_focus = *rc;
			if (m_theme && IsThemePartDefined(m_theme, LVP_LISTITEM, LISS_SELECTED))
				InflateRect(&rc_focus, -1, -1);
			if (p_data.m_use_custom_active_item_frame)
				FrameRect(dc, &rc_focus, gdi_object_t<HBRUSH>::ptr_t (CreateSolidBrush(p_data.m_active_item_frame)));
			else
				DrawFocusRect(dc, &rc_focus);
		}
	}
	void t_list_view::render_background_default(const colour_data_t & p_data, HDC dc, const RECT * rc)
	{
		FillRect(dc, rc, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(p_data.m_background)));
	}

void t_list_view::render_group(HDC dc, t_size index, t_size group, const char * text, t_size indentation, t_size level, const RECT & rc)
{
	colour_data_t p_data;
	render_get_colour_data(p_data);
	render_group_default(p_data, dc, text, indentation, level, rc);
}
void t_list_view::render_item(HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc)
{
	colour_data_t p_data;
	render_get_colour_data(p_data);
	render_item_default(p_data, dc, index, indentation, b_selected, b_window_focused, b_highlight, b_focused, rc);
}
void t_list_view::render_background(HDC dc, const RECT * rc)
{
	colour_data_t p_data;
	render_get_colour_data(p_data);
	render_background_default(p_data, dc, rc);
}

t_size t_list_view::get_text_width(const char * text, t_size length)
{
	t_size ret = 0;
	HDC hdc = GetDC(get_wnd());
	if (hdc)
	{
		HFONT fnt_old = SelectFont(hdc, m_font);
		ret = ui_helpers::get_text_width(hdc, text, strlen(text));
		SelectFont(hdc, fnt_old);
		ReleaseDC(get_wnd(), hdc);
	}
	return ret;
}


bool t_list_view::is_item_clipped(t_size index, t_size column)
{
	HDC hdc = GetDC(get_wnd());
	if (!hdc) return false;

	pfc::string8 text = get_item_text(index,column);
	HFONT fnt_old = SelectFont(hdc, m_font);
	t_size width = ui_helpers::get_text_width_color(hdc, text, text.length());
	SelectFont(hdc, fnt_old);
	ReleaseDC(get_wnd(), hdc);
	unsigned col_width = m_columns[column].m_display_size;
	//if (column == 0) width += get_total_indentation();

	return (width+7 > col_width);
	//return (width+2+(columns[col]->align == ALIGN_LEFT ? 2 : columns[col]->align == ALIGN_RIGHT ? 1 : 0) > col_width);//we use 3 for the spacing, 1 for column divider
}
