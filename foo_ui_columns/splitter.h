#ifndef _COLUMNS_SPLITTER_H_
#define _COLUMNS_SPLITTER_H_

enum orientation_t {
    horizontal,
    vertical,
};

class splitter_window_impl
    : public uie::container_ui_extension_t<ui_helpers::container_window, uie::splitter_window_v2> {
public:
    virtual orientation_t get_orientation() const = 0;
    static unsigned g_get_caption_size();
    void get_category(pfc::string_base& p_out) const override;
    unsigned get_type() const override;

    void insert_panel(unsigned index, const uie::splitter_item_t* p_item) override;

    void remove_panel(unsigned index) override;
    void replace_panel(unsigned index, const uie::splitter_item_t* p_item) override;

    unsigned get_panel_count() const override;
    uie::splitter_item_t* get_panel(unsigned index) const override;
    enum { stream_version_current = 0 };

    void set_config(stream_reader* config, t_size p_size, abort_callback& p_abort) override;
    void import_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort) override;
    void export_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    void get_config(stream_writer* out, abort_callback& p_abort) const override;

    bool is_index_valid(unsigned index) const;

    bool get_config_item_supported(unsigned index, const GUID& p_type) const override;

    bool get_config_item(
        unsigned index, const GUID& p_type, stream_writer* p_out, abort_callback& p_abort) const override;

    bool set_config_item(unsigned index, const GUID& p_type, stream_reader* p_source, abort_callback& p_abort) override;

    class splitter_host_impl : public ui_extension::window_host_ex {
        service_ptr_t<splitter_window_impl> m_this;

    public:
        const GUID& get_host_guid() const override;

        bool get_keyboard_shortcuts_enabled() const override;
        void get_children(pfc::list_base_t<uie::window::ptr>& p_out) override;

        void on_size_limit_change(HWND wnd, unsigned flags) override;
        ;

        // unsigned get_orientation();
        orientation_t get_orientation() const;

        unsigned is_resize_supported(HWND wnd) const override;

        bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height) override;

        bool override_status_text_create(service_ptr_t<ui_status_text_override>& p_out) override;

        bool is_visible(HWND wnd) const override;

        bool is_visibility_modifiable(HWND wnd, bool desired_visibility) const override;
        bool set_window_visibility(HWND wnd, bool visibility) override;

        void set_window_ptr(splitter_window_impl* p_ptr);

        void relinquish_ownership(HWND wnd) override;
    };

    bool is_point_ours(
        HWND wnd_point, const POINT& pt_screen, pfc::list_base_t<uie::window::ptr>& p_hierarchy) override;
    void get_supported_panels(
        const pfc::list_base_const_t<uie::window::ptr>& p_windows, pfc::bit_array_var& p_mask_unsupported) override;

    static void g_on_size_change();

private:
    struct t_size_limit {
        unsigned min_height{0};
        unsigned max_height{0};
        unsigned min_width{0};
        unsigned max_width{0};
        t_size_limit() = default;
        ;
    };
    class panel : public pfc::refcounted_object_root {
    public:
        class panel_container
            : public ui_helpers::container_window
            , private fbh::LowLevelMouseHookManager::HookCallback {
        public:
            enum { MSG_AUTOHIDE_END = WM_USER + 2 };

            panel_container(panel* p_panel);
            ;
            ~panel_container();
            void set_window_ptr(splitter_window_impl* p_ptr);
            void enter_autohide_hook();
            // private:
            class_data& get_class_data() const override;
            LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
            void on_hooked_message(WPARAM msg, const MSLLHOOKSTRUCT& mllhs) override;
            service_ptr_t<splitter_window_impl> m_this;

            HTHEME m_theme;
            panel* m_panel;

            bool m_hook_active;
            bool m_timer_active;

        private:
            bool test_autohide_window(HWND wnd);
        } m_container;

        GUID m_guid;
        unsigned m_caption_orientation{NULL};
        bool m_locked{false};
        bool m_hidden{false};
        bool m_autohide{false};
        HWND m_wnd{nullptr};
        HWND m_wnd_child{nullptr};
        bool m_show_caption{true};
        pfc::array_t<t_uint8> m_child_data;
        t_size_limit m_size_limits;
        uie::window_ptr m_child;
        bool m_show_toggle_area{false};
        bool m_use_custom_title{false};
        pfc::string8 m_custom_title;

        service_ptr_t<class splitter_host_impl> m_interface;

        uih::IntegerAndDpi<uint32_t> m_size{150};

        uie::splitter_item_full_v2_t* create_splitter_item(bool b_set_ptr = true);

        void set_from_splitter_item(const uie::splitter_item_t* p_source);

        void _export(stream_writer* out, abort_callback& p_abort);
        void write(stream_writer* out, abort_callback& p_abort);
        void import(stream_reader* t, abort_callback& p_abort);
        void read(stream_reader* t, abort_callback& p_abort);
        void read_extra(stream_reader* reader, abort_callback& p_abort);
        void write_extra(stream_writer* writer, abort_callback& p_abort);

        void set_hidden(bool val);

        void on_size();
        void on_size(unsigned cx, unsigned cy);

        void destroy();
        panel();

        using ptr = pfc::refcounted_object_ptr_t<panel>;
        static ptr null_ptr;
    };
    class panel_list : public pfc::list_t<pfc::refcounted_object_ptr_t<panel>> {
    public:
        bool move_up(unsigned idx);
        bool move_down(unsigned idx);
        bool find_by_wnd(HWND wnd, unsigned& p_out);
        bool find_by_wnd_child(HWND wnd, unsigned& p_out);
    };

    void read_config(stream_reader* p_reader, t_size p_size, bool is_import, abort_callback& p_abort);
    void write_config(stream_writer* p_writer, bool is_export, abort_callback& p_abort) const;

    void start_autohide_dehide(unsigned index, bool b_next_too = true);

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    void get_panels_sizes(unsigned width, unsigned height, pfc::list_base_t<unsigned>& p_out);
    bool find_by_divider_pt(POINT& pt, unsigned& p_out);
    bool test_divider_pt(const POINT& pt, unsigned p_out);

    unsigned get_panel_divider_size(unsigned index);

    void on_size_changed(unsigned width, unsigned height);
    void on_size_changed();
    void save_sizes();

    void save_sizes(unsigned width, unsigned height);

    bool can_resize_divider(t_size index) const;
    bool can_resize_panel(t_size index) const;
    int override_size(unsigned& panel, int delta);

    void refresh_children();
    void destroy_children();

    // unsigned get_orientation();
    panel_list m_panels;
    HWND m_wnd{nullptr};

    int m_last_position{NULL};
    unsigned m_panel_dragging{NULL};
    bool m_panel_dragging_valid{false};

    static gdi_object_t<HFONT>::ptr_t g_font_menu_horizontal;
    static gdi_object_t<HFONT>::ptr_t g_font_menu_vertical;
    static unsigned g_count;
    static pfc::ptr_list_t<splitter_window_impl> g_instances;

public:
    splitter_window_impl() = default;

    //
};

class WindowEnum_t {
    static BOOL CALLBACK g_EnumWindowsProc(HWND wnd, LPARAM lp) { return ((WindowEnum_t*)lp)->EnumWindowsProc(wnd); }
    BOOL EnumWindowsProc(HWND wnd)
    {
        if (GetWindow(wnd, GW_OWNER) == m_wnd_owner && IsWindowVisible(wnd))
            m_wnd_list.add_item(wnd);
        return TRUE;
    }
    HWND m_wnd_owner;

public:
    void run() { EnumWindows(&g_EnumWindowsProc, (LPARAM)this); }
    pfc::list_t<HWND, pfc::alloc_fast> m_wnd_list;
    WindowEnum_t(HWND wnd_owner) : m_wnd_owner(wnd_owner){};
};

void g_get_panel_list(uie::window_info_list_simple& p_out, uie::window_host_ptr& p_host);
void g_append_menu_panels(HMENU menu, const uie::window_info_list_simple& panels, UINT base);
void g_append_menu_splitters(HMENU menu, const uie::window_info_list_simple& panels, UINT base);
void g_run_live_edit_contextmenu(HWND wnd, POINT pt_menu, uih::TranslucentFillWindow& p_overlay, const RECT& rc_overlay,
    uie::window_ptr ptr, uie::splitter_window_ptr p_container, t_size index, uie::window_host_ptr& p_host);
void clip_minmaxinfo(MINMAXINFO& mmi);

#endif
