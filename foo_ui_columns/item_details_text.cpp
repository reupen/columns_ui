#include "pch.h"

#include "font_manager_v3.h"
#include "item_details.h"
#include "item_details_text.h"

namespace cui::panels::item_details {

namespace {

bool are_strings_equal(std::wstring_view left, std::wstring_view right)
{
    return CompareStringEx(LOCALE_NAME_INVARIANT, NORM_IGNORECASE, left.data(), gsl::narrow<int>(left.length()),
               right.data(), gsl::narrow<int>(right.length()), nullptr, nullptr, 0)
        == CSTR_EQUAL;
}

std::optional<Font> parse_font_code(std::wstring_view text)
{
    const auto parts = std::views::split(text, L'\t') | ranges::to<std::vector>;

    if (parts.size() < 2)
        return {};

    Font font;
    font.family = std::wstring_view(parts[0].data(), parts[0].size());
    try {
        std::wstring size_str(parts[1].data(), parts[1].size());
        font.size_points = std::stof(size_str);
    } catch (const std::exception&) {
        return {};
    }

    if (parts.size() > 2) {
        const std::wstring_view style(parts[2].data(), parts[2].size());
        auto style_elements = std::views::split(style, L';');

        for (auto style_element : style_elements) {
            const std::wstring_view style_element_view(style_element.data(), style_element.size());

            const auto equals_index = style_element_view.find(L'=');
            const auto style_name = style_element_view.substr(0, equals_index);
            const auto style_enabled = equals_index == std::wstring_view::npos
                || are_strings_equal(style_element_view.substr(equals_index + 1), L"true");

            if (!style_enabled)
                continue;

            if (are_strings_equal(style_name, L"bold")) {
                font.is_bold = true;
            } else if (are_strings_equal(style_name, L"italic")) {
                font.is_italic = true;
            } else if (are_strings_equal(style_name, L"underline")) {
                font.is_underline = true;
            }
        }
    }

    return font;
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
    const auto text_format = font->create_wil_text_format();

    if (!text_format)
        return {};

    try {
        const auto paragraph_alignment = get_paragraph_alignment(vertical_alignment);
        const auto wrapped_text_format = context->wrap_text_format(text_format, false);
        wrapped_text_format.set_text_alignment(uih::direct_write::get_text_alignment(horizontal_alignment));
        wrapped_text_format.set_paragraph_alignment(paragraph_alignment);
        wrapped_text_format.set_word_wrapping(word_wrapping ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
        return wrapped_text_format;
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
    std::wstring_view text)
{
    auto result = std::tuple<std::wstring, std::vector<uih::ColouredTextSegment>, std::vector<FontSegment>>{};
    auto& [stripped_text, coloured_segments, font_segments] = result;

    size_t offset{};
    size_t colour_segment_start{};
    size_t font_segment_start{};
    std::optional<COLORREF> cr_current;
    std::optional<Font> current_font;

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

            if (current_font && (is_eos || is_font_code))
                font_segments.emplace_back(
                    *current_font, font_segment_start, stripped_text.length() - font_segment_start);
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
            current_font = parse_font_code(code_text);
            font_segment_start = stripped_text.size();
        }

        ++offset;
    }

    return result;
}

} // namespace cui::panels::item_details
