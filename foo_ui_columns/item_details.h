#pragma once

#include "d2d_utils.h"
#include "event_token.h"
#include "pch.h"
#include "file_info_reader.h"
#include "item_details_text.h"

namespace cui::panels::item_details {

extern const GUID g_guid_item_details;
extern const GUID g_guid_item_details_tracking_mode;
extern const GUID g_guid_item_details_script;
extern const GUID g_guid_item_details_font_client;
extern const GUID g_guid_item_details_colour_client;
extern const GUID g_guid_item_details_hscroll;
extern const GUID g_guid_item_details_horizontal_alignment;
extern const GUID g_guid_item_details_vertical_alignment;
extern const GUID g_guid_item_details_word_wrapping;
extern const GUID g_guid_item_details_edge_style;

extern cfg_uint cfg_item_details_tracking_mode;
extern cfg_uint cfg_item_details_edge_style;
extern cfg_bool cfg_item_details_hscroll;
extern cfg_uint cfg_item_details_horizontal_alignment;
extern cfg_uint cfg_item_details_vertical_alignment;
extern cfg_bool cfg_item_details_word_wrapping;

class TitleformatHookChangeFont : public titleformat_hook {
public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag) override;

    bool process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;

    TitleformatHookChangeFont(const LOGFONT& lf, int font_size);

private:
    pfc::string8 m_default_font_face;
    int m_default_font_size;
};

class ItemDetails
    : public uie::container_uie_window_v3
    , public ui_selection_callback
    , public play_callback
    , public playlist_callback_single
    , public metadb_io_callback_dynamic {
    inline static std::unique_ptr<uie::container_window_v3> s_message_window;

    enum {
        MSG_REFRESH = WM_USER + 2,
    };

    uie::container_window_v3_config get_window_config() override;

public:
    enum TrackingMode {
        track_auto_playlist_playing,
        track_playlist,
        track_playing,
        track_auto_selection_playing,
        track_selection,
    };

    bool s_track_mode_includes_now_playing(size_t mode);

    bool s_track_mode_includes_playlist(size_t mode);

    bool s_track_mode_includes_auto(size_t mode);

    bool s_track_mode_includes_selection(size_t mode);

    class MenuNodeTrackMode : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;
        uint32_t m_source;

    public:
        static const char* get_name(uint32_t source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeTrackMode(ItemDetails* p_wnd, uint32_t p_value);
    };

    class MenuNodeSourcePopup : public ui_extension::menu_node_popup_t {
        pfc::list_t<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        size_t get_children_count() const override;
        void get_child(size_t p_index, uie::menu_node_ptr& p_out) const override;
        explicit MenuNodeSourcePopup(ItemDetails* p_wnd);
    };

    class MenuNodeAlignment : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;
        uint32_t m_type;

    public:
        static const char* get_name(uint32_t source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeAlignment(ItemDetails* p_wnd, uint32_t p_value);
    };

    class MenuNodeAlignmentPopup : public ui_extension::menu_node_popup_t {
        pfc::list_t<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        size_t get_children_count() const override;
        void get_child(size_t p_index, uie::menu_node_ptr& p_out) const override;
        explicit MenuNodeAlignmentPopup(ItemDetails* p_wnd);
    };

    class MenuNodeOptions : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        explicit MenuNodeOptions(ItemDetails* p_wnd);
    };
    class MenuNodeHorizontalScrolling : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        explicit MenuNodeHorizontalScrolling(ItemDetails* p_wnd);
    };

    class MenuNodeWordWrap : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        explicit MenuNodeWordWrap(ItemDetails* p_wnd);
    };

    // UIE funcs
    const GUID& get_extension_guid() const override;
    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    unsigned get_type() const override;

    enum {
        stream_version_current = 2
    };
    void set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort) override;
    void get_config(stream_writer* p_writer, abort_callback& p_abort) const override;
    bool have_config_popup() const override;
    bool show_config_popup(HWND wnd_parent) override;

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override;

    // UI SEL API
    void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) noexcept override;

    // PC
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override;
    void on_playback_stop(play_control::t_stop_reason p_reason) noexcept override;
    void on_playback_seek(double p_time) noexcept override;
    void on_playback_pause(bool p_state) noexcept override;
    void on_playback_edited(metadb_handle_ptr p_track) noexcept override;
    void on_playback_dynamic_info(const file_info& p_info) noexcept override;
    void on_playback_dynamic_info_track(const file_info& p_info) noexcept override;
    void on_playback_time(double p_time) noexcept override;
    void on_volume_change(float p_new_val) override {}

    // PL
    enum {
        playlist_callback_flags = flag_on_items_selection_change | flag_on_playlist_switch
    };
    void on_playlist_switch() noexcept override;
    void on_item_focus_change(size_t p_from, size_t p_to) override {}

    void on_items_added(
        size_t p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection) override
    {
    }
    void on_items_reordered(const size_t* p_order, size_t p_count) override {}
    void on_items_removing(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override {}
    void on_items_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override {}
    void on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) noexcept override;
    void on_items_modified(const bit_array& p_mask) override {}
    void on_items_modified_fromplayback(const bit_array& p_mask, play_control::t_display_level p_level) override {}
    void on_items_replaced(const bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override
    {
    }
    void on_item_ensure_visible(size_t p_idx) override {}

    void on_playlist_renamed(const char* p_new_name, size_t p_new_name_len) override {}
    void on_playlist_locked(bool p_locked) override {}

    void on_default_format_changed() override {}
    void on_playback_order_changed(size_t p_new_index) override {}

    // ML
    void on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook) noexcept override;

    static void s_on_app_activate(bool b_activated);
    static void s_on_font_change();
    static void s_on_dark_mode_status_change();
    static void s_on_colours_change();

    void set_horizontal_alignment(uint32_t horizontal_alignment);
    void set_vertical_alignment(VerticalAlignment vertical_alignment);

    void set_edge_style(uint32_t edge_style);

    void on_edge_style_change();

    void set_script(const char* p_script);

    void set_config_wnd(HWND wnd);

    ItemDetails();

private:
    static void s_create_message_window();
    static void s_destroy_message_window();
    static int s_get_padding() { return 2_spx; }

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    void register_callback();
    void deregister_callback();
    void on_app_activate(bool b_activated);

    void set_handles(const metadb_handle_list& handles);
    void refresh_contents(bool reset_vertical_scroll_position = false, bool reset_horizontal_scroll_position = false,
        bool force_update = false);
    void request_full_file_info();
    void on_full_file_info_request_completion(std::shared_ptr<helpers::FullFileInfoRequest> request);
    void release_aborted_full_file_info_requests();
    void release_all_full_file_info_requests();

    void update_display_info();
    void reset_display_info();
    void on_tracking_mode_change();
    bool check_process_on_selection_changed();

    void scroll(INT sb, int position, bool b_absolute);

    void set_window_theme() const;
    void invalidate_all(bool b_update = true);
    void update_now();
    void create_d2d_render_target();
    void reset_d2d_device_resources();
    void register_occlusion_event();
    void deregister_occlusion_event();
    bool check_occlusion_status();

    enum class ScrollbarType {
        vertical = SB_VERT,
        horizontal = SB_HORZ,
    };

    void update_scrollbar(ScrollbarType scrollbar_type, bool reset_position);
    void update_scrollbars(bool reset_vertical_position, bool reset_horizontal_position);

    void on_size();
    void on_size(int cx, int cy);

    void recreate_text_format();
    void create_text_layout();
    void on_font_change();
    void on_colours_change();

    inline static std::vector<ItemDetails*> s_windows;

    int m_last_cx{};
    int m_last_cy{};
    ui_selection_holder::ptr m_selection_holder;
    metadb_handle_list m_handles;
    metadb_handle_list m_selection_handles;
    std::shared_ptr<file_info_impl> m_full_file_info;
    std::shared_ptr<helpers::FullFileInfoRequest> m_full_file_info_request;
    std::vector<std::shared_ptr<helpers::FullFileInfoRequest>> m_aborting_full_file_info_requests;
    bool m_full_file_info_requested{};
    bool m_callback_registered{};
    bool m_nowplaying_active{};
    uint32_t m_tracking_mode;

    uih::direct_write::Context::Ptr m_direct_write_context;
    std::optional<uih::direct_write::TextFormat> m_text_format;
    std::optional<uih::direct_write::TextLayout> m_text_layout;
    wil::com_ptr<IDXGIFactory2> m_dxgi_factory;
    d2d::MainThreadD2D1Factory m_d2d_factory;
    wil::com_ptr<ID2D1HwndRenderTarget> m_d2d_render_target;
    wil::com_ptr<ID2D1SolidColorBrush> m_d2d_text_brush;
    std::unordered_map<COLORREF, wil::com_ptr<ID2D1SolidColorBrush>> m_d2d_brush_cache;
    EventToken::Ptr m_use_hardware_acceleration_change_token;
    bool m_is_occlusion_status_timer_active{};
    std::optional<DWORD> m_occlusion_status_event_cookie;

    std::optional<RECT> m_text_rect{};

    pfc::string8 m_script;
    std::wstring m_formatted_text;
    titleformat_object::ptr m_to;

    uint32_t m_horizontal_alignment{};
    VerticalAlignment m_vertical_alignment{};
    uint32_t m_edge_style{};
    bool m_hscroll{};
    bool m_word_wrapping{};
    bool m_is_updating_scroll_bars{};

    HWND m_wnd_config{};
};

class ItemDetailsConfig {
public:
    pfc::string8 m_script;
    uint32_t m_edge_style{};
    uint32_t m_horizontal_alignment{};
    VerticalAlignment m_vertical_alignment{};
    LOGFONT m_code_generator_selected_font{};
    bool m_modal{};
    bool m_timer_active{};
    service_ptr_t<ItemDetails> m_this;
    HWND m_wnd{};
    HWND m_format_code_generator_wnd{};

    enum {
        timer_id = 100
    };

    ItemDetailsConfig(const char* p_text, uint32_t edge_style, uint32_t halign, VerticalAlignment valign);

    ItemDetailsConfig(const ItemDetailsConfig&) = default;
    ItemDetailsConfig(ItemDetailsConfig&&) = default;

    bool run_modal(HWND wnd);
    void run_modeless(HWND wnd, ItemDetails* p_this) &&;

private:
    void kill_timer();
    void start_timer();
    void on_timer();
    void open_format_code_generator();
    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
};

} // namespace cui::panels::item_details
