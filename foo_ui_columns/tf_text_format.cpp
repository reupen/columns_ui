#include "pch.h"

#include "tf_text_format.h"
#include "tf_utils.h"

namespace cui::tf {

bool TextFormatTitleformatHook::process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
    titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    p_found_flag = false;

    if (m_legacy_functionality_enabled && !stricmp_utf8_ex(p_name, p_name_length, "set_font", pfc_infinite)) {
        const auto param_count = p_params->get_param_count();

        if (param_count < 2 || param_count > 3)
            return true;

        const auto face = tf::get_param(*p_params, 0);
        const auto size_points = tf::get_param(*p_params, 1);
        const auto have_flags = p_params->get_param_count() == 3;
        const auto flags = have_flags ? tf::get_param(*p_params, 2) : ""sv;

        const auto value = fmt::format("\x7\x1\uF8FF{}\uF8FF{}\uF8FF{}\x7", face, size_points, flags);
        p_out->write(titleformat_inputtypes::unknown, value.data(), value.size());
        return true;
    }

    if (!stricmp_utf8_ex(p_name, p_name_length, set_format_function_name.c_str(), pfc_infinite)) {
        const auto param_count = p_params->get_param_count();

        if (param_count != 1)
            return true;

        const auto attributes = tf::get_param(*p_params, 0);
        const auto value = fmt::format("\x7\x2\uF8FF{}\x7", attributes);
        p_out->write(titleformat_inputtypes::unknown, value.data(), value.size());
        return true;
    }

    if ((m_legacy_functionality_enabled && !stricmp_utf8_ex(p_name, p_name_length, "reset_font", pfc_infinite))
        || !stricmp_utf8_ex(p_name, p_name_length, reset_format_function_name.c_str(), pfc_infinite)) {
        switch (p_params->get_param_count()) {
        case 0: {
            p_out->write(titleformat_inputtypes::unknown,
                "\x7"
                ""
                "\x7",
                pfc_infinite);
        }
        }
        return true;
    }

    return false;
}

bool TextFormatTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    p_found_flag = false;

    if (m_legacy_functionality_enabled && !stricmp_utf8_ex(p_name, p_name_length, "default_font_face", pfc_infinite)) {
        p_out->write(titleformat_inputtypes::unknown, m_legacy_default_font_face.c_str());
        p_found_flag = true;
        return true;
    }

    if (!stricmp_utf8_ex(p_name, p_name_length, default_font_size_field_name.c_str(), pfc_infinite)) {
        p_out->write(titleformat_inputtypes::unknown, fmt::format("{:.1f}", m_default_font_size_pt).c_str());
        p_found_flag = true;
        return true;
    }
    return false;
}

bool MergingTextFormatTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    if (stricmp_utf8_ex(
            p_name, p_name_length, TextFormatTitleformatHook::default_font_size_field_name.c_str(), pfc_infinite)
        != 0) {
        p_found_flag = false;
        return false;
    }

    p_out->write(titleformat_inputtypes::unknown, fmt::format("{:.1f}", m_default_font_size_pt).c_str());
    p_found_flag = true;
    return true;
}

bool MergingTextFormatTitleformatHook::process_function(titleformat_text_out* p_out, const char* p_name,
    size_t p_name_length, titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    if (stricmp_utf8_ex(
            p_name, p_name_length, TextFormatTitleformatHook::set_format_function_name.c_str(), pfc_infinite)
        != 0)
        return false;

    const auto param_count = p_params->get_param_count();

    if (param_count != 1)
        return true;

    const auto attributes = mmh::to_utf16(get_param(*p_params, 0));

    if (const auto properties = uih::text_style::parse_format_properties(attributes)) {
        auto copy_property = [](auto&& left, auto&& right) {
            if (left)
                right = left;
        };

        copy_property(properties->font_weight, m_format_properties.font_weight);
        copy_property(properties->font_stretch, m_format_properties.font_stretch);
        copy_property(properties->font_style, m_format_properties.font_style);
        copy_property(properties->font_size, m_format_properties.font_size);
        copy_property(properties->font_family, m_format_properties.font_family);
        copy_property(properties->text_decoration, m_format_properties.text_decoration);
    }

    return true;
}

bool NullTextFormatTitleformatHook::process_function(titleformat_text_out* p_out, const char* p_name,
    size_t p_name_length, titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    p_found_flag = false;

    if (!stricmp_utf8_ex(
            p_name, p_name_length, TextFormatTitleformatHook::set_format_function_name.c_str(), pfc_infinite)) {
        return true;
    }

    if (!stricmp_utf8_ex(
            p_name, p_name_length, TextFormatTitleformatHook::reset_format_function_name.c_str(), pfc_infinite)) {
        return true;
    }

    return false;
}

bool NullTextFormatTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    p_found_flag = false;

    if (!stricmp_utf8_ex(
            p_name, p_name_length, TextFormatTitleformatHook::default_font_size_field_name.c_str(), pfc_infinite)) {
        p_out->write(titleformat_inputtypes::unknown, "0.0");
        p_found_flag = true;
        return true;
    }

    return false;
}

} // namespace cui::tf
