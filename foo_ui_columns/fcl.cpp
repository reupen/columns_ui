#include "stdafx.h"
#include "fcl.h"
#include "main_window.h"

// {EBD87879-65A7-4242-821B-812AF9F68E8F}
const GUID cui::fcl::groups::titles_playlist_view
    = {0xebd87879, 0x65a7, 0x4242, {0x82, 0x1b, 0x81, 0x2a, 0xf9, 0xf6, 0x8e, 0x8f}};

// {F17DDDF4-BB3E-4f36-B9E1-D626629F2C76}
const GUID cui::fcl::groups::titles_common
    = {0xf17dddf4, 0xbb3e, 0x4f36, {0xb9, 0xe1, 0xd6, 0x26, 0x62, 0x9f, 0x2c, 0x76}};

enum { fcl_stream_version = 2 };

// {9FAADFF3-E51A-4a8b-B4A3-D209A36AB301}
static const GUID g_fcl_header = {0x9faadff3, 0xe51a, 0x4a8b, {0xb4, 0xa3, 0xd2, 0x9, 0xa3, 0x6a, 0xb3, 0x1}};

namespace treeview {
static HTREEITEM insert_item(
    HWND wnd_tree, const char* sz_text, LPARAM data, HTREEITEM ti_parent = TVI_ROOT, HTREEITEM ti_after = TVI_LAST)
{
    uTVINSERTSTRUCT is;
    memset(&is, 0, sizeof(is));
    is.hParent = ti_parent;
    is.hInsertAfter = ti_after;
    is.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
    is.item.pszText = const_cast<char*>(sz_text);
    is.item.state = TVIS_EXPANDED;
    is.item.stateMask = TVIS_EXPANDED;
    is.item.lParam = data;
    return uTreeView_InsertItem(wnd_tree, &is);
}
} // namespace treeview

class FCLDialog {
public:
    class Node {
    public:
        HTREEITEM item{nullptr};
        cui::fcl::group_ptr group;
        bool checked{true};
        Node(HTREEITEM pitem, cui::fcl::group_ptr ptr) : item(pitem), group(std::move(ptr)){};
        Node() = default;
    };
    // cui::fcl::group_list m_groups;
    std::vector<Node> m_nodes;
    void g_populate_tree(HWND wnd_tree, cui::fcl::group_list& list, const cui::fcl::group_list_filtered& filtered,
        HTREEITEM ti_parent = TVI_ROOT)
    {
        t_size count = filtered.get_count();
        for (t_size i = 0; i < count; i++) {
            pfc::string8 name;
            filtered[i]->get_name(name);
            HTREEITEM item = treeview::insert_item(wnd_tree, name, m_nodes.size(), ti_parent);
            m_nodes.emplace_back(Node(item, filtered[i]));
            TreeView_SetCheckState(wnd_tree, item, TRUE);
            cui::fcl::group_list_filtered filtered2(list, filtered[i]->get_guid());
            list.remove_by_guid(filtered[i]->get_guid());
            g_populate_tree(wnd_tree, list, filtered2, item);
        }
    }
    static BOOL CALLBACK g_FCLDialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        FCLDialog* p_this = nullptr;
        switch (msg) {
        case WM_INITDIALOG:
            SetWindowLongPtr(wnd, DWLP_USER, lp);
            p_this = reinterpret_cast<FCLDialog*>(lp);
            break;
        default:
            p_this = reinterpret_cast<FCLDialog*>(GetWindowLongPtr(wnd, DWLP_USER));
            break;
        }
        if (p_this)
            return p_this->FCLDialogProc(wnd, msg, wp, lp);

        return FALSE;
    }
    BOOL CALLBACK FCLDialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            if (m_import)
                SetWindowText(wnd, _T("Select settings to import"));
            HWND wnd_tree = GetDlgItem(wnd, IDC_TREE);
            HWND wnd_combo = m_import ? nullptr : GetDlgItem(wnd, IDC_DEST);
            SetWindowLongPtr(wnd_tree, GWL_STYLE, GetWindowLongPtr(wnd_tree, GWL_STYLE) | TVS_CHECKBOXES);

            uih::tree_view_set_explorer_theme(wnd_tree);

            if (wnd_combo) {
                ComboBox_AddString(wnd_combo, L"Any foobar2000 installation");
                ComboBox_AddString(wnd_combo, L"This foobar2000 installation");
                ComboBox_SetCurSel(wnd_combo, 0);
            }

            SendMessage(wnd_tree, WM_SETREDRAW, FALSE, 0);
            TreeView_SetItemHeight(wnd_tree, TreeView_GetItemHeight(wnd_tree) + 2);

            cui::fcl::group_list m_groups;
            if (m_import) {
                cui::fcl::dataset_list datasets;
                std::unordered_set<GUID> groupslist;
                t_size count = datasets.get_count();
                for (t_size j = 0; j < count; j++) {
                    if (m_filter.count(datasets[j]->get_guid()) > 0) {
                        GUID guid = datasets[j]->get_group();
                        groupslist.emplace(guid);

                        cui::fcl::group_ptr ptr;
                        while (m_groups.find_by_guid(guid, ptr)) {
                            guid = ptr->get_parent_guid();
                            if (guid == GUID{} || groupslist.count(guid) > 0)
                                break;
                            groupslist.emplace(guid);
                        }
                    }
                }
                t_size i = m_groups.get_count();
                for (; i; i--)
                    if (groupslist.count(m_groups[i - 1]->get_guid()) == 0)
                        m_groups.remove_by_idx(i - 1);
            }
            m_groups.sort_by_name();
            cui::fcl::group_list_filtered filtered(m_groups, pfc::guid_null);
            g_populate_tree(wnd_tree, m_groups, filtered);

            SendMessage(wnd_tree, WM_SETREDRAW, TRUE, 0);
            RedrawWindow(wnd_tree, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
        }
            return TRUE;
        case WM_COMMAND:
            switch (wp) {
            case IDOK: {
                HWND wnd_tree = GetDlgItem(wnd, IDC_TREE);
                t_size count = m_nodes.size();
                for (t_size i = 0; i < count; i++) {
                    m_nodes[i].checked = 0 != TreeView_GetCheckState(wnd_tree, m_nodes[i].item);
                }
                HWND wnd_combo = m_import ? nullptr : GetDlgItem(wnd, IDC_DEST);
                if (wnd_combo) {
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
        case WM_DESTROY: {
            HWND wnd_tree = GetDlgItem(wnd, IDC_TREE);
            HIMAGELIST il = TreeView_GetImageList(wnd_tree, TVSIL_STATE);
            TreeView_SetImageList(wnd_tree, NULL, TVSIL_STATE);
            ImageList_Destroy(il);
            DestroyWindow(wnd_tree);
        } break;
        case WM_NCDESTROY:
            break;
        }

        return FALSE;
    }
    bool have_node_checked(const GUID& pguid)
    {
        t_size count = m_nodes.size();
        for (t_size i = 0; i < count; i++) {
            if (m_nodes[i].group->get_guid() == pguid)
                return m_nodes[i].checked;
        }
        return false;
    }
    t_uint32 get_mode() const { return m_mode; }
    FCLDialog(bool b_import = false, std::unordered_set<GUID> p_list = {})
        : m_import(b_import), m_filter(std::move(p_list))
    {
    }

private:
    t_uint32 m_mode{0};
    bool m_import;
    std::unordered_set<GUID> m_filter;
};

cui::fcl::group_impl_factory g_group_toolbars(cui::fcl::groups::toolbars, "Toolbar Layout", "The toolbar layout");
cui::fcl::group_impl_factory g_group_layout(cui::fcl::groups::layout, "Main Layout", "The main layout");
cui::fcl::group_impl_factory g_group_colours(
    cui::fcl::groups::colours_and_fonts, "Colours and Fonts", "The colours and fonts");
cui::fcl::group_impl_factory g_group_titles(
    cui::fcl::groups::title_scripts, "Title Scripts", "The titleformatting scripts");

class PanelInfo {
public:
    GUID guid{};
    pfc::string8 name;
};
class PanelInfoList : public pfc::list_t<PanelInfo> {
public:
    bool get_name_by_guid(const GUID& guid, pfc::string8& p_out)
    {
        t_size count = get_count();
        for (t_size i = 0; i < count; i++)
            if (get_item(i).guid == guid) {
                p_out = get_item(i).name;
                return true;
            }
        return false;
    }
};

class ImportResultsData {
public:
    PanelInfoList m_items;
    bool m_aborted;
    ImportResultsData(PanelInfoList items, bool baborted) : m_items(std::move(items)), m_aborted(baborted){};
};

BOOL CALLBACK g_ImportResultsProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        modeless_dialog_manager::g_add(wnd);
        SetWindowText(wnd, _T("FCL import results"));
        HWND wnd_lv = GetDlgItem(wnd, IDC_LIST);
        uih::list_view_set_explorer_theme(wnd_lv);
        auto* p_data = reinterpret_cast<ImportResultsData*>(lp);

        SetWindowText(GetDlgItem(wnd, IDC_CAPTION),
            (p_data->m_aborted
                    ? _T("The layout import was aborted because the following required panels are not installed:")
                    : _T("Some parts of the layout may not have imported because the following panels are not ")
                      _T("installed:")));

        LVCOLUMN lvc;
        memset(&lvc, 0, sizeof(LVCOLUMN));
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        uih::list_view_insert_column_text(wnd_lv, 0, _T("Name"), 150);
        uih::list_view_insert_column_text(wnd_lv, 1, _T("GUID"), 300);

        SendMessage(wnd_lv, WM_SETREDRAW, FALSE, 0);

        LVITEM lvi;
        memset(&lvi, 0, sizeof(LVITEM));
        lvi.mask = LVIF_TEXT;
        t_size count = p_data->m_items.get_count();
        for (t_size i = 0; i < count; i++) {
            pfc::string8 temp;
            uih::list_view_insert_item_text(wnd_lv, i, 0, p_data->m_items[i].name, false);
            uih::list_view_insert_item_text(wnd_lv, i, 1, pfc::print_guid(p_data->m_items[i].guid), true);
        }
        SendMessage(wnd_lv, WM_SETREDRAW, TRUE, 0);
        RedrawWindow(wnd_lv, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
    } break;
    case WM_COMMAND:
        switch (wp) {
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

void g_import_layout(HWND wnd, const char* path, bool quiet)
{
    class ImportFeedbackReceiver
        : public cui::fcl::t_import_feedback
        , public pfc::list_t<GUID> {
    public:
        void add_required_panel(const char* name, const GUID& guid) override { add_item(guid); }
    };

    // pfc::list_t<t_required_panel> required_panels;
    PanelInfoList panel_info;
    try {
        class RawDataSet {
        public:
            GUID guid{};
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
            throw pfc::exception("Need a newer version of Columns UI");
        t_uint32 mode = cui::fcl::type_public;
        if (version >= 1)
            p_file->read_lendian_t(mode, p_abort);
        {
            pfc::list_t<bool> mask;
            t_size count;
            p_file->read_lendian_t(count, p_abort);
            for (t_size i = 0; i < count; i++) {
                PanelInfo info;
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
            if (count) {
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
            t_size count;
            p_file->read_lendian_t(count, p_abort);

            pfc::array_t<pfc::array_t<t_uint32>> panel_indices;
            panel_indices.set_count(count);

            std::vector<RawDataSet> datasets;
            datasets.resize(count);

            for (auto i : ranges::views::iota(size_t{0}, count)) {
                // GUID guiditem;
                pfc::string8 name;
                p_file->read_lendian_t(datasets[i].guid, p_abort);
                p_file->read_string(name, p_abort);
                t_uint32 pcount;
                p_file->read_lendian_t(pcount, p_abort);
                panel_indices[i].set_count(pcount);
                for (t_uint32 j = 0; j < pcount; j++)
                    p_file->read_lendian_t(panel_indices[i][j], p_abort);
                // pfc::array_t<t_uint8> data;
                t_size size;
                p_file->read_lendian_t(size, p_abort);
                datasets[i].data.set_size(size);
                p_file->read(datasets[i].data.get_ptr(), size, p_abort);
            }

            std::unordered_set<GUID> datasetsguids;
            for (auto&& data_set : datasets)
                datasetsguids.emplace(data_set.guid);

            FCLDialog pFCLDialog(true, std::move(datasetsguids));
            if (!quiet) {
                const auto dialog_result = DialogBoxParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_FCL_IMPORT),
                    wnd, FCLDialog::g_FCLDialogProc, reinterpret_cast<LPARAM>(&pFCLDialog));

                if (dialog_result <= 0)
                    throw exception_aborted();
            }

            cui::fcl::dataset_list export_items;
            ImportFeedbackReceiver feed;

            uih::DisableRedrawScope p_NoRedraw(cui::main_window.get_wnd());

            for (auto export_item_index : ranges::views::iota(size_t{0}, export_items.get_count())) {
                auto ptr = export_items[export_item_index];
                auto data_set_iter
                    = ranges::find_if(datasets, [&ptr](auto&& data_set) { return data_set.guid == ptr->get_guid(); });

                if (data_set_iter != datasets.end() && (quiet || pFCLDialog.have_node_checked(ptr->get_group()))) {
                    ptr->set_data_from_ptr(
                        data_set_iter->data.get_ptr(), data_set_iter->data.get_size(), mode, feed, p_abort);
                }
            }
            if (feed.get_count()) {
                throw pfc::exception("Bug check: panels missing");
            }
        }
    } catch (const exception_aborted&) {
    } catch (const exception_fcl_dependentpanelmissing&) {
        ImportResultsData data(panel_info, true);
        const auto wnd_results = CreateDialogParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_RESULTS), wnd,
            g_ImportResultsProc, reinterpret_cast<LPARAM>(&data));
        ShowWindow(wnd_results, SW_SHOWNORMAL);
    } catch (const pfc::exception& ex) {
        popup_message::g_show(ex.what(), "Error");
    }
}

void g_import_layout(HWND wnd)
{
    pfc::string8 path;
    if (uGetOpenFileName(wnd, "Columns UI Layout (*.fcl)|*.fcl|All Files (*.*)|*.*", 0, "fcl", "Import from", nullptr,
            path, FALSE)) {
        g_import_layout(wnd, path, false);
    }
}

class ExportFeedbackReceiver
    : public cui::fcl::t_export_feedback
    , public pfc::list_t<GUID> {
public:
    void add_required_panels(const pfc::list_base_const_t<GUID>& panels) override { add_items(panels); }
    t_size find_or_add_guid(const GUID& guid)
    {
        t_size index = find_item(guid);
        if (index == pfc_infinite)
            index = add_item(guid);
        return index;
    }
};

void g_export_layout(HWND wnd, pfc::string8 path, bool is_quiet)
{
    FCLDialog pFCLDialog;
    if (!is_quiet) {
        const auto dialog_result = DialogBoxParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_FCL_EXPORT), wnd,
            FCLDialog::g_FCLDialogProc, reinterpret_cast<LPARAM>(&pFCLDialog));

        if (dialog_result <= 0)
            return;
    }

    if (path.is_empty()
        && !uGetOpenFileName(
               wnd, "Columns UI Layout (*.fcl)|*.fcl|All Files (*.*)|*.*", 0, "fcl", "Save as", nullptr, path, TRUE))
        return;

    if (path.is_empty())
        throw pfc::exception_bug_check();

    ExportFeedbackReceiver feedback;
    pfc::list_t<GUID> groups;

    if (!is_quiet) {
        const t_size count = pFCLDialog.m_nodes.size();
        for (t_size i = 0; i < count; i++)
            if (pFCLDialog.m_nodes[i].checked)
                groups.add_item(pFCLDialog.m_nodes[i].group->get_guid());
    }

    try {
        service_ptr_t<file> p_file;
        abort_callback_impl p_abort;
        filesystem::g_open_write_new(p_file, path, p_abort);
        p_file->write_lendian_t(g_fcl_header, p_abort);
        p_file->write_lendian_t((t_uint32)fcl_stream_version, p_abort);

        const uint32_t mode = is_quiet ? cui::fcl::type_private : pFCLDialog.get_mode();
        p_file->write_lendian_t(mode, p_abort);

        stream_writer_memblock mem;
        t_size actualtotal = 0;
        {
            cui::fcl::dataset_list export_items;
            t_size count = export_items.get_count();
            pfc::array_t<ExportFeedbackReceiver> feeds;
            feeds.set_count(count);
            for (t_size i = 0; i < count; i++) {
                if (is_quiet || groups.have_item(export_items[i]->get_group())) {
                    pfc::string8 name;
                    export_items[i]->get_name(name);
                    mem.write_lendian_t(export_items[i]->get_guid(), p_abort);
                    mem.write_string(name, p_abort);
                    stream_writer_memblock writer;
                    export_items[i]->get_data(&writer, mode, feeds[i], p_abort);
                    t_size pcount = feeds[i].get_count();
                    mem.write_lendian_t(pcount, p_abort);
                    for (t_size j = 0; j < pcount; j++) {
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
            t_size pcount = feedback.get_count();
            p_file->write_lendian_t(pcount, p_abort);
            for (t_size j = 0; j < pcount; j++) {
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
    } catch (const pfc::exception& ex) {
        abort_callback_impl p_abort;
        try {
            if (filesystem::g_exists(path, p_abort))
                filesystem::g_remove(path, p_abort);
        } catch (const pfc::exception&) {
        }
        popup_message::g_show(ex.what(), "Error");
    }
}
