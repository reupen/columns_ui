#include "stdafx.h"
#include "playlist_switcher_v2.h"
#include "playlist_view.h"
#include "config.h"
#include "tab_colours.h"

class export_colours : public cui::fcl::dataset {
    enum t_colour_pview_identifiers {
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
        identifier_vertical_item_padding,
        identifier_vertical_item_padding_dpi,
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Colours"; }
    const GUID& get_group() const override { return cui::fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {1D5291B1-392D-4469-B905-91202B80EB7B}
        static const GUID guid = {0x1d5291b1, 0x392d, 0x4469, {0xb9, 0x5, 0x91, 0x20, 0x2b, 0x80, 0xeb, 0x7b}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        out.write_item(identifier_vertical_item_padding, settings::playlist_view_item_padding.get_raw_value().value);
        out.write_item(identifier_vertical_item_padding_dpi, settings::playlist_view_item_padding.get_raw_value().dpi);
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        static_api_ptr_t<cui::fonts::manager> api;
        colours_manager_data::entry_ptr_t colour_manager_entry;
        g_colours_manager_data.find_by_guid(pfc::guid_null, colour_manager_entry);

        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;
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
                    colour_manager_entry->colour_mode = cui::colours::colour_mode_themed;
                else if (use_custom_colours == 1)
                    colour_manager_entry->colour_mode = cui::colours::colour_mode_custom;
                else
                    colour_manager_entry->colour_mode = cui::colours::colour_mode_system;
                break;
            }
            case colours_pview_use_system_focus_frame: {
                int use_system_frame{};
                reader.read_item(use_system_frame);
                colour_manager_entry->use_custom_active_item_frame = !use_system_frame;
                break;
            }
            case colours_pview_background:
                b_colour_read = true;
                reader.read_item(colour_manager_entry->background);
                break;
            case colours_pview_selection_background:
                reader.read_item(colour_manager_entry->selection_background);
                break;
            case colours_pview_inactive_selection_background:
                reader.read_item(colour_manager_entry->inactive_selection_background);
                break;
            case colours_pview_text:
                reader.read_item(colour_manager_entry->text);
                break;
            case colours_pview_selection_text:
                reader.read_item(colour_manager_entry->selection_text);
                break;
            case colours_pview_inactive_selection_text:
                reader.read_item(colour_manager_entry->inactive_selection_text);
                break;
            case colours_pview_header_font: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(fonts::columns_playlist_header, lf);
                api->set_font(fonts::ng_playlist_header, lf);
                api->set_font(fonts::filter_header, lf);
                font_read = true;
                break;
            }
            case colours_pview_list_font: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(fonts::columns_playlist_items, lf);
                api->set_font(fonts::ng_playlist_items, lf);
                api->set_font(fonts::filter_items, lf);
                font_read = true;
                break;
            }
            default:
                reader.skip(element_size);
                break;
            }
        }

        // on_header_font_change();
        // on_playlist_font_change();
        // pvt::ng_playlist_view_t::g_on_font_change();
        // pvt::ng_playlist_view_t::g_on_header_font_change();
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

cui::fcl::dataset_factory<export_colours> g_export_colours_t;

class export_colours_switcher : public cui::fcl::dataset {
    enum t_colour_switcher_identifiers {
        colours_switcher_mode, // not used
        colours_switcher_background,
        colours_switcher_selection_background,
        colours_switcher_inactive_selection_background,
        colours_switcher_text,
        colours_switcher_selection_text,
        colours_switcher_inactive_selection_text,
        colours_switcher_font_tabs,
        colours_switcher_font_list,
        identifier_item_height,
        identifier_item_height_dpi
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Colours"; }
    const GUID& get_group() const override { return cui::fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {1DE0CF38-5E8E-439c-8F01-B8999975AC0D}
        static const GUID guid = {0x1de0cf38, 0x5e8e, 0x439c, {0x8f, 0x1, 0xb8, 0x99, 0x99, 0x75, 0xac, 0xd}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        out.write_item(identifier_item_height, settings::playlist_switcher_item_padding.get_raw_value().value);
        out.write_item(identifier_item_height_dpi, settings::playlist_switcher_item_padding.get_raw_value().dpi);
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;

        bool font_read{false};
        bool item_padding_read = false;
        uih::IntegerAndDpi<int32_t> item_padding(0, uih::get_system_dpi_cached().cx);
        static_api_ptr_t<cui::fonts::manager> api;

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
                api->set_font(fonts::playlist_switcher, lf);
                font_read = true;
                break;
            }
            case colours_switcher_font_tabs: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(fonts::playlist_tabs, lf);
                api->set_font(fonts::splitter_tabs, lf);
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

cui::fcl::dataset_factory<export_colours_switcher> g_export_colours_switcher_t;

class export_misc_fonts : public cui::fcl::dataset {
    enum t_colour_pview_identifiers {
        font_status,
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Misc fonts"; }
    const GUID& get_group() const override { return cui::fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {0A297BE7-DE43-49da-8D8E-C8D888CF1014}
        static const GUID guid = {0xa297be7, 0xde43, 0x49da, {0x8d, 0x8e, 0xc8, 0xd8, 0x88, 0xcf, 0x10, 0x14}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        // out.write_item(font_status, cfg_status_font);
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        static_api_ptr_t<cui::fonts::manager> api;
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;
        bool font_read{false};

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id) {
            case font_status: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(fonts::status_bar, lf);
                font_read = true;
                break;
            }
            default:
                reader.skip(element_size);
                break;
            }
        }

        if (font_read)
            refresh_appearance_prefs();
        // on_status_font_change();
    }
};

cui::fcl::dataset_factory<export_misc_fonts> g_export_misc_fonts_t;
