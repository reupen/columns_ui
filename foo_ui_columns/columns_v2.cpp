#include "stdafx.h"
#include "playlist_view_tfhooks.h"
#include "columns_v2.h"

void PlaylistViewColumn::read(stream_reader* reader, abort_callback& abortCallback)
{
    width.dpi = uih::get_system_dpi_cached().cx;
    reader->read_string(name, abortCallback);
    reader->read_string(spec, abortCallback);
    reader->read_lendian_t(use_custom_colour, abortCallback);
    reader->read_string(colour_spec, abortCallback);
    reader->read_lendian_t(use_custom_sort, abortCallback);
    reader->read_string(sort_spec, abortCallback);
    reader->read_lendian_t(width.value, abortCallback);
    reader->read_lendian_t(align, abortCallback);
    reader->read_lendian_t(filter_type, abortCallback);
    reader->read_string(filter, abortCallback);
    reader->read_lendian_t(parts, abortCallback);
    reader->read_lendian_t(show, abortCallback);
    reader->read_string(edit_field, abortCallback);
}

void PlaylistViewColumn::read_extra(
    stream_reader* reader, ColumnStreamVersion streamVersion, abort_callback& abortCallback)
{
    if (streamVersion >= ColumnStreamVersion::streamVersion1) {
        reader->read_lendian_t(width.dpi, abortCallback);
    }
}

void PlaylistViewColumn::write(stream_writer* out, abort_callback& abortCallback) const
{
    out->write_string(name.get_ptr(), abortCallback);
    out->write_string(spec.get_ptr(), abortCallback);
    out->write_lendian_t(use_custom_colour, abortCallback);
    out->write_string(colour_spec, abortCallback);
    out->write_lendian_t(use_custom_sort, abortCallback);
    out->write_string(sort_spec, abortCallback);
    out->write_lendian_t(width.value, abortCallback);
    out->write_lendian_t(align, abortCallback);
    out->write_lendian_t(filter_type, abortCallback);
    out->write_string(filter, abortCallback);
    out->write_lendian_t(parts, abortCallback);
    out->write_lendian_t(show, abortCallback);
    out->write_string(edit_field, abortCallback);
}

void PlaylistViewColumn::write_extra(stream_writer* out, abort_callback& abortCallback) const
{
    out->write_lendian_t(width.dpi, abortCallback);
}

bool ColumnList::move_up(t_size idx)
{
    unsigned count = get_count();
    if (idx > 0 && idx < count) {
        order_helper order(count);
        order.swap(idx, idx - 1);
        reorder(order.get_ptr());
        return true;
    }
    return false;
}

bool ColumnList::move(t_size from, t_size to)
{
    unsigned count = get_count();
    unsigned n = from;
    unsigned idx = to;
    bool rv = false;

    order_helper order(count);

    if (n < idx) {
        while (n < idx && n < count) {
            order.swap(n, n + 1);
            n++;
        }
    } else if (n > idx) {
        while (n > idx && n > 0) {
            order.swap(n, n - 1);
            n--;
        }
    }
    if (n != from) {
        reorder(order.get_ptr());
        rv = true;
    }
    return rv;
}

bool ColumnList::move_down(t_size idx)
{
    unsigned count = get_count();
    if (idx >= 0 && idx < (count - 1)) {
        order_helper order(count);
        order.swap(idx, idx + 1);
        reorder(order.get_ptr());
        return true;
    }
    return false;
}

void ConfigColumns::get_data_raw(stream_writer* out, abort_callback& p_abort)
{
    // if (!cfg_nohscroll) playlist_view::g_save_columns(); FIXME

    t_size num = get_count();

    out->write_lendian_t(num, p_abort);

    for (t_size n = 0; n < num; n++)
        get_item(n)->write(out, p_abort);

    // Extra data added in version 0.5.0
    out->write_lendian_t(static_cast<uint32_t>(ColumnStreamVersion::streamVersionCurrent), p_abort);

    for (t_size n = 0; n < num; n++) {
        stream_writer_memblock columnExtraData;
        get_item(n)->write_extra(&columnExtraData, p_abort);
        out->write_lendian_t(gsl::narrow<uint32_t>(columnExtraData.m_data.get_size()), p_abort);
        out->write(columnExtraData.m_data.get_ptr(), columnExtraData.m_data.get_size(), p_abort);
    }
}

void ConfigColumns::set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort)
{
    pfc::list_t<PlaylistViewColumn::ptr> items;
    ColumnStreamVersion streamVersion = ColumnStreamVersion::streamVersion0;

    t_uint32 num;
    p_reader->read_lendian_t(num, p_abort);
    for (t_size i = 0; i < num; i++) {
        PlaylistViewColumn::ptr item = std::make_shared<PlaylistViewColumn>();
        item->read(p_reader, p_abort);
        items.add_item(item);
    }

    // Extra data added in version 0.5.0
    try {
        uint32_t streamVersion_;
        p_reader->read_lendian_t(streamVersion_, p_abort);
        streamVersion = static_cast<ColumnStreamVersion>(streamVersion_);
    } catch (const exception_io_data_truncation&) {
    }

    if (streamVersion >= ColumnStreamVersion::streamVersion1) {
        for (t_size i = 0; i < num; i++) {
            uint32_t columnExtraDataSize;
            p_reader->read_lendian_t(columnExtraDataSize, p_abort);
            pfc::array_staticsize_t<t_uint8> columnExtraData(columnExtraDataSize);
            p_reader->read(columnExtraData.get_ptr(), columnExtraData.get_size(), p_abort);
            stream_reader_memblock_ref columnReader(columnExtraData);
            items[i]->read_extra(&columnReader, streamVersion, p_abort);
        }
    }
    set_entries_ref(items);
}

void ConfigColumns::reset()
{
    remove_all();
    add_item(std::make_shared<PlaylistViewColumn>(
        "Artist", "[%artist%]", false, "", false, "", 180, ALIGN_LEFT, FILTER_NONE, "", 180, true, "ARTIST"));
    add_item(std::make_shared<PlaylistViewColumn>(
        "#", "[%tracknumber%]", false, "", false, "", 18, ALIGN_RIGHT, FILTER_NONE, "", 18, true, "TRACKNUMBER"));
    add_item(std::make_shared<PlaylistViewColumn>(
        "Title", "[%title%]", false, "", false, "", 300, ALIGN_LEFT, FILTER_NONE, "", 300, true, "TITLE"));
    add_item(std::make_shared<PlaylistViewColumn>(
        "Album", "[%album%]", false, "", false, "", 200, ALIGN_LEFT, FILTER_NONE, "", 200, true, "ALBUM"));
    add_item(std::make_shared<PlaylistViewColumn>(
        "Date", "[%date%]", false, "", false, "", 60, ALIGN_LEFT, FILTER_NONE, "", 60, true, "DATE"));
    add_item(std::make_shared<PlaylistViewColumn>("Length", "[%_time_elapsed% / ]%_length%", false, "", true,
        "$num(%_length_seconds%,6)", 60, ALIGN_RIGHT, FILTER_NONE, "", 60, true, ""));
}

ConfigColumns::ConfigColumns(const GUID& p_guid, ColumnStreamVersion streamVersion) : cfg_var(p_guid)
{
    reset();
}
