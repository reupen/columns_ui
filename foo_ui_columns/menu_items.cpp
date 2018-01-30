#include "stdafx.h"
#include "menu_items.h"
#include "layout.h"

extern HWND g_main_window;

namespace mainmenu_groups_columns {
// {AE078354-60EA-48a9-9405-DA0AEF00C58D}
const GUID view_columns_part = {0xae078354, 0x60ea, 0x48a9, {0x94, 0x5, 0xda, 0xa, 0xef, 0x0, 0xc5, 0x8d}};

// {16610091-8BD9-4e38-9DE5-4329BE14121B}
const GUID view_playlist_popup = {0x16610091, 0x8bd9, 0x4e38, {0x9d, 0xe5, 0x43, 0x29, 0xbe, 0x14, 0x12, 0x1b}};

// {0CEF3F5A-02C3-4865-8F62-379280E4EC3A}
const GUID playlist_font_part = {0xcef3f5a, 0x2c3, 0x4865, {0x8f, 0x62, 0x37, 0x92, 0x80, 0xe4, 0xec, 0x3a}};

// {F74D3C19-91CC-4179-954A-7BE54E9BBC80}
const GUID playlist_misc_part = {0xf74d3c19, 0x91cc, 0x4179, {0x95, 0x4a, 0x7b, 0xe5, 0x4e, 0x9b, 0xbc, 0x80}};

// {533FDC34-EE1B-4317-9D0C-489EE1215D9F}
const GUID view_layout_popup = {0x533fdc34, 0xee1b, 0x4317, {0x9d, 0xc, 0x48, 0x9e, 0xe1, 0x21, 0x5d, 0x9f}};

// {964C3B5A-1908-4407-9B8E-FC7331A99ED1}
const GUID view_layout_commands = {0x964c3b5a, 0x1908, 0x4407, {0x9b, 0x8e, 0xfc, 0x73, 0x31, 0xa9, 0x9e, 0xd1}};

// {C36EF588-ABDF-495e-A7D4-D021846F50EE}
const GUID view_layout_presets = {0xc36ef588, 0xabdf, 0x495e, {0xa7, 0xd4, 0xd0, 0x21, 0x84, 0x6f, 0x50, 0xee}};
} // namespace mainmenu_groups_columns

mainmenu_group_factory g_mainmenu_group_view_columns_part(
    mainmenu_groups_columns::view_columns_part, mainmenu_groups::view, mainmenu_commands::sort_priority_base + 1);
mainmenu_group_popup_factory g_mainmenu_group_view_playlist_popup(mainmenu_groups_columns::view_playlist_popup,
    mainmenu_groups_columns::view_columns_part, mainmenu_commands::sort_priority_base, "Columns playlist");
mainmenu_group_popup_factory g_mainmenu_group_view_layout_popup(mainmenu_groups_columns::view_layout_popup,
    mainmenu_groups_columns::view_columns_part, mainmenu_commands::sort_priority_base + 1, "Layout");
mainmenu_group_factory g_mainmenu_group_playlist_font_part(mainmenu_groups_columns::playlist_font_part,
    mainmenu_groups_columns::view_playlist_popup, mainmenu_commands::sort_priority_base);
mainmenu_group_factory g_mainmenu_group_playlisy_misc_part(mainmenu_groups_columns::playlist_misc_part,
    mainmenu_groups_columns::view_playlist_popup, mainmenu_commands::sort_priority_base + 1);
mainmenu_group_factory g_mainmenu_group_view_layout_commands_part(mainmenu_groups_columns::view_layout_commands,
    mainmenu_groups_columns::view_layout_popup, mainmenu_commands::sort_priority_base);
mainmenu_group_factory g_mainmenu_group_view_layout_presets_part(mainmenu_groups_columns::view_layout_presets,
    mainmenu_groups_columns::view_layout_popup, mainmenu_commands::sort_priority_base + 1);

const mainmenu_activate_now_playing_t g_mainmenu_activate_now_playing{};
const mainmenu_decrease_fontsize_t g_mainmenu_decrease_fontsize{};
const mainmenu_increase_fontsize_t g_mainmenu_increase_fontsize{};
const mainmenu_show_statusbar_t g_mainmenu_show_statusbar{};
const mainmenu_show_statuspane_t g_mainmenu_show_statuspane{};
const mainmenu_show_toolbars_t g_mainmenu_show_toolbars{};
const mainmenu_export_layout_t g_mainmenu_export_layout{};
const mainmenu_import_layout_t g_mainmenu_import_layout{};
// const mainmenu_activate_t g_mainmenu_activate;
const mainmenu_layout_live_edit_t g_mainmenu_layout_live_edit{};

const mainmenu_command_t* const g_mainmenu_commands_playlist_font_part[]
    = {&g_mainmenu_increase_fontsize, &g_mainmenu_decrease_fontsize};
const mainmenu_command_t* const g_mainmenu_commands_view_columns_part[] = {&g_mainmenu_show_statusbar,
    &g_mainmenu_show_statuspane, &g_mainmenu_show_toolbars /*,&g_mainmenu_export_layout,&g_mainmenu_import_layout*/};
const mainmenu_command_t* const g_mainmenu_commands_playlist_misc_part[] = {&g_mainmenu_activate_now_playing};
// const mainmenu_command_t * const g_mainmenu_commands_system_tray_part[] = {&g_mainmenu_activate};
const mainmenu_command_t* const g_mainmenu_commands_view_layout_commands_part[]
    = {&g_mainmenu_layout_live_edit /*,&g_mainmenu_export_layout,&g_mainmenu_import_layout*/};

class mainmenu_command_list_playlist_font_part_t {
public:
    static const mainmenu_command_t* const* get_ptr() { return &g_mainmenu_commands_playlist_font_part[0]; }
    static t_size get_size() { return tabsize(g_mainmenu_commands_playlist_font_part); }
    static GUID get_parent() { return mainmenu_groups_columns::view_playlist_popup; }
    static t_uint32 get_sort_priority() { return mainmenu_commands::sort_priority_dontcare; }
};

class mainmenu_command_list_view_columns_part_t {
public:
    static const mainmenu_command_t* const* get_ptr() { return &g_mainmenu_commands_view_columns_part[0]; }
    static t_size get_size() { return tabsize(g_mainmenu_commands_view_columns_part); }
    static GUID get_parent() { return mainmenu_groups_columns::view_columns_part; }
    static t_uint32 get_sort_priority() { return mainmenu_commands::sort_priority_base + 1; }
};

class mainmenu_command_list_playlist_misc_part_t {
public:
    static const mainmenu_command_t* const* get_ptr() { return &g_mainmenu_commands_playlist_misc_part[0]; }
    static t_size get_size() { return tabsize(g_mainmenu_commands_playlist_misc_part); }
    static GUID get_parent() { return mainmenu_groups_columns::playlist_misc_part; }
    static t_uint32 get_sort_priority() { return mainmenu_commands::sort_priority_dontcare; }
};

class mainmenu_command_list_view_layout_commands_part_t {
public:
    static const mainmenu_command_t* const* get_ptr()
    {
        return &g_mainmenu_commands_view_layout_commands_part[0];
    }
    static t_size get_size() { return tabsize(g_mainmenu_commands_view_layout_commands_part); }
    static GUID get_parent() { return mainmenu_groups_columns::view_layout_commands; }
    static t_uint32 get_sort_priority() { return mainmenu_commands::sort_priority_base; }
};

// {F2BC9F43-F709-4f6f-9C65-78733B08C777}
const GUID mainmenu_decrease_fontsize_t::g_guid
    = {0xf2bc9f43, 0xf709, 0x4f6f, {0x9c, 0x65, 0x78, 0x73, 0x3b, 0x8, 0xc7, 0x77}};

// {8553D7FD-EBC5-4ae7-AA28-B206FE94A0B4}
const GUID mainmenu_increase_fontsize_t::g_guid
    = {0x8553d7fd, 0xebc5, 0x4ae7, {0xaa, 0x28, 0xb2, 0x6, 0xfe, 0x94, 0xa0, 0xb4}};

// {5F944522-843B-43d2-8714-E3CA1B782B1F}
const GUID mainmenu_show_statusbar_t::g_guid
    = {0x5f944522, 0x843b, 0x43d2, {0x87, 0x14, 0xe3, 0xca, 0x1b, 0x78, 0x2b, 0x1f}};

// {2DBB8DD3-2D94-4fd8-A57D-BEB642F5BF3B}
const GUID mainmenu_show_toolbars_t::g_guid
    = {0x2dbb8dd3, 0x2d94, 0x4fd8, {0xa5, 0x7d, 0xbe, 0xb6, 0x42, 0xf5, 0xbf, 0x3b}};

// {450C17B0-70E0-42cc-AA7D-DC5B7ADFF5A8}
const GUID mainmenu_activate_now_playing_t::g_guid
    = {0x450c17b0, 0x70e0, 0x42cc, {0xaa, 0x7d, 0xdc, 0x5b, 0x7a, 0xdf, 0xf5, 0xa8}};

// {C9B68884-2A15-48fb-8B72-619B980975F1}
const GUID mainmenu_export_layout_t::g_guid
    = {0xc9b68884, 0x2a15, 0x48fb, {0x8b, 0x72, 0x61, 0x9b, 0x98, 0x9, 0x75, 0xf1}};

// {77299EC5-4E9F-4efc-8C49-0077C46A537C}
const GUID mainmenu_import_layout_t::g_guid
    = {0x77299ec5, 0x4e9f, 0x4efc, {0x8c, 0x49, 0x0, 0x77, 0xc4, 0x6a, 0x53, 0x7c}};

// {F2C92EB9-B5F1-4cec-B24E-C140831F7B09}
const GUID mainmenu_layout_live_edit_t::g_guid
    = {0xf2c92eb9, 0xb5f1, 0x4cec, {0xb2, 0x4e, 0xc1, 0x40, 0x83, 0x1f, 0x7b, 0x9}};

#if 0
// {098B6DC5-3080-48d2-BB80-DBD9ED1CDF9A}
const GUID mainmenu_activate_t = 
{ 0x98b6dc5, 0x3080, 0x48d2, { 0xbb, 0x80, 0xdb, 0xd9, 0xed, 0x1c, 0xdf, 0x9a } };
#endif

// {6FFA8DA6-05CD-403e-A487-7E572BB0C320}
const GUID mainmenu_show_statuspane_t::g_guid
    = {0x6ffa8da6, 0x5cd, 0x403e, {0xa4, 0x87, 0x7e, 0x57, 0x2b, 0xb0, 0xc3, 0x20}};

template <typename t_commands>
class mainmenu_commands_t : public mainmenu_commands {
    t_uint32 get_command_count() override { return t_commands::get_size(); }
    GUID get_command(t_uint32 p_index) override
    {
        if (p_index < t_commands::get_size())
            return t_commands::get_ptr()[p_index]->get_guid();
        return pfc::guid_null;
    }
    void get_name(t_uint32 p_index, pfc::string_base& p_out) override
    {
        if (p_index < t_commands::get_size())
            t_commands::get_ptr()[p_index]->get_name(p_out);
    }
    bool get_description(t_uint32 p_index, pfc::string_base& p_out) override
    {
        if (p_index < t_commands::get_size())
            return t_commands::get_ptr()[p_index]->get_description(p_out);
        return false;
    }
    bool get_display(t_uint32 p_index, pfc::string_base& p_text, t_uint32& p_flags) override
    {
        if (p_index < t_commands::get_size())
            return t_commands::get_ptr()[p_index]->get_display(p_text, p_flags);
        return false;
    }
    void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback) override
    {
        if (p_index < t_commands::get_size())
            t_commands::get_ptr()[p_index]->execute(p_callback);
    }

    GUID get_parent() override { return t_commands::get_parent(); }
    t_uint32 get_sort_priority() override { return t_commands::get_sort_priority(); }
};

mainmenu_commands_factory_t<mainmenu_commands_t<mainmenu_command_list_playlist_font_part_t>>
    g_mainmenu_commands_playlist_font;
mainmenu_commands_factory_t<mainmenu_commands_t<mainmenu_command_list_view_columns_part_t>> g_mainmenu_commands_columns;
mainmenu_commands_factory_t<mainmenu_commands_t<mainmenu_command_list_playlist_misc_part_t>>
    g_mainmenu_commands_playlist_misc;
mainmenu_commands_factory_t<mainmenu_commands_t<mainmenu_command_list_view_layout_commands_part_t>>
    g_mainmenu_commands_layout_commands;

class mainmenu_commands_layout : public mainmenu_commands {
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

    GUID get_parent() override { return mainmenu_groups_columns::view_layout_presets; }
    t_uint32 get_sort_priority() override { return mainmenu_commands::sort_priority_dontcare; }
};
mainmenu_commands_factory_t<mainmenu_commands_layout> g_mainmenu_commands_layout;

class live_layout_editing_button_t : public uie::button {
    const GUID& get_item_guid() const override { return mainmenu_layout_live_edit_t::g_guid; }
    HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask& p_mask_type,
        COLORREF& cr_mask, HBITMAP& bm_mask) const override
    {
        return nullptr;
    }
    unsigned get_button_state() const override
    {
        return g_layout_window.get_layout_editing_active() ? uie::BUTTON_STATE_PRESSED | uie::BUTTON_STATE_ENABLED
                                                           : uie::BUTTON_STATE_DEFAULT;
    }
    void register_callback(uie::button_callback& p_callback) override { m_callbacks.add_item(&p_callback); };
    void deregister_callback(uie::button_callback& p_callback) override { m_callbacks.remove_item(&p_callback); };
    pfc::ptr_list_t<uie::button_callback> m_callbacks;
    static pfc::ptr_list_t<live_layout_editing_button_t> m_buttons;

public:
    static void g_on_layout_editing_enabled_change(bool b_enabled)
    {
        t_size i, ic = m_buttons.get_count(), j, jc;
        for (i = 0; i < ic; i++) {
            jc = m_buttons[i]->m_callbacks.get_count();
            for (j = 0; j < jc; j++) {
                m_buttons[i]->m_callbacks[j]->on_button_state_change(
                    b_enabled ? uie::BUTTON_STATE_PRESSED | uie::BUTTON_STATE_ENABLED : uie::BUTTON_STATE_DEFAULT);
            }
        }
    }
    live_layout_editing_button_t() { m_buttons.add_item(this); }
    ~live_layout_editing_button_t() { m_buttons.remove_item(this); }
};

void mainmenu_layout_live_edit_t::execute(service_ptr_t<service_base> p_callback) const
{
    bool val = !g_layout_window.get_layout_editing_active();
    g_layout_window.set_layout_editing_active(val);
    live_layout_editing_button_t::g_on_layout_editing_enabled_change(val);
    //;
}

pfc::ptr_list_t<live_layout_editing_button_t> live_layout_editing_button_t::m_buttons;

uie::button_factory<live_layout_editing_button_t> g_live_layout_editing_button_t;