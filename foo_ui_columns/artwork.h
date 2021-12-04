#pragma once

#include "artwork_helpers.h"

namespace cui::artwork_panel {

class ArtworkPanel
    : public uie::container_ui_extension_t<>
    , public now_playing_album_art_notify
    , public play_callback
    , public playlist_callback_single
    , public ui_selection_callback {
public:
    class CompletionNotifyForwarder : public completion_notify {
    public:
        void on_completion(unsigned p_code) override;
        CompletionNotifyForwarder(ArtworkPanel* p_this);

    private:
        service_ptr_t<ArtworkPanel> m_this;
    };

    const GUID& get_extension_guid() const override;
    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    unsigned get_type() const override;

    static void g_on_edge_style_change();

    void on_album_art(album_art_data::ptr data) override;

    void on_playback_new_track(metadb_handle_ptr p_track) override;
    void on_playback_stop(play_control::t_stop_reason p_reason) override;

    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_seek(double p_time) override {}
    void on_playback_pause(bool p_state) override {}
    void on_playback_edited(metadb_handle_ptr p_track) override {}
    void on_playback_dynamic_info(const file_info& p_info) override {}
    void on_playback_dynamic_info_track(const file_info& p_info) override {}
    void on_playback_time(double p_time) override {}
    void on_volume_change(float p_new_val) override {}

    enum {
        playlist_callback_flags
        = playlist_callback_single::flag_on_items_selection_change | playlist_callback_single::flag_on_playlist_switch
    };
    void on_playlist_switch() override;
    void on_item_focus_change(t_size p_from, t_size p_to) override{};

    void on_items_added(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const pfc::bit_array& p_selection) override
    {
    }
    void on_items_reordered(const t_size* p_order, t_size p_count) override {}
    void on_items_removing(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override {}
    void on_items_removed(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override {}
    void on_items_selection_change(const pfc::bit_array& p_affected, const pfc::bit_array& p_state) override;
    void on_items_modified(const pfc::bit_array& p_mask) override {}
    void on_items_modified_fromplayback(const pfc::bit_array& p_mask, play_control::t_display_level p_level) override {}
    void on_items_replaced(const pfc::bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override
    {
    }
    void on_item_ensure_visible(t_size p_idx) override {}

    void on_playlist_renamed(const char* p_new_name, t_size p_new_name_len) override {}
    void on_playlist_locked(bool p_locked) override {}

    void on_default_format_changed() override {}
    void on_playback_order_changed(t_size p_new_index) override {}

    void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) override;

    void on_completion(unsigned p_code);

    static void g_on_colours_change();

    void force_reload_artwork();

    ArtworkPanel();

private:
    class_data& get_class_data() const override;

    class MenuNodeTrackMode : public ui_extension::menu_node_command_t {
        service_ptr_t<ArtworkPanel> p_this;
        t_size m_source;

    public:
        static const char* get_name(t_size source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeTrackMode(ArtworkPanel* p_wnd, t_size p_value);
    };

    class MenuNodeArtworkType : public ui_extension::menu_node_command_t {
        service_ptr_t<ArtworkPanel> p_this;
        t_size m_type;

    public:
        static const char* get_name(t_size source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeArtworkType(ArtworkPanel* p_wnd, t_size p_value);
        ;
    };

    class MenuNodeSourcePopup : public ui_extension::menu_node_popup_t {
        std::vector<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        unsigned get_children_count() const override;
        void get_child(unsigned p_index, uie::menu_node_ptr& p_out) const override;
        MenuNodeSourcePopup(ArtworkPanel* p_wnd);
    };

    class MenuNodeTypePopup : public ui_extension::menu_node_popup_t {
        std::vector<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        unsigned get_children_count() const override;
        void get_child(unsigned p_index, uie::menu_node_ptr& p_out) const override;
        MenuNodeTypePopup(ArtworkPanel* p_wnd);
    };
    class MenuNodePreserveAspectRatio : public ui_extension::menu_node_command_t {
        service_ptr_t<ArtworkPanel> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodePreserveAspectRatio(ArtworkPanel* p_wnd);
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
        MenuNodeLockType(ArtworkPanel* p_wnd);
    };

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override;
    enum { current_stream_version = 3 };

    void set_config(stream_reader* p_reader, t_size size, abort_callback& p_abort) override;
    void get_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    void refresh_cached_bitmap();
    void flush_cached_bitmap();
    bool refresh_image(std::optional<size_t> artwork_type_index_override = {});
    void show_stub_image();
    void flush_image(bool invalidate = true);
    void invalidate_window() const;
    size_t get_displayed_artwork_type_index() const;

    ULONG_PTR m_gdiplus_instance{NULL};
    bool m_gdiplus_initialised{false};

    std::shared_ptr<ArtworkReaderManager> m_artwork_loader;
    std::unique_ptr<Gdiplus::Bitmap> m_image;
    wil::unique_hbitmap m_bitmap;
    size_t m_selected_artwork_type_index{0};
    std::optional<size_t> m_artwork_type_override_index{};
    size_t m_track_mode;
    bool m_preserve_aspect_ratio{true};
    bool m_lock_type{false};
    bool m_dynamic_artwork_pending{};
    metadb_handle_list m_selection_handles;

    static std::vector<ArtworkPanel*> g_windows;
};

} // namespace cui::artwork_panel
