#pragma once

namespace cui::panels::item_details {

std::optional<uih::direct_write::TextFormat> create_text_format(const uih::direct_write::Context::Ptr& context,
    uih::alignment horizontal_alignment, uih::alignment vertical_alignment, bool word_wrapping);

std::optional<uih::direct_write::TextLayout> create_text_layout(
    const uih::direct_write::TextFormat& text_format, int max_width, int max_height, std::wstring_view text);

void render_text(const uih::direct_write::TextLayout& text_layout, HDC dc, RECT rect, COLORREF default_colour);

struct Font2 {
    std::wstring face;
    float size_points{10.0f};
    bool is_bold{};
    bool is_underline{};
    bool is_italic{};
};

struct FontSegment {
    Font2 font;
    size_t start_character{};
    size_t character_count{};
};

std::tuple<std::wstring, std::vector<uih::ColouredTextSegment>, std::vector<FontSegment>> process_colour_and_font_codes(
    std::wstring_view text);

} // namespace cui::panels::item_details
