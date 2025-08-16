#pragma once

namespace cui::panel_utils {

class PanelInfo {
public:
    GUID id{};
    std::wstring name;
    std::wstring category;
    bool is_single_instance{};
    uint32_t type{};
};

template <class WindowIterable = decltype(uie::window::enumerate())>
std::vector<PanelInfo> get_panel_info(const WindowIterable& windows = uie::window::enumerate())
{
    std::vector<PanelInfo> panels;

    for (auto&& window : windows) {
        PanelInfo info;
        info.id = window->get_extension_guid();

        pfc::string8 name;
        window->get_name(name);
        info.name = mmh::to_utf16(name.c_str());

        pfc::string8 category;
        window->get_category(category);
        info.category = mmh::to_utf16(category.c_str());

        info.is_single_instance = window->get_is_single_instance();
        info.type = window->get_type();

        panels.emplace_back(std::move(info));
    }

    std::ranges::sort(panels, [](const PanelInfo& left, const PanelInfo& right) {
        int result = StrCmpLogicalW(left.category.c_str(), right.category.c_str());

        if (result == 0)
            result = StrCmpLogicalW(left.name.c_str(), right.name.c_str());

        return result < 0;
    });

    return panels;
}

inline auto get_grouped_panel_info(const std::vector<PanelInfo>& panels)
{
    return panels | ranges::views::enumerate | ranges::views::chunk_by([](const auto& left, const auto& right) {
        return StrCmpLogicalW(left.second.category.c_str(), right.second.category.c_str()) == 0;
    });
}

} // namespace cui::panel_utils
