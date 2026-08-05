#pragma once

namespace cui {

struct FallbackMenuRootItem {
    std::string name_with_accelerator;
    std::optional<wchar_t> accelerator;
    uint32_t sort_priority{};
    mainmenu_manager::ptr manager;
};

class FallbackMenu : public std::enable_shared_from_this<FallbackMenu> {
public:
    using Ptr = std::shared_ptr<FallbackMenu>;

    void show(HWND wnd);
    void show_for_accelerator(HWND wnd, wchar_t accelerator);
    bool on_menu_select(uint32_t id, uint32_t flags) const;

private:
    void initialise();
    void show_internal(HWND wnd, std::optional<wchar_t> accelerator = {});

    bool m_initialised{};
    std::vector<FallbackMenuRootItem> m_root_items;
    ui_status_text_override::ptr m_status_text_override;
    uint32_t m_base_id_increment{};
};

} // namespace cui
