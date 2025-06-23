#pragma once
#include "font_utils.h"

namespace cui::fonts {

enum class FontMode {
    CommonItems,
    CommonLabels,
    Custom,
    System,
};

enum class RenderingMode : int32_t {
    Automatic,
    DirectWriteAutomatic,
    Natural,
    NaturalSymmetric,
    GdiClassic,
    GdiNatural,
    GdiAliased,
};

extern fbh::ConfigInt32 rendering_mode;
extern fbh::ConfigBool use_greyscale_antialiasing;
extern fbh::ConfigBool use_colour_glyphs;
extern fbh::ConfigBool use_alternative_emoji_font_selection;
extern fbh::ConfigString colour_emoji_font_family;
extern fbh::ConfigString monochrome_emoji_font_family;

DWRITE_RENDERING_MODE get_rendering_mode();

} // namespace cui::fonts

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
            identifier_weight_stretch_style,
            identifier_dip_size,
            identifier_typographic_font_family,
            identifier_axis_values,
        };
        GUID guid{};
        cui::fonts::FontDescription font_description{};
        cui::fonts::FontMode font_mode{cui::fonts::FontMode::System};

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

    entry_ptr_t find_by_id(GUID id);
    cui::fonts::FontDescription resolve_font_description(const entry_ptr_t& entry);

    void add_font_callback(GUID id, cui::basic_callback::ptr callback);
    void remove_font_callback(GUID id, cui::basic_callback::ptr callback);
    void dispatch_client_font_changed(cui::fonts::client::ptr client) const;

    void add_rendering_options_callback(cui::basic_callback::ptr callback);
    void remove_rendering_options_callback(cui::basic_callback::ptr callback);
    void dispatch_rendering_options_changed() const;

    void add_font_fallback_callback(cui::basic_callback::ptr callback);
    void remove_font_fallback_callback(cui::basic_callback::ptr callback);
    void dispatch_font_fallback_changed() const;

    void add_common_callback(cui::fonts::common_callback* p_callback);
    void remove_common_callback(cui::fonts::common_callback* p_callback);
    void dispatch_common_font_changed(uint32_t mask) const;

    void dispatch_all_fonts_changed() const;

    FontManagerData();
    ~FontManagerData();

private:
    pfc::ptr_list_t<cui::fonts::common_callback> m_common_font_callbacks;
    std::unordered_map<GUID, std::vector<cui::basic_callback::ptr>> m_font_callback_map;
    std::vector<cui::basic_callback::ptr> m_rendering_options_callbacks;
    std::vector<cui::basic_callback::ptr> m_font_fallback_callbacks;
};
