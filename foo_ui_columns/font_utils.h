#pragma once
#include "font_manager_v3.h"

namespace cui::fonts {

struct FontDescription {
    LOGFONT log_font{};
    int point_size_tenths{90};
    float dip_size{12.0f};
    std::optional<uih::direct_write::WeightStretchStyle> wss;
    std::wstring typographic_family_name;
    uih::direct_write::AxisValues axis_values;

    void set_dip_size(float size);
    void set_point_size(float size);
    void set_point_size_tenths(int size_tenths);
    void estimate_point_and_dip_size();
    void estimate_dip_size();
    void recalculate_log_font_height();
    void fill_wss();
    uih::direct_write::WeightStretchStyle get_wss_with_fallback();
};

class ConfigFontDescription : public cfg_var {
public:
    ConfigFontDescription(GUID guid, FontDescription font_description)
        : cfg_var(guid)
        , m_font_description(font_description)
    {
    }

    ConfigFontDescription& operator=(FontDescription& font_description)
    {
        m_font_description = font_description;
        return *this;
    }
    FontDescription& operator*() { return m_font_description; }
    FontDescription* operator->() { return &m_font_description; }

private:
    void get_data_raw(stream_writer* stream, abort_callback& aborter) override;
    void set_data_raw(stream_reader* stream, size_t size_hint, abort_callback& aborter) override;

    FontDescription m_font_description{};
};

std::optional<FontDescription> select_font(HWND wnd_parent, LOGFONT initial_font);

LOGFONT read_font(stream_reader* stream, abort_callback& aborter);
void write_font(stream_writer* stream, const LOGFONT& log_font, abort_callback& aborter);

struct SystemFont {
    LOGFONT log_font{};
    float size{};
};

SystemFont get_icon_font_for_dpi(unsigned dpi);
SystemFont get_menu_font_for_dpi(unsigned dpi);

std::optional<uih::direct_write::TextFormat> get_text_format(
    const uih::direct_write::Context::Ptr& context, const font::ptr& font_api);
std::optional<uih::direct_write::TextFormat> get_text_format(const font::ptr& font_api);

} // namespace cui::fonts
