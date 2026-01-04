#pragma once

namespace cui::tf {

class TextFormatTitleformatHook : public titleformat_hook {
public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag) override;

    bool process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;

    TextFormatTitleformatHook(
        float font_size_pt, std::string_view legacy_default_font_face = {}, bool legacy_functionality_enabled = false)
        : m_default_font_size_pt(font_size_pt)
        , m_legacy_default_font_face(legacy_default_font_face)
        , m_legacy_functionality_enabled(legacy_functionality_enabled)
    {
    }

private:
    float m_default_font_size_pt{};
    std::string m_legacy_default_font_face;
    bool m_legacy_functionality_enabled{};
};

} // namespace cui::tf
