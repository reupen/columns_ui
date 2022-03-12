#include "pch.h"
#include "item_details.h"
#include "config.h"

namespace cui::panels::item_details {

bool are_strings_equal(std::wstring_view left, std::wstring_view right)
{
    return CompareStringEx(LOCALE_NAME_INVARIANT, NORM_IGNORECASE, left.data(), gsl::narrow<int>(left.length()),
               right.data(), gsl::narrow<int>(right.length()), nullptr, nullptr, 0)
        == CSTR_EQUAL;
}

bool RawFont::s_are_equal(const RawFont& item1, const RawFont& item2)
{
    return are_strings_equal(item1.m_face, item2.m_face) && item1.m_point == item2.m_point
        && item1.m_bold == item2.m_bold && item1.m_italic == item2.m_italic && item1.m_underline == item2.m_underline;
}

bool operator==(const RawFont& item1, const RawFont& item2)
{
    return RawFont::s_are_equal(item1, item2);
}

void FontChanges::reset(bool keep_handles)
{
    if (!keep_handles)
        m_fonts.set_size(0);
    m_font_changes.resize(0);
}

bool FontChanges::find_font(const RawFont& raw_font, size_t& index)
{
    size_t count = m_fonts.get_count();
    for (size_t i = 0; i < count; i++) {
        if (m_fonts[i]->m_raw_font == raw_font) {
            index = i;
            return true;
        }
    }

    return false;
}

TitleformatHookChangeFont::TitleformatHookChangeFont(const LOGFONT& lf)
{
    HDC dc = GetDC(nullptr);
    m_default_font_size = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
    ReleaseDC(nullptr, dc);

    m_default_font_face = pfc::stringcvt::string_utf8_from_wide(lf.lfFaceName, std::size(lf.lfFaceName));
}

bool TitleformatHookChangeFont::process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
    titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    p_found_flag = false;
    if (!stricmp_utf8_ex(p_name, p_name_length, "set_font", pfc_infinite)) {
        switch (p_params->get_param_count()) {
        case 2:
        case 3: {
            bool b_have_flags = p_params->get_param_count() == 3;
            const char* face;
            const char* pointsize;
            const char* flags;
            size_t face_length;
            size_t pointsize_length;
            size_t flags_length;
            p_params->get_param(0, face, face_length);
            p_params->get_param(1, pointsize, pointsize_length);
            if (b_have_flags)
                p_params->get_param(2, flags, flags_length);
            pfc::string8 temp;
            temp.add_byte('\x7');
            temp.add_string(face, face_length);
            temp.add_byte('\t');
            temp.add_string(pointsize, pointsize_length);
            temp.add_byte('\t');
            if (b_have_flags)
                temp.add_string(flags, flags_length);
            temp.add_byte('\x7');
            p_out->write(titleformat_inputtypes::unknown, temp);
            p_found_flag = true;
        }
        }
        return true;
    }
    if (!stricmp_utf8_ex(p_name, p_name_length, "reset_font", pfc_infinite)) {
        switch (p_params->get_param_count()) {
        case 0: {
            p_out->write(titleformat_inputtypes::unknown,
                "\x07"
                ""
                "\x07",
                pfc_infinite);
            p_found_flag = true;
        }
        }
        return true;
    }
    return false;
}

bool TitleformatHookChangeFont::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    p_found_flag = false;
    if (!stricmp_utf8_ex(p_name, p_name_length, "default_font_face", pfc_infinite)) {
        p_out->write(titleformat_inputtypes::unknown, m_default_font_face);
        p_found_flag = true;
        return true;
    }
    if (!stricmp_utf8_ex(p_name, p_name_length, "default_font_size", pfc_infinite)) {
        p_out->write_int(titleformat_inputtypes::unknown, m_default_font_size);
        p_found_flag = true;
        return true;
    }
    return false;
}

void g_parse_font_format_string(const wchar_t* str, size_t len, RawFont& p_out)
{
    size_t ptr = 0;
    while (ptr < len) {
        size_t keyStart = ptr;
        while (ptr < len && str[ptr] != '=' && str[ptr] != ';')
            ptr++;
        size_t keyLen = ptr - keyStart;

        bool valueValid = false;
        size_t valueStart = 0;
        size_t valueLen = 0;

        if (str[ptr] == '=') {
            ptr++;
            valueStart = ptr;
            while (ptr < len && str[ptr] != ';')
                ptr++;
            valueLen = ptr - valueStart;
            ptr++;
            valueValid = true;
        } else if (ptr < len)
            ptr++;

        if (are_strings_equal(L"bold"sv, {&str[keyStart], keyLen})) {
            p_out.m_bold = (!valueValid || are_strings_equal(L"true"sv, {&str[valueStart], valueLen}));
        } else if (are_strings_equal(L"italic"sv, {&str[keyStart], keyLen})) {
            p_out.m_italic = (!valueValid || are_strings_equal(L"true"sv, {&str[valueStart], valueLen}));
        } else if (are_strings_equal(L"underline"sv, {&str[keyStart], keyLen})) {
            p_out.m_underline = (!valueValid || are_strings_equal(L"true"sv, {&str[valueStart], valueLen}));
        }
    }
}

class ItemDetailsFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_item_details_font_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item details"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { ItemDetails::g_on_font_change(); }
};

class ItemDetailsColoursClient : public colours::client {
public:
    const GUID& get_client_guid() const override { return g_guid_item_details_colour_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item details"; }

    uint32_t get_supported_colours() const override
    {
        return colours::colour_flag_background | colours::colour_flag_text;
    } // bit-mask
    uint32_t get_supported_bools() const override { return colours::bool_flag_dark_mode_enabled; } // bit-mask

    bool get_themes_supported() const override { return false; }

    void on_bool_changed(uint32_t mask) const override
    {
        if (mask & colours::bool_flag_dark_mode_enabled)
            ItemDetails::s_on_dark_mode_status_change();
    }
    void on_colour_changed(uint32_t mask) const override { ItemDetails::g_on_colours_change(); }
};

namespace {
ItemDetailsFontClient::factory<ItemDetailsFontClient> g_font_client_item_details;
ItemDetailsColoursClient::factory<ItemDetailsColoursClient> g_colour_client_item_details;
} // namespace

} // namespace cui::panels::item_details
