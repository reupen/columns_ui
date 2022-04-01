#include "pch.h"

#include "colour_manager_data.h"
#include "config.h"
#include "config_appearance.h"

namespace cui {

namespace {

class PlaylistViewAppearanceDataSet : public fcl::dataset {
    enum ItemID {
        /** Legacy */
        colours_pview_mode,
        colours_pview_background,
        colours_pview_selection_background,
        colours_pview_inactive_selection_background,
        colours_pview_text,
        colours_pview_selection_text,
        colours_pview_inactive_selection_text,
        colours_pview_header_font,
        colours_pview_list_font,
        colours_pview_use_system_focus_frame,
        /** Non-legacy */
        identifier_vertical_item_padding,
        identifier_vertical_item_padding_dpi,
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Colours"; }
    const GUID& get_group() const override { return fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {1D5291B1-392D-4469-B905-91202B80EB7B}
        static const GUID guid = {0x1d5291b1, 0x392d, 0x4469, {0xb9, 0x5, 0x91, 0x20, 0x2b, 0x80, 0xeb, 0x7b}};
        return guid;
    }
    void get_data(stream_writer* p_writer, uint32_t type, fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        out.write_item(identifier_vertical_item_padding, settings::playlist_view_item_padding.get_raw_value().value);
        out.write_item(identifier_vertical_item_padding_dpi, settings::playlist_view_item_padding.get_raw_value().dpi);
    }
    void set_data(stream_reader* p_reader, size_t stream_size, uint32_t type, fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        const auto api = fb2k::std_api_get<fonts::manager>();
        const auto colour_manager_entry = g_colour_manager_data.get_global_entry(false);

        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        uint32_t element_id;
        uint32_t element_size;
        bool b_colour_read = false;

        bool item_padding_read = false;
        bool font_read{false};
        uih::IntegerAndDpi<int32_t> item_padding(0, uih::get_system_dpi_cached().cx);

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id) {
            case identifier_vertical_item_padding:
                reader.read_item(item_padding.value);
                item_padding_read = true;
                break;
            case identifier_vertical_item_padding_dpi:
                reader.read_item(item_padding.dpi);
                item_padding_read = true;
                break;
            case colours_pview_mode: {
                int use_custom_colours{};
                reader.read_item(use_custom_colours);
                if (use_custom_colours == 2)
                    colour_manager_entry->colour_set.colour_mode = colours::colour_mode_themed;
                else if (use_custom_colours == 1)
                    colour_manager_entry->colour_set.colour_mode = colours::colour_mode_custom;
                else
                    colour_manager_entry->colour_set.colour_mode = colours::colour_mode_system;
                break;
            }
            case colours_pview_use_system_focus_frame: {
                int use_system_frame{};
                reader.read_item(use_system_frame);
                colour_manager_entry->colour_set.use_custom_active_item_frame = !use_system_frame;
                break;
            }
            case colours_pview_background:
                b_colour_read = true;
                reader.read_item(colour_manager_entry->colour_set.background);
                break;
            case colours_pview_selection_background:
                reader.read_item(colour_manager_entry->colour_set.selection_background);
                break;
            case colours_pview_inactive_selection_background:
                reader.read_item(colour_manager_entry->colour_set.inactive_selection_background);
                break;
            case colours_pview_text:
                reader.read_item(colour_manager_entry->colour_set.text);
                break;
            case colours_pview_selection_text:
                reader.read_item(colour_manager_entry->colour_set.selection_text);
                break;
            case colours_pview_inactive_selection_text:
                reader.read_item(colour_manager_entry->colour_set.inactive_selection_text);
                break;
            case colours_pview_header_font: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(::fonts::columns_playlist_header, lf);
                api->set_font(::fonts::ng_playlist_header, lf);
                api->set_font(::fonts::filter_header, lf);
                font_read = true;
                break;
            }
            case colours_pview_list_font: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(::fonts::columns_playlist_items, lf);
                api->set_font(::fonts::ng_playlist_items, lf);
                api->set_font(::fonts::filter_items, lf);
                font_read = true;
                break;
            }
            default:
                reader.skip(element_size);
                break;
            }
        }

        if (b_colour_read)
            on_global_colours_change();

        if (font_read)
            refresh_appearance_prefs();

        if (item_padding_read)
            settings::playlist_view_item_padding = item_padding;
        // refresh_all_playlist_views();
        // pvt::ng_playlist_view_t::g_update_all_items();
    }
};

fcl::dataset_factory<PlaylistViewAppearanceDataSet> g_export_colours_t;

} // namespace

} // namespace cui
