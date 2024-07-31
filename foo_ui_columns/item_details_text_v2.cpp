#include "pch.h"

#include "font_manager_v3.h"
#include "item_details.h"
#include "item_details_text_v2.h"

namespace cui::panels::item_details {

namespace {

bool are_strings_equal(std::wstring_view left, std::wstring_view right)
{
    return CompareStringEx(LOCALE_NAME_INVARIANT, NORM_IGNORECASE, left.data(), gsl::narrow<int>(left.length()),
               right.data(), gsl::narrow<int>(right.length()), nullptr, nullptr, 0)
        == CSTR_EQUAL;
}

std::optional<Font2> parse_font_code(std::wstring_view text)
{
    const auto parts = std::views::split(text, L'\t') | ranges::to<std::vector>;

    if (parts.size() < 2)
        return {};

    Font2 font;
    font.face = std::wstring_view(parts[0].data(), parts[0].size());
    try {
        std::wstring size_str(parts[1].data(), parts[1].size());
        font.size_points = std::stof(size_str);
    } catch (const std::exception&) {
        return {};
    }

    if (parts.size() > 2) {
        const std::wstring_view style(parts[2].data(), parts[2].size());
        auto style_parts = std::views::split(style, L';');

        for (auto style_part : style_parts) {
            const std::wstring_view style_part_view(style_part.data(), style_part.size());
            const auto style_type = style_part_view.substr(0, style_part_view.find('='));

            // FIXME: Check value

            if (are_strings_equal(style_type, L"bold")) {
                font.is_bold = true;
            } else if (are_strings_equal(style_type, L"italic")) {
                font.is_italic = true;
            } else if (are_strings_equal(style_type, L"underline")) {
                font.is_underline = true;
            }
        }
    }

    return font;
}

} // namespace

std::optional<uih::direct_write::TextFormat> create_text_format(const uih::direct_write::Context::Ptr& context,
    uih::alignment horizontal_alignment, uih::alignment vertical_alignment, bool word_wrapping)
{
    const auto api = fb2k::std_api_get<cui::fonts::manager_v3>();
    const auto font = api->get_client_font(g_guid_item_details_font_client);
    const auto text_format = font->create_wil_text_format();

    if (!text_format)
        return {};

    try {
        const auto paragraph_alignment = [vertical_alignment] {
            switch (vertical_alignment) {
            default:
                return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
            case uih::ALIGN_CENTRE:
                return DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
            case uih::ALIGN_RIGHT:
                return DWRITE_PARAGRAPH_ALIGNMENT_FAR;
            }
        }();
        const auto wrapped_text_format = context->wrap_text_format(text_format, false);
        wrapped_text_format.set_text_alignment(uih::direct_write::get_text_alignment(horizontal_alignment));
        wrapped_text_format.set_paragraph_alignment(paragraph_alignment);
        wrapped_text_format.set_word_wrapping(word_wrapping ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
        return wrapped_text_format;

        // TODO Vertical alignment
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

void render_text(const uih::direct_write::TextLayout& text_layout, HDC dc, RECT rect, COLORREF default_colour)
{
    try {
        text_layout.render(dc, rect, default_colour);
    }
    CATCH_LOG();
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
    std::optional<Font2> current_font;

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
