#pragma once

#include "filter.h"
#include "filter_config_var.h"

namespace filter_panel {

class FilterSearchToolbar : public uie::container_uie_window_v2 {
public:
    static bool g_activate();
    static bool g_filter_search_bar_has_stream(
        FilterSearchToolbar const* p_seach_bar, const FilterStream::ptr& p_stream);
    static void s_on_favourites_change();

    static void g_initialise_filter_stream(const FilterStream::ptr& p_stream)
    {
        for (t_size i = 0, count = g_active_instances.get_count(); i < count; i++) {
            if (!cfg_orderedbysplitters || g_filter_search_bar_has_stream(g_active_instances[i], p_stream)) {
                if (!g_active_instances[i]->m_active_search_string.is_empty()) {
                    p_stream->m_source_overriden = true;
                    p_stream->m_source_handles = g_active_instances[i]->m_active_handles;
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
    enum { id_edit = 668, id_toolbar };

    enum { idc_clear = 1001, idc_favourite = 1002, msg_favourite_selected = WM_USER + 2 };

    enum { TIMER_QUERY = 1001 };

    enum { config_version_current = 0 };

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
    void on_size(t_size cx, t_size cy) override;
    void activate();

    static LRESULT WINAPI g_on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    using uie::container_uie_window_v2::on_size;

    HWND m_search_editbox{};
    HWND m_wnd_toolbar{};
    HWND m_wnd_last_focused{};
    WNDPROC m_proc_search_edit{};
    bool m_favourite_state{};
    bool m_query_timer_active{};
    bool m_show_clear_button{cfg_showsearchclearbutton};
    pfc::string8 m_active_search_string;
    metadb_handle_list m_active_handles;
    wil::unique_hfont m_font;
    HIMAGELIST m_imagelist{};
    int m_combo_cx{};
    int m_combo_cy{};
    int m_toolbar_cx{};
    int m_toolbar_cy{};

    static pfc::ptr_list_t<FilterSearchToolbar> g_active_instances;
};

}; // namespace filter_panel
