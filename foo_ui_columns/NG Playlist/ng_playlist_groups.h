#pragma once

namespace pvt {

    class group_t
    {
    public:
        group_t(const char * p_string, playlist_filter_type p_filter_type = FILTER_NONE, const char * p_filter = "")
            : string(p_string), filter_type(p_filter_type), filter_playlists(p_filter)
        {};
        group_t() : filter_type(FILTER_NONE) {};
        void write(stream_writer * p_stream,abort_callback & p_abort)
        {
            p_stream->write_string(string.get_ptr(), p_abort);
            p_stream->write_lendian_t((t_size&)filter_type, p_abort);
            p_stream->write_string(filter_playlists.get_ptr(), p_abort);
        }
        void read(t_size version, stream_reader * p_stream,abort_callback & p_abort)
        {
            p_stream->read_string(string, p_abort);
            if (version >= 1)
            {
                p_stream->read_lendian_t((t_size&)filter_type, p_abort);
                p_stream->read_string(filter_playlists, p_abort);
            }
        }
        pfc::string8 string;
        playlist_filter_type filter_type;
        pfc::string8 filter_playlists;
    private:
    };


    class cfg_groups_t : public cfg_var
    {
        enum {stream_version_current=1};
    public:
        const pfc::list_base_const_t<group_t> & get_groups() const {return m_groups;}
        t_size add_group (const group_t & p_group, bool notify_playlist_views = true);
        void remove_group (t_size index);
        void replace_group (t_size index, const group_t & p_group);
        void swap (t_size index1, t_size index2);
        void set_groups(const pfc::list_base_const_t<group_t> & p_groups, bool b_update_views = true);
        cfg_groups_t(const GUID & p_guid) : cfg_var(p_guid)
        {
            add_group(group_t("$if2(%album artist%,<no artist>)[ / %album%]"), false);
        };
    private:
        void get_data_raw(stream_writer * p_stream,abort_callback & p_abort) override
        {
            t_size count = m_groups.get_count(), i;
            p_stream->write_lendian_t(t_size(stream_version_current), p_abort);
            p_stream->write_lendian_t(count, p_abort);
            for (i=0; i<count; i++)
                m_groups[i].write(p_stream, p_abort);
        }

        void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort) override
        {
            t_size count, i, version;
            p_stream->read_lendian_t(version, p_abort);
            if (version <= stream_version_current)
            {
                m_groups.remove_all();
                p_stream->read_lendian_t(count, p_abort);
                m_groups.set_count(count);
                for (i=0; i<count; i++)
                    m_groups[i].read(version, p_stream, p_abort);
            }
        }
        pfc::list_t<group_t> m_groups;
    };
    extern cfg_groups_t g_groups;
}