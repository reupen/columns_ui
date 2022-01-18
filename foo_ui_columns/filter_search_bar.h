#pragma once

#include "filter.h"
#include "filter_config_var.h"

namespace cui::panels::filter {

class FilterSearchToolbar : public uie::container_uie_window_v2 {
public:
    static bool g_activate();
    static bool g_filter_search_bar_has_stream(
        FilterSearchToolbar const* p_seach_bar, const FilterStream::ptr& p_stream);
    static void s_on_favourites_change();

    static void g_initialise_filter_stream(const FilterStream::ptr& p_stream)
    {
        for (auto&& window : s_windows) {
            if (!cfg_orderedbysplitters || g_filter_search_bar_has_stream(window, p_stream)) {
                if (!window->m_active_search_string.is_empty()) {
                    p_stream->m_source_overriden = true;
                    p_stream->m_source_handles = window->m_active_handles;
                    break;
                }
            }
        }
    }

    void get_name(pfc::string_base& out) const override { out = "Filter search"; }

    const GUID& get_extension_guid() const override { return cui::toolbars::guid_filter_search_bar; }

    void get_category(pfc::string_base& out) const override { out = "Toolbars"; }

    unsigned get_type() const override { return uie::type_toolbar; }

    t_uint32 get_flags() const override { return flag_default_flags_plus_transparent_background; }

private:
    class FontClient : public cui::fonts::client {
        const GUID& get_client_guid() const override { return font_client_id; }
        void get_name(pfc::string_base& p_out) const override { p_out = "Filter search"; }
        cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_items; }
        void on_font_changed() const override { s_update_font(); }
    };

    class ColourClient : public cui::colours::client {
        const GUID& get_client_guid() const override { return colour_client_id; }
        void get_name(pfc::string_base& p_out) const override { p_out = "Filter search"; }
        size_t get_supported_colours() const override
        {
            return cui::colours::colour_flag_text | cui::colours::colour_flag_background;
        }
        size_t get_supported_bools() const override { return 0; }
        bool get_themes_supported() const override { return false; }
        void on_bool_changed(t_size mask) const override {}
        void on_colour_changed(t_size mask) const override { s_update_colours(); }
    };

    enum { id_edit = 668, id_toolbar };

    enum { idc_clear = 1001, idc_favourite = 1002, msg_favourite_selected = WM_USER + 2 };

    enum { TIMER_QUERY = 1001 };

    enum { config_version_current = 0 };

    static LRESULT WINAPI g_on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    static void s_recreate_font();
    static void s_recreate_background_brush();
    static void s_update_colours();
    static void s_update_font();

    const GUID& get_class_guid() override { return cui::toolbars::guid_filter_search_bar; }

    void set_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort) override;
    void get_config(stream_writer* p_writer, abort_callback& p_abort) const override;
    void get_menu_items(uie::menu_hook_t& p_hook) override;

    void on_show_clear_button_change();
    void on_search_editbox_change();
    void commit_search_results(const char* str, bool b_force_autosend = false, bool b_stream_update = false);

    void refresh_favourites(bool is_initial);
    void update_favourite_icon(const char* p_new = nullptr);

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    void create_edit();
    void recalculate_dimensions();
    void on_size(t_size cx, t_size cy) override;
    void activate();

    LRESULT on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    using uie::container_uie_window_v2::on_size;

    static constexpr GUID font_client_id
        = {0xfc156d41, 0xcbb, 0x4b98, {0x88, 0x8b, 0x36, 0x4f, 0x38, 0x17, 0x2a, 0x2e}};

    static constexpr GUID colour_client_id
        = {0xa8ce267b, 0x39a0, 0x4d64, {0xbb, 0xaa, 0xc8, 0xee, 0x14, 0x58, 0xfb, 0x79}};

    inline static fonts::client::factory<FontClient> s_font_client;
    inline static wil::unique_hfont s_font;
    inline static colours::client::factory<ColourClient> s_colour_client;
    inline static wil::unique_hbrush s_background_brush;
    inline static std::vector<FilterSearchToolbar*> s_windows;

    HWND m_search_editbox{};
    HWND m_wnd_toolbar{};
    HWND m_wnd_last_focused{};
    WNDPROC m_proc_search_edit{};
    bool m_favourite_state{};
    bool m_query_timer_active{};
    bool m_show_clear_button{cfg_showsearchclearbutton};
    pfc::string8 m_active_search_string;
    metadb_handle_list m_active_handles;
    HIMAGELIST m_imagelist{};
    int m_combo_cx{};
    int m_combo_cy{};
    int m_toolbar_cx{};
    int m_toolbar_cy{};
};

}; // namespace cui::panels::filter
