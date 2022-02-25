#include "stdafx.h"
#include "playlist_switcher_v2.h"

namespace cui::panels::playlist_switcher {

// cfg_int cfg_playlist_panel_delete(GUID{0x264215a8,0x1f5e,0x58cd,0x80,0x4c,0x12,0xc1,0x2c,0xf2,0xff,0xe3},1);
cfg_struct_t<LOGFONT> cfg_plist_font(
    GUID{0xd61b2f01, 0xa845, 0xc1f5, {0x99, 0x37, 0x26, 0xab, 0x74, 0x70, 0xf5, 0x0f}}, get_icon_font());

// {70A5C273-67AB-4bb6-B61C-F7975A6871FD}
const GUID g_guid_playlist_switcher_font
    = {0x70a5c273, 0x67ab, 0x4bb6, {0xb6, 0x1c, 0xf7, 0x97, 0x5a, 0x68, 0x71, 0xfd}};

class PlaylistSwitcherFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_playlist_switcher_font; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Playlist switcher"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { PlaylistSwitcher::g_on_font_items_change(); }
};

PlaylistSwitcherFontClient::factory<PlaylistSwitcherFontClient> g_font_client_switcher;

// {EB38A997-3B5F-4126-8746-262AA9C1F94B}
const GUID PlaylistSwitcherColoursClient::g_guid
    = {0xeb38a997, 0x3b5f, 0x4126, {0x87, 0x46, 0x26, 0x2a, 0xa9, 0xc1, 0xf9, 0x4b}};

PlaylistSwitcherColoursClient::factory<PlaylistSwitcherColoursClient> g_appearance_client_ps_impl;

void PlaylistSwitcherColoursClient::on_colour_changed(uint32_t mask) const
{
    PlaylistSwitcher::g_redraw_all();
}

void PlaylistSwitcherColoursClient::on_bool_changed(uint32_t mask) const
{
    if (mask & colours::bool_flag_dark_mode_enabled) {
        PlaylistSwitcher::s_on_dark_mode_status_change();
    }
}

} // namespace cui::panels::playlist_switcher
