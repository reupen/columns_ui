#pragma once

#include "drop_down_list_toolbar.h"

template <typename BaseArgs>
struct CoreDropDownToolbarArgs : BaseArgs {
    using ID = size_t;
    using ItemList = std::vector<std::tuple<ID, std::string>>;

    class CoreDropDownToolbarCallback : public fb2k::toolbarDropDownNotify {
    public:
        void contentChanged() override { DropDownListToolbar<CoreDropDownToolbarArgs>::s_refresh_all_items_safe(); }
        void selectionChanged() override { DropDownListToolbar<CoreDropDownToolbarArgs>::s_update_active_item_safe(); }
    };

    static fb2k::toolbarDropDown::ptr get_api()
    {
        for (auto api : fb2k::toolbarDropDown::enumerate()) {
            if (api->getGuid() == BaseArgs::core_toolbar_id)
                return api;
        }

        return {};
    }

    static auto get_items()
    {
        ItemList items;

        const auto api = get_api();

        if (api.is_empty())
            return items;

        for (const auto index : std::ranges::views::iota(size_t{0}, api->getNumValues())) {
            pfc::string8 value;
            api->getValue(index, value);

            auto should_include_value{true};

            if constexpr (requires() { BaseArgs::ignored_value; })
                should_include_value = stricmp_utf8(value, BaseArgs::ignored_value) != 0;

            if (should_include_value)
                items.emplace_back(index, value);
        }

        return items;
    }

    static ID get_active_item()
    {
        const auto api = get_api();

        if (api.is_empty())
            return ID{};

        return api->getSelectedIndex();
    }

    static void set_active_item(ID id)
    {
        const auto api = get_api();

        if (api.is_empty())
            return;

        api->setSelectedIndex(id);
    }

    static void on_click()
    {
        const auto api = get_api();

        if (api.is_empty())
            return;

        api->onDropDown();
    }

    static void on_first_window_created()
    {
        const auto api = get_api();

        if (api.is_empty())
            return;

        s_callback = std::make_unique<CoreDropDownToolbarCallback>();
        api->addNotify(s_callback.get());
    }

    static void on_last_window_destroyed()
    {
        const auto api = get_api();

        if (api.is_empty())
            return;

        api->removeNotify(s_callback.get());
        s_callback.reset();
    }

    static bool is_available() { return get_api().is_valid(); }
    inline static std::unique_ptr<CoreDropDownToolbarCallback> s_callback;
    static constexpr bool refresh_on_click = false;
};
