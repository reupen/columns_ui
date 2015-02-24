#include "stdafx.h"


void playlist_view::create_inline_edit_v2(t_uint32 index, t_uint32 count, unsigned column)
{
	pfc::list_t<t_uint32> temp;
	t_size i;
	for (i = 0; i < count; i++)
		temp.add_item(index + i);
	create_inline_edit_v2(temp, column);
}


void playlist_view::create_inline_edit_v2(t_uint32 index, unsigned column)
{
	pfc::list_t<t_uint32> temp;
	temp.add_item(index);
	create_inline_edit_v2(temp, column);
}


void playlist_view::exit_inline_edit()
{
	if (m_wnd_edit)
	{
		DestroyWindow(m_wnd_edit);
		m_wnd_edit = 0;
	}

	if (m_edit_timer)
	{
		KillTimer(wnd_playlist, EDIT_TIMER_ID);
		m_edit_timer = false;
	}
	m_edit_field.set_string(pfc::empty_string_t<char>());
	m_edit_item.release();
	m_edit_items.remove_all();
	m_edit_indices.remove_all();
}

void playlist_view::create_inline_edit_v2(const pfc::list_base_const_t<t_uint32> & indices, unsigned column)
{
	t_size indices_count = indices.get_count();
	if (!indices_count) return;
	t_size indices_spread = indices[indices_count - 1] - indices[0] + 1;
	if (!(column < g_get_cache().active_column_get_active_count()))
	{
		console::print("internal error - edit column index out of range");
		return;
	}
	static_api_ptr_t<playlist_manager> playlist_api;
	t_size active_count = playlist_api->activeplaylist_get_item_count();
	//if (!(index < active_count))
	//{
	//	console::print("internal error - edit item index out of range");
	//	return;
	//}

	if (m_edit_timer)
	{
		KillTimer(wnd_playlist, EDIT_TIMER_ID);
		m_edit_timer = false;
	}

	t_size median = indices[0] + indices_spread / 2;//indices[(indices_count/2)];

	bool start_visible = is_visible(indices[0]);
	bool end_visible = is_visible(indices[indices_count - 1]);

	if (!start_visible || !end_visible)
	{
		SCROLLINFO si;
		memset(&si, 0, sizeof(SCROLLINFO));
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS | SIF_TRACKPOS | SIF_PAGE | SIF_RANGE;
		GetScrollInfo(wnd_playlist, SB_VERT, &si);
		t_size target;
		if (indices_count > si.nPage)
		{
			target = median;
			scroll(scroll_vertically, scroll_position_delta, target - scroll_item_offset - ((si.nPage > 1 ? si.nPage - 1 : 0) / 2));
		}
		else
		{
			target = indices[0] > scroll_item_offset ? indices[indices_count - 1] : indices[0];
			//else target = !start_visible ? indices[0] : indices[indices_count-1];
			scroll(scroll_vertically, scroll_position_delta, target - scroll_item_offset - (target > scroll_item_offset ? (si.nPage > 1 ? si.nPage - 1 : 0) : 0));
		}
	}

	int item_height = get_item_height();

	int x = 0;
	{
		pfc::array_t<int, pfc::alloc_fast_aggressive> widths;
		get_column_widths(widths);
		unsigned n, count = widths.get_size();
		for (n = 0; n < count && n < column; n++)
		{
			x += widths[n];
		}
	}

	RECT rc_playlist, rc_items;
	GetClientRect(wnd_playlist, &rc_playlist);
	get_playlist_rect(&rc_items);

	int font_height = uGetFontHeight(g_font);
	int header_height = get_header_height();

	int y = (indices[0] - scroll_item_offset)*item_height + header_height;
	if (y < header_height) y = header_height;
	int cx = get_column_width(column);
	int cy = min(item_height*indices_spread, rc_items.bottom - rc_items.top);

	if (x - horizontal_offset + cx > rc_playlist.right)
		scroll(scroll_horizontally, scroll_position_delta, x - horizontal_offset + (cx > rc_playlist.right ? 0 : cx - rc_playlist.right));
	else if (x - horizontal_offset < 0)
		scroll(scroll_horizontally, scroll_position_delta, x - horizontal_offset);

	x -= horizontal_offset;

	if (m_wnd_edit)
		save_inline_edit_v2();

	m_edit_field.set_string(pfc::empty_string_t<char>());
	m_edit_items.remove_all();
	m_edit_items.set_count(indices_count);
	//pfc::list_t<bool> mask;

	pfc::array_t<file_info_impl> infos;
	pfc::ptr_list_t<const char> ptrs;
	infos.set_count(indices_count);
	//mask.set_count(indices_count);
	t_size i;

	pfc::string8 meta;
	meta = g_get_columns()[g_get_cache().active_column_active_to_actual(column)]->edit_field;
	m_edit_field = meta;

	bool matching = true;

	for (i = 0; i < indices_count; i++)
	{
		if (playlist_api->activeplaylist_get_item_handle(m_edit_items[i], indices[i]))
		{
			//mask[i] = true;
			m_edit_items[i]->get_info(infos[i]);
			ptrs.add_item(infos[i].meta_get(meta, 0));
		}
		else
		{
			m_edit_save = false;
			exit_inline_edit();
			return;
			//mask[i]=false;
			ptrs.add_item((const char *)NULL);
		}
		//exit_inline_edit();
		if (matching && i>0 && ((ptrs[i] && ptrs[i - 1] && strcmp(ptrs[i], ptrs[i - 1])) || ((!ptrs[i] || !ptrs[i - 1]) && (ptrs[i] != ptrs[i - 1]))))
			matching = false;
	}

	pfc::string8 text = matching ? (ptrs[0] ? ptrs[0] : "") : "<multiple values>";

	if (!m_wnd_edit)
	{
		m_edit_save = true;
		m_edit_changed = false;
		m_wnd_edit = CreateWindowEx(0, WC_EDIT, pfc::stringcvt::string_os_from_utf8(text).get_ptr(), WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | ES_LEFT |
			ES_AUTOHSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER | WS_CLIPCHILDREN, x,
			y,
			cx, cy, wnd_playlist, HMENU(667),
			core_api::get_my_instance(), 0);

		SetWindowLongPtr(m_wnd_edit, GWL_USERDATA, (LPARAM)(this));
		m_inline_edit_proc = (WNDPROC)SetWindowLongPtr(m_wnd_edit, GWL_WNDPROC, (LPARAM)(g_inline_edit_hook_v2));
		SetWindowPos(m_wnd_edit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		SendMessage(m_wnd_edit, WM_SETFONT, (WPARAM)g_font, MAKELONG(TRUE, 0));
	}
	else
	{
		SendMessage(m_wnd_edit, WM_SETTEXT, 0, (LPARAM)_T(""));
		SetWindowPos(m_wnd_edit, NULL, x,
			y,
			cx, cy, SWP_NOZORDER);
		//if (ptr)
		uSendMessageText(m_wnd_edit, WM_SETTEXT, 0, text);
	}


	RECT rc;
	rc.left = x + 1;
	rc.top = y + (cy - font_height) / 2;
	rc.right = x + cx;
	rc.bottom = rc.top + font_height;
	MapWindowPoints(wnd_playlist, m_wnd_edit, (LPPOINT)&rc, 2);


	SendMessage(m_wnd_edit, EM_SETRECT, NULL, (LPARAM)&rc);

	SendMessage(m_wnd_edit, EM_SETSEL, 0, -1);
	SetFocus(m_wnd_edit);

	m_edit_indices.remove_all();
	m_edit_indices.add_items(indices);
	m_edit_column = column;

}

#if 0
void playlist_view::save_inline_edit()
{
	static_api_ptr_t<metadb_io> tagger_api;

	if (m_edit_save && !m_edit_saving && !tagger_api->is_busy() && m_edit_item.is_valid())
	{
		//pfc::vartoggle_t<bool>(m_edit_saving, true);
		m_edit_saving = true;

		pfc::string8 text;
		uGetWindowText(m_wnd_edit, text);
		file_info_impl info;
		if (m_edit_item->get_info(info))
		{
			const char * ptr = info.meta_get(m_edit_field, 0);
			if ((!ptr  && text.length()) || (ptr &&strcmp(ptr, text)))
			{
				info.meta_set(m_edit_field, text);
				tagger_api->update_info(m_edit_item, info, wnd_playlist, false);
			}
		}
		m_edit_saving = false;
	}
	m_edit_save = true;
}
#endif


void playlist_view::save_inline_edit_v2()
{
	static_api_ptr_t<metadb_io> tagger_api;

	if (m_edit_save && /*m_edit_changed &&*/ !m_edit_saving && !tagger_api->is_busy() && m_edit_items.get_count())
	{
		//pfc::vartoggle_t<bool>(m_edit_saving, true);
		m_edit_saving = true;

		pfc::string8 text;
		uGetWindowText(m_wnd_edit, text);

		if (strcmp(text, "<multiple values>"))
		{
			metadb_handle_list ptrs(m_edit_items);
			pfc::list_t<file_info_impl> infos;
			pfc::list_t<bool> mask;
			pfc::ptr_list_t<file_info> infos_ptr;
			t_size i, count = ptrs.get_count();
			mask.set_count(count);
			infos.set_count(count);
			//infos.set_count(count);
			for (i = 0; i < count; i++)
			{
				assert(ptrs[i].is_valid());
				mask[i] = !ptrs[i]->get_info(infos[i]);
				infos_ptr.add_item(&infos[i]);
				if (!mask[i])
				{
					const char * ptr = infos[i].meta_get(m_edit_field, 0);
					if (!(mask[i] = !((!ptr  && text.length()) || (ptr && strcmp(ptr, text)))))
						infos[i].meta_set(m_edit_field, text);
				}
			}
			infos_ptr.remove_mask(mask.get_ptr());
			ptrs.remove_mask(mask.get_ptr());

			{
				{
					tagger_api->update_info_multi(ptrs, infos_ptr, wnd_playlist, false);
				}
			}
		}
		m_edit_saving = false;
	}
	m_edit_save = true;
}

LRESULT WINAPI playlist_view::g_inline_edit_hook_v2(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	playlist_view * p_this;
	LRESULT rv;

	p_this = reinterpret_cast<playlist_view*>(GetWindowLongPtr(wnd, GWL_USERDATA));

	rv = p_this ? p_this->on_inline_edit_message_v2(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);;

	return rv;
}

LRESULT playlist_view::on_inline_edit_message_v2(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_KILLFOCUS:
		save_inline_edit_v2();
		//if (wp == NULL)
		//	console::print("wp==NULL");
		PostMessage(wnd_playlist, MSG_KILL_INLINE_EDIT, 0, 0);
		break;
	case WM_SETFOCUS:
		break;
	case WM_GETDLGCODE:
		return CallWindowProc(m_inline_edit_proc, wnd, msg, wp, lp) | DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		switch (wp)
		{
		case VK_TAB:
		{
			unsigned count = g_get_cache().active_column_get_active_count();
			t_size indices_count = m_edit_indices.get_count();
			assert(indices_count);
			if (count && indices_count)
			{
				static_api_ptr_t<playlist_manager> api;
				bool back = (GetKeyState(VK_SHIFT) & KF_UP) != 0;
				if (back)
				{
					unsigned column = m_edit_column;
					unsigned playlist_index = m_edit_indices[0];
					t_size playlist_count = api->activeplaylist_get_item_count();
					pfc::string8_fast_aggressive temp;
					bool found = false;
					do
					{
						while (column > 0 && !(found = !g_get_columns()[g_get_cache().active_column_active_to_actual(--column)]->edit_field.is_empty()))
						{
						}
					} while (!found && indices_count == 1 && playlist_index > 0 && (column = count) && (playlist_index-- >= 0));

					if (found)
					{
						if (indices_count > 1)
							create_inline_edit_v2(pfc::list_t<t_uint32>(m_edit_indices), column);
						else
							create_inline_edit_v2(playlist_index, column);
					}
				}
				else
				{
					unsigned column = m_edit_column + 1;
					unsigned playlist_index = m_edit_indices[0];
					t_size playlist_count = api->activeplaylist_get_item_count();
					pfc::string8_fast_aggressive temp;
					bool found = false;
					do
					{
						while (column < count && !(found = !g_get_columns()[g_get_cache().active_column_active_to_actual(column)]->edit_field.is_empty()))
						{
							column++;
						}
					} while (!found && indices_count == 1 && ++playlist_index < playlist_count && !(column = 0));

					if (found)
					{
						//console::formatter () << "column: " << column;
						if (indices_count>1)
							create_inline_edit_v2(pfc::list_t<t_uint32>(m_edit_indices), column);
						else
							create_inline_edit_v2(playlist_index, column);
					}
				}
			}
		}
		return 0;
		case VK_ESCAPE:
			m_edit_save = false;
			exit_inline_edit();
			return 0;
		case VK_RETURN:
			if ((GetKeyState(VK_CONTROL) & KF_UP) == 0)
				exit_inline_edit();
			//SetFocus(wnd_playlist);
			return 0;
		}
		break;
	}
	return CallWindowProc(m_inline_edit_proc, wnd, msg, wp, lp);
}
