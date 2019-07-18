#ifndef _COLUMNS_2_H_
#define _COLUMNS_2_H_

class PlaylistViewColumnBase : public pfc::refcounted_object_root {
    using self_t = PlaylistViewColumnBase;

public:
    using ptr = pfc::refcounted_object_ptr_t<self_t>;

    pfc::string8 name;
    pfc::string8 spec;
    bool use_custom_colour{false};
    pfc::string8 colour_spec;
    bool use_custom_sort{false};
    pfc::string8 sort_spec;
    uih::IntegerAndDpi<int32_t> width{100};
    alignment align{ALIGN_LEFT};
    playlist_filter_type filter_type{FILTER_NONE};
    pfc::string8 filter;
    t_uint32 parts{1};
    bool show{true};
    pfc::string8 edit_field;

    PlaylistViewColumnBase() = default;

    PlaylistViewColumnBase(const char* pname, const char* pspec, bool b_use_custom_colour, const char* p_colour_spec,
        bool b_use_custom_sort, const char* p_sort_spec, int p_width, alignment p_align,
        playlist_filter_type p_filter_type, const char* p_filter_string, unsigned p_parts, bool b_show,
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
        , edit_field(p_edit_field){};
};

enum class ColumnStreamVersion {
    streamVersion0 = 0,
    streamVersion1 = 1,
    streamVersionCurrent = streamVersion1

};

class PlaylistViewColumn : public PlaylistViewColumnBase {
    using self_t = PlaylistViewColumn;

public:
    using ptr = pfc::refcounted_object_ptr_t<self_t>;

    ptr source_item;

    void read(stream_reader* reader, abort_callback& abortCallback);
    void write(stream_writer* writer, abort_callback& abortCallback) const;
    void read_extra(stream_reader* reader, ColumnStreamVersion streamVersion, abort_callback& abortCallback);
    void write_extra(stream_writer* writer, abort_callback& abortCallback) const;

    PlaylistViewColumn() = default;

    PlaylistViewColumn(const char* pname, const char* pspec, bool b_use_custom_colour, const char* p_colour_spec,
        bool b_use_custom_sort, const char* p_sort_spec, unsigned p_width, alignment p_align,
        playlist_filter_type p_filter_type, const char* p_filter_string, unsigned p_parts, bool b_show,
        const char* p_edit_field)
        : PlaylistViewColumnBase(pname, pspec, b_use_custom_colour, p_colour_spec, b_use_custom_sort, p_sort_spec, p_width,
              p_align, p_filter_type, p_filter_string, p_parts, b_show, p_edit_field)
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
        t_size count = entries.get_count();
        set_count(count);
        for (t_size i = 0; i < count; i++) {
            PlaylistViewColumn::ptr item = new PlaylistViewColumn(*entries[i].get_ptr());
            if (keep_reference_to_source_items)
                item->source_item = entries[i];
            (*this)[i] = item;
        }
    }

    void set_widths(ColumnListCRef entries)
    {
        // remove_all();
        t_size count = get_count();
        if (count == entries.get_count())
            for (t_size i = 0; i < count; i++)
                (*this)[i]->width = entries[i]->width;
    }

    void set_widths(const pfc::list_base_const_t<t_size>& widths)
    {
        // remove_all();
        t_size count = get_count();
        if (count == widths.get_count())
            for (t_size i = 0; i < count; i++)
                (*this)[i]->width = widths[i];
    }

    bool move_up(t_size idx);
    bool move_down(t_size idx);
    bool move(t_size from, t_size to);
};

class ConfigColumns
    : public cfg_var
    , public ColumnList {
public:
    void reset();

    ConfigColumns(const GUID& p_guid, ColumnStreamVersion streamVersion);

protected:
    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort) override;
};

extern ConfigColumns g_columns;

#endif
