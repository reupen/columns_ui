#include "stdafx.h"

/**
 * Contains configuration variable and appearance clients for the legacy playlist view.
 *
 * These are being temporarily retained, so they are are not lost following an upgrade and a downgrade.
 */

cfg_int cfg_oldglobal(GUID{0x512eace5, 0x25c3, 0xb722, {0x28, 0x8b, 0xb3, 0x4a, 0xc5, 0x80, 0xf4, 0xbf}}, 0);

class appearance_client_pv_impl : public cui::colours::client {
public:
    static const GUID g_guid;

    const GUID& get_client_guid() const override { return g_guid; };

    void get_name(pfc::string_base& p_out) const override { p_out = "Legacy playlist"; };

    t_size get_supported_colours() const override { return cui::colours::colour_flag_all; }; // bit-mask

    t_size get_supported_bools() const override
    {
        return cui::colours::bool_flag_use_custom_active_item_frame;
    }; // bit-mask
    bool get_themes_supported() const override { return true; };

    void on_colour_changed(t_size mask) const override{};

    void on_bool_changed(t_size mask) const override{};
};

// {0CF29D60-1262-4f55-A6E1-BC4AE6579D19}
const GUID appearance_client_pv_impl::g_guid
    = {0xcf29d60, 0x1262, 0x4f55, {0xa6, 0xe1, 0xbc, 0x4a, 0xe6, 0x57, 0x9d, 0x19}};

appearance_client_pv_impl::factory<appearance_client_pv_impl> g_appearance_client_pv_impl;

// {82196D79-69BC-4041-8E2A-E3B4406BB6FC}
static const GUID font_client_cp_guid = {0x82196d79, 0x69bc, 0x4041, {0x8e, 0x2a, 0xe3, 0xb4, 0x40, 0x6b, 0xb6, 0xfc}};

class font_client_cp : public cui::fonts::client {
public:
    const GUID& get_client_guid() const override { return font_client_cp_guid; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Legacy playlist: Items"; }

    cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_items; }

    void on_font_changed() const override {}
};

font_client_cp::factory<font_client_cp> g_font_client_cp;

// {C0D3B76C-324D-46d3-BB3C-E81C7D3BCB85}
static const GUID font_client_cph_guid = {0xc0d3b76c, 0x324d, 0x46d3, {0xbb, 0x3c, 0xe8, 0x1c, 0x7d, 0x3b, 0xcb, 0x85}};

class font_client_cph : public cui::fonts::client {
public:
    const GUID& get_client_guid() const override { return font_client_cph_guid; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Legacy playlist: Column titles"; }

    cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_items; }

    void on_font_changed() const override {}
};

font_client_cph::factory<font_client_cph> g_font_client_cph;
