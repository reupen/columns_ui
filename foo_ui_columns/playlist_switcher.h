#pragma once

namespace cui::panels::playlist_switcher {

class PlaylistSwitcherColoursClient : public colours::client {
public:
    static constexpr GUID id{0xeb38a997, 0x3b5f, 0x4126, {0x87, 0x46, 0x26, 0x2a, 0xa9, 0xc1, 0xf9, 0x4b}};

    const GUID& get_client_guid() const override { return id; }

    void get_name(pfc::string_base& p_out) const override { p_out = "Playlist switcher"; }

    uint32_t get_supported_colours() const override { return colours::colour_flag_all; } // bit-mask
    uint32_t get_supported_bools() const override
    {
        return colours::bool_flag_use_custom_active_item_frame | colours::bool_flag_dark_mode_enabled;
    } // bit-mask
    bool get_themes_supported() const override { return true; }

    void on_colour_changed(uint32_t mask) const override;
    void on_bool_changed(uint32_t mask) const override;
};

} // namespace cui::panels::playlist_switcher
