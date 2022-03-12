#pragma once

#include "pch.h"

namespace cui::panels::playlist_tabs {

void g_on_autohide_tabs_change();
void g_on_multiline_tabs_change();
void g_on_tabs_font_change();

void remove_playlist_helper(size_t index);
constexpr unsigned SWITCH_TIMER_ID = 670u;

class PlaylistTabs
    : public uie::container_ui_extension_t<ui_helpers::container_window, uie::splitter_window_v2>
    , public playlist_callback {
public:
    enum : uint32_t { MSG_RESET_SIZE_LIMITS = WM_USER + 3 };
    class WindowHost : public ui_extension::window_host {
    public:
        unsigned is_resize_supported(HWND wnd) const override;

        bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height) override;
        bool is_visible(HWND wnd) const override;
        bool is_visibility_modifiable(HWND wnd, bool desired_visibility) const override;
        bool set_window_visibility(HWND wnd, bool visibility) override;

        bool get_keyboard_shortcuts_enabled() const override;

        virtual bool get_show_shortcuts() const;

        void on_size_limit_change(HWND wnd, unsigned flags) override;

        const GUID& get_host_guid() const override;

        bool override_status_text_create(service_ptr_t<ui_status_text_override>& p_out) override;

        void relinquish_ownership(HWND wnd) override;

        void set_this(PlaylistTabs* ptr);

    private:
        service_ptr_t<PlaylistTabs> m_this;
    };

private:
    service_ptr_t<ui_status_text_override> m_status_override;

    WNDPROC tabproc{nullptr};

    bool m_dragging{false};
    size_t m_dragging_idx{0};
    RECT m_dragging_rect{};

    bool m_playlist_switched{false};
    bool m_switch_timer{false};
    unsigned m_switch_playlist{0};
    bool initialised{false};

    t_int32 m_mousewheel_delta{0};
    UINT ID_CUSTOM_BASE{NULL};

    service_ptr_t<contextmenu_manager> p_manager;

    class_data& get_class_data() const override;

public:
    static pfc::ptr_list_t<PlaylistTabs> list_wnd;

    HWND wnd_tabs{nullptr};
    LRESULT WINAPI hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT WINAPI main_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    PlaylistTabs() = default;

    ~PlaylistTabs();

    class PlaylistTabsDropTarget : public IDropTarget {
    public:
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObject) override;
        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;
        HRESULT STDMETHODCALLTYPE DragEnter(
            IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
        HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
        HRESULT STDMETHODCALLTYPE DragLeave() override;
        HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
        explicit PlaylistTabsDropTarget(PlaylistTabs* p_wnd);

    private:
        bool m_last_rmb{};
        bool m_is_accepted_type{};
        long drop_ref_count{};
        POINTL last_over{};
        service_ptr_t<PlaylistTabs> p_list;
        wil::com_ptr_t<IDataObject> m_DataObject;
        wil::com_ptr_t<IDropTargetHelper> m_DropTargetHelper;
    };

    void FB2KAPI on_items_removing(
        size_t p_playlist, const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override;
    // called before actually removing them
    void FB2KAPI on_items_removed(
        size_t p_playlist, const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override;

    void on_playlist_activate(size_t p_old, size_t p_new) override;

    void on_playlists_reorder(const size_t* p_order, size_t p_count) override;
    void on_playlist_created(size_t p_index, const char* p_name, size_t p_name_len) override;
    void on_playlists_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override;
    void on_playlist_renamed(size_t p_index, const char* p_new_name, size_t p_new_name_len) override;

    void on_items_added(size_t, size_t, const pfc::list_base_const_t<metadb_handle_ptr>&, const bit_array&) override;
    void on_items_reordered(size_t, const size_t*, size_t) override;
    void on_items_selection_change(size_t, const bit_array&, const bit_array&) override;
    void on_item_focus_change(size_t, size_t, size_t) override;
    void on_items_modified(size_t, const bit_array&) override;
    void on_items_modified_fromplayback(size_t, const bit_array&, play_control::t_display_level) override;
    void on_items_replaced(size_t, const bit_array&, const pfc::list_base_const_t<t_on_items_replaced_entry>&) override;
    void on_item_ensure_visible(size_t, size_t) override;
    void on_playlists_removing(const bit_array&, size_t, size_t) override;
    void on_default_format_changed() override;
    void on_playback_order_changed(size_t) override;
    void on_playlist_locked(size_t, bool) override;

    void kill_switch_timer();

    void switch_to_playlist_delayed2(unsigned idx);

    static const GUID extension_guid;

    const GUID& get_extension_guid() const override;

    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    bool get_short_name(pfc::string_base& out) const override;

    void set_styles(bool visible = true);

    void reset_size_limits();

    void on_size();

    void adjust_rect(bool b_larger, RECT* rc);

    void on_size(unsigned cx, unsigned cy);

    unsigned get_type() const override;
    static void on_font_change();
    bool create_tabs();

    void create_child();
    void destroy_child();

    bool is_point_ours(HWND wnd_point, const POINT& pt_screen, pfc::list_base_t<window::ptr>& p_hierarchy) override;
    void get_supported_panels(
        const pfc::list_base_const_t<window::ptr>& p_windows, bit_array_var& p_mask_unsupported) override;

    void insert_panel(size_t index, const uie::splitter_item_t* p_item) override;
    void remove_panel(size_t index) override;
    void replace_panel(size_t index, const uie::splitter_item_t* p_item) override;
    size_t get_panel_count() const override;
    size_t get_maximum_panel_count() const override;
    uie::splitter_item_t* get_panel(size_t index) const override;

    void import_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort) override;
    void export_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    void refresh_child_data(abort_callback& aborter = fb2k::noAbort) const;
    void set_config(stream_reader* config, size_t p_size, abort_callback& p_abort) override;
    void get_config(stream_writer* out, abort_callback& p_abort) const override;

    void on_child_position_change();

private:
    static HFONT g_font;

    GUID m_child_guid{};
    mutable pfc::array_t<uint8_t> m_child_data;
    service_ptr_t<WindowHost> m_host;
    ui_extension::window_ptr m_child;
    HWND m_child_wnd{nullptr};
    HWND m_host_wnd{nullptr};

    unsigned m_child_top{0};

    MINMAXINFO mmi{};
    std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;
};

extern ui_extension::window_host_factory<PlaylistTabs::WindowHost> g_tab_host;

} // namespace cui::panels::playlist_tabs
