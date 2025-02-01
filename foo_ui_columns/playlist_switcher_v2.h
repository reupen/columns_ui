#pragma once

#include "dark_mode_dialog.h"
#include "playlist_manager_utils.h"
#include "playlist_switcher.h"
#include "list_view_panel.h"

namespace cui::panels::playlist_switcher {

constexpr GUID items_font_id = {0x70a5c273, 0x67ab, 0x4bb6, {0xb6, 0x1c, 0xf7, 0x97, 0x5a, 0x68, 0x71, 0xfd}};

class PlaylistSwitcher
    : public utils::ListViewPanelBase<PlaylistSwitcherColoursClient::id, items_font_id>
    , private playlist_callback
    , private play_callback {
    enum {
        ID_PLAY = 1,
        ID_SWITCH,
        ID_REMOVE,
        ID_RENAME,
        ID_NEW,
        ID_SAVE,
        ID_SAVE_ALL,
        ID_LOAD,
        ID_UP,
        ID_DOWN,
        ID_CUT,
        ID_COPY,
        ID_PASTE,
        ID_AUTOPLAYLIST,
        ID_RECYCLER_CLEAR,
        ID_RECYCLER_BASE
    };

    enum {
        TIMER_SWITCH = TIMER_BASE
    };

    class DropSource : public IDropSource {
        long refcount;
        service_ptr_t<PlaylistSwitcher> m_window;
        DWORD m_initial_key_state;

    public:
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject) override;
        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;
        HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;
        HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect) override;
        DropSource(PlaylistSwitcher* p_window, DWORD initial_key_state);
    };

    class DropTarget : public IDropTarget {
    public:
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObject) override;
        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;
        HRESULT STDMETHODCALLTYPE DragEnter(
            IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
        HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
        HRESULT STDMETHODCALLTYPE DragLeave() override;
        HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

        explicit DropTarget(PlaylistSwitcher* p_window);

    private:
        long drop_ref_count;
        bool m_last_rmb;
        bool m_is_playlists;
        bool m_is_accepted_type;
        service_ptr_t<PlaylistSwitcher> m_window;
        wil::com_ptr<IDataObject> m_DataObject;
        service_ptr_t<ole_interaction_v2> m_ole_api;
        service_ptr_t<playlist_manager_v4> m_playlist_api;
        wil::com_ptr<IDropTargetHelper> m_DropTargetHelper;
    };

public:
    static void g_on_font_items_change();
    static void g_on_edgestyle_change();
    static void g_on_vertical_item_padding_change();
    static void g_redraw_all();
    static void s_on_dark_mode_status_change();
    static void g_refresh_all_items();

    void destroy_switch_timer()
    {
        if (m_switch_timer_active) {
            KillTimer(get_wnd(), TIMER_SWITCH);
            m_switch_timer_active = false;
            m_switch_playlist.reset();
        }
    }
    void set_switch_timer(size_t index)
    {
        if (!m_switch_timer_active || !m_switch_playlist || m_switch_playlist->m_playlist != index) {
            if (index != m_playlist_api->get_active_playlist()) {
                destroy_switch_timer();
                m_switch_playlist = std::make_shared<playlist_position_reference_tracker>(false);
                m_switch_playlist->m_playlist = index;
                SetTimer(get_wnd(), TIMER_SWITCH, cfg_autoswitch_delay, nullptr);
                m_switch_timer_active = true;
            }
        }
    }

    bool notify_on_timer(UINT_PTR timerid) override
    {
        if (timerid == TIMER_SWITCH) {
            size_t index = m_switch_playlist ? m_switch_playlist->m_playlist : pfc_infinite;
            destroy_switch_timer();
            if (index != pfc_infinite)
                m_playlist_api->set_active_playlist(index);
            return true;
        }
        return false;
    }

    void get_insert_items(size_t base, size_t count, pfc::list_t<InsertItem>& p_out);
    void refresh_all_items();
    void refresh_items(size_t base, size_t count, bool b_update = true);
    void add_items(size_t base, size_t count);
    void refresh_columns();

    void notify_on_initialisation() override;
    void notify_on_create() override;
    void notify_on_destroy() override;

    std::unique_ptr<ListViewSearchContextBase> create_search_context() override
    {
        if (cfg_playlist_switcher_use_tagz)
            return std::make_unique<SearchContext>();

        return ListViewPanelBase::create_search_context();
    }

    void move_selection(int delta) override;

    bool notify_before_create_inline_edit(
        const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse) override;
    bool notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
        pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries) override;
    void notify_save_inline_edit(const char* value) override;

    const char* get_drag_unit_singular() const override { return "playlist"; }
    const char* get_drag_unit_plural() const override { return "playlists"; }

    bool do_drag_drop(WPARAM wp) override;

    bool notify_on_contextmenu(const POINT& pt, bool from_keyboard) override;

    bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp) override
    {
        uie::window_ptr p_this = this;
        bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
        return ret;
    }

    bool notify_on_middleclick(bool on_item, size_t index) override
    {
        if (cfg_mclick && on_item && index < m_playlist_api->get_playlist_count()) {
            m_playlist_api->remove_playlist_switch(index);
            return true;
        }
        return false;
    }

    bool notify_on_doubleleftclick_nowhere() override
    {
        m_playlist_api->set_active_playlist(
            m_playlist_api->create_playlist("Untitled", pfc_infinite, m_playlist_api->get_playlist_count()));
        return true;
    }

    bool notify_on_keyboard_keydown_remove() override
    {
        size_t index = m_playlist_api->get_active_playlist();
        if (index < m_playlist_api->get_playlist_count()) {
            pfc::string8 name;
            m_playlist_api->playlist_get_name(index, name);

            if (dark::modal_info_box(get_wnd(), "Delete playlist",
                    fmt::format("Are you sure you want to delete the \"{}\" playlist?", name.c_str()).c_str(),
                    uih::InfoBoxType::Neutral, uih::InfoBoxModalType::YesNo))
                m_playlist_api->remove_playlist_switch(index);
        }
        return true;
    }

    bool notify_on_keyboard_keydown_cut() override
    {
        size_t index = m_playlist_api->get_active_playlist();
        if (index != pfc_infinite)
            playlist_manager_utils::cut(pfc::list_single_ref_t<size_t>(index));
        return true;
    }

    bool notify_on_keyboard_keydown_copy() override
    {
        size_t index = m_playlist_api->get_active_playlist();
        if (index != pfc_infinite)
            playlist_manager_utils::copy(pfc::list_single_ref_t<size_t>(index));
        return true;
    }

    bool notify_on_keyboard_keydown_paste() override
    {
        size_t index = m_playlist_api->get_active_playlist();
        if (index == pfc_infinite)
            index = m_playlist_api->get_playlist_count();
        else
            index++;
        playlist_manager_utils::paste(get_wnd(), index);
        return true;
    }

    void execute_default_action(size_t index, size_t column, bool b_keyboard, bool b_ctrl) override
    {
        if (m_playlist_api->playlist_get_item_count(index)) {
            m_playlist_api->set_playing_playlist(index);
            play_control::get()->start();
        }
    }
    void notify_on_selection_change(
        const bit_array& p_affected, const bit_array& p_status, notification_source_t p_notification_source) override
    {
        if (p_notification_source != notification_source_rmb) {
            size_t numSelected = get_selection_count(2);

            if (numSelected == 1) {
                bit_array_bittable mask(get_item_count());
                get_selection_state(mask);
                size_t index = 0;
                size_t count = get_item_count();
                while (index < count) {
                    if (mask[index])
                        break;
                    index++;
                }
                m_playlist_api->set_active_playlist(index);
            } else // if (numSelected == 0)
                m_playlist_api->set_active_playlist(pfc_infinite);
        }
    }

    void notify_on_set_focus(HWND wnd_lost) override
    {
        m_selection_holder = ui_selection_manager::get()->acquire();
        m_selection_holder->set_playlist_tracking();
    }

    void notify_on_kill_focus(HWND wnd_receiving) override { m_selection_holder.release(); }

    size_t get_playing_playlist()
    {
        return m_playback_api->is_playing() ? m_playlist_api->get_playing_playlist() : pfc_infinite;
    }

    void on_playing_playlist_change(size_t p_playing_playlist)
    {
        size_t previous_playing = m_playing_playlist;
        m_playing_playlist = p_playing_playlist;
        if (previous_playing != pfc_infinite && previous_playing < get_item_count())
            refresh_items(previous_playing, 1);
        if (p_playing_playlist != previous_playing && p_playing_playlist != pfc_infinite
            && p_playing_playlist < get_item_count())
            refresh_items(p_playing_playlist, 1);
    }

    void refresh_playing_playlist() { m_playing_playlist = get_playing_playlist(); }

    void on_items_added(size_t p_playlist, size_t p_start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const bit_array& p_selection) noexcept override;
    void on_items_reordered(size_t p_playlist, const size_t* p_order, size_t p_count) override {}
    void on_items_removing(size_t p_playlist, const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override
    {
    }
    void on_items_removed(
        size_t p_playlist, const bit_array& p_mask, size_t p_old_count, size_t p_new_count) noexcept override;
    void on_items_selection_change(size_t p_playlist, const bit_array& p_affected, const bit_array& p_state) override {}
    void on_item_focus_change(size_t p_playlist, size_t p_from, size_t p_to) override {}

    void on_items_modified(size_t p_playlist, const bit_array& p_mask) noexcept override;
    void on_items_modified_fromplayback(
        size_t p_playlist, const bit_array& p_mask, play_control::t_display_level p_level) override
    {
    }

    void on_items_replaced(size_t p_playlist, const bit_array& p_mask,
        const pfc::list_base_const_t<t_on_items_replaced_entry>& p_data) noexcept override;

    void on_item_ensure_visible(size_t p_playlist, size_t p_idx) override {}

    void on_playlist_activate(size_t p_old, size_t p_new) noexcept override;
    void on_playlist_created(size_t p_index, const char* p_name, size_t p_name_len) noexcept override;
    void on_playlists_reorder(const size_t* p_order, size_t p_count) noexcept override;
    void on_playlists_removing(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override {}
    void on_playlists_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) noexcept override;
    void on_playlist_renamed(size_t p_index, const char* p_new_name, size_t p_new_name_len) noexcept override;

    void on_default_format_changed() override {}
    void on_playback_order_changed(size_t p_new_index) override {}
    void on_playlist_locked(size_t p_playlist, bool p_locked) noexcept override;

    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) noexcept override;
    void on_playback_new_track(metadb_handle_ptr p_track) noexcept override;
    void on_playback_stop(play_control::t_stop_reason p_reason) noexcept override;
    void on_playback_seek(double p_time) override {}
    void on_playback_pause(bool p_state) override {}
    void on_playback_edited(metadb_handle_ptr p_track) override {}
    void on_playback_dynamic_info(const file_info& p_info) override {}
    void on_playback_dynamic_info_track(const file_info& p_info) override {}
    void on_playback_time(double p_time) override {}
    void on_volume_change(float p_new_val) override {}

    const GUID& get_extension_guid() const override { return guid_playlist_switcher; }
    void get_name(pfc::string_base& out) const override { out = "Playlist switcher"; }
    void get_category(pfc::string_base& out) const override { out = "Panels"; }
    bool get_short_name(pfc::string_base& out) const override
    {
        out.set_string("Playlists");
        return true;
    }
    unsigned get_type() const override { return uie::type_panel; }

    PlaylistSwitcher()
        : ListViewPanelBase(std::make_unique<uih::lv::DefaultRenderer>(true))
        , m_playing_playlist(pfc_infinite)
    {
    }

private:
    class SearchContext : public ListViewSearchContextBase {
    public:
        const char* get_item_text(size_t index) override
        {
            playlist_manager::get()->playlist_get_name(index, m_playlist_name);
            return m_playlist_name.c_str();
        }

    private:
        pfc::string8 m_playlist_name;
    };

    contextmenu_manager::ptr m_contextmenu_manager;
    UINT m_contextmenu_manager_base{NULL};
    ui_status_text_override::ptr m_status_text_override;
    ui_selection_holder::ptr m_selection_holder;

    bool m_switch_timer_active{false};
    std::shared_ptr<playlist_position_reference_tracker> m_switch_playlist;

    bool m_dragging{false};
    wil::com_ptr<IDataObject> m_DataObject;

    std::shared_ptr<playlist_position_reference_tracker> m_edit_playlist;
    size_t m_playing_playlist;
    service_ptr_t<playlist_manager_v3> m_playlist_api;
    service_ptr_t<playback_control> m_playback_api;

    static const GUID g_guid_font;
    static std::vector<PlaylistSwitcher*> g_windows;
};

} // namespace cui::panels::playlist_switcher
