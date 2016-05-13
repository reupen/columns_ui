#include "stdafx.h"
#include "filter.h"

namespace filter_panel {
	void filter_panel_t::populate_list_from_chain(const metadb_handle_list_t<pfc::alloc_fast> & handles, bool b_last_in_chain)
	{
		//pfc::hires_timer t0;
		//t0.start();
		//SendMessage(get_wnd(), WM_SETREDRAW, FALSE, NULL);
		bool b_redraw = disable_redrawing();
		pfc::list_t<pfc::string_simple_t<WCHAR> > previous_nodes;
		bool b_all_was_selected = false;
		if (m_nodes.get_count())
		{
			pfc::list_t<bool> sel_data;
			sel_data.set_count(m_nodes.get_count());
			bit_array_var_table selection(sel_data.get_ptr(), sel_data.get_count());
			get_selection_state(selection);
			t_size i, count = sel_data.get_count();
			b_all_was_selected = selection[0];
			for (i = 1; i<count; i++)
				if (selection[i])
					previous_nodes.add_item(m_nodes[i].m_value);
		}
		//console::formatter() << "popc: " << t0.query_reset();
		populate_list(handles);
		//t0.query_reset();
		{
			t_size i, count = previous_nodes.get_count();
			pfc::array_t<bool> new_selection;
			new_selection.set_count(m_nodes.get_count());
			new_selection.fill_null();
			if (count || b_all_was_selected)
			{
				bool b_found = false;
				new_selection[0] = b_all_was_selected;
				for (i = 0; i<count; i++)
				{
					t_size index;
					if (mmh::bsearch_partial_t(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, previous_nodes[i].get_ptr(), 1, index))
					{
						new_selection[index] = true;
						b_found = true;
					}
				}
				if (!b_found)
					new_selection[0] = true; //m_nodes.get_count() >= 1
				set_selection_state(bit_array_var_table(new_selection.get_ptr(), new_selection.get_count()),
					bit_array_var_table(new_selection.get_ptr(), new_selection.get_count()), false);
				//if (b_last_in_chain)
				//	send_results_to_playlist();
				//else
				//	update_subsequent_filters();
			}
			else
			{
			}
		}
		if (b_redraw)
			enable_redrawing();
		//SendMessage(get_wnd(), WM_SETREDRAW, TRUE, NULL);
		//RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
		//console::formatter() << "popc: " << t0.query_reset();
	}

	void filter_panel_t::_on_items_added(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);


		metadb_handle_list actualHandles = handles;
#ifdef FILTER_OLD_SEARCH
		bool b_filter = m_query_active && strnlen(m_search_query, 1);
		if (b_filter)
		{
			try {
				search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_search_query);
				pfc::array_t<bool> data;
				data.set_size(actualHandles.get_count());
				api->test_multi(actualHandles, data.get_ptr());
				actualHandles.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
			}
			catch (pfc::exception const &) {};
		}
#endif
		metadb_handle_list_t<pfc::alloc_fast_aggressive> handlesNotifyNext;
		handlesNotifyNext.prealloc(actualHandles.get_count());

		m_nodes[0].m_handles.add_items(actualHandles);

		bool b_no_selection = get_selection_count(1) == 0 || get_item_selected(0);

		{
			pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;
			//data0.prealloc(handles.get_count());


			get_data_entries_v2(actualHandles.get_ptr(), actualHandles.get_count(), data0, g_showemptyitems);

			mmh::permutation_t permutation(data0.get_count());
			mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);

			pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
			pfc::list_base_const_t<data_entry_t> & data = data2;

			pfc::list_t<t_list_view::t_item_insert, pfc::alloc_fast_aggressive> items;
			items.prealloc(data.get_count());

			{
				if (!m_field_data.is_empty())
				{


					data_entry_t * p_data = data0.get_ptr();
					t_size * perm = permutation.get_ptr();

					//node_t node;
					t_size i, count = data.get_count(), counter = 0;

					for (i = 0; i<count; i++)
						if (i + 1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i + 1]].m_text.get_ptr())))
							counter++;

					for (i = 0; i<count; i++)
					{
						t_size start = i;
						while (p_data[perm[i]].m_same_as_next && i + 1<count)
							i++;
						t_size handles_count = 1 + i - start, k;

						t_size index_item;
						bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, p_data[perm[start]].m_text.get_ptr(), 1, index_item);
						if (b_exact)
						{
							t_size current_count = m_nodes[index_item].m_handles.get_count();
							m_nodes[index_item].m_handles.set_count(current_count + handles_count);

							bool b_selected = !b_no_selection && get_item_selected(index_item);

							for (k = 0; k<handles_count; k++)
								m_nodes[index_item].m_handles[current_count + k] = p_data[perm[start + k]].m_handle;

							if (b_selected && handles_count)
								handlesNotifyNext.add_items_fromptr(m_nodes[index_item].m_handles.get_ptr() + current_count, handles_count);
						}
						else
						{
							node_t node;
							node.m_value = p_data[perm[start]].m_text.get_ptr();
							node.m_handles.set_count(handles_count);

							for (k = 0; k<handles_count; k++)
								node.m_handles[k] = p_data[perm[start + k]].m_handle;

							m_nodes.insert_item(node, index_item);
							t_item_insert item;
							insert_items(index_item, 1, &item);
						}
					}

					update_first_node_text(true);
				}
			}
		}
		if (index + 1<windows.get_count())
		{
			//if (index==0)
			//	g_update_subsequent_filters(windows, index+1, false, false);
			if (b_no_selection)
				windows[index + 1]->_on_items_added(actualHandles);
			else if (handlesNotifyNext.get_count())
				windows[index + 1]->_on_items_added(handlesNotifyNext);
		}
	}

	void filter_panel_t::_on_items_removed(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);

		metadb_handle_list_t<pfc::alloc_fast_aggressive> handlesNotifyNext;
		handlesNotifyNext.prealloc(handles.get_count());

		bool b_no_selection = get_selection_count(1) == 0 || get_item_selected(0);
		{
			t_size j, count = handles.get_count();
			for (j = 0; j<count; j++)
				m_nodes[0].m_handles.remove_item(handles[j]);
		}
		{
			pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;
			//data0.prealloc(handles.get_count());


			get_data_entries_v2(handles, data0, g_showemptyitems);

			mmh::permutation_t permutation(data0.get_count());
			mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);

			pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
			pfc::list_base_const_t<data_entry_t> & data = data2;

			{
				if (!m_field_data.is_empty())
				{

					data_entry_t * p_data = data0.get_ptr();
					t_size * perm = permutation.get_ptr();

					//node_t node;
					t_size i, count = data.get_count(), counter = 0;

					for (i = 0; i<count; i++)
						if (i + 1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i + 1]].m_text.get_ptr())))
							counter++;

					pfc::array_t<bool> mask0;
					mask0.set_count(m_nodes.get_count());
					mask0.fill_null();


					for (i = 0; i<count; i++)
					{
						t_size start = i;
						while (p_data[perm[i]].m_same_as_next && i + 1<count)
							i++;
						t_size handles_count = 1 + i - start;

						t_size index_item;
						bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, p_data[perm[start]].m_text.get_ptr(), 1, index_item);
						if (b_exact)
						{
							//t_size current_count = m_nodes[index_item].m_handles.get_count();
							//m_nodes[index_item].m_handles.set_count(handles_count);

							bool b_selected = !b_no_selection && get_item_selected(index_item);

							t_size k;
							for (k = 0; k<handles_count; k++)
							{
								m_nodes[index_item].m_handles.remove_item(p_data[perm[start + k]].m_handle);
								if (b_selected) handlesNotifyNext.add_item(p_data[perm[start + k]].m_handle);
							}


							if (m_nodes[index_item].m_handles.get_count() == 0)
							{
								mask0[index_item] = true;
							}
						}
					}

#if 0
					node_t node;
					t_size i, count = data.get_count();
					for (i = 0; i<count; i++)
					{
						node.m_handles.add_item(data[i].m_handle);
						if (i + 1 == count || StrCmpLogicalW(data[i].m_text.get_ptr(), data[i + 1].m_text.get_ptr()))
						{
							t_size index_item;
							bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, data[i].m_text.get_ptr(), 1, index_item);
							if (b_exact)
							{
								t_size j, countj = node.m_handles.get_count();
								for (j = 0; j<countj; j++)
									m_nodes[index_item].m_handles.remove_item(node.m_handles[j]);
								if (m_nodes[index_item].m_handles.get_count() == 0)
								{
									mask0[index_item] = true;
								}
							}
						}
					}
#endif
					m_nodes.remove_mask(mask0.get_ptr());
					remove_items(bit_array_table(mask0.get_ptr(), mask0.get_size()));
					update_first_node_text(true);
				}
			}
		}
		//if (index==0)
		//	g_update_subsequent_filters(windows, index+1, false, false);
		if (index + 1<windows.get_count())
		{
			if (b_no_selection)
				windows[index + 1]->_on_items_removed(handles);
			else if (handlesNotifyNext.get_count())
				windows[index + 1]->_on_items_removed(handlesNotifyNext);
		}

	}

	void filter_panel_t::on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);
		if (index == 0 || index == pfc_infinite)
		{
			_on_items_added(metadb_handle_list_t<pfc::alloc_fast_aggressive>(handles));
		}
	}
	void filter_panel_t::on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);
		if (index == 0 || index == pfc_infinite)
		{
			_on_items_removed(metadb_handle_list_t<pfc::alloc_fast_aggressive>(handles));
		}
	}

	void filter_panel_t::_on_items_modified(const metadb_handle_list_t<pfc::alloc_fast_aggressive> & handles)
	{
		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);

		pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;//, dataFilter;
		data0.prealloc(handles.get_count());


		metadb_handle_list actualHandles = handles;
#ifdef FILTER_OLD_SEARCH
		bool b_filter = m_query_active && strnlen(m_search_query, 1);
		if (b_filter)
		{
			try {
				search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_search_query);
				pfc::array_t<bool> data;
				data.set_size(actualHandles.get_count());
				api->test_multi(actualHandles, data.get_ptr());
				actualHandles.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
			}
			catch (pfc::exception const &) {};
			//dataFilter.prealloc(filterHandles.get_count());
			//get_data_entries_v2(filterHandles, dataFilter, g_showemptyitems);

			//permutationFilter.set_count(dataFilter.get_count());
			//mmh::g_sort_get_permutation_qsort_v2(dataFilter.get_ptr(), permutationFilter, data_entry_t::g_compare, false);

			t_size j, count = handles.get_count();
			for (j = 0; j<count; j++)
				m_nodes[0].m_handles.remove_item(handles[j]);
			m_nodes[0].m_handles.add_items(actualHandles);
		}
#endif

		metadb_handle_list_t<pfc::alloc_fast_aggressive> handlesNotifyNext;
		handlesNotifyNext.prealloc(actualHandles.get_count());

		get_data_entries_v2(actualHandles.get_ptr(), actualHandles.get_count(), data0, g_showemptyitems);

		mmh::permutation_t permutation(data0.get_count());
		mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);

		pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
		pfc::list_base_const_t<data_entry_t> & data = data2;
		//pfc::list_permutation_t<data_entry_t> dataFilter2(dataFilter, permutationFilter.get_ptr(), permutationFilter.get_count());

		//pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & dataAdd = b_filter ? dataFilter2 : data2;

		bool b_no_selection = get_selection_count(1) == 0 || get_item_selected(0);

		{
			if (!m_field_data.is_empty())
			{
				{
					t_size j, countj = handles.get_count();
					t_size k, countk = m_nodes.get_count();
					for (k = 1; k<countk; k++)
					{
						pfc::array_t<bool> mask;
						mask.set_count(m_nodes[k].m_handles.get_count());
						mask.fill_null();
						for (j = 0; j<countj; j++)
						{
							t_size index_found = m_nodes[k].m_handles.find_item(handles[j]);
							if (index_found != pfc_infinite)
								mask[index_found] = true;
						}
						m_nodes[k].m_handles.remove_mask(mask.get_ptr());
					}
				}

				data_entry_t * p_data = data0.get_ptr();
				t_size * perm = permutation.get_ptr();

				//node_t node;
				t_size i, count = data.get_count(), counter = 0;

				for (i = 0; i<count; i++)
					if (i + 1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i + 1]].m_text.get_ptr())))
						counter++;

				for (i = 0; i<count; i++)
				{
					t_size start = i;
					while (p_data[perm[i]].m_same_as_next && i + 1<count)
						i++;
					t_size handles_count = 1 + i - start, k;

					t_size index_item;
					bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, p_data[perm[start]].m_text.get_ptr(), 1, index_item);
					if (b_exact)
					{
						t_size current_count = m_nodes[index_item].m_handles.get_count();
						m_nodes[index_item].m_handles.set_count(current_count + handles_count);

						bool b_selected = !b_no_selection && get_item_selected(index_item);

						for (k = 0; k<handles_count; k++)
						{
							m_nodes[index_item].m_handles[current_count + k] = p_data[perm[start + k]].m_handle;
						}
						if (b_selected && handles_count)
							handlesNotifyNext.add_items_fromptr(m_nodes[index_item].m_handles.get_ptr() + current_count, handles_count);
					}
					else
					{
						node_t node;
						node.m_value = p_data[perm[start]].m_text.get_ptr();
						node.m_handles.set_count(handles_count);

						for (k = 0; k<handles_count; k++)
							node.m_handles[k] = p_data[perm[start + k]].m_handle;

						m_nodes.insert_item(node, index_item);
						t_item_insert item;
						insert_items(index_item, 1, &item, false);
					}
				}

#if 0
				node_t node;
				t_size i, count = data.get_count();
				for (i = 0; i<count; i++)
				{
					node.m_handles.add_item(data[i].m_handle);
					if (i + 1 == count || StrCmpLogicalW(data[i].m_text.get_ptr(), data[i + 1].m_text.get_ptr()))
					{
						t_size index_item;
						bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, data[i].m_text.get_ptr(), 1, index_item);
						if (b_exact)
						{
							/*t_size j, countj = node.m_handles.get_count();
							for (j=0; j<countj; j++)
							if (!*/
							m_nodes[index_item].m_handles.add_items(node.m_handles);
							node.m_handles.remove_all();
							//metadb_handle_list_helper::remove_duplicates(m_nodes[index_item].m_handles);
						}
						else
						{
							t_string_list_fast temp;
							node.m_value = data[i].m_text.get_ptr();
							m_nodes.insert_item(node, index_item);
							insert_items(index_item, 1, &t_item_insert());
							node.m_handles.remove_all();
						}
					}
				}
#endif
				{
					t_size k, countk = m_nodes.get_count();
					pfc::array_t<bool> mask0;
					mask0.set_count(countk);
					mask0[0] = false;
					//mask0.fill_null();
					for (k = 1; k<countk; k++)
					{
						mask0[k] = m_nodes[k].m_handles.get_count() == 0;
					}
					m_nodes.remove_mask(mask0.get_ptr());
					remove_items(bit_array_table(mask0.get_ptr(), mask0.get_size()), true);
				}
				update_first_node_text(true);
			}
		}
		if (index + 1<windows.get_count())
		{
#ifdef FILTER_OLD_SEARCH
			if (b_filter)
				g_update_subsequent_filters(windows, index + 1, false, false);
			else
#endif
				if (b_no_selection)
					windows[index + 1]->_on_items_modified(actualHandles);
				else if (handlesNotifyNext.get_count())
					windows[index + 1]->_on_items_modified(handlesNotifyNext);
		}
	}

	void filter_panel_t::on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & handles)
	{
		//pfc::hires_timer timer;
		//timer.start();

		pfc::ptr_list_t<filter_panel_t> windows;
		get_windows(windows);
		t_size index = windows.find_item(this);
		if (index == 0 || index == pfc_infinite)
		{
#if 1
			metadb_handle_list_t<pfc::alloc_fast_aggressive> actualHandles = handles;
			_on_items_modified(actualHandles);
#else
			{
				pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;//, dataFilter;
				data0.prealloc(handles.get_count());

				//mmh::permutation_t permutationFilter;
				metadb_handle_list actualHandles = handles;
				bool b_filter = m_query_active && strnlen(m_search_query, 1);
				if (b_filter)
				{
					try {
						search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_search_query);
						pfc::array_t<bool> data;
						data.set_size(actualHandles.get_count());
						api->test_multi(actualHandles, data.get_ptr());
						actualHandles.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
					}
					catch (pfc::exception const &) {};
					//dataFilter.prealloc(filterHandles.get_count());
					//get_data_entries_v2(filterHandles, dataFilter, g_showemptyitems);

					//permutationFilter.set_count(dataFilter.get_count());
					//mmh::g_sort_get_permutation_qsort_v2(dataFilter.get_ptr(), permutationFilter, data_entry_t::g_compare, false);

					t_size j, count = handles.get_count();
					for (j = 0; j<count; j++)
						m_nodes[0].m_handles.remove_item(handles[j]);
					m_nodes[0].m_handles.add_items(actualHandles);
				}

				metadb_handle_list_t<pfc::alloc_fast_aggressive> handlesNotifyNext;
				handlesNotifyNext.prealloc(actualHandles.get_count());

				get_data_entries_v2(actualHandles.get_ptr(), actualHandles.get_count(), data0, g_showemptyitems);

				mmh::permutation_t permutation(data0.get_count());
				mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);

				pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
				pfc::list_base_const_t<data_entry_t> & data = data2;
				//pfc::list_permutation_t<data_entry_t> dataFilter2(dataFilter, permutationFilter.get_ptr(), permutationFilter.get_count());

				//pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & dataAdd = b_filter ? dataFilter2 : data2;

				{
					if (!m_field_data.is_empty())
					{
						{
							t_size j, countj = handles.get_count();
							t_size k, countk = m_nodes.get_count();
							for (k = 1; k<countk; k++)
							{
								pfc::array_t<bool> mask;
								mask.set_count(m_nodes[k].m_handles.get_count());
								mask.fill_null();
								for (j = 0; j<countj; j++)
								{
									t_size index_found = m_nodes[k].m_handles.find_item(handles[j]);
									if (index_found != pfc_infinite)
										mask[index_found] = true;
								}
								m_nodes[k].m_handles.remove_mask(mask.get_ptr());
							}
						}

						data_entry_t * p_data = data0.get_ptr();
						t_size * perm = permutation.get_ptr();

						//node_t node;
						t_size i, count = data.get_count(), counter = 0;

						for (i = 0; i<count; i++)
							if (i + 1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i + 1]].m_text.get_ptr())))
								counter++;

						for (i = 0; i<count; i++)
						{
							t_size start = i;
							while (p_data[perm[i]].m_same_as_next && i + 1<count)
								i++;
							t_size handles_count = 1 + i - start, k;

							t_size index_item;
							bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, p_data[perm[start]].m_text.get_ptr(), 1, index_item);
							if (b_exact)
							{
								t_size current_count = m_nodes[index_item].m_handles.get_count();
								m_nodes[index_item].m_handles.set_count(current_count + handles_count);

								bool b_selected = get_item_selected(index_item);

								for (k = 0; k<handles_count; k++)
								{
									m_nodes[index_item].m_handles[current_count + k] = p_data[perm[start + k]].m_handle;
								}
								if (b_selected && handles_count)
									handlesNotifyNext.add_items_fromptr(m_nodes[index_item].m_handles.get_ptr() + current_count, handles_count);
							}
							else
							{
								node_t node;
								node.m_value = p_data[perm[start]].m_text.get_ptr();
								node.m_handles.set_count(handles_count);

								for (k = 0; k<handles_count; k++)
									node.m_handles[k] = p_data[perm[start + k]].m_handle;

								m_nodes.insert_item(node, index_item);
								insert_items(index_item, 1, &t_item_insert(), false);
							}
						}

#if 0
						node_t node;
						t_size i, count = data.get_count();
						for (i = 0; i<count; i++)
						{
							node.m_handles.add_item(data[i].m_handle);
							if (i + 1 == count || StrCmpLogicalW(data[i].m_text.get_ptr(), data[i + 1].m_text.get_ptr()))
							{
								t_size index_item;
								bool b_exact = mmh::bsearch_partial_t(m_nodes.get_count() - 1, m_nodes, node_t::g_compare, data[i].m_text.get_ptr(), 1, index_item);
								if (b_exact)
								{
									/*t_size j, countj = node.m_handles.get_count();
									for (j=0; j<countj; j++)
									if (!*/
									m_nodes[index_item].m_handles.add_items(node.m_handles);
									node.m_handles.remove_all();
									//metadb_handle_list_helper::remove_duplicates(m_nodes[index_item].m_handles);
								}
								else
								{
									t_string_list_fast temp;
									node.m_value = data[i].m_text.get_ptr();
									m_nodes.insert_item(node, index_item);
									insert_items(index_item, 1, &t_item_insert());
									node.m_handles.remove_all();
								}
							}
						}
#endif
						{
							t_size k, countk = m_nodes.get_count();
							pfc::array_t<bool> mask0;
							mask0.set_count(countk);
							mask0[0] = false;
							//mask0.fill_null();
							for (k = 1; k<countk; k++)
							{
								mask0[k] = m_nodes[k].m_handles.get_count() == 0;
							}
							m_nodes.remove_mask(mask0.get_ptr());
							remove_items(bit_array_table(mask0.get_ptr(), mask0.get_size()), true);
						}
						update_first_node_text(true);
					}
				}
			}
			if (index == 0)
				g_update_subsequent_filters(windows, index + 1, false, false);
			//if (index+1<windows.get_count())
			//	windows[index+1]->on_items_modified(handlesNotifyNext);
#endif
		}

		//console::formatter() << "filter_panel_t::on_items_modified() " << timer.query() << " s";
	}

#if 0
	void filter_panel_t::get_data_entries(const pfc::list_base_const_t<metadb_handle_ptr> & handles, pfc::list_base_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out)
	{
		pfc::stringcvt::string_wide_from_utf8_t<pfc::alloc_fast_aggressive> str_utf16;
		if (!m_field_data.is_empty())
		{
			if (m_field_data.m_use_script)
			{
				pfc::string8_fastalloc buffer;
				titleformat_object_wrapper to(m_field_data.m_script);
				buffer.prealloc(32);
				t_size i, count = handles.get_count();
				for (i = 0; i<count; i++)
				{
					data_entry_t temp;
					temp.m_handle = handles[i];
					temp.m_handle->format_title(NULL, buffer, to, NULL);
					str_utf16.convert(buffer);
					temp.m_text = str_utf16.get_ptr();
					p_out.add_item(temp);
				}
			}
			else
			{
				in_metadb_sync sync;
				t_size i, count = handles.get_count(), k, kcount = m_field_data.m_fields.get_count();
				for (i = 0; i<count; i++)
				{
					const file_info * info = NULL;
					if (handles[i]->get_info_locked(info))
					{
						for (k = 0; k<kcount; k++)
						{
							t_size j, fieldcount = info->meta_get_count_by_name(m_field_data.m_fields[k]);
							for (j = 0; j<fieldcount; j++)
							{
								data_entry_t temp;
								temp.m_handle = handles[i];
								//temp.m_text_utf8 = info->meta_get(m_field, j);
								str_utf16.convert(info->meta_get(m_field_data.m_fields[k], j));
								temp.m_text = str_utf16.get_ptr();
								p_out.add_item(temp);
							}
							if (fieldcount)
								break;
						}
					}
				}
			}
		}
	}
#endif

	void filter_panel_t::get_data_entries_v2(const pfc::list_base_const_t<metadb_handle_ptr> & handles, pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out, bool b_show_empty)
	{
		metadb_handle_list p_handles(handles);
		get_data_entries_v2(p_handles.get_ptr(), p_handles.get_count(), p_out, b_show_empty);
	}

	void filter_panel_t::get_data_entries_v2(const metadb_handle_ptr * p_handles, t_size count, pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> & p_out, bool b_show_empty)
	{
#if 0
		p_out.set_count(count);
		pfc::stringcvt::string_wide_from_utf8_t<pfc::alloc_fast_aggressive> str_utf16;
		pfc::string8_fast_aggressive buffer;
		pfc::string8 spec;
		spec << "%" << m_field << "%";
		titleformat_object::ptr to;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to, spec);
		t_size i;
		for (i = 0; i<count; i++)
		{
			p_out[i].m_handle = p_handles[i];
			p_handles[i]->format_title(NULL, buffer, to, NULL);
			str_utf16.convert(buffer);
			p_out[i].m_text = str_utf16;
		}
#else
		class handle_info_t
		{
		public:
			metadb_info_container::ptr m_info;
			t_size m_value_count;
			t_size m_field_index;
		};
		//pfc::stringcvt::string_wide_from_utf8_t<pfc::alloc_fast_aggressive> str_utf16;
		if (!m_field_data.is_empty())
		{

			if (m_field_data.m_use_script)
			{
				pfc::string8_fastalloc buffer;
				titleformat_object_wrapper to(m_field_data.m_script);
				buffer.prealloc(32);
				p_out.set_count(count);
				data_entry_t * pp_out = p_out.get_ptr();
				t_size i, k = 0;
				for (i = 0; i<count; i++)
				{
					p_handles[i]->format_title(nullptr, buffer, to, nullptr);
					if (b_show_empty || pfc::strlen_max(buffer, 1))
					{
						pp_out[k].m_handle = p_handles[i];
						pp_out[k].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(buffer));
						pfc::stringcvt::convert_utf8_to_wide_unchecked(pp_out[k].m_text.get_ptr(), buffer);
						//int size = LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, NULL, NULL);
						//pp_out[k].m_sortkey.set_size(size);
						//LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, (LPWSTR)pp_out[k].m_sortkey.get_ptr(), size);
						k++;
					}
				}
				p_out.set_count(k);
			}
			else
			{
				pfc::list_t<handle_info_t> infos;
				infos.set_count(count);
				handle_info_t * p_infos = infos.get_ptr();

				t_size i, counter = 0, k = 0, l, lcount = m_field_data.m_fields.get_count();

#if 1
				for (i = 0; i<count; i++)
				{
					if (p_handles[i]->get_info_ref(p_infos[i].m_info))
					{
						for (l = 0; l<lcount; l++)
						{
							p_infos[i].m_field_index = p_infos[i].m_info->info().meta_find(m_field_data.m_fields[l]);
							p_infos[i].m_value_count = p_infos[i].m_field_index != pfc_infinite ? p_infos[i].m_info->info().meta_enum_value_count(p_infos[i].m_field_index) : 0;
							counter += p_infos[i].m_value_count;
							if (p_infos[i].m_value_count)
								break;
						}
					}
					else p_infos[i].m_value_count = 0;
				}

				p_out.set_count(counter);

				data_entry_t * pp_out = p_out.get_ptr();
				for (i = 0; i<count; i++)
				{
					t_size j;
					for (j = 0; j<p_infos[i].m_value_count; j++)
					{
						const char * str = p_infos[i].m_info->info().meta_enum_value(p_infos[i].m_field_index, j);
						if (b_show_empty || *str)
						{
							pp_out[k].m_handle = p_handles[i];
							pp_out[k].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(str));
							pfc::stringcvt::convert_utf8_to_wide_unchecked(pp_out[k].m_text.get_ptr(), str);
							//int size = LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, NULL, NULL);
							//pp_out[k].m_sortkey.set_size(size);
							//LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, (LPWSTR)pp_out[k].m_sortkey.get_ptr(), size);
							k++;
						}
					}
				}
				p_out.set_count(k);

#else
				class thread_get_meta_t : public mmh::thread_v2_t
				{
					DWORD on_thread()
					{
						t_size i, l, lcount = m_field_data->m_fields.get_count();

						for (i = m_thread_index; i<m_item_count; i += m_thread_count)
						{
							if (m_handles[i]->get_info_locked(m_infos[i].m_info))
							{
								for (l = 0; l<lcount; l++)
								{
									m_infos[i].m_field_index = m_infos[i].m_info->meta_find(m_field_data->m_fields[l]);
									m_infos[i].m_value_count = m_infos[i].m_field_index != pfc_infinite ? m_infos[i].m_info->meta_enum_value_count(m_infos[i].m_field_index) : 0;
									m_counter += m_infos[i].m_value_count;
									if (m_infos[i].m_value_count)
										break;
								}
							}
							else m_infos[i].m_value_count = 0;
						}

						m_done.set_state(true);
						m_continue.wait_for(-1);

						//count = m_out->get_count();
						data_entry_t * pp_out = m_out->get_ptr();
						for (i = m_thread_index; i<m_item_count; i += m_thread_count)
						{
							t_size j;
							for (j = 0; j<m_infos[i].m_value_count; j++)
							{
								const char * str = m_infos[i].m_info->meta_enum_value(m_infos[i].m_field_index, j);
								if (m_show_empty || *str)
								{
									long insert = InterlockedIncrement(m_interlocked_position) - 1;
									pp_out[insert].m_handle = m_handles[i];
									pp_out[insert].m_text.set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(str));
									pfc::stringcvt::convert_utf8_to_wide_unchecked(pp_out[insert].m_text.get_ptr(), str);
									//int size = LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, NULL, NULL);
									//pp_out[k].m_sortkey.set_size(size);
									//LCMapString(LOCALE_USER_DEFAULT, LCMAP_SORTKEY, pp_out[k].m_text.get_ptr(), -1, (LPWSTR)pp_out[k].m_sortkey.get_ptr(), size);
								}
							}
						}

						return 0;
					}
				public:

					thread_get_meta_t() : m_counter(0) {};

					handle_info_t * m_infos;
					const metadb_handle_ptr * m_handles;
					t_size m_thread_index, m_thread_count, m_item_count, m_counter;
					const field_data_t * m_field_data;

					pfc::array_t<bool> * m_mask_remove;
					pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> * m_out;
					bool m_show_empty;

					volatile unsigned long * m_interlocked_position;

					win32_event m_done;
					win32_event m_continue;
				};

				t_size threadIndex, threadCount = GetOptimalWorkerThreadCount();
				volatile unsigned long interlocked_position = 0;

				pfc::array_staticsize_t<thread_get_meta_t> threads(threadCount);
				pfc::array_staticsize_t<HANDLE> threadHandles(threadCount), waitHandles(threadCount);

				pfc::array_t<bool> mask;

				for (threadIndex = 0; threadIndex<threadCount; threadIndex++)
				{
					threads[threadIndex].m_infos = p_infos;
					threads[threadIndex].m_handles = p_handles;
					threads[threadIndex].m_thread_count = threadCount;
					threads[threadIndex].m_thread_index = threadIndex;
					threads[threadIndex].m_item_count = count;
					threads[threadIndex].m_field_data = &m_field_data;
					//threads[threadIndex].m_mask_remove = &mask;
					threads[threadIndex].m_show_empty = b_show_empty;
					threads[threadIndex].m_out = &p_out;
					threads[threadIndex].m_interlocked_position = &interlocked_position;

					threads[threadIndex].m_done.create(true, false);
					threads[threadIndex].m_continue.create(true, false);
					waitHandles[threadIndex] = threads[threadIndex].m_done.get();
					threads[threadIndex].create_thread();
					threadHandles[threadIndex] = threads[threadIndex].get_thread();
				}

				WaitForMultipleObjects(threadCount, waitHandles.get_ptr(), TRUE, pfc_infinite);

				for (threadIndex = 0; threadIndex<threadCount; threadIndex++)
				{
					counter += threads[threadIndex].m_counter;
				}

				p_out.set_count(counter);

				for (threadIndex = 0; threadIndex<threadCount; threadIndex++)
				{
					threads[threadIndex].m_continue.set_state(true);
				}

				WaitForMultipleObjects(threadCount, threadHandles.get_ptr(), TRUE, pfc_infinite);

				for (threadIndex = 0; threadIndex<threadCount; threadIndex++)
				{
					threads[threadIndex].release_thread();
				}

				p_out.set_count(interlocked_position);
#endif
			}
#if 0
			for (i = 0; i<count; i++)
			{
				const file_info * info = NULL;
				if (p_handles[i]->get_info_locked(info))
				{
					t_size j, fieldcount = info->meta_get_count_by_name(m_field);
					for (j = 0; j<fieldcount; j++)
					{
						data_entry_t temp;
						temp.m_handle = p_handles[i];
						//temp.m_text_utf8 = info->meta_get(m_field, j);
						str_utf16.convert(info->meta_get(m_field, j));
						temp.m_text = str_utf16.get_ptr();
						p_out.add_item(temp);
					}
				}
			}
#endif
		}
#endif
	}

	void filter_panel_t::populate_list(const metadb_handle_list_t<pfc::alloc_fast> & handles)
	{
		clear_all_items();
		m_nodes.remove_all();

		//m_nodes.prealloc(handles.get_count());

		pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> data0;
		//data0.prealloc(handles.get_count());

#ifdef FILTER_OLD_SEARCH
		metadb_handle_list filtered_list;
		bool b_filter = m_query_active && strnlen(m_search_query, 1);
		const metadb_handle_list & actualHandles = b_filter ? filtered_list : handles;
		if (b_filter)
		{
			filtered_list = handles;
			try {
				search_filter::ptr api = static_api_ptr_t<search_filter_manager>()->create(m_search_query);
				pfc::array_t<bool> data;
				data.set_size(filtered_list.get_count());
				api->test_multi(filtered_list, data.get_ptr());
				filtered_list.remove_mask(bit_array_not(bit_array_table(data.get_ptr(), data.get_count())));
			}
			catch (pfc::exception const &) {};
		}
#else
		const metadb_handle_list & actualHandles = handles;
#endif

		{
			//pfc::hires_timer timer;
			//timer.start();
			get_data_entries_v2(actualHandles.get_ptr(), actualHandles.get_size(), data0, g_showemptyitems);
			//	console::formatter() << "get_data_entries_v2: " << timer.query() << " s";
		}

		mmh::permutation_t permutation(data0.get_count());
		{
			pfc::hires_timer timer;
			timer.start();
			mmh::g_sort_get_permutation_qsort_v2(data0.get_ptr(), permutation, data_entry_t::g_compare, false, false);
			//	console::formatter() << "g_sort_get_permutation_qsort_v2: " << timer.query() << " s";
		}

		//data.reorder(permutation.get_ptr());
		pfc::list_permutation_t<data_entry_t> data2(data0, permutation.get_ptr(), permutation.get_count());
		pfc::list_base_const_t<data_entry_t> & data = data2;

		pfc::list_t<t_list_view::t_item_insert, pfc::alloc_fast_aggressive> items;
		items.prealloc(data.get_count());
		{
			{
				data_entry_t * p_data = data0.get_ptr();
				t_size * perm = permutation.get_ptr();
				//pfc::hires_timer timer;
				//timer.start();
				t_size i, count = data.get_count(), j;
				t_size counter = 0;
#if 1
				for (i = 0; i<count; i++)
					if (i + 1 == count || !(p_data[perm[i]].m_same_as_next = !StrCmpLogicalW(p_data[perm[i]].m_text.get_ptr(), p_data[perm[i + 1]].m_text.get_ptr())))
						//if (i +1 == count || !(p_data[perm[i]].m_same_as_next = !CompareString(LOCALE_USER_DEFAULT, NULL, p_data[perm[i]].m_text.get_ptr(), -1, p_data[perm[i+1]].m_text.get_ptr(), -1)))
						counter++;
				//counter++;
				/*if (i +1 == count)
				counter++ ;
				else
				{
				t_size a = p_data[perm[i]].m_sortkey.get_size(), b = p_data[perm[i+1]].m_sortkey.get_size();
				if (!(p_data[perm[i]].m_same_as_next = !memcmp(p_data[perm[i]].m_sortkey.get_ptr(), p_data[perm[i+1]].m_sortkey.get_ptr(), min(a,b))))
				counter++;
				}*/

#else
				class thread_impl : public mmh::thread_t
				{
				public:
					pfc::list_t<data_entry_t, pfc::alloc_fast_aggressive> * entries;
					mmh::permutation_t * perm;
					t_size index, count, counter;

					thread_impl() : counter(0) {};

					DWORD on_thread()
					{
						t_size total_count = entries->get_count(), i;
						for (i = 0; i<count; i++)
						{
							if (i + index + 1 >= total_count) break;
							data_entry_t & ptr = (*entries)[(((*perm)[i + index]))];
							if (!(ptr.m_same_as_next = !StrCmpLogicalW(entries->get_item_ref((*perm)[i + index]).m_text.get_ptr(), entries->get_item_ref((*perm)[i + index + 1]).m_text.get_ptr())))
								counter++;
						}
						return 0;
					}
				};
				thread_impl thread[2];
				t_size hc = tabsize(thread);
				pfc::array_t<HANDLE> hthreads;
				hthreads.set_count(hc);
				for (j = 0; j<hc; j++)
				{
					thread[j].entries = &data0;
					thread[j].perm = &permutation;
					thread[j].index = j*(count / hc);
					thread[j].count = j + 1 == hc ? (count / hc + (count - (count / hc)*hc)) : count / hc;
					thread[j].create_thread();
					hthreads[j] = thread[j].get_thread();
				}

				DWORD ret = WaitForMultipleObjects(hthreads.get_count(), hthreads.get_ptr(), TRUE, pfc_infinite);
				for (j = 0; j<hc; j++)
				{
					counter += thread[j].counter;
					thread[j].release_thread();
				}
				if (count) counter++;
#endif
				m_nodes.set_count(counter + 1);
				//pfc::list_t<node_t> & p_nodes = m_nodes;
				node_t * p_nodes = m_nodes.get_ptr();
				{
					p_nodes[0].m_handles.add_items(actualHandles);
					p_nodes[0].m_value.set_string(L"All");
				}

				for (i = 0, j = 1; i<count; i++)
				{
					t_size start = i;
					while (p_data[perm[i]].m_same_as_next && i + 1<count)
						i++;
					t_size handles_count = 1 + i - start, k;
#ifdef _DEBUG
					PFC_ASSERT(j < counter + 1);
#endif
					p_nodes[j].m_handles.set_count(handles_count);
					for (k = 0; k<handles_count; k++)
						p_nodes[j].m_handles[k] = p_data[perm[start + k]].m_handle;
					p_nodes[j].m_value = p_data[perm[start]].m_text.get_ptr();
					j++;
#if 0
					node_t node;
					if (i + 1 == count || !data[i].m_same_as_next /*StrCmpLogicalW(data[i].m_text, data[i+1].m_text)*/)
					{
						t_string_list_fast temp;
						node.m_value = data[i].m_text;
						//m_strings.add_item(data[i].m_text_utf8);
						//temp.add_item(data[i].m_text_utf8);
						m_nodes.add_item(node);
						node.m_handles.remove_all();
						//	start = i+1;
					}
#endif
				}
				update_first_node_text();
			}
		}
		items.set_count(m_nodes.get_count());
		insert_items(0, items.get_count(), items.get_ptr());
	}


}