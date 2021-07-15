#pragma once

namespace cui::panels::playlist_switcher {

class PlaylistSwitcherColoursClient : public cui::colours::client {
public:
    static const GUID g_guid;

    const GUID& get_client_guid() const override { return g_guid; };

    void get_name(pfc::string_base& p_out) const override { p_out = "Playlist switcher"; };

    t_size get_supported_colours() const override { return cui::colours::colour_flag_all; }; // bit-mask
    t_size get_supported_bools() const override
    {
        return cui::colours::bool_flag_use_custom_active_item_frame;
    }; // bit-mask
    bool get_themes_supported() const override { return true; };

    void on_colour_changed(t_size mask) const override;
    void on_bool_changed(t_size mask) const override{};
};

} // namespace cui::panels::playlist_switcher
