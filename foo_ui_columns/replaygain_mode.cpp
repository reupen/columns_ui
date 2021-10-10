#include "stdafx.h"

#include "drop_down_list_toolbar.h"
#include "panel_menu_item.h"

class ReplayGainCoreSettingsNotifyLambda : public replaygain_core_settings_notify {
public:
    using Callback = std::function<void(const t_replaygain_config&)>;
    ReplayGainCoreSettingsNotifyLambda(Callback calback) : m_callback{std::move(calback)} {}

    void on_changed(const t_replaygain_config& cfg) override { m_callback(cfg); }

    Callback m_callback;
};

struct ReplayGainModeToolbarArgs {
    using ID = uint32_t;
    using ItemList = std::vector<std::tuple<ID, std::string>>;
    static ItemList get_items()
    {
        if (!is_available())
            return {};

        return {
            std::make_tuple(t_replaygain_config::source_mode_none, "Disable ReplayGain"),
            std::make_tuple(t_replaygain_config::source_mode_track, "Prefer track gain"),
            std::make_tuple(t_replaygain_config::source_mode_album, "Prefer album gain"),
            std::make_tuple(t_replaygain_config::source_mode_byPlaybackOrder, "By playback order"),
        };
    }
    static ID get_active_item()
    {
        if (!is_available())
            return ID{};

        auto api = replaygain_manager_v2::get();
        t_replaygain_config config{};
        api->get_core_settings(config);
        return config.m_source_mode;
    }
    static void set_active_item(ID id)
    {
        if (!is_available())
            return;

        auto rg_api = replaygain_manager_v2::get();
        t_replaygain_config config{};
        rg_api->get_core_settings(config);
        config.m_source_mode = id;
        rg_api->set_core_settings(config);

        auto playback_api = playback_control_v3::get();
        playback_api->restart();
    }
    static void get_menu_items(uie::menu_hook_t& p_hook)
    {
        p_hook.add_node(new cui::panel_helpers::CommandMenuNode{
            "ReplayGain options", [] { ui_control::get()->show_preferences(preferences_page::guid_playback); }});
    }
    static void on_first_window_created()
    {
        if (!is_available())
            return;
        callback = std::make_unique<ReplayGainCoreSettingsNotifyLambda>(
            [](auto&&) { DropDownListToolbar<ReplayGainModeToolbarArgs>::s_refresh_all_items_safe(); });
        auto api = replaygain_manager_v2::get();
        api->add_notify(callback.get());
    }
    static void on_last_window_destroyed()
    {
        auto api = replaygain_manager_v2::get();
        api->remove_notify(callback.get());
    }
    static bool is_available() { return static_api_test_t<replaygain_manager_v2>(); }
    static std::unique_ptr<ReplayGainCoreSettingsNotifyLambda> callback;
    static constexpr bool refresh_on_click = false;
    static constexpr auto no_items_text = ""sv;
    static constexpr const wchar_t* class_name{L"columns_ui_replaygain_mode_-bdvzDNKnwDniA"};
    static constexpr const char* name{"ReplayGain mode"};
    static constexpr GUID extension_guid{0xad9a81f7, 0x723a, 0x4cce, {0x87, 0xb6, 0x13, 0x39, 0xb, 0xda, 0xc2, 0x16}};
    static constexpr GUID colour_client_id{0x3096eaa4, 0xf97a, 0x4ff8, {0x84, 0xbd, 0xc6, 0xa, 0xcd, 0x69, 0x5e, 0xf9}};
    static constexpr GUID font_client_id{0x25df8f6b, 0x78e9, 0x4555, {0xaf, 0x63, 0xa9, 0xc5, 0x8f, 0x2a, 0xbe, 0xe9}};
};

std::unique_ptr<ReplayGainCoreSettingsNotifyLambda> ReplayGainModeToolbarArgs::callback;

ui_extension::window_factory<DropDownListToolbar<ReplayGainModeToolbarArgs>> replaygain_mode_toolbar;
