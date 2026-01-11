#pragma once

#include "common.h"
#include "ng_playlist_style.h"

namespace cui::panels::playlist_view {

class PlaylistViewGroup : public uih::ListView::Group {
public:
    using self_t = PlaylistViewGroup;
    using ptr = pfc::refcounted_object_ptr_t<self_t>;
    SharedCellStyleData::ptr m_style_data;

    bool m_artwork_load_attempted{false};
    bool m_artwork_load_succeeded{false};
    wil::shared_hbitmap m_artwork_bitmap;

    void reset_artwork()
    {
        m_artwork_bitmap.reset();
        m_artwork_load_attempted = false;
        m_artwork_load_succeeded = false;
    }

    PlaylistViewGroup() = default;
};

class Group {
public:
    explicit Group(const char* p_string, PlaylistFilterType p_filter_type = FILTER_NONE, const char* p_filter = "")
        : string(p_string)
        , filter_type(p_filter_type)
        , filter_playlists(p_filter)
    {
    }
    Group() = default;
    void write(stream_writer* p_stream, abort_callback& p_abort)
    {
        p_stream->write_string(string.get_ptr(), p_abort);
        p_stream->write_lendian_t(reinterpret_cast<uint32_t&>(filter_type), p_abort);
        p_stream->write_string(filter_playlists.get_ptr(), p_abort);
    }
    void read(uint32_t version, stream_reader* p_stream, abort_callback& p_abort)
    {
        p_stream->read_string(string, p_abort);
        if (version >= 1) {
            p_stream->read_lendian_t(reinterpret_cast<uint32_t&>(filter_type), p_abort);
            p_stream->read_string(filter_playlists, p_abort);
        }
    }
    pfc::string8 string;
    PlaylistFilterType filter_type{FILTER_NONE};
    pfc::string8 filter_playlists;

private:
};

class ConfigGroups : public cfg_var {
    enum {
        stream_version_current = 1
    };

public:
    const pfc::list_base_const_t<Group>& get_groups() const { return m_groups; }
    size_t add_group(const Group& p_group, bool notify_playlist_views = true);
    void remove_group(size_t index);
    void replace_group(size_t index, const Group& p_group);
    void swap(size_t index1, size_t index2);
    void reorder(const size_t* order);
    void set_groups(const pfc::list_base_const_t<Group>& p_groups, bool b_update_views = true);
    explicit ConfigGroups(const GUID& p_guid) : cfg_var(p_guid)
    {
        add_group(Group("$if2(%album artist%,<no artist>)[ / %album%]"), false);
    }

private:
    void get_data_raw(stream_writer* p_stream, abort_callback& p_abort) override
    {
        size_t count = m_groups.get_count();
        p_stream->write_lendian_t(static_cast<uint32_t>(stream_version_current), p_abort);
        p_stream->write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
        for (size_t i = 0; i < count; i++)
            m_groups[i].write(p_stream, p_abort);
    }

    void set_data_raw(stream_reader* p_stream, size_t p_sizehint, abort_callback& p_abort) override
    {
        const auto version = p_stream->read_lendian_t<uint32_t>(p_abort);
        if (version <= stream_version_current) {
            m_groups.remove_all();
            const auto count = p_stream->read_lendian_t<uint32_t>(p_abort);
            m_groups.set_count(count);
            for (size_t i = 0; i < count; i++)
                m_groups[i].read(version, p_stream, p_abort);
        }
    }
    pfc::list_t<Group> m_groups;
};

extern ConfigGroups g_groups;

} // namespace cui::panels::playlist_view
