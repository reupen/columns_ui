#include "pch.h"

#include "item_details.h"

#include "tf_utils.h"

namespace cui::panels::item_details {

class ItemDetailsFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_item_details_font_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item details"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { ItemDetails::s_on_font_change(); }
};

class ItemDetailsColoursClient : public colours::client {
public:
    const GUID& get_client_guid() const override { return g_guid_item_details_colour_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item details"; }

    uint32_t get_supported_colours() const override
    {
        return colours::colour_flag_background | colours::colour_flag_text;
    } // bit-mask
    uint32_t get_supported_bools() const override { return colours::bool_flag_dark_mode_enabled; } // bit-mask

    bool get_themes_supported() const override { return false; }

    void on_bool_changed(uint32_t mask) const override
    {
        if (mask & colours::bool_flag_dark_mode_enabled)
            ItemDetails::s_on_dark_mode_status_change();
    }
    void on_colour_changed(uint32_t mask) const override { ItemDetails::s_on_colours_change(); }
};

namespace {
ItemDetailsFontClient::factory<ItemDetailsFontClient> g_font_client_item_details;
ItemDetailsColoursClient::factory<ItemDetailsColoursClient> g_colour_client_item_details;
} // namespace

} // namespace cui::panels::item_details
