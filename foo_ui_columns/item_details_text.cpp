#include "stdafx.h"
#include "item_details.h"

namespace cui::panels::item_details {

std::wstring g_get_raw_font_changes(const wchar_t* formatted_text, RawFontChanges& p_out)
{
    std::wstring p_new_text;

    const wchar_t* ptr = formatted_text;

    while (*ptr) {
        const wchar_t* addStart = ptr;
        while (*ptr && *ptr != '\x7')
            ptr++;
        p_new_text.append(addStart, ptr - addStart);
        if (*ptr == '\x7') {
            ptr++;
            const wchar_t* start = ptr;

            while (*ptr && *ptr != '\x7' && *ptr != '\t')
                ptr++;

            bool b_tab = false;

            if ((b_tab = *ptr == '\t') || *ptr == '\x7') {
                RawFontChange temp;
                t_size count = ptr - start;
                ptr++;

                if (b_tab) {
                    const wchar_t* size_start = ptr;
                    while (*ptr && *ptr != '\x7' && *ptr != '\t')
                        ptr++;
                    t_size size_len = ptr - size_start;

                    temp.m_font_data.m_point = mmh::strtoul_n(size_start, size_len);
                    if ((b_tab = *ptr == '\t') || *ptr == '\x7')
                        ptr++;

                    if (b_tab) {
                        const wchar_t* format_start = ptr;
                        while (*ptr && *ptr != '\x7')
                            ptr++;
                        t_size format_len = ptr - format_start;
                        if (*ptr == '\x7') {
                            ptr++;
                            g_parse_font_format_string(format_start, format_len, temp.m_font_data);
                        }
                    }
                } else if (count == 0)
                    temp.m_reset = true;

                temp.m_font_data.m_face = std::wstring_view(start, count);
                temp.m_character_index = p_new_text.length();
                p_out.add_item(temp);
            }
        }
    }
    return p_new_text;
}

void g_get_font_changes(const RawFontChanges& raw_font_changes, FontChanges& font_changes)
{
    t_size count = raw_font_changes.get_count();
    if (count) {
        pfc::list_t<bool> fonts_to_keep_mask;

        fonts_to_keep_mask.add_items_repeat(false, font_changes.m_fonts.get_count());
        font_changes.m_font_changes.resize(count);

        HDC dc = GetDC(nullptr);
        LOGFONT lf_base;
        memset(&lf_base, 0, sizeof(lf_base));

        for (t_size i = 0; i < count; i++) {
            if (raw_font_changes[i].m_reset) {
                font_changes.m_font_changes[i].m_font = font_changes.m_default_font;
            } else {
                LOGFONT lf = lf_base;
                wcsncpy_s(lf.lfFaceName, raw_font_changes[i].m_font_data.m_face.c_str(), _TRUNCATE);
                lf.lfHeight = -MulDiv(raw_font_changes[i].m_font_data.m_point, GetDeviceCaps(dc, LOGPIXELSY), 72);
                if (raw_font_changes[i].m_font_data.m_bold)
                    lf.lfWeight = FW_BOLD;
                if (raw_font_changes[i].m_font_data.m_underline)
                    lf.lfUnderline = TRUE;
                if (raw_font_changes[i].m_font_data.m_italic)
                    lf.lfItalic = TRUE;

                t_size index;
                if (font_changes.find_font(raw_font_changes[i].m_font_data, index)) {
                    if (index < fonts_to_keep_mask.get_count())
                        fonts_to_keep_mask[index] = true;
                } else {
                    Font::Ptr font = std::make_shared<Font>();
                    font->m_font.reset(CreateFontIndirect(&lf));
                    font->m_height = uGetFontHeight(font->m_font.get());
                    font->m_raw_font = raw_font_changes[i].m_font_data;
                    index = font_changes.m_fonts.add_item(font);
                }

                font_changes.m_font_changes[i].m_font = font_changes.m_fonts[index];
            }
            font_changes.m_font_changes[i].m_text_index = raw_font_changes[i].m_character_index;
        }
        font_changes.m_fonts.remove_mask(pfc::bit_array_not(
            pfc::bit_array_table(fonts_to_keep_mask.get_ptr(), fonts_to_keep_mask.get_count(), true)));

        ReleaseDC(nullptr, dc);
    }
}

bool g_text_ptr_skip_font_change(const wchar_t*& ptr)
{
    if (*ptr == '\x7') {
        ptr++;

        while (*ptr && *ptr != '\x7')
            ptr++;

        if (*ptr == '\x7')
            ptr++;

        return true;
    }
    return false;
}

std::wstring g_get_text_line_lengths(const wchar_t* text, LineLengths& line_lengths)
{
    std::wstring p_out;
    line_lengths.clear();
    line_lengths.reserve(256);

    const wchar_t* ptr = text;
    while (*ptr) {
        const wchar_t* start = ptr;
        t_size counter = 0;
        while (*ptr && *ptr != '\r' && *ptr != '\n') {
            if (!g_text_ptr_skip_font_change(ptr)) {
                ptr++;
                counter++;
            }
        }

        line_lengths.emplace_back(counter);

        p_out.append(start, ptr - start);

        if (*ptr == '\r')
            ptr++;
        if (*ptr == '\n')
            ptr++;
    }
    return p_out;
}

std::vector<std::wstring_view> get_colour_fragments(std::wstring_view text)
{
    std::vector<std::wstring_view> fragments;

    size_t offset{};
    while (true) {
        const size_t index = text.find(L"\3"sv, offset);

        const auto fragment_length = index == std::wstring_view::npos ? std::wstring_view::npos : index - offset;
        const auto fragment = text.substr(offset, fragment_length);

        if (fragment.length() > 0)
            fragments.emplace_back(fragment);

        if (index == std::wstring_view::npos)
            break;

        offset = text.find(L"\3"sv, index + 1);

        if (offset == std::wstring_view::npos)
            break;

        ++offset;
    }

    return fragments;
}

auto split_lines_into_fragments(
    std::wstring_view text, std::vector<std::wstring_view> lines, const FontChanges& font_data)
{
    std::vector<std::tuple<std::wstring_view, std::vector<std::wstring_view>>> sub_fragments_by_line;
    auto&& font_changes = font_data.m_font_changes;
    auto font_change_iter = font_changes.begin();

    for (auto&& line : lines) {
        std::vector<std::wstring_view> sub_fragments;
        size_t line_character_pos{};

        while (line_character_pos < line.length()) {
            const size_t character_pos = line.data() + line_character_pos - text.data();

            // There could be multiple font changes in a row, so this needs to be a loop.
            while (font_change_iter != font_changes.end() && font_change_iter->m_text_index < character_pos)
                ++font_change_iter;

            size_t font_fragment_length{line.length()};

            assert(font_change_iter == font_changes.end() || font_change_iter->m_text_index >= character_pos);

            while (font_change_iter != font_changes.end()) {
                if (font_change_iter->m_text_index > character_pos) {
                    font_fragment_length = font_change_iter->m_text_index - character_pos;
                    break;
                }
                ++font_change_iter;
            }

            const auto font_fragment = line.substr(line_character_pos, font_fragment_length);
            ranges::push_back(sub_fragments, get_colour_fragments(font_fragment));

            line_character_pos += font_fragment.length();
        }

        sub_fragments_by_line.emplace_back(line, sub_fragments);
    }
    return sub_fragments_by_line;
}

auto get_text_truncate_point(std::wstring_view text)
{
    const auto all_break_chars = L" -–—\u200B"sv;
    const auto break_after_chars = L" -\u200B"sv;

    auto pos = text.find_last_not_of(break_after_chars);

    if (pos == std::wstring_view::npos)
        return text.length();

    pos = text.find_last_of(all_break_chars, pos - 1);

    if (pos == std::wstring_view::npos)
        return text.length();

    return pos + 1;
}

DisplayInfo g_get_multiline_text_dimensions(HDC dc, std::wstring_view text, const LineLengths& line_lengths,
    const FontChanges& font_data, bool word_wrapping, int max_width)
{
    auto&& font_changes = font_data.m_font_changes;
    const int vertical_line_padding = uih::scale_dpi_value(2);

    DisplayInfo display_info;
    std::vector<std::wstring_view> lines;

    std::wstring::size_type line_start{};
    for (auto&& line_length : line_lengths) {
        lines.emplace_back(text.data() + line_start, line_length);
        line_start += line_length;
    }

    // Break each line into sub-strings each of the same colour and font.
    // Note: The input string contains colour codes. They need to be accounted
    // for when calculating offsets.
    auto fragments_by_line = split_lines_into_fragments(text, lines, font_data);

    wil::unique_select_object selected_font;
    size_t last_append_pos{};
    auto font_iter = font_changes.begin();

    for (auto&& [line, fragments] : fragments_by_line) {
        int line_height{gsl::narrow<int>(uGetTextHeight(dc))};
        bool is_line_height_explicitly_set{};
        int line_width{};

        for (auto&& [fragment_index, fragment] : ranges::views::enumerate(fragments)) {
            const size_t character_pos = fragment.data() - text.data();
            size_t fragment_character_pos{};

            while (font_iter != font_changes.end() && font_iter->m_text_index <= character_pos) {
                selected_font.reset();
                selected_font = wil::SelectObject(dc, font_iter->m_font->m_font.get());
                if (fragment_index == 0 && !is_line_height_explicitly_set) {
                    line_height = font_iter->m_font->m_height;
                    is_line_height_explicitly_set = true;
                } else {
                    line_height = (std::max)(line_height, font_iter->m_font->m_height);
                    is_line_height_explicitly_set = true;
                }
                ++font_iter;
            }

            while (fragment_character_pos < fragment.length()) {
                size_t max_chars{};
                uih::UniscribeTextRenderer script_string;

                script_string.analyse(dc, fragment.data() + fragment_character_pos,
                    fragment.length() - fragment_character_pos, (std::max)(max_width - line_width, 0), word_wrapping,
                    true, line_width);

                if (word_wrapping) {
                    max_chars = gsl::narrow<size_t>(script_string.get_output_character_count());

                    if (max_chars > fragment.length()) {
                        uBugCheck();
                    }
                }

                // Note: Despite indications in its documentation otherwise, Uniscribe appears to use UTF-16
                // code units and not Unicode code points (otherwise this comparison would not work correctly).
                if (word_wrapping && max_chars < fragment.substr(fragment_character_pos).length()) {
                    std::vector<int> character_extents(fragment.length());
                    script_string.get_character_logical_extents(character_extents.data());

                    // Wrap at the end of the last word within this fragment
                    max_chars = get_text_truncate_point(fragment.substr(fragment_character_pos, max_chars));

                    // If no characters fit in the available space, put one in to avoid weirdness and
                    // the risk of an infinite loop.
                    if (max_chars == 0)
                        ++max_chars;

                    line_width += character_extents.at(max_chars - 1);

                    const auto* wrapped_line_start = text.data() + last_append_pos;
                    const auto* wrapped_line_end = fragment.data() + fragment_character_pos + max_chars;

                    const size_t wrapped_line_length = wrapped_line_end - wrapped_line_start;

                    display_info.line_sizes.push_back(
                        {wrapped_line_length, line_width, line_height + vertical_line_padding});

                    fragment_character_pos += max_chars;
                    last_append_pos = wrapped_line_end - text.data();
                    line_height = gsl::narrow<int>(uGetTextHeight(dc));
                    is_line_height_explicitly_set = false;
                    line_width = 0;
                } else {
                    fragment_character_pos += fragment.size();
                    line_width += script_string.get_output_width();
                }
            }
        }

        if (line.empty() || last_append_pos < gsl::narrow<size_t>(line.data() + line.size() - text.data())) {
            const size_t length = line.data() + line.size() - text.data() - last_append_pos;
            display_info.line_sizes.push_back({length, line_width, line_height + vertical_line_padding});
            last_append_pos += length;
        }
    }

    display_info.sz.cx = ranges::accumulate(
        display_info.line_sizes, 0, [](auto&& val, auto&& line_info) { return (std::max)(val, line_info.m_width); });

    display_info.sz.cy = ranges::accumulate(
        display_info.line_sizes, 0, [](auto&& val, auto&& line_info) { return val + line_info.m_height; });

    return display_info;
}

void g_text_out_multiline_font(HDC dc, RECT rc_placement, const wchar_t* text, const FontChanges& font_changes,
    const LineSizes& wrapped_line_sizes, COLORREF cr_text, uih::alignment align)
{
    pfc::stringcvt::string_utf8_from_wide utf8_converter;

    const auto half_padding_size = uih::scale_dpi_value(2);

    const COLORREF cr_old = GetTextColor(dc);

    SetTextColor(dc, cr_text);

    const t_size font_change_count = font_changes.m_font_changes.size();
    t_size font_index = 0;

    const t_size line_count = wrapped_line_sizes.size();
    t_size start_line = 0;

    RECT rc_line = rc_placement;
    const t_size y_start_pos = rc_placement.top < 0 ? 0 - rc_placement.top : 0;

    for (size_t line_index{0}, running_height{0}; line_index < line_count; line_index++) {
        running_height += wrapped_line_sizes[line_index].m_height;

        if (running_height > y_start_pos)
            break;

        start_line = line_index;

        if (line_index)
            rc_line.top += wrapped_line_sizes[line_index - 1].m_height;
    }

    const wchar_t* text_ptr = text;
    for (size_t line_index = 0; line_index < start_line; line_index++) {
        if (line_index < line_count) {
            text_ptr += wrapped_line_sizes[line_index].m_length;

            while (font_index < font_change_count
                && gsl::narrow<t_size>(text_ptr - text) > font_changes.m_font_changes[font_index].m_text_index)
                font_index++;
        }
    }

    bool was_font_changed = false;
    HFONT fnt_old = nullptr;

    if (font_index) {
        HFONT fnt = SelectFont(dc, font_changes.m_font_changes[font_index - 1].m_font->m_font.get());
        if (!was_font_changed) {
            fnt_old = fnt;
            was_font_changed = true;
        }
    }

    if (start_line) {
        // Back track to the last colour code and render it
        if (*text_ptr != '\x3') {
            const wchar_t* ptr = text_ptr;
            while (ptr-- >= text) {
                if (*ptr == '\x3') {
                    const wchar_t* code_end = ptr;
                    do {
                        ptr--;
                    } while (ptr >= text && *ptr != '\x3');
                    if (ptr >= text && code_end != ptr && *ptr == '\x3') {
                        utf8_converter.convert(ptr, code_end - ptr + 1);
                        uih::text_out_colours_tab(dc, utf8_converter, pfc_infinite, 0, 0, &rc_line, false, cr_text,
                            false, false, uih::ALIGN_LEFT, nullptr, false, false);
                    }
                    break;
                }
            }
        }
    }

    for (size_t line_index = start_line; line_index < line_count; line_index++) {
        const wchar_t* start_of_line = text_ptr;

        if (rc_line.top > rc_placement.bottom)
            break;

        t_size line_font_change_count = 0;
        t_size num_characters_remaining = wrapped_line_sizes[line_index].m_length;

        while ((font_index + line_font_change_count < font_change_count
            && (text_ptr - text + num_characters_remaining)
                > (font_changes.m_font_changes[font_index + line_font_change_count].m_text_index))) {
            line_font_change_count++;
        }

        const t_size line_height = wrapped_line_sizes[line_index].m_height;

        rc_line.bottom = rc_line.top + line_height;
        rc_line.left = rc_placement.left;
        rc_line.right = rc_placement.right;

        const t_size line_width = RECT_CX(rc_line);
        const t_size line_text_width = wrapped_line_sizes[line_index].m_width;

        if (line_text_width < line_width) {
            if (align == uih::ALIGN_CENTRE)
                rc_line.left += (line_width - line_text_width) / 2;
            else if (align == uih::ALIGN_RIGHT)
                rc_line.left += (line_width - line_text_width);
        }

        const auto left_padding = rc_line.left;

        while (line_font_change_count) {
            int end_x_position = NULL;
            t_size num_characters_to_render = num_characters_remaining;
            if (gsl::narrow<t_size>(text_ptr - text) < font_changes.m_font_changes[font_index].m_text_index)
                num_characters_to_render = font_changes.m_font_changes[font_index].m_text_index - (text_ptr - text);
            else if (line_font_change_count > 1)
                num_characters_to_render = font_changes.m_font_changes[font_index + 1].m_text_index - (text_ptr - text);

            if (gsl::narrow<t_size>(text_ptr - text) >= font_changes.m_font_changes[font_index].m_text_index) {
                HFONT fnt = SelectFont(dc, font_changes.m_font_changes[font_index].m_font->m_font.get());
                if (!was_font_changed) {
                    fnt_old = fnt;
                    was_font_changed = true;
                }
                font_index++;
                line_font_change_count--;
            }
            RECT rc_font = rc_line;
            rc_font.bottom -= half_padding_size;

            utf8_converter.convert(text_ptr, num_characters_to_render);
            BOOL ret = uih::text_out_colours_tab(dc, utf8_converter, pfc_infinite, 0, 0, &rc_font, false, cr_text,
                false, false, uih::ALIGN_LEFT, nullptr, false, false, &end_x_position, rc_line.left - left_padding);
            rc_line.left = end_x_position;
            text_ptr += num_characters_to_render;
            num_characters_remaining -= num_characters_to_render;
        }

        if (num_characters_remaining) {
            RECT rc_font = rc_line;
            rc_font.bottom -= half_padding_size;

            utf8_converter.convert(text_ptr, num_characters_remaining);
            uih::text_out_colours_tab(dc, utf8_converter, pfc_infinite, 0, 0, &rc_font, false, cr_text, false, false,
                uih::ALIGN_LEFT, nullptr, false, false, nullptr, rc_line.left - left_padding);
        }

        if (line_index < line_count)
            text_ptr = start_of_line + wrapped_line_sizes[line_index].m_length;

        rc_line.top = rc_line.bottom;
    }

    SetTextColor(dc, cr_old);
    if (was_font_changed)
        SelectFont(dc, fnt_old);
}

} // namespace cui::panels::item_details
