#pragma once

#include "artwork_decoder.h"
#include "artwork_reader.h"

namespace cui::artwork_panel {

enum class ClickAction : int32_t {
    open_image_viewer,
    show_next_artwork_type,
};

extern fbh::ConfigInt32 click_action;

class ArtworkPanel
    : public uie::container_uie_window_v3
    , public now_playing_album_art_notify
    , public play_callback
    , public playlist_callback_single
    , public ui_selection_callback {
public:
    const GUID& get_extension_guid() const override;
    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    unsigned get_type() const override;

    static void g_on_edge_style_change();

    void on_album_art(album_art_data::ptr data) noexcept override;

    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override;
    void on_playback_stop(play_control::t_stop_reason p_reason) noexcept override;

    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_seek(double p_time) override {}
    void on_playback_pause(bool p_state) override {}
    void on_playback_edited(metadb_handle_ptr p_track) override {}
    void on_playback_dynamic_info(const file_info& p_info) override {}
    void on_playback_dynamic_info_track(const file_info& p_info) override {}
    void on_playback_time(double p_time) override {}
    void on_volume_change(float p_new_val) override {}

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

    void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) noexcept override;

    void on_artwork_loaded(bool artwork_changed);

    static void g_on_colours_change();
    static void s_on_dark_mode_status_change();

    void force_reload_artwork();
    bool is_core_image_viewer_available() const;
    void open_core_image_viewer() const;
    void show_next_artwork_type();
    void set_artwork_type_index(size_t index);

    ArtworkPanel();

private:
    uie::container_window_v3_config get_window_config() override;

    class MenuNodeTrackMode : public ui_extension::menu_node_command_t {
        service_ptr_t<ArtworkPanel> p_this;
        uint32_t m_source;

    public:
        static const char* get_name(uint32_t source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeTrackMode(ArtworkPanel* p_wnd, uint32_t p_value);
    };

    class MenuNodeArtworkType : public ui_extension::menu_node_command_t {
        service_ptr_t<ArtworkPanel> p_this;
        uint32_t m_type;

    public:
        static const char* get_name(uint32_t source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeArtworkType(ArtworkPanel* p_wnd, uint32_t p_value);
    };

    class MenuNodeSourcePopup : public ui_extension::menu_node_popup_t {
        std::vector<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        size_t get_children_count() const override;
        void get_child(size_t p_index, uie::menu_node_ptr& p_out) const override;
        explicit MenuNodeSourcePopup(ArtworkPanel* p_wnd);
    };

    class MenuNodeTypePopup : public ui_extension::menu_node_popup_t {
        std::vector<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        size_t get_children_count() const override;
        void get_child(size_t p_index, uie::menu_node_ptr& p_out) const override;
        explicit MenuNodeTypePopup(ArtworkPanel* p_wnd);
    };
    class MenuNodePreserveAspectRatio : public ui_extension::menu_node_command_t {
        service_ptr_t<ArtworkPanel> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        explicit MenuNodePreserveAspectRatio(ArtworkPanel* p_wnd);
    };

    class MenuNodeOptions : public ui_extension::menu_node_command_t {
    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
    };
    class MenuNodeLockType : public ui_extension::menu_node_command_t {
        service_ptr_t<ArtworkPanel> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        explicit MenuNodeLockType(ArtworkPanel* p_wnd);
    };

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override;
    enum {
        current_stream_version = 3
    };

    void request_artwork(const metadb_handle_ptr& track, bool is_from_playback = false);

    void set_config(stream_reader* p_reader, size_t size, abort_callback& p_abort) override;
    void get_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    void create_d2d_render_target();
    void refresh_image();
    void clear_image();
    void show_stub_image();
    void invalidate_window() const;
    size_t get_displayed_artwork_type_index() const;

    wil::com_ptr<ID2D1Factory> m_d2d_factory;
    wil::com_ptr<ID2D1HwndRenderTarget> m_d2d_render_target;

    std::shared_ptr<ArtworkReaderManager> m_artwork_reader;
    ArtworkDecoder m_artwork_decoder;
    size_t m_selected_artwork_type_index{0};
    std::optional<size_t> m_artwork_type_override_index{};
    uint32_t m_track_mode;
    bool m_preserve_aspect_ratio{true};
    bool m_artwork_type_locked{false};
    bool m_dynamic_artwork_pending{};
    metadb_handle_list m_selection_handles;

    static std::vector<ArtworkPanel*> g_windows;
};

} // namespace cui::artwork_panel
