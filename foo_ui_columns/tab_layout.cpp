#include "stdafx.h"

#include "tab_layout.h"
#include "functional.h"
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

void LayoutTabNode::build()
{
    assert(m_window.is_empty());
    assert(m_children.empty());

    if (uie::window::create_by_guid(m_item->get_ptr()->get_panel_guid(), m_window)) {
        stream_writer_memblock config;
        m_item->get_ptr()->get_panel_config(&config);
        m_window->get_name(m_name);
        try {
            m_window->set_config_from_ptr(config.m_data.get_ptr(), config.m_data.get_size(), fb2k::noAbort);
        } catch (const pfc::exception& ex) {
            console::formatter formatter;
            formatter << "warning: " << m_name << ": function uie::window::set_config; error " << ex.what();
        }
    } else
        m_name = "<unknown>";

    if (m_window.is_valid() && m_window->service_query_t(m_splitter)) {
        for (auto i : ranges::views::iota(size_t{0}, m_splitter->get_panel_count())) {
            auto& child = m_children.emplace_back(std::make_shared<LayoutTabNode>());
            m_splitter->get_panel(i, *child->m_item);

            child->build();
        }
    }
}

bool LayoutTabNode::have_item(const GUID& p_guid)
{
    if (m_item->get_ptr()->get_panel_guid() == p_guid)
        return true;

    return ranges::any_of(m_children, [p_guid](auto&& node) { return node->have_item(p_guid); });
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

void LayoutTab::populate_tree(HWND wnd, const LayoutTabNode::ptr& node, HTREEITEM ti_parent, HTREEITEM ti_after)
{
    SendMessage(m_wnd_tree, WM_SETREDRAW, FALSE, NULL);
    auto node_map = __populate_tree(m_wnd_tree, node, ti_parent, ti_after);
    SendMessage(m_wnd_tree, WM_SETREDRAW, TRUE, NULL);
    merge_maps(m_node_map, node_map);
}

std::unordered_map<HTREEITEM, LayoutTabNode::ptr> LayoutTab::__populate_tree(
    HWND wnd_tree, const LayoutTabNode::ptr& node, HTREEITEM ti_parent, HTREEITEM ti_after)
{
    std::unordered_map<HTREEITEM, LayoutTabNode::ptr> node_map;
    const auto ti_item = insert_item_in_tree_view(wnd_tree, node->m_name, ti_parent, ti_after, node->m_expanded);

    if (ti_item) {
        node_map[ti_item] = node;

        for (auto&& child_node : node->m_children) {
            auto child_node_map = __populate_tree(wnd_tree, child_node, ti_item);
            merge_maps(node_map, child_node_map);
        }
    }

    return node_map;
}

void LayoutTab::remove_node(HWND wnd, HTREEITEM ti)
{
    HTREEITEM ti_parent = TreeView_GetParent(m_wnd_tree, ti);
    if (!ti_parent)
        return;

    auto p_parent_node = m_node_map.at(ti_parent);
    unsigned index = tree_view_get_child_index(m_wnd_tree, ti);
    if (index < p_parent_node->m_children.size()) {
        p_parent_node->m_children.erase(p_parent_node->m_children.begin() + index);
        p_parent_node->m_splitter->remove_panel(index);
        TreeView_DeleteItem(m_wnd_tree, ti);
        save_item(wnd, ti_parent);
    }
}

void LayoutTab::insert_item(HWND wnd, HTREEITEM ti_parent, const GUID& p_guid, HTREEITEM ti_after)
{
    // uie::splitter_item_simple_t p_item;
    // p_item.set_panel_guid(p_guid);
    auto p_node = std::make_shared<LayoutTabNode>();
    *p_node->m_item = new uie::splitter_item_simple_t;
    p_node->m_item->get_ptr()->set_panel_guid(p_guid);
    auto p_parent = m_node_map.at(ti_parent);
    service_ptr_t<uie::splitter_window> p_splitter;
    if (p_parent->m_window.is_valid() && p_parent->m_window->service_query_t(p_splitter)) {
        auto& parent_children = p_parent->m_children;

        unsigned index
            = ti_after != TVI_LAST ? tree_view_get_child_index(m_wnd_tree, ti_after) + 1 : parent_children.size();
        if (index <= parent_children.size()) {
            p_splitter->insert_panel(index, p_node->m_item->get_ptr());
            parent_children.insert(parent_children.begin() + index, p_node);
            build_node_and_populate_tree(wnd, p_node, ti_parent, ti_after);
            save_item(wnd, ti_parent);
        }
    }
}

void LayoutTab::copy_item(HWND wnd, HTREEITEM ti)
{
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
    auto p_node = std::make_shared<LayoutTabNode>();
    auto splitter_item = splitter_utils::get_splitter_item_from_clipboard_safe(wnd);

    if (!splitter_item || !fix_paste_item(*splitter_item))
        return;

    *p_node->m_item = splitter_item.release();

    auto p_parent = m_node_map.at(ti_parent);
    service_ptr_t<uie::splitter_window> p_splitter;
    if (p_parent->m_window.is_valid() && p_parent->m_window->service_query_t(p_splitter)) {
        unsigned index{};
        auto& parent_children = p_parent->m_children;

        if (ti_after == TVI_LAST)
            index = parent_children.size();
        else
            index = tree_view_get_child_index(m_wnd_tree, ti_after) + 1;

        if (index <= parent_children.size()) {
            p_splitter->insert_panel(index, p_node->m_item->get_ptr());
            parent_children.insert(parent_children.begin() + index, p_node);
            build_node_and_populate_tree(wnd, p_node, ti_parent, ti_after);
            save_item(wnd, ti_parent);
        }
    }
}

void LayoutTab::move_item(HWND wnd, HTREEITEM ti, bool up)
{
    HTREEITEM ti_parent = TreeView_GetParent(m_wnd_tree, ti);

    auto p_parent_node = m_node_map.at(ti_parent);
    auto& parent_children = p_parent_node->m_children;
    unsigned index = tree_view_get_child_index(m_wnd_tree, ti);
    if (up) {
        if (index > 0) {
            p_parent_node->m_splitter->move_up(index);
            std::swap(parent_children[index], parent_children[index - 1]);
            HTREEITEM ti_prev = TreeView_GetPrevSibling(m_wnd_tree, ti);
            TreeView_DeleteItem(m_wnd_tree, ti_prev);

            const auto child_node = parent_children[index];
            populate_tree(wnd, child_node, ti_parent, ti);

            save_item(wnd, ti_parent);
        }
    }
    if (!up) {
        if (index + 1 < parent_children.size()) {
            p_parent_node->m_splitter->move_down(index);
            std::swap(parent_children[index], parent_children[index + 1]);
            HTREEITEM ti_next = TreeView_GetNextSibling(m_wnd_tree, ti);
            HTREEITEM ti_prev = TreeView_GetPrevSibling(m_wnd_tree, ti);
            if (!ti_prev)
                ti_prev = TVI_FIRST;
            TreeView_DeleteItem(m_wnd_tree, ti_next);

            const auto child_node = parent_children[index];
            populate_tree(wnd, child_node, ti_parent, ti_prev);

            save_item(wnd, ti_parent);
        }
    }
}

void LayoutTab::print_index_out_of_range()
{
    console::print("layout editor: internal error: index out of range");
}

void LayoutTab::build_node_and_populate_tree(HWND wnd, LayoutTabNode::ptr node, HTREEITEM ti_parent, HTREEITEM ti_after)
{
    node->build();
    populate_tree(wnd, node, ti_parent, ti_after);
}

void LayoutTab::switch_splitter(HWND wnd, HTREEITEM ti, const GUID& p_guid)
{
    HTREEITEM ti_parent = TreeView_GetParent(m_wnd_tree, ti);

    auto old_node = m_node_map.at(ti);
    LayoutTabNode::ptr p_parent_node;
    if (ti_parent)
        p_parent_node = m_node_map.at(ti_parent);

    uie::window_ptr window;
    service_ptr_t<uie::splitter_window> splitter;
    if (uie::window::create_by_guid(p_guid, window) && window->service_query_t(splitter)) {
        unsigned count = std::min(old_node->m_children.size(), splitter->get_maximum_panel_count());
        if (count == old_node->m_children.size()
            || MessageBox(wnd, _T("The number of child items will not fit in the selected splitter type. Continue?"),
                   _T("Warning"), MB_YESNO | MB_ICONEXCLAMATION)
                == IDYES) {
            for (unsigned n = 0; n < count; n++)
                splitter->add_panel(old_node->m_children[n]->m_item->get_ptr());
            stream_writer_memblock conf;
            try {
                abort_callback_dummy abort_callback;
                splitter->get_config(&conf, abort_callback);
            } catch (const pfc::exception&) {
            }

            auto new_node = std::make_shared<LayoutTabNode>();
            new_node->m_item = old_node->m_item;
            new_node->m_expanded = old_node->m_expanded;
            new_node->m_item->get_ptr()->set_panel_guid(p_guid);
            new_node->m_item->get_ptr()->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());

            unsigned index = tree_view_get_child_index(m_wnd_tree, ti);
            if (p_parent_node) {
                ranges::replace(p_parent_node->m_children, old_node, new_node);
                if (index < p_parent_node->m_children.size())
                    p_parent_node->m_splitter->replace_panel(
                        index, p_parent_node->m_children[index]->m_item->get_ptr());
                else
                    print_index_out_of_range();
                HTREEITEM ti_prev = TreeView_GetPrevSibling(m_wnd_tree, ti);
                if (!ti_prev)
                    ti_prev = TVI_FIRST;
                TreeView_DeleteItem(m_wnd_tree, ti);
                build_node_and_populate_tree(wnd, new_node, ti_parent, ti_prev);
                save_item(wnd, ti_parent);
            } else {
                TreeView_DeleteItem(m_wnd_tree, ti);
                build_node_and_populate_tree(wnd, new_node);
                m_changed = true;
            }
        }
    }
}

void LayoutTab::change_base(HWND wnd, const GUID& p_guid)
{
    m_node_map.clear();
    // Note: This sends TVN_DELETED notifications
    TreeView_DeleteAllItems(m_wnd_tree);
    m_node_root->m_children.clear();

    m_node_root->m_item->get_ptr()->set_panel_guid(p_guid);
    m_node_root->m_window.release();
    m_node_root->m_splitter.release();
    build_node_and_populate_tree(wnd, m_node_root);
    m_changed = true;
}

void LayoutTab::save_item(HWND wnd, HTREEITEM ti)
{
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
    HTREEITEM parent = TreeView_GetParent(m_wnd_tree, ti);
    if (parent) {
        auto p_parent_node = m_node_map.at(parent);
        auto splitter = p_parent_node->m_splitter;
        if (splitter.is_valid()) {
            unsigned index = tree_view_get_child_index(m_wnd_tree, ti);
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
    if (!ti)
        return;

    HTREEITEM ti_parent = TreeView_GetParent(m_wnd_tree, ti);

    if (!ti_parent)
        return;

    auto p_node = m_node_map.at(ti);
    auto p_node_parent = m_node_map.at(ti_parent);
    unsigned index = tree_view_get_child_index(m_wnd_tree, ti);
    if (index < p_node_parent->m_splitter->get_panel_count()) {
        abort_callback_dummy abortCallback;
        p_node_parent->m_splitter->set_config_item(index, guid, val, abortCallback);
        p_node_parent->m_splitter->get_panel(index, *p_node->m_item);
        save_item(wnd, ti_parent);
    }
}

void LayoutTab::set_item_property_stream(HWND wnd, const GUID& guid, stream_reader* val)
{
    set_item_property_stream(wnd, TreeView_GetSelection(m_wnd_tree), guid, val);
}

void LayoutTab::run_configure(HWND wnd)
{
    HTREEITEM ti = TreeView_GetSelection(m_wnd_tree);
    if (!ti)
        return;

    HTREEITEM ti_parent = TreeView_GetParent(m_wnd_tree, ti);
    if (!ti_parent)
        return;

    auto p_node = m_node_map.at(ti);
    auto p_node_parent = m_node_map.at(ti_parent);
    unsigned index = tree_view_get_child_index(m_wnd_tree, ti);
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
    build_node_and_populate_tree(wnd, m_node_root);
}

void LayoutTab::deinitialise_tree(HWND wnd)
{
    m_node_map.clear();
    // Note: This sends TVN_DELETED notifications
    TreeView_DeleteAllItems(m_wnd_tree);
    m_node_root.reset();

    on_tree_selection_change(nullptr);
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
        m_wnd_tree = GetDlgItem(wnd, IDC_TREE);
        uih::tree_view_set_explorer_theme(m_wnd_tree);
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
        m_wnd_tree = nullptr;
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

                auto param = reinterpret_cast<LPNMTREEVIEW>(hdr);
                on_tree_selection_change(param->itemNew.hItem);
                break;
            }
            }
        }
    } break;
    case WM_CONTEXTMENU: {
        if ((HWND)wp == m_wnd_tree) {
            TRACK_CALL_TEXT("tab_layout::WM_CONTEXTMENU");

            POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            HTREEITEM treeitem = TreeView_GetSelection(m_wnd_tree);

            TVHITTESTINFO ti;
            memset(&ti, 0, sizeof(ti));

            if (pt.x == -1 && pt.y == -1) {
                RECT rc;
                TreeView_GetItemRect(m_wnd_tree, treeitem, &rc, TRUE);
                ti.pt.x = rc.left;
                ti.pt.y = rc.top + (rc.bottom - rc.top) / 2;
                pt = ti.pt;
                MapWindowPoints(m_wnd_tree, HWND_DESKTOP, &pt, 1);
            } else {
                ti.pt = pt;
                ScreenToClient(m_wnd_tree, &ti.pt);
            }
            SendMessage(m_wnd_tree, TVM_HITTEST, 0, (long)&ti);
            if (ti.hItem) {
                enum { ID_REMOVE = 1, ID_MOVE_UP, ID_MOVE_DOWN, ID_COPY, ID_PASTE, ID_CHANGE_BASE };
                unsigned ID_INSERT_BASE = ID_CHANGE_BASE + 1;
                HTREEITEM ti_parent = TreeView_GetParent(m_wnd_tree, ti.hItem);

                SendMessage(m_wnd_tree, TVM_SELECTITEM, TVGN_CARET, (long)ti.hItem);

                unsigned index = tree_view_get_child_index(m_wnd_tree, ti.hItem);

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
                if (p_splitter.is_valid() && p_node->m_children.size() < p_splitter->get_maximum_panel_count()) {
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
                    && p_node->m_children.size() < p_splitter->get_maximum_panel_count())
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

void LayoutTab::on_tree_selection_change(HTREEITEM tree_item)
{
    if (!m_initialised)
        return;

    const std::initializer_list<std::tuple<GUID, int>> bool_item_and_control_ids = {
        {uie::splitter_window::bool_hidden, IDC_HIDDEN},
        {uie::splitter_window::bool_locked, IDC_LOCKED},
        {uie::splitter_window::bool_autohide, IDC_AUTOHIDE},
        {uie::splitter_window::bool_show_caption, IDC_CAPTION},
        {uie::splitter_window::bool_show_toggle_area, IDC_TOGGLE_AREA},
        {uie::splitter_window::bool_use_custom_title, IDC_USE_CUSTOM_TITLE},
    };

    const std::initializer_list<std::tuple<GUID, int>> other_item_and_control_ids
        = {{uie::splitter_window::uint32_orientation, IDC_CAPTIONSTYLE},
            {uie::splitter_window::string_custom_title, IDC_CUSTOM_TITLE}};

    const auto all_item_and_control_ids = ranges::views::concat(bool_item_and_control_ids, other_item_and_control_ids);

    std::unordered_map<GUID, bool> supported_map;
    std::unordered_map<GUID, bool> enable_map;
    std::unordered_map<GUID, bool> bool_value_map;
    bool enable_configure = false;
    unsigned orientation = 0;
    pfc::string8 custom_title;

    if (tree_item) {
        LayoutTabNode::ptr parent_node;
        const auto node = m_node_map.at(tree_item);
        const unsigned index = tree_view_get_child_index(m_wnd_tree, tree_item);

        if (const auto ti_parent = TreeView_GetParent(m_wnd_tree, tree_item); ti_parent)
            parent_node = m_node_map.at(ti_parent);

        enable_configure = node->m_window.is_valid() && node->m_window->have_config_popup();

        if (parent_node && index < parent_node->m_splitter->get_panel_count()) {
            for (auto&& [item_id, _] : all_item_and_control_ids) {
                supported_map[item_id] = parent_node->m_splitter->get_config_item_supported(index, item_id);
            }

            for (auto&& [item_id, _] : bool_item_and_control_ids) {
                if (supported_map[item_id])
                    parent_node->m_splitter->get_config_item(index, item_id, bool_value_map[item_id]);
            }

            if (supported_map[uie::splitter_window::uint32_orientation])
                parent_node->m_splitter->get_config_item(index, uie::splitter_window::uint32_orientation, orientation);

            if (supported_map[uie::splitter_window::string_custom_title]) {
                stream_writer_memblock str;
                parent_node->m_splitter->get_config_item(index, uie::splitter_window::string_custom_title, &str);
                stream_reader_memblock_ref(str.m_data.get_ptr(), str.m_data.get_size())
                    .read_string(custom_title, fb2k::noAbort);
            }

            enable_map = supported_map;
            enable_map[uie::splitter_window::string_custom_title]
                = supported_map[uie::splitter_window::bool_use_custom_title]
                && supported_map[uie::splitter_window::string_custom_title]
                && bool_value_map[uie::splitter_window::bool_use_custom_title];
        }
    }
    m_initialising = true;

    for (auto&& [item_id, control_id] : all_item_and_control_ids)
        EnableWindow(m_helper.get_control_wnd(control_id), enable_map[item_id]);

    EnableWindow(m_helper.get_control_wnd(IDC_CONFIGURE), enable_configure);

    for (auto&& [item_id, control_id] : bool_item_and_control_ids)
        Button_SetCheck(m_helper.get_control_wnd(control_id), bool_value_map[item_id] ? BST_CHECKED : BST_UNCHECKED);

    uSendDlgItemMessageText(m_helper.get_wnd(), IDC_CUSTOM_TITLE, WM_SETTEXT, NULL, custom_title);
    SendDlgItemMessage(m_helper.get_wnd(), IDC_CAPTIONSTYLE, CB_SETCURSEL, orientation, 0);
    m_initialising = false;
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
