#include "pch.h"

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

    static constexpr void on_first_window_created() {}
    static constexpr void on_last_window_destroyed() {}
    static constexpr bool is_available() { return true; }
    static constexpr bool refresh_on_click = false;
    static constexpr auto no_items_text = ""sv;
    static constexpr const wchar_t* class_name{L"columns_ui_playback_order_i3z1Bci1KNo"};
    static constexpr const char* name{"Playback order"};
    static constexpr GUID extension_guid{0xaba09e7e, 0x9c95, 0x443e, {0xbd, 0xfc, 0x4, 0x9d, 0x66, 0xb3, 0x24, 0xa0}};
    static constexpr GUID colour_client_id{
        0x9ab1c765, 0x31c7, 0x4ae9, {0xbf, 0xc1, 0xc6, 0x65, 0x76, 0xbf, 0x24, 0x83}};
    static constexpr GUID font_client_id{0xb1ace74e, 0xddc0, 0x451f, {0x9a, 0x9a, 0xfc, 0x49, 0xdc, 0x4d, 0x5c, 0xfc}};
};

ui_extension::window_factory<DropDownListToolbar<PlaybackOrderToolbarArgs>> playback_order_toolbar;

class OrderToolbarPlaylistCallback : public playlist_callback_single_static {
public:
    void on_items_added(
        size_t p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection) override
    {
    }
    void on_items_reordered(const size_t* p_order, size_t p_count) override {}
    void on_items_removing(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override {}
    void on_items_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) override {}
    void on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) override {}
    void on_item_focus_change(size_t p_from, size_t p_to) override {}
    void on_items_modified(const bit_array& p_mask) override {}
    void on_items_modified_fromplayback(const bit_array& p_mask, play_control::t_display_level p_level) override {}
    void on_items_replaced(const bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override
    {
    }
    void on_item_ensure_visible(size_t p_idx) override {}
    void on_playlist_switch() override {}
    void on_playlist_renamed(const char* p_new_name, size_t p_new_name_len) override {}
    void on_playlist_locked(bool p_locked) override {}
    void on_default_format_changed() override {}
    void on_playback_order_changed(size_t p_new_index) noexcept override
    {
        DropDownListToolbar<PlaybackOrderToolbarArgs>::s_update_active_item_safe();
    }
    unsigned get_flags() override { return flag_on_playback_order_changed; }
};

service_factory_single_t<OrderToolbarPlaylistCallback> order_toolbar_playlist_callback;
