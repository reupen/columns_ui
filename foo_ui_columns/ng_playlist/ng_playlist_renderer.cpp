#include "pch.h"

#include "ng_playlist.h"

namespace cui::panels::playlist_view {

void PlaylistViewRenderer::render_begin(uih::lv::RendererContext context)
{
    const auto monitor = MonitorFromWindow(context.wnd, MONITOR_DEFAULTTONEAREST);

    if (monitor != m_monitor)
        m_playlist_view->flush_artwork_images();

    m_monitor = monitor;
}

void PlaylistViewRenderer::render_group_info(uih::lv::RendererContext context, size_t index, RECT rc)
{
    const auto bitmap = m_playlist_view->request_group_artwork(index, m_monitor);

    if (!bitmap)
        return;

    wil::unique_hdc bitmap_dc(CreateCompatibleDC(context.dc));
    BITMAP bitmap_info{};
    GetObject(bitmap.get(), sizeof(BITMAP), &bitmap_info);

    RECT rc_bitmap{};
    rc_bitmap.left = rc.left + (wil::rect_width(rc) - bitmap_info.bmWidth) / 2;
    rc_bitmap.top = rc.top;
    rc_bitmap.right = rc_bitmap.left + std::min(bitmap_info.bmWidth, wil::rect_width(rc));
    rc_bitmap.bottom = rc_bitmap.top + std::min(bitmap_info.bmHeight, wil::rect_height(rc));

    auto _ = wil::SelectObject(bitmap_dc.get(), bitmap.get());

    BLENDFUNCTION blend_function{};
    blend_function.BlendOp = AC_SRC_OVER;
    blend_function.BlendFlags = 0;
    blend_function.SourceConstantAlpha = 255;
    blend_function.AlphaFormat = AC_SRC_ALPHA;

    GdiAlphaBlend(context.dc, rc_bitmap.left, rc_bitmap.top, wil::rect_width(rc_bitmap), wil::rect_height(rc_bitmap),
        bitmap_dc.get(), 0, 0, wil::rect_width(rc_bitmap), wil::rect_height(rc_bitmap), blend_function);
}

void PlaylistViewRenderer::render_item(uih::lv::RendererContext context, size_t index,
    std::vector<uih::lv::RendererSubItem> sub_items, int indentation, bool b_selected, bool b_window_focused,
    bool b_highlight, bool should_hide_focus, bool b_focused, RECT rc)
{
    colours::helper p_helper(ColoursClient::id);

    const auto calculated_use_highlight = b_highlight && !context.high_contrast_active;

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
        if (context.use_dark_mode)
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

        if (context.item_text_format && context.bitmap_render_target) {
            text_out_columns_and_colours(*context.item_text_format, context.wnd, context.dc, sub_item.text,
                1_spx + (column_index == 0 ? indentation : 0), 3_spx, rc_subitem, cr_text,
                {.bitmap_render_target = context.bitmap_render_target,
                    .is_selected = b_selected,
                    .align = sub_item.alignment,
                    .enable_ellipses = cfg_ellipsis != 0});
        }

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
    if (!(context.group_text_format && context.bitmap_render_target))
        return;

    colours::helper p_helper(ColoursClient::id);
    bool b_theme_enabled = p_helper.get_themed();

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

    const auto x_offset = 1_spx + indentation * gsl::narrow<int>(level);
    const auto border = 3_spx;

    const auto text_width
        = text_out_columns_and_colours(*context.group_text_format, context.wnd, context.dc, text, x_offset, border, rc,
            cr, {.bitmap_render_target = context.bitmap_render_target, .enable_ellipses = cfg_ellipsis != 0});

    const auto line_height = 1_spx;
    const auto line_top = rc.top + wil::rect_height(rc) / 2 - line_height / 2;
    RECT rc_line = {
        rc.left + x_offset + border * 2 + text_width + 3_spx,
        line_top,
        rc.right - 4_spx,
        line_top + line_height,
    };

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
