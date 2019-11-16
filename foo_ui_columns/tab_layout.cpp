#include "stdafx.h"

#include "tab_layout.h"
#include "layout.h"
#include "config.h"
#include "rename_dialog.h"
#include "splitter_utils.h"

namespace cui::prefs {

template <class Destination, class Source>
void merge_maps(Destination& target, Source& source)
{
    source.merge(target);
    assert(target.empty());
    target.swap(source);
}

bool LayoutTabNode::have_item(const GUID& p_guid)
{
    if (m_item->get_ptr()->get_panel_guid() == p_guid)
        return true;
    unsigned n;
    unsigned count;
    for (n = 0, count = m_children.get_count(); n < count; n++) {
        if (m_children[n]->have_item(p_guid))
            return true;
    }
    return false;
}

HTREEITEM LayoutTab::insert_item_in_tree_view(
    HWND wnd_tree, const char* sz_text, HTREEITEM ti_parent, HTREEITEM ti_after, bool is_expanded)
{
    uTVINSERTSTRUCT is;
    memset(&is, 0, sizeof(is));
    is.hParent = ti_parent;
    is.hInsertAfter = ti_after;
    is.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
    is.item.pszText = const_cast<char*>(sz_text);
    is.item.state = is_expanded ? TVIS_EXPANDED : 0;
    is.item.stateMask = TVIS_EXPANDED;
    return uTreeView_InsertItem(wnd_tree, &is);
}

void LayoutTab::get_panel_list(uie::window_info_list_simple& p_out)
{
    service_enum_t<ui_extension::window> e;
    uie::window_ptr l;

    if (e.first(l))
        do {
            if (true) {
                uie::window_info_simple info;

                l->get_name(info.name);
                l->get_category(info.category);
                info.guid = l->get_extension_guid();
                info.prefer_multiple_instances = l->get_is_single_instance();
                info.type = l->get_type();

                // FIXME
                // if (!info.prefer_multiple_instances || !p_obj_config->have_extension(info.guid))
                p_out.add_item(info);

                l.release();
            }
        } while (e.next(l));

    p_out.sort_by_category_and_name();
}

HTREEITEM LayoutTab::tree_view_get_child_by_index(HWND wnd_tv, HTREEITEM ti, unsigned index)
{
    HTREEITEM item = TreeView_GetChild(wnd_tv, ti);
    for (unsigned n = 0; n < index && item; n++)
        item = TreeView_GetNextSibling(wnd_tv, item);
    return item;
}

unsigned LayoutTab::tree_view_get_child_index(HWND wnd_tv, HTREEITEM ti)
{
    HTREEITEM item = ti;
    unsigned n = 0;
    while ((item = TreeView_GetPrevSibling(wnd_tv, item)))
        n++;
    return n;
}

void LayoutTab::populate_tree(HWND wnd, LayoutTabNode::ptr p_node, HTREEITEM ti_parent, HTREEITEM ti_after)
{
    HWND wnd_tree = GetDlgItem(wnd, IDC_TREE);
    SendMessage(wnd_tree, WM_SETREDRAW, FALSE, NULL);
    auto node_map = __populate_tree(wnd_tree, p_node, ti_parent, ti_after);
    SendMessage(wnd_tree, WM_SETREDRAW, TRUE, NULL);
    merge_maps(m_node_map, node_map);
}

void LayoutTab::repopulate_node(HWND wnd, LayoutTabNode::ptr node, HTREEITEM ti_parent, HTREEITEM ti_after)
{
    HWND wnd_tree = GetDlgItem(wnd, IDC_TREE);
    SendMessage(wnd_tree, WM_SETREDRAW, FALSE, NULL);
    auto node_map = __repopulate_node(wnd_tree, node, ti_parent, ti_after);
    SendMessage(wnd_tree, WM_SETREDRAW, TRUE, NULL);
    merge_maps(m_node_map, node_map);
}

std::unordered_map<HTREEITEM, LayoutTabNode::ptr> LayoutTab::__populate_tree(
    HWND wnd_tree, LayoutTabNode::ptr p_node, HTREEITEM ti_parent, HTREEITEM ti_after)
{
    std::unordered_map<HTREEITEM, LayoutTabNode::ptr> node_map;

    uie::window_ptr p_wnd;
    pfc::string8 sz_text;
    if (uie::window::create_by_guid(p_node->m_item->get_ptr()->get_panel_guid(), p_wnd)) {
        stream_writer_memblock conf;
        p_node->m_item->get_ptr()->get_panel_config(&conf);
        p_wnd->get_name(sz_text);
        try {
            abort_callback_dummy abortCallback;
            p_wnd->set_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size(), abortCallback);
        } catch (const pfc::exception& ex) {
            console::formatter formatter;
            formatter << "warning: " << sz_text << ": function uie::window::set_config; error " << ex.what();
        }
    } else
        sz_text = "<unknown>";

    HTREEITEM ti_item = nullptr;

    if ((ti_item = insert_item_in_tree_view(wnd_tree, sz_text, ti_parent, ti_after, p_node->m_expanded))) {
        node_map[ti_item] = p_node;
        p_node->m_window = p_wnd;

        service_ptr_t<uie::splitter_window> p_splitter;
        if (p_wnd.is_valid() && p_wnd->service_query_t(p_splitter)) {
            p_node->m_splitter = p_splitter;
            unsigned count = p_splitter->get_panel_count();
            for (unsigned n = 0; n < count; n++) {
                // pfc::rcptr_t<uie::splitter_item_ptr> p_child = pfc::rcnew_t<uie::splitter_item_ptr>();
                auto p_child_node = std::make_shared<LayoutTabNode>();
                p_node->m_children.insert_item(p_child_node, n);

                // uie::splitter_item_ptr child;
                p_splitter->get_panel(n, *p_child_node->m_item);
                auto child_node_map = __populate_tree(wnd_tree, p_child_node, ti_item);
                merge_maps(node_map, child_node_map);
            }
        }
    }

    return node_map;
}

std::unordered_map<HTREEITEM, LayoutTabNode::ptr> LayoutTab::__repopulate_node(
    HWND wnd_tree, LayoutTabNode::ptr node, HTREEITEM ti_parent, HTREEITEM ti_after)
{
    std::unordered_map<HTREEITEM, LayoutTabNode::ptr> node_map;
    const auto window = node->m_window;
    pfc::string8 sz_text;

    if (window.is_valid())
        window->get_name(sz_text);
    else
        sz_text = "<unknown>";

    const auto ti_item = insert_item_in_tree_view(wnd_tree, sz_text, ti_parent, ti_after, node->m_expanded);

    if (ti_item) {
        node_map[ti_item] = node;

        for (auto&& child : node->m_children) {
            auto child_node_map = __populate_tree(wnd_tree, child, ti_item);
            merge_maps(node_map, child_node_map);
        }
    }

    return node_map;
}

void LayoutTab::remove_node(HWND wnd, HTREEITEM ti)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    HTREEITEM ti_parent = TreeView_GetParent(wnd_tv, ti);
    if (!ti_parent)
        return;

    auto p_parent_node = m_node_map.at(ti_parent);
    unsigned index = tree_view_get_child_index(wnd_tv, ti);
    if (index < p_parent_node->m_children.get_count()) {
        p_parent_node->m_children.remove_by_idx(index);
        p_parent_node->m_splitter->remove_panel(index);
        TreeView_DeleteItem(wnd_tv, ti);
        save_item(wnd, ti_parent);
    }
}

void LayoutTab::insert_item(HWND wnd, HTREEITEM ti_parent, const GUID& p_guid, HTREEITEM ti_after)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);

    // uie::splitter_item_simple_t p_item;
    // p_item.set_panel_guid(p_guid);
    auto p_node = std::make_shared<LayoutTabNode>();
    *p_node->m_item = new uie::splitter_item_simple_t;
    p_node->m_item->get_ptr()->set_panel_guid(p_guid);
    auto p_parent = m_node_map.at(ti_parent);
    service_ptr_t<uie::splitter_window> p_splitter;
    if (p_parent->m_window.is_valid() && p_parent->m_window->service_query_t(p_splitter)) {
        unsigned index
            = ti_after != TVI_LAST ? tree_view_get_child_index(wnd_tv, ti_after) + 1 : p_parent->m_children.get_count();
        if (index <= p_parent->m_children.get_count()) {
            p_splitter->insert_panel(index, p_node->m_item->get_ptr());
            p_parent->m_children.insert_item(p_node, index);
            populate_tree(wnd, p_node, ti_parent, ti_after);
            save_item(wnd, ti_parent);
        }
    }
}

void LayoutTab::copy_item(HWND wnd, HTREEITEM ti)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    auto p_node = m_node_map.at(ti);
    splitter_utils::copy_splitter_item_to_clipboard_safe(wnd, p_node->m_item->get_ptr());
}

bool LayoutTab::_fix_single_instance_recur(uie::splitter_window_ptr& p_window)
{
    if (!p_window.is_valid())
        return false;

    bool modified = false;
    t_size i;
    t_size count = p_window->get_panel_count();
    pfc::array_staticsize_t<bool> mask(count);

    for (i = 0; i < count; i++) {
        uie::window_ptr p_child_window;
        uie::splitter_item_ptr p_si;
        p_window->get_panel(i, p_si);
        if (!uie::window::create_by_guid(p_si->get_panel_guid(), p_child_window))
            mask[i] = true;
        else
            mask[i] = p_child_window->get_is_single_instance() && m_node_root->have_item(p_si->get_panel_guid());
    }

    for (i = count; i > 0; i--)
        if (mask[i - 1]) {
            p_window->remove_panel(i - 1);
            modified = true;
        }

    count = p_window->get_panel_count();

    for (i = 0; i < count; i++) {
        uie::window_ptr p_child_window;
        uie::splitter_window_ptr p_child_sw;

        uie::splitter_item_ptr p_si;
        p_window->get_panel(i, p_si);
        if (uie::window::create_by_guid(p_si->get_panel_guid(), p_child_window)) {
            if (p_child_window->service_query_t(p_child_sw)) {
                stream_writer_memblock sw;
                abort_callback_dummy abortCallback;
                p_si->get_panel_config(&sw);
                p_child_window->set_config_from_ptr(sw.m_data.get_ptr(), sw.m_data.get_size(), abortCallback);
                if (_fix_single_instance_recur(p_child_sw)) {
                    sw.m_data.set_size(0);
                    p_child_window->get_config(&sw, abortCallback);
                    p_si->set_panel_config_from_ptr(sw.m_data.get_ptr(), sw.m_data.get_size());
                    p_window->replace_panel(i, p_si.get_ptr());
                    modified = true;
                }
            }
        }
    }
    return modified;
}

bool LayoutTab::fix_paste_item(uie::splitter_item_full_v3_impl_t& item)
{
    uie::window::ptr p_window;
    if (!uie::window::create_by_guid(item.get_panel_guid(), p_window))
        return false;

    if (p_window->get_is_single_instance() && m_node_root->have_item(item.get_panel_guid()))
        return false;

    uie::splitter_window_ptr p_sw;
    if (p_window->service_query_t(p_sw)) {
        stream_writer_memblock sw;
        abort_callback_dummy aborter;
        item.get_panel_config(&sw);

        p_window->set_config_from_ptr(sw.m_data.get_ptr(), sw.m_data.get_size(), aborter);
        if (_fix_single_instance_recur(p_sw)) {
            sw.m_data.set_size(0);
            p_window->get_config(&sw, aborter);
            item.set_panel_config_from_ptr(sw.m_data.get_ptr(), sw.m_data.get_size());
        }
    }
    return true;
}

void LayoutTab::paste_item(HWND wnd, HTREEITEM ti_parent, HTREEITEM ti_after)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    auto p_node = std::make_shared<LayoutTabNode>();
    auto splitter_item = splitter_utils::get_splitter_item_from_clipboard_safe(wnd);

    if (!splitter_item || !fix_paste_item(*splitter_item))
        return;

    *p_node->m_item = splitter_item.release();

    auto p_parent = m_node_map.at(ti_parent);
    service_ptr_t<uie::splitter_window> p_splitter;
    if (p_parent->m_window.is_valid() && p_parent->m_window->service_query_t(p_splitter)) {
        unsigned index{};

        if (ti_after == TVI_LAST)
            index = p_parent->m_children.get_count();
        else
            index = tree_view_get_child_index(wnd_tv, ti_after) + 1;

        if (index <= p_parent->m_children.get_count()) {
            p_splitter->insert_panel(index, p_node->m_item->get_ptr());
            p_parent->m_children.insert_item(p_node, index);
            populate_tree(wnd, p_node, ti_parent, ti_after);
            save_item(wnd, ti_parent);
        }
    }
}

void LayoutTab::move_item(HWND wnd, HTREEITEM ti, bool up)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    HTREEITEM ti_parent = TreeView_GetParent(wnd_tv, ti);

    auto p_parent_node = m_node_map.at(ti_parent);
    unsigned index = tree_view_get_child_index(wnd_tv, ti);
    if (up) {
        if (index > 0) {
            p_parent_node->m_splitter->move_up(index);
            p_parent_node->m_children.swap_items(index, index - 1);
            HTREEITEM ti_prev = TreeView_GetPrevSibling(wnd_tv, ti);
            TreeView_DeleteItem(wnd_tv, ti_prev);

            const auto child_node = p_parent_node->m_children[index];
            repopulate_node(wnd, child_node, ti_parent, ti);

            save_item(wnd, ti_parent);
        }
    }
    if (!up) {
        if (index + 1 < p_parent_node->m_children.get_count()) {
            p_parent_node->m_splitter->move_down(index);
            p_parent_node->m_children.swap_items(index, index + 1);
            HTREEITEM ti_next = TreeView_GetNextSibling(wnd_tv, ti);
            HTREEITEM ti_prev = TreeView_GetPrevSibling(wnd_tv, ti);
            if (!ti_prev)
                ti_prev = TVI_FIRST;
            TreeView_DeleteItem(wnd_tv, ti_next);

            const auto child_node = p_parent_node->m_children[index];
            repopulate_node(wnd, child_node, ti_parent, ti_prev);

            save_item(wnd, ti_parent);
        }
    }
}

void LayoutTab::print_index_out_of_range()
{
    console::print("layout editor: internal error: index out of range");
}

void LayoutTab::switch_splitter(HWND wnd, HTREEITEM ti, const GUID& p_guid)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    HTREEITEM ti_parent = TreeView_GetParent(wnd_tv, ti);

    auto p_node = m_node_map.at(ti);
    LayoutTabNode::ptr p_parent_node;
    if (ti_parent)
        p_parent_node = m_node_map.at(ti_parent);

    uie::window_ptr window;
    service_ptr_t<uie::splitter_window> splitter;
    if (uie::window::create_by_guid(p_guid, window) && window->service_query_t(splitter)) {
        unsigned count = min(p_node->m_children.get_count(), splitter->get_maximum_panel_count());
        if (count == p_node->m_children.get_count()
            || MessageBox(wnd, _T("The number of child items will not fit in the selected splitter type. Continue?"),
                   _T("Warning"), MB_YESNO | MB_ICONEXCLAMATION)
                == IDYES) {
            for (unsigned n = 0; n < count; n++)
                splitter->add_panel(p_node->m_children[n]->m_item->get_ptr());
            stream_writer_memblock conf;
            try {
                abort_callback_dummy abort_callback;
                splitter->get_config(&conf, abort_callback);
            } catch (const pfc::exception&) {
            }
            p_node->m_item->get_ptr()->set_panel_guid(p_guid);
            p_node->m_item->get_ptr()->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());
            // p_node->m_window = window;
            // p_node->m_splitter = splitter;
            p_node->m_children.remove_all();

            unsigned index = tree_view_get_child_index(wnd_tv, ti);
            if (p_parent_node) {
                if (index < p_parent_node->m_children.get_count())
                    p_parent_node->m_splitter->replace_panel(
                        index, p_parent_node->m_children[index]->m_item->get_ptr());
                else
                    print_index_out_of_range();
                HTREEITEM ti_prev = TreeView_GetPrevSibling(wnd_tv, ti);
                if (!ti_prev)
                    ti_prev = TVI_FIRST;
                TreeView_DeleteItem(wnd_tv, ti);
                populate_tree(wnd, p_node, ti_parent, ti_prev);
                save_item(wnd, ti_parent);
            } else {
                TreeView_DeleteItem(wnd_tv, ti);
                populate_tree(wnd, p_node);
                m_changed = true;
            }
        }
    }
}

void LayoutTab::change_base(HWND wnd, const GUID& p_guid)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    m_node_map.clear();
    // Note: This sends TVN_DELETED notifications
    TreeView_DeleteAllItems(wnd_tv);
    m_node_root->m_children.remove_all();

    m_node_root->m_item->get_ptr()->set_panel_guid(p_guid);
    m_node_root->m_window.release();
    m_node_root->m_splitter.release();
    populate_tree(wnd, m_node_root);
    m_changed = true;
}

void LayoutTab::save_item(HWND wnd, HTREEITEM ti)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);

    auto p_node = m_node_map.at(ti);
    if (p_node->m_window.is_valid()) {
        stream_writer_memblock conf;
        try {
            abort_callback_dummy abortCallback;
            p_node->m_window->get_config(&conf, abortCallback);
        } catch (const pfc::exception&) {
        }
        p_node->m_item->get_ptr()->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());
    }
    HTREEITEM parent = TreeView_GetParent(wnd_tv, ti);
    if (parent) {
        auto p_parent_node = m_node_map.at(parent);
        auto splitter = p_parent_node->m_splitter;
        if (splitter.is_valid()) {
            unsigned index = tree_view_get_child_index(wnd_tv, ti);
            if (index < splitter->get_panel_count()) {
                splitter->replace_panel(index, p_node->m_item->get_ptr());
                save_item(wnd, parent);
            }
        }
    }
    m_changed = true;
}

void LayoutTab::set_item_property_stream(HWND wnd, HTREEITEM ti, const GUID& guid, stream_reader* val)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    if (!ti)
        return;

    HTREEITEM ti_parent = TreeView_GetParent(wnd_tv, ti);

    if (!ti_parent)
        return;

    auto p_node = m_node_map.at(ti);
    auto p_node_parent = m_node_map.at(ti_parent);
    unsigned index = tree_view_get_child_index(wnd_tv, ti);
    if (index < p_node_parent->m_splitter->get_panel_count()) {
        abort_callback_dummy abortCallback;
        p_node_parent->m_splitter->set_config_item(index, guid, val, abortCallback);
        p_node_parent->m_splitter->get_panel(index, *p_node->m_item);
        save_item(wnd, ti_parent);
    }
}

void LayoutTab::set_item_property_stream(HWND wnd, const GUID& guid, stream_reader* val)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    set_item_property_stream(wnd, TreeView_GetSelection(wnd_tv), guid, val);
}

void LayoutTab::run_configure(HWND wnd)
{
    HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
    HTREEITEM ti = TreeView_GetSelection(wnd_tv);
    if (!ti)
        return;

    HTREEITEM ti_parent = TreeView_GetParent(wnd_tv, ti);
    if (!ti_parent)
        return;

    auto p_node = m_node_map.at(ti);
    auto p_node_parent = m_node_map.at(ti_parent);
    unsigned index = tree_view_get_child_index(wnd_tv, ti);
    if (index < p_node_parent->m_splitter->get_panel_count()) {
        if (p_node->m_window.is_valid() && p_node->m_window->show_config_popup(wnd)) {
            save_item(wnd, ti);
        }
    }
}

void LayoutTab::initialise_tree(HWND wnd)
{
    m_node_root = std::make_shared<LayoutTabNode>();
    cfg_layout.get_preset(m_active_preset, *m_node_root->m_item);
    // g_layout_window.get_child(*g_node_root->m_item);
    populate_tree(wnd, m_node_root);
}

void LayoutTab::deinitialise_tree(HWND wnd)
{
    m_node_map.clear();
    // Note: This sends TVN_DELETED notifications
    TreeView_DeleteAllItems(GetDlgItem(wnd, IDC_TREE));
    m_node_root.reset();
}

void LayoutTab::apply()
{
    if (m_changed) {
        m_changed = false;
        if (m_active_preset != cfg_layout.get_active())
            cfg_layout.save_active_preset();
        cfg_layout.set_preset(m_active_preset, m_node_root->m_item->get_ptr());
        cfg_layout.set_active_preset(m_active_preset);
        // g_layout_window.set_child(g_node_root->m_item->get_ptr());
    }
}

void LayoutTab::initialise_presets(HWND wnd)
{
    unsigned count = cfg_layout.get_presets().get_count();
    for (unsigned n = 0; n < count; n++) {
        uSendDlgItemMessageText(wnd, IDC_PRESETS, CB_ADDSTRING, 0, cfg_layout.get_presets()[n].m_name);
    }
    ComboBox_SetCurSel(GetDlgItem(wnd, IDC_PRESETS), m_active_preset);
}

void LayoutTab::switch_to_preset(HWND wnd, unsigned index)
{
    if (index < cfg_layout.get_presets().get_count()) {
        if (m_changed)
            cfg_layout.set_preset(m_active_preset, m_node_root->m_item->get_ptr());
        m_changed = true;
        deinitialise_tree(wnd);
        m_active_preset = index;
        initialise_tree(wnd);
    }
}

BOOL LayoutTab::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        uih::tree_view_set_explorer_theme(GetDlgItem(wnd, IDC_TREE));
        cfg_layout.save_active_preset();
        if (!cfg_layout.get_presets().get_count())
            cfg_layout.reset_presets();
        m_changed = false;
        m_active_preset = cfg_layout.get_active();
        if (m_active_preset >= cfg_layout.get_presets().get_count()) {
            m_active_preset = 0;
            m_changed = true;
        }
        initialise_presets(wnd);
        initialise_tree(wnd);

        uSendDlgItemMessageText(wnd, IDC_CAPTIONSTYLE, CB_ADDSTRING, 0, "Horizontal");
        uSendDlgItemMessageText(wnd, IDC_CAPTIONSTYLE, CB_ADDSTRING, 0, "Vertical");

        m_initialised = true;
    } break;
    case WM_DESTROY:
        m_initialised = false;
        apply();
        deinitialise_tree(wnd);
        break;
    case WM_COMMAND:
        switch (wp) {
        case (CBN_SELCHANGE << 16) | IDC_PRESETS:
            switch_to_preset(wnd, SendMessage((HWND)lp, CB_GETCURSEL, 0, 0));
            break;
        case IDC_NEW_PRESET: {
            const auto preset_name = helpers::show_rename_dialog_box(wnd, "New preset: Enter name", "New preset");

            if (preset_name) {
                t_size index = cfg_layout.add_preset(*preset_name);
                uSendDlgItemMessageText(wnd, IDC_PRESETS, CB_ADDSTRING, NULL, preset_name->get_ptr());
                SendDlgItemMessage(wnd, IDC_PRESETS, CB_SETCURSEL, index, NULL);
                switch_to_preset(wnd, index);
            }
        } break;
        case IDC_DUPLICATE_PRESET: {
            pfc::string8 suggested_preset_name;
            cfg_layout.get_preset_name(m_active_preset, suggested_preset_name);
            suggested_preset_name << " (copy)";

            const auto preset_name
                = helpers::show_rename_dialog_box(wnd, "Duplicate preset: Enter name", suggested_preset_name);

            if (preset_name) {
                ConfigLayout::Preset preset;
                preset.m_name = *preset_name;
                preset.set(m_node_root->m_item->get_ptr());
                auto preset_index = cfg_layout.add_preset(preset);

                uSendDlgItemMessageText(wnd, IDC_PRESETS, CB_ADDSTRING, NULL, *preset_name);
                SendDlgItemMessage(wnd, IDC_PRESETS, CB_SETCURSEL, preset_index, NULL);
                switch_to_preset(wnd, preset_index);
            }
        } break;
        case IDC_RENAME_PRESET: {
            pfc::string8 current_name;
            cfg_layout.get_preset_name(m_active_preset, current_name);
            const auto new_preset_name
                = helpers::show_rename_dialog_box(wnd, "Rename preset: Enter name", current_name);

            HWND wnd_combo = GetDlgItem(wnd, IDC_PRESETS);
            unsigned index = ComboBox_GetCurSel(wnd_combo);

            if (new_preset_name) {
                cfg_layout.set_preset_name(index, new_preset_name->c_str(), new_preset_name->get_length());
                ComboBox_DeleteString(wnd_combo, index);
                uSendDlgItemMessageText(wnd, IDC_PRESETS, CB_INSERTSTRING, index, *new_preset_name);
                ComboBox_SetCurSel(wnd_combo, index);
            }
        } break;
        case IDC_DELETE_PRESET: {
            deinitialise_tree(wnd);
            HWND wnd_combo = GetDlgItem(wnd, IDC_PRESETS);
            t_size count = cfg_layout.delete_preset(m_active_preset);
            ComboBox_DeleteString(wnd_combo, m_active_preset);
            if (!count) {
                cfg_layout.reset_presets();
                m_active_preset = 0;
                initialise_presets(wnd);
                ComboBox_SetCurSel(wnd_combo, m_active_preset);
                initialise_tree(wnd);
            } else {
                ComboBox_SetCurSel(
                    wnd_combo, m_active_preset < ComboBox_GetCount(wnd_combo) ? m_active_preset : --m_active_preset);
                initialise_tree(wnd);
            }
            m_changed = true;
        } break;
        case IDC_RESET_PRESETS:
            if (win32_helpers::message_box(wnd,
                    _T(
                                               "This will reset layout presets to default values. Are you sure you wish to so this?"
                                           ),
                    _T("Reset presets?"), MB_YESNO | MB_ICONQUESTION)
                == IDYES) {
                deinitialise_tree(wnd);
                HWND wnd_combo = GetDlgItem(wnd, IDC_PRESETS);
                ComboBox_ResetContent(wnd_combo);
                cfg_layout.reset_presets();
                m_active_preset = 0;
                initialise_presets(wnd);
                ComboBox_SetCurSel(wnd_combo, m_active_preset);
                initialise_tree(wnd);
                m_changed = true;
            }
            break;
        case IDC_LOCKED:
            set_item_property(wnd, uie::splitter_window::bool_locked, (bool)(Button_GetCheck(HWND(lp)) != 0));
            break;
        case IDC_USE_CUSTOM_TITLE: {
            bool val = Button_GetCheck(HWND(lp)) != 0;
            set_item_property(wnd, uie::splitter_window::bool_use_custom_title, val);
            EnableWindow(GetDlgItem(wnd, IDC_CUSTOM_TITLE), val);
        } break;
        case IDC_CUSTOM_TITLE | (EN_CHANGE << 16):
            if (!m_initialising) {
                string_utf8_from_window text((HWND)lp);
                stream_writer_memblock str;
                abort_callback_impl p_abort;
                str.write_string(text, p_abort);
                stream_reader_memblock_ref reader(str.m_data.get_ptr(), str.m_data.get_size());
                set_item_property_stream(wnd, uie::splitter_window::string_custom_title, &reader);
            }
            break;
        case IDC_AUTOHIDE:
            set_item_property(wnd, uie::splitter_window::bool_autohide, (bool)(Button_GetCheck(HWND(lp)) != 0));
            break;
        case IDC_CAPTION:
            set_item_property(wnd, uie::splitter_window::bool_show_caption, (bool)(Button_GetCheck(HWND(lp)) != 0));
            break;
        case IDC_HIDDEN:
            set_item_property(wnd, uie::splitter_window::bool_hidden, (bool)(Button_GetCheck(HWND(lp)) != 0));
            break;
        case IDC_TOGGLE_AREA:
            set_item_property(wnd, uie::splitter_window::bool_show_toggle_area, (bool)(Button_GetCheck(HWND(lp)) != 0));
            break;
        case (CBN_SELCHANGE << 16) | IDC_CAPTIONSTYLE:
            set_item_property(wnd, uie::splitter_window::uint32_orientation,
                (t_uint32)(SendMessage((HWND)lp, CB_GETCURSEL, 0, 0) ? 1 : 0));
            break;
        case IDC_CONFIGURE:
            run_configure(wnd);
            break;
        case IDC_APPLY:
            apply();
            break;
        }
        break;
    case WM_NOTIFY: {
        auto hdr = (LPNMHDR)lp;

        switch (hdr->idFrom) {
        case IDC_TREE:
            switch (hdr->code) {
            case TVN_ITEMEXPANDED: {
                auto param = reinterpret_cast<LPNMTREEVIEW>(hdr);
                auto node = m_node_map.at(param->itemNew.hItem);
                node->m_expanded = (param->itemNew.state & TVIS_EXPANDED) != 0;
                break;
            }
            case TVN_DELETEITEM: {
                auto param = reinterpret_cast<LPNMTREEVIEW>(hdr);
                m_node_map.erase(param->itemOld.hItem);
                break;
            }
            case TVN_SELCHANGED: {
                TRACK_CALL_TEXT("tab_layout::TVN_SELCHANGED");

                bool hidden = false;
                bool locked = false;
                bool orientation = false;
                bool caption = false;
                bool autohide = false;
                bool configure = false;
                bool toggle = false;
                bool use_custom_title = false;
                bool custom_title = false;

                bool hidden_val = false;
                bool locked_val = false;
                unsigned orientation_val = 0;
                bool caption_val = false;
                bool autohide_val = false;
                bool toggle_val = false;
                bool use_custom_title_val = false;
                pfc::string8 custom_title_val;

                auto param = (LPNMTREEVIEW)hdr;
                if (param->itemNew.hItem) {
                    auto p_node = m_node_map.at(param->itemNew.hItem);
                    LayoutTabNode::ptr p_parent_node;

                    HTREEITEM ti_parent = TreeView_GetParent(param->hdr.hwndFrom, param->itemNew.hItem);

                    if (ti_parent) {
                        p_parent_node = m_node_map.at(ti_parent);
                    }
                    unsigned index = tree_view_get_child_index(param->hdr.hwndFrom, param->itemNew.hItem);

                    configure = p_node->m_window.is_valid() && p_node->m_window->have_config_popup();

                    if (p_parent_node && index < p_parent_node->m_splitter->get_panel_count()) {
                        hidden = p_parent_node->m_splitter->get_config_item_supported(
                            index, uie::splitter_window::bool_hidden);
                        locked = p_parent_node->m_splitter->get_config_item_supported(
                            index, uie::splitter_window::bool_locked);
                        orientation = p_parent_node->m_splitter->get_config_item_supported(
                            index, uie::splitter_window::uint32_orientation);
                        autohide = p_parent_node->m_splitter->get_config_item_supported(
                            index, uie::splitter_window::bool_autohide);
                        caption = p_parent_node->m_splitter->get_config_item_supported(
                            index, uie::splitter_window::bool_show_caption);
                        toggle = p_parent_node->m_splitter->get_config_item_supported(
                            index, uie::splitter_window::bool_show_toggle_area);
                        use_custom_title = p_parent_node->m_splitter->get_config_item_supported(
                            index, uie::splitter_window::bool_use_custom_title);
                        bool cust_title_supported = p_parent_node->m_splitter->get_config_item_supported(
                            index, uie::splitter_window::string_custom_title);
                        custom_title = use_custom_title && cust_title_supported;

                        if (hidden)
                            p_parent_node->m_splitter->get_config_item(
                                index, uie::splitter_window::bool_hidden, hidden_val);
                        if (locked)
                            p_parent_node->m_splitter->get_config_item(
                                index, uie::splitter_window::bool_locked, locked_val);
                        if (orientation)
                            p_parent_node->m_splitter->get_config_item(
                                index, uie::splitter_window::uint32_orientation, orientation_val);
                        if (autohide)
                            p_parent_node->m_splitter->get_config_item(
                                index, uie::splitter_window::bool_autohide, autohide_val);
                        if (caption)
                            p_parent_node->m_splitter->get_config_item(
                                index, uie::splitter_window::bool_show_caption, caption_val);
                        if (toggle)
                            p_parent_node->m_splitter->get_config_item(
                                index, uie::splitter_window::bool_show_toggle_area, toggle_val);
                        if (use_custom_title)
                            p_parent_node->m_splitter->get_config_item(
                                index, uie::splitter_window::bool_use_custom_title, use_custom_title_val);
                        if (!use_custom_title_val)
                            custom_title = false;
                        if (cust_title_supported) {
                            stream_writer_memblock str;
                            p_parent_node->m_splitter->get_config_item(
                                index, uie::splitter_window::string_custom_title, &str);
                            abort_callback_dummy abortCallback;
                            stream_reader_memblock_ref(str.m_data.get_ptr(), str.m_data.get_size())
                                .read_string(custom_title_val, abortCallback);
                        }
                    }
                }
                m_initialising = true;
                EnableWindow(GetDlgItem(wnd, IDC_HIDDEN), hidden);
                EnableWindow(GetDlgItem(wnd, IDC_LOCKED), locked);
                EnableWindow(GetDlgItem(wnd, IDC_CAPTIONSTYLE), orientation);
                EnableWindow(GetDlgItem(wnd, IDC_CONFIGURE), configure);
                EnableWindow(GetDlgItem(wnd, IDC_CAPTION), caption);
                EnableWindow(GetDlgItem(wnd, IDC_AUTOHIDE), autohide);
                EnableWindow(GetDlgItem(wnd, IDC_TOGGLE_AREA), toggle);
                EnableWindow(GetDlgItem(wnd, IDC_USE_CUSTOM_TITLE), use_custom_title);
                EnableWindow(GetDlgItem(wnd, IDC_CUSTOM_TITLE), custom_title);
                Button_SetCheck(GetDlgItem(wnd, IDC_CAPTION), caption_val);
                Button_SetCheck(GetDlgItem(wnd, IDC_LOCKED), locked_val);
                Button_SetCheck(GetDlgItem(wnd, IDC_HIDDEN), hidden_val);
                Button_SetCheck(GetDlgItem(wnd, IDC_AUTOHIDE), autohide_val);
                Button_SetCheck(GetDlgItem(wnd, IDC_TOGGLE_AREA), toggle_val);
                Button_SetCheck(GetDlgItem(wnd, IDC_USE_CUSTOM_TITLE), use_custom_title_val);
                uSendDlgItemMessageText(wnd, IDC_CUSTOM_TITLE, WM_SETTEXT, NULL, custom_title_val);
                SendDlgItemMessage(wnd, IDC_CAPTIONSTYLE, CB_SETCURSEL, orientation_val, 0);
                m_initialising = false;
                break;
            }
            }
        }
    } break;
    case WM_CONTEXTMENU: {
        if ((HWND)wp == GetDlgItem(wnd, IDC_TREE)) {
            TRACK_CALL_TEXT("tab_layout::WM_CONTEXTMENU");

            HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
            POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            HTREEITEM treeitem = TreeView_GetSelection(wnd_tv);

            TVHITTESTINFO ti;
            memset(&ti, 0, sizeof(ti));

            if (pt.x == -1 && pt.y == -1) {
                RECT rc;
                TreeView_GetItemRect(wnd_tv, treeitem, &rc, TRUE);
                ti.pt.x = rc.left;
                ti.pt.y = rc.top + (rc.bottom - rc.top) / 2;
                pt = ti.pt;
                MapWindowPoints(wnd_tv, HWND_DESKTOP, &pt, 1);
            } else {
                ti.pt = pt;
                ScreenToClient(wnd_tv, &ti.pt);
            }
            SendMessage(wnd_tv, TVM_HITTEST, 0, (long)&ti);
            if (ti.hItem) {
                enum { ID_REMOVE = 1, ID_MOVE_UP, ID_MOVE_DOWN, ID_COPY, ID_PASTE, ID_CHANGE_BASE };
                unsigned ID_INSERT_BASE = ID_CHANGE_BASE + 1;
                HTREEITEM ti_parent = TreeView_GetParent(wnd_tv, ti.hItem);

                SendMessage(wnd_tv, TVM_SELECTITEM, TVGN_CARET, (long)ti.hItem);

                unsigned index = tree_view_get_child_index(wnd_tv, ti.hItem);

                auto p_node = m_node_map.at(ti.hItem);
                LayoutTabNode::ptr p_parent_node;

                service_ptr_t<uie::splitter_window> p_splitter;
                if (p_node->m_window.is_valid())
                    p_node->m_window->service_query_t(p_splitter);

                HMENU menu = CreatePopupMenu();

                uie::window_info_list_simple panels;
                get_panel_list(panels);

                if (ti_parent) {
                    p_parent_node = m_node_map.at(ti_parent);
                }

                if (!ti_parent) {
                    HMENU menu_change_base = CreatePopupMenu();
                    HMENU popup = nullptr;
                    unsigned count = panels.get_count();
                    for (unsigned n = 0; n < count; n++) {
                        if (!n || uStringCompare(panels[n - 1].category, panels[n].category)) {
                            if (n)
                                uAppendMenu(
                                    menu_change_base, MF_STRING | MF_POPUP, (UINT)popup, panels[n - 1].category);
                            popup = CreatePopupMenu();
                        }
                        uAppendMenu(popup, (MF_STRING), ID_CHANGE_BASE + n, panels[n].name);
                        ID_INSERT_BASE++;
                        if (n == count - 1)
                            uAppendMenu(menu_change_base, MF_STRING | MF_POPUP, (UINT)popup, panels[n].category);
                    }
                    uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)menu_change_base, "Change base");
                }
                unsigned ID_SWITCH_BASE = ID_INSERT_BASE + 1;
                if (p_splitter.is_valid() && p_node->m_children.get_count() < p_splitter->get_maximum_panel_count()) {
                    HMENU menu_change_base = CreatePopupMenu();
                    HMENU popup = nullptr;
                    unsigned count = panels.get_count();
                    unsigned last = 0;
                    for (unsigned n = 0; n < count; n++) {
                        if (!panels[n].prefer_multiple_instances || !m_node_root->have_item(panels[n].guid)) {
                            if (!popup || uStringCompare(panels[last].category, panels[n].category)) {
                                if (popup)
                                    uAppendMenu(
                                        menu_change_base, MF_STRING | MF_POPUP, (UINT)popup, panels[last].category);
                                popup = CreatePopupMenu();
                            }
                            uAppendMenu(popup, (MF_STRING), ID_INSERT_BASE + n, panels[n].name);
                            last = n;
                        }
                        if (n == count - 1)
                            uAppendMenu(menu_change_base, MF_STRING | MF_POPUP, (UINT)popup, panels[n].category);
                    }
                    uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)menu_change_base, "Insert panel");
                    ID_SWITCH_BASE += count;
                }
                if (p_splitter.is_valid()) {
                    unsigned count_exts = panels.get_count();
                    HMENU menu_insert = CreatePopupMenu();
                    for (unsigned n = 0; n < count_exts; n++) {
                        if (panels[n].type & uie::type_splitter) {
                            uAppendMenu(menu_insert, (MF_STRING), ID_SWITCH_BASE + n, panels[n].name);
                        }
                    }
                    AppendMenu(menu, MF_STRING | MF_POPUP, (UINT)menu_insert, _T("Change splitter type"));
                }
                if (ti_parent) {
                    if (GetMenuItemCount(menu))
                        AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
                    if (p_parent_node && index)
                        AppendMenu(menu, MF_STRING, ID_MOVE_UP, _T("Move up"));
                    if (p_parent_node && index + 1 < p_parent_node->m_splitter->get_panel_count())
                        AppendMenu(menu, MF_STRING, ID_MOVE_DOWN, _T("Move down"));
                    AppendMenu(menu, MF_STRING, ID_REMOVE, _T("Remove panel"));
                }
                AppendMenu(menu, MF_STRING, ID_COPY, _T("Copy panel"));
                if (splitter_utils::is_splitter_item_in_clipboard() && p_splitter.is_valid()
                    && p_node->m_children.get_count() < p_splitter->get_maximum_panel_count())
                    AppendMenu(menu, MF_STRING, ID_PASTE, _T("Paste panel"));

                menu_helpers::win32_auto_mnemonics(menu);

                unsigned cmd
                    = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, nullptr);
                DestroyMenu(menu);

                if (cmd) {
                    if (cmd >= ID_SWITCH_BASE) {
                        switch_splitter(wnd, ti.hItem, panels[cmd - ID_SWITCH_BASE].guid);
                    } else if (cmd >= ID_INSERT_BASE) {
                        insert_item(wnd, ti.hItem, panels[cmd - ID_INSERT_BASE].guid);
                    } else if (cmd >= ID_CHANGE_BASE) {
                        change_base(wnd, panels[cmd - ID_CHANGE_BASE].guid);
                    } else if (cmd == ID_REMOVE) {
                        remove_node(wnd, ti.hItem);
                    } else if (cmd == ID_MOVE_UP) {
                        move_item(wnd, ti.hItem, true);
                    } else if (cmd == ID_MOVE_DOWN) {
                        move_item(wnd, ti.hItem, false);
                    } else if (cmd == ID_COPY) {
                        copy_item(wnd, ti.hItem);
                    } else if (cmd == ID_PASTE) {
                        paste_item(wnd, ti.hItem);
                    }
                }
            }
        }
        return 0;
    } break;
    }
    return 0;
}

HWND LayoutTab::create(HWND wnd)
{
    return m_helper.create(wnd, IDD_PREFS_LAYOUT_PRESETS,
        [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
}

const char* LayoutTab::get_name()
{
    return "Layout";
}

bool LayoutTab::get_help_url(pfc::string_base& p_out)
{
    p_out = "http://yuo.be/wiki/columns_ui:config:layout";
    return true;
}

} // namespace cui::prefs
