#pragma once

#include "list_view_panel.h"

namespace cui::panels::filter {

class AppearanceClient : public colours::client {
public:
    static constexpr GUID id{0x4d6774af, 0xc292, 0x44ac, {0x8a, 0x8f, 0x3b, 0x8, 0x55, 0xdc, 0xbd, 0xf4}};

    const GUID& get_client_guid() const override { return id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Filter panel"; }
    uint32_t get_supported_colours() const override { return colours::colour_flag_all; }
    uint32_t get_supported_bools() const override
    {
        return colours::bool_flag_use_custom_active_item_frame | colours::bool_flag_dark_mode_enabled;
    }
    bool get_themes_supported() const override { return true; }
    void on_colour_changed(uint32_t mask) const override;
    void on_bool_changed(uint32_t mask) const override;
};

class Field {
public:
    pfc::string8 m_name;
    pfc::string8 m_field;
    bool m_last_sort_direction{};
};

class Node {
public:
    using Ptr = std::shared_ptr<Node>;

    metadb_handle_list_t<pfc::alloc_fast_aggressive> m_handles;
    std::wstring m_value;
    bool m_handles_sorted{false};

    void ensure_handles_sorted();
    void remove_handles(metadb_handle_list_cref to_remove);

    static int g_compare(const Node::Ptr& i1, const WCHAR* i2);
    static int g_compare_ptr_with_node(const Node::Ptr& i1, const Node::Ptr& i2);
};

class FieldData {
public:
    bool m_use_script{};
    bool m_last_sort_direction{};
    pfc::string8 m_script;
    pfc::string8 m_name;
    std::vector<pfc::string8> m_fields;

    bool is_empty() const { return !m_use_script && m_fields.empty(); }
    void reset() { *this = FieldData(); }
};

class DataEntry {
public:
    metadb_handle_ptr m_handle;
    pfc::array_t<WCHAR> m_text;

    bool m_same_as_next{};

    static int g_compare(const DataEntry& i1, const DataEntry& i2)
    {
        return StrCmpLogicalW(i1.m_text.get_ptr(), i2.m_text.get_ptr());
    }
};

class FilterStream {
public:
    using self_t = FilterStream;
    using ptr = std::shared_ptr<self_t>;
    /** Unordered */
    pfc::ptr_list_t<class FilterPanel> m_windows;

    bool m_source_overriden{false};
    metadb_handle_list m_source_handles;

    bool is_visible();
};

class FilterPanel
    : public ListViewPanelBase<AppearanceClient, uie::window>
    , fbh::LibraryCallback {
    friend class FilterSearchToolbar;

public:
    FilterPanel() : ListViewPanelBase(std::make_unique<uih::lv::DefaultRenderer>(true)) {}

    enum {
        TIMER_QUERY = TIMER_BASE
    };

    enum {
        config_version_current = 1
    };

    enum Action {
        action_send_to_autosend,
        action_send_to_autosend_play,
        action_send_to_new,
        action_send_to_new_play,
        action_add_to_active,
    };

    static size_t g_get_stream_index_by_window(const uie::window_ptr& ptr);
    static void g_on_orderedbysplitters_change();
    static void g_on_fields_change();
    static size_t g_get_field_index_by_name(const char* p_name);
    static void g_on_field_title_change(const char* p_old, const char* p_new);
    static void g_on_vertical_item_padding_change();
    static void g_on_show_column_titles_change();
    static void g_on_allow_sorting_change();
    static void g_on_show_sort_indicators_change();
    static void g_on_field_query_change(const Field& field);
    static void g_on_showemptyitems_change(bool b_val, bool update_filters = true);
    static void g_on_edgestyle_change();
    static void s_on_dark_mode_status_change();
    static void g_on_font_items_change();
    static void g_on_font_header_change();
    static void g_redraw_all();
    static void g_on_new_field(const Field& field);
    static void g_on_fields_swapped(size_t index_1, size_t index_2);
    static void g_on_field_removed(size_t index);

    ~FilterPanel() = default;

    bool is_visible() const
    {
        RECT rc;
        return get_wnd() && IsWindowVisible(get_wnd()) && GetClientRect(get_wnd(), &rc) && wil::rect_height(rc) > 0;
    }

    const GUID& get_extension_guid() const override;
    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    unsigned get_type() const override;
    void set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort) override;
    void get_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    FilterStream::ptr m_stream;

private:
    static const GUID g_extension_guid;
    static pfc::list_t<FilterStream::ptr> g_streams;
    static std::vector<FilterPanel*> g_windows;
    static pfc::list_t<FieldData> g_field_data;

    FieldData m_field_data;

    static void g_create_field_data(const Field& field, FieldData& p_out);
    static void g_load_fields();
    static void g_update_subsequent_filters(const pfc::list_base_const_t<FilterPanel*>& windows, size_t index,
        bool b_check_needs_update = false, bool b_update_playlist = true);

    size_t get_field_index();
    void set_field(const FieldData& field, bool b_force = false);
    void get_windows(pfc::list_base_t<FilterPanel*>& windows);
    FilterPanel* get_next_window();

    void get_initial_handles(metadb_handle_list_t<pfc::alloc_fast_aggressive>& p_out);
    void update_subsequent_filters(bool b_allow_autosend = true);
    size_t make_data_entries(const metadb_handle_list_t<pfc::alloc_fast_aggressive>& handles,
        std::vector<DataEntry>& p_out, bool b_show_empty) const;
    [[nodiscard]] std::vector<DataEntry> make_data_entries_using_script_fb2k_v2(
        const metadb_handle_list_t<pfc::alloc_fast_aggressive>& handles, bool b_show_empty) const;
    [[nodiscard]] std::vector<DataEntry> make_data_entries_using_script_fb2k_v1(
        const metadb_handle_list_t<pfc::alloc_fast_aggressive>& handles, bool b_show_empty) const;
    [[nodiscard]] std::vector<DataEntry> make_data_entries_using_metadata_fb2k_v2(
        const metadb_handle_list_t<pfc::alloc_fast_aggressive>& handles, bool b_show_empty) const;
    [[nodiscard]] std::vector<DataEntry> make_data_entries_using_metadata_fb2k_v1(
        const metadb_handle_list_t<pfc::alloc_fast_aggressive>& handles, bool b_show_empty) const;
    void populate_list(const metadb_handle_list_t<pfc::alloc_fast>& handles);
    void populate_list_from_chain(const metadb_handle_list_t<pfc::alloc_fast>& handles, bool b_last_in_chain);
    void refresh(bool b_allow_autosend = true);
    void refresh_groups();
    void refresh_columns();
    void refresh_stream();

    void get_selection_handles(
        metadb_handle_list_t<pfc::alloc_fast_aggressive>& p_out, bool fallback = true, bool b_sort = false);
    bool get_nothing_or_all_node_selected() { return get_selection_count(1) == 0 || get_item_selected(0); }
    void do_selection_action(Action action = action_send_to_autosend);
    void do_items_action(const bit_array& p_nodes, Action action = action_send_to_autosend);
    void send_results_to_playlist(bool b_play = false);

    void update_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& p_data);
    void add_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& p_data);
    void remove_nodes(metadb_handle_list_t<pfc::alloc_fast_aggressive>& p_data);
    void update_first_node_text(bool b_update = false);

    void on_items_added(const pfc::list_base_const_t<metadb_handle_ptr>& p_data) override;
    void on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr>& p_data) override;
    void on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr>& p_data) override;

    void execute_default_action(size_t index, size_t column, bool b_keyboard, bool b_ctrl) override;
    size_t get_highlight_item() override;
    void move_selection(int delta) override {}
    void notify_update_item_data(size_t index) override;
    bool notify_on_middleclick(bool on_item, size_t index) override;
    void notify_on_selection_change(
        const bit_array& p_affected, const bit_array& p_status, notification_source_t p_notification_source) override;
    bool notify_before_create_inline_edit(
        const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse) override;
    bool notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
        pfc::string_base& p_text, size_t& p_flags, mmh::ComPtr<IUnknown>& pAutocompleteEntries) override;
    void notify_save_inline_edit(const char* value) override;
    void notify_exit_inline_edit() override;
    void notify_on_set_focus(HWND wnd_lost) override;
    void notify_on_kill_focus(HWND wnd_receiving) override;
    void notify_on_create() override;
    void notify_on_initialisation() override;
    void notify_on_destroy() override;
    bool notify_on_keyboard_keydown_search() override;
    bool notify_on_contextmenu_header(const POINT& pt, const HDHITTESTINFO& ht) override;
    void notify_on_menu_select(WPARAM wp, LPARAM lp) override;
    bool notify_on_contextmenu(const POINT& pt, bool from_keyboard) override;
    void notify_sort_column(size_t index, bool b_descending, bool b_selection_only) override;
    bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp) override;
    bool do_drag_drop(WPARAM wp) override;

    size_t get_drag_item_count() override { return m_drag_item_count; }

    static bool g_showemptyitems;
    static bool g_showallnode;

    ui_selection_holder::ptr m_selection_holder;
    size_t m_drag_item_count{0};
    pfc::string8 m_edit_previous_value;
    std::vector<pfc::string8> m_edit_fields;
    metadb_handle_list m_edit_handles;
    pfc::list_t<Node::Ptr> m_nodes;
    bool m_show_search{false};
    bool m_pending_sort_direction{false};
    contextmenu_manager::ptr m_contextmenu_manager;
    UINT m_contextmenu_manager_base{0};
    ui_status_text_override::ptr m_status_text_override;
};

} // namespace cui::panels::filter
