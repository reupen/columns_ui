#include "stdafx.h"
#include "item_details.h"


void g_get_text_font_data(const char * p_text, pfc::string8_fast_aggressive & p_new_text, font_change_data_list_t & p_out)
{
    p_out.prealloc(32);

    const char * ptr = p_text;

    while (*ptr)
    {
        const char * addStart = ptr;
        while (*ptr && *ptr != '\x7') ptr++;
        p_new_text.add_string(addStart, ptr - addStart);
        if (*ptr == '\x7')
        {
            ptr++;
            const char * start = ptr;

            while (*ptr && *ptr != '\x7' && *ptr != '\t') ptr++;

            bool b_tab = false;

            if ((b_tab = *ptr == '\t') || *ptr == '\x7')
            {
                font_change_data_t temp;
                t_size count = ptr - start;
                ptr++;

                if (b_tab)
                {
                    const char * pSizeStart = ptr;
                    while (*ptr && *ptr != '\x7' && *ptr != '\t') ptr++;
                    t_size sizeLen = ptr - pSizeStart;

                    temp.m_font_data.m_point = mmh::strtoul_n(pSizeStart, sizeLen);
                    if ((b_tab = *ptr == '\t') || *ptr == '\x7') ptr++;

                    if (b_tab)
                    {
                        //ptr++;
                        const char * pFormatStart = ptr;
                        while (*ptr && *ptr != '\x7') ptr++;
                        t_size formatLen = ptr - pFormatStart;
                        if (*ptr == '\x7')
                        {
                            ptr++;
                            g_parse_font_format_string(pFormatStart, formatLen, temp.m_font_data);
                        }
                    }
                }
                else if (count == 0)
                    temp.m_reset = true;

                temp.m_font_data.m_face.set_string(start, count);
                temp.m_character_index = p_new_text.get_length();//start-p_text-1;
                p_out.add_item(temp);
            }
        }
    }
}

void g_get_text_font_info(const font_change_data_list_t & p_data, font_change_info_t & p_info)
{
    t_size i, count = p_data.get_count();
    if (count)
    {
        pfc::list_t<bool> maskKeepFonts;

        //maskKeepFonts.set_count(p_info.m_fonts.get_count());
        maskKeepFonts.add_items_repeat(false, p_info.m_fonts.get_count());

        //p_info.m_fonts.set_count (count);
        p_info.m_font_changes.set_count(count);

        HDC dc = GetDC(nullptr);
        LOGFONT lf_base;
        memset(&lf_base, 0, sizeof(lf_base));

        pfc::stringcvt::string_wide_from_utf8_fast wideconv;

        for (i = 0; i < count; i++)
        {
            if (p_data[i].m_reset)
            {
                p_info.m_font_changes[i].m_font = p_info.m_default_font;
            }
            else
            {
                wideconv.convert(p_data[i].m_font_data.m_face);
                LOGFONT lf = lf_base;
                wcsncpy_s(lf.lfFaceName, wideconv.get_ptr(), _TRUNCATE);
                lf.lfHeight = -MulDiv(p_data[i].m_font_data.m_point, GetDeviceCaps(dc, LOGPIXELSY), 72);
                if (p_data[i].m_font_data.m_bold)
                    lf.lfWeight = FW_BOLD;
                if (p_data[i].m_font_data.m_underline)
                    lf.lfUnderline = TRUE;
                if (p_data[i].m_font_data.m_italic)
                    lf.lfItalic = TRUE;

                t_size index;
                if (p_info.find_font(p_data[i].m_font_data, index))
                {
                    if (index < maskKeepFonts.get_count())
                        maskKeepFonts[index] = true;
                }
                else
                {
                    font_t::ptr_t font = new font_t;
                    font->m_font = CreateFontIndirect(&lf);
                    font->m_height = uGetFontHeight(font->m_font);
                    font->m_data = p_data[i].m_font_data;
                    index = p_info.m_fonts.add_item(font);
                    //maskKeepFonts.add_item(true);
                }

                p_info.m_font_changes[i].m_font = p_info.m_fonts[index];
            }
            p_info.m_font_changes[i].m_text_index = p_data[i].m_character_index;

        }
        p_info.m_fonts.remove_mask(pfc::bit_array_not(pfc::bit_array_table(maskKeepFonts.get_ptr(), maskKeepFonts.get_count(), true)));

        ReleaseDC(nullptr, dc);
    }
}


#if 0
t_size text_get_multiline_lines(const char * text)
{
    t_size count = 0;
    const char * ptr = text;
    while (*ptr)
    {
        const char * start = ptr;
        while (*ptr && *ptr != '\r' && *ptr != '\n') ptr++;

        count++;

        if (*ptr == '\r') ptr++;
        if (*ptr == '\n') ptr++;
    }
    return count;
}
#endif

bool g_text_ptr_skip_font_change(const char * & ptr)
{
    //while (*ptr && *ptr != '\x7') ptr++;
    if (*ptr == '\x7')
    {
        ptr++;

        while (*ptr && *ptr != '\x7') ptr++;

        if (*ptr == '\x7')
            ptr++;

        return true;
    }
    return false;
}

void g_get_text_multiline_data(const char * text, pfc::string8_fast_aggressive & p_out, pfc::list_t<line_info_t, pfc::alloc_fast_aggressive> & indices)
{
    indices.remove_all();
    p_out.prealloc(strlen(text));
    indices.prealloc(256);

    const char * ptr = text;
    while (*ptr)
    {
        const char * start = ptr;
        t_size counter = 0;
        while (*ptr && *ptr != '\r' && *ptr != '\n')
        {
            if (!g_text_ptr_skip_font_change(ptr))
            {
                ptr++;
                counter++;
            }
        }

        line_info_t temp;
        temp.m_raw_bytes = counter;
        //if (*ptr)
        indices.add_item(temp/*p_out.get_length()*/);

        p_out.add_string(start, ptr - start);

        if (*ptr == '\r') ptr++;
        if (*ptr == '\n') ptr++;
    }
}

#if 0
t_size get_text_ptr_length_colour(const char * ptr, t_size ptr_length)
{
    t_size i = 0, charCount = 0;

    while (ptr[i] && i<ptr_length)
    {
        while (ptr[i] && i<ptr_length && ptr[i] != '\x3')
        {
            charCount++;
            i++;
        }
        while (i<ptr_length && ptr[i] == '\x3')
        {
            i++;

            while (ptr[i] && i<ptr_length && ptr[i] != '\x3') i++;

            if (i<ptr_length && ptr[i] == '\x3') i++;
        }
    }
    return charCount;
}
#endif

t_size g_get_text_ptr_characters_colour(const char * ptr, t_size ptr_length)
{
    t_size i = 0, charCount = 0;

    while (ptr[i] && i<ptr_length)
    {
        while (ptr[i] && i<ptr_length && ptr[i] != '\x3')
        {
            charCount++;
            unsigned cLen = uCharLength(&ptr[i]);
            if (cLen == 0)
                return charCount;
            i += cLen;
        }
        while (i<ptr_length && ptr[i] == '\x3')
        {
            i++;

            while (ptr[i] && i<ptr_length && ptr[i] != '\x3') i++;

            if (i<ptr_length && ptr[i] == '\x3') i++;
        }
    }
    return charCount;
}

t_size g_increase_text_ptr_colour(const char * ptr, t_size ptr_length, t_size count)
{
    t_size i = 0, charCount = 0;

    while (ptr[i] && i<ptr_length && charCount<count)
    {
        while (ptr[i] && i<ptr_length && charCount<count && ptr[i] != '\x3')
        {
            charCount++;
            i++;
        }
        while (i<ptr_length && ptr[i] == '\x3')
        {
            i++;

            while (ptr[i] && i<ptr_length && ptr[i] != '\x3') i++;

            if (i<ptr_length && ptr[i] == '\x3') i++;
        }
    }
    return i;
}

#if 0
t_size utf8_char_prev_len(const char * str, t_size ptr)
{
    t_size ret = 0;

    while (ptr && (str[ptr - 1] & 0xC0) == 0x80) { ptr--; ret++; }

    if (ptr) ret++;

    return ret;
}
#endif

bool text_ptr_find_break(const char * ptr, t_size length, t_size desiredPositionChar, t_size & positionChar, t_size & positionByte)
{
    t_size i = 0, charPos = 0, prevSpaceByte = pfc_infinite, prevSpaceChar = pfc_infinite;

    while (i<length/* && ptr[i] != ' '*/)
    {
        if (i< length && ptr[i] == ' ')
        {
            if (charPos >= desiredPositionChar)
            {
                positionChar = charPos;
                positionByte = i;
                return true;
            }
            prevSpaceChar = charPos;
            prevSpaceByte = i;
        }
        if (charPos >= desiredPositionChar)
        {
            if (prevSpaceByte != pfc_infinite)
            {
                positionChar = prevSpaceChar;
                positionByte = prevSpaceByte;
                return true;
            }
        }
        if (i<length && ptr[i] != '\x3')
        {
            t_size cLen = uCharLength(&ptr[i]);
            if (!cLen) break;
            i += cLen;
            charPos++;
        }
        else
        {
            while (i<length && ptr[i] == '\x3')
            {
                i++;

                while (ptr[i] && i<length && ptr[i] != '\x3') i++;

                if (i<length && ptr[i] == '\x3') i++;
            }
        }
    }


    positionChar = charPos;
    positionByte = i;
    return false;
}

#define ELLIPSIS "\xe2\x80\xa6"//"\x85"
#define ELLIPSIS_LEN 3

void g_get_multiline_text_dimensions(HDC dc, pfc::string8_fast_aggressive & text_new, const line_info_list_t & rawLines, display_line_info_list_t & displayLines, const font_change_info_t & p_font_data, t_size line_height, SIZE & sz, bool b_word_wrapping, t_size max_width)
{
    displayLines.remove_all();
    displayLines.prealloc(rawLines.get_count() * 2);
    sz.cx = 0;
    sz.cy = 0;
    t_size i, count = rawLines.get_count();
    displayLines.set_count(count);
    for (i = 0; i<count; i++)
    {
        displayLines[i].m_raw_bytes = rawLines[i].m_raw_bytes;
    }
    t_size ptr = 0;
    //if (b_word_wrapping && max_width)
    {
        t_size heightDefault = uGetTextHeight(dc);
        t_size heightCurrent = heightDefault;

        t_size fontChangesCount = p_font_data.m_font_changes.get_count();
        t_size fontPtr = 0;
        bool b_fontChanged = false;
        HFONT fnt_old = nullptr;

        pfc::string8 text = text_new;
        text_new.reset();

        uih::CharacterExtentsCalculator pGetTextExtentExPoint;
        pfc::array_t<INT, pfc::alloc_fast_aggressive> widths;

        for (i = 0; i<count; i++)
        {
            t_size thisFontChangeCount = 0;
            t_size ptrRemaining = displayLines[i].m_raw_bytes;
            t_size thisLineHeight = heightCurrent;

            {
                while ((fontPtr + thisFontChangeCount < fontChangesCount && (ptr + ptrRemaining) > p_font_data.m_font_changes[fontPtr + thisFontChangeCount].m_text_index))
                {
                    thisLineHeight = max(thisLineHeight, (heightCurrent = p_font_data.m_font_changes[fontPtr + thisFontChangeCount].m_font->m_height));
                    thisFontChangeCount++;
                }
            }

            int widthCuml = 0;
            t_size ptrStart = ptr;

            bool bWrapped = false;
            t_size textWrappedPtr = 0, ptrLengthNoColours = 0, ptrLength = 0; // no colour codes
            t_size ptrTextWidth = 0, ptrCharacterExtent = 0;

            t_size lineTotalChars = g_get_text_ptr_characters_colour(&text[ptr], ptrRemaining);
            pfc::array_t<INT, pfc::alloc_fast_aggressive> character_extents;
            character_extents.prealloc(lineTotalChars);

            while (true && thisFontChangeCount)
            {
                unsigned width = NULL;
                t_size ptrThisCount = ptrRemaining;
                if (ptr < p_font_data.m_font_changes[fontPtr].m_text_index)
                    ptrThisCount = p_font_data.m_font_changes[fontPtr].m_text_index - (ptr);
                else if (thisFontChangeCount>1)
                    ptrThisCount = p_font_data.m_font_changes[fontPtr + 1].m_text_index - (ptr);

                if ((ptr) >= p_font_data.m_font_changes[fontPtr].m_text_index)
                {
                    HFONT fnt = SelectFont(dc, p_font_data.m_font_changes[fontPtr].m_font->m_font.get());
                    if (!b_fontChanged)
                    {
                        fnt_old = fnt;
                        b_fontChanged = true;
                    }
                    fontPtr++;
                    thisFontChangeCount--;
                }

                SIZE sz2 = {0};
                t_size length_chars_no_colours = g_get_text_ptr_characters_colour(&text[ptr], ptrThisCount);
                INT max_chars = length_chars_no_colours;

                character_extents.increase_size(length_chars_no_colours);
                //widths.set_size(length_chars_no_colours);
                pGetTextExtentExPoint.run(dc, &text[ptr], ptrThisCount, ptrTextWidth > max_width ? 0 : max_width - ptrTextWidth, b_word_wrapping ? &max_chars : nullptr, character_extents.get_ptr() + ptrCharacterExtent, &sz2, nullptr, false);

                if ((unsigned)max_chars < length_chars_no_colours)
                {
                    textWrappedPtr = max_chars;
                    bWrapped = true;
                    ptrLengthNoColours = length_chars_no_colours;
                    ptrLength = ptrThisCount;
                    ptrCharacterExtent += length_chars_no_colours;
                    break;
                }

                ptr += ptrThisCount;
                ptrRemaining -= ptrThisCount;

                t_size ptrTextWidthPrev = ptrTextWidth;

                if (max_chars)
                {
                    ptrTextWidth += character_extents[ptrCharacterExtent + max_chars - 1];
                    widthCuml += character_extents[ptrCharacterExtent + max_chars - 1];
                }

                for (int k = 0; k < max_chars; k++)
                    character_extents[ptrCharacterExtent + k] += ptrTextWidthPrev;

                ptrCharacterExtent += length_chars_no_colours;
            }

            if (!bWrapped && ptrRemaining)
            {
                t_size ptrThisCount = ptrRemaining;
                SIZE sz2;
                t_size length_chars_no_colours = g_get_text_ptr_characters_colour(&text[ptr], ptrThisCount);
                INT max_chars = length_chars_no_colours;

                character_extents.increase_size(length_chars_no_colours);
                character_extents.fill_null();
                pGetTextExtentExPoint.run(dc, &text[ptr], ptrThisCount, ptrTextWidth > max_width ? 0 : max_width - ptrTextWidth, b_word_wrapping ? &max_chars : nullptr, character_extents.get_ptr() + ptrCharacterExtent, &sz2, nullptr, false);

#if 0
                {
                    pfc::stringcvt::string_wide_from_utf8 wstr(&text[ptr], ptrThisCount);
                    DRAWTEXTPARAMS dtp;
                    memset(&dtp, 0, sizeof(dtp));
                    dtp.cbSize = sizeof(dtp);
                    RECT rc = { 0 };
                    rc.right = 100;
                    rc.bottom = 1;
                    DrawTextEx(dc, const_cast<wchar_t*>(wstr.get_ptr()), wstr.length(), &rc, DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL | DT_SINGLELINE, &dtp);

                    HRESULT hr = DrawThemeText(NULL, dc, NULL, NULL, wstr.get_ptr(), wstr.length(), NULL, NULL, &rc);

                }
#endif

                t_size ptrTextWidthPrev = ptrTextWidth;
                if ((unsigned)max_chars < length_chars_no_colours)
                {
                    textWrappedPtr = max_chars;
                    bWrapped = true;
                    ptrLengthNoColours = length_chars_no_colours;
                    ptrLength = ptrThisCount;
                    //break;
                }
                else
                {
                    ptr += ptrThisCount; //FIXME
                    ptrRemaining -= ptrThisCount;
                    if (max_chars)
                    {
                        ptrTextWidth += character_extents[ptrCharacterExtent + max_chars - 1]; //well it's too late now but meh
                        widthCuml += character_extents[ptrCharacterExtent + max_chars - 1];
                    }
                }
                for (int k = 0; k < max_chars; k++)
                    character_extents[ptrCharacterExtent + k] += ptrTextWidthPrev;
                ptrCharacterExtent += length_chars_no_colours;
            }

            bool b_skipped = false;
            t_size wrapChar = 0, wrapByte = 0;
            if (bWrapped)
            {
                if (text_ptr_find_break(&text[ptrStart], ptrLength + (ptr - ptrStart), (textWrappedPtr ? textWrappedPtr - 1 : 0) + g_get_text_ptr_characters_colour(&text[ptrStart], ptr - ptrStart), wrapChar, wrapByte))
                {
                    //textWrappedPtr = (INT)wrapChar; /??
                    //wrapByte++;
                    text_new.add_string(text + ptrStart, wrapByte + 1);
                    //displayLines[i].m_raw_bytes--;
                    b_skipped = true;
                }
                else
                {
                    if (wrapByte == 0) //invalid utf-8
                        break;
#if 0
                    if (wrapByte >= ptrLength + (ptr - ptrStart)
                        && text[ptrLength + (ptr - ptrStart)]
                        && text[ptrLength + (ptr - ptrStart)] != ' '
                        )
                    {
                        if (textWrappedPtr)
                            widthCuml = character_extents[textWrappedPtr - 1]; //doesn't include clipped part... meh!
                        bWrapped = false;
                    }
#endif
                    text_new.add_string(text + ptrStart, wrapByte);
                }
            }

            if (bWrapped)
            {
                while (fontPtr
                    && p_font_data.m_font_changes[fontPtr - 1].m_text_index >= ptrStart
                    && p_font_data.m_font_changes[fontPtr - 1].m_text_index - ptrStart > wrapByte)
                    fontPtr--;

                if (fontPtr)
                {
                    SelectFont(dc, p_font_data.m_font_changes[fontPtr - 1].m_font->m_font.get());
                }

                //if (wrapByte > (ptr - ptrStart))
                //    widthCuml += uih::get_text_width_colour(dc, &text[ptr], wrapByte - (ptr - ptrStart));// widths[wrapChar-1];

                t_size widthChar = min(textWrappedPtr ? textWrappedPtr - 1 : 0, wrapChar);
                if (b_skipped && widthChar) widthChar--;
                if (widthChar < character_extents.get_size())
                    widthCuml = character_extents[widthChar];

                //max(sz.cx, uih::get_text_width_colour(dc, text + ptr, wrapByte));

                //if (b_skipped) 
                //    wrapByte++;
                t_size inc = wrapByte;//increase_text_ptr_colour (&text[ptr], indices[i], wrapChar);
                ptr = ptrStart + wrapByte;
                displayLines[i].m_raw_bytes -= inc;
                if (b_skipped)
                {
                    ptr++;
                    displayLines[i].m_raw_bytes--;
                    //inc--;
                }

                display_line_info_t temp;
                temp.m_raw_bytes = inc;
                temp.m_bytes = temp.m_raw_bytes;
                if (b_skipped)
                {
                    temp.m_raw_bytes++;
                    //temp.m_bytes--;
                }
                temp.m_width = widthCuml;
                temp.m_height = thisLineHeight + 2;
                displayLines.insert_item(temp, i);
                //text.remove_chars()
                count++;
            }
            else
            {
                displayLines[i].m_width = widthCuml;
                displayLines[i].m_height = thisLineHeight + 2;
                displayLines[i].m_bytes = displayLines[i].m_raw_bytes;
                text_new.add_string(text + ptrStart, displayLines[i].m_raw_bytes);
                ptr = ptrStart + displayLines[i].m_raw_bytes;
            }

            sz.cx = max(sz.cx, widthCuml);;
            sz.cy += thisLineHeight + 2;

        }
        if (b_fontChanged)
            SelectFont(dc, fnt_old);
    }
    /*else
    {
    for (i=0; i<count; i++)
    {
    sz.cx = max(sz.cx, uih::get_text_width_colour(dc, &text_new[ptr], displayLines[i].m_byte_count));
    ptr += displayLines[i].m_byte_count;
    }
    }*/
}

#if 0
void get_multiline_text_dimensions(HDC dc, pfc::string8_fast_aggressive & text_new, const line_info_list_t & rawLines, display_line_info_list_t & displayLines, t_size line_height, SIZE & sz, bool b_word_wrapping, t_size max_width)
{
    displayLines.remove_all();
    displayLines.prealloc(rawLines.get_count() * 2);
    sz.cx = 0;
    t_size i, count = rawLines.get_count();
    displayLines.set_count(count);
    for (i = 0; i<count; i++)
    {
        displayLines[i].m_byte_count = rawLines[i].m_byte_count;
    }
    t_size ptr = 0;
    if (b_word_wrapping && max_width)
    {
        pfc::string8 text = text_new;
        text_new.reset();
        uih::CharacterExtentsCalculator pGetTextExtentExPoint;
        pfc::array_t<INT, pfc::alloc_fast_aggressive> widths;
        for (i = 0; i<count; i++)
        {
            SIZE sz2;
            INT max_chars = NULL;
            t_size length_colours = displayLines[i].m_byte_count;
            t_size length_chars_no_colours = get_text_ptr_characters_colour(&text[ptr], displayLines[i].m_byte_count);

            widths.set_size(length_chars_no_colours);
            //widths.fill_null();

            pGetTextExtentExPoint.run(dc, &text[ptr], length_colours, max_width, &max_chars, widths.get_ptr(), &sz2, NULL, false);

            //FIXME! Chars not bytes
            if ((unsigned)max_chars < length_chars_no_colours)
            {
                bool b_skipped = false;
                t_size wrapChar = 0, wrapByte = 0;
                if (/*max_chars && */text_ptr_find_break(&text[ptr], length_colours, max_chars ? max_chars - 1 : 0, wrapChar, wrapByte))
                {
                    max_chars = (INT)wrapChar + 1;
                    text_new.add_string(text + ptr, wrapByte);
                    //text.remove_chars(ptr + max_chars2, 1);
                    displayLines[i].m_byte_count--;
                    b_skipped = true;
                }
                else
                    text_new.add_string(text + ptr, wrapByte);

                sz.cx = max(sz.cx, uih::get_text_width_colour(dc, text + ptr, wrapByte));

                t_size inc = wrapByte;//increase_text_ptr_colour (&text[ptr], indices[i], wrapChar);
                ptr += inc;
                if (b_skipped) ptr++;
                displayLines[i].m_byte_count -= inc;
                display_line_info_t temp;
                temp.m_byte_count = inc;
                displayLines.insert_item(temp, i);
                //text.remove_chars()
                count++;
            }
            else
            {
                text_new.add_string(text + ptr, displayLines[i].m_byte_count);
                sz.cx = max(sz.cx, sz2.cx);
                ptr += displayLines[i].m_byte_count;
            }

        }
    }
    else
    {
        for (i = 0; i<count; i++)
        {
            sz.cx = max(sz.cx, uih::get_text_width_colour(dc, &text_new[ptr], displayLines[i].m_byte_count));
            ptr += displayLines[i].m_byte_count;
        }
    }
    sz.cy = line_height * displayLines.get_count();
}
#endif

#if 0
void get_multiline_text_dimensions_const(HDC dc, const char * text, const line_info_list_t & newLineData, t_size line_height, SIZE & sz, bool b_word_wrapping, t_size width)
{
    pfc::string8_fast_aggressive rawText = text;
    display_line_info_list_t newLineDataWrapped;
    get_multiline_text_dimensions(dc, rawText, newLineData, newLineDataWrapped, line_height, sz, b_word_wrapping, width);
}
#endif

void g_get_multiline_text_dimensions_const(HDC dc, const char * text, const line_info_list_t & newLineData, const font_change_info_t & p_font_data, t_size line_height, SIZE & sz, bool b_word_wrapping, t_size width)
{
    pfc::string8_fast_aggressive rawText = text;
    display_line_info_list_t newLineDataWrapped;
    g_get_multiline_text_dimensions(dc, rawText, newLineData, newLineDataWrapped, p_font_data, line_height, sz, b_word_wrapping, width);
}

void g_text_out_multiline_font(HDC dc, const RECT & rc_topleft, t_size line_height, const char * text, const font_change_info_t & p_font_data, const display_line_info_list_t & newLineDataWrapped, const SIZE & sz, COLORREF cr_text, uih::alignment align, bool b_hscroll, bool word_wrapping)
{
    pfc::string8_fast_aggressive rawText = text;

    RECT rc = rc_topleft;

    t_size widthMax = rc.right>4 ? rc.right - 4 : 0;

    int newRight = rc.left + sz.cx + 4;
    if (b_hscroll && newRight > rc.right)
        rc.right = newRight;
    rc.bottom = rc.top + sz.cy;

    COLORREF cr_old = GetTextColor(dc);

    SetTextColor(dc, cr_text);

    t_size fontChangesCount = p_font_data.m_font_changes.get_count();
    t_size fontPtr = 0;

    t_size i, count = newLineDataWrapped.get_count(), start = 0;//(rc.top<0?(0-rc.top)/line_height : 0);

    RECT rc_line = rc;
    const t_size ySkip = rc.top < 0 ? 0 - rc.top : 0; //Hackish - meh

    {
        t_size yCuml = 0;
        for (i = 0; i<count; i++)
        {
            yCuml += newLineDataWrapped[i].m_height;
            if (yCuml > ySkip)
                break;
            start = i;
            if (i)
                rc_line.top += newLineDataWrapped[i - 1].m_height;
        }
    }

    const char * ptr = rawText;
    for (i = 0; i<start/*+1*/; i++)
    {
        if (i < count)
        {
            ptr += newLineDataWrapped[i].m_raw_bytes;
            while (fontPtr < fontChangesCount && gsl::narrow<t_size>(ptr - rawText) > p_font_data.m_font_changes[fontPtr].m_text_index) fontPtr++;
        }
    }
    bool b_fontChanged = false;
    HFONT fnt_old = nullptr;


    if (fontPtr)
    {
        HFONT fnt = SelectFont(dc, p_font_data.m_font_changes[fontPtr - 1].m_font->m_font.get());
        if (!b_fontChanged)
        {
            fnt_old = fnt;
            b_fontChanged = true;
        }
    }

    if (start)
    {
        if (*ptr != '\x3')
        {
            const char * ptrC = ptr;
            while (--ptrC > rawText)
            {
                if (*ptrC == '\x3')
                {
                    const char * ptrCEnd = ptrC;
                    do { ptrC--; } while (ptrC >= rawText && *ptrC != '\x3');
                    if (ptrC >= rawText &&  *ptrC == '\x3')
                    {
                        uih::text_out_colours_tab(dc, ptrC, ptrCEnd - ptrC + 1, 0, 0, &rc_line, false, cr_text, false, false, false && !b_hscroll, uih::ALIGN_LEFT, nullptr, false, false);
                    }
                    break;
                }
            }
        }
    }

    for (i = start; i<count/*+1*/; i++)
    {
        const char * ptrStart = ptr;

        if (rc_line.top > rc_topleft.bottom) break;

        t_size thisFontChangeCount = 0;
        t_size ptrRemaining = newLineDataWrapped[i].m_bytes;
        t_size thisLineHeight = newLineDataWrapped[i].m_height;

        {
            while ((fontPtr + thisFontChangeCount < fontChangesCount && (ptr - rawText.get_ptr() + ptrRemaining) >(p_font_data.m_font_changes[fontPtr + thisFontChangeCount].m_text_index)))
            {
                thisFontChangeCount++;
            }
        }

        rc_line.bottom = rc_line.top + thisLineHeight;
        rc_line.left = rc.left + min(RECT_CX(rc), 2);
        rc_line.right = rc.right - min(RECT_CX(rc), 2);

        t_size widthLine = RECT_CX(rc_line), widthLineText = newLineDataWrapped[i].m_width;

        if (widthLineText < widthLine)
        {
            if (align == uih::ALIGN_CENTRE)
                rc_line.left += (widthLine - widthLineText) / 2;
            else if (align == uih::ALIGN_RIGHT)
                rc_line.left += (widthLine - widthLineText);
        }

        while (thisFontChangeCount)
        {
            int width = NULL;
            t_size ptrThisCount = ptrRemaining;
            if (gsl::narrow<t_size>(ptr - rawText) < p_font_data.m_font_changes[fontPtr].m_text_index)
                ptrThisCount = p_font_data.m_font_changes[fontPtr].m_text_index - (ptr - rawText);
            else if (thisFontChangeCount>1)
                ptrThisCount = p_font_data.m_font_changes[fontPtr + 1].m_text_index - (ptr - rawText);

            //rc_line.top = rc_line.bottom - 4 - p_font_data.m_fonts[p_font_data.m_font_changes[fontPtr].m_font_index]->m_height;

            if (gsl::narrow<t_size>(ptr - rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index)
            {
                HFONT fnt = SelectFont(dc, p_font_data.m_font_changes[fontPtr].m_font->m_font.get());
                if (!b_fontChanged)
                {
                    fnt_old = fnt;
                    b_fontChanged = true;
                }
                fontPtr++;
                thisFontChangeCount--;
            }
            RECT rc_font = rc_line;
            int extra = RECT_CY(rc_font) - uGetTextHeight(dc);
            rc_font.bottom -= 2;//extra/4;
            BOOL ret = uih::text_out_colours_tab(dc, ptr, ptrThisCount, 0, 0, &rc_font, false, cr_text, false, false, false && !b_hscroll, uih::ALIGN_LEFT, nullptr, false, false, &width);
            rc_line.left = width; //width == position actually!!
            ptr += ptrThisCount;
            ptrRemaining -= ptrThisCount;
        }

        if (ptrRemaining)
        {
            RECT rc_font = rc_line;
            int extra = RECT_CY(rc_font) - uGetTextHeight(dc);
            rc_font.bottom -= 2;
            uih::text_out_colours_tab(dc, ptr, ptrRemaining, 0, 0, &rc_font, false, cr_text, false, false, false && !b_hscroll, uih::ALIGN_LEFT, nullptr, false, false);
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
            ptr = ptrStart + newLineDataWrapped[i].m_raw_bytes;

        rc_line.top = rc_line.bottom;
        //rc_line.left = rc.left;
    }

    SetTextColor(dc, cr_old);
    if (b_fontChanged)
        SelectFont(dc, fnt_old);
}

#if 0
void text_out_multiline(HDC dc, const RECT & rc_topleft, t_size line_height, const char * text, COLORREF cr_text, uih::alignment align, bool b_hscroll, bool word_wrapping)
{
    pfc::string8_fast_aggressive rawText;
    pfc::list_t<t_size, pfc::alloc_fast_aggressive> newLinePositions;

    text_get_multiline_data(text, rawText, newLinePositions);

    RECT rc = rc_topleft;

    SIZE sz;
    get_multiline_text_dimensions(dc, rawText, newLinePositions, line_height, sz, word_wrapping, rc.right>4 ? rc.right - 4 : 0);
    int newRight = rc.left + sz.cx + 4;
    if (b_hscroll && newRight > rc.right)
        rc.right = newRight;
    rc.bottom = rc.top + sz.cy;

    COLORREF cr_old = GetTextColor(dc);

    SetTextColor(dc, cr_text);

    t_size i, count = newLinePositions.get_count(), start = (rc.top<0 ? (0 - rc.top) / line_height : 0);
    const char * ptr = rawText;
    for (i = 0; i<start/*+1*/; i++)
    {
        if (i < count)
            ptr += newLinePositions[i];
    }
    for (i = start; i<count/*+1*/; i++)
    {
        RECT rc_line = rc;
        rc_line.top = rc.top + line_height * i;
        rc_line.bottom = rc_line.top + line_height;

        if (rc_line.top > rc_topleft.bottom) break;

        uih::text_out_colours_tab(dc, ptr, i<count ? newLinePositions[i] : strlen(ptr), 0, 2, &rc_line, false, cr_text, false, false, !b_hscroll, align, NULL, false);
        if (i < count)
            ptr += newLinePositions[i];
    }

    SetTextColor(dc, cr_old);
}
#endif

