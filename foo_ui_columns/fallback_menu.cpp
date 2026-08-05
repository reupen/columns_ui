#include "pch.h"

#include "fallback_menu.h"
#include "menu_mnemonics.h"
#include "win32.h"

namespace cui {

void FallbackMenu::show(HWND wnd)
{
    show_internal(wnd, {});
}

void FallbackMenu::show_for_accelerator(HWND wnd, wchar_t accelerator)
{
    initialise();

    if (ranges::find_if(m_root_items, [&](const auto& item) { return item.accelerator == accelerator; })
        == ranges::end(m_root_items))
        return;

    fb2k::inMainThread([this, self{weak_from_this()}, wnd, accelerator] {
        if (self.expired())
            return;

        show_internal(wnd, accelerator);
    });
}

bool FallbackMenu::on_menu_select(uint32_t id, uint32_t flags) const
{
    if (!m_status_text_override.is_valid() || m_root_items.empty() || m_base_id_increment == 0)
        return false;

    if (flags & MF_POPUP) {
        m_status_text_override->revert_text();
        return true;
    }

    const auto root_group_index = (id - 1) / m_base_id_increment;

    if (root_group_index >= m_root_items.size())
        return false;

    pfc::string8 description;

    m_root_items[root_group_index].manager->get_description(
        id - 1 - m_base_id_increment * gsl::narrow<uint32_t>(root_group_index), description);

    if (description.empty())
        m_status_text_override->revert_text();
    else
        m_status_text_override->override_text(description);

    return true;
}

void FallbackMenu::initialise()
{
    if (m_initialised)
        return;

    m_initialised = true;

    MnemonicManager mnemonic_manager;

    for (const auto group : mainmenu_group::enumerate()) {
        if (group->get_parent() != GUID{})
            continue;

        mainmenu_group_popup::ptr popup_group;
        popup_group &= group;

        if (!popup_group.is_valid())
            continue;

        FallbackMenuRootItem root_item;

        std::string name;
        mmh::StringAdaptor adapted_name(name);
        popup_group->get_display_string(adapted_name);

        mmh::StringAdaptor adapted_name_with_accelerator(root_item.name_with_accelerator);
        root_item.accelerator = mnemonic_manager.process_string(name.c_str(), adapted_name_with_accelerator);

        root_item.sort_priority = popup_group->get_sort_priority();
        root_item.manager = mainmenu_manager::get();
        root_item.manager->instantiate(popup_group->get_guid());

        m_root_items.emplace_back(std::move(root_item));
    }

    std::ranges::sort(
        m_root_items, [](const auto& left, const auto& right) { return left.sort_priority < right.sort_priority; });

    m_base_id_increment = UINT16_MAX / gsl::narrow<uint16_t>(m_root_items.size());
}

void FallbackMenu::show_internal(HWND wnd, std::optional<wchar_t> accelerator)
{
    initialise();

    if (m_root_items.empty() || m_base_id_increment == 0)
        return;

    const auto menu_flags
        = standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? mainmenu_manager::flag_show_shortcuts : 0;

    uih::Menu menu;
    HMENU accelerator_menu{};

    for (auto&& [index, root_item] : ranges::views::enumerate(m_root_items)) {
        uih::Menu submenu;

        if (accelerator && root_item.accelerator == accelerator)
            accelerator_menu = submenu.get();

        root_item.manager->generate_menu_win32(
            submenu.get(), 1 + m_base_id_increment * gsl::narrow<uint32_t>(index), m_base_id_increment, menu_flags);
        menu.append_submenu(std::move(submenu), mmh::to_utf16(root_item.name_with_accelerator.c_str()));
    }

    menu_helpers::win32_auto_mnemonics(menu.get());

    fb2k::inMainThread([accelerator, accelerator_menu, menu{menu.get()}] {
        const auto menu_wnd = win32::find_window_for_menu(menu);

        if (!menu_wnd)
            return;

        if (accelerator && accelerator_menu) {
            SendMessage(menu_wnd, WM_CHAR, *accelerator, 0);

            if (const auto submenu_wnd = win32::find_window_for_menu(accelerator_menu))
                PostMessage(submenu_wnd, WM_KEYDOWN, VK_DOWN, 0);
        } else {
            PostMessage(menu_wnd, WM_KEYDOWN, VK_DOWN, 0);
        }
    });

    const auto self = shared_from_this();

    ui_control::get()->override_status_text_create(m_status_text_override);

    POINT pt{};
    ClientToScreen(wnd, &pt);
    const auto id = menu.run(wnd, pt, {.is_right_button = false});

    m_status_text_override.reset();

    if (id == 0)
        return;

    const auto root_group_index = (id - 1) / m_base_id_increment;

    if (root_group_index >= m_root_items.size())
        return;

    m_root_items[root_group_index].manager->execute_command(
        id - 1 - m_base_id_increment * gsl::narrow<uint32_t>(root_group_index));
}

} // namespace cui
