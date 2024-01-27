#include "pch.h"
#include "ng_playlist.h"

namespace cui::panels::playlist_view {

void PlaylistViewRenderer::render_group_info(uih::lv::RendererContext context, size_t index, RECT rc)
{
    const auto bm = m_playlist_view->request_group_artwork(index);

    if (!bm)
        return;

    RECT deflated_rect{rc};
    InflateRect(&deflated_rect, -m_playlist_view->get_artwork_left_right_padding(), 0);

    HDC dcc = CreateCompatibleDC(context.dc);
    BITMAP bminfo{};
    GetObject(bm.get(), sizeof(BITMAP), &bminfo);

    RECT rc_bitmap;
    rc_bitmap.left = deflated_rect.left + (wil::rect_width(deflated_rect) - bminfo.bmWidth) / 2;
    rc_bitmap.top = deflated_rect.top;
    rc_bitmap.right = rc_bitmap.left + std::min(bminfo.bmWidth, wil::rect_width(deflated_rect));
    rc_bitmap.bottom = rc_bitmap.top + std::min(bminfo.bmHeight, wil::rect_height(deflated_rect));

    HBITMAP bm_old = SelectBitmap(dcc, bm.get());
    BitBlt(context.dc, rc_bitmap.left, rc_bitmap.top, wil::rect_width(rc_bitmap), wil::rect_height(rc_bitmap), dcc, 0,
        0, SRCCOPY);
    SelectBitmap(dcc, bm_old);
    DeleteDC(dcc);
}

void PlaylistViewRenderer::render_item(uih::lv::RendererContext context, size_t index,
    std::vector<uih::lv::RendererSubItem> sub_items, int indentation, bool b_selected, bool b_window_focused,
    bool b_highlight, bool should_hide_focus, bool b_focused, RECT rc)
{
    colours::helper p_helper(ColoursClient::g_guid);

    const auto calculated_use_highlight = b_highlight && !context.m_is_high_contrast_active;

    int theme_state = NULL;

    if (b_selected) {
        theme_state = (calculated_use_highlight ? LISS_HOTSELECTED
                                                : (b_window_focused ? LISS_SELECTED : LISS_SELECTEDNOTFOCUS));
    } else if (calculated_use_highlight) {
        theme_state = LISS_HOT;
    }

    bool b_theme_enabled = p_helper.get_themed();
    // NB Third param of IsThemePartDefined "must be 0". But this works.
    bool b_themed = b_theme_enabled && context.list_view_theme
        && IsThemePartDefined(context.list_view_theme, LVP_LISTITEM, theme_state);

    const style_data_t& style_data = m_playlist_view->get_style_data(index);

    COLORREF cr_text = RGB(255, 0, 0);
    if (b_themed && theme_state) {
        if (FAILED(GetThemeColor(context.list_view_theme, LVP_LISTITEM, LISS_SELECTED, TMT_TEXTCOLOR, &cr_text)))
            cr_text = GetThemeSysColor(context.list_view_theme, b_selected ? COLOR_BTNTEXT : COLOR_WINDOWTEXT);

        if (IsThemeBackgroundPartiallyTransparent(context.list_view_theme, LVP_LISTITEM, theme_state))
            DrawThemeParentBackground(context.wnd, context.dc, &rc);

        RECT rc_background{rc};
        if (context.m_use_dark_mode)
            // This is inexplicable, but it needs to be done to get the same appearance as Windows Explorer.
            InflateRect(&rc_background, 1, 1);
        DrawThemeBackground(context.list_view_theme, context.dc, LVP_LISTITEM, theme_state, &rc_background, &rc);
    }

    RECT rc_subitem = rc;

    for (size_t column_index = 0; column_index < sub_items.size(); column_index++) {
        auto& sub_item = sub_items[column_index];
        auto& sub_style_data = style_data[column_index];

        rc_subitem.right = rc_subitem.left + sub_item.width;
        if (!(b_themed && theme_state)) {
            if (b_selected)
                cr_text = !b_window_focused ? sub_style_data->selected_text_colour_non_focus
                                            : sub_style_data->selected_text_colour;
            else
                cr_text = sub_style_data->text_colour;

            COLORREF cr_back;
            if (b_selected)
                cr_back = !b_window_focused ? sub_style_data->selected_background_colour_non_focus
                                            : sub_style_data->selected_background_colour;
            else
                cr_back = sub_style_data->background_colour;

            FillRect(context.dc, &rc_subitem, wil::unique_hbrush(CreateSolidBrush(cr_back)).get());
        }
        text_out_colours_tab(context.dc, sub_item.text.data(), sub_item.text.size(),
            uih::scale_dpi_value(1) + (column_index == 0 ? indentation : 0), uih::scale_dpi_value(3), &rc_subitem,
            b_selected, cr_text, true, cfg_ellipsis != 0, sub_item.alignment);

        const auto frame_width = uih::scale_dpi_value(1);

        if (sub_style_data->use_frame_left) {
            HPEN pen = CreatePen(PS_SOLID, frame_width, sub_style_data->frame_left);
            auto pen_old = (HPEN)SelectObject(context.dc, pen);

            MoveToEx(context.dc, rc_subitem.left, rc_subitem.top, nullptr);
            LineTo(context.dc, rc_subitem.left, rc_subitem.bottom);
            SelectObject(context.dc, pen_old);
            DeleteObject(pen);
        }
        if (sub_style_data->use_frame_top) {
            HPEN pen = CreatePen(PS_SOLID, frame_width, sub_style_data->frame_top);
            auto pen_old = (HPEN)SelectObject(context.dc, pen);

            MoveToEx(context.dc, rc_subitem.left, rc_subitem.top, nullptr);
            LineTo(context.dc, rc_subitem.right, rc_subitem.top);
            SelectObject(context.dc, pen_old);
            DeleteObject(pen);
        }
        if (sub_style_data->use_frame_right) {
            HPEN pen = CreatePen(PS_SOLID, frame_width, sub_style_data->frame_right);
            auto pen_old = (HPEN)SelectObject(context.dc, pen);

            MoveToEx(context.dc, rc_subitem.right - frame_width, rc_subitem.top, nullptr);
            LineTo(context.dc, rc_subitem.right - frame_width, rc_subitem.bottom);
            SelectObject(context.dc, pen_old);
            DeleteObject(pen);
        }
        if (sub_style_data->use_frame_bottom) {
            HPEN pen = CreatePen(PS_SOLID, frame_width, sub_style_data->frame_bottom);
            auto pen_old = (HPEN)SelectObject(context.dc, pen);

            MoveToEx(context.dc, rc_subitem.right - frame_width, rc_subitem.bottom - frame_width, nullptr);
            LineTo(context.dc, rc_subitem.left - frame_width, rc_subitem.bottom - frame_width);
            SelectObject(context.dc, pen_old);
            DeleteObject(pen);
        }
        rc_subitem.left = rc_subitem.right;
    }
    if (b_focused) {
        render_focus_rect(context, should_hide_focus, rc);
    }
}

void PlaylistViewRenderer::render_group(uih::lv::RendererContext context, size_t item_index, size_t group_index,
    std::string_view text, int indentation, size_t level, RECT rc)
{
    colours::helper p_helper(ColoursClient::g_guid);
    bool b_theme_enabled = p_helper.get_themed();

    int text_width = NULL;

    const auto* group = m_playlist_view->get_item(item_index)->get_group(group_index);
    if (!group->m_style_data.is_valid())
        m_playlist_view->notify_update_item_data(item_index);

    COLORREF cr = NULL;
    if (!(b_theme_enabled && context.list_view_theme
            && IsThemePartDefined(context.list_view_theme, LVP_GROUPHEADER, NULL)
            && SUCCEEDED(
                GetThemeColor(context.list_view_theme, LVP_GROUPHEADER, LVGH_OPEN, TMT_HEADING1TEXTCOLOR, &cr))))
        cr = group->m_style_data->text_colour;

    {
        wil::unique_hbrush br(CreateSolidBrush(group->m_style_data->background_colour));
        FillRect(context.dc, &rc, br.get());
    }

    text_out_colours_tab(context.dc, text.data(), text.size(),
        uih::scale_dpi_value(1) + indentation * gsl::narrow<int>(level), uih::scale_dpi_value(3), &rc, false, cr, true,
        cfg_ellipsis != 0, uih::ALIGN_LEFT, nullptr, true, true, &text_width);

    auto cx = static_cast<long>(std::min(text_width, MAXLONG));

    RECT rc_line = {cx + uih::scale_dpi_value(7), rc.top + wil::rect_height(rc) / 2 - uih::scale_dpi_value(1) / 2,
        rc.right - uih::scale_dpi_value(4),
        rc.top + wil::rect_height(rc) / 2 - uih::scale_dpi_value(1) / 2 + uih::scale_dpi_value(1)};

    if (rc_line.right > rc_line.left) {
        if (b_theme_enabled && context.list_view_theme
            && IsThemePartDefined(context.list_view_theme, LVP_GROUPHEADERLINE, NULL)
            && SUCCEEDED(DrawThemeBackground(
                context.list_view_theme, context.dc, LVP_GROUPHEADERLINE, LVGH_OPEN, &rc_line, nullptr))) {
        } else {
            COLORREF cr = NULL;
            if (!(b_theme_enabled && context.list_view_theme
                    && IsThemePartDefined(context.list_view_theme, LVP_GROUPHEADER, NULL)
                    && SUCCEEDED(GetThemeColor(
                        context.list_view_theme, LVP_GROUPHEADER, LVGH_OPEN, TMT_HEADING1TEXTCOLOR, &cr))))
                cr = group->m_style_data->text_colour;
            wil::unique_hpen pen(CreatePen(PS_SOLID, uih::scale_dpi_value(1), cr));
            HPEN pen_old = SelectPen(context.dc, pen.get());
            MoveToEx(context.dc, rc_line.left, rc_line.top, nullptr);
            LineTo(context.dc, rc_line.right, rc_line.top);
            SelectPen(context.dc, pen_old);
        }
    }
}

} // namespace cui::panels::playlist_view
