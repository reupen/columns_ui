#include "pch.h"

#include "config.h"

namespace cui {

namespace {

class PlaylistSwitcherAppearanceDataSet : public fcl::dataset {
    enum ItemID {
        /** Legacy */
        colours_switcher_scheme, // not used
        colours_switcher_background,
        colours_switcher_selection_background,
        colours_switcher_inactive_selection_background,
        colours_switcher_text,
        colours_switcher_selection_text,
        colours_switcher_inactive_selection_text,
        colours_switcher_font_tabs,
        colours_switcher_font_list,
        /** Non-legacy */
        identifier_item_height,
        identifier_item_height_dpi
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Colours"; }
    const GUID& get_group() const override { return fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {1DE0CF38-5E8E-439c-8F01-B8999975AC0D}
        static const GUID guid = {0x1de0cf38, 0x5e8e, 0x439c, {0x8f, 0x1, 0xb8, 0x99, 0x99, 0x75, 0xac, 0xd}};
        return guid;
    }
    void get_data(stream_writer* p_writer, uint32_t type, fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        out.write_item(identifier_item_height, settings::playlist_switcher_item_padding.get_raw_value().value);
        out.write_item(identifier_item_height_dpi, settings::playlist_switcher_item_padding.get_raw_value().dpi);
    }
    void set_data(stream_reader* p_reader, size_t stream_size, uint32_t type, fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        uint32_t element_id;
        uint32_t element_size;

        bool font_read{false};
        bool item_padding_read = false;
        uih::IntegerAndDpi<int32_t> item_padding(0, uih::get_system_dpi_cached().cx);
        const auto api = fb2k::std_api_get<fonts::manager>();

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id) {
            case identifier_item_height:
                item_padding_read = true;
                reader.read_item(item_padding.value);
                break;
            case identifier_item_height_dpi:
                reader.read_item(item_padding.dpi);
                break;
            case colours_switcher_font_list: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(::fonts::playlist_switcher, lf);
                font_read = true;
                break;
            }
            case colours_switcher_font_tabs: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(::fonts::playlist_tabs, lf);
                api->set_font(::fonts::splitter_tabs, lf);
                font_read = true;
                break;
            }
            default:
                reader.skip(element_size);
                break;
            }
        }

        if (item_padding_read)
            settings::playlist_switcher_item_padding = item_padding;

        if (font_read)
            refresh_appearance_prefs();
        // update_playlist_switcher_panels();
        // on_switcher_font_change();
        // g_on_tabs_font_change();
    }
};

fcl::dataset_factory<PlaylistSwitcherAppearanceDataSet> g_export_colours_switcher_t;

} // namespace

} // namespace cui
