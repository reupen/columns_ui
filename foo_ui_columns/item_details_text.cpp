#include "stdafx.h"
#include "item_details.h"

std::wstring g_get_text_font_data(const wchar_t* p_text, font_change_data_list_t& p_out)
{
    std::wstring p_new_text;

    const wchar_t* ptr = p_text;

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
                FontChangeData temp;
                t_size count = ptr - start;
                ptr++;

                if (b_tab) {
                    const wchar_t* pSizeStart = ptr;
                    while (*ptr && *ptr != '\x7' && *ptr != '\t')
                        ptr++;
                    t_size sizeLen = ptr - pSizeStart;

                    temp.m_font_data.m_point = mmh::strtoul_n(pSizeStart, sizeLen);
                    if ((b_tab = *ptr == '\t') || *ptr == '\x7')
                        ptr++;

                    if (b_tab) {
                        // ptr++;
                        const wchar_t* pFormatStart = ptr;
                        while (*ptr && *ptr != '\x7')
                            ptr++;
                        t_size formatLen = ptr - pFormatStart;
                        if (*ptr == '\x7') {
                            ptr++;
                            g_parse_font_format_string(pFormatStart, formatLen, temp.m_font_data);
                        }
                    }
                } else if (count == 0)
                    temp.m_reset = true;

                temp.m_font_data.m_face = std::wstring_view(start, count);
                temp.m_character_index = p_new_text.length(); // start-p_text-1;
                p_out.add_item(temp);
            }
        }
    }
    return p_new_text;
}

void g_get_text_font_info(const font_change_data_list_t& p_data, FontChangeNotify& p_info)
{
    t_size count = p_data.get_count();
    if (count) {
        pfc::list_t<bool> maskKeepFonts;

        // maskKeepFonts.set_count(p_info.m_fonts.get_count());
        maskKeepFonts.add_items_repeat(false, p_info.m_fonts.get_count());

        // p_info.m_fonts.set_count (count);
        p_info.m_font_changes.resize(count);

        HDC dc = GetDC(nullptr);
        LOGFONT lf_base;
        memset(&lf_base, 0, sizeof(lf_base));

        for (t_size i = 0; i < count; i++) {
            if (p_data[i].m_reset) {
                p_info.m_font_changes[i].m_font = p_info.m_default_font;
            } else {
                LOGFONT lf = lf_base;
                wcsncpy_s(lf.lfFaceName, p_data[i].m_font_data.m_face.c_str(), _TRUNCATE);
                lf.lfHeight = -MulDiv(p_data[i].m_font_data.m_point, GetDeviceCaps(dc, LOGPIXELSY), 72);
                if (p_data[i].m_font_data.m_bold)
                    lf.lfWeight = FW_BOLD;
                if (p_data[i].m_font_data.m_underline)
                    lf.lfUnderline = TRUE;
                if (p_data[i].m_font_data.m_italic)
                    lf.lfItalic = TRUE;

                t_size index;
                if (p_info.find_font(p_data[i].m_font_data, index)) {
                    if (index < maskKeepFonts.get_count())
                        maskKeepFonts[index] = true;
                } else {
                    Font::ptr_t font = std::make_shared<Font>();
                    font->m_font.reset(CreateFontIndirect(&lf));
                    font->m_height = uGetFontHeight(font->m_font.get());
                    font->m_data = p_data[i].m_font_data;
                    index = p_info.m_fonts.add_item(font);
                    // maskKeepFonts.add_item(true);
                }

                p_info.m_font_changes[i].m_font = p_info.m_fonts[index];
            }
            p_info.m_font_changes[i].m_text_index = p_data[i].m_character_index;
        }
        p_info.m_fonts.remove_mask(
            pfc::bit_array_not(pfc::bit_array_table(maskKeepFonts.get_ptr(), maskKeepFonts.get_count(), true)));

        ReleaseDC(nullptr, dc);
    }
}

bool g_text_ptr_skip_font_change(const wchar_t*& ptr)
{
    // while (*ptr && *ptr != '\x7') ptr++;
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

std::wstring g_get_text_multiline_data(const wchar_t* text, line_lengths& indices)
{
    std::wstring p_out;
    indices.clear();
    indices.reserve(256);

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

        indices.emplace_back(counter);

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
    std::wstring_view text, std::vector<std::wstring_view> lines, const FontChangeNotify& font_data)
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

DisplayInfo g_get_multiline_text_dimensions(HDC dc, std::wstring_view text, const line_lengths& line_lengths,
    const FontChangeNotify& font_data, bool b_word_wrapping, int max_width)
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
                    fragment.length() - fragment_character_pos, (std::max)(max_width - line_width, 0), b_word_wrapping,
                    true, line_width);

                if (b_word_wrapping) {
                    max_chars = gsl::narrow<size_t>(script_string.get_output_character_count());

                    if (max_chars > fragment.length()) {
                        uBugCheck();
                    }
                }

                // Note: Despite indications in its documentation otherwise, Uniscribe appears to use UTF-16
                // code units and not Unicode code points (otherwise this comparison would not work correctly).
                if (b_word_wrapping && max_chars < fragment.substr(fragment_character_pos).length()) {
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

                    display_info.line_info.push_back(
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
            display_info.line_info.push_back({length, line_width, line_height + vertical_line_padding});
            last_append_pos += length;
        }
    }

    display_info.sz.cx = ranges::accumulate(
        display_info.line_info, 0, [](auto&& val, auto&& line_info) { return (std::max)(val, line_info.m_width); });

    display_info.sz.cy = ranges::accumulate(
        display_info.line_info, 0, [](auto&& val, auto&& line_info) { return val + line_info.m_height; });

    return display_info;
}

void g_text_out_multiline_font(HDC dc, const RECT& rc_topleft, t_size line_height, const wchar_t* text,
    const FontChangeNotify& p_font_data, const display_line_info_list_t& newLineDataWrapped, const SIZE& sz,
    COLORREF cr_text, uih::alignment align, bool b_hscroll, bool word_wrapping)
{
    pfc::stringcvt::string_utf8_from_wide utf8_converter;
    std::wstring rawText = text;

    RECT rc = rc_topleft;
    const auto half_padding_size = uih::scale_dpi_value(2);
    const auto padding_size = half_padding_size * 2;

    t_size widthMax = rc.right > padding_size ? rc.right - padding_size : 0;

    int newRight = rc.left + sz.cx + padding_size;
    if (b_hscroll && newRight > rc.right)
        rc.right = newRight;
    rc.bottom = rc.top + sz.cy;

    COLORREF cr_old = GetTextColor(dc);

    SetTextColor(dc, cr_text);

    t_size fontChangesCount = p_font_data.m_font_changes.size();
    t_size fontPtr = 0;

    t_size i;
    t_size count = newLineDataWrapped.size();
    t_size start = 0; //(rc.top<0?(0-rc.top)/line_height : 0);

    RECT rc_line = rc;
    const t_size ySkip = rc.top < 0 ? 0 - rc.top : 0; // Hackish - meh

    {
        t_size yCuml = 0;
        for (i = 0; i < count; i++) {
            yCuml += newLineDataWrapped[i].m_height;
            if (yCuml > ySkip)
                break;
            start = i;
            if (i)
                rc_line.top += newLineDataWrapped[i - 1].m_height;
        }
    }

    const wchar_t* ptr = rawText.data();
    for (i = 0; i < start /*+1*/; i++) {
        if (i < count) {
            ptr += newLineDataWrapped[i].m_length;
            while (fontPtr < fontChangesCount
                && gsl::narrow<t_size>(ptr - rawText.data()) > p_font_data.m_font_changes[fontPtr].m_text_index)
                fontPtr++;
        }
    }
    bool b_fontChanged = false;
    HFONT fnt_old = nullptr;

    if (fontPtr) {
        HFONT fnt = SelectFont(dc, p_font_data.m_font_changes[fontPtr - 1].m_font->m_font.get());
        if (!b_fontChanged) {
            fnt_old = fnt;
            b_fontChanged = true;
        }
    }

    if (start) {
        if (*ptr != '\x3') {
            const wchar_t* ptrC = ptr;
            while (--ptrC > rawText) {
                if (*ptrC == '\x3') {
                    const wchar_t* ptrCEnd = ptrC;
                    do {
                        ptrC--;
                    } while (ptrC >= rawText && *ptrC != '\x3');
                    if (ptrC >= rawText && *ptrC == '\x3') {
                        utf8_converter.convert(ptrC, ptrCEnd - ptrC + 1);
                        uih::text_out_colours_tab(dc, utf8_converter, pfc_infinite, 0, 0, &rc_line, false, cr_text,
                            false, false && !b_hscroll, uih::ALIGN_LEFT, nullptr, false, false);
                    }
                    break;
                }
            }
        }
    }

    for (i = start; i < count /*+1*/; i++) {
        const wchar_t* ptrStart = ptr;

        if (rc_line.top > rc_topleft.bottom)
            break;

        t_size thisFontChangeCount = 0;
        t_size ptrRemaining = newLineDataWrapped[i].m_length;
        t_size thisLineHeight = newLineDataWrapped[i].m_height;

        {
            while ((fontPtr + thisFontChangeCount < fontChangesCount
                && (ptr - rawText.data() + ptrRemaining)
                    > (p_font_data.m_font_changes[fontPtr + thisFontChangeCount].m_text_index))) {
                thisFontChangeCount++;
            }
        }

        rc_line.bottom = rc_line.top + thisLineHeight;
        rc_line.left = rc.left + min(RECT_CX(rc), half_padding_size);
        rc_line.right = rc.right - min(RECT_CX(rc), half_padding_size);

        t_size widthLine = RECT_CX(rc_line);
        t_size widthLineText = newLineDataWrapped[i].m_width;

        if (widthLineText < widthLine) {
            if (align == uih::ALIGN_CENTRE)
                rc_line.left += (widthLine - widthLineText) / 2;
            else if (align == uih::ALIGN_RIGHT)
                rc_line.left += (widthLine - widthLineText);
        }

        auto left_padding = rc_line.left;

        while (thisFontChangeCount) {
            int width = NULL;
            t_size ptrThisCount = ptrRemaining;
            if (gsl::narrow<t_size>(ptr - rawText.data()) < p_font_data.m_font_changes[fontPtr].m_text_index)
                ptrThisCount = p_font_data.m_font_changes[fontPtr].m_text_index - (ptr - rawText.data());
            else if (thisFontChangeCount > 1)
                ptrThisCount = p_font_data.m_font_changes[fontPtr + 1].m_text_index - (ptr - rawText.data());

            // rc_line.top = rc_line.bottom - 4 -
            // p_font_data.m_fonts[p_font_data.m_font_changes[fontPtr].m_font_index]->m_height;

            if (gsl::narrow<t_size>(ptr - rawText.data()) >= p_font_data.m_font_changes[fontPtr].m_text_index) {
                HFONT fnt = SelectFont(dc, p_font_data.m_font_changes[fontPtr].m_font->m_font.get());
                if (!b_fontChanged) {
                    fnt_old = fnt;
                    b_fontChanged = true;
                }
                fontPtr++;
                thisFontChangeCount--;
            }
            RECT rc_font = rc_line;
            rc_font.bottom -= half_padding_size;

            utf8_converter.convert(ptr, ptrThisCount);
            BOOL ret
                = uih::text_out_colours_tab(dc, utf8_converter, pfc_infinite, 0, 0, &rc_font, false, cr_text, false,
                    false && !b_hscroll, uih::ALIGN_LEFT, nullptr, false, false, &width, rc_line.left - left_padding);
            rc_line.left = width; // width == position actually!!
            ptr += ptrThisCount;
            ptrRemaining -= ptrThisCount;
        }

        if (ptrRemaining) {
            RECT rc_font = rc_line;
            rc_font.bottom -= half_padding_size;

            utf8_converter.convert(ptr, ptrRemaining);
            uih::text_out_colours_tab(dc, utf8_converter, pfc_infinite, 0, 0, &rc_font, false, cr_text, false,
                false && !b_hscroll, uih::ALIGN_LEFT, nullptr, false, false, nullptr, rc_line.left - left_padding);
        }

#if 0
        if (fontPtr < fontChangesCount && (ptr - rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index)
        {
            do
            {
                unsigned width = NULL;
                t_size ptrThisCount = ptrRemaining;
                if (fontPtr + 1 < fontChangesCount)
                    ptrThisCount = min(p_font_data.m_font_changes[fontPtr + 1].m_text_index - (ptr - rawText), ptrThisCount);
                if (fontPtr < fontChangesCount && (ptr - rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index)
                {
                    HFONT fnt = SelectFont(dc, p_font_data.m_fonts[p_font_data.m_font_changes[fontPtr].m_font_index]->m_font);
                    if (!b_fontChanged)
                    {
                        fnt_old = fnt;
                        b_fontChanged = true;
                    }
                }
                uih::text_out_colours_tab(dc, ptr, ptrThisCount, 0, 2, &rc_line, false, cr_text, false, false, !b_hscroll, align, &width, false);
                rc_line.left += width;
                if (fontPtr < fontChangesCount && (ptr - rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index)
                    fontPtr++;
                ptr += ptrThisCount;
                ptrRemaining -= ptrThisCount;
            } while (ptrRemaining || (fontPtr < fontChangesCount && (ptr - rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index));

        }
        else
            uih::text_out_colours_tab(dc, ptr, newLinePositions[i], 0, 2, &rc_line, false, cr_text, false, false, !b_hscroll, align, NULL, false);
#endif

        if (i < count)
            ptr = ptrStart + newLineDataWrapped[i].m_length;

        rc_line.top = rc_line.bottom;
        // rc_line.left = rc.left;
    }

    SetTextColor(dc, cr_old);
    if (b_fontChanged)
        SelectFont(dc, fnt_old);
}
