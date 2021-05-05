#pragma once

namespace cui::fonts {

struct FontDescription {
    LOGFONT log_font{};
    int point_size_tenths{};

    void estimate_point_size();
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
    void set_data_raw(stream_reader* stream, t_size size_hint, abort_callback& aborter) override;

    FontDescription m_font_description{};
};

std::optional<FontDescription> select_font(HWND wnd_parent, LOGFONT initial_font);
void get_next_font_size_step(LOGFONT& log_font, bool up);

LOGFONT read_font(stream_reader* stream, abort_callback& aborter);
void write_font(stream_writer* stream, const LOGFONT& log_font, abort_callback& aborter);

} // namespace cui::fonts
