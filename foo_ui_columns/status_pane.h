#pragma once
#include "volume.h"

void g_split_string_by_crlf(const char* text, pfc::string_list_impl& p_out);

class status_pane
    : public ui_helpers::container_window
    , private playlist_callback_single
    , play_callback {
    class volume_panel_attributes {
    public:
        static const TCHAR* const get_class_name() { return _T("volume_toolbar_pain"); }
        static bool get_show_caption() { return false; }
        static COLORREF get_background_colour() { return -1; /*RGB(230,230,255);*/ }
    };
    volume_control_t<false, false, volume_panel_attributes> m_volume_control;

    class titleformat_hook_impl : public titleformat_hook {
    public:
        bool process_field(
            titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override
        {
            p_found_flag = false;
            if (!stricmp_utf8_ex(p_name, p_name_length, "is_status_pane", pfc_infinite)) {
                p_out->write(titleformat_inputtypes::unknown, "true");
                p_found_flag = true;
                return true;
            }
            return false;
        }

        bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
            titleformat_hook_function_params* p_params, bool& p_found_flag) override
        {
            p_found_flag = false;
            return false;
        }

        titleformat_hook_impl() {}

    private:
    };

    class_data& get_class_data() const override
    {
        __implement_get_class_data_child_ex3(L"CUI_STATUS_PAIN", false, true, CS_DBLCLKS, IDC_ARROW);
    }

    /** PLAYLIST CALLBACKS */
    void on_items_added(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const pfc::bit_array& p_selection) override
    {
        on_playlist_change();
    }
    void on_items_reordered(const t_size* p_order, t_size p_count) override {}
    void on_items_removing(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override {}
    void on_items_removed(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override
    {
        on_playlist_change();
    }
    void on_items_selection_change(const pfc::bit_array& p_affected, const pfc::bit_array& p_state) override
    {
        on_playlist_change();
    }
    void on_item_focus_change(t_size p_from, t_size p_to) override {}
    void on_items_modified(const pfc::bit_array& p_mask) override {}
    void on_items_modified_fromplayback(const pfc::bit_array& p_mask, play_control::t_display_level p_level) override {}
    void on_items_replaced(const pfc::bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override
    {
    }
    void on_item_ensure_visible(t_size p_idx) override {}

    void on_playlist_switch() override { on_playlist_change(); }
    void on_playlist_renamed(const char* p_new_name, t_size p_new_name_len) override {}
    void on_playlist_locked(bool p_locked) override {}

    void on_default_format_changed() override {}
    void on_playback_order_changed(t_size p_new_index) override {}

    /** PLAY CALLBACKS */
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override
    {
        m_track_label = "Opening...";
        invalidate_all();
    }

    void on_playback_new_track(metadb_handle_ptr p_track) override
    {
        // Note: On start-up, playback can actually be paused when we get here
        // (and on_playback_pause() won't be called).
        update_playback_status_text();
        update_playing_text();
        invalidate_all();
    }

    void on_playback_stop(play_control::t_stop_reason p_reason) override
    {
        if (p_reason != playback_control::stop_reason_shutting_down) {
            m_track_label = "";
            update_playing_text();
            invalidate_all();
        }
    }

    void on_playback_seek(double p_time) override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_playback_pause(bool p_state) override
    {
        m_track_label = p_state ? "Paused:" : "Playing:";
        update_playing_text();
        invalidate_all();
    }
    void on_playback_edited(metadb_handle_ptr p_track) override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_playback_dynamic_info(const file_info& p_info) override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_playback_dynamic_info_track(const file_info& p_info) override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_playback_time(double p_time) override
    {
        update_playing_text();
        invalidate_all();
    }
    void on_volume_change(float p_new_val) override {}

    /** PRIVATE FUNCTIONS */
    void update_playlist_data() { get_length_data(m_selection, m_item_count, m_length_text); }
    void on_playlist_change()
    {
        update_playlist_data();
        invalidate_all();
    }
    void invalidate_all(bool b_update = true)
    {
        RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE | (b_update ? RDW_UPDATENOW : NULL));
    }
    void render_background(HDC dc, const RECT& rc);

    void update_playback_status_text();

    void update_playing_text()
    {
        metadb_handle_ptr track;
        static_api_ptr_t<play_control> play_api;
        play_api->get_now_playing(track);
        if (track.is_valid()) {
            service_ptr_t<titleformat_object> to_status;
            static_api_ptr_t<titleformat_compiler>()->compile_safe(
                to_status, main_window::config_status_bar_script.get());
            titleformat_hook_impl tf_hook;
            play_api->playback_format_title_ex(
                track, &tf_hook, playing1, to_status, nullptr, play_control::display_level_all);

            track.release();
        } else {
            playing1.force_reset();
        }
    }
    void get_length_data(bool& p_selection, t_size& p_count, pfc::string_base& p_out);

public:
    status_pane() : m_selection(false), m_item_count(0), m_menu_active(false), m_theme(nullptr){};
    t_size get_ideal_height() { return uGetFontHeight(m_font) * 2 + 6 + 6; }
    void enter_menu_mode(const char* p_text)
    {
        m_menu_active = true;
        m_menu_text = p_text;
        invalidate_all();
    }
    void exit_menu_mode()
    {
        m_menu_active = false;
        m_menu_text.reset();
        invalidate_all();
    }
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    void on_font_changed();

    static const GUID g_guid_font;

private:
    bool m_selection;
    t_size m_item_count;
    pfc::string8 m_length_text;
    pfc::string8 m_track_label;
    pfc::string8_fast_aggressive playing1;
    pfc::string8 m_menu_text;
    bool m_menu_active;
    gdi_object_t<HFONT>::ptr_t m_font;
    HTHEME m_theme;
};
