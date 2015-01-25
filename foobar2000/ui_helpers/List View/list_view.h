#ifndef _LIST_PVT_H_
#define _LIST_PVT_H_

class t_list_view : public ui_helpers::container_window
{
public:
	enum 
	{
		IDC_HEADER = 1001,
		IDC_TOOLTIP = 1002,
		IDC_INLINEEDIT = 667,
		IDC_SEARCHBOX = 668,
	};
	enum {TIMER_SCROLL_UP = 1001,TIMER_SCROLL_DOWN = 1002,TIMER_END_SEARCH,EDIT_TIMER_ID,TIMER_BASE};
	enum {MSG_KILL_INLINE_EDIT = WM_USER + 3};
	class t_column
	{
	public:
		pfc::string8 m_title;
		t_size m_size;
		t_size m_display_size;
		t_size m_autosize_weight;
		ui_helpers::alignment m_alignment;

		t_column(const char * title, t_size cx, t_size p_autosize_weight = 1, ui_helpers::alignment alignment = ui_helpers::ALIGN_LEFT)
			: m_title(title), m_size(cx), m_display_size(cx), m_autosize_weight(p_autosize_weight),
			m_alignment(alignment)
		{};
		t_column() : m_size(0), m_alignment(ui_helpers::ALIGN_LEFT), m_autosize_weight(1), m_display_size(0) {};
	};
	typedef pfc::list_t<pfc::string8_fast_aggressive, pfc::alloc_fast_aggressive> t_string_list_fast;
	typedef pfc::list_base_const_t<pfc::string8_fast_aggressive> t_string_list_const_fast;
	typedef const pfc::list_base_const_t<pfc::string8_fast_aggressive> & t_string_list_cref_fast;

	class t_item_insert
	{
	public:
		t_string_list_fast m_subitems;
		t_string_list_fast m_groups;
		t_item_insert() {};
		t_item_insert(const t_string_list_const_fast & text, const t_string_list_const_fast & p_groups)
		{
		m_subitems.add_items(text);
		m_groups.add_items(p_groups);
		};
	};

private:
	t_list_view::class_data & t_list_view::get_class_data() const 
	{
		__implement_get_class_data_ex(_T("NGLV"), _T(""), false, 0, WS_CHILD | WS_CLIPSIBLINGS| WS_CLIPCHILDREN | WS_TABSTOP | WS_BORDER, NULL, CS_DBLCLKS|CS_HREDRAW);
	}

protected:
	enum edge_style_t {
		edge_none,
		edge_sunken,
		edge_grey,
		edge_solid,
	};
	class t_item;
	class t_group;

	typedef pfc::refcounted_object_ptr_t<t_group> t_group_ptr;
	typedef pfc::refcounted_object_ptr_t<t_item> t_item_ptr;

	class t_group : public pfc::refcounted_object_root
	{
	public:
		pfc::string8 m_text;

		t_group(const char * p_text) : m_text(p_text) {};
		t_group() {};
	private:
	};

	class t_item : public pfc::refcounted_object_root
	{
	public:
		//pfc::list_t<t_string_list_fast> m_subitems_v2;
		t_uint8 m_line_count;
		t_string_list_fast m_subitems;
		pfc::array_t<t_group_ptr> m_groups;

		//t_size m_position;
		t_size m_display_index;
		t_size m_display_position;
		bool m_selected;

		void update_line_count()
		{
			m_line_count = 1;
			for (t_size i = 0, count = m_subitems.get_count(); i<count; i++)
			{
				t_uint8 lc = 1;
				const char * ptr = m_subitems[i];
				while (*ptr) 
				{
					if (*ptr == '\n')
					{
						if (++lc == 255)
							break;
					}
					ptr++;
				}
				m_line_count = max (m_line_count, lc);
			}
		}

		/*t_item(const t_string_list_const_fast & p_text)
			: m_position(0), m_selected(false) 
			{
				m_subitems.add_items(p_text);
			};
		t_item(const t_string_list_const_fast & p_text, t_size group_count)
			: m_position(0), m_selected(false) 
			{
				m_subitems.add_items(p_text);
				m_groups.set_count(group_count);
			};*/
		t_item() : /*m_position(0), */m_selected(false) , m_display_index(0), m_display_position(0), m_line_count(1)
		{
		};
	private:
	};

public:
	t_list_view() 
		: m_theme(NULL), m_selecting(false), m_selecting_start(pfc_infinite), m_scroll_position(0), m_group_count(0), m_item_height(1),
		m_selecting_move(false), m_selecting_moved(false), m_dragging_rmb(false), m_shift_start(pfc_infinite), m_wnd_header(NULL), m_timer_scroll_up(false),
		m_timer_scroll_down(false), m_lbutton_down_ctrl(false),m_insert_mark_index(pfc_infinite) , m_shown(false), m_focus_index(pfc_infinite),
		m_autosize(false), m_wnd_inline_edit(NULL), m_proc_inline_edit(NULL), m_inline_edit_save(false), m_inline_edit_saving(false),
		m_inline_edit_column(pfc_infinite), m_timer_inline_edit(false), m_selecting_start_column(pfc_infinite), m_inline_edit_prevent(false),
		m_initialised(false), m_always_show_focus(false), m_prevent_wm_char_processing(false), m_timer_search(false), m_show_header(true),
		m_show_tooltips(true), m_limit_tooltips_to_clipped_items(true), m_wnd_tooltip(NULL), m_rc_tooltip(win32::rect_null),
		m_tooltip_last_index(-1), m_tooltip_last_column(-1), m_ignore_column_size_change_notification(false), m_vertical_item_padding(4),
		m_lf_items_valid(false), m_lf_header_valid(false), m_sorting_enabled(false), m_sort_column_index(pfc_infinite), m_sort_direction(false),
		m_show_sort_indicators(true), m_edge_style(edge_grey), m_sizing(false), m_single_selection(false),m_alternate_selection(false),
		m_show_group_info_area(false), m_group_info_area_width(0), m_group_info_area_height(0), m_have_indent_column(false),
		m_search_editbox(NULL), m_proc_search_edit(NULL), m_search_box_hot(false), m_lf_group_header_valid(NULL), m_group_height(1),
		m_allow_header_rearrange(false), m_highlight_item_index(pfc_infinite), m_highlight_selected_item_index(pfc_infinite),
		m_group_level_indentation_enabled(true), m_proc_original_inline_edit(NULL), m_inline_edit_prevent_kill(false), m_variable_height_items(false)
		//, m_search_box_theme(NULL)
	{
		m_dragging_initial_point.x=0; m_dragging_initial_point.y=0;
		m_dragging_rmb_initial_point.x=0; m_dragging_rmb_initial_point.y=0;
	};

	unsigned calculate_header_height();
	void set_columns(const pfc::list_base_const_t<t_column> & columns);
	void set_column_widths(const pfc::list_base_const_t<t_size> & widths);
	void add_item(const t_string_list_const_fast & text, const t_string_list_const_fast & p_groups, t_size size);
	void set_group_count(t_size count, bool b_update_columns = true);
	t_size get_group_count() const {return m_group_count;}
	t_size get_columns_width();
	t_size get_columns_display_width();
	t_size get_column_display_width(t_size index);
	ui_helpers::alignment get_column_alignment(t_size index);
	t_size get_column_count();
	void _set_scroll_position(t_size val)
	{
		m_scroll_position = val;
	}
	t_size _get_scroll_position() const
	{
		return m_scroll_position;
	}

	void set_show_header (bool b_val);
	void set_show_tooltips (bool b_val);
	void set_limit_tooltips_to_clipped_items (bool b_val);
	void set_autosize (bool b_val);
	void set_always_show_focus (bool b_val);
	void set_variable_height_items (bool b_variable_height_items)
	{
		m_variable_height_items = b_variable_height_items;
	}
	void set_single_selection (bool b_single_selection)
	{
		m_single_selection = b_single_selection;
	}
	void set_alternate_selection_model (bool b_alternate_selection)
	{
		m_alternate_selection = b_alternate_selection;
	}
	void set_allow_header_rearrange (bool b_allow_header_rearrange)
	{
		m_allow_header_rearrange = b_allow_header_rearrange;
	}
	void set_vertical_item_padding(int val)
	{
		m_vertical_item_padding = val;
		if (m_initialised)
		{
			m_item_height = get_default_item_height();
			m_group_height = get_default_group_height();
			refresh_item_positions();
			//invalidate_all(false);
			//update_scroll_info();
			//UpdateWindow(get_wnd());
		}
	}
	void set_font(const LPLOGFONT lplf)
	{
		m_lf_items = *lplf;
		m_lf_items_valid = true;
		if (m_initialised)
		{
			exit_inline_edit();
			destroy_tooltip();
			m_font = CreateFontIndirect(lplf);
			m_item_height = get_default_item_height();
			//invalidate_all(false);
			//update_scroll_info();
			//UpdateWindow(get_wnd());
			if (m_group_count)
				update_header();
			refresh_item_positions();
		}
	}
	void set_group_font(const LPLOGFONT lplf)
	{
		m_lf_group_header = *lplf;
		m_lf_group_header_valid = true;
		if (m_initialised)
		{
			exit_inline_edit();
			destroy_tooltip();
			m_group_font = CreateFontIndirect(lplf);
			m_group_height = get_default_group_height();
			refresh_item_positions();
		}
	}
	void set_header_font(const LPLOGFONT lplf)
	{
		m_lf_header = *lplf;
		m_lf_header_valid = true;
		if (m_initialised && m_wnd_header)
		{
			SendMessage(m_wnd_header, WM_SETFONT, NULL, MAKELPARAM(FALSE,0));
			m_font_header = CreateFontIndirect(lplf);
			//RedrawWindow(m_wnd_header, NULL, NULL, RDW_INVALIDATE|RDW_ERASE);
			SendMessage(m_wnd_header, WM_SETFONT, (WPARAM)m_font_header.get(), MAKELPARAM(TRUE,0));
			on_size();
		}
	}
	void set_sorting_enabled(bool b_val)
	{
		m_sorting_enabled = b_val;
		if (m_initialised && m_wnd_header)
		{
			SetWindowLong(m_wnd_header, GWL_STYLE, 
				(GetWindowLongPtr(m_wnd_header, GWL_STYLE) & ~HDS_BUTTONS) | (b_val ? HDS_BUTTONS : 0)
				);
		}
	}
	void set_show_sort_indicators(bool b_val)
	{
		m_show_sort_indicators = b_val;
		if (m_initialised && m_wnd_header)
		{
			set_sort_column(m_sort_column_index, m_sort_direction);
		}
	}
	void set_edge_style(t_size b_val)
	{
		m_edge_style = (edge_style_t)b_val;
		if (get_wnd())
		{
			SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, 
				(GetWindowLongPtr(get_wnd(), GWL_EXSTYLE) & ~(WS_EX_STATICEDGE|WS_EX_CLIENTEDGE))
				| (m_edge_style == edge_sunken ? WS_EX_CLIENTEDGE : (m_edge_style == edge_grey ? WS_EX_STATICEDGE : NULL)));
			SetWindowLongPtr(get_wnd(), GWL_STYLE, 
				(GetWindowLongPtr(get_wnd(), GWL_STYLE) & ~(WS_BORDER))
				| (m_edge_style == edge_solid ? WS_BORDER : NULL));
			SetWindowPos(get_wnd(),0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
		}
	}

	void on_size(bool b_update = true, bool b_update_scroll = true);
	void on_size(int cx, int cy, bool b_update = true, bool b_update_scroll = true);

	void reposition_header()
	{
		RECT rc;
		GetClientRect(get_wnd(), &rc);
		int cx = RECT_CX(rc);
		RECT rc_header;
		get_header_rect(&rc_header);
		SetWindowPos(m_wnd_header, NULL, -m_horizontal_scroll_position, 0, cx+m_horizontal_scroll_position, RECT_CY(rc_header), SWP_NOZORDER);
	}

	void get_column_sizes (pfc::list_t<t_column> & p_out);
	void update_column_sizes();

	//void insert_item(t_size index, const t_string_list_const_fast & text, const t_string_list_const_fast & p_groups, t_size size);
	//void insert_items(t_size index_start, const pfc::list_base_const_t<t_item_insert> & items, bool b_update_display = true);
	void insert_items(t_size index_start, t_size count, const t_item_insert * items, bool b_update_display = true);
	void replace_items(t_size index_start, const pfc::list_base_const_t<t_item_insert> & items, bool b_update_display = true);
	void remove_item(t_size index);
	void remove_items(const bit_array & p_mask, bool b_update_display = true);

	enum t_hit_test_value {hit_test_nowhere, hit_test_on, hit_test_above, hit_test_below, hit_test_obscured_above,
		hit_test_obscured_below, hit_test_below_items, hit_test_on_group, hit_test_right_of_item, hit_test_left_of_item,
		hit_test_right_of_group, hit_test_left_of_group};

	struct t_hit_test_result
	{
		t_size index;
		t_size group_level;
		t_size column;
		//t_size index_visible;
		//t_size index_partially_obscured;
		t_hit_test_value result;
		t_hit_test_result() : result(hit_test_nowhere), index(NULL), group_level(NULL), column(NULL)
		{};
	};
	void hit_test_ex(POINT pt_client, t_hit_test_result & result, bool drag_drop = false);
	void update_scroll_info(bool b_update = true, bool b_vertical = true, bool b_horizontal = true);
	void _update_scroll_info_vertical();
	void _update_scroll_info_horizontal();
	bool is_visible(t_size index);
	void ensure_visible(t_size index);
	void scroll(bool b_sb, int val, bool b_horizontal = false);

	void get_item_group(t_size index, t_size level, t_size & index_start, t_size & count);
	void set_insert_mark(t_size index);
	void remove_insert_mark();

	void set_highlight_item(t_size index);
	void remove_highlight_item();
	void set_highlight_selected_item(t_size index);
	void remove_highlight_selected_item();

	/** Rect relative to main window client area */
	void get_header_rect(LPRECT rc);
	
	/** Current height*/
	unsigned get_header_height();
	void get_items_rect(LPRECT rc);
	void get_items_size(LPRECT rc);

	t_size get_items_top() {RECT rc; get_items_rect(&rc); return rc.top;}

	void get_search_box_rect(LPRECT rc);
	unsigned get_search_box_height();

	void invalidate_all(bool b_update = true, bool b_children=false);
	void invalidate_items(t_size index, t_size count, bool b_update_display = true);
	void invalidate_items(const bit_array & mask, bool b_update_display = true)
	{
		t_size i,start,count=get_item_count();
		for (i=0; i<count; i++)
		{
			start=i;
			while (i<count && mask[i])
			{
				i++;
			}
			if (i>start)
			{
				invalidate_items(start, i-start, b_update_display);
			}
		}
	}
	void invalidate_item_group_info_area(t_size index, bool b_update_display = true)
	{
		t_size count=0;
		get_item_group(index, m_group_count?m_group_count-1:0, index, count);
		{
			RECT rc_client;
			get_items_rect(&rc_client);
			t_size groups = get_item_display_group_count(index);
			t_size item_y = get_item_position(index);
			t_size items_cy = count*m_item_height, group_area_cy = get_group_info_area_height();
			if (get_show_group_info_area() && items_cy < group_area_cy)
				items_cy = group_area_cy;

			RECT rc_invalidate = {0, item_y - m_scroll_position+rc_client.top - groups*m_item_height, RECT_CX(rc_client), item_y + group_area_cy - m_scroll_position + rc_client.top};
			if (IntersectRect(&rc_invalidate, &rc_client, &rc_invalidate))
			{
				RedrawWindow(get_wnd(), &rc_invalidate, NULL, RDW_INVALIDATE|(b_update_display?RDW_UPDATENOW:0));
			}
		}
	}
	void update_items(t_size index, t_size count, bool b_update_display = true);
	void update_all_items(bool b_update_display = true);

	t_size get_previous_item(t_size y, bool b_include_headers = false);
	t_size get_next_item(t_size y, bool b_include_headers = false);
	t_size get_last_viewable_item();
	t_size get_last_item();
	int get_default_item_height();
	int get_default_group_height();
	t_size get_item_height() const {return m_item_height;}
	t_size get_item_height(t_size index) const 
	{
		t_size ret = 1;
		if (m_variable_height_items && index < m_items.get_count()) 
			ret = m_items[index]->m_line_count*get_item_height(); 
		else ret = get_item_height(); 
		return ret;
	}

	/*void get_subitem_text_rect(t_size index, t_size columns, const RECT * prc)
	{
	}*/

	void clear_all_items() {m_items.remove_all();}

	t_size get_item_position(t_size index, bool b_include_headers = false)
	{
		t_size ret = 0;
		if (index < m_items.get_count())
			ret = m_items[index]->m_display_position;
		/*t_size ret = m_items[index]->m_display_index * m_item_height - (b_include_headers ? get_item_display_group_count(index)*m_item_height : 0);
		if (get_show_group_info_area() && m_group_count)
		{
			t_size group_cy = get_group_info_area_size();
			t_size i = 0;
			while (i < index)
			{
				t_size gstart=i, gcount=0;
				get_item_group(i, m_group_count-1, gstart, gcount);
				t_size gheight = gcount*m_item_height;
				if (index >= i+gcount && gheight < group_cy)
					ret += group_cy - gheight;
				i+= gcount;

			}
		}*/
		return ret;
	}
	t_size get_item_position_bottom(t_size index, bool b_include_headers = false)
	{
		return get_item_position(index, b_include_headers) + get_item_height(index);
	}

	t_size get_group_minimum_inner_height() {return get_show_group_info_area() ? get_group_info_area_total_height() : 0;}

	t_size get_item_group_bottom(t_size index, bool b_include_headers = false)
	{
		t_size gstart=index, gcount=0;
		get_item_group(index, m_group_count?m_group_count-1:0, gstart, gcount);
		t_size ret = 0;
		if (gcount)
			index = gstart+gcount-1;
		ret += get_item_position(index, b_include_headers);
		ret += m_item_height-1;
		if (get_show_group_info_area() && m_group_count)
		{
			t_size gheight = gcount*m_item_height;
			t_size group_cy = get_group_info_area_total_height();
			if (gheight < group_cy)
				ret += group_cy - gheight;
		}
		return ret;
	}

	void refresh_item_positions(bool b_update_display = true)
	{
		__calculate_item_positions();
		update_scroll_info();
		if (b_update_display)
			RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
	}

	enum notification_source_t
	{
		notification_source_unknown,
		notification_source_rmb,
	};

	bool copy_selected_items_as_text(t_size default_single_item_column = pfc_infinite);

	//CLIENT FUNCTIONS
	void get_selection_state(bit_array_var & out);
	void set_selection_state(const bit_array & p_affected,const bit_array & p_status, bool b_notify = true, bool b_update_display = true, notification_source_t p_notification_source = notification_source_unknown);
	t_size get_focus_item();
	void set_focus_item(t_size index, bool b_notify = true, bool b_update_display = true);
	bool get_item_selected(t_size index);
	bool is_range_selected (t_size index, t_size count)
	{
		t_size i;
		for (i=0; i<count; i++)
			if (!get_item_selected(i+index)) return false;
		return count != 0;
	}
	t_size get_selection_count(t_size max = pfc_infinite);
	t_size get_selected_item_single()
	{
		t_size numSelected = get_selection_count(2), index = 0;
		if (numSelected == 1)
		{
			bit_array_bittable mask(get_item_count());
			get_selection_state(mask);
			t_size count=get_item_count();
			while (index < count)
			{
				if (mask[index]) break;
				index++;
			}
		}
		else index = pfc_infinite;
		return index;
	}
	void sort_by_column(t_size index, bool b_descending, bool b_selection_only = false)
	{
		notify_sort_column(index, b_descending, b_selection_only);
		if (!b_selection_only)
			set_sort_column(index, b_descending);
	}
	void update_item_data (t_size index) 
	{
		notify_update_item_data(index);
		//if (m_variable_height_items) 
		{
			//m_items[index]->update_line_count();
			//__calculate_item_positions(index+1);
			//scrollbars?
		}
	};

	//SELECTION HELPERS
	void set_item_selected(t_size index, bool b_state);
	void set_item_selected_single(t_size index, bool b_notify = true, notification_source_t p_notification_source = notification_source_unknown);

	//CLIENT NOTIFICATION
	virtual void notify_on_selection_change(const bit_array & p_affected,const bit_array & p_status, notification_source_t p_notification_source) {};
	virtual void notify_on_focus_item_change(t_size new_index) {};

	virtual void notify_on_initialisation(){}; //set settings here
	virtual void notify_on_create(){}; //populate list here
	virtual void notify_on_destroy(){};

	virtual bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp, bool & b_processed)	{return false;};
	virtual bool notify_on_contextmenu_header(const POINT & pt, const HDHITTESTINFO & ht){return false;};
	virtual bool notify_on_contextmenu(const POINT & pt){return false;};
	virtual bool notify_on_timer(UINT_PTR timerid){return false;};
	virtual void notify_on_time_change(){};
	virtual void notify_on_menu_select(WPARAM wp, LPARAM lp){};

	virtual bool notify_on_middleclick(bool on_item, t_size index) {return false;};
	virtual bool notify_on_doubleleftclick_nowhere() {return false;};
	virtual void notify_sort_column(t_size index, bool b_descending, bool b_selection_only) {};

	virtual bool notify_on_keyboard_keydown_remove()	{return false;};
	virtual bool notify_on_keyboard_keydown_undo()		{return false;};
	virtual bool notify_on_keyboard_keydown_redo()		{return false;};

	virtual bool notify_on_keyboard_keydown_cut()		{return false;};
	virtual bool notify_on_keyboard_keydown_copy()		{copy_selected_items_as_text(); return true;};
	virtual bool notify_on_keyboard_keydown_paste()		{return false;};

	virtual bool notify_on_keyboard_keydown_search()	{return false;};

	virtual void notify_on_set_focus(HWND wnd_lost) {};
	virtual void notify_on_kill_focus(HWND wnd_receiving) {};

	virtual void notify_on_column_size_change(t_size index, t_size new_width) {};
	virtual void notify_on_group_info_area_size_change(t_size new_width) {};

	virtual void notify_on_header_rearrange(t_size index_from, t_size index_to) {};

	t_string_list_fast & get_item_subitems(t_size index) {return m_items[index]->m_subitems;} //hmmm
	//const t_item * get_item(t_size index) {return m_items[index].get_ptr();} 
	virtual void notify_update_item_data (t_size index) {};
	virtual t_size get_highlight_item() {return pfc_infinite;}
	virtual void execute_default_action (t_size index, t_size column, bool b_keyboard, bool b_ctrl) {};
	virtual void move_selection (int delta) {};
	virtual bool do_drag_drop(WPARAM wp) {return false;};

	bool disable_redrawing();
	void enable_redrawing();
	void update_window() {UpdateWindow(get_wnd());}

	const char * get_item_text(t_size index, t_size column);
	t_size get_item_count() {return m_items.get_count();}
	void activate_inline_editing(t_size column_start = 0);
	void activate_inline_editing(const pfc::list_base_const_t<t_size> & indices, t_size column);
	void activate_inline_editing(t_size index, t_size column);
protected:
	//STORAGE
	virtual t_size storage_get_focus_item();
	virtual void storage_set_focus_item(t_size index);
	virtual void storage_get_selection_state(bit_array_var & out);
	virtual bool storage_set_selection_state(const bit_array & p_affected,const bit_array & p_status, bit_array_var * p_changed = NULL); //return: hint if sel didnt change
	virtual bool storage_get_item_selected(t_size index);
	virtual t_size storage_get_selection_count(t_size max);

	/*virtual void storage_insert_items(t_size index_start, const pfc::list_base_const_t<t_item_insert> & items);
	virtual void storage_replace_items(t_size index_start, const pfc::list_base_const_t<t_item_insert> & items);
	virtual void storage_remove_items(const bit_array & p_mask);
	virtual void storaget_set_item_subitems(t_size index, t_string_list_cref_fast p_subitems);
	virtual t_string_list_cref_fast storaget_get_item_subitems(t_size index);*/

	virtual t_item * storage_create_item() {return new t_item;}
	virtual t_group * storage_create_group() {return new t_group;}

	t_item * get_item(t_size index) {return m_items[index].get_ptr();}
	t_size get_item_display_index(t_size index) {return m_items[index]->m_display_index;}
	t_size get_item_display_group_count(t_size index)
	{
		if (index == 0)
			return m_group_count;
		else
		{
			t_size counter = 0, i = m_group_count;
			while (i && m_items[index]->m_groups[i-1] != m_items[index-1]->m_groups[i-1])
			{
				i--;
				counter++;
			}
			return counter;
		}
	}

	void on_focus_change(t_size index_prev, t_size index_new, bool b_update_display = true);

	void set_group_level_indentation_enabled(bool b_val)
	{
		m_group_level_indentation_enabled = b_val;

		//FIXME set after creation?
	}

	t_size get_item_indentation();
	t_size get_default_indentation_step();
	void set_group_info_area_size(t_size width, t_size height) 
	{
		m_group_info_area_width = width;
		m_group_info_area_height = height;
		if (m_initialised)
		{
			update_column_sizes();
			update_header();
			refresh_item_positions();
		}
	}
	t_size get_group_info_area_width() {return get_show_group_info_area()?m_group_info_area_width:0;}
	t_size get_group_info_area_height() {return get_show_group_info_area()?m_group_info_area_height:0;}
	t_size get_group_info_area_total_width() {return get_show_group_info_area()?m_group_info_area_width+get_default_indentation_step():0;}
	t_size get_group_info_area_total_height() {return get_show_group_info_area()?m_group_info_area_height+get_default_indentation_step():0;}
	void set_show_group_info_area(bool val) 
	{
		bool b_old = get_show_group_info_area();
		m_show_group_info_area = val;
		bool b_new = get_show_group_info_area();
		if (m_initialised && (b_old != b_new))
		{
			update_column_sizes();
			update_header();
			refresh_item_positions();
		}
	}
	bool is_header_column_real(t_size index)
	{
		if (m_have_indent_column)
			return index != 0;
		else return true;
	}
	t_size header_column_to_real_column(t_size index)
	{
		if (m_have_indent_column && index != pfc_infinite)
			return index - 1;
		else return index;
	}
	bool get_show_group_info_area() {return m_group_count?m_show_group_info_area:false;}
	t_size get_total_indentation() {return get_item_indentation() + get_group_info_area_total_width();}

	struct colour_data_t
	{
		bool m_themed;
		bool m_use_custom_active_item_frame;
		COLORREF
			m_text,
			m_selection_text,
			m_inactive_selection_text,
			m_background,
			m_selection_background,
			m_inactive_selection_background,
			m_active_item_frame, 
			m_group_background,
			m_group_text;

	};
	virtual void render_get_colour_data(colour_data_t & p_out);

	void render_group_line_default(const colour_data_t & p_data, HDC dc, const RECT * rc);
	void render_group_background_default(const colour_data_t & p_data, HDC dc, const RECT * rc);
	COLORREF get_group_text_colour_default();
	bool get_group_text_colour_default(COLORREF & cr);

	void render_group_default(const colour_data_t & p_data, HDC dc, const char * text, t_size indentation, t_size level, const RECT & rc);
	void render_item_default(const colour_data_t & p_data, HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc);
	void render_background_default(const colour_data_t & p_data, HDC dc, const RECT * rc);

	virtual void render_group_info(HDC dc, t_size index, t_size group_count, const RECT & rc) {};
	virtual void render_group(HDC dc, t_size index, t_size group, const char * text, t_size indentation, t_size level, const RECT & rc);
	virtual void render_item(HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc);
	virtual void render_background(HDC dc, const RECT * rc);

	HTHEME get_theme() {return m_theme;}

	void set_sort_column (t_size index, bool b_direction);
	void clear_sort_column () {set_sort_column(pfc_infinite, false);}

	void show_search_box(const char * label, bool b_focus = true);
	void close_search_box(bool b_notify = true);
	bool is_search_box_open();
	void focus_search_box();

	void __search_box_update_hot_status(const POINT & pt);

	virtual void notify_on_search_box_contents_change(const char * p_str) {};
	virtual void notify_on_search_box_close() {};

public:
	void create_timer_scroll_up();
	void create_timer_scroll_down();
	void destroy_timer_scroll_up();
	void destroy_timer_scroll_down();

	enum inline_edit_flags_t
	{
		inline_edit_uppercase = 1<<0,
		inline_edit_autocomplete = 1<<1,
	};
private:
	void create_timer_search();
	void destroy_timer_search();

	void process_keydown(int offset, bool alt_down, bool repeat);
	bool on_wm_notify_header(LPNMHDR lpnm, LRESULT & ret);
	bool on_wm_keydown(WPARAM wp, LPARAM lp, LRESULT & ret, bool & b_processed);

	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	void render_items(HDC dc, const RECT & rc_update, t_size cx);
	void __insert_items_v2(t_size index_start, const pfc::list_base_const_t<t_item_insert> & items);
	void __insert_items_v3(t_size index_start, t_size count, const t_item_insert * items);
	void __replace_items_v2(t_size index_start, const pfc::list_base_const_t<t_item_insert> & items);
	void __remove_item(t_size index);
	void __calculate_item_positions(t_size index_start = 0);

	static LRESULT WINAPI g_on_inline_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	LRESULT on_inline_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	void create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column);
	void save_inline_edit();
	void exit_inline_edit();

	virtual bool notify_before_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, bool b_source_mouse) {return false;};
	virtual bool notify_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, pfc::string_base & p_test, t_size & p_flags, mmh::comptr_t<IUnknown> & pAutocompleteEntries) {return true;};
	virtual void notify_save_inline_edit(const char * value) {};
	virtual void notify_exit_inline_edit() {};

	void on_search_string_change(WCHAR c);

	static LRESULT WINAPI g_on_search_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	LRESULT on_search_edit_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	void reset_columns();

	void create_header();
	void destroy_header();
	void build_header();
	void update_header(bool b_update=true);

	void create_tooltip(/*t_size index, t_size column, */const char * str);
	void destroy_tooltip();
	bool is_item_clipped(t_size index, t_size column);
	t_size get_text_width(const char * text, t_size length);

	gdi_object_t<HFONT>::ptr_t m_font, m_font_header, m_group_font;

	HTHEME m_theme;
	HWND m_wnd_header;
	HWND m_wnd_inline_edit;
	WNDPROC m_proc_inline_edit, m_proc_original_inline_edit;
	bool m_inline_edit_save;
	bool m_inline_edit_saving;
	bool m_timer_inline_edit;
	bool m_inline_edit_prevent;
	bool m_inline_edit_prevent_kill;
	t_size m_inline_edit_column;
	pfc::list_t <t_size> m_inline_edit_indices;
	//mmh::comptr_t<IUnknown> m_inline_edit_autocomplete_entries;
	mmh::comptr_t<IAutoComplete> m_inline_edit_autocomplete;

	LOGFONT m_lf_items;
	LOGFONT m_lf_header;
	LOGFONT m_lf_group_header;
	bool m_lf_items_valid;
	bool m_lf_header_valid;
	bool m_lf_group_header_valid;

	bool m_selecting;
	bool m_selecting_move;
	bool m_selecting_moved;
	bool m_dragging_rmb;
	t_size m_selecting_start;
	t_size m_selecting_start_column;
	t_hit_test_result m_lbutton_down_hittest;
	int m_scroll_position;
	int m_horizontal_scroll_position;
	t_size m_group_count;
	t_size m_item_height;
	t_size m_group_height;
	t_size m_shift_start;
	bool m_timer_scroll_up;
	bool m_timer_scroll_down;
	bool m_lbutton_down_ctrl;
	t_size m_insert_mark_index;
	t_size m_highlight_item_index;
	t_size m_highlight_selected_item_index;
	POINT m_dragging_initial_point, m_dragging_rmb_initial_point;
	bool m_shown;
	t_size m_focus_index;
	bool m_autosize;
	bool m_initialised;
	bool m_always_show_focus;
	bool m_show_header;
	bool m_ignore_column_size_change_notification;
	int m_vertical_item_padding;

	bool m_variable_height_items;

	bool m_prevent_wm_char_processing;
	bool m_timer_search;
	pfc::string8 m_search_string;

	bool m_show_tooltips;
	bool m_limit_tooltips_to_clipped_items;
	HWND m_wnd_tooltip;
	RECT m_rc_tooltip;
	t_size m_tooltip_last_index;
	t_size m_tooltip_last_column;

	bool m_sorting_enabled;
	bool m_show_sort_indicators;
	t_size m_sort_column_index;
	bool m_sort_direction;
	edge_style_t m_edge_style;
	bool m_sizing;

	bool m_single_selection, m_alternate_selection;
	bool m_allow_header_rearrange;

	t_size m_group_info_area_width, m_group_info_area_height;
	bool m_show_group_info_area;
	bool m_have_indent_column;

	HWND m_search_editbox;
	WNDPROC m_proc_search_edit;
	pfc::string8 m_search_label;
	bool m_search_box_hot;
	//HTHEME m_search_box_theme;
	//gdi_object_t<HBRUSH>::ptr_t m_search_box_hot_brush, m_search_box_nofocus_brush;

	bool m_group_level_indentation_enabled;

	pfc::list_t<t_item_ptr, pfc::alloc_fast> m_items;
	pfc::list_t<t_column> m_columns;

};

#endif //_LIST_PVT_H_