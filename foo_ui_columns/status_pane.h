#pragma once
#include "volume.h"

namespace cui::status_pane {

extern fbh::ConfigString status_pane_script;
extern ConfigMenuItem double_click_action;

class StatusPane
    : playlist_callback_single
    , play_callback {
    class StatusPaneVolumeBarAttributes {
    public:
        static const TCHAR* get_class_name() { return _T("volume_toolbar_pain"); }
        static bool get_show_caption() { return false; }
        static COLORREF get_background_colour() { return -1; /*RGB(230,230,255);*/ }
    };
    VolumeBar<false, false, StatusPaneVolumeBarAttributes> m_volume_control;

    /** PLAYLIST CALLBACKS */
    void on_items_added(size_t p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const bit_array& p_selection) noexcept override
    {
        on_playlist_change();
    }
    void on_items_reordered(const size_t* p_order, size_t p_count) override {}
    void on_items_removing(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override {}
    void on_items_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) noexcept override
    {
        on_playlist_change();
    }
    void on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) noexcept override
    {
        on_playlist_change();
    }
    void on_item_focus_change(size_t p_from, size_t p_to) override {}
    void on_items_modified(const bit_array& p_mask) override {}
    void on_items_modified_fromplayback(const bit_array& p_mask, play_control::t_display_level p_level) override {}
    void on_items_replaced(const bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override
    {
    }
    void on_item_ensure_visible(size_t p_idx) override {}

    void on_playlist_switch() noexcept override { on_playlist_change(); }
    void on_playlist_renamed(const char* p_new_name, size_t p_new_name_len) override {}
    void on_playlist_locked(bool p_locked) override {}

    void on_default_format_changed() override {}
    void on_playback_order_changed(size_t p_new_index) override {}

    /** PLAY CALLBACKS */
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) noexcept override
    {
        m_track_label = "Opening...";
        invalidate_all();
    }

    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override
    {
        // Note: On start-up, playback can actually be paused when we get here
        // (and on_playback_pause() won't be called).
        update_playback_status_text();
        update_playing_text();
        invalidate_all();
    }

    void on_playback_stop(play_control::t_stop_reason p_reason) noexcept override
    {
        if (p_reason != playback_control::stop_reason_shutting_down) {
            m_track_label = "";
            update_playing_text();
            invalidate_all();
        }
    }

    void on_playback_seek(double p_time) noexcept override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_playback_pause(bool p_state) noexcept override
    {
        m_track_label = p_state ? "Paused:" : "Playing:";
        update_playing_text();
        invalidate_all();
    }
    void on_playback_edited(metadb_handle_ptr p_track) noexcept override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_playback_dynamic_info(const file_info& p_info) noexcept override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_playback_dynamic_info_track(const file_info& p_info) noexcept override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_playback_time(double p_time) noexcept override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_volume_change(float p_new_val) override {}

    /** PRIVATE FUNCTIONS */
    void update_playlist_data() { get_length_data(m_selection, m_item_count, m_length_text); }
    void on_playlist_change()
    {
        update_playlist_data();
        invalidate_all();
    }
    void invalidate_all(bool b_update = true) const
    {
        RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE | (b_update ? RDW_UPDATENOW : NULL));
    }
    void render_background(HDC dc, const RECT& rc);

    void update_playback_status_text();

    void update_playing_text();
    void get_length_data(bool& p_selection, size_t& p_count, pfc::string_base& p_out);

public:
    StatusPane()
    {
        m_window = std::make_unique<uie::container_window_v3>(
            uie::container_window_v3_config(L"columns_ui_status_pane_mN4A3Qy1Spk", false, CS_DBLCLKS),
            [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    }

    HWND create(HWND parent) const { return m_window->create(parent); }
    void destroy() const { m_window->destroy(); }
    HWND get_wnd() const { return m_window->get_wnd(); }

    void refresh_playing_text_section()
    {
        if (!get_wnd())
            return;

        update_playing_text();
        invalidate_all();
    }

    int get_ideal_height() const { return m_font_height * 2 + 2_spx + 4_spx + 3_spx * 2; }
    void enter_menu_mode(const char* p_text)
    {
        m_menu_active = true;
        m_menu_text = p_text;
        invalidate_all();
    }
    void exit_menu_mode()
    {
        m_menu_active = false;
        m_menu_text.reset();
        invalidate_all();
    }
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    void on_font_changed();

    static const GUID g_guid_font;

private:
    void recreate_font();

    bool m_selection{false};
    size_t m_item_count{0};
    pfc::string8 m_length_text;
    pfc::string8 m_track_label;
    pfc::string8_fast_aggressive playing1;
    pfc::string8 m_menu_text;
    bool m_menu_active{false};
    int m_font_height{};
    uih::direct_write::Context::Ptr m_direct_write_context;
    std::optional<uih::direct_write::TextFormat> m_text_format;
    HTHEME m_theme{nullptr};
    std::unique_ptr<uie::container_window_v3> m_window;
    std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;
};

extern class StatusPane g_status_pane;

} // namespace cui::status_pane
