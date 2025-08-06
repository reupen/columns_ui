#pragma once

#include "menu_helpers.h"

namespace cui::config {

constexpr GUID advconfig_branch_columns_ui_id{
    0xd2dd7ffc, 0xf780, 0x4fa3, {0x89, 0x52, 0x38, 0xd8, 0x2c, 0x8c, 0x1e, 0x4b}};
constexpr GUID advconfig_branch_system_tray_id{
    0x8b39648d, 0x3e4, 0x4748, {0x8d, 0xd3, 0x71, 0x6b, 0x76, 0x8a, 0x39, 0xf0}};

extern advconfig_checkbox_factory advbool_system_tray_icon_x_buttons;
extern advconfig_checkbox_factory advbool_close_to_system_tray_icon;

extern cfg_bool cfg_playlist_tabs_middle_click;

extern fbh::ConfigBool use_hardware_acceleration;

} // namespace cui::config

class ConfigWindowPlacement : public cfg_var {
public:
    explicit ConfigWindowPlacement(const GUID& p_guid);

    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort) override;
    WINDOWPLACEMENT get_value() const;

    WINDOWPLACEMENT m_value{};
    int32_t m_dpi{USER_DEFAULT_SCREEN_DPI};
};

class ConfigMenuItem : public cfg_struct_t<MenuItemIdentifier> {
public:
    using cfg_struct_t<MenuItemIdentifier>::operator=;
    using cfg_struct_t<MenuItemIdentifier>::operator MenuItemIdentifier;
    void reset()
    {
        MenuItemIdentifier temp;
        *this = temp;
    }
    explicit ConfigMenuItem(const GUID& p_guid, const MenuItemIdentifier& p_val)
        : cfg_struct_t<MenuItemIdentifier>(p_guid, p_val)
    {
    }
    explicit ConfigMenuItem(const GUID& p_guid, const GUID& p_val, const GUID& psub = pfc::guid_null)
        : cfg_struct_t<MenuItemIdentifier>(p_guid, MenuItemIdentifier{p_val, psub})
    {
    }
    explicit ConfigMenuItem(const GUID& p_guid) : cfg_struct_t<MenuItemIdentifier>(p_guid, MenuItemIdentifier{}) {}
};

namespace settings {
extern cfg_bool show_status_pane;

extern fbh::ConfigObjectBoolFactory<> allow_locked_panel_resizing;
extern fbh::ConfigInt32DpiAware custom_splitter_divider_width;

extern fbh::ConfigInt32DpiAware playlist_switcher_item_padding;
extern fbh::ConfigInt32DpiAware playlist_view_item_padding;
} // namespace settings

extern cfg_string cfg_playlist_switcher_tagz;
extern ConfigMenuItem cfg_playlist_double;

extern cfg_int cfg_global;
extern cfg_int cfg_cur_prefs_col;
extern cfg_int cfg_header_hottrack;
extern cfg_int cfg_sortsel;
extern cfg_int cfg_global_sort;
extern cfg_int cfg_vis_edge;
extern cfg_int cfg_lock;
extern cfg_int cfg_header;
extern cfg_int cfg_drop_at_end;
extern cfg_int cfg_mclick;
extern cfg_int cfg_mclick2;
extern cfg_int cfg_balloon;
extern cfg_int cfg_scroll_h_no_v;
extern cfg_int cfg_ellipsis;
extern cfg_int cfg_tabs_multiline;
extern cfg_int cfg_frame;
extern cfg_int cfg_show_seltime;
extern cfg_int cfg_show_selcount;
extern cfg_int cfg_plistframe;
extern cfg_int cfg_tooltips_clipped;
extern cfg_int cfg_np;
extern cfg_int cfg_show_systray;
extern cfg_int cfg_minimise_to_tray;
extern cfg_int cfg_show_vol;
extern cfg_int cfg_custom_icon;
extern cfg_int cfg_custom_buttons;
extern cfg_int cfg_drag_autoswitch;
extern cfg_int cfg_plist_width;
extern cfg_int cfg_drag_pl;
extern cfg_int cfg_pl_autohide;
extern cfg_int cfg_sel_dp;
extern cfg_int cfg_alternative_sel;
extern cfg_int cfg_plm_rename;
extern cfg_int cfg_pgen_playlist;
extern cfg_int cfg_pgen_tf;
extern cfg_int cfg_autoswitch_delay;
extern cfg_int cfg_pgen_dir;
extern cfg_int cfg_custom_buttons_over;
extern cfg_int cfg_custom_buttons_transparency;
extern cfg_int cfg_replace_drop_underscores;
extern cfg_int cfg_status;
extern cfg_int cfg_show_sort_arrows;
extern cfg_int cfg_toolbar_disable_default_drawing;
extern cfg_int cfg_sidebar_use_custom_show_delay;
extern cfg_int cfg_sidebar_show_delay;
extern cfg_int cfg_sidebar_hide_delay;
extern cfg_int cfg_toolbars;
extern cfg_int cfg_playlist_switcher_use_tagz;
extern cfg_int cfg_playlist_middle_action;
extern cfg_int cfg_playlist_panel_delete;
extern cfg_int cfg_nohscroll;

extern cfg_bool cfg_main_window_is_hidden;

extern cfg_string cfg_tray_icon_path;
extern cfg_string cfg_export;
extern cfg_string cfg_import;
extern cfg_string cfg_custom_buttons_path;
extern cfg_string cfg_globalstring;
extern cfg_string cfg_colour;
extern cfg_string cfg_pgenstring;

extern ConfigMenuItem cfg_statusdbl;

extern ConfigWindowPlacement cfg_window_placement_columns;
