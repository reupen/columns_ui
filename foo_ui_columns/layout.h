#pragma once

/*!
 * \file layout.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Classes used for hosting panels in the main area of the UI
 */

class cfg_layout_t : public cfg_var {
public:
    class preset {
    public:
        pfc::array_t<t_uint8> m_val;
        GUID m_guid{};
        pfc::string_simple m_name;
        void write(stream_writer* out, abort_callback& p_abort);
        void read(stream_reader* out, abort_callback& p_abort);

        void get(uie::splitter_item_ptr& p_out);
        void set(const uie::splitter_item_t* item);
    };

    unsigned get_active() { return m_active; }

    /** from configuration, not from ui */
    const pfc::list_base_const_t<preset>& get_presets() const;

    void get_preset(t_size index, uie::splitter_item_ptr& p_out);
    void set_preset(t_size index, const uie::splitter_item_t* item);
    void get_preset_name(t_size index, pfc::string_base& p_out);
    void set_preset_name(t_size index, const char* ptr, t_size len);

    void get_active_preset_for_use(uie::splitter_item_ptr& p_out);
    t_size delete_preset(t_size index);

    t_size add_preset(const char* p_name, t_size len = pfc_infinite);
    t_size add_preset(const preset& item);
    void set_active_preset(t_size index);
    void save_active_preset();

    void set_presets(const pfc::list_base_const_t<preset>& presets, t_size active);
    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;

    void set_data_raw(stream_reader*, unsigned p_sizehint, abort_callback& p_abort) override;

    void reset_presets(); // needs services

    cfg_layout_t(const GUID& p_guid);

private:
    enum { stream_version_current = 0 };

    pfc::list_t<preset> m_presets;

    unsigned m_active;

    // bool m_initialised;
};

class layout_window
    : public ui_helpers::container_window
    , private uih::MessageHook {
public:
    enum { MSG_LAYOUT_SET_FOCUS = WM_USER + 2, MSG_EDIT_PANEL, MSG_EDIT_PANEL_V2 };

    static void g_get_default_presets(pfc::list_t<cfg_layout_t::preset>& p_out);

    void refresh_child();
    void relinquish_child();

    void get_child(uie::splitter_item_ptr& p_out);
    void set_child(const uie::splitter_item_t* item);

    bool set_focus();
    void show_window();

    void export_config(stream_writer* p_out, t_uint32 mode, pfc::list_base_t<GUID>& panels, abort_callback& p_abort);
    bool import_config_to_object(stream_reader* p_reader, t_size size, t_uint32 mode, cfg_layout_t::preset& p_out,
        pfc::list_base_t<GUID>& panels, abort_callback& p_abort);

    void show_menu_access_keys();
    void hide_menu_access_keys();
    bool on_menu_char(unsigned short c);
    bool set_menu_focus();
    bool is_menu_focused();
    HWND get_previous_menu_focus_window() const;

    void set_layout_editing_active(bool b_val);
    bool get_layout_editing_active();

    layout_window() = default;

private:
    class live_edit_data_t {
    public:
        pfc::list_t<uie::window::ptr> m_hierarchy;
        POINT m_point;
        HWND m_wnd;

        void reset() { m_hierarchy.remove_all(); }
    };

    void enter_layout_editing_mode();
    void exit_layout_editing_mode();
    uih::TranslucentFillWindow m_trans_fill;
    void run_live_edit_base(POINT pt_menu);
    void run_live_edit_base_delayed(POINT pt_menu);
    void run_live_edit_base_delayed_v2(HWND wnd, POINT pt, pfc::list_t<uie::window::ptr>& p_hierarchy);
    void run_live_edit_base_v2(const live_edit_data_t& p_data);
    bool on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp) override;

    class_data& get_class_data() const override
    {
        __implement_get_class_data(_T("{DA9A1375-A411-48a9-AF74-4AC29FF9BE9C}"), false);
    }

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    void create_child();
    void destroy_child();

    bool __is_menu_focused_recur(const uie::window_ptr& p_wnd);
    bool __set_menu_focus_recur(const uie::window_ptr& p_wnd);
    bool __set_focus_recur(const uie::window_ptr& p_wnd);
    void __show_menu_access_keys_recur(const uie::window_ptr& p_wnd);
    void __hide_menu_access_keys_recur(const uie::window_ptr& p_wnd);
    bool __on_menu_char_recur(const uie::window_ptr& p_wnd, unsigned short c);
    bool __get_previous_menu_focus_window_recur(const uie::window_ptr& p_wnd, HWND& wnd_previous) const;

    uie::window_ptr m_child;
    GUID m_child_guid{columns_ui::panels::guid_playlist_view_v2};
    pfc::array_t<t_uint8> m_child_data;
    HWND m_child_wnd{nullptr};
    bool m_layout_editing_active{false};
    live_edit_data_t m_live_edit_data;
};

extern layout_window g_layout_window;
extern cfg_layout_t cfg_layout;
