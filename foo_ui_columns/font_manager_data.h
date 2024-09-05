#pragma once
#include "font_utils.h"

class FontManagerData : public cfg_var {
public:
    static const GUID g_cfg_guid;
    enum {
        cfg_version = 0
    };
    void get_data_raw(stream_writer* p_stream, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_stream, size_t p_sizehint, abort_callback& p_abort) override;

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
        void write_extra_data(stream_writer* stream, abort_callback& aborter) const;
        void write_extra_data_v2(stream_writer* stream, abort_callback& aborter) const;
        void read(uint32_t version, stream_reader* p_stream, abort_callback& p_abort);
        void read_extra_data(stream_reader* stream, abort_callback& aborter);
        void read_extra_data_v2(stream_reader* stream, abort_callback& aborter);
        void _export(stream_writer* p_stream, abort_callback& p_abort);
        void import(stream_reader* p_reader, size_t stream_size, uint32_t type, abort_callback& p_abort);
        void reset_fonts();

        Entry();
    };
    using entry_ptr_t = std::shared_ptr<Entry>;
    pfc::list_t<entry_ptr_t> m_entries;
    entry_ptr_t m_common_items_entry;
    entry_ptr_t m_common_labels_entry;

    entry_ptr_t find_by_guid(GUID id);
    cui::fonts::FontDescription resolve_font_description(const entry_ptr_t& entry);

    void register_common_callback(cui::fonts::common_callback* p_callback);
    void deregister_common_callback(cui::fonts::common_callback* p_callback);

    void g_on_common_font_changed(uint32_t mask);
    void on_rendering_options_change();

    pfc::ptr_list_t<cui::fonts::common_callback> m_callbacks;

    FontManagerData();
};

namespace cui::fonts {

enum class RenderingMode : int32_t {
    Automatic = DWRITE_RENDERING_MODE_DEFAULT,
    Natural = DWRITE_RENDERING_MODE_NATURAL,
    NaturalSymmetric = DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC,
    GdiClassic = DWRITE_RENDERING_MODE_GDI_CLASSIC,
    GdiNatural = DWRITE_RENDERING_MODE_GDI_NATURAL,
};

extern fbh::ConfigInt32 rendering_mode;
extern fbh::ConfigBool force_greyscale_antialiasing;

DWRITE_RENDERING_MODE get_rendering_mode();

} // namespace cui::fonts
