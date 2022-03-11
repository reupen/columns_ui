#include "pch.h"

#include "drop_down_list_toolbar.h"

struct OutputFormatToolbarArgs {
    using ID = uint32_t;
    using ItemList = std::vector<std::tuple<ID, std::string>>;
    static auto get_items()
    {
        outputCoreConfig_t config;
        output_manager_v2::get()->getCoreConfig(config);

        if (output_entry::ptr output; !output_entry::g_find(config.m_output, output)
            || !(output->get_config_flags() & output_entry::flag_needs_bitdepth_config))
            return ItemList{};

        static constexpr auto bit_depths = {8, 16, 24, 32};
        // clang-format off
        return bit_depths
            | ranges::views::filter([](ID bd) { return !(bd == 8 && core_version_info_v2::get()->test_version(1, 6, 7, 0)); })
            | ranges::views::transform([](ID bd) { return std::make_tuple(bd, std::to_string(bd) + "-bit"); })
            | ranges::to<ItemList>();
        // clang-format on
    }
    static ID get_active_item()
    {
        auto api = output_manager_v2::get();
        outputCoreConfig_t config;
        api->getCoreConfig(config);
        return config.m_bitDepth;
    }
    static void set_active_item(ID bitDepth)
    {
        auto api = output_manager_v2::get();
        outputCoreConfig_t config;
        api->getCoreConfig(config);
        config.m_bitDepth = bitDepth;
        api->setCoreConfig(config);
    }
    static void get_menu_items(uie::menu_hook_t& p_hook) {}
    static void on_first_window_created()
    {
        auto api = output_manager_v2::get();
        callback_handle
            = api->addCallback([] { DropDownListToolbar<OutputFormatToolbarArgs>::s_refresh_all_items_safe(); });
    }
    static void on_last_window_destroyed() { callback_handle.release(); }
    static bool is_available() { return true; }
    static service_ptr callback_handle;
    static constexpr bool refresh_on_click = false;
    static constexpr auto no_items_text = "Auto"sv;
    static constexpr const wchar_t* class_name{L"columns_ui_output_format_TBWOn9HkOxhkU"};
    static constexpr const char* name{"Output format"};
    static constexpr GUID extension_guid{0xa379ccd9, 0xbc38, 0x4e2b, {0x85, 0xd6, 0x97, 0x5d, 0x11, 0x7b, 0xdd, 0xcc}};
    static constexpr GUID colour_client_id{0xb5eaad91, 0x3939, 0x4cbe, {0x9a, 0x1f, 0xa9, 0xbb, 0xdc, 0xd0, 0x4, 0x8d}};
    static constexpr GUID font_client_id{0x1fb03f29, 0x1a1d, 0x4d11, {0xb5, 0x74, 0xaf, 0x2e, 0xb2, 0x62, 0x48, 0xc2}};
};

service_ptr OutputFormatToolbarArgs::callback_handle;

ui_extension::window_factory<DropDownListToolbar<OutputFormatToolbarArgs>> output_format_toolbar;
