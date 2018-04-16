#include "stdafx.h"

#include "drop_down_list_toolbar.h"

struct OutputDeviceToolbarArgs {
    using ID = std::tuple<GUID, GUID>;
    using ItemList = std::vector<std::tuple<ID, std::string>>;
    static auto get_items()
    {
        ItemList items;

        if (!is_available())
            return items;

        auto api = output_manager_v2::get();

        api->listDevices([&items](auto&& name, auto&& output_id, auto&& device_id) {
            items.emplace_back(std::make_tuple(output_id, device_id), name);
        });

        return items;
    }
    static ID get_active_item()
    {
        if (!is_available())
            return ID{};

        auto api = output_manager_v2::get();
        outputCoreConfig_t config{};
        api->getCoreConfig(config);
        return std::make_tuple(config.m_output, config.m_device);
    }
    static void set_active_item(ID id)
    {
        if (!is_available())
            return;

        auto api = output_manager_v2::get();
        auto items = get_items();
        const auto iter
            = std::find_if(items.begin(), items.end(), [id](auto&& item) { return std::get<ID>(item) == id; });

        if (iter != items.end()) {
            auto&& [output_id, device_id] = std::get<ID>(*iter);
            api->setCoreConfigDevice(output_id, device_id);
        }
    }
    static void on_first_window_created()
    {
        if (!is_available())
            return;

        auto api = output_manager_v2::get();
        callback_handle = api->addCallback([] { DropDownListToolbar<OutputDeviceToolbarArgs>::s_refresh_all_items(); });
    }
    static void on_last_window_destroyed() { callback_handle.release(); }
    static bool is_available() { return static_api_test_t<output_manager_v2>(); }
    static service_ptr callback_handle;
    static constexpr bool refresh_on_click = true;
    static constexpr const wchar_t* class_name{L"columns_ui_output_device_DQLvIKXzFVY"};
    static constexpr const char* name{"Output device"};
    static constexpr GUID extension_guid{0xc25e4fe6, 0xb9a0, 0x48c3, {0xa4, 0xc0, 0x44, 0x3d, 0x69, 0xda, 0xd3, 0x6d}};
};

service_ptr OutputDeviceToolbarArgs::callback_handle;

ui_extension::window_factory<DropDownListToolbar<OutputDeviceToolbarArgs>> output_device_toolbar;
