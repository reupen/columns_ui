#include "stdafx.h"
#include "splitter.h"

LRESULT splitter_window_impl::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_NCCREATE:
        m_wnd = wnd;
        g_instances.add_item(this);
        break;
    case WM_CREATE:
        if (!g_count++)
        {
            g_font_menu_horizontal = uCreateMenuFont();
            g_font_menu_vertical = uCreateMenuFont(true);
        }
        refresh_children();
        break;
    case WM_DESTROY:
        destroy_children();
        if (!--g_count)
        {
            g_font_menu_horizontal.release();
            g_font_menu_vertical.release();
        }
        break;
    case WM_NCDESTROY:
        g_instances.remove_item(this);
        m_wnd = nullptr;
        break;
    case WM_SHOWWINDOW:
        if (wp == TRUE && lp == 0)
        {
            unsigned n, count = m_panels.get_count();
            for (n = 0; n<count; n++)
            {
                ShowWindow(m_panels[n]->m_wnd_child, SW_SHOWNORMAL);
                ShowWindow(m_panels[n]->m_wnd, SW_SHOWNORMAL);
            }
            RedrawWindow(wnd, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);

        }
        break;
    case WM_WINDOWPOSCHANGED:
    {
        LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE))
        {
            on_size_changed(lpwp->cx, lpwp->cy);
        }
    }
    break;
    /*case WM_SIZE:
    on_size_changed(LOWORD(lp), HIWORD(lp));
    break;*/
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lp;

        lpmmi->ptMinTrackSize.y = 0;
        lpmmi->ptMinTrackSize.x = 0;
        lpmmi->ptMaxTrackSize.y = get_orientation() == vertical ? 0 : MAXLONG;
        lpmmi->ptMaxTrackSize.x = get_orientation() == horizontal ? 0 : MAXLONG;

        unsigned n, count = m_panels.get_count();
        bool b_found = false;

        for (n = 0; n<count; n++)
        {
            MINMAXINFO mmi;
            memset(&mmi, 0, sizeof(MINMAXINFO));
            mmi.ptMaxTrackSize.x = MAXLONG;
            mmi.ptMaxTrackSize.y = MAXLONG;

            if (m_panels[n]->m_wnd_child)
            {
                b_found = true;
                unsigned divider_size = get_panel_divider_size(n);

                unsigned caption_height = m_panels[n]->m_show_caption ? g_get_caption_size() : 0;
                if (m_panels[n]->m_hidden)
                {
                    if (get_orientation() == horizontal)
                    {
                        if (m_panels[n]->m_caption_orientation == vertical)
                        {
                            mmi.ptMinTrackSize.x = caption_height;
                            mmi.ptMaxTrackSize.x = caption_height;
                        }
                    }
                    else
                    {
                        if (m_panels[n]->m_caption_orientation == horizontal)
                        {
                            mmi.ptMinTrackSize.y = caption_height;
                            mmi.ptMaxTrackSize.y = caption_height;
                        }
                    }
                }
                else
                {
                    SendMessage(m_panels[n]->m_wnd_child, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
                    if (caption_height)
                    {
                        if (m_panels[n]->m_caption_orientation == horizontal)
                        {
                            mmi.ptMinTrackSize.y += caption_height;
                            if (mmi.ptMaxTrackSize.y < MAXLONG - (long)caption_height) mmi.ptMaxTrackSize.y += caption_height;
                            else mmi.ptMaxTrackSize.y = MAXLONG;
                        }
                        else
                        {
                            mmi.ptMinTrackSize.x += caption_height;
                            if (mmi.ptMaxTrackSize.x < MAXLONG - (long)caption_height) mmi.ptMaxTrackSize.x += caption_height;
                            else mmi.ptMaxTrackSize.x = MAXLONG;
                        }
                    }
                }

                if (m_panels[n]->m_show_toggle_area && !m_panels[n]->m_autohide)
                {
                    mmi.ptMinTrackSize.x++;
                    if (mmi.ptMaxTrackSize.x < MAXLONG)
                        mmi.ptMaxTrackSize.x++;
                }


                if (get_orientation() == vertical)
                {
                    lpmmi->ptMinTrackSize.y += mmi.ptMinTrackSize.y + divider_size;

                    lpmmi->ptMinTrackSize.x = max(mmi.ptMinTrackSize.x, lpmmi->ptMinTrackSize.x);

                    if (lpmmi->ptMaxTrackSize.y <= MAXLONG - mmi.ptMaxTrackSize.y && lpmmi->ptMaxTrackSize.y + mmi.ptMaxTrackSize.y <= MAXLONG - (long)divider_size)
                    {
                        lpmmi->ptMaxTrackSize.y += mmi.ptMaxTrackSize.y + divider_size;
                    }
                    else
                    {
                        lpmmi->ptMaxTrackSize.y = MAXLONG;
                    }
                    lpmmi->ptMaxTrackSize.x = min(mmi.ptMaxTrackSize.x, lpmmi->ptMaxTrackSize.x);
                }
                else
                {
                    lpmmi->ptMinTrackSize.x += mmi.ptMinTrackSize.x + divider_size;
                    lpmmi->ptMinTrackSize.y = max(mmi.ptMinTrackSize.y, lpmmi->ptMinTrackSize.y);
                    if (lpmmi->ptMaxTrackSize.x <= MAXLONG - mmi.ptMaxTrackSize.x && lpmmi->ptMaxTrackSize.x + mmi.ptMaxTrackSize.x <= MAXLONG - (long)divider_size)
                    {
                        lpmmi->ptMaxTrackSize.x += mmi.ptMaxTrackSize.x + divider_size;
                    }
                    else
                    {
                        lpmmi->ptMaxTrackSize.x = MAXLONG;
                    }
                    lpmmi->ptMaxTrackSize.y = min(mmi.ptMaxTrackSize.y, lpmmi->ptMaxTrackSize.y);
                }
            }
        }
        if (b_found)
        {
            if (get_orientation() == vertical)
                lpmmi->ptMaxTrackSize.x = max(lpmmi->ptMaxTrackSize.x, lpmmi->ptMinTrackSize.x);
            else
                lpmmi->ptMaxTrackSize.y = max(lpmmi->ptMaxTrackSize.y, lpmmi->ptMinTrackSize.y);
        }
        else
        {
            if (get_orientation() == vertical)
                lpmmi->ptMaxTrackSize.y = MAXLONG;
            else
                lpmmi->ptMaxTrackSize.x = MAXLONG;
        }

        if (0)
        {
            lpmmi->ptMinTrackSize.y = 0;
            if (get_orientation() == horizontal) lpmmi->ptMaxTrackSize.x = 0;
            lpmmi->ptMinTrackSize.x = 0;
            if (get_orientation() == vertical) lpmmi->ptMaxTrackSize.y = 0;
        }
    }
    return 0;
    case WM_MOUSEHOVER:
    {
        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        HWND child = RealChildWindowFromPoint(wnd, pt);
        if (child == wnd)
        {
            unsigned p_panel = -1;
            bool b_have_next = false;
            bool b_on_divider = false;

            b_on_divider = find_by_divider_pt(pt, p_panel);

            if (b_on_divider)
            {
                if (p_panel < m_panels.get_count())
                {
                    b_have_next = (p_panel + 1 < m_panels.get_count());
                }

                if (is_index_valid(p_panel))
                    start_autohide_dehide(p_panel);
            }
        }

    }
    break;
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
    {
        if (m_panels.get_count())
        {

            POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
            HWND child = RealChildWindowFromPoint(wnd, pt);
            if (child == wnd)
            {
                unsigned p_panel = -1;
                bool b_have_next = false;
                bool b_on_divider = false;
                if (m_panel_dragging_valid)
                {
                    b_on_divider = true;
                    p_panel = m_panel_dragging;
                }
                else
                    b_on_divider = find_by_divider_pt(pt, p_panel);

                if (b_on_divider)
                {
                    if (p_panel < m_panels.get_count())
                    {
                        b_have_next = (p_panel + 1 < m_panels.get_count());
                    }

                    if (msg == WM_MOUSEMOVE && ((is_index_valid(p_panel) && m_panels[p_panel]->m_autohide) || (b_have_next && m_panels[p_panel + 1]->m_autohide)))
                    {
                        if (cfg_sidebar_use_custom_show_delay && !cfg_sidebar_show_delay)
                        {
                            if ((is_index_valid(p_panel)))
                            {
                                start_autohide_dehide(p_panel);
                            }
                        }
                        else
                        {
                            TRACKMOUSEEVENT tme;
                            memset(&tme, 0, sizeof(TRACKMOUSEEVENT));
                            tme.cbSize = sizeof(TRACKMOUSEEVENT);
                            tme.dwFlags = TME_QUERY;
                            tme.hwndTrack = wnd;
                            _TrackMouseEvent(&tme);

                            if (!(tme.dwFlags & TME_HOVER))
                            {
                                memset(&tme, 0, sizeof(TRACKMOUSEEVENT));
                                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                                tme.hwndTrack = wnd;
                                tme.dwHoverTime = cfg_sidebar_use_custom_show_delay ? cfg_sidebar_show_delay : HOVER_DEFAULT;
                                tme.dwFlags = TME_HOVER;
                                _TrackMouseEvent(&tme);
                            }
                        }

                    }
                }

                if (b_on_divider && is_index_valid(p_panel) && can_resize_divider(p_panel))
                {
                    SetCursor(LoadCursor(nullptr, get_orientation() == horizontal ? IDC_SIZEWE : IDC_SIZENS));

                    if (msg == WM_LBUTTONDOWN)
                    {
                        save_sizes();

                        m_panel_dragging = p_panel;
                        SetCapture(wnd);

                        m_last_position = (get_orientation() == vertical ? pt.y : pt.x);
                        m_panel_dragging_valid = true;
                    }
                }
                else
                {
                    if (!(wp & MK_LBUTTON)) SetCursor(LoadCursor(nullptr, IDC_ARROW));
                    m_panel_dragging_valid = false;
                }
            }

            if (m_panel_dragging_valid && wp & MK_LBUTTON && is_index_valid(m_panel_dragging))
            {
                int new_height = m_last_position - (get_orientation() == vertical ? pt.y : pt.x);
                int delta = (get_orientation() == vertical ? pt.y : pt.x) - m_last_position;
                //console::formatter() << "before or: pt = " << pt.y << "," << pt.x << " lastpos: " << m_last_position << " enddelta: " << delta;
                auto & p_panel = m_panels[m_panel_dragging];
                if (p_panel->m_hidden && delta)
                {
                    p_panel->m_hidden = false;
                    p_panel->m_size = 0;
                    get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
                }
                int delta_changed = override_size(m_panel_dragging, delta);
                m_last_position = (get_orientation() == vertical ? pt.y : pt.x) + delta_changed;
                on_size_changed();
                if (delta + delta_changed)
                    start_autohide_dehide(m_panel_dragging);
            }
        }
        //msg_last = msg;
        //lp_last = lp;
        //wp_last = wp;

    }
    break;
    case WM_LBUTTONDBLCLK:
    {
        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        HWND child = ChildWindowFromPoint(wnd, pt);
        if (child == wnd)
        {
            unsigned p_panel = -1;
            if (find_by_divider_pt(pt, p_panel) && is_index_valid(p_panel))
            {
                bool b_have_next = is_index_valid(p_panel + 1);
                if (m_panels[p_panel]->m_locked && !m_panels[p_panel]->m_autohide && (!b_have_next || !m_panels[p_panel + 1]->m_locked))
                {
                    m_panels[p_panel]->m_hidden = !m_panels[p_panel]->m_hidden;
                    get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
                    on_size_changed();
                }
                else if (!m_panels[p_panel]->m_locked && b_have_next && m_panels[p_panel]->m_locked && !m_panels[p_panel]->m_autohide)
                {
                    m_panels[p_panel + 1]->m_hidden = !m_panels[p_panel]->m_hidden;
                    get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
                    on_size_changed();
                }
            }
        }
    }
    break;
    case WM_LBUTTONUP:
        if (m_panel_dragging_valid)
        {
            m_panel_dragging_valid = false;
            if (GetCapture() == wnd)
                ReleaseCapture();
            //SetCursor(LoadCursor(0, IDC_ARROW));
        }
        break;
#if 0
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(wnd, &ps);
        COLORREF cr = GetSysColor(COLOR_3DFACE);
        gdi_object_t<HBRUSH>::ptr_t br_line = CreateSolidBrush(/*RGB(226, 226, 226)*/cr);

        t_size n, count = m_panels.get_count();
        for (n = 0; n + 1<count; n++)
        {
            pfc::refcounted_object_ptr_t<panel> p_item = m_panels.get_item(n);

            if (p_item->m_wnd_child)
            {
                RECT rc_area;
                GetRelativeRect(p_item->m_wnd_child, m_wnd, &rc_area);
                if (get_orientation() == vertical)
                {
                    rc_area.top = rc_area.bottom;
                    rc_area.bottom += 2;
                    //FillRect(ps.hdc, &rc_area, GetSysColorBrush(COLOR_WINDOW));
                    //rc_area.top++;
                }
                else
                {
                    rc_area.left = rc_area.right;
                    rc_area.right += 2;
                    //FillRect(ps.hdc, &rc_area, GetSysColorBrush(COLOR_WINDOW));
                    //rc_area.right--;
                }
                FillRect(ps.hdc, &rc_area, br_line);
            }
        }

        EndPaint(wnd, &ps);

    }
    ;
#endif
#if 0
    case WM_CONTEXTMENU:
        if ((HWND)wp == wnd)
        {
            window_transparent_fill m_trans_fill;

            if (m_layout_editing_active)
            {
                RECT rc;
                GetRelativeRect(wnd, HWND_DESKTOP, &rc);
                ShowWindow(m_trans_fill.create(get_wnd(), 0, ui_helpers::window_position_t(rc)), SW_SHOWNORMAL);
                POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };

                HMENU menu = CreatePopupMenu();
                HMENU menu_add = CreatePopupMenu();
                uie::window_info_list_simple panels;
                g_get_panel_list(panels);
                enum { ID_CLOSE = 1, ID_ADD_BASE = 2 };
                g_append_menu_panels(menu_add, panels, ID_ADD_BASE);
                pfc::string8 temp;
                get_name(temp);
                uAppendMenu(menu, MF_STRING | MF_GRAYED, (UINT_PTR)0, temp);
                uAppendMenu(menu, MF_MENUBREAK, (UINT_PTR)0, NULL);
                AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)menu_add, L"Add panel");

                int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, m_trans_fill.get_wnd(), 0);
                DestroyMenu(menu);
                m_trans_fill.destroy();

                if (cmd)
                {
                    if (cmd >= ID_ADD_BASE && cmd < panels.get_count() + ID_ADD_BASE)
                    {
                        pfc::refcounted_object_ptr_t<panel> ptr = new panel;
                        ptr->m_guid = panels[cmd - ID_ADD_BASE].guid;
                        m_panels.add_item(ptr);
                        refresh_children();
                        get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
                        uie::splitter_window_v2_ptr sw2;
                        if (ptr->m_child.is_valid() && ptr->m_child->service_query_t(sw2))
                        {
                            sw2->enter_layout_editing_mode();
                        }
                    }
                }
            }
            return 0;
        }
        break;
#endif
    }
    return DefWindowProc(wnd, msg, wp, lp);
}
