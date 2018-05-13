#include "stdafx.h"

#include "drop_down_list_toolbar.h"

struct PlaybackOrderToolbarArgs {
    using ID = GUID;
    using ItemList = std::vector<std::tuple<ID, std::string>>;

    static auto get_items()
    {
        auto api = playlist_manager_v4::get();
        const auto count = api->playback_order_get_count();
        ItemList items(count);

        for (unsigned i = 0; i < count; i++) {
            items[i] = std::make_tuple(api->playback_order_get_guid(i), api->playback_order_get_name(i));
        }
        return items;
    }

    static ID get_active_item()
    {
        auto api = playlist_manager_v4::get();
        return api->playback_order_get_guid(api->playback_order_get_active());
    }

    static void set_active_item(ID id)
    {
        auto api = playlist_manager_v4::get();
        auto items = get_items();
        const auto iter
            = std::find_if(items.begin(), items.end(), [id](auto&& item) { return std::get<ID>(item) == id; });

        if (iter != items.end())
            api->playback_order_set_active(iter - items.begin());
    }

    static void get_menu_items(uie::menu_hook_t& p_hook) {}
    static constexpr void on_first_window_created() {}
    static constexpr void on_last_window_destroyed() {}
    static constexpr bool is_available() { return true; }
    static constexpr bool refresh_on_click = false;
    static constexpr const wchar_t* class_name{L"columns_ui_playback_order_i3z1Bci1KNo"};
    static constexpr const char* name{"Playback order"};
    static constexpr GUID extension_guid{0xaba09e7e, 0x9c95, 0x443e, {0xbd, 0xfc, 0x4, 0x9d, 0x66, 0xb3, 0x24, 0xa0}};
};

ui_extension::window_factory<DropDownListToolbar<PlaybackOrderToolbarArgs>> playback_order_toolbar;

class OrderToolbarPlaylistCallback : public playlist_callback_single_static {
public:
    void on_items_added(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const bit_array& p_selection) override{};
    void on_items_reordered(const t_size* p_order, t_size p_count) override{};
    void on_items_removing(const bit_array& p_mask, t_size p_old_count, t_size p_new_count) override{};
    void on_items_removed(const bit_array& p_mask, t_size p_old_count, t_size p_new_count) override{};
    void on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) override{};
    void on_item_focus_change(t_size p_from, t_size p_to) override{};
    void on_items_modified(const bit_array& p_mask) override{};
    void on_items_modified_fromplayback(const bit_array& p_mask, play_control::t_display_level p_level) override{};
    void on_items_replaced(const bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override{};
    void on_item_ensure_visible(t_size p_idx) override{};
    void on_playlist_switch() override{};
    void on_playlist_renamed(const char* p_new_name, t_size p_new_name_len) override{};
    void on_playlist_locked(bool p_locked) override{};
    void on_default_format_changed() override{};
    void on_playback_order_changed(t_size p_new_index) override
    {
        DropDownListToolbar<PlaybackOrderToolbarArgs>::s_update_active_item_safe();
    }
    unsigned get_flags() override { return flag_on_playback_order_changed; };
};

service_factory_single_t<OrderToolbarPlaylistCallback> order_toolbar_playlist_callback;
