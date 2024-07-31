#pragma once

namespace cui::panels::item_details {

enum class VerticalAlignment {
    Top,
    Centre,
    Bottom,
};

DWRITE_PARAGRAPH_ALIGNMENT get_paragraph_alignment(VerticalAlignment alignment);

std::optional<uih::direct_write::TextFormat> create_text_format(const uih::direct_write::Context::Ptr& context,
    uih::alignment horizontal_alignment, VerticalAlignment vertical_alignment, bool word_wrapping);

std::optional<uih::direct_write::TextLayout> create_text_layout(
    const uih::direct_write::TextFormat& text_format, int max_width, int max_height, std::wstring_view text);

struct Font {
    std::wstring family;
    float size_points{10.0f};
    bool is_bold{};
    bool is_underline{};
    bool is_italic{};
};

struct FontSegment {
    Font font;
    size_t start_character{};
    size_t character_count{};
};

std::tuple<std::wstring, std::vector<uih::ColouredTextSegment>, std::vector<FontSegment>> process_colour_and_font_codes(
    std::wstring_view text);

} // namespace cui::panels::item_details
