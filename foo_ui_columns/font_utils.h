#pragma once

namespace cui::fonts {

struct WeightStretchStyle {
    std::wstring family_name{L"Segoe UI"sv};
    DWRITE_FONT_WEIGHT weight{DWRITE_FONT_WEIGHT_REGULAR};
    DWRITE_FONT_STRETCH stretch{DWRITE_FONT_STRETCH_NORMAL};
    DWRITE_FONT_STYLE style{DWRITE_FONT_STYLE_NORMAL};
};

struct FontDescription {
    LOGFONT log_font{};
    int point_size_tenths{90};
    float dip_size{12.0f};
    std::optional<WeightStretchStyle> wss;

    void estimate_point_and_dip_size();
    void estimate_dip_size();
    void recalculate_log_font_height();
    void fill_wss();
    WeightStretchStyle get_wss_with_fallback();
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
void get_next_font_size_step(LOGFONT& log_font, bool up);

LOGFONT read_font(stream_reader* stream, abort_callback& aborter);
void write_font(stream_writer* stream, const LOGFONT& log_font, abort_callback& aborter);

struct SystemFont {
    LOGFONT log_font{};
    float size{};
};

SystemFont get_icon_font_for_dpi(unsigned dpi);
SystemFont get_menu_font_for_dpi(unsigned dpi);

} // namespace cui::fonts
