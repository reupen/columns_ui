#include "pch.h"

#include "font_utils.h"

namespace cui::fonts {

void FontDescription::estimate_point_size()
{
    point_size_tenths = -MulDiv(log_font.lfHeight, 720, uih::get_system_dpi_cached().cy);
}

void ConfigFontDescription::get_data_raw(stream_writer* stream, abort_callback& aborter)
{
    write_font(stream, m_font_description.log_font, aborter);
    stream->write_lendian_t(m_font_description.point_size_tenths, aborter);
}

void ConfigFontDescription::set_data_raw(stream_reader* stream, t_size size_hint, abort_callback& aborter)
{
    m_font_description.log_font = read_font(stream, aborter);
    m_font_description.estimate_point_size();
    try {
        stream->read_lendian_t(m_font_description.point_size_tenths, aborter);
    } catch (const exception_io_data_truncation&) {
    }
}

LOGFONT read_font(stream_reader* stream, abort_callback& aborter)
{
    LOGFONT log_font{};

    stream->read_lendian_t(log_font.lfHeight, aborter);
    stream->read_lendian_t(log_font.lfWidth, aborter);
    stream->read_lendian_t(log_font.lfEscapement, aborter);
    stream->read_lendian_t(log_font.lfOrientation, aborter);
    stream->read_lendian_t(log_font.lfWeight, aborter);

    stream->read_lendian_t(log_font.lfItalic, aborter);
    stream->read_lendian_t(log_font.lfUnderline, aborter);
    stream->read_lendian_t(log_font.lfStrikeOut, aborter);
    stream->read_lendian_t(log_font.lfCharSet, aborter);
    stream->read_lendian_t(log_font.lfOutPrecision, aborter);
    stream->read_lendian_t(log_font.lfClipPrecision, aborter);
    stream->read_lendian_t(log_font.lfQuality, aborter);
    stream->read_lendian_t(log_font.lfPitchAndFamily, aborter);
    stream->read(&log_font.lfFaceName, sizeof(log_font.lfFaceName), aborter);

    return log_font;
}

void write_font(stream_writer* stream, const LOGFONT& log_font, abort_callback& aborter)
{
    LOGFONT lf = log_font;
    t_size face_len = pfc::wcslen_max(lf.lfFaceName, std::size(lf.lfFaceName));

    if (face_len < std::size(lf.lfFaceName)) {
        memset(lf.lfFaceName + face_len, 0, sizeof(WCHAR) * (std::size(lf.lfFaceName) - face_len));
    }

    stream->write_lendian_t(lf.lfHeight, aborter);
    stream->write_lendian_t(lf.lfWidth, aborter);
    stream->write_lendian_t(lf.lfEscapement, aborter);
    stream->write_lendian_t(lf.lfOrientation, aborter);
    stream->write_lendian_t(lf.lfWeight, aborter);

    stream->write_lendian_t(lf.lfItalic, aborter);
    stream->write_lendian_t(lf.lfUnderline, aborter);
    stream->write_lendian_t(lf.lfStrikeOut, aborter);
    stream->write_lendian_t(lf.lfCharSet, aborter);
    stream->write_lendian_t(lf.lfOutPrecision, aborter);
    stream->write_lendian_t(lf.lfClipPrecision, aborter);
    stream->write_lendian_t(lf.lfQuality, aborter);
    stream->write_lendian_t(lf.lfPitchAndFamily, aborter);

    stream->write(&lf.lfFaceName, sizeof(lf.lfFaceName), aborter);
}

std::optional<FontDescription> select_font(HWND wnd_parent, LOGFONT initial_font)
{
    if (!ModalDialogPrologue())
        return {};

    // Note: Using a hook to get the window handle of the font picker dialog
    // requires supplying a custom dialog template from font.dlg to avoid an
    // older dialog template being used. Given the hassle involved in that,
    // we pass the parent window to modal_dialog_scope instead.

    modal_dialog_scope scope(wnd_parent);

    LOGFONT lf{initial_font};
    CHOOSEFONT cf{};
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = wnd_parent;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT;
    cf.nFontType = SCREEN_FONTTYPE;

    if (ChooseFont(&cf))
        return {{lf, cf.iPointSize}};

    return {};
}

int CALLBACK FontSizesProc(const LOGFONT* log_font, const TEXTMETRIC* ptm, DWORD FontType, LPARAM lp);

struct fontsizeinfo {
    bool up;
    int size;
    int new_size;
    int caps;
    bool changed;
};

void get_next_font_size_step(LOGFONT& log_font, bool up)
{
    LOGFONT new_log_font = log_font;

    HDC dc = GetDC(nullptr);

    fontsizeinfo size{};

    size.up = up;
    size.caps = GetDeviceCaps(dc, LOGPIXELSY);
    size.size = -MulDiv(new_log_font.lfHeight, 72, size.caps);
    size.new_size = up ? size.size : 0;
    size.changed = false;

    EnumFontFamiliesEx(dc, &new_log_font, FontSizesProc, reinterpret_cast<LPARAM>(&size), 0);
    if (size.changed)
        new_log_font.lfHeight = -MulDiv(size.new_size, size.caps, 72);

    ReleaseDC(nullptr, dc);

    if (new_log_font.lfHeight)
        log_font = new_log_font;
}

int CALLBACK FontSizesProc(const LOGFONT* log_font, const TEXTMETRIC* ptm, DWORD FontType, LPARAM lp)
{
    auto* font_size_info = reinterpret_cast<fontsizeinfo*>(lp);

    if (FontType != TRUETYPE_FONTTYPE) {
        const int point_size = abs(MulDiv(log_font->lfHeight, 72, font_size_info->caps));

        if (font_size_info->up) {
            if (point_size > font_size_info->size
                && (!(font_size_info->new_size > font_size_info->size) || point_size < font_size_info->new_size)) {
                font_size_info->new_size = point_size;
                font_size_info->changed = true;
            }
        } else {
            if (point_size < font_size_info->size && point_size > font_size_info->new_size) {
                font_size_info->new_size = point_size;
                font_size_info->changed = true;
            }
        }
        return 1;
    }

    if (font_size_info->size > 1 && !font_size_info->up)
        font_size_info->new_size = font_size_info->size - 1;
    else if (font_size_info->size < MAXLONG && font_size_info->up)
        font_size_info->new_size = font_size_info->size + 1;

    font_size_info->changed = true;
    return 0;
}

} // namespace cui::fonts
