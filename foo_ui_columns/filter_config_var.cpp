#include "stdafx.h"
#include "filter_config_var.h"

namespace filter_panel {

const GUID g_guid_cfg_sort{0xe9f1557a, 0x4650, 0x4b59, {0xac, 0xd6, 0xb2, 0xad, 0x6e, 0xe9, 0xb8, 0x7d}};
const GUID g_guid_cfg_sort_string{0x4b39f556, 0x9d06, 0x464e, {0xad, 0x16, 0xf, 0x5f, 0xa0, 0x2c, 0xf5, 0x10}};
const GUID g_guid_cfg_autosend{0xa5edcfec, 0x897d, 0x4101, {0xab, 0x8a, 0x39, 0xed, 0x80, 0x7, 0x3a, 0xd1}};
const GUID g_guid_doubleclickaction{0xff6ea2bc, 0x8d58, 0x4a1f, {0xab, 0x13, 0xbc, 0xe, 0x4e, 0x6f, 0x8a, 0x6d}};
const GUID g_guid_middleclickaction{0x8ac152d3, 0x1756, 0x4c46, {0xb4, 0x3b, 0xa4, 0x2f, 0x81, 0x0, 0xfd, 0x2d}};
const GUID guid_cfg_fields{0x78ab6a, 0x26f7, 0x4e5c, {0xa8, 0x59, 0xec, 0xd8, 0x87, 0x40, 0xbb, 0xe6}};
const GUID g_guid_edgestyle{0x1fea2799, 0x9d1c, 0x4b42, {0xaf, 0x83, 0xe5, 0x17, 0x72, 0x58, 0x55, 0x93}};
const GUID g_guid_orderedbysplitters{0x196fc1c1, 0x388a, 0x4fd8, {0x9c, 0xe1, 0x33, 0x20, 0xea, 0xa0, 0xb4, 0xdc}};
const GUID g_guid_showemptyitems{0xca5d2277, 0xf09a, 0x4262, {0xba, 0xce, 0x27, 0x91, 0x92, 0xab, 0xaf, 0x7e}};
const GUID g_guid_itempadding{0x580cc260, 0x119b, 0x4743, {0xac, 0x96, 0xf3, 0x95, 0xd4, 0x6c, 0xc9, 0x90}};
const GUID g_guid_favouritequeries{0x1002788f, 0x898b, 0x46f5, {0xb6, 0xad, 0x17, 0x90, 0x76, 0x32, 0x80, 0x78}};
const GUID g_guid_showsearchclearbutton{0xd88e24f5, 0x2690, 0x4daa, {0xa4, 0x4a, 0xab, 0xbd, 0x74, 0xd7, 0xc4, 0x62}};
const GUID g_guid_show_column_titles{0x236dcbf0, 0x3cc, 0x4d67, {0xa4, 0x5b, 0x26, 0xc6, 0xd4, 0x4c, 0xfa, 0xc9}};
const GUID g_guid_allow_sorting{0x874794a3, 0x904c, 0x4fda, {0x8d, 0xf6, 0x2a, 0xda, 0x1, 0xe3, 0x69, 0x7a}};
const GUID g_guid_show_sort_indicators{0x71e31f54, 0xf410, 0x4f76, {0x84, 0x6d, 0x85, 0x86, 0x72, 0x3e, 0xcc, 0x8a}};

cfg_fields_t cfg_field_list(guid_cfg_fields);

cfg_string cfg_sort_string(g_guid_cfg_sort_string, "%album artist% - %album% - %discnumber% - %tracknumber% - %title%");
cfg_bool cfg_sort(g_guid_cfg_sort, true);
cfg_bool cfg_autosend(g_guid_cfg_autosend, true);
cfg_bool cfg_orderedbysplitters(g_guid_orderedbysplitters, true);
cfg_bool cfg_showemptyitems(g_guid_showemptyitems, false);
cfg_int cfg_doubleclickaction(g_guid_doubleclickaction, 1);
cfg_int cfg_middleclickaction(g_guid_middleclickaction, 0);
cfg_int cfg_edgestyle(g_guid_edgestyle, 2);
fbh::ConfigInt32DpiAware cfg_vertical_item_padding(g_guid_itempadding, 4);
fbh::ConfigBool cfg_show_column_titles(g_guid_show_column_titles, true);
fbh::ConfigBool cfg_allow_sorting(g_guid_allow_sorting, true);
fbh::ConfigBool cfg_show_sort_indicators(g_guid_show_sort_indicators, true);
fbh::ConfigBool cfg_reverse_sort_tracks(
    GUID{0x1edc5277, 0x2dd6, 0x4276, {0x80, 0xb6, 0xda, 0xa4, 0x1e, 0xd4, 0x6e, 0xe5}}, false);

cfg_bool cfg_showsearchclearbutton(g_guid_showsearchclearbutton, true);

cfg_favouriteslist cfg_favourites(g_guid_favouritequeries);

void cfg_fields_t::set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort)
{
    t_uint32 version;
    p_stream->read_lendian_t(version, p_abort);
    if (version <= stream_version_current) {
        t_uint32 count, i;
        p_stream->read_lendian_t(count, p_abort);
        set_count(count);
        for (i = 0; i < count; i++) {
            p_stream->read_string((*this)[i].m_name, p_abort);
            p_stream->read_string((*this)[i].m_field, p_abort);
        }

        uint32_t sub_stream_version;
        bool sub_stream_version_read = false;
        try {
            p_stream->read_lendian_t(sub_stream_version, p_abort);
            sub_stream_version_read = true;
        } catch (const exception_io_data_truncation&) {
        }

        if (sub_stream_version_read && sub_stream_version <= sub_stream_version_current) {
            for (i = 0; i < count; i++) {
                uint32_t extra_data_size;
                p_stream->read_lendian_t(extra_data_size, p_abort);

                pfc::array_staticsize_t<t_uint8> column_extra_data(extra_data_size);
                p_stream->read(column_extra_data.get_ptr(), column_extra_data.get_size(), p_abort);

                stream_reader_memblock_ref column_reader(column_extra_data);
                column_reader.read_lendian_t((*this)[i].m_last_sort_direction, p_abort);
            }
        }
    }
}
void cfg_fields_t::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
{
    p_stream->write_lendian_t((t_uint32)stream_version_current, p_abort);
    t_uint32 i, count = gsl::narrow<uint32_t>(get_count());
    p_stream->write_lendian_t(count, p_abort);
    for (i = 0; i < count; i++) {
        const auto& field = (*this)[i];
        p_stream->write_string(field.m_name, p_abort);
        p_stream->write_string(field.m_field, p_abort);
    }

    p_stream->write_lendian_t(static_cast<uint32_t>(sub_stream_version_current), p_abort);
    for (i = 0; i < count; i++) {
        const auto& field = (*this)[i];
        stream_writer_memblock field_writer;
        field_writer.write_lendian_t(field.m_last_sort_direction, p_abort);

        uint32_t extra_size = field_writer.m_data.get_size();
        p_stream->write_lendian_t(extra_size, p_abort);
        p_stream->write(field_writer.m_data.get_ptr(), field_writer.m_data.get_size(), p_abort);
    }
}
void cfg_fields_t::reset()
{
    set_count(3);
    t_size i = 0;
    (*this)[i].m_name = ((*this)[i].m_field = "Genre");
    i++;
    (*this)[i].m_field = "Album Artist;Artist";
    (*this)[i].m_name = "Artist";
    i++;
    (*this)[i].m_name = ((*this)[i].m_field = "Album");
}

bool cfg_fields_t::have_name(const char* p_name)
{
    t_size count = get_count();
    for (t_size i = 0; i < count; i++)
        if (!stricmp_utf8(p_name, (*this)[i].m_name))
            return true;
    return false;
}

bool cfg_fields_t::find_by_name(const char* p_name, size_t& p_index)
{
    t_size count = get_count();
    for (t_size i = 0; i < count; i++)
        if (!stricmp_utf8(p_name, (*this)[i].m_name)) {
            p_index = i;
            return true;
        }
    return false;
}

void cfg_fields_t::fix_name(const char* p_name, pfc::string8& p_out)
{
    t_size i = 0;
    p_out = p_name;
    while (have_name(p_out)) {
        p_out.reset();
        p_out << p_name << " (" << (++i) << ")";
    }
}
void cfg_fields_t::fix_name(pfc::string8& p_name)
{
    fix_name(pfc::string8(p_name), p_name);
}

void cfg_favouriteslist::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
{
    t_uint32 m = gsl::narrow<t_uint32>(get_count()), v = 0;
    p_stream->write_lendian_t(v, p_abort);
    p_stream->write_lendian_t(m, p_abort);
    for (t_uint32 n = 0; n < m; n++)
        p_stream->write_string(get_item(n), p_abort);
}

void cfg_favouriteslist::set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort)
{
    t_uint32 count, version;
    p_stream->read_lendian_t(version, p_abort);
    if (version <= 0) {
        p_stream->read_lendian_t(count, p_abort);
        pfc::string8_fast_aggressive temp;
        temp.prealloc(32);
        for (t_uint32 n = 0; n < count; n++) {
            p_stream->read_string(temp, p_abort);
            add_item(temp);
        }
    }
}

bool cfg_favouriteslist::have_item(const char* p_item)
{
    t_size count = get_count();
    for (t_size i = 0; i < count; i++)
        if (!strcmp(p_item, get_item(i)))
            return true;
    return false;
}

bool cfg_favouriteslist::find_item(const char* p_item, t_size& index)
{
    t_size count = get_count();
    for (t_size i = 0; i < count; i++)
        if (!strcmp(p_item, get_item(i))) {
            index = i;
            return true;
        }
    return false;
}

} // namespace filter_panel
