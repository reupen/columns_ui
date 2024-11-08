#include "pch.h"

#include "font_manager_v3.h"
#include "item_details.h"
#include "item_details_text.h"

#include "font_utils.h"

namespace cui::panels::item_details {

namespace {

bool are_strings_equal(std::wstring_view left, std::wstring_view right)
{
    return CompareStringEx(LOCALE_NAME_INVARIANT, NORM_IGNORECASE, left.data(), gsl::narrow<int>(left.length()),
               right.data(), gsl::narrow<int>(right.length()), nullptr, nullptr, 0)
        == CSTR_EQUAL;
}

template <typename Char>
std::basic_string_view<Char> trim(const std::basic_string_view<Char>& value)
{
    const auto space_chars = L" \u00a0\u200b\u202f\ufeff\r\n";
    const auto start = value.find_first_not_of(space_chars);
    const auto end = value.find_last_not_of(space_chars);

    if (start > end || start == std::string_view::npos)
        return {};

    return value.substr(start, end - start + 1);
}

std::optional<float> safe_stof(const std::wstring& value)
{
    try {
        return std::stof(value);
    } catch (const std::exception&) {
        return {};
    }
}

std::optional<int> safe_stoi(const std::wstring& value)
{
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        return {};
    }
}

std::optional<StylePropertiesMap> parse_font_code_v1(
    const std::vector<std::wstring_view>& parts, const uih::direct_write::Context::Ptr& direct_write_context)
{
    if (parts.size() < 4)
        return {};

    const auto& family_part = parts[1];
    const auto& size_part = parts[2];

    LOGFONT lf{};
    wcsncpy_s(lf.lfFaceName, std::wstring(family_part).c_str(), _TRUNCATE);

    std::wstring size_str(size_part.data(), size_part.size());
    const auto size_points = safe_stof(size_str);

    if (!size_points)
        return {};

    TextDecorationType text_decoration{TextDecorationType::None};
    const auto& style_part = parts[3];
    auto style_elements = std::views::split(style_part, L';');

    for (auto style_element : style_elements) {
        const std::wstring_view style_element_view(style_element.data(), style_element.size());

        const auto equals_index = style_element_view.find(L'=');
        const auto style_name = style_element_view.substr(0, equals_index);
        const auto style_enabled = equals_index == std::wstring_view::npos
            || are_strings_equal(style_element_view.substr(equals_index + 1), L"true");

        if (!style_enabled)
            continue;

        if (are_strings_equal(style_name, L"bold")) {
            lf.lfWeight = true;
        } else if (are_strings_equal(style_name, L"italic")) {
            lf.lfItalic = true;
        } else if (are_strings_equal(style_name, L"underline")) {
            text_decoration = TextDecorationType::Underline;
        }
    }

    try {
        const auto direct_write_font = direct_write_context->create_font(lf);

        wil::com_ptr_t<IDWriteFontFamily> font_family;
        THROW_IF_FAILED(direct_write_font->GetFontFamily(&font_family));

        wil::com_ptr_t<IDWriteLocalizedStrings> localised_strings;
        THROW_IF_FAILED(font_family->GetFamilyNames(&localised_strings));

        const auto family_name = uih::direct_write::get_localised_string(localised_strings);

        const auto weight = direct_write_font->GetWeight();
        const auto stretch = direct_write_font->GetStretch();
        const auto style = direct_write_font->GetStyle();

        return StylePropertiesMap{{StylePropertyType::FontFamily, family_name},
            {StylePropertyType::FontSize, *size_points}, {StylePropertyType::FontWeight, weight},
            {StylePropertyType::FontStretch, stretch}, {StylePropertyType::FontStyle, style},
            {StylePropertyType::TextDecoration, text_decoration}};
    }
    CATCH_LOG()

    return {};
}

struct PropertyParser {
    StylePropertyType property_type;
    std::function<std::optional<StylePropertyValue>(const std::wstring_view&)> parse;
};

std::optional<StylePropertyValue> parse_font_family(const std::wstring_view& value)
{
    return std::wstring(value);
}

std::optional<StylePropertyValue> parse_font_size(const std::wstring_view& value)
{
    return safe_stof(std::wstring(value));
}

std::optional<StylePropertyValue> parse_font_weight(const std::wstring_view& value)
{
    if (const auto converted_value = safe_stoi(std::wstring(value)))
        return static_cast<DWRITE_FONT_WEIGHT>(std::clamp(*converted_value, 1, 999));

    return {};
}

std::optional<StylePropertyValue> parse_font_stretch(const std::wstring_view& value)
{
    if (const auto converted_value = safe_stoi(std::wstring(value)))
        return static_cast<DWRITE_FONT_STRETCH>(std::clamp(*converted_value, 1, 9));

    return {};
}

std::optional<StylePropertyValue> parse_font_style(const std::wstring_view& value)
{
    if (value == L"normal")
        return DWRITE_FONT_STYLE_NORMAL;

    if (value == L"italic")
        return DWRITE_FONT_STYLE_ITALIC;

    if (value == L"oblique")
        return DWRITE_FONT_STYLE_OBLIQUE;

    return {};
}

std::optional<StylePropertyValue> parse_text_decoration(const std::wstring_view& value)
{
    if (value == L"underline")
        return TextDecorationType::Underline;

    if (value == L"none")
        return TextDecorationType::None;

    return {};
}

const std::unordered_map<std::wstring_view, PropertyParser> property_parsers{
    {L"font-family"sv, {StylePropertyType::FontFamily, parse_font_family}},
    {L"font-size"sv, {StylePropertyType::FontSize, parse_font_size}},
    {L"font-weight"sv, {StylePropertyType::FontWeight, parse_font_weight}},
    {L"font-stretch"sv, {StylePropertyType::FontStretch, parse_font_stretch}},
    {L"font-style"sv, {StylePropertyType::FontStyle, parse_font_style}},
    {L"text-decoration"sv, {StylePropertyType::TextDecoration, parse_text_decoration}}};

StylePropertiesMap parse_font_code_v2(const std::vector<std::wstring_view>& parts)
{
    const auto& properties_part = parts[1];
    auto property_exprs = std::views::split(properties_part, L';');

    StylePropertiesMap properties;

    for (const auto& property_expr : property_exprs) {
        const std::wstring_view style_element_view(property_expr.data(), property_expr.size());

        const auto colon_index = style_element_view.find(L':');

        if (colon_index == std::wstring_view::npos)
            continue;

        const auto key = trim(style_element_view.substr(0, colon_index));
        const auto value = trim(style_element_view.substr(colon_index + 1));

        const auto property_parser_iter = property_parsers.find(key);

        if (property_parser_iter == std::end(property_parsers))
            continue;

        const auto& [property_type, parse] = property_parser_iter->second;

        if (value == L"initial")
            properties.insert_or_assign(property_type, InitialValue{});
        else if (auto parsed_value = parse(value))
            properties.insert_or_assign(property_type, *parsed_value);
    }

    return properties;
}

std::optional<StylePropertiesMap> parse_font_code(
    std::wstring_view text, const uih::direct_write::Context::Ptr& direct_write_context)
{
    const auto parts_split_views = std::views::split(text, L'\t') | ranges::to<std::vector>;
    const auto parts = parts_split_views
        | ranges::views::transform([](auto&& part) { return std::wstring_view(part.data(), part.size()); })
        | ranges::to<std::vector>;
    ;

    if (parts.size() < 2)
        return {};

    const auto& version = parts[0];

    if (version == L"\x1"sv)
        return parse_font_code_v1(parts, direct_write_context);

    if (version == L"\x2"sv)
        return parse_font_code_v2(parts);

    return {};
}

} // namespace

DWRITE_PARAGRAPH_ALIGNMENT get_paragraph_alignment(VerticalAlignment alignment)
{
    switch (alignment) {
    default:
        return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
    case VerticalAlignment::Centre:
        return DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
    case VerticalAlignment::Bottom:
        return DWRITE_PARAGRAPH_ALIGNMENT_FAR;
    }
}

std::optional<uih::direct_write::TextFormat> create_text_format(const uih::direct_write::Context::Ptr& context,
    uih::alignment horizontal_alignment, VerticalAlignment vertical_alignment, bool word_wrapping)
{
    const auto api = fb2k::std_api_get<cui::fonts::manager_v3>();
    const auto font = api->get_client_font(g_guid_item_details_font_client);
    const auto text_format = fonts::get_text_format(context, font);

    if (!text_format)
        return {};

    try {
        const auto paragraph_alignment = get_paragraph_alignment(vertical_alignment);
        text_format->set_text_alignment(uih::direct_write::get_text_alignment(horizontal_alignment));
        text_format->set_paragraph_alignment(paragraph_alignment);
        text_format->set_word_wrapping(word_wrapping ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
        return text_format;
    }
    CATCH_LOG()

    return {};
}

std::optional<uih::direct_write::TextLayout> create_text_layout(
    const uih::direct_write::TextFormat& text_format, int max_width, int max_height, std::wstring_view text)
{
    try {
        const auto width = gsl::narrow_cast<float>(max_width);
        const auto height = gsl::narrow_cast<float>(max_height);

        return text_format.create_text_layout(
            text, uih::direct_write::px_to_dip(width), uih::direct_write::px_to_dip(height));
    }
    CATCH_LOG();

    return {};
}

std::tuple<std::wstring, std::vector<uih::ColouredTextSegment>, std::vector<FontSegment>> process_colour_and_font_codes(
    std::wstring_view text, const uih::direct_write::Context::Ptr& direct_write_context)
{
    auto result = std::tuple<std::wstring, std::vector<uih::ColouredTextSegment>, std::vector<FontSegment>>{};
    auto& [stripped_text, coloured_segments, font_segments] = result;

    size_t offset{};
    size_t colour_segment_start{};
    size_t font_segment_start{};
    std::optional<COLORREF> cr_current;
    std::optional<StylePropertiesMap> current_font;

    while (true) {
        const size_t index = text.find_first_of(L"\3\7"sv, offset);

        const auto fragment_length = index == std::wstring_view::npos ? std::wstring_view::npos : index - offset;
        const auto fragment = text.substr(offset, fragment_length);
        const auto is_eos = index == std::wstring_view::npos;
        const auto is_colour_code = !is_eos && text[index] == L'\3';
        const auto is_font_code = !is_eos && text[index] == L'\7';

        if (!fragment.empty()) {
            stripped_text.append(fragment);

            if (cr_current && (is_eos || is_colour_code))
                coloured_segments.emplace_back(
                    *cr_current, colour_segment_start, stripped_text.length() - colour_segment_start);

            if (current_font && (is_eos || is_font_code)) {
                const auto cleaned_font = ranges::views::remove_if(*current_font, [](auto&& pair) {
                    return std::holds_alternative<InitialValue>(pair.second);
                }) | ranges::to<StylePropertiesMap>;

                font_segments.emplace_back(
                    cleaned_font, font_segment_start, stripped_text.length() - font_segment_start);
            }
        }

        if (is_eos)
            break;

        offset = text.find(is_colour_code ? L'\3' : L'\7', index + 1);

        if (offset == std::wstring_view::npos)
            break;

        const auto code_text = text.substr(index + 1, offset - index - 1);

        if (is_colour_code) {
            cr_current = uih::parse_colour_code(code_text, false);
            colour_segment_start = stripped_text.size();
        } else {
            auto new_properties = parse_font_code(code_text, direct_write_context);

            const auto last_font_segment_end = font_segments.empty()
                ? std::nullopt
                : std::make_optional(font_segments.rbegin()->start_character + font_segments.rbegin()->character_count);
            const auto continues_from_last_segment
                = current_font && (!last_font_segment_end || *last_font_segment_end == stripped_text.size());

            if (!new_properties) {
                current_font.reset();
            } else if (current_font && continues_from_last_segment) {
                new_properties->merge(*current_font);
                current_font = new_properties;
            } else {
                current_font = new_properties;
            }

            font_segment_start = stripped_text.size();
        }

        ++offset;
    }

    return result;
}

} // namespace cui::panels::item_details
