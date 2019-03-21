#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"
#include "seekbar.h"
#include "main_window.h"
#include "notification_area.h"
#include "status_bar.h"

class config_object_notify_pfc : public config_object_notify {
public:
    t_size get_watched_object_count() override { return 1; }
    GUID get_watched_object(t_size p_index) override { return standard_config_objects::bool_playback_follows_cursor; }
    void on_watched_object_changed(const service_ptr_t<config_object>& p_object) override
    {
        bool val = p_object->get_data_bool_simple(false);
        pvt::ng_playlist_view_t::g_on_playback_follows_cursor_change(val);
    }
};

service_factory_single_t<config_object_notify_pfc> g_config_object_notify_pfc;

class play_callback_ui : public play_callback_static {
public:
    enum { flags = flag_on_playback_all | flag_on_volume_change };

    unsigned get_flags() override { return flags; }
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override
    {
        if (g_status) {
            statusbartext = "Loading track..";
            status_update_main(false);
        }
    }
    void on_playback_new_track(metadb_handle_ptr p_track) override
    {
        g_playing = true;
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
            update_systray(true);
            update_status();
            cui::main_window.queue_taskbar_button_update();
        }

        seek_bar_extension::update_seekbars();
        seek_bar_extension::update_seek_timer();
    }

    void on_playback_stop(play_control::t_stop_reason p_reason) override
    {
        g_playing = false;
        seek_bar_extension::update_seek_timer();
        if (cui::main_window.get_wnd() && p_reason != play_control::stop_reason_shutting_down) {
            cui::main_window.reset_title();

            if (g_icon_created)
                uShellNotifyIcon(NIM_MODIFY, cui::main_window.get_wnd(), 1, MSG_NOTICATION_ICON, g_icon,
                    core_version_info_v2::get()->get_name());
            statusbartext = core_version_info::g_get_version_string();
            status_update_main(false);
            cui::main_window.queue_taskbar_button_update();
        }
        if (p_reason != play_control::stop_reason_shutting_down)
            seek_bar_extension::update_seekbars();
    }
    void on_playback_seek(double p_time) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
            update_status();
        }
        seek_bar_extension::update_seekbars(true);
    }
    void on_playback_pause(bool b_state) override
    {
        g_playing = (b_state == 0);
        seek_bar_extension::update_seek_timer();
        if (cui::main_window.get_wnd()) {
            update_systray(true, b_state ? 2 : 1);
            cui::main_window.update_title();
            update_status();
            cui::main_window.queue_taskbar_button_update();
        }
    }

    void on_playback_edited(metadb_handle_ptr p_track) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
            update_status();
        }
    }

    void on_playback_dynamic_info(const file_info& p_info) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
            update_status();
        }
    }
    void on_playback_dynamic_info_track(const file_info& p_info) override
    {
        if (cui::main_window.get_wnd()) {
            update_systray(true);
            cui::main_window.update_title();
            update_status();
        }
    }
    void on_playback_time(double p_time) override
    {
        if (cui::main_window.get_wnd()) {
            cui::main_window.update_title();
            update_status();
        }
    }
    void on_volume_change(float p_new_val) override
    {
        if (cui::main_window.get_wnd() && g_status)
            status_bar::set_part_sizes(status_bar::t_part_volume);
    }
};

static play_callback_static_factory_t<play_callback_ui> blah;

class playlist_callback_columns : public playlist_callback_static {
    unsigned get_flags() override { return playlist_callback::flag_all; }

    void on_items_added(unsigned p_playlist, unsigned start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const pfc::bit_array& p_selection) override
    {
    }
    void on_items_reordered(unsigned p_playlist, const unsigned* order, unsigned count) override {}

    void on_items_removing(
        unsigned p_playlist, const pfc::bit_array& p_mask, unsigned p_old_count, unsigned p_new_count) override
    {
    }
    void on_items_removed(
        unsigned p_playlist, const pfc::bit_array& p_mask, unsigned p_old_count, unsigned p_new_count) override
    {
    }

    void on_items_selection_change(
        unsigned p_playlist, const pfc::bit_array& affected, const pfc::bit_array& state) override
    {
    }
    void on_item_focus_change(unsigned p_playlist, unsigned from, unsigned to) override {}
    void on_items_modified(unsigned p_playlist, const pfc::bit_array& p_mask) override {}
    void on_items_modified_fromplayback(
        unsigned p_playlist, const pfc::bit_array& p_mask, play_control::t_display_level p_level) override
    {
    }
    void on_items_replaced(unsigned p_playlist, const pfc::bit_array& p_mask,
        const pfc::list_base_const_t<t_on_items_replaced_entry>& p_data) override
    {
    }
    void on_item_ensure_visible(unsigned p_playlist, unsigned idx) override {}

    void on_playlist_activate(unsigned p_old, unsigned p_new) override
    {
        if (cui::main_window.get_wnd()) {
            if (g_status && main_window::config_get_status_show_lock())
                status_bar::set_part_sizes(status_bar::t_part_lock);
        }
    };

    void on_playlist_created(unsigned p_index, const char* p_name, unsigned p_name_len) override {}
    void on_playlists_reorder(const unsigned* p_order, unsigned p_count) override {}
    void on_playlists_removing(const pfc::bit_array& p_mask, unsigned p_old_count, unsigned p_new_count) override {}
    void on_playlists_removed(const pfc::bit_array& p_mask, unsigned p_old_count, unsigned p_new_count) override {}
    void on_playlist_renamed(unsigned p_index, const char* p_new_name, unsigned p_new_name_len) override {}

    void on_default_format_changed() override{};
    void on_playback_order_changed(unsigned p_new_index) override{}
    void on_playlist_locked(unsigned p_playlist, bool p_locked) override
    {
        if (cui::main_window.get_wnd()) {
            if (g_status && main_window::config_get_status_show_lock())
                status_bar::set_part_sizes(status_bar::t_part_lock);
        }
    };

    virtual void on_item_replaced(unsigned pls, unsigned item, metadb_handle* from, metadb_handle* to) {}
};

static service_factory_single_t<playlist_callback_columns> foo9;
