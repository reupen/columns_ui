#include "stdafx.h"

// {EBD87879-65A7-4242-821B-812AF9F68E8F}
const GUID cui::fcl::groups::titles_playlist_view = 
{ 0xebd87879, 0x65a7, 0x4242, { 0x82, 0x1b, 0x81, 0x2a, 0xf9, 0xf6, 0x8e, 0x8f } };

// {F17DDDF4-BB3E-4f36-B9E1-D626629F2C76}
const GUID cui::fcl::groups::titles_common = 
{ 0xf17dddf4, 0xbb3e, 0x4f36, { 0xb9, 0xe1, 0xd6, 0x26, 0x62, 0x9f, 0x2c, 0x76 } };

enum {fcl_stream_version=1};

// {9FAADFF3-E51A-4a8b-B4A3-D209A36AB301}
static const GUID g_fcl_header = 
{ 0x9faadff3, 0xe51a, 0x4a8b, { 0xb4, 0xa3, 0xd2, 0x9, 0xa3, 0x6a, 0xb3, 0x1 } };

namespace treeview
{
	static HTREEITEM insert_item(HWND wnd_tree, const char * sz_text, LPARAM data, HTREEITEM ti_parent = TVI_ROOT, HTREEITEM ti_after = TVI_LAST)
	{
		uTVINSERTSTRUCT is;
		memset(&is,0,sizeof(is));
		is.hParent = ti_parent;
		is.hInsertAfter = ti_after;
		is.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
		is.item.pszText = const_cast<char*>(sz_text);
		is.item.state = TVIS_EXPANDED;
		is.item.stateMask = TVIS_EXPANDED;
		is.item.lParam = data;
		return uTreeView_InsertItem(wnd_tree,&is);
	}
}

class FCLDialog
{
public:
	class t_node
	{
	public:
		HTREEITEM item;
		cui::fcl::group_ptr group;
		bool checked;
		t_node(HTREEITEM pitem, cui::fcl::group_ptr ptr) : group(ptr), item(pitem), checked(true)
		{};
		t_node() : item(NULL), checked(true) {};
	};
	//cui::fcl::group_list m_groups;
	pfc::list_t<t_node> m_nodes;
	void g_populate_tree ( HWND wnd_tree, cui::fcl::group_list & list, const cui::fcl::group_list_filtered & filtered, HTREEITEM ti_parent = TVI_ROOT)
	{
		t_size i, count = filtered.get_count();
		for (i=0; i<count; i++)
		{
			pfc::string8 name;
			filtered[i]->get_name(name);
			HTREEITEM item = treeview::insert_item(wnd_tree, name, m_nodes.get_count() , ti_parent);
			m_nodes.add_item(t_node(item, filtered[i]));
			TreeView_SetCheckState(wnd_tree, item, TRUE);
			cui::fcl::group_list_filtered filtered2(list, filtered[i]->get_guid());
			list.remove_by_guid(filtered[i]->get_guid());
			g_populate_tree(wnd_tree, list, filtered2, item);
		}
	}
	static BOOL CALLBACK g_FCLDialogProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		FCLDialog * p_this = NULL;
		switch(msg)
		{
		case WM_INITDIALOG:
			SetWindowLongPtr(wnd, DWL_USER, lp);
			p_this = reinterpret_cast<FCLDialog*>(lp);
			break;
		default:
			p_this = reinterpret_cast<FCLDialog*>(GetWindowLongPtr(wnd, DWL_USER));
			break;
		}
		if (p_this)
			return p_this->FCLDialogProc(wnd, msg, wp, lp);

		return FALSE;
	}
	BOOL CALLBACK FCLDialogProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				if (m_import)
					SetWindowText(wnd, _T("Select settings to import"));
				HWND wnd_tree = GetDlgItem(wnd, IDC_TREE);
				HWND wnd_combo = m_import ? NULL : GetDlgItem(wnd, IDC_DEST);
				SetWindowLongPtr(wnd_tree, GWL_STYLE, GetWindowLongPtr(wnd_tree, GWL_STYLE)|TVS_CHECKBOXES);

				g_set_treeview_window_explorer_theme(wnd_tree);

				if (wnd_combo)
				{
					ComboBox_AddString(wnd_combo, L"Any foobar2000 installation");
					ComboBox_AddString(wnd_combo, L"This foobar2000 installation");
					ComboBox_SetCurSel(wnd_combo, 0);
				}

				SendMessage(wnd_tree, WM_SETREDRAW, FALSE, 0);
				TreeView_SetItemHeight(wnd_tree, TreeView_GetItemHeight(wnd_tree)+2);

				cui::fcl::group_list m_groups;
				if (m_import)
				{
					cui::fcl::dataset_list datasets;
					pfc::list_t<GUID> groupslist;
					t_size j, count = datasets.get_count();
					for (j=0; j<count; j++)
					{
						if (m_filter.have_item(datasets[j]->get_guid()))
						{
							GUID guid = datasets[j]->get_group();
							if (!groupslist.have_item(guid))
								groupslist.add_item(guid);

							cui::fcl::group_ptr ptr;
							while (m_groups.find_by_guid(guid, ptr))
							{
								guid = ptr->get_parent_guid();
								if (guid != pfc::guid_null)
									if (!groupslist.have_item(guid))
										groupslist.add_item(guid);
								else break;

							}
						}
					}
					t_size i = m_groups.get_count();
					for (; i; i--)
						if (!groupslist.have_item(m_groups[i-1]->get_guid()))
							m_groups.remove_by_idx(i-1);
				}
				m_groups.sort_by_name();
				cui::fcl::group_list_filtered filtered(m_groups, pfc::guid_null);
				g_populate_tree(wnd_tree, m_groups, filtered);

				SendMessage(wnd_tree, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(wnd_tree,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
			}
			return TRUE;
		case WM_COMMAND:
			switch (wp)
			{
			case IDOK:
				{
					HWND wnd_tree = GetDlgItem(wnd, IDC_TREE);
					t_size i, count = m_nodes.get_count();
					for (i=0; i<count; i++)
					{
						m_nodes[i].checked = 0 != TreeView_GetCheckState(wnd_tree, m_nodes[i].item);
					}
					HWND wnd_combo = m_import ? NULL : GetDlgItem(wnd, IDC_DEST);
					if (wnd_combo)
					{
						m_mode = ComboBox_GetCurSel(wnd_combo);
					}
				}
				EndDialog(wnd, 1);
				return FALSE;
			case IDCANCEL:
				EndDialog(wnd, 0);
				return FALSE;
			}
			break;
		case WM_CLOSE:
			EndDialog(wnd, 0);
			return 0;
		case WM_DESTROY:
			{
				HWND wnd_tree = GetDlgItem(wnd, IDC_TREE);
				HIMAGELIST il = TreeView_GetImageList(wnd_tree, TVSIL_STATE);
				TreeView_SetImageList(wnd_tree, NULL, TVSIL_STATE);
				ImageList_Destroy(il);
				DestroyWindow(wnd_tree);
			}
			break;
		case WM_NCDESTROY:
			break;
		}

		return FALSE;
	}
	bool have_node_checked (const GUID & pguid)
	{
		t_size i, count = m_nodes.get_count();
		for (i=0; i<count; i++)
		{
			if (m_nodes[i].group->get_guid() == pguid)
				return m_nodes[i].checked;
		}
		return false;
	}
	t_uint32 get_mode() const
	{
		return m_mode;
	}
	FCLDialog(bool b_import= false, const pfc::list_base_const_t<GUID> & p_list = pfc::list_t<GUID>())
		: m_import(b_import), m_mode(0)
	{m_filter.add_items(p_list);};
	private:
		t_uint32 m_mode;
		bool m_import;
		pfc::list_t<GUID> m_filter;
};

cui::fcl::group_impl_factory g_group_toolbars(cui::fcl::groups::toolbars, "Toolbar Layout", "The toolbar layout");
cui::fcl::group_impl_factory g_group_layout(cui::fcl::groups::layout, "Main Layout", "The main layout");
cui::fcl::group_impl_factory g_group_colours(cui::fcl::groups::colours_and_fonts, "Colours and Fonts", "The colours and fonts");
cui::fcl::group_impl_factory g_group_titles(cui::fcl::groups::title_scripts, "Title Scripts", "The titleformatting scripts");

class t_panel_info
{
public:
	GUID guid;
	pfc::string8 name;
};
class panel_info_list : public pfc::list_t<t_panel_info>
{
public:
	bool get_name_by_guid(const GUID & guid, pfc::string8 & p_out)
	{
		t_size i, count = get_count();
		for (i=0; i<count; i++)
			if (get_item(i).guid == guid)
			{
				p_out = get_item(i).name;
				return true;
			}
		return false;
	}
};

class t_import_results_data
{
public:
	panel_info_list m_items;
	bool m_aborted;
	t_import_results_data(const panel_info_list& items, bool baborted) : m_items(items), m_aborted(baborted)
	{};
};

BOOL CALLBACK g_ImportResultsProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			modeless_dialog_manager::g_add(wnd);
			SetWindowText(wnd, _T("FCL import results"));
			HWND wnd_lv = GetDlgItem(wnd, IDC_LIST);
			g_set_listview_window_explorer_theme(wnd_lv);
			t_import_results_data * p_data = reinterpret_cast<t_import_results_data*>(lp);

			SetWindowText(GetDlgItem(wnd, IDC_CAPTION), 
				(p_data->m_aborted ? _T("The layout import was aborted because the following required panels are not installed:") : _T("Some parts of the layout may not have imported because the following panels are not installed:"))
				);

			LVCOLUMN lvc;
			memset(&lvc, 0, sizeof(LVCOLUMN));
			lvc.mask = LVCF_TEXT|LVCF_WIDTH;

			ListView_InsertColumnText(wnd_lv, 0, _T("Name"), 150);
			ListView_InsertColumnText(wnd_lv, 1, _T("GUID"), 300);

			SendMessage(wnd_lv, WM_SETREDRAW, FALSE, 0);

			LVITEM lvi;
			memset(&lvi, 0, sizeof(LVITEM));
			lvi.mask=LVIF_TEXT;
			t_size i, count=p_data->m_items.get_count();
			for (i=0;i<count;i++)
			{
				pfc::string8 temp;
				ListView_InsertItemText(wnd_lv, i, 0, p_data->m_items[i].name, false);
				ListView_InsertItemText(wnd_lv, i, 1, pfc::print_guid(p_data->m_items[i].guid), true);
			}
			SendMessage(wnd_lv, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(wnd_lv,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
		}
		break;
	case WM_COMMAND:
		switch (wp)
		{
		case IDCANCEL:
			DestroyWindow(wnd);
			return 0;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(wnd);
		return 0;
	case WM_NCDESTROY:
		modeless_dialog_manager::g_remove(wnd);
		break;
	}

	return FALSE;
}

PFC_DECLARE_EXCEPTION(exception_fcl_dependentpanelmissing, pfc::exception, "Missing dependent panel(s)");

void g_import_layout(HWND wnd, const char * path)
{
	class t_import_feedback_impl : public cui::fcl::t_import_feedback, public pfc::list_t<GUID>
	{
	public:
		virtual void add_required_panel(const char * name, const GUID & guid)
		{
			add_item(guid);
		}
	};

		//pfc::list_t<t_required_panel> required_panels;
		panel_info_list panel_info;
		try
		{
			class t_dataset
			{
			public:
				GUID guid;
				pfc::array_t<t_uint8> data;
			};

			service_ptr_t<file> p_file;
			abort_callback_impl p_abort;
			filesystem::g_open_read(p_file, path, p_abort);
			GUID guid;
			t_uint32 version;
			p_file->read_lendian_t(guid, p_abort);
			if (guid != g_fcl_header)
				throw pfc::exception("Unrecognised file header");
			p_file->read_lendian_t(version, p_abort);
			if (version > fcl_stream_version)
				throw pfc::exception("Need newer foo_ui_columns");
			t_uint32 mode = cui::fcl::type_public;
			if (version >= 1)
				p_file->read_lendian_t(mode, p_abort);
			{
				pfc::list_t<bool> mask;
				t_size i, count;
				p_file->read_lendian_t(count, p_abort);
				for (i=0; i<count; i++)
				{
					t_panel_info info;
					p_file->read_lendian_t(info.guid, p_abort);
					p_file->read_string(info.name, p_abort);
					panel_info.add_item(info);

					uie::window_ptr ptr;
					mask.add_item(uie::window::create_by_guid(info.guid, ptr));
				}
				panel_info.remove_mask(mask.get_ptr());
			}
			{
				t_size count = panel_info.get_count();
				if (count)
				{
					throw exception_fcl_dependentpanelmissing();
					/*pfc::string8 msg, name;
					msg << "Import aborted: The following required panels are not present.\r\n\r\nGUID, Name\r\n";
					t_size i, count = panel_info.get_count();
					for (i=0; i<count; i++)
					{
						msg << pfc::print_guid(panel_info[i].guid);
						msg << ", " << panel_info[i].name;
						msg << "\r\n";
						//required_panels.add_item(t_required_panel(
					}
					throw pfc::exception(msg);*/
				}
			}
			{
				cui::fcl::dataset_list export_items;
				t_size i, count;
				p_file->read_lendian_t(count, p_abort);
				t_import_feedback_impl feed;
				pfc::array_t< pfc::array_t<t_uint32> > panel_indices;
				panel_indices.set_count(count);
				pfc::array_t< t_dataset > datasets;
				datasets.set_count(count);
				for (i=0; i<count; i++)
				{
					//GUID guiditem;
					pfc::string8 name;
					p_file->read_lendian_t(datasets[i].guid, p_abort);
					p_file->read_string(name, p_abort);
					t_uint32 pcount, j;
					p_file->read_lendian_t(pcount, p_abort);
					panel_indices[i].set_count(pcount);
					for (j=0; j<pcount; j++)
						p_file->read_lendian_t(panel_indices[i][j], p_abort);
					//pfc::array_t<t_uint8> data;
					t_size size;
					p_file->read_lendian_t(size, p_abort);
					datasets[i].data.set_size(size);
					p_file->read(datasets[i].data.get_ptr(), size, p_abort);
				}
				pfc::list_t<GUID> datasetsguids;
				for (i=0; i<count; i++)
					datasetsguids.add_item(datasets[i].guid);
				FCLDialog pFCLDialog(true, datasetsguids);
				if (!uDialogBox (IDD_FCL, wnd, FCLDialog::g_FCLDialogProc, (LPARAM)&pFCLDialog))
					throw exception_aborted();
				ui_helpers::DisableRedrawScope p_NoRedraw(g_main_window);
				for (i=0; i<count; i++)
				{
					cui::fcl::dataset_ptr ptr;
					if (export_items.find_by_guid(datasets[i].guid, ptr) && pFCLDialog.have_node_checked(ptr->get_group()))
						ptr->set_data(&stream_reader_memblock_ref(datasets[i].data.get_ptr(), datasets[i].data.get_size()), datasets[i].data.get_size(),mode,feed, p_abort);
				}
				if (feed.get_count())
				{
					throw pfc::exception("Bug check: panels missing");
					/*pfc::string8 msg, name;
					msg << "The following required panels are not present. Some parts of the layout may not have been imported.\r\n\r\nGUID, Name\r\n";
					t_size i, count = feed.get_count();
					for (i=0; i<count; i++)
					{
						bool b_name = panel_info.get_name_by_guid(feed[i], name);
						msg << pfc::print_guid(feed[i]);
						if (b_name) msg << ", " << name;
						msg << "\r\n";
					}
					throw pfc::exception(msg);*/
				}
			}
		}
		catch (const exception_aborted &)
		{
		}
		catch (const exception_fcl_dependentpanelmissing &)
		{
			t_import_results_data data(panel_info, true);
			ShowWindow(uCreateDialog(IDD_RESULTS, wnd, g_ImportResultsProc, (LPARAM)&data), SW_SHOWNORMAL);
		}
		catch (const pfc::exception & ex)
		{
			popup_message::g_show(ex.what(), "Error");
		};
}

void g_import_layout2(HWND wnd, const char * path)
{
	class t_import_feedback_impl : public cui::fcl::t_import_feedback, public pfc::list_t<GUID>
	{
	public:
		virtual void add_required_panel(const char * name, const GUID & guid)
		{
			add_item(guid);
		}
	};

	//pfc::list_t<t_required_panel> required_panels;
	panel_info_list panel_info;
	try
	{
		class t_dataset
		{
		public:
			GUID guid;
			pfc::array_t<t_uint8> data;
		};

		service_ptr_t<file> p_file;
		abort_callback_impl p_abort;
		filesystem::g_open_read(p_file, path, p_abort);
		GUID guid;
		t_uint32 version;
		p_file->read_lendian_t(guid, p_abort);
		if (guid != g_fcl_header)
			throw pfc::exception("Unrecognised file header");
		p_file->read_lendian_t(version, p_abort);
		if (version > fcl_stream_version)
			throw pfc::exception("Need newer foo_ui_columns");
		t_uint32 mode = cui::fcl::type_public;
		if (version >= 1)
			p_file->read_lendian_t(mode, p_abort);
		{
			pfc::list_t<bool> mask;
			t_size i, count;
			p_file->read_lendian_t(count, p_abort);
			for (i = 0; i<count; i++)
			{
				t_panel_info info;
				p_file->read_lendian_t(info.guid, p_abort);
				p_file->read_string(info.name, p_abort);
				panel_info.add_item(info);

				uie::window_ptr ptr;
				mask.add_item(uie::window::create_by_guid(info.guid, ptr));
			}
			panel_info.remove_mask(mask.get_ptr());
		}
		{
			t_size count = panel_info.get_count();
			if (count)
			{
				throw exception_fcl_dependentpanelmissing();
				/*pfc::string8 msg, name;
				msg << "Import aborted: The following required panels are not present.\r\n\r\nGUID, Name\r\n";
				t_size i, count = panel_info.get_count();
				for (i=0; i<count; i++)
				{
				msg << pfc::print_guid(panel_info[i].guid);
				msg << ", " << panel_info[i].name;
				msg << "\r\n";
				//required_panels.add_item(t_required_panel(
				}
				throw pfc::exception(msg);*/
			}
		}
		{
			cui::fcl::dataset_list export_items;
			t_size i, count;
			p_file->read_lendian_t(count, p_abort);
			t_import_feedback_impl feed;
			pfc::array_t< pfc::array_t<t_uint32> > panel_indices;
			panel_indices.set_count(count);
			pfc::array_t< t_dataset > datasets;
			datasets.set_count(count);
			for (i = 0; i<count; i++)
			{
				//GUID guiditem;
				pfc::string8 name;
				p_file->read_lendian_t(datasets[i].guid, p_abort);
				p_file->read_string(name, p_abort);
				t_uint32 pcount, j;
				p_file->read_lendian_t(pcount, p_abort);
				panel_indices[i].set_count(pcount);
				for (j = 0; j<pcount; j++)
					p_file->read_lendian_t(panel_indices[i][j], p_abort);
				//pfc::array_t<t_uint8> data;
				t_size size;
				p_file->read_lendian_t(size, p_abort);
				datasets[i].data.set_size(size);
				p_file->read(datasets[i].data.get_ptr(), size, p_abort);
			}
			pfc::list_t<GUID> datasetsguids;
			for (i = 0; i<count; i++)
				datasetsguids.add_item(datasets[i].guid);
			//FCLDialog pFCLDialog(true, datasetsguids);
			//if (!uDialogBox (IDD_FCL, wnd, FCLDialog::g_FCLDialogProc, (LPARAM)&pFCLDialog))
			//	throw exception_aborted();
			ui_helpers::DisableRedrawScope p_NoRedraw(g_main_window);
			for (i = 0; i<count; i++)
			{
				cui::fcl::dataset_ptr ptr;
				if (export_items.find_by_guid(datasets[i].guid, ptr))
					ptr->set_data(&stream_reader_memblock_ref(datasets[i].data.get_ptr(), datasets[i].data.get_size()), datasets[i].data.get_size(), mode, feed, p_abort);
			}
			if (feed.get_count())
			{
				throw pfc::exception("Bug check: panels missing");
				/*pfc::string8 msg, name;
				msg << "The following required panels are not present. Some parts of the layout may not have been imported.\r\n\r\nGUID, Name\r\n";
				t_size i, count = feed.get_count();
				for (i=0; i<count; i++)
				{
				bool b_name = panel_info.get_name_by_guid(feed[i], name);
				msg << pfc::print_guid(feed[i]);
				if (b_name) msg << ", " << name;
				msg << "\r\n";
				}
				throw pfc::exception(msg);*/
			}
		}
	}
	catch (const exception_aborted &)
	{
	}
	catch (const exception_fcl_dependentpanelmissing &)
	{
		t_import_results_data data(panel_info, true);
		ShowWindow(uCreateDialog(IDD_RESULTS, wnd, g_ImportResultsProc, (LPARAM)&data), SW_SHOWNORMAL);
	}
	catch (const pfc::exception & ex)
	{
		popup_message::g_show(ex.what(), "Error");
	};
}
void g_import_layout(HWND wnd)
{
	pfc::string8 path;
	if (uGetOpenFileName(wnd, "Columns UI Layout (*.fcl)|*.fcl|All Files (*.*)|*.*", 0, "fcl", "Import from", NULL, path, FALSE))
	{
		g_import_layout(wnd, path);
	}
}

	class t_export_feedback_impl : public cui::fcl::t_export_feedback, public pfc::list_t<GUID>
	{
	public:
		virtual void add_required_panels(const pfc::list_base_const_t<GUID> & panels)
		{
			add_items(panels);
		}
		t_size find_or_add_guid(const GUID & guid)
		{
			t_size index = find_item(guid);
			if (index == pfc_infinite)
				index = add_item(guid);
			return index;
		}
	};

void g_export_layout(HWND wnd)
{
	pfc::string8 path;
	FCLDialog pFCLDialog;
	if (/*MessageBox(wnd, _T("Layout setting exporting is available for testing purposes only. FCL files produced will not work with the release version of Columns UI."), _T("Warning"), MB_OK) == IDOK && */uDialogBox (IDD_FCL1, wnd, FCLDialog::g_FCLDialogProc, (LPARAM)&pFCLDialog) && uGetOpenFileName(wnd, "Columns UI Layout (*.fcl)|*.fcl|All Files (*.*)|*.*", 0, "fcl", "Save as", NULL, path, TRUE))
	{
		t_export_feedback_impl feedback;
		pfc::list_t<GUID> groups;
		{
			t_size i, count = pFCLDialog.m_nodes.get_count();
			for (i=0; i<count; i++)
				if (pFCLDialog.m_nodes[i].checked)
					groups.add_item(pFCLDialog.m_nodes[i].group->get_guid());
		}
		try
		{
			service_ptr_t<file> p_file;
			abort_callback_impl p_abort;
			filesystem::g_open_write_new(p_file, path, p_abort);
			p_file->write_lendian_t(g_fcl_header, p_abort);
			p_file->write_lendian_t((t_uint32)fcl_stream_version, p_abort);
			p_file->write_lendian_t((t_uint32)pFCLDialog.get_mode(), p_abort);

			stream_writer_memblock mem;
			t_size actualtotal=0;
			{
				cui::fcl::dataset_list export_items;
				t_size i, count = export_items.get_count();
				pfc::array_t< t_export_feedback_impl > feeds;
				feeds.set_count(count);
				for (i=0; i<count; i++)
				{
					if (groups.have_item(export_items[i]->get_group()))
					{
						pfc::string8 name;
						export_items[i]->get_name(name);
						mem.write_lendian_t(export_items[i]->get_guid(), p_abort);
						mem.write_string(name, p_abort);
						stream_writer_memblock writer;
						export_items[i]->get_data(&writer, pFCLDialog.get_mode(), feeds[i], p_abort);
						t_size j, pcount = feeds[i].get_count();
						mem.write_lendian_t(pcount, p_abort);
						for (j=0; j<pcount; j++)
						{
							t_uint32 temp = feedback.find_or_add_guid(feeds[i][j]);
							mem.write_lendian_t(temp, p_abort);
						}						
						mem.write_lendian_t((t_uint32)writer.m_data.get_size(), p_abort);
						mem.write(writer.m_data.get_ptr(), writer.m_data.get_size(), p_abort);
						actualtotal++;
					}
				}
			}

			{
				t_size j, pcount = feedback.get_count();
				p_file->write_lendian_t(pcount, p_abort);
				for (j=0; j<pcount; j++)
				{
					uie::window_ptr ptr;
					pfc::string8 name;
					if (uie::window::create_by_guid(feedback[j], ptr))
						ptr->get_name(name);
					p_file->write_lendian_t(feedback[j], p_abort);
					p_file->write_string(name, p_abort);
				}
				/*pfc::list_t<uie::window_ptr> windows;
				uie::window_ptr ptr;
				service_enum_t<uie::window> window_enum;
				while (window_enum.next(ptr))
				{
					windows.add_item(ptr);
				}
				t_size i, count = windows.get_count();
				p_file->write_lendian_t(count, p_abort);
				for (i=0; i<count; i++)
				{
					pfc::string8 temp;
					p_file->write_lendian_t(windows[i]->get_extension_guid(), p_abort);
					windows[i]->get_name(temp);
					p_file->write_string(temp, p_abort);
				}*/
			}

			p_file->write_lendian_t(actualtotal, p_abort);
			p_file->write(mem.m_data.get_ptr(), mem.m_data.get_size(), p_abort);
		}
		catch (const pfc::exception & ex)
		{
			abort_callback_impl p_abort;
			try {if (filesystem::g_exists(path, p_abort)) filesystem::g_remove(path, p_abort);} catch (const pfc::exception &) {};
			popup_message::g_show(ex.what(), "Error");
		};
	}
}

