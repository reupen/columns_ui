#pragma once

namespace cui::fonts {

struct rendering_options {
    DWRITE_RENDERING_MODE rendering_mode{DWRITE_RENDERING_MODE_DEFAULT};
    bool force_greyscale_antialiasing{};
};

/**
 * Part of the preliminary DirectWrite-friendly manager_v3 interface.
 *
 * Subject to change, only currently for internal use.
 */
class NOVTABLE font : public service_base {
public:
    [[nodiscard]] virtual const wchar_t* family_name() noexcept = 0;
    [[nodiscard]] virtual DWRITE_FONT_WEIGHT weight() noexcept = 0;
    [[nodiscard]] virtual DWRITE_FONT_STRETCH stretch() noexcept = 0;
    [[nodiscard]] virtual DWRITE_FONT_STYLE style() noexcept = 0;
    [[nodiscard]] virtual float size() noexcept = 0;

    [[nodiscard]] virtual LOGFONT log_font() noexcept = 0;

    [[nodiscard]] virtual pfc::com_ptr_t<IDWriteTextFormat> create_text_format(
        const wchar_t* locale_name = L"") noexcept
        = 0;

    [[nodiscard]] virtual DWRITE_RENDERING_MODE rendering_mode() noexcept = 0;
    [[nodiscard]] virtual bool force_greyscale_antialiasing() noexcept = 0;

    rendering_options get_rendering_options() { return {rendering_mode(), force_greyscale_antialiasing()}; }

#ifdef __WIL_COM_INCLUDED
    wil::com_ptr_t<IDWriteTextFormat> create_wil_text_format(const wchar_t* locale_name = L"")
    {
        wil::com_ptr_t<IDWriteTextFormat> text_format;
        text_format.attach(create_text_format(locale_name).detach());
        return text_format;
    }
#endif

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(font);
};

/**
 * Preliminary DirectWrite-friendly manager_v3 interface.
 *
 * Subject to change, only currently for internal use.
 */
class NOVTABLE manager_v3 : public service_base {
public:
    [[nodiscard]] virtual font::ptr get_client_font(GUID id) const = 0;
    virtual void set_client_font_size(GUID id, float size) = 0;

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager_v3);
};

} // namespace cui::fonts
