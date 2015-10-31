#include "stdafx.h"

template <typename t_list, typename t_compare>
static void g_sort_qsort(t_list & p_list, t_compare p_compare, bool stabilise)
{
	t_size size = pfc::array_size_t(p_list);
	mmh::permutation_t perm(size);
	mmh::g_sort_get_permutation_qsort(p_list, perm, p_compare, stabilise);
	p_list.reorder(perm.get_ptr());
}

struct process_bydir_entry
{
	metadb_handle * m_item;
	const char * m_path;

	inline static bool g_is_separator(char c) { return c == '\\' || c == '|' || c == '/'; }

	static const char * g_advance(const char * p_path)
	{
		const char * ptr = p_path;
		while (*ptr && !g_is_separator(*ptr)) ptr++;
		while (*ptr && g_is_separator(*ptr)) ptr++;
		return ptr;
	}

	static unsigned g_get_segment_length(const char * p_path)
	{
		unsigned ret = 0;
		while (p_path[ret] && !g_is_separator(p_path[ret])) ret++;
		return ret;
	}

	unsigned get_segment_length() const { return g_get_segment_length(m_path); }

	inline static int g_compare_segment(const char * p_path1, const char * p_path2)
	{
		return metadb::path_compare_ex(p_path1, g_get_segment_length(p_path1), p_path2, g_get_segment_length(p_path2));
	}

	inline static int g_compare_segment(const process_bydir_entry & p_item1, const process_bydir_entry & p_item2)
	{
		return metadb::path_compare_ex(p_item1.m_path, p_item1.get_segment_length(), p_item2.m_path, p_item2.get_segment_length());
	}

	inline static int g_compare(const process_bydir_entry & p_item1, const process_bydir_entry & p_item2) { return metadb::path_compare(p_item1.m_path, p_item2.m_path); }

	enum { is_bydir = true };
};

struct process_byformat_entry
{
	metadb_handle * m_item;
	const char * m_path;

	inline static bool g_is_separator(char c) { return c == '|'; }

	static const char * g_advance(const char * p_path)
	{
		const char * ptr = p_path;
		while (*ptr && !g_is_separator(*ptr)) ptr++;
		while (*ptr && g_is_separator(*ptr)) ptr++;
		return ptr;
	}

	static t_size g_get_segment_length(const char * p_path)
	{
		t_size ret = 0;
		while (p_path[ret] && !g_is_separator(p_path[ret])) ret++;
		return ret;
	}

	t_size get_segment_length() const { return g_get_segment_length(m_path); }

	inline static int g_compare_segment(const char * p_path1, const char * p_path2)
	{
		return stricmp_utf8_ex(p_path1, g_get_segment_length(p_path1), p_path2, g_get_segment_length(p_path2));
	}

	inline static int g_compare_segment(const process_byformat_entry & p_item1, const process_byformat_entry & p_item2)
	{
		return stricmp_utf8_ex(p_item1.m_path, p_item1.get_segment_length(), p_item2.m_path, p_item2.get_segment_length());
	}

	inline static int g_compare(const process_byformat_entry & p_item1, const process_byformat_entry & p_item2) { return stricmp_utf8(p_item1.m_path, p_item2.m_path); }

	enum { is_bydir = false };
};

template<typename t_entry>
class process_entry_list_wrapper_t : public list_base_const_t<metadb_handle_ptr>
{
public:
	process_entry_list_wrapper_t(const t_entry* p_data, t_size p_count) : m_data(p_data), m_count(p_count) {}
	t_size get_count() const
	{
		return m_count;
	}
	void get_item_ex(metadb_handle_ptr & p_out, t_size n) const
	{
		p_out = m_data[n].m_item;
	}


private:
	const t_entry * m_data;
	t_size m_count;
};

template<typename t_entry>
static void process_level_recur_t(const t_entry * p_items, t_size const p_items_count, node_ptr p_parent, bool b_add_only)
{
	p_parent->set_bydir(t_entry::is_bydir);
	p_parent->set_data(process_entry_list_wrapper_t<t_entry>(p_items, p_items_count), !b_add_only);
	p_parent->m_label_dirty = cfg_show_numbers != 0;
	assert(p_items_count > 0);
	pfc::array_t<t_entry> items_local; items_local.set_size(p_items_count);
	t_size items_local_ptr = 0;
	t_size n;
	const char * last_path = 0;
	bool b_node_added = false;
	for (n = 0; n < p_items_count; n++)
	{
		const char * current_path = p_items[n].m_path;
		while (*current_path && t_entry::g_is_separator(*current_path)) current_path++;
		if (items_local_ptr > 0 && t_entry::g_compare_segment(last_path, current_path) != 0)
		{
			bool b_new = false;
			node_ptr p_node = p_parent->find_or_add_child(last_path, t_entry::g_get_segment_length(last_path), !b_add_only, b_new);
			if (b_new)
				b_node_added = true;
			process_level_recur_t<t_entry>(items_local.get_ptr(), items_local_ptr, p_node, b_add_only);
			items_local_ptr = 0;
			last_path = 0;
		}

		if (*current_path != 0)
		{
			items_local[items_local_ptr].m_item = p_items[n].m_item;
			items_local[items_local_ptr].m_path = t_entry::g_advance(current_path);
			items_local_ptr++;
			last_path = current_path;
		}
	}

	if (items_local_ptr > 0)
	{
		bool b_new = false;
		node_ptr p_node = p_parent->find_or_add_child(last_path, t_entry::g_get_segment_length(last_path), !b_add_only, b_new);
		if (b_new)
			b_node_added = true;
		process_level_recur_t<t_entry>(items_local.get_ptr(), items_local_ptr, p_node, b_add_only);
		items_local_ptr = 0;
		last_path = 0;
	}
	if (b_node_added && cfg_show_numbers2)
	{
		t_size i, count = p_parent->get_children().get_count();
		for (i = 0; i < count; i++)
			p_parent->get_children()[i]->m_label_dirty = true;
	}
}

struct process_byformat_branch_segment
{
	t_size m_first_choice;
	t_size m_last_choice;
	t_size m_current_choice;
};

struct process_byformat_branch_choice
{
	t_size m_start;
	t_size m_end;
};

template <typename t_string_storage>
t_size process_byformat_add_branches_t(t_string_storage & p_string_storage, const char * p_text)
{
	const char * marker = strchr(p_text, 4);
	if (marker == 0)
	{
		p_string_storage.append_fromptr(p_text, strlen(p_text) + 1);
		return 1;
	}
	else
	{
		t_size branch_count = 1;

		list_t<process_byformat_branch_segment> segments;
		list_t<process_byformat_branch_choice> choices;

		// compute segments and branch count
		t_size ptr = 0;
		while (p_text[ptr] != 0)
		{
			// begin choice
			if (p_text[ptr] == 4)
			{
				ptr++;

				if (p_text[ptr] == 4) return 0; // empty choice

				process_byformat_branch_segment segment;
				segment.m_first_choice = choices.get_count();
				segment.m_current_choice = segment.m_first_choice;

				process_byformat_branch_choice choice;
				choice.m_start = ptr;

				while (p_text[ptr] != 0 && p_text[ptr] != 4)
				{
					if (p_text[ptr] == 5)
					{
						choice.m_end = ptr;
						choices.add_item(choice);
						ptr++;
						choice.m_start = ptr;
					}
					else ptr++;
				}

				choice.m_end = ptr;

				if (p_text[ptr] != 0) ptr++;

				if (choice.m_end > choice.m_start)
					choices.add_item(choice);

				segment.m_last_choice = choices.get_count();

				if (segment.m_last_choice == segment.m_first_choice)
					return 0; // only empty choices

				segments.add_item(segment);

				branch_count *= segment.m_last_choice - segment.m_first_choice;
			}
			else
			{
				process_byformat_branch_segment segment;
				segment.m_first_choice = choices.get_count();
				segment.m_current_choice = segment.m_first_choice;

				process_byformat_branch_choice choice;
				choice.m_start = ptr;

				while (p_text[ptr] != 0 && p_text[ptr] != 4) ptr++;

				choice.m_end = ptr;
				choices.add_item(choice);

				segment.m_last_choice = choices.get_count();
				segments.add_item(segment);
			}
		}

		// assemble branches
		for (t_size branch_index = 0; branch_index < branch_count; branch_index++)
		{
			t_size segment_count = segments.get_count();
			for (t_size segment_index = 0; segment_index < segment_count; segment_index++)
			{
				const process_byformat_branch_choice & choice = choices[segments[segment_index].m_current_choice];
				p_string_storage.append_fromptr(&p_text[choice.m_start], choice.m_end - choice.m_start);
			}
			p_string_storage.append_single(0);
			for (t_size segment_index = 0; segment_index < segment_count; segment_index++)
			{
				process_byformat_branch_segment & segment = segments[segment_count - segment_index - 1];
				segment.m_current_choice++;
				if (segment.m_current_choice >= segment.m_last_choice)
					segment.m_current_choice = segment.m_first_choice;
				else
					break;
			}
		}
		return branch_count;
		}
	}

void setup_tree(HWND list, HTREEITEM parent, node_ptr ptr, t_size level, t_size idx, t_size max_idx, metadb_handle_list_t<pfc::alloc_fast_aggressive> & entries, HTREEITEM ti_after /*, bool b_sort,const service_ptr_t<titleformat_object> & p_sort_script*/)
{
	static string8_fastalloc sz_text;

	HTREEITEM item = TVI_ROOT;

	ptr->purge_empty_children(list);

	if (ptr->m_ti)
	{
		item = ptr->m_ti;
	}
	if ((!ptr->m_ti || ptr->m_label_dirty) && (level > 0 || cfg_show_root))
	{
		sz_text.reset();

		if (cfg_show_numbers2 && max_idx > 0)
		{
			t_size pad = 0;
			while (max_idx > 0)
			{
				max_idx /= 10;
				pad++;
			}
			char temp1[128], temp2[128];
			sprintf(temp1, "%%0%uu. ", pad);
			sprintf(temp2, temp1, idx + 1);
			sz_text += temp2;
		}

		sz_text += ptr->get_val();

		if (cfg_show_numbers)
		{
			t_size num = ptr->get_num_children();
			if (num > 0)
			{
				char blah[64];
				sprintf(blah, " (%u)", num);
				sz_text += blah;
			}
		}


		if (ptr->m_ti)
		{
			pfc::stringcvt::string_os_from_utf8_fast wstr(sz_text);
			TVITEM tvi;
			memset(&tvi, 0, sizeof(tvi));
			tvi.hItem = ptr->m_ti;
			tvi.mask = TVIF_TEXT;
			tvi.pszText = const_cast<WCHAR*>(wstr.get_ptr());
			TreeView_SetItem(list, &tvi);
		}
		else
		{
			uTVINSERTSTRUCT is;
			memset(&is, 0, sizeof(is));
			is.hParent = parent;
			is.hInsertAfter = ti_after;
			is.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
			is.item.pszText = const_cast<char*>(sz_text.get_ptr());
			is.item.lParam = (int)ptr.get_ptr();
			is.item.state = level < 1 ? TVIS_EXPANDED : 0;
			is.item.stateMask = TVIS_EXPANDED;
			item = uTreeView_InsertItem(list, &is);


			ptr->m_ti = item;
		}
		ptr->m_label_dirty = false;
	}

	const list_t<node_ptr> & children = ptr->get_children();

	unsigned n;

#if 0
	if (b_sort)
	{
		array_t<unsigned> order;
		order.set_size(children.get_count());
		{
			TRACK_CALL_TEXT("sort");
			for (n = 0; n < children.get_count(); n++)
			{
				entries.add_item(children[n]->get_entries()[0]);
			}

			order_helper::g_fill(order.get_ptr(), children.get_count());

			entries.sort_by_format_get_order(order.get_ptr(), p_sort_script, 0);

			entries.remove_all();
		}

		for (n = 0; n < children.get_count(); n++)
			setup_tree(list, item, children[order[n]], level + 1, n, children.get_count(), entries, true, p_sort_script);

	}
	else
#endif
	{
		for (n = 0; n < children.get_count(); n++)
		{
			HTREEITEM ti_aft = n ? children[n - 1]->m_ti : NULL;
			if (ti_aft == NULL) ti_aft = TVI_FIRST;
			setup_tree(list, item, children[n], level + 1, n, children.get_count(), entries, ti_aft/*,false,p_sort_script*/);
		}
	}
}



void album_list_window::refresh_tree_internal()
{
	metadb_handle_list_t<pfc::alloc_fast_aggressive> library;
	m_filter_ptr.release();

	static_api_ptr_t<library_manager> api;

	{
		string8 pattern;
		if (wnd_edit) uGetWindowText(wnd_edit, pattern);
		if (!wnd_edit || pattern.is_empty())
		{
			api->get_all_items(library);
		}
		else
		{
			try
			{
				try
				{
					service_ptr_t<album_list_window> p_this = this;
					m_filter_ptr = static_api_ptr_t<search_filter_manager_v2>()->create_ex(pattern, mmh::fb2k::completion_notify_create_copyptr(p_this, 0), NULL);
				}
				catch (exception_service_not_found const &)
				{
					m_filter_ptr = static_api_ptr_t<search_filter_manager>()->create(pattern);
				}

				in_metadb_sync lock;
				api->get_all_items(library);
				pfc::array_t<bool> mask;
				mask.set_count(library.get_count());
				m_filter_ptr->test_multi(library, mask.get_ptr());
				library.remove_mask(bit_array_not(bit_array_table(mask.get_ptr(), mask.get_count())));
			}
			catch (pfc::exception const &) {};

		}
	}

	if (is_bydir())
	{
		const t_size count = library.get_count();

		if (count > 0)
		{
			pfc::list_t<process_bydir_entry> entries;
			entries.set_size(count);
			pfc::array_t<string8> strings;
			strings.set_size(count);

			unsigned n;

			{
				for (n = 0; n < count; n++)
				{
					api->get_relative_path(library[n], strings[n]);
					entries[n].m_path = strings[n];
					entries[n].m_item = library[n].get_ptr();
		}
	}

			{
				g_sort_qsort(entries, process_bydir_entry::g_compare, false);
			}

			{
				m_root = new node(0, 0, this);
				process_level_recur_t(entries.get_ptr(), count, m_root, true);
			}
}
}
	else
	{
		const t_size count = library.get_count();

		if (count > 0)
		{
			service_ptr_t<titleformat_object> script;

			{
				static_api_ptr_t<titleformat_compiler>()->compile_safe(script, get_hierarchy());
			}

			//array_t<process_byformat_entry> entries(count);
			list_t<process_byformat_entry> entries;
			//entries.prealloc(count);

#if 1

			pfc::array_t<char, pfc::alloc_fast_aggressive> stringbuffer;
			stringbuffer.prealloc(1024 * 16);


			{
				string8_fastalloc formatbuffer;

				{
					in_metadb_sync lock;

					static const file_info_impl null_info;

					for (t_size n = 0; n < count; n++)
					{
						const playable_location & location = library[n]->get_location();
						const file_info * info;
						if (!library[n]->get_info_locked(info))
							info = &null_info;
#if 1
						library[n]->format_title(
							&titleformat_hook_impl_file_info_branch(location, info),
							formatbuffer,
							script,
							&titleformat_text_filter_impl_reserved_chars("|")
							);
#else
						script->run_filtered(
							&titleformat_hook_impl_file_info_branch(location, info),
							formatbuffer,
							&titleformat_text_filter_impl_reserved_chars("|"));
#endif
						t_size branch_count = process_byformat_add_branches_t(stringbuffer, formatbuffer);
						process_byformat_entry entry;
						entry.m_item = library[n].get_ptr();
						entries.add_items_repeat(entry, branch_count);
					}
				}

				const char * bufptr = stringbuffer.get_ptr();
				for (t_size n = 0; n < entries.get_count(); n++)
				{
					entries[n].m_path = bufptr;
					bufptr += strlen(bufptr) + 1;
				}
			}
#else

			array_t<string8> strings(count);

			{
				string8_fastalloc formatbuffer;
				albumlist_profiler(formatmode_setup);
				for (t_size n = 0; n < count; n++)
				{
					library[n]->format_title(0, formatbuffer, script, &titleformat_text_filter_impl_reserved_chars("|"));
					strings[n] = formatbuffer;
					entries[n].m_path = strings[n];
					entries[n].m_item = library[n].get_ptr();
				}

			}
#endif

			{
				g_sort_qsort(entries, process_byformat_entry::g_compare, false);
			}

			{
				m_root = new node(0, 0, this);
				process_level_recur_t(entries.get_ptr(), entries.get_count(), m_root, true);
			}
		}
	}
}

__forceinline void g_node_remove_tracks_recur(const node_ptr & ptr, const metadb_handle_list & p_tracks)
{
	if (ptr.is_valid())
	{
		t_size i, count = ptr->get_entries().get_count(), index;
		bit_array_bittable mask(count);
		bool b_found = false;

		const metadb_handle_ptr * p_entries = ptr->get_entries().get_ptr();
		const node_ptr * p_nodes = ptr->get_children().get_ptr();

		for (i = 0; i < count; i++)
		{
			//if (metadb_handle_list_helper::bsearch_by_pointer(p_tracks, ptr->get_entries()[i]) != pfc_infinite)
			if (pfc::bsearch_simple_inline_t(p_tracks.get_ptr(), p_tracks.get_count(), p_entries[i], index))
			{
				mask.set(i, true);
				b_found = true;
			}
		}

		if (b_found)
			ptr->remove_entries(mask);

		count = ptr->get_children().get_count();
		for (i = 0; i < count; i++)
		{
			g_node_remove_tracks_recur(p_nodes[i], p_tracks);
		}
	}
}

void album_list_window::refresh_tree_internal_remove_tracks(metadb_handle_list & p_tracks)
{
	g_node_remove_tracks_recur(m_root, p_tracks);
}

void album_list_window::refresh_tree_internal_add_tracks(metadb_handle_list & p_tracks)
{
	metadb_handle_list_t<pfc::alloc_fast_aggressive> current_tracks, new_tracks = p_tracks;
	m_filter_ptr.release();

	if (m_root.is_valid())
		current_tracks = m_root->get_entries();



	static_api_ptr_t<library_manager> api;

	{
		string8 pattern;
		if (wnd_edit) uGetWindowText(wnd_edit, pattern);
		if (!wnd_edit || pattern.is_empty())
		{

		}
		else
		{
			try
			{
				try
				{
					service_ptr_t<album_list_window> p_this = this;
					m_filter_ptr = static_api_ptr_t<search_filter_manager_v2>()->create_ex(pattern, mmh::fb2k::completion_notify_create_copyptr(p_this, 0), NULL);
				}
				catch (exception_service_not_found const &)
				{
					m_filter_ptr = static_api_ptr_t<search_filter_manager>()->create(pattern);
				}

				pfc::array_t<bool> mask;
				mask.set_count(new_tracks.get_count());
				m_filter_ptr->test_multi(new_tracks, mask.get_ptr());
				new_tracks.remove_mask(bit_array_not(bit_array_table(mask.get_ptr(), mask.get_count())));
			}
			catch (pfc::exception const &) {};

		}
	}

	if (is_bydir())
	{
		const t_size count = new_tracks.get_count();

		if (count > 0)
		{
			pfc::list_t<process_bydir_entry> entries;
			entries.set_size(count);
			pfc::array_t<string8> strings;
			strings.set_size(count);

			unsigned n;

			{
				for (n = 0; n < count; n++)
				{
					api->get_relative_path(new_tracks[n], strings[n]);
					entries[n].m_path = strings[n];
					entries[n].m_item = new_tracks[n].get_ptr();
				}
			}

			{
				g_sort_qsort(entries, process_bydir_entry::g_compare, false);
			}

			{
				if (!m_root.is_valid())
					m_root = new node(0, 0, this);
				process_level_recur_t(entries.get_ptr(), count, m_root, false);
			}
		}
	}
	else
	{
		const t_size count = new_tracks.get_count();

		if (count > 0)
		{
			service_ptr_t<titleformat_object> script;

			{
				static_api_ptr_t<titleformat_compiler>()->compile_safe(script, get_hierarchy());
			}

			//array_t<process_byformat_entry> entries(count);
			list_t<process_byformat_entry> entries;
			//entries.prealloc(count);

#if 1

			pfc::array_t<char, pfc::alloc_fast_aggressive> stringbuffer;
			stringbuffer.prealloc(1024 * 16);


			{
				string8_fastalloc formatbuffer;

				{
					in_metadb_sync lock;

					static const file_info_impl null_info;

					for (t_size n = 0; n < count; n++)
					{
						const playable_location & location = new_tracks[n]->get_location();
						const file_info * info;
						if (!new_tracks[n]->get_info_locked(info))
							info = &null_info;
#if 1
						new_tracks[n]->format_title(
							&titleformat_hook_impl_file_info_branch(location, info),
							formatbuffer,
							script,
							&titleformat_text_filter_impl_reserved_chars("|")
							);
#else
						script->run_filtered(
							&titleformat_hook_impl_file_info_branch(location, info),
							formatbuffer,
							&titleformat_text_filter_impl_reserved_chars("|"));
#endif
						t_size branch_count = process_byformat_add_branches_t(stringbuffer, formatbuffer);
						process_byformat_entry entry;
						entry.m_item = new_tracks[n].get_ptr();
						entries.add_items_repeat(entry, branch_count);
					}
			}

				const char * bufptr = stringbuffer.get_ptr();
				for (t_size n = 0; n < entries.get_count(); n++)
				{
					entries[n].m_path = bufptr;
					bufptr += strlen(bufptr) + 1;
				}
			}
#else

			array_t<string8> strings(count);

			{
				string8_fastalloc formatbuffer;
				albumlist_profiler(formatmode_setup);
				for (t_size n = 0; n < count; n++)
				{
					library[n]->format_title(0, formatbuffer, script, &titleformat_text_filter_impl_reserved_chars("|"));
					strings[n] = formatbuffer;
					entries[n].m_path = strings[n];
					entries[n].m_item = library[n].get_ptr();
				}

				}
#endif

				{
					g_sort_qsort(entries, process_byformat_entry::g_compare, false);
				}

				{
					if (!m_root.is_valid())
						m_root = new node(0, 0, this);
					process_level_recur_t(entries.get_ptr(), entries.get_count(), m_root, false);
				}
		}
	}
}

void album_list_window::on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & p_const_data)
{
	if (!m_populated) return;

	TRACK_CALL_TEXT("album_list_panel_refresh_tree");

	metadb_handle_list p_data = p_const_data;

	uSendMessage(wnd_tv, WM_SETREDRAW, FALSE, 0);

	try {
		refresh_tree_internal_add_tracks(p_data);
	}
	catch (pfc::exception const & e) {
		popup_message::g_show(string_formatter() << "Album list panel: An error occured while generating the tree (" << e << ").", "Error", popup_message::icon_error);
		m_root.release();
	}

	if (m_root.is_valid())
	{
		//m_root->sort_children();
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> entries;
			TRACK_CALL_TEXT("album_list_panel_setup_tree");
			setup_tree(wnd_tv, TVI_ROOT, m_root, 0, 0, 0, entries/*,b_sort,sort_script*/);
		}
	}

	uSendMessage(wnd_tv, WM_SETREDRAW, TRUE, 0);
}
void album_list_window::on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & p_data_const)
{
	if (!m_populated) return;

	metadb_handle_list p_data = p_data_const;

	mmh::permutation_t perm(p_data.get_count());
	mmh::g_sort_get_permutation_qsort_v2(p_data.get_ptr(), perm, pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, false);
	p_data.reorder(perm.get_ptr());


	uSendMessage(wnd_tv, WM_SETREDRAW, FALSE, 0);

	try {
		refresh_tree_internal_remove_tracks(p_data);
	}
	catch (pfc::exception const & e) {
		popup_message::g_show(string_formatter() << "Album list panel: An error occured while generating the tree (" << e << ").", "Error", popup_message::icon_error);
		m_root.release();
	}

	if (m_root.is_valid())
	{
		if (!m_root->get_entries().get_count())
		{
			TreeView_DeleteItem(wnd_tv, m_root->m_ti);
			m_root.release();
			p_selection.release();
		}
		else
			//m_root->sort_children();
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> entries;
			TRACK_CALL_TEXT("album_list_panel_setup_tree");
			setup_tree(wnd_tv, TVI_ROOT, m_root, 0, 0, 0, entries/*,b_sort,sort_script*/);
		}
	}

	uSendMessage(wnd_tv, WM_SETREDRAW, TRUE, 0);
}
void album_list_window::on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & p_const_data)
{
	if (!m_populated) return;

	metadb_handle_list p_data = p_const_data;

	mmh::permutation_t perm(p_data.get_count());
	mmh::g_sort_get_permutation_qsort_v2(p_data.get_ptr(), perm, pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, false);
	p_data.reorder(perm.get_ptr());


	uSendMessage(wnd_tv, WM_SETREDRAW, FALSE, 0);

	try {
		refresh_tree_internal_remove_tracks(p_data);
		refresh_tree_internal_add_tracks(p_data);
	}
	catch (pfc::exception const & e) {
		popup_message::g_show(string_formatter() << "Album list panel: An error occured while generating the tree (" << e << ").", "Error", popup_message::icon_error);
		m_root.release();
	}

	if (m_root.is_valid())
	{
		//m_root->sort_children();
		{
			metadb_handle_list_t<pfc::alloc_fast_aggressive> entries;
			TRACK_CALL_TEXT("album_list_panel_setup_tree");
			setup_tree(wnd_tv, TVI_ROOT, m_root, 0, 0, 0, entries/*,b_sort,sort_script*/);
		}
	}

	uSendMessage(wnd_tv, WM_SETREDRAW, TRUE, 0);
}

void album_list_window::refresh_tree()
{
	TRACK_CALL_TEXT("album_list_panel_refresh_tree");

	if (!wnd_tv) return;

	m_populated = true;

	static_api_ptr_t<library_manager> api;
	if (!api->is_library_enabled())
	{
		//api->show_preferences();
		//popup_message::g_show("Media library is not available. Please configure media library before using album list panel.","Information");
	}
	else
	{
		//bool b_sort = !!cfg_sorttree;

		uSendMessage(wnd_tv, WM_SETREDRAW, FALSE, 0);
		uSendMessage(wnd_tv, TVM_DELETEITEM, 0, (long)TVI_ROOT);
		p_selection.release();
		m_root.release();
#if 0

		if (is_bydir())
		{
			metadb_handle_list library;

			api->get_all_items(library);

			const unsigned count = library.get_count();

			if (count > 0)
			{
				array_t<process_bydir_entry> entries(count);
				array_t<string8> strings(count);

				unsigned n;

				{
					for (n = 0; n < count; n++)
					{
						api->get_relative_path(library[n], strings[n]);
						entries[n].m_path = strings[n];
						entries[n].m_item = library[n].get_ptr();
					}
				}

				{
					pfc::sort_t(entries, process_bydir_entry::g_compare, count);
				}

				{
					process_level_recur_t(entries.get_ptr(), count, &m_root);
				}
			}
		}
		else
		{
			metadb_handle_list library;

			api->get_all_items(library);

			const unsigned count = library.get_count();

			if (count > 0)
			{
				service_ptr_t<titleformat_object> script;

				static_api_ptr_t<titleformat_compiler>()->compile_safe(script, get_hierarchy());

				array_t<process_byformat_entry> entries(count);
				array_t<string8> strings(count);
				string8_fastalloc formatbuffer;

				unsigned n;

				{
					for (n = 0; n < count; n++)
					{
						metadb_handle * item = library[n].get_ptr();
						item->format_title(0, formatbuffer, script, &titleformat_text_filter_impl_reserved_chars("|"));
						strings[n] = formatbuffer;
						entries[n].m_path = strings[n];
						entries[n].m_item = library[n].get_ptr();
					}
				}

				{
					pfc::sort_t(entries, process_byformat_entry::g_compare, count);
				}

				{
					process_level_recur_t(entries.get_ptr(), count, &m_root);
				}
			}
		}
#endif
#ifdef USE_TIMER
		hires_timer timer;
		timer.start();
#endif
		try {
			refresh_tree_internal();
		}
		catch (pfc::exception const & e) {
			popup_message::g_show(string_formatter() << "Album list panel: An error occured while generating the tree (" << e << ").", "Error", popup_message::icon_error);
			m_root.release();
		}

		if (m_root.is_valid())
		{

			//if (!b_sort)
			m_root->sort_children();

			{
				metadb_handle_list_t<pfc::alloc_fast_aggressive> entries;

				service_ptr_t<titleformat_object> sort_script;
				//if (b_sort) static_api_ptr_t<titleformat_compiler>()->compile_safe(sort_script,cfg_sort_order);
				//if (sort_script.is_empty()) b_sort = false;

				TRACK_CALL_TEXT("album_list_panel_setup_tree");
				setup_tree(wnd_tv, TVI_ROOT, m_root, 0, 0, 0, entries/*,b_sort,sort_script*/);
			}
		}
#ifdef USE_TIMER
		console::formatter() << "Album list panel: initialised in " << pfc::format_float(timer.query(), 0, 3) << " s";
#endif

#if 0

		g_enum_callback.run(this);

		//pointers to hierarchy strings kept by nodes for sorting

		if (!b_sort) m_root.sort_children();

		{
			metadb_handle_list_fast entries;

			service_ptr_t<titleformat_object> sort_script;
			if (b_sort) static_api_ptr_t<titleformat>()->compile_safe(sort_script, cfg_sort_order);

			setup_tree(wnd_tv, TVI_ROOT, &m_root, 0, 0, 0, entries, b_sort, sort_script);
		}
#endif

		uSendMessage(wnd_tv, WM_SETREDRAW, TRUE, 0);
	}
}
