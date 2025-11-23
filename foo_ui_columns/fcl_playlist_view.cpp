#include "pch.h"

#include "colour_manager_data.h"
#include "config.h"
#include "config_appearance.h"
#include "tab_dark_mode.h"
#include "ng_playlist/ng_playlist.h"

namespace cui {

namespace {

class PlaylistViewAppearanceDataSet : public fcl::dataset {
    enum ItemID {
        /** Legacy */
        colours_pview_scheme,
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
        identifier_indent_groups,
        identifier_use_custom_group_indentation_amount,
        identifier_custom_group_indentation_amount,
        identifier_show_artwork,
        identifier_sticky_artwork,
        identifier_show_artwork_reflection,
        identifier_artwork_width,
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

        out.write_item(identifier_indent_groups, panels::playlist_view::cfg_indent_groups);
        out.write_item(
            identifier_custom_group_indentation_amount, panels::playlist_view::cfg_custom_group_indentation_amount);
        out.write_item(identifier_use_custom_group_indentation_amount,
            panels::playlist_view::cfg_use_custom_group_indentation_amount);

        out.write_item(identifier_show_artwork, panels::playlist_view::cfg_show_artwork);
        out.write_item(identifier_sticky_artwork, panels::playlist_view::cfg_sticky_artwork);
        out.write_item(identifier_show_artwork_reflection, panels::playlist_view::cfg_artwork_reflection);
        out.write_item(identifier_artwork_width, panels::playlist_view::cfg_artwork_width);
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

        const auto old_artwork_width = panels::playlist_view::cfg_artwork_width.get_scaled_value();
        const auto old_show_artwork_reflection = panels::playlist_view::cfg_artwork_reflection.get();

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
            case identifier_indent_groups:
                reader.read_item(panels::playlist_view::cfg_indent_groups);
                break;
            case identifier_use_custom_group_indentation_amount:
                reader.read_item(panels::playlist_view::cfg_use_custom_group_indentation_amount);
                break;
            case identifier_custom_group_indentation_amount:
                reader.read_item(panels::playlist_view::cfg_custom_group_indentation_amount);
                break;
            case identifier_show_artwork:
                reader.read_item(panels::playlist_view::cfg_show_artwork);
                break;
            case identifier_sticky_artwork:
                reader.read_item(panels::playlist_view::cfg_sticky_artwork);
                break;
            case identifier_show_artwork_reflection:
                reader.read_item(panels::playlist_view::cfg_artwork_reflection);
                break;
            case identifier_artwork_width:
                reader.read_item(panels::playlist_view::cfg_artwork_width);
                break;
            case colours_pview_scheme: {
                int use_custom_colours{};
                reader.read_item(use_custom_colours);
                if (use_custom_colours == 2)
                    colour_manager_entry->colour_set.colour_scheme = colours::ColourSchemeThemed;
                else if (use_custom_colours == 1)
                    colour_manager_entry->colour_set.colour_scheme = colours::ColourSchemeCustom;
                else
                    colour_manager_entry->colour_set.colour_scheme = colours::ColourSchemeSystem;
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

        if (b_colour_read) {
            const auto old_is_dark = colours::is_dark_mode_active();
            colours::dark_mode_status.set(WI_EnumValue(cui::colours::DarkModeStatus::Disabled));

            prefs::g_tab_dark_mode.refresh();

            if (old_is_dark == colours::is_dark_mode_active())
                on_global_colours_change();
        }

        if (font_read)
            refresh_appearance_prefs();

        if (item_padding_read)
            settings::playlist_view_item_padding = item_padding;

        if (old_artwork_width != panels::playlist_view::cfg_artwork_width.get_scaled_value()
            || old_show_artwork_reflection != panels::playlist_view::cfg_artwork_reflection.get())
            panels::playlist_view::PlaylistView::g_on_artwork_width_change();

        panels::playlist_view::PlaylistView::g_on_show_artwork_change();
        panels::playlist_view::PlaylistView::s_on_sticky_artwork_change();
    }
};

fcl::dataset_factory<PlaylistViewAppearanceDataSet> g_export_colours_t;

} // namespace

} // namespace cui
