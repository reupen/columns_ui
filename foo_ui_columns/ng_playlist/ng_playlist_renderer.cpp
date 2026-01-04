#include "pch.h"

#include "ng_playlist.h"

namespace cui::panels::playlist_view {

void PlaylistViewRenderer::render_begin(const uih::lv::RendererContext& context)
{
    const auto monitor = MonitorFromWindow(context.wnd, MONITOR_DEFAULTTONEAREST);

    if (monitor != m_monitor)
        m_playlist_view->flush_artwork_images();

    m_monitor = monitor;
}

void PlaylistViewRenderer::render_group_info(const uih::lv::RendererContext& context, size_t index, RECT rc)
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

void PlaylistViewRenderer::render_item(const uih::lv::RendererContext& context, size_t index,
    std::vector<uih::lv::RendererSubItem> sub_items, int indentation, bool is_selected, bool is_window_focused,
    bool is_highlighted, bool should_hide_focus, bool is_item_focused, RECT rc)
{
    colours::helper p_helper(ColoursClient::id);

    const auto calculated_use_highlight = is_highlighted && !context.high_contrast_active;

    int theme_state = NULL;

    if (is_selected) {
        theme_state = (calculated_use_highlight ? LISS_HOTSELECTED
                                                : (is_window_focused ? LISS_SELECTED : LISS_SELECTEDNOTFOCUS));
    } else if (calculated_use_highlight) {
        theme_state = LISS_HOT;
    }

    bool b_theme_enabled = p_helper.get_themed();
    // NB Third param of IsThemePartDefined "must be 0". But this works.
    bool b_themed = b_theme_enabled && context.list_view_theme
        && IsThemePartDefined(context.list_view_theme, LVP_LISTITEM, theme_state);

    const auto is_themed_colours = b_themed && theme_state;

    COLORREF text_colour = RGB(255, 0, 0);
    if (is_themed_colours) {
        if (FAILED(GetThemeColor(context.list_view_theme, LVP_LISTITEM, LISS_SELECTED, TMT_TEXTCOLOR, &text_colour)))
            text_colour = GetThemeSysColor(context.list_view_theme, is_selected ? COLOR_BTNTEXT : COLOR_WINDOWTEXT);

        if (IsThemeBackgroundPartiallyTransparent(context.list_view_theme, LVP_LISTITEM, theme_state))
            DrawThemeParentBackground(context.wnd, context.dc, &rc);

        RECT rc_background{rc};
        if (context.use_dark_mode)
            // This is inexplicable, but it needs to be done to get the same appearance as Windows Explorer.
            InflateRect(&rc_background, 1, 1);
        DrawThemeBackground(context.list_view_theme, context.dc, LVP_LISTITEM, theme_state, &rc_background, &rc);
    }

    const style_data_t& style_data = m_playlist_view->get_style_data(index);
    RECT rc_subitem = rc;

    bool sub_item_fill_required{};

    const auto window_background_colour = p_helper.get_colour(colours::colour_identifier_t::colour_background);

    if (!is_themed_colours) {
        auto background_colours = style_data | ranges::views::transform([&](auto cell_style) {
            return cell_style->get_background_colour(is_selected, is_window_focused);
        });

        const auto all_cells_same_background
            = ranges::adjacent_find(background_colours, ranges::not_equal_to{}) == ranges::end(background_colours);

        if (all_cells_same_background && style_data.size() > 0) {
            const auto background_colour = background_colours.front();

            if (background_colour != window_background_colour) {
                SetDCBrushColor(context.dc, background_colour);
                PatBlt(context.dc, rc.left, rc.top, wil::rect_width(rc), wil::rect_height(rc), PATCOPY);
            }
        }

        sub_item_fill_required = !all_cells_same_background;
    }

    for (size_t column_index = 0; column_index < sub_items.size(); column_index++) {
        auto& sub_item = sub_items[column_index];
        auto& sub_style_data = style_data[column_index];

        rc_subitem.right = rc_subitem.left + sub_item.width;

        auto _ = wil::scope_exit([&] { rc_subitem.left = rc_subitem.right; });

        if (!RectVisible(context.dc, &rc_subitem))
            continue;

        if (!is_themed_colours)
            text_colour = sub_style_data->get_text_colour(is_selected, is_window_focused);

        if (sub_item_fill_required) {
            const auto background_colour = sub_style_data->get_background_colour(is_selected, is_window_focused);

            if (background_colour != window_background_colour) {
                SetDCBrushColor(context.dc, background_colour);
                PatBlt(context.dc, rc_subitem.left, rc_subitem.top, wil::rect_width(rc_subitem),
                    wil::rect_height(rc_subitem), PATCOPY);
            }
        }

        if (context.item_text_format && context.bitmap_render_target) {
            text_out_columns_and_styles(*context.item_text_format, context.wnd, context.dc, sub_item.text,
                1_spx + (column_index == 0 ? indentation : 0), 3_spx, rc_subitem, text_colour,
                {.bitmap_render_target = context.bitmap_render_target,
                    .is_selected = is_selected,
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
    }
    if (is_item_focused) {
        render_focus_rect(context, should_hide_focus, rc);
    }
}

void PlaylistViewRenderer::render_group(const uih::lv::RendererContext& context, size_t item_index, size_t group_index,
    std::string_view text, int indentation, RECT rc)
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

    if (const auto background_colour = group->m_style_data->background_colour;
        background_colour != p_helper.get_colour(colours::colour_identifier_t::colour_background)) {
        SetDCBrushColor(context.dc, background_colour);
        PatBlt(context.dc, rc.left, rc.top, wil::rect_width(rc), wil::rect_height(rc), PATCOPY);
    }

    const auto x_offset = 1_spx + indentation;
    const auto border = 3_spx;

    const auto text_width
        = text_out_columns_and_styles(*context.group_text_format, context.wnd, context.dc, text, x_offset, border, rc,
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
