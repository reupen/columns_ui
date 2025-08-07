#pragma once

#include "artwork_decoder.h"
#include "artwork_reader.h"
#include "system_appearance_manager.h"

namespace cui::artwork_panel {

enum class ClickAction : int32_t {
    open_image_viewer,
    show_next_artwork_type,
    show_in_file_explorer,
};

enum class ColourManagementMode : int32_t {
    Legacy,
    Advanced,
};

extern fbh::ConfigInt32 click_action;
extern fbh::ConfigInt32 colour_management_mode;

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
    static void s_on_use_advanced_colour_change();

    void force_reload_artwork();
    bool is_core_image_viewer_available() const;
    void open_core_image_viewer() const;
    bool is_show_in_file_explorer_available() const;
    void show_in_file_explorer();
    void show_next_artwork_type();
    void set_artwork_type_index(uint32_t index);
    void set_tracking_mode(uint32_t new_tracking_mode);
    void toggle_preserve_aspect_ratio();
    void toggle_lock_artwork_type();

    ArtworkPanel();

private:
    uie::container_window_v3_config get_window_config() override;

    class MenuNodeSourcePopup : public ui_extension::menu_node_popup_t {
    public:
        explicit MenuNodeSourcePopup(service_ptr_t<ArtworkPanel> p_wnd);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        size_t get_children_count() const override;
        void get_child(size_t p_index, uie::menu_node_ptr& p_out) const override;

    private:
        std::vector<ui_extension::menu_node_ptr> m_items;
    };

    class MenuNodeTypePopup : public ui_extension::menu_node_popup_t {
    public:
        explicit MenuNodeTypePopup(service_ptr_t<ArtworkPanel> p_wnd);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        size_t get_children_count() const override;
        void get_child(size_t p_index, uie::menu_node_ptr& p_out) const override;

    private:
        std::vector<ui_extension::menu_node_ptr> m_items;
    };

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override;
    enum {
        current_stream_version = 3
    };

    void request_artwork(const metadb_handle_ptr& track, bool is_from_playback = false);

    void set_config(stream_reader* p_reader, size_t size, abort_callback& p_abort) override;
    void get_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    void update_dxgi_output_desc();
    void create_d2d_device_resources();
    void reset_d2d_device_resources(bool keep_devices = false);
    void create_effects();
    void refresh_image();
    void clear_image();
    void reset_effects();
    D2D1_VECTOR_2F calculate_scaling_factor(const wil::com_ptr<ID2D1Image>& image) const;
    void update_scale_effect();
    void queue_decode(const album_art_data::ptr& data);
    void show_stub_image();
    void invalidate_window() const;
    uint32_t get_displayed_artwork_type_index() const;
    bool is_advanced_colour_active() const;

    wil::com_ptr<ID2D1Factory1> m_d2d_factory;
    wil::com_ptr<ID2D1Device> m_d2d_device;
    wil::com_ptr<ID2D1DeviceContext> m_d2d_device_context;
    wil::com_ptr<ID3D11Device> m_d3d_device;
    wil::com_ptr<ID3D11DeviceContext> m_d3d_device_context;
    wil::com_ptr<IDXGIFactory2> m_dxgi_factory;
    wil::com_ptr<IDXGISwapChain1> m_dxgi_swap_chain;
    std::optional<DXGI_FORMAT> m_swap_chain_format;
    std::optional<unsigned> m_sdr_white_level;
    std::optional<DXGI_OUTPUT_DESC1> m_dxgi_output_desc;
    wil::com_ptr<ID2D1Effect> m_scale_effect;
    wil::com_ptr<ID2D1Effect> m_output_effect;

    EventToken::Ptr m_use_hardware_acceleration_change_token;
    EventToken::Ptr m_display_change_token;
    std::shared_ptr<ArtworkReaderManager> m_artwork_reader;
    ArtworkDecoder m_artwork_decoder;
    std::optional<std::jthread> m_show_in_explorer_thread;
    uint32_t m_selected_artwork_type_index{};
    std::optional<uint32_t> m_artwork_type_override_index{};
    uint32_t m_track_mode;
    bool m_preserve_aspect_ratio{true};
    bool m_artwork_type_locked{};
    bool m_dynamic_artwork_pending{};
    bool m_using_flip_model_swap_chain{};
    bool m_scale_effect_needs_updating{};
    metadb_handle_list m_selection_handles;

    static std::vector<ArtworkPanel*> g_windows;
};

} // namespace cui::artwork_panel
