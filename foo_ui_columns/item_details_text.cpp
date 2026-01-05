#include "pch.h"

#include "item_details.h"
#include "item_details_text.h"

#include "font_utils.h"

namespace cui::panels::item_details {

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
    const auto font = api->get_font(g_guid_item_details_font_client);
    const auto text_format = fonts::get_text_format(context, font, false);

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

} // namespace cui::panels::item_details
