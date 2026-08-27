#pragma once

namespace cui::utils {

enum class TrackingMode : uint32_t {
    playing_item_or_playlist_selection,
    playlist_selection,
    playing_item,
    playing_item_or_active_selection,
    active_selection,
    playlist_selection_or_playing_item,
    active_selection_or_playing_item,
};

wil::zstring_view get_tracking_mode_name(TrackingMode mode);

using ContextTrackerCallback = std::function<void()>;

class ContextTracker
    : public ui_selection_callback_impl_base_ex<ui_selection_manager_v2::flag_no_now_playing>
    , public play_callback_impl_base
    , public playlist_callback_single_impl_base {
public:
    ContextTracker(TrackingMode tracking_mode, bool track_single_item, ContextTrackerCallback callback);

    bool is_playing_item() const { return m_is_tracking_playing; }
    const metadb_handle_list& get_tracks() const { return m_tracks; }

    void activate_ui_selection_tracking() { ui_selection_callback_activate(true); }
    void deactivate_ui_selection_tracking() { ui_selection_callback_activate(false); }

    void set_tracking_mode(TrackingMode tracking_mode);

    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override;
    void on_playback_stop(play_control::t_stop_reason p_reason) noexcept override;

protected:
    void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) noexcept override;

    void on_items_added(
        size_t p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection) override;
    void on_items_removing(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override;
    void on_items_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override;
    void on_playlist_switch() noexcept override;
    void on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) noexcept override;

    void refresh_tracks();

private:
    bool tracking_prioritises_playing_item() const;
    bool tracking_falls_back_to_playing_item() const;
    bool tracking_includes_playing_item() const;
    bool tracking_includes_playlist_selection() const;
    bool tracking_includes_active_selection() const;

    metadb_handle_list get_playlist_selection() const;

    void set_selection_tracks(metadb_handle_list tracks);
    void set_tracks(metadb_handle_list tracks);

    TrackingMode m_tracking_mode;
    bool m_track_single_item{};
    ContextTrackerCallback m_callback;
    playback_control_v3::ptr m_playback_control;
    playlist_manager_v4::ptr m_playlist_manager;
    metadb_handle_ptr m_playing_item;
    metadb_handle_list m_tracks;
    metadb_handle_list m_ui_selection_tracks;
    bool m_is_tracking_playing{};
    bool m_process_playlist_items_removed{};
};

} // namespace cui::utils
