#include "../stdafx.h"
#include "ng_playlist.h"

namespace pvt
{

        void ng_playlist_view_t::render_group_info(HDC dc, t_size index, t_size group_count, const RECT & rc2)
        {
            if (!m_gdiplus_initialised)
                return;
            HBITMAP bm = request_group_artwork(index, group_count);
    
            if (bm)
            {
                HDC dcc = CreateCompatibleDC(dc);
                BITMAP bminfo;
                memset(&bminfo, 0, sizeof(bminfo));
                GetObject(bm, sizeof(BITMAP), &bminfo);

                RECT rc = rc2;

                /*t_size padding=get_default_indentation_step();

                if (RECT_CX(rc)<padding)
                    rc.right=rc.left;
                else 
                    rc.right-=padding;

                if (RECT_CY(rc)<padding)
                    rc.bottom=rc.left;
                else 
                    rc.bottom-=padding;*/

                RECT rc_bitmap;
                rc_bitmap.left = rc.left + (RECT_CX(rc)-bminfo.bmWidth)/2;
                rc_bitmap.top = rc.top;
                rc_bitmap.right = rc_bitmap.left+ min(bminfo.bmWidth,RECT_CX(rc));
                rc_bitmap.bottom = rc_bitmap.top + min(bminfo.bmHeight,RECT_CY(rc));

                HBITMAP bm_old = SelectBitmap(dcc, bm);
                BitBlt(dc, rc_bitmap.left, rc_bitmap.top, RECT_CX(rc_bitmap), RECT_CY(rc_bitmap), dcc, 0, 0, SRCCOPY);
                SelectBitmap(dcc, bm_old);
                DeleteDC(dcc);
            }
        }
        void ng_playlist_view_t::render_background(HDC dc, const RECT * rc)
        {
            cui::colours::helper p_helper(appearance_client_ngpv_impl::g_guid);
            gdi_object_t<HBRUSH>::ptr_t br = CreateSolidBrush(p_helper.get_colour(cui::colours::colour_background));
            FillRect(dc, rc, br);
        }

        void ng_playlist_view_t::render_item(HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc_outter_item)
        {
            cui::colours::helper p_helper(appearance_client_ngpv_impl::g_guid);

            RECT rc_inner = *rc_outter_item;
            rc_inner.left += indentation;
            const RECT * rc = &rc_inner;

            const t_item * item = get_item(index);
            int theme_state = NULL;
            if (b_selected)
                theme_state = (b_highlight ? LISS_HOTSELECTED : (b_window_focused ? LISS_SELECTED : LISS_SELECTEDNOTFOCUS));
            else if (b_highlight)
                theme_state = LISS_HOT;

            bool b_theme_enabled = p_helper.get_themed();
            //NB Third param of IsThemePartDefined "must be 0". But this works.
            bool b_themed = b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_LISTITEM, theme_state);

            const style_data_t & style_data = get_style_data(index);

            COLORREF cr_text = NULL;
            if (b_themed && theme_state)
            {
                cr_text = GetThemeSysColor(get_theme(), b_selected ? COLOR_BTNTEXT : COLOR_WINDOWTEXT);;
                {
                    if (IsThemeBackgroundPartiallyTransparent(get_theme(), LVP_LISTITEM, theme_state))
                        DrawThemeParentBackground(get_wnd(), dc, rc);
                    DrawThemeBackground(get_theme(), dc, LVP_LISTITEM, theme_state, rc, nullptr);
                }
            }

            RECT rc_subitem = *rc_outter_item;
            t_size k, countk=get_column_count();

            for (k=0; k<countk; k++)
            {
                rc_subitem.right = rc_subitem.left + get_column_display_width(k);
                if (k==0) rc_subitem.left += indentation;

                if (!(b_themed && theme_state))
                {
                    if (b_selected)
                        cr_text = !b_window_focused ? style_data[k]->selected_text_colour_non_focus : style_data[k]->selected_text_colour;
                    else
                        cr_text = style_data[k]->text_colour;

                    COLORREF cr_back;
                    if (b_selected)
                        cr_back = !b_window_focused ? style_data[k]->selected_background_colour_non_focus : style_data[k]->selected_background_colour;
                    else
                        cr_back = style_data[k]->background_colour;

                    FillRect(dc, &rc_subitem, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(cr_back)));
                }
                uih::text_out_colours_tab(dc, get_item_text(index,k), strlen(get_item_text(index,k)), 1, 3, &rc_subitem, b_selected, cr_text, true, true, (cfg_ellipsis != 0), get_column_alignment(k));

                if (style_data[k]->use_frame_left)
                {
                    HPEN pen = CreatePen(PS_SOLID, 1, style_data[k]->frame_left);
                    HPEN pen_old = (HPEN)SelectObject(dc, pen);

                    MoveToEx(dc, rc_subitem.left, rc_subitem.top, nullptr);
                    LineTo(dc, rc_subitem.left, rc_subitem.bottom);
                    SelectObject(dc, pen_old);
                    DeleteObject(pen);
                }
                if (style_data[k]->use_frame_top)
                {
                    HPEN pen = CreatePen(PS_SOLID, 1, style_data[k]->frame_top);
                    HPEN pen_old = (HPEN)SelectObject(dc, pen);

                    MoveToEx(dc, rc_subitem.left, rc_subitem.top, nullptr);
                    LineTo(dc, rc_subitem.right, rc_subitem.top);
                    SelectObject(dc, pen_old);
                    DeleteObject(pen);
                }
                if (style_data[k]->use_frame_right)
                {
                    HPEN pen = CreatePen(PS_SOLID, 1, style_data[k]->frame_right);
                    HPEN pen_old = (HPEN)SelectObject(dc, pen);

                    MoveToEx(dc, rc_subitem.right-1, rc_subitem.top, nullptr);
                    LineTo(dc, rc_subitem.right-1, rc_subitem.bottom);
                    SelectObject(dc, pen_old);
                    DeleteObject(pen);
                }
                if (style_data[k]->use_frame_bottom)
                {
                    HPEN pen = CreatePen(PS_SOLID, 1, style_data[k]->frame_bottom);
                    HPEN pen_old = (HPEN)SelectObject(dc, pen);

                    MoveToEx(dc, rc_subitem.right-1, rc_subitem.bottom-1, nullptr);
                    LineTo(dc, rc_subitem.left-1, rc_subitem.bottom-1);
                    SelectObject(dc, pen_old);
                    DeleteObject(pen);
                }
                rc_subitem.left = rc_subitem.right;
            }
            if (b_focused)
            {
                RECT rc_focus = *rc;
                if (b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_LISTITEM, LISS_SELECTED))
                    InflateRect(&rc_focus, -1, -1);
                if (!p_helper.get_bool(cui::colours::bool_use_custom_active_item_frame))
                {
                    DrawFocusRect(dc, &rc_focus);
                }
                else
                {
                    gdi_object_t<HBRUSH>::ptr_t br = CreateSolidBrush(p_helper.get_colour(cui::colours::colour_active_item_frame));
                    FrameRect(dc, &rc_focus, br);
                }
            }
        }

        void ng_playlist_view_t::render_group(HDC dc, t_size index, t_size group, const char * text, t_size indentation, t_size level, const RECT & rc)
        {
            cui::colours::helper p_helper(appearance_client_ngpv_impl::g_guid);
            bool b_theme_enabled = p_helper.get_themed();

            int text_width = NULL;

            item_group_ng_t * item = get_item(index)->get_group(group);
            if (!item->m_style_data.is_valid())
                notify_update_item_data(index);

            COLORREF cr = NULL;
            if (!(b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_GROUPHEADER, NULL) && SUCCEEDED(GetThemeColor(get_theme(), LVP_GROUPHEADER, LVGH_OPEN, TMT_HEADING1TEXTCOLOR, &cr))))
                cr = item->m_style_data->text_colour;

            {
                gdi_object_t<HBRUSH>::ptr_t br = CreateSolidBrush(item->m_style_data->background_colour);
                FillRect(dc, &rc, br);
            }

            uih::text_out_colours_tab(dc, text, strlen(text), 2 + indentation*level, 2, &rc, false, cr, true, true, true, uih::ALIGN_LEFT, nullptr, true, true, &text_width);

            LONG cx = (LONG)min(text_width, MAXLONG);

            RECT rc_line = { cx + 7, rc.top + RECT_CY(rc) / 2, rc.right - 4, rc.top + RECT_CY(rc) / 2 + 1 };

            if (rc_line.right > rc_line.left)
            {
                if (b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_GROUPHEADERLINE, NULL) && SUCCEEDED(DrawThemeBackground(get_theme(), dc, LVP_GROUPHEADERLINE, LVGH_OPEN, &rc_line, nullptr)))
                {
                }
                else
                {
                    COLORREF cr = NULL;
                    if (!(b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_GROUPHEADER, NULL) && SUCCEEDED(GetThemeColor(get_theme(), LVP_GROUPHEADER, LVGH_OPEN, TMT_HEADING1TEXTCOLOR, &cr))))
                        cr = item->m_style_data->text_colour;
                    gdi_object_t<HPEN>::ptr_t pen = CreatePen(PS_SOLID, 1, cr);
                    HPEN pen_old = SelectPen(dc, pen);
                    MoveToEx(dc, rc_line.left, rc_line.top, nullptr);
                    LineTo(dc, rc_line.right, rc_line.top);
                    SelectPen(dc, pen_old);
                }
            }
        }

}