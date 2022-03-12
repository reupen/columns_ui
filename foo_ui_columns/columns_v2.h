#pragma once

#include "common.h"

class PlaylistViewColumnBase {
public:
    pfc::string8 name;
    pfc::string8 spec;
    bool use_custom_colour{false};
    pfc::string8 colour_spec;
    bool use_custom_sort{false};
    pfc::string8 sort_spec;
    uih::IntegerAndDpi<int32_t> width{100};
    Alignment align{ALIGN_LEFT};
    PlaylistFilterType filter_type{FILTER_NONE};
    pfc::string8 filter;
    uint32_t parts{1};
    bool show{true};
    pfc::string8 edit_field;

    PlaylistViewColumnBase() = default;

    PlaylistViewColumnBase(const char* pname, const char* pspec, bool b_use_custom_colour, const char* p_colour_spec,
        bool b_use_custom_sort, const char* p_sort_spec, int p_width, Alignment p_align,
        PlaylistFilterType p_filter_type, const char* p_filter_string, unsigned p_parts, bool b_show,
        const char* p_edit_field)
        : name(pname)
        , spec(pspec)
        , use_custom_colour(b_use_custom_colour)
        , colour_spec(p_colour_spec)
        , use_custom_sort(b_use_custom_sort)
        , sort_spec(p_sort_spec)
        , width(p_width)
        , align(p_align)
        , filter_type(p_filter_type)
        , filter(p_filter_string)
        , parts(p_parts)
        , show(b_show)
        , edit_field(p_edit_field)
    {
    }
};

enum class ColumnStreamVersion {
    streamVersion0 = 0,
    streamVersion1 = 1,
    streamVersionCurrent = streamVersion1

};

class PlaylistViewColumn : public PlaylistViewColumnBase {
    using self_t = PlaylistViewColumn;

public:
    using ptr = std::shared_ptr<self_t>;

    ptr source_item;

    void read(stream_reader* reader, abort_callback& abortCallback);
    void write(stream_writer* writer, abort_callback& abortCallback) const;
    void read_extra(stream_reader* reader, ColumnStreamVersion streamVersion, abort_callback& abortCallback);
    void write_extra(stream_writer* writer, abort_callback& abortCallback) const;

    PlaylistViewColumn() = default;

    PlaylistViewColumn(const char* pname, const char* pspec, bool b_use_custom_colour, const char* p_colour_spec,
        bool b_use_custom_sort, const char* p_sort_spec, unsigned p_width, Alignment p_align,
        PlaylistFilterType p_filter_type, const char* p_filter_string, unsigned p_parts, bool b_show,
        const char* p_edit_field)
        : PlaylistViewColumnBase(pname, pspec, b_use_custom_colour, p_colour_spec, b_use_custom_sort, p_sort_spec,
            p_width, p_align, p_filter_type, p_filter_string, p_parts, b_show, p_edit_field)
    {
    }

    PlaylistViewColumn(const PlaylistViewColumn&) = default;
    PlaylistViewColumn& operator=(const PlaylistViewColumn&) = default;
    PlaylistViewColumn(PlaylistViewColumn&&) = default;
    PlaylistViewColumn& operator=(PlaylistViewColumn&&) = default;
    ~PlaylistViewColumn() = default;

private:
    service_ptr_t<titleformat_object> to_display;
    service_ptr_t<titleformat_object> to_colour;
    service_ptr_t<titleformat_object> to_sort;
};

using ColumnListCRef = const pfc::list_base_const_t<PlaylistViewColumn::ptr>&;

class ColumnList : public pfc::list_t<PlaylistViewColumn::ptr> {
public:
    void set_entries_ref(ColumnListCRef entries)
    {
        remove_all();
        add_items(entries);
    }

    void set_entries_copy(ColumnListCRef entries, bool keep_reference_to_source_items = false)
    {
        // remove_all();
        size_t count = entries.get_count();
        set_count(count);
        for (size_t i = 0; i < count; i++) {
            PlaylistViewColumn::ptr item = std::make_shared<PlaylistViewColumn>(*entries[i].get());
            if (keep_reference_to_source_items)
                item->source_item = entries[i];
            (*this)[i] = item;
        }
    }

    bool move_up(size_t idx);
    bool move_down(size_t idx);
    bool move(size_t from, size_t to);
};

class ConfigColumns
    : public cfg_var
    , public ColumnList {
public:
    void reset();

    ConfigColumns(const GUID& p_guid, ColumnStreamVersion streamVersion);

protected:
    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_reader, size_t p_sizehint, abort_callback& p_abort) override;
};

extern ConfigColumns g_columns;
