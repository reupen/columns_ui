#include "pch.h"

#include "tab_layout.h"

#include "layout.h"
#include "config.h"
#include "dark_mode_dialog.h"
#include "panel_utils.h"
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

    const auto panel_id = m_item->get_ptr()->get_panel_guid();

    if (panel_id != GUID{} && uie::window::create_by_guid(panel_id, m_window)) {
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

bool LayoutTabNode::empty() const
{
    return m_item->get_ptr()->get_panel_guid() == GUID{};
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
    uTVINSERTSTRUCT is{};
    is.hParent = ti_parent;
    is.hInsertAfter = ti_after;
    is.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
    is.item.pszText = const_cast<char*>(sz_text);
    is.item.state = is_expanded ? TVIS_EXPANDED : 0;
    is.item.stateMask = TVIS_EXPANDED;
    return uTreeView_InsertItem(wnd_tree, &is);
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
    if (ti_parent == TVI_ROOT && node->empty())
        return {};

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

    if (!ti_parent) {
        TreeView_DeleteAllItems(m_wnd_tree);
        m_node_root = std::make_shared<LayoutTabNode>();
        *m_node_root->m_item = new uie::splitter_item_simple_t;
        m_node_map.clear();
        m_changed = true;
        on_tree_selection_change(nullptr);
        return;
    }

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
    auto p_node = std::make_shared<LayoutTabNode>();
    *p_node->m_item = new uie::splitter_item_simple_t;
    p_node->m_item->get_ptr()->set_panel_guid(p_guid);
    auto p_parent = m_node_map.at(ti_parent);
    service_ptr_t<uie::splitter_window> p_splitter;
    if (p_parent->m_window.is_valid() && p_parent->m_window->service_query_t(p_splitter)) {
        auto& parent_children = p_parent->m_children;

        const size_t index
            = ti_after != TVI_LAST ? tree_view_get_child_index(m_wnd_tree, ti_after) + 1 : parent_children.size();
        if (index <= parent_children.size()) {
            p_splitter->insert_panel(index, p_node->m_item->get_ptr());
            parent_children.insert(parent_children.begin() + index, p_node);
            build_node_and_populate_tree(wnd, p_node, ti_parent, ti_after);
            save_item(wnd, ti_parent);
        }
    }
}

void LayoutTab::copy_item(HWND wnd, HTREEITEM ti) const
{
    const auto node = m_node_map.at(ti);
    splitter_utils::copy_splitter_item_to_clipboard_safe(wnd, node->m_item->get_ptr());
}

void LayoutTab::cut_item(HWND wnd, HTREEITEM ti)
{
    const auto node = m_node_map.at(ti);

    if (splitter_utils::copy_splitter_item_to_clipboard_safe(wnd, node->m_item->get_ptr(), true))
        remove_node(wnd, ti);
}

bool LayoutTab::_fix_single_instance_recur(uie::splitter_window_ptr& p_window)
{
    if (!p_window.is_valid())
        return false;

    bool modified = false;
    size_t i;
    size_t count = p_window->get_panel_count();
    pfc::array_staticsize_t<bool> mask(count);

    for (i = 0; i < count; i++) {
        uie::window_ptr p_child_window;
        uie::splitter_item_ptr p_si;
        p_window->get_panel(i, p_si);

        uie::window::create_by_guid(p_si->get_panel_guid(), p_child_window);

        mask[i] = p_child_window.is_valid() && p_child_window->get_is_single_instance()
            && m_node_root->have_item(p_si->get_panel_guid());
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
    if (m_node_root->empty())
        return true;

    uie::window::ptr p_window;
    if (!uie::window::create_by_guid(item.get_panel_guid(), p_window))
        return true;

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

    if (!splitter_item || !(ti_parent || m_node_root->empty()) || !fix_paste_item(*splitter_item))
        return;

    *p_node->m_item = splitter_item.release();

    if (!ti_parent) {
        m_node_root = p_node;
        build_node_and_populate_tree(wnd, m_node_root);
        m_changed = true;
        return;
    }

    auto p_parent = m_node_map.at(ti_parent);
    service_ptr_t<uie::splitter_window> p_splitter;
    if (p_parent->m_window.is_valid() && p_parent->m_window->service_query_t(p_splitter)) {
        size_t index{};
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
        const auto count = std::min(old_node->m_children.size(), splitter->get_maximum_panel_count());
        if (count == old_node->m_children.size()
            || dark::modal_info_box(wnd, "Change container type",
                "The number of child items will not fit in the selected container type. Do you want to continue?",
                uih::InfoBoxType::Warning, uih::InfoBoxModalType::YesNo)) {
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
    }
}

void LayoutTab::initialise_presets(HWND wnd)
{
    const auto count = cfg_layout.get_presets().get_count();
    for (size_t n = 0; n < count; n++) {
        uSendDlgItemMessageText(wnd, IDC_PRESETS, CB_ADDSTRING, 0, cfg_layout.get_presets()[n].m_name);
    }
    ComboBox_SetCurSel(GetDlgItem(wnd, IDC_PRESETS), m_active_preset);
}

void LayoutTab::switch_to_preset(HWND wnd, size_t index)
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

INT_PTR LayoutTab::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        uih::enhance_edit_control(wnd, IDC_CUSTOM_TITLE);

        m_wnd_tree = GetDlgItem(wnd, IDC_TREE);
        TreeView_SetIndent(m_wnd_tree, 0);
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
        break;
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
                size_t index = cfg_layout.add_preset(*preset_name);
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
            if (cfg_layout.get_presets().size() <= 1) {
                dark::modeless_info_box(wnd, "Delete preset",
                    "This is the only preset. It cannot be deleted as at least one preset must exist.",
                    uih::InfoBoxType::Information);
                break;
            }

            const auto is_shift_down = GetKeyState(VK_SHIFT) & 0x8000;

            if (!is_shift_down) {
                pfc::string8 preset_name;
                cfg_layout.get_preset_name(m_active_preset, preset_name);

                const auto confirmation_message
                    = fmt::format("Are you sure that you want to delete the layout preset ‘{}’?", preset_name.c_str());

                if (!dark::modal_info_box(wnd, "Delete preset", confirmation_message.c_str(), uih::InfoBoxType::Warning,
                        uih::InfoBoxModalType::YesNo))
                    break;
            }

            deinitialise_tree(wnd);
            HWND wnd_combo = GetDlgItem(wnd, IDC_PRESETS);
            cfg_layout.delete_preset(m_active_preset);
            ComboBox_DeleteString(wnd_combo, m_active_preset);
            ComboBox_SetCurSel(
                wnd_combo, m_active_preset < ComboBox_GetCount(wnd_combo) ? m_active_preset : --m_active_preset);
            initialise_tree(wnd);
            m_changed = true;
            break;
        }
        case IDC_RESET_PRESETS:
            if (dark::modal_info_box(wnd, "Reset presets",
                    "This will replace all layout presets with the default preset. Are you sure you wish to so this?",
                    uih::InfoBoxType::Neutral, uih::InfoBoxModalType::YesNo)) {
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
                const auto text = uGetWindowText((HWND)lp);
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
                (uint32_t)(SendMessage((HWND)lp, CB_GETCURSEL, 0, 0) ? 1 : 0));
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
        if (handle_wm_contextmenu(wnd, reinterpret_cast<HWND>(wp), {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}))
            return 0;
        break;
    }
    }
    return 0;
}

bool LayoutTab::handle_wm_contextmenu(HWND wnd, HWND contextmenu_wnd, POINT pt)
{
    TRACK_CALL_TEXT("tab_layout::WM_CONTEXTMENU");

    if (contextmenu_wnd != m_wnd_tree)
        return false;

    HTREEITEM treeitem = TreeView_GetSelection(m_wnd_tree);

    TVHITTESTINFO ti{};

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

    SendMessage(m_wnd_tree, TVM_HITTEST, 0, reinterpret_cast<LPARAM>(&ti));

    if (ti.hItem || m_node_root->empty()) {
        const auto panels = panel_utils::get_panel_info();

        enum {
            ID_REMOVE = 1,
            ID_MOVE_UP,
            ID_MOVE_DOWN,
            ID_CUT,
            ID_COPY,
            ID_PASTE,
            ID_REPLACE_ROOT_BASE,
        };

        const auto id_add_child_base = ID_REPLACE_ROOT_BASE + gsl::narrow<uint32_t>(panels.size());
        const auto id_splitter_type_base = id_add_child_base + gsl::narrow<uint32_t>(panels.size());

        HTREEITEM ti_parent = ti.hItem ? TreeView_GetParent(m_wnd_tree, ti.hItem) : nullptr;

        if (ti.hItem)
            SendMessage(m_wnd_tree, TVM_SELECTITEM, TVGN_CARET, reinterpret_cast<LPARAM>(ti.hItem));

        std::optional panel_index
            = ti.hItem ? std::make_optional(tree_view_get_child_index(m_wnd_tree, ti.hItem)) : std::nullopt;

        auto p_node = ti.hItem ? m_node_map.at(ti.hItem) : m_node_root;
        LayoutTabNode::ptr p_parent_node;

        service_ptr_t<uie::splitter_window> p_splitter;
        if (p_node->m_window.is_valid())
            p_node->m_window->service_query_t(p_splitter);

        uih::Menu menu;

        if (ti_parent)
            p_parent_node = m_node_map.at(ti_parent);

        auto grouped_panels = panel_utils::get_grouped_panel_info(panels);

        if (p_splitter.is_valid() && p_node->m_children.size() < p_splitter->get_maximum_panel_count()) {
            uih::Menu insert_menu;
            for (auto&& group : grouped_panels) {
                uih::Menu category_menu;

                for (auto&& [index, panel] : group) {
                    if (!panel.is_single_instance || !m_node_root->have_item(panel.id))
                        category_menu.append_command(id_add_child_base + gsl::narrow<uint32_t>(index), panel.name);
                }

                if (category_menu.size() > 0) {
                    const auto& category = group.front().second.category;
                    insert_menu.append_submenu(std::move(category_menu), category);
                }
            }

            menu.append_submenu(std::move(insert_menu), L"Add child");
        }

        if (p_splitter.is_valid()) {
            uih::Menu change_splitter_menu;
            const auto node_panel_id = p_node->m_item->get_ptr()->get_panel_guid();

            for (auto&& [index, panel] : ranges::views::enumerate(panels)) {
                if (panel.type & uie::type_splitter) {
                    change_splitter_menu.append_command(id_splitter_type_base + gsl::narrow<uint32_t>(index),
                        panel.name, {.is_radio_checked = node_panel_id == panel.id});
                }
            }

            menu.append_submenu(std::move(change_splitter_menu), L"Container type");
        }

        if (!ti.hItem) {
            uih::Menu change_base_menu;

            for (auto&& group : grouped_panels) {
                uih::Menu popup;

                for (auto&& [index, panel] : group)
                    popup.append_command(ID_REPLACE_ROOT_BASE + gsl::narrow<uint32_t>(index), panel.name);

                const auto& category = group.front().second.category;
                change_base_menu.append_submenu(std::move(popup), category);
            }

            menu.append_submenu(std::move(change_base_menu), L"Add panel");
        }

        if (ti_parent) {
            if (menu.size() > 0)
                menu.append_separator();

            if (p_parent_node && *panel_index && *panel_index > 0)
                menu.append_command(ID_MOVE_UP, L"Move up");

            if (p_parent_node && panel_index && *panel_index + 1 < p_parent_node->m_splitter->get_panel_count())
                menu.append_command(ID_MOVE_DOWN, L"Move down");
        }

        if (ti.hItem)
            menu.append_command(ID_REMOVE, L"Remove");

        const auto item_in_clipboard = splitter_utils::is_splitter_item_in_clipboard();

        if (menu.size() > 0 && (ti.hItem || item_in_clipboard))
            menu.append_separator();

        if (ti.hItem) {
            menu.append_command(ID_CUT, L"Cut");
            menu.append_command(ID_COPY, L"Copy");
        }

        if (item_in_clipboard
            && (!ti.hItem
                || (p_splitter.is_valid() && p_node->m_children.size() < p_splitter->get_maximum_panel_count()))) {
            menu.append_command(ID_PASTE, L"Paste");
        }

        menu_helpers::win32_auto_mnemonics(menu.get());

        switch (const auto cmd = menu.run(wnd, pt)) {
        case ID_REMOVE:
            if (ti_parent || !p_splitter.is_valid()
                || cui::dark::modal_info_box(wnd, "Remove root panel",
                    "This will remove the root panel, including all children. Do you want to continue?",
                    uih::InfoBoxType::Neutral, uih::InfoBoxModalType::YesNo))
                remove_node(wnd, ti.hItem);
            break;
        case ID_MOVE_UP:
            move_item(wnd, ti.hItem, true);
            break;
        case ID_MOVE_DOWN:
            move_item(wnd, ti.hItem, false);
            break;
        case ID_CUT:
            cut_item(wnd, ti.hItem);
            break;
        case ID_COPY:
            copy_item(wnd, ti.hItem);
            break;
        case ID_PASTE:
            paste_item(wnd, ti.hItem);
            break;
        default:
            if (cmd >= id_splitter_type_base)
                switch_splitter(wnd, ti.hItem, panels[cmd - id_splitter_type_base].id);
            else if (cmd >= id_add_child_base)
                insert_item(wnd, ti.hItem, panels[cmd - id_add_child_base].id);
            else if (cmd >= ID_REPLACE_ROOT_BASE)
                change_base(wnd, panels[cmd - ID_REPLACE_ROOT_BASE].id);
            break;
        }
    }
    return true;
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

} // namespace cui::prefs
