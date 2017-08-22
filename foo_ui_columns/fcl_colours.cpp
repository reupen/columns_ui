#include "stdafx.h"
#include "playlist_switcher_v2.h"
#include "playlist_view.h"
#include "config.h"

class export_colours : public cui::fcl::dataset
{
    enum t_colour_pview_identifiers
    {
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
    void get_name (pfc::string_base & p_out) const override
    {
        p_out = "Colours";
    }
    const GUID & get_group () const override
    {
        return cui::fcl::groups::colours_and_fonts;
    }
    const GUID & get_guid () const override
    {
        // {1D5291B1-392D-4469-B905-91202B80EB7B}
        static const GUID guid = 
        { 0x1d5291b1, 0x392d, 0x4469, { 0xb9, 0x5, 0x91, 0x20, 0x2b, 0x80, 0xeb, 0x7b } };
        return guid;
    }
    void get_data (stream_writer * p_writer, t_uint32 type, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        /*out.write_item(colours_pview_mode, cfg_pv_use_custom_colours);
        out.write_item(colours_pview_background, cfg_back);
        out.write_item(colours_pview_selection_background, cfg_pv_selected_back);
        out.write_item(colours_pview_inactive_selection_background, cfg_pv_selected_text_no_focus);
        out.write_item(colours_pview_text, cfg_pv_text_colour);
        out.write_item(colours_pview_selection_text, cfg_pv_selected_text_colour);
        out.write_item(colours_pview_inactive_selection_text, cfg_pv_selected_text_no_focus);
        out.write_item(colours_pview_list_font, cfg_font);
        out.write_item(colours_pview_header_font, cfg_header_font);*/
        //out.write_item(colours_pview_use_system_focus_frame, cfg_pv_use_system_frame);
        out.write_item(identifier_vertical_item_padding, settings::playlist_view_item_padding.get_raw_value().value);
        out.write_item(identifier_vertical_item_padding_dpi, settings::playlist_view_item_padding.get_raw_value().dpi);
    }
    void set_data (stream_reader * p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback & feedback, abort_callback & p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;
        bool b_colour_read=false;

        bool item_padding_read = false;
        uih::IntegerAndDpi<int32_t> item_padding(0, uih::get_system_dpi_cached().cx);

        while (reader.get_remaining())
        {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id)
            {
            case identifier_vertical_item_padding:
                reader.read_item(item_padding.value);
                item_padding_read = true;
                break;
            case identifier_vertical_item_padding_dpi:
                reader.read_item(item_padding.dpi);
                item_padding_read = true;
                break;
            case colours_pview_mode:
                reader.read_item(cfg_pv_use_custom_colours);
                break;
            case colours_pview_use_system_focus_frame:
                reader.read_item(cfg_pv_use_system_frame);
                break;
            case colours_pview_background:
                b_colour_read=true;
                reader.read_item(cfg_back);
                break;
            case colours_pview_selection_background:
                reader.read_item(cfg_pv_selected_back);
                break;
            case colours_pview_inactive_selection_background:
                reader.read_item(cfg_pv_selected_text_no_focus);
                break;
            case colours_pview_text:
                reader.read_item(cfg_pv_text_colour);
                break;
            case colours_pview_selection_text:
                reader.read_item(cfg_pv_selected_text_colour);
                break;
            case colours_pview_inactive_selection_text:
                reader.read_item(cfg_pv_selected_text_no_focus);
                break;
            default:
                reader.skip(element_size);
                break;
            };
        }

        //on_header_font_change();
        //on_playlist_font_change();
        //pvt::ng_playlist_view_t::g_on_font_change();
        //pvt::ng_playlist_view_t::g_on_header_font_change();
        if (b_colour_read)
            g_import_pv_colours_to_unified_global();

        if (item_padding_read)
            settings::playlist_view_item_padding = item_padding;
        //refresh_all_playlist_views();    
        //pvt::ng_playlist_view_t::g_update_all_items();
    }
};

cui::fcl::dataset_factory<export_colours> g_export_colours_t;

class export_colours_switcher : public cui::fcl::dataset
{
    enum t_colour_switcher_identifiers
    {
        colours_switcher_mode, //not used
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
    void get_name (pfc::string_base & p_out) const override
    {
        p_out = "Colours";
    }
    const GUID & get_group () const override
    {
        return cui::fcl::groups::colours_and_fonts;
    }
    const GUID & get_guid () const override
    {
        // {1DE0CF38-5E8E-439c-8F01-B8999975AC0D}
        static const GUID guid = 
        { 0x1de0cf38, 0x5e8e, 0x439c, { 0x8f, 0x1, 0xb8, 0x99, 0x99, 0x75, 0xac, 0xd } };
        return guid;
    }
    void get_data (stream_writer * p_writer, t_uint32 type, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        out.write_item(identifier_item_height, settings::playlist_switcher_item_padding.get_raw_value().value);
        out.write_item(identifier_item_height_dpi, settings::playlist_switcher_item_padding.get_raw_value().dpi);
    }
    void set_data (stream_reader * p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback & feedback, abort_callback & p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;

        bool item_padding_read = false;
        uih::IntegerAndDpi<int32_t> item_padding(0, uih::get_system_dpi_cached().cx);

        while (reader.get_remaining())
        {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id)
            {
            case identifier_item_height:
                item_padding_read = true;
                reader.read_item(item_padding.value);
                break;
            case identifier_item_height_dpi:
                reader.read_item(item_padding.dpi);
                break;
            default:
                reader.skip(element_size);
                break;
            };

        }

        if (item_padding_read)
            settings::playlist_switcher_item_padding = item_padding;

        //update_playlist_switcher_panels();
        //on_switcher_font_change();
        //g_on_tabs_font_change();
    }
};

cui::fcl::dataset_factory<export_colours_switcher> g_export_colours_switcher_t;
