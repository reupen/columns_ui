#pragma once

#include "layout.h"
#include "config.h"
#include "splitter_utils.h"

namespace cui::prefs {

class LayoutTabNode {
public:
    using ptr = std::shared_ptr<LayoutTabNode>;

    bool have_item(const GUID& p_guid);

    uie::window_ptr m_window;
    service_ptr_t<uie::splitter_window> m_splitter;
    pfc::list_t<ptr> m_children;
    pfc::rcptr_t<uie::splitter_item_ptr> m_item;
    LayoutTabNode() : m_item(pfc::rcnew_t<uie::splitter_item_ptr>()) {}
};

class LayoutTab : public PreferencesTab {
public:
    HWND create(HWND wnd) override;
    const char* get_name() override;
    bool get_help_url(pfc::string_base& p_out) override;

private:
    static HTREEITEM insert_item_in_tree_view(
        HWND wnd_tree, const char* sz_text, HTREEITEM ti_parent = TVI_ROOT, HTREEITEM ti_after = TVI_LAST);
    static void get_panel_list(uie::window_info_list_simple& p_out);
    static HTREEITEM tree_view_get_child_by_index(HWND wnd_tv, HTREEITEM ti, unsigned index);
    static unsigned tree_view_get_child_index(HWND wnd_tv, HTREEITEM ti);
    [[nodiscard]] static std::unordered_map<HTREEITEM, LayoutTabNode::ptr> __populate_tree(
        HWND wnd_tree, LayoutTabNode::ptr p_node, HTREEITEM ti_parent, HTREEITEM ti_after = TVI_LAST);
    static void print_index_out_of_range();

    void populate_tree(HWND wnd, const uie::splitter_item_t* item, LayoutTabNode::ptr p_node,
        HTREEITEM ti_parent = TVI_ROOT, HTREEITEM ti_after = TVI_LAST);
    void populate_tree(
        HWND wnd, LayoutTabNode::ptr p_node, HTREEITEM ti_parent = TVI_ROOT, HTREEITEM ti_after = TVI_LAST);
    void remove_tree_item(HWND wnd_tv, HTREEITEM ti);
    void remove_node(HWND wnd, HTREEITEM ti);
    void insert_item(HWND wnd, HTREEITEM ti_parent, const GUID& p_guid, HTREEITEM ti_after = TVI_LAST);
    void copy_item(HWND wnd, HTREEITEM ti);
    bool _fix_single_instance_recur(uie::splitter_window_ptr& p_window);

    /**
     * \brief Fixes a copied splitter item prior to pasting
     *
     * If the copied panel is a single-instance panel, this checks if there are any other instances
     * of the panel in the layout. It also checks if the panel can be instantiated, as if it can't be
     * there's no way to know if it's a single-instance panel.
     *
     * If the copied item is a splitter panel, this also removes any child panels from the splitter
     * that don't meet the above two conditions.
     *
     * \param item      Splitter item to fix
     * \return          Whether to proceed with pasting
     */
    bool fix_paste_item(uie::splitter_item_full_v3_impl_t& item);

    void paste_item(HWND wnd, HTREEITEM ti_parent, HTREEITEM ti_after = TVI_LAST);
    void move_item(HWND wnd, HTREEITEM ti, bool up);
    void switch_splitter(HWND wnd, HTREEITEM ti, const GUID& p_guid);
    void change_base(HWND wnd, const GUID& p_guid);
    void save_item(HWND wnd, HTREEITEM ti);

    template <typename T>
    void set_item_property(HWND wnd, HTREEITEM ti, const GUID& guid, const T& val)
    {
        stream_reader_memblock_ref reader(&val, sizeof(T));
        set_item_property_stream(wnd, ti, guid, &reader);
    }

    void set_item_property_stream(HWND wnd, HTREEITEM ti, const GUID& guid, stream_reader* val);

    template <typename T>
    void set_item_property(HWND wnd, const GUID& guid, const T& val)
    {
        HWND wnd_tv = GetDlgItem(wnd, IDC_TREE);
        set_item_property(wnd, TreeView_GetSelection(wnd_tv), guid, val);
    }

    void set_item_property_stream(HWND wnd, const GUID& guid, stream_reader* val);
    void run_configure(HWND wnd);
    void initialise_tree(HWND wnd);
    void deinitialise_tree(HWND wnd);

    void apply();
    void initialise_presets(HWND wnd);
    void switch_to_preset(HWND wnd, unsigned index);

    BOOL on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    bool m_initialising{};
    bool m_initialised{};
    bool m_changed{};
    unsigned m_active_preset{};
    std::unordered_map<HTREEITEM, LayoutTabNode::ptr> m_node_map;
    LayoutTabNode::ptr m_node_root;
    PreferencesTabHelper m_helper{IDC_TITLE1};
};

} // namespace cui::prefs
