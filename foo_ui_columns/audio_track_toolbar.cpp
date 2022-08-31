#include "pch.h"

#include "drop_down_list_toolbar.h"

namespace cui {

namespace {

constexpr GUID stream_selector_api_id{0x6A6FF0B0, 0x3FB8, 0x4413, {0xBB, 0x76, 0xFF, 0x48, 0xC8, 0x8F, 0xF0, 0x57}};

fb2k::toolbarDropDown::ptr get_stream_selector_api()
{
    for (auto api : fb2k::toolbarDropDown::enumerate()) {
        if (api->getGuid() == stream_selector_api_id)
            return api;
    }

    return {};
}

struct AudioTrackToolbarArgs {
    using ID = size_t;
    using ItemList = std::vector<std::tuple<ID, std::string>>;

    class AudioTrackCallback : public fb2k::toolbarDropDownNotify {
    public:
        void contentChanged() override { DropDownListToolbar<AudioTrackToolbarArgs>::s_refresh_all_items_safe(); }
        void selectionChanged() override { DropDownListToolbar<AudioTrackToolbarArgs>::s_update_active_item_safe(); }
    };

    static auto get_items()
    {
        ItemList items;

        const auto api = get_stream_selector_api();

        if (api.is_empty())
            return items;

        for (const auto index : std::ranges::views::iota(size_t{0}, api->getNumValues())) {
            pfc::string8 value;
            api->getValue(index, value);

            if (stricmp_utf8(value, "not playing") != 0)
                items.emplace_back(index, value);
        }

        return items;
    }

    static ID get_active_item()
    {
        const auto api = get_stream_selector_api();

        if (api.is_empty())
            return ID{};

        return api->getSelectedIndex();
    }

    static void set_active_item(ID id)
    {
        const auto api = get_stream_selector_api();

        if (api.is_empty())
            return;

        api->setSelectedIndex(id);
    }

    static void on_click()
    {
        const auto api = get_stream_selector_api();

        if (api.is_empty())
            return;

        api->onDropDown();
    }

    static void get_menu_items(uie::menu_hook_t& p_hook) {}

    static void on_first_window_created()
    {
        const auto api = get_stream_selector_api();

        if (api.is_empty())
            return;

        s_callback = std::make_unique<AudioTrackCallback>();
        api->addNotify(s_callback.get());
    }

    static void on_last_window_destroyed()
    {
        const auto api = get_stream_selector_api();

        if (api.is_empty())
            return;

        api->removeNotify(s_callback.get());
        s_callback.reset();
    }

    static bool is_available() { return get_stream_selector_api().is_valid(); }
    inline static std::unique_ptr<AudioTrackCallback> s_callback;
    static constexpr bool refresh_on_click = false;
    static constexpr auto no_items_text = "(not playing)"sv;
    static constexpr const wchar_t* class_name{L"columns_ui_audio_track_toolbar_xvkMz8coqQY"};
    static constexpr const char* name{"Audio track"};
    static constexpr GUID extension_guid{0xee6d2fed, 0x158, 0x4fa9, {0xaf, 0x1c, 0x72, 0xf2, 0x34, 0x56, 0xb7, 0x8c}};
    static constexpr GUID colour_client_id{0x91f80f2e, 0x4e58, 0x4600, {0xba, 0x4f, 0x13, 0xc7, 0xeb, 0x5, 0x7d, 0xf0}};
    static constexpr GUID font_client_id{0xe547f854, 0x1efe, 0x4fca, {0x8d, 0x42, 0xe4, 0x24, 0x99, 0x12, 0xbc, 0x9a}};
};

ui_extension::window_factory<DropDownListToolbar<AudioTrackToolbarArgs>> _;

} // namespace

} // namespace cui
