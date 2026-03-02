#pragma once

#include "common.h"

namespace cui::playlist_view {

struct ColumnDefinition {
    pfc::string8 title;
    bool is_shown{true};
    pfc::string8 display_script;
    pfc::string8 edit_field;
    bool use_custom_style_script{};
    pfc::string8 style_script;
    bool use_custom_sorting_script{};
    pfc::string8 sorting_script;
    uih::IntegerAndDpi<int32_t> width{100};
    uint32_t weight{1};
    Alignment alignment{ALIGN_LEFT};
    PlaylistFilterType playlist_filter_mode{FILTER_NONE};
    pfc::string8 playlist_filter_pattern;
};

} // namespace cui::playlist_view

enum class ColumnStreamVersion {
    streamVersion0 = 0,
    streamVersion1 = 1,
    streamVersionCurrent = streamVersion1
};

class PlaylistViewColumn {
    using self_t = PlaylistViewColumn;

public:
    using ptr = std::shared_ptr<self_t>;

    ptr source_item;
    cui::playlist_view::ColumnDefinition def;

    void read(stream_reader* reader, abort_callback& abortCallback);
    void write(stream_writer* writer, abort_callback& abortCallback) const;
    void read_extra(stream_reader* reader, ColumnStreamVersion streamVersion, abort_callback& abortCallback);
    void write_extra(stream_writer* writer, abort_callback& abortCallback) const;

    PlaylistViewColumn() = default;
    PlaylistViewColumn(const cui::playlist_view::ColumnDefinition& def_) : def(def_) {}

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

    ConfigColumns(const GUID& p_guid);

protected:
    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_reader, size_t p_sizehint, abort_callback& p_abort) override;
};

extern ConfigColumns g_columns;
