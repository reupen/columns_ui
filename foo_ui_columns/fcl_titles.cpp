#include "stdafx.h"
#include "fcl.h"
#include "ng_playlist/ng_playlist_groups.h"
#include "ng_playlist/ng_playlist.h"

cui::fcl::group_impl_factory fclgroupcolumns(
    cui::fcl::groups::titles_playlist_view, "Playlist Scripts", "Playlist Scripts", cui::fcl::groups::title_scripts);
cui::fcl::group_impl_factory fclgroupcommon(
    cui::fcl::groups::titles_common, "Common Scripts", "Common Scripts", cui::fcl::groups::title_scripts);

class PlaylistViewColumnsDataSet : public cui::fcl::dataset {
    enum t_identifiers {
        identifier_column,
    };
    enum t_columns {
        identifier_name,
        identifier_display,
        identifier_sort,
        identifier_use_sort,
        identifier_style,
        identifier_use_style,
        identifier_width,
        identifier_alignment,
        identifier_filter_type,
        identifier_filter,
        identifier_resize,
        identifier_show,
        identifier_edit_field,
        identifier_width_dpi

    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Columns"; }
    const GUID& get_group() const override { return cui::fcl::groups::titles_playlist_view; }
    const GUID& get_guid() const override
    {
        // {2415AAE7-7F5E-4ad8-94E2-1E730A2139EF}
        static const GUID guid = {0x2415aae7, 0x7f5e, 0x4ad8, {0x94, 0xe2, 0x1e, 0x73, 0xa, 0x21, 0x39, 0xef}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        t_size count = g_columns.get_count();
        pfc::string8 temp;
        out.write_raw(count);

        for (t_size i = 0; i < count; i++) {
            stream_writer_memblock sw;
            fbh::fcl::Writer w(&sw, p_abort);
            w.write_item(identifier_name, g_columns[i]->name);
            w.write_item(identifier_display, g_columns[i]->spec);
            w.write_item(identifier_style, g_columns[i]->colour_spec);
            w.write_item(identifier_edit_field, g_columns[i]->edit_field);
            w.write_item(identifier_filter, g_columns[i]->filter);
            w.write_item(identifier_sort, g_columns[i]->sort_spec);
            w.write_item(identifier_use_style, g_columns[i]->use_custom_colour);
            w.write_item(identifier_use_sort, g_columns[i]->use_custom_sort);
            w.write_item(identifier_show, g_columns[i]->show);
            w.write_item(identifier_filter_type, (t_uint32)g_columns[i]->filter_type);
            w.write_item(identifier_alignment, (t_uint32)g_columns[i]->align);
            w.write_item(identifier_resize, g_columns[i]->parts);
            w.write_item(identifier_width, g_columns[i]->width.value);
            w.write_item(identifier_width_dpi, g_columns[i]->width.dpi);

            out.write_item(identifier_column, sw.m_data.get_ptr(), sw.m_data.get_size());
        }
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_size count;
        reader.read_item(count);
        ColumnList newcolumns;
        for (t_size i = 0; i < count; i++) {
            t_uint32 column_id;
            t_uint32 column_size;

            reader.read_item(column_id);
            reader.read_item(column_size);

            PlaylistViewColumn::ptr item = new PlaylistViewColumn;

            fbh::fcl::Reader reader2(reader, column_size, p_abort);

            t_uint32 element_id;
            t_uint32 element_size;

            auto dpiRead = false;

            while (reader2.get_remaining()) {
                reader2.read_item(element_id);
                reader2.read_item(element_size);

                switch (element_id) {
                case identifier_name:
                    reader2.read_item(item->name, element_size);
                    break;
                case identifier_filter:
                    reader2.read_item(item->filter, element_size);
                    break;
                case identifier_sort:
                    reader2.read_item(item->sort_spec, element_size);
                    break;
                case identifier_display:
                    reader2.read_item(item->spec, element_size);
                    break;
                case identifier_edit_field:
                    reader2.read_item(item->edit_field, element_size);
                    break;
                case identifier_style:
                    reader2.read_item(item->colour_spec, element_size);
                    break;
                case identifier_resize:
                    reader2.read_item(item->parts);
                    break;
                case identifier_width:
                    reader2.read_item(item->width.value);
                    break;
                case identifier_width_dpi: {
                    reader2.read_item(item->width.dpi);
                    dpiRead = true;
                    break;
                }
                case identifier_alignment: {
                    t_uint32 temp;
                    reader2.read_item(temp);
                    item->align = ((alignment)temp);
                } break;
                case identifier_filter_type: {
                    t_uint32 temp;
                    reader2.read_item(temp);
                    item->filter_type = ((playlist_filter_type)temp);
                } break;
                case identifier_use_sort:
                    reader2.read_item(item->use_custom_sort);
                    break;
                case identifier_use_style:
                    reader2.read_item(item->use_custom_colour);
                    break;
                case identifier_show:
                    reader2.read_item(item->show);
                    break;
                default:
                    reader2.skip(element_size);
                    break;
                }
            }
            if (!dpiRead)
                item->width.dpi = uih::get_system_dpi_cached().cx;
            newcolumns.add_item(item);
        }

        g_columns.set_entries_ref(newcolumns);
        pvt::PlaylistView::g_on_columns_change();
    }
};

cui::fcl::dataset_factory<PlaylistViewColumnsDataSet> g_export_columns_t;

class PlaylistViewGroupsDataSet : public cui::fcl::dataset {
    enum t_identifiers {
        identifier_groups,
        identifier_show_groups,
        /*identifier_show_artwork, //Need somewhere to stick these
        identifier_artwork_width,
        identifier_artwork_reflection,*/
    };

    enum t_group_identifiers { identifier_group };

    enum t_columns {
        identifier_script,
        identifier_playlist_filter_mode,
        identifier_playlist_filter_string,

    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Groups"; }
    const GUID& get_group() const override { return cui::fcl::groups::titles_playlist_view; }
    const GUID& get_guid() const override
    {
        // {A89F68C6-B40A-4200-BA2A-68999F704FFD}
        static const GUID guid = {0xa89f68c6, 0xb40a, 0x4200, {0xba, 0x2a, 0x68, 0x99, 0x9f, 0x70, 0x4f, 0xfd}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);

        out.write_item(identifier_show_groups, pvt::cfg_grouping);
        // out.write_item(identifier_show_artwork, pvt::cfg_show_artwork);
        // out.write_item(identifier_artwork_width, pvt::cfg_artwork_width);
        // out.write_item(identifier_artwork_reflection, pvt::cfg_artwork_reflection);

        stream_writer_memblock groups_sw;
        fbh::fcl::Writer groups_writer(&groups_sw, p_abort);

        const pfc::list_base_const_t<pvt::group_t>& groups = pvt::g_groups.get_groups();
        t_size count = groups.get_count();
        pfc::string8 temp;

        groups_writer.write_raw(count);

        for (t_size i = 0; i < count; i++) {
            const pvt::group_t& group = groups[i];
            stream_writer_memblock sw;
            fbh::fcl::Writer w(&sw, p_abort);
            w.write_item(identifier_script, group.string);
            w.write_item(identifier_playlist_filter_mode, (t_uint32)group.filter_type);
            w.write_item(identifier_playlist_filter_string, group.filter_playlists);

            groups_writer.write_item(identifier_group, sw.m_data.get_ptr(), sw.m_data.get_size());
        }

        out.write_item(identifier_groups, groups_sw.m_data.get_ptr(), groups_sw.m_data.get_size());
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);

        t_uint32 element_id;
        t_uint32 element_size;

        pfc::list_t<pvt::group_t> newgroups;
        bool b_groups_set = false;

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id) {
            case identifier_show_groups:
                reader.read_item(pvt::cfg_grouping);
                break;
            // case identifier_artwork_reflection:
            //    reader.read_item(pvt::cfg_artwork_reflection);
            //    break;
            // case identifier_show_artwork:
            //    reader.read_item(pvt::cfg_show_artwork);
            //    break;
            // case identifier_artwork_width:
            //    reader.read_item(pvt::cfg_artwork_width);
            //    break;
            case identifier_groups: {
                t_size count;
                reader.read_item(count);
                for (t_size i = 0; i < count; i++) {
                    t_uint32 group_id;
                    t_uint32 group_size;

                    reader.read_item(group_id);
                    reader.read_item(group_size);

                    pvt::group_t item;

                    fbh::fcl::Reader reader2(reader, group_size, p_abort);

                    t_uint32 group_element_id;
                    t_uint32 group_element_size;

                    while (reader2.get_remaining()) {
                        reader2.read_item(group_element_id);
                        reader2.read_item(group_element_size);

                        switch (group_element_id) {
                        case identifier_script:
                            reader2.read_item(item.string, group_element_size);
                            break;
                        case identifier_playlist_filter_mode: {
                            t_uint32 temp;
                            reader2.read_item(temp);
                            item.filter_type = (playlist_filter_type&)temp;
                        } break;
                        case identifier_playlist_filter_string:
                            reader2.read_item(item.filter_playlists, group_element_size);
                            break;
                        default:
                            reader2.skip(group_element_size);
                            break;
                        }
                    }
                    newgroups.add_item(item);
                }
                b_groups_set = true;
                break;
            }
            default:
                reader.skip(element_size);
                break;
            }
        }

        if (b_groups_set)
            pvt::g_groups.set_groups(newgroups, false);
        pvt::PlaylistView::g_on_groups_change();
        // pvt::ng_playlist_view_t::g_on_show_artwork_change();
        // pvt::ng_playlist_view_t::g_on_artwork_width_change();
    }
};

cui::fcl::dataset_factory<PlaylistViewGroupsDataSet> g_export_groups_t;

class PlaylistViewMiscDataSet : public cui::fcl::dataset {
    enum t_identifiers {
        identifier_global_script,
        identifier_style_script,
        identifier_show_header,
        identifier_autosize_columns,
        identifier_use_globals_for_sorting,
        identifier_use_dates,
        identifier_use_globals,
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Colours"; }
    const GUID& get_group() const override { return cui::fcl::groups::titles_playlist_view; }
    const GUID& get_guid() const override
    {
        // {190F4811-899A-4366-A181-FF5161FC1C77}
        static const GUID guid = {0x190f4811, 0x899a, 0x4366, {0xa1, 0x81, 0xff, 0x51, 0x61, 0xfc, 0x1c, 0x77}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);

        out.write_item(identifier_global_script, cfg_globalstring);
        out.write_item(identifier_style_script, cfg_colour);
        out.write_item(identifier_show_header, cfg_header);
        out.write_item(identifier_autosize_columns, cfg_nohscroll);
        out.write_item(identifier_use_globals_for_sorting, cfg_global_sort);
        out.write_item(identifier_use_dates, static_cast<int32_t>(true));
        out.write_item(identifier_use_globals, cfg_global);
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id) {
            case identifier_global_script:
                reader.read_item(cfg_globalstring, element_size);
                break;
            case identifier_style_script:
                reader.read_item(cfg_colour, element_size);
                break;
            case identifier_show_header:
                reader.read_item(cfg_header);
                break;
            case identifier_autosize_columns:
                reader.read_item(cfg_nohscroll);
                break;
            case identifier_use_globals_for_sorting:
                reader.read_item(cfg_global_sort);
                break;
            case identifier_use_globals:
                reader.read_item(cfg_global);
                break;
            default:
                reader.skip(element_size);
                break;
            }
        }

        pvt::PlaylistView::g_on_autosize_change();
        pvt::PlaylistView::g_on_vertical_item_padding_change();
        pvt::PlaylistView::g_on_show_header_change();
        pvt::PlaylistView::g_update_all_items();
    }
};

cui::fcl::dataset_factory<PlaylistViewMiscDataSet> g_export_pview_t;

class TitlesDataSet : public cui::fcl::dataset {
    enum t_identifiers {
        identifier_main_window_title,
        identifier_status_bar,
        identifier_notification_icon_tooltip,
        identifier_copy_command,
        identifier_playlist,
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Titles"; }
    const GUID& get_group() const override { return cui::fcl::groups::titles_common; }
    const GUID& get_guid() const override
    {
        // {9AF6A28B-3EFF-4d1a-AD81-FA1759F1D66C}
        static const GUID guid = {0x9af6a28b, 0x3eff, 0x4d1a, {0xad, 0x81, 0xfa, 0x17, 0x59, 0xf1, 0xd6, 0x6c}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);

        out.write_item(identifier_status_bar, main_window::config_status_bar_script.get());
        out.write_item(identifier_notification_icon_tooltip, main_window::config_notification_icon_script.get());
        out.write_item(identifier_main_window_title, main_window::config_main_window_title_script.get());
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            pfc::string8 buffer;

            switch (element_id) {
            case identifier_main_window_title:
                reader.read_item(buffer, element_size);
                main_window::config_main_window_title_script.set(buffer);
                break;
            case identifier_notification_icon_tooltip:
                reader.read_item(buffer, element_size);
                main_window::config_notification_icon_script.set(buffer);
                break;
            case identifier_status_bar:
                reader.read_item(buffer, element_size);
                main_window::config_status_bar_script.set(buffer);
                break;
            default:
                reader.skip(element_size);
                break;
            }
        }
    }
};

cui::fcl::dataset_factory<TitlesDataSet> g_export_titles_t;
