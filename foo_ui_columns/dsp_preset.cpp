#include "stdafx.h"

#include "drop_down_list_toolbar.h"
#include "panel_menu_item.h"

// Not exposed in the foobar2000 SDK
constexpr GUID dsp_manager_page_id{0xEB1878C9, 0x4B31, 0x46E3, {0x95, 0x2B, 0x6F, 0x7E, 0x1F, 0xD3, 0x63, 0xDD}};

struct DspPresetToolbarArgs {
    using ID = size_t;
    using ItemList = std::vector<std::tuple<ID, std::string>>;
    static ItemList get_items()
    {
        if (!is_available())
            return {};

        auto api = dsp_config_manager_v2::get();
        const auto count = api->get_preset_count();
        ItemList items(count);

        for (ID i{0}; i < count; ++i) {
            pfc::string8 name;
            api->get_preset_name(i, name);
            items[i] = std::make_tuple(i, name.c_str());
        }
        return items;
    }
    static ID get_active_item()
    {
        if (!is_available())
            return ID{};

        auto api = dsp_config_manager_v2::get();
        return api->get_selected_preset();
    }
    static void set_active_item(ID id)
    {
        if (!is_available())
            return;

        auto api = dsp_config_manager_v2::get();
        api->select_preset(id);
    }
    static void get_menu_items(uie::menu_hook_t& p_hook)
    {
        p_hook.add_node(new cui::panel_helpers::CommandMenuNode{
            "DSP Manager", [] { ui_control::get()->show_preferences(dsp_manager_page_id); }});
    }
    static constexpr void on_first_window_created() {}
    static constexpr void on_last_window_destroyed() {}
    static bool is_available() { return static_api_test_t<dsp_config_manager_v2>(); }
    static constexpr bool refresh_on_click = true;
    static constexpr const wchar_t* class_name{L"columns_ui_dsp_preset_TB7ds8Gd8SMzVA"};
    static constexpr const char* name{"DSP preset"};
    static constexpr GUID extension_guid{0x9bd325a4, 0xb2e6, 0x47ad, {0x9b, 0x54, 0x8d, 0xa3, 0x5f, 0x42, 0x78, 0x49}};
};

ui_extension::window_factory<DropDownListToolbar<DspPresetToolbarArgs>> dsp_preset_toolbar;

class DspPresetToolbarConfigCallback : public dsp_config_callback {
public:
    void on_core_settings_change(const dsp_chain_config& p_newdata) override
    {
        DropDownListToolbar<DspPresetToolbarArgs>::s_refresh_all_items_safe();
    }
};

static service_factory_single_t<DspPresetToolbarConfigCallback> dsp_preset_toolbar_config_callback;
