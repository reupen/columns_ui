#include "stdafx.h"
#include "menu_items.h"
#include "layout.h"
#include "ng_playlist/ng_playlist.h"
#include "main_window.h"
#include "button_items.h"

namespace cui::main_menu {

namespace groups {

constexpr GUID view_columns_part = {0xae078354, 0x60ea, 0x48a9, {0x94, 0x5, 0xda, 0xa, 0xef, 0x0, 0xc5, 0x8d}};
constexpr GUID view_playlist_popup = {0x16610091, 0x8bd9, 0x4e38, {0x9d, 0xe5, 0x43, 0x29, 0xbe, 0x14, 0x12, 0x1b}};
constexpr GUID playlist_font_part = {0xcef3f5a, 0x2c3, 0x4865, {0x8f, 0x62, 0x37, 0x92, 0x80, 0xe4, 0xec, 0x3a}};
constexpr GUID playlist_misc_part = {0xf74d3c19, 0x91cc, 0x4179, {0x95, 0x4a, 0x7b, 0xe5, 0x4e, 0x9b, 0xbc, 0x80}};
constexpr GUID view_layout_popup = {0x533fdc34, 0xee1b, 0x4317, {0x9d, 0xc, 0x48, 0x9e, 0xe1, 0x21, 0x5d, 0x9f}};
constexpr GUID view_layout_commands = {0x964c3b5a, 0x1908, 0x4407, {0x9b, 0x8e, 0xfc, 0x73, 0x31, 0xa9, 0x9e, 0xd1}};
constexpr GUID view_layout_presets = {0xc36ef588, 0xabdf, 0x495e, {0xa7, 0xd4, 0xd0, 0x21, 0x84, 0x6f, 0x50, 0xee}};

static mainmenu_group_factory g_mainmenu_group_view_columns_part(
    view_columns_part, mainmenu_groups::view, mainmenu_commands::sort_priority_base + 1);

static mainmenu_group_popup_factory g_mainmenu_group_view_playlist_popup(
    view_playlist_popup, view_columns_part, mainmenu_commands::sort_priority_base, "Playlist view");

static mainmenu_group_popup_factory g_mainmenu_group_view_layout_popup(
    view_layout_popup, view_columns_part, mainmenu_commands::sort_priority_base + 1, "Layout");

static mainmenu_group_factory g_mainmenu_group_playlist_font_part(
    playlist_font_part, view_playlist_popup, mainmenu_commands::sort_priority_base);

static mainmenu_group_factory g_mainmenu_group_playlisy_misc_part(
    playlist_misc_part, view_playlist_popup, mainmenu_commands::sort_priority_base + 1);

static mainmenu_group_factory g_mainmenu_group_view_layout_commands_part(
    view_layout_commands, view_layout_popup, mainmenu_commands::sort_priority_base);

static mainmenu_group_factory g_mainmenu_group_view_layout_presets_part(
    view_layout_presets, view_layout_popup, mainmenu_commands::sort_priority_base + 1);
} // namespace groups

namespace commands {

static const MainMenuCommand activate_now_playing{activate_now_playing_id, "Activate now playing",
    "Activates the currently playing item.", [] { playlist_manager::get()->highlight_playing_item(); }};

static const MainMenuCommand show_groups{show_groups_id, "Show groups", "Shows or hides playlist view groups.",
    [] {
        pvt::cfg_grouping = !pvt::cfg_grouping;
        pvt::ng_playlist_view_t::g_on_groups_change();
    },
    [] { return static_cast<bool>(pvt::cfg_grouping); }};

static const MainMenuCommand decrease_font{
    {0xf2bc9f43, 0xf709, 0x4f6f, {0x9c, 0x65, 0x78, 0x73, 0x3b, 0x8, 0xc7, 0x77}}, "Decrease font size",
    "Decreases the playlist view font size.", [] { set_font_size(false); }};

static const MainMenuCommand increase_font{
    {0x8553d7fd, 0xebc5, 0x4ae7, {0xaa, 0x28, 0xb2, 0x6, 0xfe, 0x94, 0xa0, 0xb4}}, "Increase font size",
    "Increases the playlist font size.", [] { set_font_size(true); }};

static const MainMenuCommand show_status_bar{
    {0x5f944522, 0x843b, 0x43d2, {0x87, 0x14, 0xe3, 0xca, 0x1b, 0x78, 0x2b, 0x1f}}, "Show status bar",
    "Shows or hides the Columns UI status bar.",
    [] {
        cfg_status = !cfg_status;
        on_show_status_change();
    },
    [] { return cfg_status != 0; }};

static const MainMenuCommand show_status_pane{
    {0x6ffa8da6, 0x5cd, 0x403e, {0xa4, 0x87, 0x7e, 0x57, 0x2b, 0xb0, 0xc3, 0x20}}, "Show status pane",
    "Shows or hides the Columns UI status pane.",
    [] {
        settings::show_status_pane = !settings::show_status_pane;
        on_show_status_pane_change();
    },
    [] { return settings::show_status_pane != 0; }};

static const MainMenuCommand show_toolbars{
    {0x2dbb8dd3, 0x2d94, 0x4fd8, {0xa5, 0x7d, 0xbe, 0xb6, 0x42, 0xf5, 0xbf, 0x3b}}, "Show toolbars",
    "Shows or hides the Columns UI toolbars.",
    [] {
        cfg_toolbars = !cfg_toolbars;
        on_show_toolbars_change();
    },
    [] { return cfg_toolbars != 0; }};

static const MainMenuCommand live_editing{toggle_live_editing_id, "Live editing",
    "Enables or disables live editing of the Columns UI layout.",
    [] {
        bool val = !g_layout_window.get_layout_editing_active();
        g_layout_window.set_layout_editing_active(val);
        button_items::LiveLayoutEditingButton::s_on_change();
    },
    [] { return g_layout_window.get_layout_editing_active(); }};

class MainMenuCommands : public mainmenu_commands {
public:
    template <class... Commands>
    MainMenuCommands(GUID parent, uint32_t sort_priority, Commands&&... commands)
        : m_parent(parent), m_sort_priority(sort_priority), m_commands{std::forward<Commands>(commands)...}
    {
    }

    t_uint32 get_command_count() override { return m_commands.size(); }
    GUID get_command(t_uint32 p_index) override { return m_commands[p_index].guid; }
    void get_name(t_uint32 p_index, pfc::string_base& p_out) override { p_out = m_commands[p_index].name; }

    bool get_description(t_uint32 p_index, pfc::string_base& p_out) override
    {
        p_out = m_commands[p_index].description;
        return true;
    }

    GUID get_parent() override { return m_parent; }
    t_uint32 get_sort_priority() override { return m_sort_priority; }

    bool get_display(t_uint32 p_index, pfc::string_base& p_text, t_uint32& p_flags) override
    {
        auto& command = m_commands[p_index];
        p_text = command.name;
        if (command.is_ticked_callback && command.is_ticked_callback())
            p_flags |= flag_checked;
        return cui::main_window.get_wnd() != nullptr;
    }

    void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback) override
    {
        m_commands[p_index].execute_callback();
    }

private:
    GUID m_parent{};
    uint32_t m_sort_priority{};
    std::vector<MainMenuCommand> m_commands;
};

static service_factory_single_t<MainMenuCommands> mainmenu_commands_view(groups::view_columns_part,
    mainmenu_commands::sort_priority_base + 2, show_status_bar, show_status_pane, show_toolbars);

static service_factory_single_t<MainMenuCommands> mainmenu_commands_layout(
    groups::view_layout_commands, mainmenu_commands::sort_priority_base, live_editing);

static service_factory_single_t<MainMenuCommands> mainmenu_commands_playlist_misc(
    groups::playlist_misc_part, mainmenu_commands::sort_priority_dontcare, activate_now_playing);

static service_factory_single_t<MainMenuCommands> mainmenu_commands_playlist_font(
    groups::view_playlist_popup, mainmenu_commands::sort_priority_dontcare, show_groups, increase_font, decrease_font);

class MainMenuLayoutPresets : public mainmenu_commands {
    t_uint32 get_command_count() override { return cfg_layout.get_presets().get_count(); }
    GUID get_command(t_uint32 p_index) override
    {
        pfc::string8 name, buff;
        get_name(p_index, name);
        buff << "[View/Layout] " << name;
        return static_api_ptr_t<hasher_md5>()->process_single_guid(buff.get_ptr(), buff.get_length());
    }
    void get_name(t_uint32 p_index, pfc::string_base& p_out) override
    {
        if (p_index < cfg_layout.get_presets().get_count())
            p_out = cfg_layout.get_presets()[p_index].m_name;
    }
    bool get_description(t_uint32 p_index, pfc::string_base& p_out) override
    {
        if (p_index < cfg_layout.get_presets().get_count()) {
            pfc::string8 name;
            get_name(p_index, name);
            p_out.reset();
            p_out << "Changes active layout to " << name << ".";
        }
        return false;
    }
    bool get_display(t_uint32 p_index, pfc::string_base& p_text, t_uint32& p_flags) override
    {
        if (g_layout_window.get_wnd()) {
            p_flags = p_index == cfg_layout.get_active() ? mainmenu_commands::flag_checked : 0;
            get_name(p_index, p_text);
            return true;
        }
        return false;
    }
    void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback) override
    {
        if (p_index < cfg_layout.get_presets().get_count()) {
            cfg_layout.save_active_preset();
            cfg_layout.set_active_preset(p_index);
        }
    }

    GUID get_parent() override { return groups::view_layout_presets; }
    t_uint32 get_sort_priority() override { return mainmenu_commands::sort_priority_dontcare; }
};
static mainmenu_commands_factory_t<MainMenuLayoutPresets> mainmenu_commands_layout_presets;
} // namespace commands
} // namespace cui::main_menu
