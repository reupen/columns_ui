#pragma once

#include "artwork_decoder.h"
#include "artwork_reader.h"
#include "context_tracker.h"
#include "system_appearance_manager.h"

#ifdef _DEBUG
#define ENABLE_METADATA_VIEWER
#endif

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
    , public now_playing_album_art_notify {
public:
    const GUID& get_extension_guid() const override;
    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    unsigned get_type() const override;

    static void g_on_edge_style_change();

    void on_album_art(album_art_data::ptr data) noexcept override;

    void on_artwork_loaded(bool artwork_changed);

    static void g_on_colours_change();
    static void s_on_dark_mode_status_change();
    static void s_on_use_advanced_colour_change();

    void force_reload_artwork();
    void soft_reload_selection_artwork();
    bool is_core_image_viewer_available() const;
    void open_core_image_viewer() const;
#ifdef ENABLE_METADATA_VIEWER
    bool is_show_metadata_available() const;
    void show_metadata() const;
#endif
    bool is_show_in_file_explorer_available() const;
    void show_in_file_explorer();
    bool is_copy_image_path_to_clipboard_available() const;
    void copy_image_path_to_clipboard() const;
    void show_next_artwork_type();
    void set_artwork_type_index(uint32_t index);
    void set_tracking_mode(utils::TrackingMode new_tracking_mode);
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
    void handle_wm_contextmenu(HWND wnd, POINT pt);

    void update_dxgi_output_desc();
    void update_swap_chain_buffers_size() const;
    void create_d2d_device_resources();
    void reset_d2d_device_resources(bool keep_devices = false);
    void register_occlusion_event();
    void deregister_occlusion_event();
    void create_effects();
    void refresh_image();
    void clear_image();
    void reset_effects();
    D2D1_VECTOR_2F calculate_scaling_factor(
        const wil::com_ptr<ID2D1Bitmap>& bitmap, wic::PhotoOrientation orientation) const;
    void update_transform_effect();
    void queue_decode(const album_art_data::ptr& data);
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
    wil::com_ptr<ID2D1Effect> m_transform_effect;
    std::optional<wic::PhotoOrientation> m_transform_effect_photo_orientation;
    wil::com_ptr<ID2D1Effect> m_output_effect;
    wil::unique_hpowernotify m_power_notify_handle;
    std::optional<DWORD> m_occlusion_status_event_cookie;
    bool m_is_occlusion_status_timer_active{};

    mmh::EventToken::Ptr m_use_hardware_acceleration_change_token;
    mmh::EventToken::Ptr m_display_change_token;
    mmh::EventToken::Ptr m_metadb_io_change_token;
    std::shared_ptr<ArtworkReaderManager> m_artwork_reader;
    ArtworkDecoder m_artwork_decoder;
    std::optional<std::jthread> m_show_in_explorer_thread;
    uint32_t m_selected_artwork_type_index{};
    std::optional<uint32_t> m_artwork_type_override_index{};
    utils::TrackingMode m_tracking_mode;
    bool m_preserve_aspect_ratio{true};
    bool m_artwork_type_locked{};
    bool m_dynamic_artwork_pending{};
    bool m_using_flip_model_swap_chain{};
    bool m_transform_effect_needs_updating{};

    std::optional<utils::ContextTracker> m_context_tracker;
    metadb_handle_ptr m_current_track;

    static std::vector<ArtworkPanel*> g_windows;
};

} // namespace cui::artwork_panel
