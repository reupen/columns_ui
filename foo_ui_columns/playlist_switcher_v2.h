#pragma once

#include "playlist_manager_utils.h"
#include "playlist_switcher.h"
#include "list_view_panel.h"

class PlaylistSwitcher
    : public ListViewPanelBase<PlaylistSwitcherColoursClient, uie::window>
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

    enum { TIMER_SWITCH = TIMER_BASE };

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

        DropTarget(PlaylistSwitcher* p_window);

    private:
        long drop_ref_count;
        bool m_last_rmb;
        bool m_is_playlists;
        bool m_is_accepted_type;
        service_ptr_t<PlaylistSwitcher> m_window;
        wil::com_ptr_t<IDataObject> m_DataObject;
        service_ptr_t<ole_interaction_v2> m_ole_api;
        service_ptr_t<playlist_manager_v4> m_playlist_api;
        wil::com_ptr_t<IDropTargetHelper> m_DropTargetHelper;
    };

public:
    static void g_on_font_items_change();
    static void g_on_edgestyle_change();
    static void g_on_vertical_item_padding_change();
    static void g_redraw_all();
    static void g_refresh_all_items();

    void destroy_switch_timer()
    {
        if (m_switch_timer_active) {
            KillTimer(get_wnd(), TIMER_SWITCH);
            m_switch_timer_active = false;
            m_switch_playlist.reset();
        }
    }
    void set_switch_timer(t_size index)
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
            t_size index = m_switch_playlist ? m_switch_playlist->m_playlist : pfc_infinite;
            destroy_switch_timer();
            if (index != pfc_infinite)
                m_playlist_api->set_active_playlist(index);
            return true;
        }
        return false;
    }

    void get_insert_items(t_size base, t_size count, pfc::list_t<uih::ListView::InsertItem>& p_out);
    void refresh_all_items();
    void refresh_items(t_size base, t_size count, bool b_update = true);
    void add_items(t_size base, t_size count);
    void refresh_columns();

    void notify_on_initialisation() override;
    void notify_on_create() override;
    void notify_on_destroy() override;

    void move_selection(int delta) override;

    bool notify_before_create_inline_edit(
        const pfc::list_base_const_t<t_size>& indices, unsigned column, bool b_source_mouse) override;
    bool notify_create_inline_edit(const pfc::list_base_const_t<t_size>& indices, unsigned column,
        pfc::string_base& p_text, t_size& p_flags, mmh::ComPtr<IUnknown>& pAutocompleteEntries) override;
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
    };

    bool notify_on_middleclick(bool on_item, t_size index) override
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
        t_size index = m_playlist_api->get_active_playlist();
        if (index < m_playlist_api->get_playlist_count()) {
            pfc::string8 name;
            m_playlist_api->playlist_get_name(index, name);
            pfc::string_formatter formatter;
            if (uMessageBox(get_wnd(), formatter << "Are you sure you want to delete the \"" << name << "\" playlist?",
                    "Delete Playlist", MB_YESNO)
                == IDYES)
                m_playlist_api->remove_playlist_switch(index);
        }
        return true;
    }

    bool notify_on_keyboard_keydown_cut() override
    {
        t_size index = m_playlist_api->get_active_playlist();
        if (index != pfc_infinite)
            playlist_manager_utils::cut(pfc::list_single_ref_t<t_size>(index));
        return true;
    }

    bool notify_on_keyboard_keydown_copy() override
    {
        t_size index = m_playlist_api->get_active_playlist();
        if (index != pfc_infinite)
            playlist_manager_utils::copy(pfc::list_single_ref_t<t_size>(index));
        return true;
    }

    bool notify_on_keyboard_keydown_paste() override
    {
        t_size index = m_playlist_api->get_active_playlist();
        if (index == pfc_infinite)
            index = m_playlist_api->get_playlist_count();
        else
            index++;
        playlist_manager_utils::paste(get_wnd(), index);
        return true;
    }

    void execute_default_action(t_size index, t_size column, bool b_keyboard, bool b_ctrl) override
    {
        if (m_playlist_api->playlist_get_item_count(index)) {
            m_playlist_api->set_playing_playlist(index);
            static_api_ptr_t<play_control>()->start();
        }
    }
    void notify_on_selection_change(const pfc::bit_array& p_affected, const pfc::bit_array& p_status,
        notification_source_t p_notification_source) override
    {
        if (p_notification_source != notification_source_rmb) {
            t_size numSelected = get_selection_count(2);

            if (numSelected == 1) {
                pfc::bit_array_bittable mask(get_item_count());
                get_selection_state(mask);
                t_size index = 0;
                t_size count = get_item_count();
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
        m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
        m_selection_holder->set_playlist_tracking();
    }

    void notify_on_kill_focus(HWND wnd_receiving) override { m_selection_holder.release(); }

    t_size get_playing_playlist()
    {
        return m_playback_api->is_playing() ? m_playlist_api->get_playing_playlist() : pfc_infinite;
    }

    void on_playing_playlist_change(unsigned p_playing_playlist)
    {
        t_size previous_playing = m_playing_playlist;
        m_playing_playlist = p_playing_playlist;
        if (previous_playing != pfc_infinite && previous_playing < get_item_count())
            refresh_items(previous_playing, 1);
        if (p_playing_playlist != previous_playing && p_playing_playlist != pfc_infinite
            && p_playing_playlist < get_item_count())
            refresh_items(p_playing_playlist, 1);
    }

    void refresh_playing_playlist() { m_playing_playlist = get_playing_playlist(); }

    void on_items_added(t_size p_playlist, t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const pfc::bit_array& p_selection) override;
    void on_items_reordered(t_size p_playlist, const t_size* p_order, t_size p_count) override{};
    void on_items_removing(
        t_size p_playlist, const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override{};
    void on_items_removed(
        t_size p_playlist, const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override;
    void on_items_selection_change(
        t_size p_playlist, const pfc::bit_array& p_affected, const pfc::bit_array& p_state) override
    {
    }
    void on_item_focus_change(t_size p_playlist, t_size p_from, t_size p_to) override {}

    void on_items_modified(t_size p_playlist, const pfc::bit_array& p_mask) override;
    void on_items_modified_fromplayback(
        t_size p_playlist, const pfc::bit_array& p_mask, play_control::t_display_level p_level) override{};

    void on_items_replaced(t_size p_playlist, const pfc::bit_array& p_mask,
        const pfc::list_base_const_t<t_on_items_replaced_entry>& p_data) override;

    void on_item_ensure_visible(t_size p_playlist, t_size p_idx) override{};

    void on_playlist_activate(t_size p_old, t_size p_new) override;
    void on_playlist_created(t_size p_index, const char* p_name, t_size p_name_len) override;
    void on_playlists_reorder(const t_size* p_order, t_size p_count) override;
    void on_playlists_removing(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override{};
    void on_playlists_removed(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override;
    void on_playlist_renamed(t_size p_index, const char* p_new_name, t_size p_new_name_len) override;

    void on_default_format_changed() override{};
    void on_playback_order_changed(t_size p_new_index) override{};
    void on_playlist_locked(t_size p_playlist, bool p_locked) override;

    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override;
    void on_playback_new_track(metadb_handle_ptr p_track) override;
    void on_playback_stop(play_control::t_stop_reason p_reason) override;
    void on_playback_seek(double p_time) override{};
    void on_playback_pause(bool p_state) override{};
    void on_playback_edited(metadb_handle_ptr p_track) override{};
    void on_playback_dynamic_info(const file_info& p_info) override{};
    void on_playback_dynamic_info_track(const file_info& p_info) override{};
    void on_playback_time(double p_time) override{};
    void on_volume_change(float p_new_val) override{};

    const GUID& get_extension_guid() const override { return cui::panels::guid_playlist_switcher; }
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
        , m_playing_playlist(pfc_infinite){};

private:
    contextmenu_manager::ptr m_contextmenu_manager;
    UINT_PTR m_contextmenu_manager_base{NULL};
    ui_status_text_override::ptr m_status_text_override;
    ui_selection_holder::ptr m_selection_holder;

    bool m_switch_timer_active{false};
    std::shared_ptr<playlist_position_reference_tracker> m_switch_playlist;

    bool m_dragging{false};
    wil::com_ptr_t<IDataObject> m_DataObject;

    std::shared_ptr<playlist_position_reference_tracker> m_edit_playlist;
    t_size m_playing_playlist;
    service_ptr_t<playlist_manager_v3> m_playlist_api;
    service_ptr_t<playback_control> m_playback_api;

    static const GUID g_guid_font;
    static std::vector<PlaylistSwitcher*> g_windows;
};
