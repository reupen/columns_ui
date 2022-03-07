#pragma once
#include "font_utils.h"

class FontManagerData : public cfg_var {
public:
    static const GUID g_cfg_guid;
    enum { cfg_version = 0 };
    void get_data_raw(stream_writer* p_stream, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort) override;

    class Entry {
    public:
        enum ItemID {
            identifier_guid,
            identifier_mode,
            identifier_font,
            identifier_point_size_tenths,
        };
        GUID guid{};
        cui::fonts::FontDescription font_description{};
        cui::fonts::font_mode_t font_mode{cui::fonts::font_mode_system};

        LOGFONT get_normalised_font(unsigned dpi = uih::get_system_dpi_cached().cy);

        void write(stream_writer* p_stream, abort_callback& p_abort);
        void write_extra_data(stream_writer* p_stream, abort_callback& p_abort);
        void read(t_uint32 version, stream_reader* p_stream, abort_callback& p_abort);
        void read_extra_data(stream_reader* p_stream, abort_callback& p_abort);
        void _export(stream_writer* p_stream, abort_callback& p_abort);
        virtual void import(stream_reader* p_reader, t_size stream_size, t_uint32 type, abort_callback& p_abort);
        void reset_fonts();

        Entry();
    };
    using entry_ptr_t = std::shared_ptr<Entry>;
    pfc::list_t<entry_ptr_t> m_entries;
    entry_ptr_t m_common_items_entry;
    entry_ptr_t m_common_labels_entry;

    void find_by_guid(const GUID& p_guid, entry_ptr_t& p_out);

    void register_common_callback(cui::fonts::common_callback* p_callback);
    void deregister_common_callback(cui::fonts::common_callback* p_callback);

    void g_on_common_font_changed(uint32_t mask);

    pfc::ptr_list_t<cui::fonts::common_callback> m_callbacks;

    FontManagerData();
};
