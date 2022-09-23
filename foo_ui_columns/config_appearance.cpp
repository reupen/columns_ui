#include "pch.h"

#include "config_appearance.h"

#include "config_host.h"
#include "main_window.h"
#include "system_appearance_manager.h"
#include "tab_colours.h"
#include "tab_dark_mode.h"
#include "tab_fonts.h"
#include "setup_dialog.h"

TabDarkMode g_tab_dark_mode;
cui::colours::ColourManagerData g_colour_manager_data;
FontManagerData g_font_manager_data;
TabColours g_tab_appearance;
TabFonts g_tab_appearance_fonts;

namespace cui::colours {

namespace {

const GUID dark_mode_status_id = {0x1278cd90, 0x1d95, 0x48e8, {0x87, 0x3a, 0x1, 0xf9, 0xad, 0x2d, 0xbc, 0x5f}};

}

fbh::ConfigInt32 dark_mode_status(
    dark_mode_status_id, WI_EnumValue(DarkModeStatus::Disabled), [](auto&& new_value, auto&& old_value) {
        g_tab_dark_mode.refresh();
        QuickSetupDialog::s_refresh();

        const auto old_enabled = old_value == WI_EnumValue(DarkModeStatus::Enabled)
            || (old_value == WI_EnumValue(DarkModeStatus::UseSystemSetting)
                && system_appearance_manager::is_dark_mode_enabled());

        const auto new_enabled = new_value == WI_EnumValue(DarkModeStatus::Enabled)
            || (new_value == WI_EnumValue(DarkModeStatus::UseSystemSetting)
                && system_appearance_manager::is_dark_mode_enabled());

        if (old_enabled == new_enabled)
            return;

        handle_effective_dark_mode_status_change();
    });

void handle_effective_dark_mode_status_change()
{
    const auto wnd = main_window.get_wnd();
    bool is_redraw_disabled{};

    if (wnd) {
        is_redraw_disabled = GetProp(wnd, L"SysSetRedraw") != nullptr;
        if (!is_redraw_disabled) {
            SetWindowRedraw(wnd, FALSE);
        }
    }

    auto _ = gsl::finally([wnd, is_redraw_disabled] {
        if (wnd && !is_redraw_disabled) {
            SetWindowRedraw(wnd, TRUE);
            RedrawWindow(wnd, nullptr, nullptr,
                RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW);
            main_window.set_dark_mode_attributes(true);
        }
    });

    common_colour_callback_manager.s_on_common_bool_changed(bool_flag_dark_mode_enabled);
    common_colour_callback_manager.s_on_common_colour_changed(colour_flag_all);
    ColoursClientList colours_clients;
    ColoursClientList::g_get_list(colours_clients);

    for (auto&& client : colours_clients) {
        if (client.m_ptr->get_supported_bools() & bool_flag_dark_mode_enabled)
            client.m_ptr->on_bool_changed(bool_flag_dark_mode_enabled);

        client.m_ptr->on_colour_changed(colour_flag_all);
    }

    g_tab_appearance.handle_external_configuration_change();
}

bool handle_system_dark_mode_status_change()
{
    if (dark_mode_status == WI_EnumValue(DarkModeStatus::UseSystemSetting)) {
        handle_effective_dark_mode_status_change();
        return true;
    }
    return false;
}

bool handle_system_dark_mode_availability_change()
{
    g_tab_dark_mode.refresh();

    if (dark_mode_status == WI_EnumValue(DarkModeStatus::Enabled)) {
        handle_effective_dark_mode_status_change();
        return true;
    }
    return false;
}

} // namespace cui::colours

cui::colours::ColourScheme g_get_global_colour_scheme(bool is_dark)
{
    const auto ptr = g_colour_manager_data.get_global_entry(is_dark);
    return ptr->colour_set.colour_scheme;
}

void g_set_global_colour_scheme(cui::colours::ColourScheme scheme, bool is_dark)
{
    const auto ptr = g_colour_manager_data.get_global_entry(is_dark);
    if (ptr->colour_set.colour_scheme == scheme)
        return;

    ptr->colour_set.colour_scheme = scheme;

    if (is_dark != cui::colours::is_dark_mode_active())
        return;

    cui::colours::common_colour_callback_manager.s_on_common_colour_changed(cui::colours::colour_flag_all);
    if (g_tab_appearance.is_active()) {
        g_tab_appearance.handle_external_configuration_change();
    }
    ColoursClientList m_colours_client_list;
    ColoursClientList::g_get_list(m_colours_client_list);
    size_t count = m_colours_client_list.get_count();
    for (size_t i = 0; i < count; i++) {
        const auto p_data = g_colour_manager_data.get_entry(m_colours_client_list[i].m_guid);
        if (p_data->colour_set.colour_scheme == cui::colours::ColourSchemeGlobal)
            m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
    }
}

void on_global_colours_change()
{
    if (g_tab_appearance.is_active()) {
        g_tab_appearance.handle_external_configuration_change();
    }
    cui::colours::common_colour_callback_manager.s_on_common_colour_changed(cui::colours::colour_flag_all);
    ColoursClientList m_colours_client_list;
    ColoursClientList::g_get_list(m_colours_client_list);
    size_t count = m_colours_client_list.get_count();
    for (size_t i = 0; i < count; i++) {
        const auto p_data = g_colour_manager_data.get_entry(m_colours_client_list[i].m_guid);
        if (p_data->colour_set.colour_scheme == cui::colours::ColourSchemeGlobal)
            m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
    }
}

namespace fonts {
// {82196D79-69BC-4041-8E2A-E3B4406BB6FC}
const GUID columns_playlist_items = {0x82196d79, 0x69bc, 0x4041, {0x8e, 0x2a, 0xe3, 0xb4, 0x40, 0x6b, 0xb6, 0xfc}};

// {B9D5EA18-5827-40be-A896-302A71BCAA9C}
const GUID status_bar = {0xb9d5ea18, 0x5827, 0x40be, {0xa8, 0x96, 0x30, 0x2a, 0x71, 0xbc, 0xaa, 0x9c}};

// {C0D3B76C-324D-46d3-BB3C-E81C7D3BCB85}
const GUID columns_playlist_header = {0xc0d3b76c, 0x324d, 0x46d3, {0xbb, 0x3c, 0xe8, 0x1c, 0x7d, 0x3b, 0xcb, 0x85}};

// {19F8E0B3-E822-4f07-B200-D4A67E4872F9}
const GUID ng_playlist_items = {0x19f8e0b3, 0xe822, 0x4f07, {0xb2, 0x0, 0xd4, 0xa6, 0x7e, 0x48, 0x72, 0xf9}};

// {30FBD64C-2031-4f0b-A937-F21671A2E195}
const GUID ng_playlist_header = {0x30fbd64c, 0x2031, 0x4f0b, {0xa9, 0x37, 0xf2, 0x16, 0x71, 0xa2, 0xe1, 0x95}};

// {6F000FC4-3F86-4fc5-80EA-F7AA4D9551E6}
const GUID splitter_tabs = {0x6f000fc4, 0x3f86, 0x4fc5, {0x80, 0xea, 0xf7, 0xaa, 0x4d, 0x95, 0x51, 0xe6}};

// {942C36A4-4E28-4cea-9644-F223C9A838EC}
const GUID playlist_tabs = {0x942c36a4, 0x4e28, 0x4cea, {0x96, 0x44, 0xf2, 0x23, 0xc9, 0xa8, 0x38, 0xec}};

// {70A5C273-67AB-4bb6-B61C-F7975A6871FD}
const GUID playlist_switcher = {0x70a5c273, 0x67ab, 0x4bb6, {0xb6, 0x1c, 0xf7, 0x97, 0x5a, 0x68, 0x71, 0xfd}};

// {D93F1EF3-4AEE-4632-B5BF-0220CEC76DED}
const GUID filter_items = {0xd93f1ef3, 0x4aee, 0x4632, {0xb5, 0xbf, 0x2, 0x20, 0xce, 0xc7, 0x6d, 0xed}};

// {FCA8752B-C064-41c4-9BE3-E125C7C7FC34}
const GUID filter_header = {0xfca8752b, 0xc064, 0x41c4, {0x9b, 0xe3, 0xe1, 0x25, 0xc7, 0xc7, 0xfc, 0x34}};
} // namespace fonts

void refresh_appearance_prefs()
{
    if (g_tab_appearance_fonts.is_active()) {
        g_tab_appearance_fonts.update_mode_combobox();
        g_tab_appearance_fonts.update_font_desc();
        g_tab_appearance_fonts.update_change();
    }
}

static PreferencesTab* g_tabs_appearance[] = {&g_tab_dark_mode, &g_tab_appearance, &g_tab_appearance_fonts};

// {FA25D859-C808-485d-8AB7-FCC10F29ECE5}
const GUID g_guid_cfg_child_appearance = {0xfa25d859, 0xc808, 0x485d, {0x8a, 0xb7, 0xfc, 0xc1, 0xf, 0x29, 0xec, 0xe5}};

cfg_int cfg_child_appearance(g_guid_cfg_child_appearance, 0);

// {41E6D7ED-A1DC-4d84-9BC9-352DAF7788B0}
constexpr GUID g_guid_colour_preferences
    = {0x41e6d7ed, 0xa1dc, 0x4d84, {0x9b, 0xc9, 0x35, 0x2d, 0xaf, 0x77, 0x88, 0xb0}};

static service_factory_single_t<PreferencesTabsHost> g_config_tabs("Colours and fonts", g_tabs_appearance,
    g_guid_colour_preferences, g_guid_columns_ui_preferences_page, cfg_child_appearance);

// {15FD4FF9-0622-4077-BFBB-DF0102B6A068}
const GUID cui::colours::ColourManagerData::g_cfg_guid
    = {0x15fd4ff9, 0x622, 0x4077, {0xbf, 0xbb, 0xdf, 0x1, 0x2, 0xb6, 0xa0, 0x68}};

// {6B71F91C-6B7E-4dbe-B27B-C493AA513FD0}
const GUID FontManagerData::g_cfg_guid = {0x6b71f91c, 0x6b7e, 0x4dbe, {0xb2, 0x7b, 0xc4, 0x93, 0xaa, 0x51, 0x3f, 0xd0}};
