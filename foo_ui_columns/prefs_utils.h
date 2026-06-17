#pragma once

void populate_menu_combo(HWND wnd, unsigned ID, unsigned ID_DESC, const MenuItemIdentifier& p_item,
    const std::vector<MenuItemInfo>& p_cache, bool insert_none);
void on_menu_combo_change(HWND wnd, LPARAM lp, class ConfigMenuItem& cfg_menu_store,
    const std::vector<MenuItemInfo>& p_cache, unsigned ID_DESC);

namespace cui::prefs {

HFONT create_default_ui_font(unsigned point_size);
HFONT create_default_title_font();
void show_generic_title_formatting_tools_menu(HWND dialog_wnd, HWND button_wnd, GUID font_id);

[[nodiscard]] auto bind_checkbox(
    HWND control_wnd, auto& config_item, config::SourceID ignored_source_id = config::SourceID::Preferences)
{
    if (config_item)
        Button_SetCheck(control_wnd, BST_CHECKED);

    auto on_change = [control_wnd, ignored_source_id, &config_item](auto source_id) {
        if (source_id != WI_EnumValue(ignored_source_id))
            Button_SetCheck(control_wnd, config_item ? BST_CHECKED : BST_UNCHECKED);
    };

    if constexpr (requires { config_item.on_change([](auto, auto) {}); })
        return config_item.on_change([on_change](auto new_value, auto source_id) { on_change(source_id); });
    else
        return config_item.on_change(
            [on_change](auto new_value, auto old_value, auto source_id) { on_change(source_id); });
}

[[nodiscard]] auto bind_checkbox(HWND dialog_wnd, UINT control_id, auto& config_item,
    config::SourceID ignored_source_id = config::SourceID::Preferences)
{
    return bind_checkbox(GetDlgItem(dialog_wnd, control_id), config_item, ignored_source_id);
}

} // namespace cui::prefs
