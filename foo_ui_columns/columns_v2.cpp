#include "pch.h"
#include "columns_v2.h"

namespace cui::playlist_view {

namespace {
const std::initializer_list default_columns{
    ColumnDefinition{
        .title{"Artist"},
        .display_script{"[%artist%]"},
        .edit_field{"ARTIST"},
        .width{180},
        .weight{100},
    },
    ColumnDefinition{
        .title{"#"},
        .display_script{"$ifgreater(%totaldiscs%,1,%disc%.,)[%tracknumber%]"},
        .edit_field{"TRACKNUMBER"},
        .width{22},
        .weight{10},
        .alignment{ALIGN_RIGHT},
    },
    ColumnDefinition{
        .title{"Title"},
        .display_script{"[%title%]"},
        .edit_field{"TITLE"},
        .width{300},
        .weight{150},
    },
    ColumnDefinition{
        .title{"Album artist"},
        .is_shown{false},
        .display_script{"[%album artist%]"},
        .edit_field{"ALBUM ARTIST"},
        .width{180},
        .weight{100},
    },
    ColumnDefinition{
        .title{"Album"},
        .display_script{"[%album%]"},
        .edit_field{"ALBUM"},
        .width{200},
        .weight{100},
    },
    ColumnDefinition{
        .title{"Composer"},
        .is_shown{false},
        .display_script{"[%composer%]"},
        .edit_field{"COMPOSER"},
        .width{180},
        .weight{100},
    },
    ColumnDefinition{
        .title{"Comments"},
        .is_shown{false},
        .display_script{"[%comment%]"},
        .edit_field{"COMMENT"},
        .width{200},
        .weight{100},
    },
    ColumnDefinition{
        .title{"Genre"},
        .is_shown{false},
        .display_script{"[%genre%]"},
        .edit_field{"GENRE"},
        .width{100},
        .weight{20},
    },
    ColumnDefinition{
        .title{"Year"},
        .display_script{"$if3(%year%,$year($meta(date,0)),)"},
        .edit_field{"DATE"},
        .width{40},
        .weight{10},
        .alignment{ALIGN_RIGHT},
    },
    ColumnDefinition{
        .title{"Date"},
        .is_shown{false},
        .display_script{"[%date%]"},
        .edit_field{"DATE"},
        .width{80},
        .weight{20},
    },
    ColumnDefinition{
        .title{"Rating"},
        .is_shown{false},
        .display_script{"$if(%rating%,$repeat(★,%rating%)$repeat(☆,$sub(5,%rating%)),)"},
        .edit_field{"RATING"},
        .use_custom_sorting_script{true},
        .sorting_script{"[%rating%]"},
        .width{100},
        .weight{10},
    },
    ColumnDefinition{
        .title{"Codec"},
        .is_shown{false},
        .display_script{"%codec%[ · %codec_profile%]"},
        .width{100},
        .weight{20},
    },
    ColumnDefinition{
        .title{"Channels"},
        .is_shown{false},
        .display_script{"[%channels%]"},
        .use_custom_sorting_script{true},
        .sorting_script{"[$info(channels)]"},
        .width{80},
        .weight{10},

    },
    ColumnDefinition{
        .title{"Sample rate"},
        .is_shown{false},
        .display_script{"[%samplerate% Hz]"},
        .use_custom_sorting_script{true},
        .sorting_script{"[%samplerate%]"},
        .width{80},
        .weight{10},
        .alignment{ALIGN_RIGHT},
    },
    ColumnDefinition{
        .title{"Bit rate"},
        .is_shown{false},
        .display_script{"[%bitrate% kbps]"},
        .use_custom_sorting_script{true},
        .sorting_script{"[%bitrate%]"},
        .width{65},
        .weight{10},
        .alignment{ALIGN_RIGHT},
    },
    ColumnDefinition{
        .title{"Track gain"},
        .is_shown{false},
        .display_script{"[%replaygain_track_gain%]"},
        .use_custom_sorting_script{true},
        .sorting_script{"[$add($replace(%replaygain_track_gain%,.,),100000)]"},
        .width{80},
        .weight{10},
        .alignment{ALIGN_RIGHT},
    },
    ColumnDefinition{
        .title{"Album gain"},
        .is_shown{false},
        .display_script{"[%replaygain_album_gain%]"},
        .use_custom_sorting_script{true},
        .sorting_script{"[$add($replace(%replaygain_album_gain%,.,),100000)]"},
        .width{80},
        .weight{10},
        .alignment{ALIGN_RIGHT},
    },
    ColumnDefinition{
        .title{"Path"},
        .is_shown{false},
        .display_script{"[%path%]"},
        .width{200},
        .weight{150},
    },
    ColumnDefinition{
        .title{"Last modified"},
        .is_shown{false},
        .display_script{"[%last_modified%]"},
        .width{120},
        .weight{10},
    },
    ColumnDefinition{
        .title{"Size"},
        .is_shown{false},
        .display_script{"[%filesize_natural%]"},
        .use_custom_sorting_script{true},
        .sorting_script{"[%filesize%]"},
        .width{80},
        .weight{20},
        .alignment{ALIGN_RIGHT},
    },
    ColumnDefinition{
        .title{"Play count"},
        .is_shown{false},
        .display_script{"[%play_count%]"},
        .width{70},
        .weight{10},
        .alignment{ALIGN_RIGHT},
    },
    ColumnDefinition{
        .title{"Last played"},
        .is_shown{false},
        .display_script{"[%last_played%]"},
        .width{120},
        .weight{10},
    },
    ColumnDefinition{
        .title{"Length"},
        .display_script{"[%playback_time% ∕ ]%length%"},
        .use_custom_sorting_script{true},
        .sorting_script{"[%length_seconds%]"},
        .width{80},
        .weight{10},
        .alignment{ALIGN_RIGHT},
    },
};

} // namespace

} // namespace cui::playlist_view

void PlaylistViewColumn::read(stream_reader* reader, abort_callback& abortCallback)
{
    def.width.dpi = uih::get_system_dpi_cached().cx;
    reader->read_string(def.title, abortCallback);
    reader->read_string(def.display_script, abortCallback);
    reader->read_lendian_t(def.use_custom_style_script, abortCallback);
    reader->read_string(def.style_script, abortCallback);
    reader->read_lendian_t(def.use_custom_sorting_script, abortCallback);
    reader->read_string(def.sorting_script, abortCallback);
    reader->read_lendian_t(def.width.value, abortCallback);
    reader->read_lendian_t(def.alignment, abortCallback);
    reader->read_lendian_t(def.playlist_filter_mode, abortCallback);
    reader->read_string(def.playlist_filter_pattern, abortCallback);
    reader->read_lendian_t(def.weight, abortCallback);
    reader->read_lendian_t(def.is_shown, abortCallback);
    reader->read_string(def.edit_field, abortCallback);
}

void PlaylistViewColumn::read_extra(
    stream_reader* reader, ColumnStreamVersion streamVersion, abort_callback& abortCallback)
{
    if (streamVersion >= ColumnStreamVersion::streamVersion1) {
        reader->read_lendian_t(def.width.dpi, abortCallback);
    }
}

void PlaylistViewColumn::write(stream_writer* out, abort_callback& abortCallback) const
{
    out->write_string(def.title.get_ptr(), abortCallback);
    out->write_string(def.display_script.get_ptr(), abortCallback);
    out->write_lendian_t(def.use_custom_style_script, abortCallback);
    out->write_string(def.style_script, abortCallback);
    out->write_lendian_t(def.use_custom_sorting_script, abortCallback);
    out->write_string(def.sorting_script, abortCallback);
    out->write_lendian_t(def.width.value, abortCallback);
    out->write_lendian_t(def.alignment, abortCallback);
    out->write_lendian_t(def.playlist_filter_mode, abortCallback);
    out->write_string(def.playlist_filter_pattern, abortCallback);
    out->write_lendian_t(def.weight, abortCallback);
    out->write_lendian_t(def.is_shown, abortCallback);
    out->write_string(def.edit_field, abortCallback);
}

void PlaylistViewColumn::write_extra(stream_writer* out, abort_callback& abortCallback) const
{
    out->write_lendian_t(def.width.dpi, abortCallback);
}

bool ColumnList::move_up(size_t idx)
{
    const auto count = get_count();
    if (idx > 0 && idx < count) {
        order_helper order(count);
        order.swap(idx, idx - 1);
        reorder(order.get_ptr());
        return true;
    }
    return false;
}

bool ColumnList::move(size_t from, size_t to)
{
    const auto count = get_count();
    auto n = from;
    auto idx = to;
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

bool ColumnList::move_down(size_t idx)
{
    const auto count = get_count();
    if (count > 0 && idx < count - 1) {
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

    size_t num = get_count();

    out->write_lendian_t(gsl::narrow<uint32_t>(num), p_abort);

    for (size_t n = 0; n < num; n++)
        get_item(n)->write(out, p_abort);

    // Extra data added in version 0.5.0
    out->write_lendian_t(static_cast<uint32_t>(ColumnStreamVersion::streamVersionCurrent), p_abort);

    for (size_t n = 0; n < num; n++) {
        stream_writer_memblock columnExtraData;
        get_item(n)->write_extra(&columnExtraData, p_abort);
        out->write_lendian_t(gsl::narrow<uint32_t>(columnExtraData.m_data.get_size()), p_abort);
        out->write(columnExtraData.m_data.get_ptr(), columnExtraData.m_data.get_size(), p_abort);
    }
}

void ConfigColumns::set_data_raw(stream_reader* p_reader, size_t p_sizehint, abort_callback& p_abort)
{
    list_t<PlaylistViewColumn::ptr> items;
    ColumnStreamVersion streamVersion = ColumnStreamVersion::streamVersion0;

    uint32_t num;
    p_reader->read_lendian_t(num, p_abort);
    for (size_t i = 0; i < num; i++) {
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
        for (size_t i = 0; i < num; i++) {
            uint32_t columnExtraDataSize;
            p_reader->read_lendian_t(columnExtraDataSize, p_abort);
            pfc::array_staticsize_t<uint8_t> columnExtraData(columnExtraDataSize);
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

    for (const auto& def : cui::playlist_view::default_columns) {
        add_item(std::make_shared<PlaylistViewColumn>(def));
    }
}

ConfigColumns::ConfigColumns(const GUID& p_guid) : cfg_var(p_guid)
{
    reset();
}
