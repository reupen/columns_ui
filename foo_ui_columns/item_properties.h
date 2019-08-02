#pragma once

#include "list_view_panel.h"

struct InfoSection {
    t_uint32 id;
    const char* name;
    InfoSection(t_uint32 p_id, const char* p_name) : id(p_id), name(p_name){};
};

class Field {
public:
    pfc::string8 m_name;
    pfc::string8 m_name_friendly;

    Field(const char* friendly, const char* field) : m_name(field), m_name_friendly(friendly){};
    Field() = default;
};

class FieldsList : public uih::ListView {
public:
    t_size m_edit_index, m_edit_column;
    pfc::list_t<Field>& m_fields;
    FieldsList(pfc::list_t<Field>& p_fields);

    void get_insert_items(t_size base, t_size count, pfc::list_t<uih::ListView::InsertItem>& items);
    void notify_on_create() override;
    bool notify_before_create_inline_edit(
        const pfc::list_base_const_t<t_size>& indices, unsigned column, bool b_source_mouse) override;
    bool notify_create_inline_edit(const pfc::list_base_const_t<t_size>& indices, unsigned column,
        pfc::string_base& p_text, t_size& p_flags, mmh::ComPtr<IUnknown>& pAutocompleteEntries) override;
    void notify_save_inline_edit(const char* value) override;

private:
};

t_size g_get_info_section_id_by_name(const char* p_name);

extern const InfoSection g_info_sections[5];

class ItemPropertiesColoursClient : public cui::colours::client {
public:
    static const GUID g_guid;

    const GUID& get_client_guid() const override { return g_guid; };

    void get_name(pfc::string_base& p_out) const override { p_out = "Item properties"; };

    t_size get_supported_colours() const override { return cui::colours::colour_flag_all; }; // bit-mask

    t_size get_supported_bools() const override
    {
        return cui::colours::bool_flag_use_custom_active_item_frame;
    }; // bit-mask
    bool get_themes_supported() const override { return true; };

    void on_colour_changed(t_size mask) const override;

    void on_bool_changed(t_size mask) const override{};
};

class ItemPropertiesConfig {
public:
    pfc::list_t<Field> m_fields;
    t_size m_edge_style;
    t_uint32 m_info_sections_mask;
    bool m_show_columns, m_show_groups;

    bool m_initialising;
    static BOOL CALLBACK g_DialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    ItemPropertiesConfig(pfc::list_t<Field> p_fields, t_size edge_style, t_uint32 info_sections_mask,
        bool b_show_columns, bool b_show_groups);

    bool run_modal(HWND wnd);

private:
    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    FieldsList m_field_list;
};

class ItemProperties
    : public ListViewPanelBase<ItemPropertiesColoursClient, uie::window>
    , public ui_selection_callback
    , public play_callback
    , public metadb_io_callback_dynamic {
    class MessageWindow : public ui_helpers::container_window {
        class_data& get_class_data() const override;
        LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    };

    static MessageWindow g_message_window;

    enum {
        MSG_REFRESH = WM_USER + 2,
    };

public:
    enum TrackingMode {
        track_selection,
        track_nowplaying,
        track_automatic,
    };
    // UIE funcs
    const GUID& get_extension_guid() const override;
    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    unsigned get_type() const override;

    enum { config_version_current = 5 };
    void set_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort) override;
    void get_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    bool have_config_popup() const override;
    bool show_config_popup(HWND wnd_parent) override;
    class MenuNodeTrackMode : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemProperties> p_this;
        t_size m_source;

    public:
        static const char* get_name(t_size source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeTrackMode(ItemProperties* p_wnd, t_size p_value);
        ;
    };
    class ModeNodeAutosize : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemProperties> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        ModeNodeAutosize(ItemProperties* p_wnd);
        ;
    };
    class MenuNodeSourcePopup : public ui_extension::menu_node_popup_t {
        pfc::list_t<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        unsigned get_children_count() const override;
        void get_child(unsigned p_index, uie::menu_node_ptr& p_out) const override;
        MenuNodeSourcePopup(ItemProperties* p_wnd);
        ;
    };

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override;

    // NGLV
    void notify_on_initialisation() override;
    void notify_on_create() override;
    void notify_on_destroy() override;
    void notify_on_set_focus(HWND wnd_lost) override;
    void notify_on_kill_focus(HWND wnd_receiving) override;
    bool notify_on_keyboard_keydown_copy() override;
    bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp) override;
    void notify_on_column_size_change(t_size index, int new_width) override;
    bool notify_before_create_inline_edit(
        const pfc::list_base_const_t<t_size>& indices, unsigned column, bool b_source_mouse) override;
    static void g_print_field(const char* field, const file_info& p_info, pfc::string_base& p_out);
    bool notify_create_inline_edit(const pfc::list_base_const_t<t_size>& indices, unsigned column,
        pfc::string_base& p_text, t_size& p_flags, mmh::ComPtr<IUnknown>& pAutocompleteEntries) override;
    void notify_save_inline_edit(const char* value) override;

    // UI SEL API
    void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) override;

    // PC
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) override;
    void on_playback_stop(play_control::t_stop_reason p_reason) override;
    void on_playback_seek(double p_time) override {}
    void on_playback_pause(bool p_state) override {}
    void on_playback_edited(metadb_handle_ptr p_track) override {}
    void on_playback_dynamic_info(const file_info& p_info) override {}
    void on_playback_dynamic_info_track(const file_info& p_info) override {}
    void on_playback_time(double p_time) override {}
    void on_volume_change(float p_new_val) override {}

    void on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook) override;

    static void g_on_app_activate(bool b_activated);
    static void g_redraw_all();
    static void g_on_font_items_change();
    static void g_on_font_header_change();
    static void g_on_font_groups_change();

    ItemProperties();

private:
    void register_callback();
    void deregister_callback();
    void on_app_activate(bool b_activated);
    void refresh_contents();
    void on_tracking_mode_change();
    bool check_process_on_selection_changed();

    static std::vector<ItemProperties*> g_windows;

    ui_selection_holder::ptr m_selection_holder;
    metadb_handle_list m_handles, m_selection_handles;
    pfc::list_t<Field> m_fields;
    bool m_callback_registered{false};
    t_size m_tracking_mode;

    t_uint32 m_info_sections_mask;
    bool m_show_column_titles, m_show_group_titles;

    bool m_autosizing_columns{true};
    uih::IntegerAndDpi<int32_t> m_column_name_width{80};
    uih::IntegerAndDpi<int32_t> m_column_field_width{125};

    t_size m_edge_style;
    t_size m_edit_column, m_edit_index;
    pfc::string8 m_edit_field;
    metadb_handle_list m_edit_handles;
};
