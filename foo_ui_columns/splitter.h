#pragma once

namespace cui::panels::splitter {

enum Orientation {
    horizontal,
    vertical,
};

class FlatSplitterPanel : public uie::container_uie_window_v3_t<uie::splitter_window_v2> {
public:
    virtual Orientation get_orientation() const = 0;
    static int g_get_caption_size();
    void get_category(pfc::string_base& p_out) const override;
    unsigned get_type() const override;

    void insert_panel(size_t index, const uie::splitter_item_t* p_item) override;

    void remove_panel(size_t index) override;
    void replace_panel(size_t index, const uie::splitter_item_t* p_item) override;

    size_t get_panel_count() const override;
    uie::splitter_item_t* get_panel(size_t index) const override;
    enum {
        stream_version_current = 0
    };

    void set_config(stream_reader* config, size_t p_size, abort_callback& p_abort) override;
    void import_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort) override;
    void export_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    void get_config(stream_writer* out, abort_callback& p_abort) const override;

    bool is_index_valid(size_t index) const;

    bool get_config_item_supported(size_t index, const GUID& p_type) const override;

    bool get_config_item(
        size_t index, const GUID& p_type, stream_writer* p_out, abort_callback& p_abort) const override;

    bool set_config_item(size_t index, const GUID& p_type, stream_reader* p_source, abort_callback& p_abort) override;

    class FlatSplitterPanelHost : public ui_extension::window_host_ex {
        service_ptr_t<FlatSplitterPanel> m_this;

    public:
        const GUID& get_host_guid() const override;

        bool get_keyboard_shortcuts_enabled() const override;
        void get_children(pfc::list_base_t<window::ptr>& p_out) override;

        void on_size_limit_change(HWND wnd, unsigned flags) override;

        // unsigned get_orientation();
        Orientation get_orientation() const;

        unsigned is_resize_supported(HWND wnd) const override;

        bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height) override;

        bool override_status_text_create(service_ptr_t<ui_status_text_override>& p_out) override;

        bool is_visible(HWND wnd) const override;

        bool is_visibility_modifiable(HWND wnd, bool desired_visibility) const override;
        bool set_window_visibility(HWND wnd, bool visibility) override;

        void set_window_ptr(FlatSplitterPanel* p_ptr);

        void relinquish_ownership(HWND wnd) override;
    };

    bool is_point_ours(HWND wnd_point, const POINT& pt_screen, pfc::list_base_t<window::ptr>& p_hierarchy) override;
    void get_supported_panels(
        const pfc::list_base_const_t<window::ptr>& p_windows, bit_array_var& p_mask_unsupported) override;

    static void g_on_size_change();

private:
    struct SizeLimit {
        unsigned min_height{0};
        unsigned max_height{0};
        unsigned min_width{0};
        unsigned max_width{0};
        SizeLimit() = default;
    };
    class Panel : public std::enable_shared_from_this<Panel> {
    public:
        class PanelContainer : fbh::LowLevelMouseHookManager::HookCallback {
        public:
            enum {
                MSG_AUTOHIDE_END = WM_USER + 2
            };

            explicit PanelContainer(Panel* p_panel);

            ~PanelContainer();

            HWND create(HWND parent) const;
            void destroy() const;
            HWND get_wnd() const { return m_wnd; }

            void set_window_ptr(FlatSplitterPanel* p_ptr);
            void enter_autohide_hook();
            // private:
            LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
            void on_hooked_message(WPARAM msg, const MSLLHOOKSTRUCT& mllhs) override;
            service_ptr_t<FlatSplitterPanel> m_this;

            wil::unique_htheme m_theme;
            Panel* m_panel;

            bool m_hook_active;
            bool m_timer_active;

        private:
            void reopen_theme();
            void close_theme();
            bool test_autohide_window(HWND wnd);

            inline constexpr static auto class_name = L"foo_ui_columns_splitter_panel_child_container";

            HWND m_wnd{};
            std::unique_ptr<uie::container_window_v3> m_window;
            std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;
        } m_container;

        GUID m_guid{};
        unsigned m_caption_orientation{NULL};
        bool m_locked{false};
        bool m_hidden{false};
        bool m_autohide{false};
        HWND m_wnd{nullptr};
        HWND m_wnd_child{nullptr};
        bool m_show_caption{true};
        pfc::array_t<uint8_t> m_child_data;
        SizeLimit m_size_limits;
        uie::window_ptr m_child;
        bool m_show_toggle_area{false};
        bool m_use_custom_title{false};
        pfc::string8 m_custom_title;

        service_ptr_t<class FlatSplitterPanelHost> m_interface;

        uih::IntegerAndDpi<uint32_t> m_size{150};

        uie::splitter_item_full_v2_t* create_splitter_item(bool b_set_ptr = true);

        void set_from_splitter_item(const uie::splitter_item_t* p_source);

        void refresh_child_data(abort_callback& aborter = fb2k::noAbort);
        void _export(stream_writer* out, abort_callback& p_abort);
        void write(stream_writer* out, abort_callback& p_abort);
        void import(stream_reader* t, abort_callback& p_abort);
        void read(stream_reader* t, abort_callback& p_abort);
        void read_extra(stream_reader* reader, abort_callback& p_abort);
        void write_extra(stream_writer* writer, abort_callback& p_abort);

        void set_hidden(bool val);

        void on_size();
        void on_size(int cx, int cy);

        void destroy();
        Panel();

        using Ptr = std::shared_ptr<Panel>;
    };

    void read_config(stream_reader* p_reader, size_t p_size, bool is_import, abort_callback& p_abort);
    void write_config(stream_writer* p_writer, bool is_export, abort_callback& p_abort) const;

    void start_autohide_dehide(size_t index, bool b_next_too = true);

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    void get_panels_sizes(unsigned width, unsigned height, pfc::list_base_t<unsigned>& p_out);
    bool find_by_divider_pt(POINT& pt, size_t& p_out);
    bool test_divider_pt(const POINT& pt, size_t p_out);

    int get_panel_divider_size(size_t index) const;

    void on_size_changed(unsigned width, unsigned height);
    void on_size_changed();
    void save_sizes();

    void save_sizes(unsigned width, unsigned height);

    bool can_resize_divider(size_t index) const;
    bool can_resize_panel(size_t index) const;
    int override_size(const size_t panel, int delta);

    void refresh_children();
    void destroy_children();

    std::vector<Panel::Ptr>::iterator find_panel_by_container_wnd(HWND wnd);
    std::vector<Panel::Ptr>::iterator find_panel_by_panel_wnd(HWND wnd);

    std::vector<Panel::Ptr> m_panels;
    HWND m_wnd{nullptr};

    int m_last_position{};
    size_t m_panel_dragging{};
    bool m_panel_dragging_valid{false};
    std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;

    static wil::unique_hfont g_font_menu_horizontal;
    static wil::unique_hfont g_font_menu_vertical;
    static unsigned g_count;
    static pfc::ptr_list_t<FlatSplitterPanel> g_instances;

public:
    FlatSplitterPanel() = default;
};

} // namespace cui::panels::splitter
