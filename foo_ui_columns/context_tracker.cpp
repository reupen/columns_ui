#include "pch.h"

#include "context_tracker.h"

namespace cui::utils {

namespace {

const std::unordered_map<TrackingMode, wil::zstring_view> tracking_mode_labels{
    {TrackingMode::playing_item_or_active_selection, "Playing item or current selection"_zv},
    {TrackingMode::active_selection_or_playing_item, "Current selection or playing item"_zv},
    {TrackingMode::playing_item_or_playlist_selection, "Playing item or playlist selection"_zv},
    {TrackingMode::playlist_selection_or_playing_item, "Playlist selection or playing item"_zv},
    {TrackingMode::active_selection, "Current selection"_zv},
    {TrackingMode::playlist_selection, "Playlist selection"_zv},
    {TrackingMode::playing_item, "Playing item"_zv},
};

bool check_process_on_selection_changed()
{
    HWND wnd_focus = GetFocus();
    if (wnd_focus == nullptr)
        return false;

    DWORD processid = NULL;
    GetWindowThreadProcessId(wnd_focus, &processid);
    return processid == GetCurrentProcessId();
}

} // namespace

void ContextTracker::on_playback_new_track(metadb_handle_ptr p_track) noexcept
{
    if (tracking_includes_playing_item())
        m_playing_item = p_track;

    if (m_is_tracking_playing || tracking_prioritises_playing_item()
        || (tracking_falls_back_to_playing_item() && m_tracks.size() == 0)) {
        set_tracks(pfc::list_single_ref_t(p_track), true);
    }
}

void ContextTracker::on_playback_stop(play_control::t_stop_reason p_reason) noexcept
{
    if (p_reason != play_control::stop_reason_starting_another)
        m_playing_item.reset();

    if (m_is_tracking_playing && p_reason != play_control::stop_reason_starting_another
        && p_reason != play_control::stop_reason_shutting_down) {
        metadb_handle_list tracks;
        std::optional<size_t> playlist_selection_index;
        if (m_tracking_mode == TrackingMode::playing_item_or_playlist_selection) {
            std::tie(playlist_selection_index, tracks) = get_playlist_selection();
        } else if (m_tracking_mode == TrackingMode::playing_item_or_active_selection) {
            tracks = m_ui_selection_tracks;
        }
        set_tracks(std::move(tracks), false, playlist_selection_index);
    }
}

wil::zstring_view get_tracking_mode_name(TrackingMode mode)
{
    return tracking_mode_labels.at(mode);
}

ContextTracker::ContextTracker(TrackingMode tracking_mode, bool track_single_item,
    ContextTrackerCallback tracks_change_callback, ContextTrackerCallback playlist_index_change_callback)
    : play_callback_impl_base(flag_on_playback_new_track | flag_on_playback_stop)
    , playlist_callback_single_impl_base(flag_on_items_added | flag_on_items_removing | flag_on_items_removed
          | flag_on_items_reordered | flag_on_items_replaced | flag_on_items_selection_change | flag_on_playlist_switch)
    , m_tracking_mode(tracking_mode)
    , m_track_single_item(track_single_item)
    , m_tracks_change_callback(std::move(tracks_change_callback))
    , m_playlist_index_change_callback(std::move(playlist_index_change_callback))
    , m_playback_control(playback_control_v3::get())
    , m_playlist_manager(playlist_manager_v4::get())
{
    ui_selection_manager_v2::get()->get_selection(m_ui_selection_tracks, ui_selection_manager_v2::flag_no_now_playing);
    refresh_tracks();
}

std::optional<size_t> ContextTracker::get_playlist_selection_index() const
{
    if (tracking_includes_playlist_selection() && (!m_is_tracking_playing || !tracking_prioritises_playing_item()))
        return m_playlist_selection_index;

    return {};
}

void ContextTracker::set_tracking_mode(TrackingMode tracking_mode, bool notify)
{
    if (tracking_mode == m_tracking_mode)
        return;

    m_tracking_mode = tracking_mode;

    refresh_tracks();

    if (notify)
        m_tracks_change_callback();
}

void ContextTracker::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) noexcept
{
    if (!check_process_on_selection_changed())
        return;

    if (!m_track_single_item)
        m_ui_selection_tracks = p_selection;
    else if (p_selection.size() > 0)
        m_ui_selection_tracks = pfc::list_single_ref_t(p_selection[0]);
    else
        m_ui_selection_tracks.remove_all();

    if (tracking_includes_active_selection() && (!tracking_prioritises_playing_item() || !m_is_tracking_playing)) {
        set_selection_tracks(m_ui_selection_tracks);
    }
}

void ContextTracker::on_items_added(
    size_t p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection)
{
    if (!tracking_includes_playlist_selection() || (tracking_prioritises_playing_item() && m_is_tracking_playing))
        return;

    for (const auto index : ranges::views::iota(size_t{}, p_data.size())) {
        if (p_selection[index]) {
            auto [playlist_index, tracks] = get_playlist_selection();
            set_selection_tracks(std::move(tracks), playlist_index, true);
            return;
        }
    }

    if (!m_playlist_selection_index)
        return;

    const auto new_index = *m_playlist_selection_index + (p_base <= *m_playlist_selection_index ? p_data.size() : 0);
    set_playlist_selection_index(new_index, true);
}

void ContextTracker::on_items_removing(const bit_array& p_mask, size_t p_old_count, size_t p_new_count)
{
    m_process_playlist_items_removed = false;

    if (!tracking_includes_playlist_selection() || (tracking_prioritises_playing_item() && m_is_tracking_playing))
        return;

    m_playlist_manager->activeplaylist_enum_items(
        [&](size_t index, const metadb_handle_ptr& track, bool is_selected) {
            m_process_playlist_items_removed = true;
            return !is_selected;
        },
        p_mask);
}

void ContextTracker::on_items_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count)
{
    if (m_process_playlist_items_removed) {
        m_process_playlist_items_removed = false;

        if (!tracking_includes_playlist_selection() || (tracking_prioritises_playing_item() && m_is_tracking_playing))
            return;

        auto [playlist_index, tracks] = get_playlist_selection();
        set_selection_tracks(std::move(tracks), playlist_index, true);
        return;
    }

    if (!m_playlist_selection_index)
        return;

    assert(*m_playlist_selection_index < p_old_count);

    if (*m_playlist_selection_index >= p_old_count) {
        set_playlist_selection_index({}, true);
        return;
    }

    if (p_mask[*m_playlist_selection_index]) {
        auto [playlist_selection_index, _] = get_playlist_selection(true);
        set_playlist_selection_index(playlist_selection_index, true);
        return;
    }

    auto new_playlist_selection_index = m_playlist_selection_index;

    for (const auto index : ranges::views::iota(size_t{}, *new_playlist_selection_index)) {
        if (p_mask[index])
            --*new_playlist_selection_index;
    }

    set_playlist_selection_index(new_playlist_selection_index, true);
}

void ContextTracker::on_items_reordered(const t_size* p_order, t_size p_count)
{
    if (!tracking_includes_playlist_selection() || (tracking_prioritises_playing_item() && m_is_tracking_playing))
        return;

    auto [playlist_selection_index, tracks] = get_playlist_selection();
    set_selection_tracks(std::move(tracks), playlist_selection_index);
}

void ContextTracker::on_items_replaced(
    const bit_array& p_mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data)
{
    if (!tracking_includes_playlist_selection() || (tracking_prioritises_playing_item() && m_is_tracking_playing))
        return;

    if (m_playlist_selection_index && m_track_single_item && !p_mask[*m_playlist_selection_index])
        return;

    auto [playlist_selection_index, tracks] = get_playlist_selection();
    set_selection_tracks(std::move(tracks), playlist_selection_index);
}

void ContextTracker::on_playlist_switch() noexcept
{
    if (!tracking_includes_playlist_selection() || (tracking_prioritises_playing_item() && m_is_tracking_playing))
        return;

    auto [playlist_selection_index, tracks] = get_playlist_selection();
    set_selection_tracks(std::move(tracks), playlist_selection_index, true);
}

void ContextTracker::on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) noexcept
{
    if (!tracking_includes_playlist_selection() || (tracking_prioritises_playing_item() && m_is_tracking_playing))
        return;

    auto [playlist_selection_index, tracks] = get_playlist_selection();
    set_selection_tracks(std::move(tracks), playlist_selection_index);
}

void ContextTracker::refresh_tracks()
{
    metadb_handle_list tracks;

    m_is_tracking_playing = false;
    m_playlist_selection_index.reset();
    std::optional<size_t> playlist_selection_index;

    if (tracking_includes_playlist_selection()) {
        std::tie(playlist_selection_index, tracks) = get_playlist_selection();
    } else if (tracking_includes_active_selection()) {
        tracks = m_ui_selection_tracks;
    }

    if (tracking_includes_playing_item())
        m_playback_control->get_now_playing(m_playing_item);

    if ((tracking_prioritises_playing_item() || (tracking_falls_back_to_playing_item() && tracks.size() == 0))
        && m_playback_control->is_playing()) {
        tracks = pfc::list_single_ref_t(m_playing_item);
        m_is_tracking_playing = true;
    }

    m_tracks = std::move(tracks);

    if (!m_is_tracking_playing)
        m_playlist_selection_index = playlist_selection_index;
}

bool ContextTracker::tracking_prioritises_playing_item() const
{
    return m_tracking_mode == TrackingMode::playing_item_or_playlist_selection
        || m_tracking_mode == TrackingMode::playing_item_or_active_selection
        || m_tracking_mode == TrackingMode::playing_item;
}

bool ContextTracker::tracking_falls_back_to_playing_item() const
{
    return m_tracking_mode == TrackingMode::playlist_selection_or_playing_item
        || m_tracking_mode == TrackingMode::active_selection_or_playing_item;
}

bool ContextTracker::tracking_includes_playing_item() const
{
    return m_tracking_mode == TrackingMode::playlist_selection_or_playing_item
        || m_tracking_mode == TrackingMode::active_selection_or_playing_item
        || m_tracking_mode == TrackingMode::playing_item_or_playlist_selection
        || m_tracking_mode == TrackingMode::playing_item_or_active_selection
        || m_tracking_mode == TrackingMode::playing_item;
}

bool ContextTracker::tracking_includes_playlist_selection() const
{
    return m_tracking_mode == TrackingMode::playing_item_or_playlist_selection
        || m_tracking_mode == TrackingMode::playlist_selection
        || m_tracking_mode == TrackingMode::playlist_selection_or_playing_item;
}

bool ContextTracker::tracking_includes_active_selection() const
{
    return m_tracking_mode == TrackingMode::playing_item_or_active_selection
        || m_tracking_mode == TrackingMode::active_selection
        || m_tracking_mode == TrackingMode::active_selection_or_playing_item;
}

std::tuple<std::optional<size_t>, metadb_handle_list> ContextTracker::get_playlist_selection(bool index_only) const
{
    metadb_handle_list tracks;
    std::optional<size_t> playlist_selection_index{};

    if (m_track_single_item) {
        m_playlist_manager->activeplaylist_enum_items(
            [&](size_t index, const metadb_handle_ptr& item, bool is_selected) {
                if (is_selected) {
                    tracks.add_item(item);
                    playlist_selection_index = index;
                }

                return !is_selected;
            },
            bit_array_true());
    } else if (!index_only) {
        m_playlist_manager->activeplaylist_get_selected_items(tracks);
    }

    return std::make_tuple(playlist_selection_index, std::move(tracks));
}

void ContextTracker::set_selection_tracks(
    metadb_handle_list tracks, std::optional<size_t> playlist_selection_index, bool is_playlist_modification)
{
    if (tracking_falls_back_to_playing_item() && tracks.size() == 0) {
        const auto is_playing = m_playback_control->is_playing();

        if (is_playing && !m_is_tracking_playing) {
            set_tracks(pfc::list_single_ref_t(m_playing_item), true, {}, is_playlist_modification);
        }

        if (m_is_tracking_playing)
            return;
    }

    set_tracks(std::move(tracks), false, playlist_selection_index, is_playlist_modification);
}

void ContextTracker::set_tracks(metadb_handle_list tracks, bool is_tracking_playing,
    std::optional<size_t> playlist_selection_index, bool is_playlist_modification)
{
    const auto old_is_tracking_playing = m_is_tracking_playing;
    m_is_tracking_playing = is_tracking_playing;

    if (!is_tracking_playing && !old_is_tracking_playing && m_track_single_item && tracks == m_tracks) {
        set_playlist_selection_index(playlist_selection_index, is_playlist_modification);
        return;
    }

    m_tracks = std::move(tracks);
    m_playlist_selection_index = !is_tracking_playing ? playlist_selection_index : std::nullopt;
    m_tracks_change_callback();
}

void ContextTracker::set_playlist_selection_index(
    std::optional<size_t> playlist_selection_index, bool is_playlist_modification)
{
    const auto old_playlist_selection_index = std::exchange(m_playlist_selection_index, playlist_selection_index);

    if (!m_playlist_index_change_callback
        || (!is_playlist_modification && old_playlist_selection_index == playlist_selection_index))
        return;

    if (!m_playlist_selection_index || !tracking_includes_playlist_selection()
        || (m_is_tracking_playing && tracking_prioritises_playing_item()))
        return;

    m_playlist_index_change_callback();
}

} // namespace cui::utils
