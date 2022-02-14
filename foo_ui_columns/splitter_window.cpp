#include "stdafx.h"
#include "splitter.h"

namespace cui::panels::splitter {

ui_extension::window_host_factory<FlatSplitterPanel::FlatSplitterPanelHost> g_splitter_host_vert;

unsigned FlatSplitterPanel::g_count = 0;
wil::unique_hfont FlatSplitterPanel::g_font_menu_horizontal;
wil::unique_hfont FlatSplitterPanel::g_font_menu_vertical;

void FlatSplitterPanel::insert_panel(unsigned index, const uie::splitter_item_t* p_item)
{
    if (index <= m_panels.get_count()) {
        auto temp = std::make_shared<Panel>();
        temp->set_from_splitter_item(p_item);
        m_panels.insert_item(temp, index);

        if (get_wnd()) {
            refresh_children();
        }
    }
}

void FlatSplitterPanel::replace_panel(unsigned index, const uie::splitter_item_t* p_item)
{
    if (index < m_panels.get_count()) {
        if (get_wnd())
            m_panels[index]->destroy();
        auto temp = std::make_shared<Panel>();
        temp->set_from_splitter_item(p_item);
        m_panels.replace_item(index, temp);

        if (get_wnd())
            refresh_children();
    }
}

void FlatSplitterPanel::destroy_children()
{
    unsigned count = m_panels.get_count();
    for (unsigned n = 0; n < count; n++) {
        std::shared_ptr<Panel> pal = m_panels[n];
        if (pal->m_child.is_valid()) {
            //            pal->m_child_data.set_size(0);
            //            stream_writer_memblock_ref blah(pal->m_child_data);
            //            pal->m_child->get_config(&blah);
            pal->m_child->destroy_window();
            pal->m_wnd_child = nullptr;
            DestroyWindow(pal->m_wnd);
            pal->m_wnd = nullptr;
            pal->m_child.release();
            pal->m_interface.release();
            // pal->m_container.m_this.release();
        }
    }

    // m_wnd = NULL;
}

void FlatSplitterPanel::refresh_children()
{
    unsigned n;
    unsigned count = m_panels.get_count();
    unsigned size_cumulative = 0;
    pfc::array_t<bool> new_items;
    new_items.set_count(count);
    new_items.fill_null();
    for (n = 0; n < count; n++) {
        if (!m_panels[n]->m_wnd) {
            uie::window_ptr p_ext = m_panels[n]->m_child;

            bool b_new = false;

            if (!p_ext.is_valid()) {
                create_by_guid(m_panels[n]->m_guid, p_ext);
                b_new = true;
            }

            if (!m_panels[n]->m_interface.is_valid()) {
                service_ptr_t<service_base> temp;
                g_splitter_host_vert.instance_create(temp);
                uie::window_host_ptr ptr;
                if (temp->service_query_t(ptr)) {
                    m_panels[n]->m_interface = static_cast<FlatSplitterPanelHost*>(ptr.get_ptr());
                    m_panels[n]->m_interface->set_window_ptr(this);
                }
            }

            if (p_ext.is_valid()
                && p_ext->is_available(
                    uie::window_host_ptr(static_cast<uie::window_host*>(m_panels[n]->m_interface.get_ptr())))) {
                pfc::string8 name;
                if (m_panels[n]->m_use_custom_title) {
                    name = m_panels[n]->m_custom_title;
                } else {
                    if (!p_ext->get_short_name(name))
                        p_ext->get_name(name);
                }

                HWND wnd_host = m_panels[n]->m_container.create(m_wnd);
                m_panels[n]->m_container.set_window_ptr(this);

                uSetWindowText(wnd_host, name);

                if (wnd_host) {
                    if (b_new) {
                        try {
                            abort_callback_dummy p_abort;
                            p_ext->set_config_from_ptr(
                                m_panels[n]->m_child_data.get_ptr(), m_panels[n]->m_child_data.get_size(), p_abort);
                        } catch (const exception_io& e) {
                            console::formatter formatter;
                            formatter << "Error setting panel config: " << e.what();
                        }
                    }

                    HWND wnd_panel = p_ext->create_or_transfer_window(
                        wnd_host, uie::window_host_ptr(m_panels[n]->m_interface.get_ptr())); // FIXX
                    if (wnd_panel) {
                        SetWindowLongPtr(
                            wnd_panel, GWL_STYLE, GetWindowLongPtr(wnd_panel, GWL_STYLE) | WS_CLIPSIBLINGS);
                        MINMAXINFO mmi{};
                        mmi.ptMaxTrackSize.x = MAXLONG;
                        mmi.ptMaxTrackSize.y = MAXLONG;
                        SendMessage(wnd_panel, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
                        helpers::clip_minmaxinfo(mmi);

                        m_panels[n]->m_wnd = wnd_host;
                        m_panels[n]->m_wnd_child = wnd_panel;
                        m_panels[n]->m_child = p_ext;
                        m_panels[n]->m_size_limits.min_height = mmi.ptMinTrackSize.y;
                        m_panels[n]->m_size_limits.min_width = mmi.ptMinTrackSize.x;
                        m_panels[n]->m_size_limits.max_width = mmi.ptMaxTrackSize.x;
                        m_panels[n]->m_size_limits.max_height = mmi.ptMaxTrackSize.y;

                        /*console::formatter() << "name: " << name <<
                        " min width: " << (t_int32)mmi.ptMinTrackSize.x
                        << " min height: " << (t_int32)mmi.ptMinTrackSize.y
                        << " max width: " << (t_int32)mmi.ptMaxTrackSize.y
                        << " max height: " << (t_int32)mmi.ptMaxTrackSize.y;*/

                    } else {
                        m_panels[n]->m_container.destroy();
                    }
                }
            }
            new_items[n] = true; // b_new;
        }
    }

    on_size_changed();

    if (IsWindowVisible(get_wnd())) {
        for (n = 0; n < count; n++) {
            if (new_items[n]) {
                ShowWindow(m_panels[n]->m_wnd_child, SW_SHOWNORMAL);
                ShowWindow(m_panels[n]->m_wnd, SW_SHOWNORMAL);
            }
        }
        get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
        RedrawWindow(get_wnd(), nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);
    }
}

void FlatSplitterPanel::on_size_changed(unsigned width, unsigned height)
{
    pfc::list_t<unsigned> sizes;
    get_panels_sizes(width, height, sizes);
    unsigned count = m_panels.get_count();

    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE);

    HDWP dwp = BeginDeferWindowPos(m_panels.get_count());
    if (dwp) {
        unsigned size_cumulative = 0;
        for (unsigned n = 0; n < count; n++) {
            if (m_panels[n]->m_child.is_valid() && m_panels[n]->m_wnd) {
                unsigned size = sizes[n];

                unsigned x = get_orientation() == horizontal ? size_cumulative : 0;
                unsigned y = get_orientation() == horizontal ? 0 : size_cumulative;
                unsigned cx = get_orientation() == horizontal ? size - get_panel_divider_size(n) : width;
                unsigned cy = get_orientation() == horizontal ? height : size - get_panel_divider_size(n);

                dwp = DeferWindowPos(dwp, m_panels[n]->m_wnd, HWND_BOTTOM, x, y, cx, cy, 0);

                size_cumulative += size;
            }
        }
        EndDeferWindowPos(dwp);
    }
    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_UPDATENOW);
}

void FlatSplitterPanel::on_size_changed()
{
    RECT rc;
    GetClientRect(m_wnd, &rc);
    on_size_changed(rc.right, rc.bottom);
}

bool FlatSplitterPanel::find_by_divider_pt(POINT& pt, unsigned& p_out)
{
    unsigned count = m_panels.get_count();
    for (unsigned n = 0; n < count; n++) {
        std::shared_ptr<Panel> p_item = m_panels.get_item(n);

        if (p_item->m_wnd_child) {
            RECT rc_area;
            GetRelativeRect(p_item->m_wnd_child, m_wnd, &rc_area);

            if (PtInRect(&rc_area, pt))
                return false;

            bool in_divider = false;
            if (get_orientation() == vertical) {
                in_divider = (pt.y >= rc_area.bottom) && (pt.y < (rc_area.bottom + (LONG)get_panel_divider_size(n)));
            } else {
                in_divider = pt.x >= rc_area.right && pt.x < (rc_area.right + (LONG)get_panel_divider_size(n));
            }
            if (in_divider) {
                p_out = n;
                return true;
            }
        }
    }
    return false;
}

bool FlatSplitterPanel::test_divider_pt(const POINT& pt, unsigned index)
{
    unsigned divider_index;
    POINT pt2 = pt;
    if (find_by_divider_pt(pt2, divider_index)) {
        return divider_index == index || (index && divider_index == index - 1);
    }
    return false;
}

void FlatSplitterPanel::save_sizes(unsigned width, unsigned height)
{
    pfc::list_t<unsigned> sizes;
    get_panels_sizes(width, height, sizes);
    unsigned count = m_panels.get_count();

    for (unsigned n = 0; n < count; n++) {
        if (!m_panels[n]->m_hidden)
            m_panels[n]->m_size = sizes[n] - get_panel_divider_size(n);
    }
}

void FlatSplitterPanel::save_sizes()
{
    RECT rc;
    GetClientRect(m_wnd, &rc);
    save_sizes(rc.right, rc.bottom);
}

void FlatSplitterPanel::get_panels_sizes(
    unsigned client_width, unsigned client_height, pfc::list_base_t<unsigned>& p_out)
{
    struct t_size_info {
        unsigned height;
        bool sized;
        unsigned parts;
    };

    unsigned n;
    unsigned count = m_panels.get_count();
    unsigned height_allocated = 0;

    if (count) {
        pfc::array_t<t_size_info> size_info;
        size_info.set_size(count);
        // size_info.fill(0);
        memset(size_info.get_ptr(), 0, size_info.get_size() * sizeof(t_size_info));

        unsigned caption_size = g_get_caption_size();
        // unsigned divider_width = 2;

        int available_height = get_orientation() == horizontal ? client_width : client_height;
        unsigned available_parts = 0;

        for (n = 0; n < count; n++) {
            unsigned panel_divider_size = get_panel_divider_size(n);

            unsigned height = m_panels[n]->m_hidden ? 0 : m_panels[n]->m_size.get_scaled_value();
            if (height > MAXLONG)
                height = MAXLONG;
            if (available_height > (-MAXLONG + (int)height))
                available_height -= height;
            else
                available_height = -MAXLONG;
            if (available_height > (-MAXLONG + (int)panel_divider_size))
                available_height -= panel_divider_size;
            else
                available_height = -MAXLONG;

            size_info[n].height = height + panel_divider_size;
            size_info[n].parts = (m_panels[n]->m_locked || m_panels[n]->m_hidden) ? 0 : 1;
            available_parts += size_info[n].parts;
        }

        do {
            unsigned this_pass_available_parts = available_parts;
            int this_pass_available_height = available_height;

            for (n = 0; n < count; n++) {
                if (!size_info[n].sized) {
                    unsigned panel_divider_size = get_panel_divider_size(n);
                    unsigned panel_caption_size
                        = (get_orientation() != m_panels[n]->m_caption_orientation && m_panels[n]->m_show_caption)
                        ? caption_size
                        : 0;

                    unsigned height = size_info[n].height;

                    int adjustment = 0;
                    {
                        adjustment = this_pass_available_parts
                            ? MulDiv(this_pass_available_height, size_info[n].parts, this_pass_available_parts)
                            : 0;
                        this_pass_available_parts -= size_info[n].parts;
                        this_pass_available_height -= adjustment;
                    }

                    if ((adjustment < 0
                            && (height > panel_divider_size ? height - panel_divider_size : 0)
                                < (unsigned)(adjustment * -1))) {
                        adjustment = (height > panel_divider_size ? height - panel_divider_size : 0) * -1;
                        size_info[n].sized = true;
                    }

                    unsigned unadjusted = height;

                    bool hidden = m_panels[n]->m_hidden;

                    height += adjustment;

                    unsigned min_height = hidden
                        ? 0
                        : (get_orientation() == horizontal ? m_panels[n]->m_size_limits.min_width
                                                           : m_panels[n]->m_size_limits.min_height);
                    if (min_height < (unsigned)(pfc_infinite)-panel_divider_size - caption_size)
                        min_height += panel_divider_size + panel_caption_size;

                    unsigned max_height = hidden
                        ? 0
                        : (get_orientation() == horizontal ? m_panels[n]->m_size_limits.max_width
                                                           : m_panels[n]->m_size_limits.max_height);
                    if (max_height < (unsigned)(pfc_infinite)-panel_divider_size - caption_size)
                        max_height += panel_divider_size + panel_caption_size;

                    if (get_orientation() == horizontal && m_panels[n]->m_show_toggle_area
                        && !m_panels[n]->m_autohide) {
                        if (max_height < unsigned(pfc_infinite) - 1)
                            max_height++;
                        if (min_height < unsigned(pfc_infinite) - 1)
                            min_height++;
                    }

                    if (height < min_height) {
                        height = min_height;
                        adjustment = (height - unadjusted);
                        size_info[n].sized = true;
                    } else if (height > max_height) {
                        height = max_height;
                        adjustment = (height - unadjusted);
                        size_info[n].sized = true;
                    }
                    if (m_panels[n]->m_locked || hidden)
                        size_info[n].sized = true;

                    if (size_info[n].sized)
                        available_parts -= size_info[n].parts;

                    available_height -= (height - unadjusted);
                    size_info[n].height = height;
                }
            }
        } while (available_parts && available_height);

        for (n = 0; n < count; n++) {
            p_out.add_item(size_info[n].height);
        }
    }
}

bool FlatSplitterPanel::can_resize_divider(t_size index) const
{
    t_size count_left = 0;
    t_size count_right = 0;
    for (t_size i = 0, count = m_panels.get_count(); i < count; i++) {
        if (can_resize_panel(i)) {
            if (i <= index)
                count_left++;
            else
                count_right++;
        }
    }
    return count_left && count_right;
}

bool FlatSplitterPanel::can_resize_panel(t_size index) const
{
    const auto& panel = m_panels[index];

    if (!settings::allow_locked_panel_resizing && panel->m_locked)
        return false;

    if (panel->m_wnd_child) {
        RECT rc_window;
        GetWindowRect(panel->m_wnd_child, &rc_window);

        if (get_orientation() == vertical && RECT_CY(rc_window) == panel->m_size_limits.max_height
            && RECT_CY(rc_window) == panel->m_size_limits.min_height)
            return false;

        if (get_orientation() == horizontal && RECT_CX(rc_window) == panel->m_size_limits.max_width
            && RECT_CX(rc_window) == panel->m_size_limits.min_width)
            return false;
    }

    return true;
}

int FlatSplitterPanel::override_size(unsigned& panel, int delta)
{
    // console::formatter() << "Overriding " << panel << " by " << delta;
    struct t_min_max_info {
        unsigned min_height;
        unsigned max_height;
        unsigned height;
        // unsigned caption_height;
    };

    unsigned count = m_panels.get_count();
    if (count) {
        save_sizes();
        if (panel + 1 < count) {
            unsigned n = 0;

            unsigned the_caption_height = g_get_caption_size();
            pfc::array_t<t_min_max_info> minmax;
            minmax.set_size(count);

            // minmax.fill(0);
            memset(minmax.get_ptr(), 0, minmax.get_size() * sizeof(t_min_max_info));

            for (n = 0; n < count; n++) {
                unsigned caption_height
                    = m_panels[n]->m_show_caption && m_panels[n]->m_caption_orientation != get_orientation()
                    ? the_caption_height
                    : 0;
                unsigned min_height = m_panels[n]->m_hidden ? 0
                    : get_orientation() == vertical         ? m_panels[n]->m_size_limits.min_height
                                                            : m_panels[n]->m_size_limits.min_width;
                unsigned max_height = m_panels[n]->m_hidden ? 0
                    : get_orientation() == vertical         ? m_panels[n]->m_size_limits.max_height
                                                            : m_panels[n]->m_size_limits.max_width;

                if (min_height < (unsigned)(0 - caption_height))
                    min_height += caption_height;
                if (max_height < (unsigned)(0 - caption_height))
                    max_height += caption_height;

                if (get_orientation() == horizontal && m_panels[n]->m_show_toggle_area && !m_panels[n]->m_autohide) {
                    if (max_height < unsigned(pfc_infinite) - 1)
                        max_height++;
                    if (min_height < unsigned(pfc_infinite) - 1)
                        min_height++;
                    caption_height++;
                }

                // minmax[n].caption_height = caption_height;
                minmax[n].min_height = min_height;
                minmax[n].max_height = max_height;
                minmax[n].height = m_panels[n]->m_hidden ? caption_height : m_panels[n]->m_size.get_scaled_value();
            }

            bool is_up = delta < 0; // new_height < m_panels[panel].height;
            bool is_down = delta > 0; // new_height > m_panels[panel].height;

            if (is_up /*&& !m_panels[panel].locked*/) {
                unsigned diff_abs = 0;
                unsigned diff_avail = abs(delta);

                unsigned n = panel + 1;
                while (n < count && diff_abs < diff_avail) {
                    {
                        unsigned height = minmax[n].height
                            + (diff_avail - diff_abs); //(diff_avail-diff_abs > m_panels[n]->height ? 0 :
                                                       // m_panels[n]->height-(diff_avail-diff_abs));

                        unsigned min_height = minmax[n].min_height;
                        unsigned max_height = minmax[n].max_height;

                        if (height < min_height) {
                            height = min_height;
                        } else if (height > max_height) {
                            height = max_height;
                        }

                        diff_abs += height - minmax[n].height;
                    }
                    n++;
                }

                n = panel + 1;
                unsigned obtained = 0;
                while (n > 0 && obtained < diff_abs) {
                    n--;
                    //                    if (!m_panels[n]->locked)
                    {
                        unsigned height
                            = (diff_abs - obtained > minmax[n].height ? 0 : minmax[n].height - (diff_abs - obtained));

                        // unsigned caption_height = m_panels[n]->m_show_caption ? the_caption_height : 0;

                        unsigned min_height = minmax[n].min_height;
                        unsigned max_height = minmax[n].max_height;

                        if (height < min_height) {
                            height = min_height;
                        } else if (height > max_height) {
                            height = max_height;
                        }

                        obtained += minmax[n].height - height;
                        minmax[n].height = height;
                        if (!m_panels[n]->m_hidden)
                            m_panels[n]->m_size = height;
                    }
                }
                n = panel;
                unsigned obtained2 = obtained;

                while (n < count - 1 && obtained2) {
                    n++;
                    unsigned height = (minmax[n].height);

                    unsigned min_height = minmax[n].min_height;
                    unsigned max_height = minmax[n].max_height;

                    height += obtained2;

                    if (height < min_height) {
                        height = min_height;
                    } else if (height > max_height) {
                        height = max_height;
                    }

                    obtained2 -= height - minmax[n].height;
                    minmax[n].height = height;
                    if (!m_panels[n]->m_hidden)
                        m_panels[n]->m_size = height;
                }
                return (abs(delta) - obtained);
            }
            if (is_down /*&& !m_panels[panel].locked*/) {
                unsigned diff_abs = 0;
                unsigned diff_avail = abs(delta);

                n = panel + 1;
                while (n > 0 && diff_abs < diff_avail) {
                    n--;
                    {
                        unsigned height = minmax[n].height
                            + (diff_avail - diff_abs); //(diff_avail-diff_abs > m_panels[n]->height ? 0 :
                        // m_panels[n]->height-(diff_avail-diff_abs));
                        // console::formatter() << "1: " << height << " " << minmax[n].height << " " <<
                        // (diff_avail-diff_abs);

                        unsigned min_height = minmax[n].min_height;
                        unsigned max_height = minmax[n].max_height;

                        if (height < min_height) {
                            height = min_height;
                        } else if (height > max_height) {
                            height = max_height;
                        }

                        diff_abs += height - minmax[n].height;
                    }
                }
                n = panel;
                unsigned obtained = 0;
                while (n < count - 1 && obtained < diff_abs) {
                    n++;
                    //                if (!m_panels[n]->locked)
                    {
                        unsigned height
                            = (diff_abs - obtained > minmax[n].height ? 0 : minmax[n].height - (diff_abs - obtained));
                        // console::formatter() << "2: " << height << " " << minmax[n].height << " " <<
                        // (diff_abs-obtained);

                        // unsigned caption_height = minmax[n].caption_height;
                        unsigned min_height = minmax[n].min_height;
                        unsigned max_height = minmax[n].max_height;

                        if (height < min_height) {
                            height = min_height;
                        } else if (height > max_height) {
                            height = max_height;
                        }

                        obtained += minmax[n].height - height;
                        minmax[n].height = height;
                        if (!m_panels[n]->m_hidden)
                            m_panels[n]->m_size = height;
                    }
                }
                n = panel + 1;
                unsigned obtained2 = obtained;
                while (n > 0 && obtained2) {
                    n--;
                    unsigned height = (minmax[n].height);
                    unsigned min_height = minmax[n].min_height;
                    unsigned max_height = minmax[n].max_height;

                    height += obtained2;

                    if (height < min_height) {
                        height = min_height;
                    } else if (height > max_height) {
                        height = max_height;
                    }

                    obtained2 -= height - minmax[n].height;

                    minmax[n].height = height;

                    if (!m_panels[n]->m_hidden)
                        m_panels[n]->m_size = height;
                }
                // console::formatter() << "3: " << abs(delta) << " " << obtained;
                return 0 - (abs(delta) - obtained);
            }
        }
    }
    return 0;
}

void FlatSplitterPanel::start_autohide_dehide(unsigned p_panel, bool b_next_too)
{
    bool b_have_next = b_next_too && is_index_valid(p_panel + 1);
    auto& panel_before = m_panels[p_panel];
    auto& panel_after = b_have_next ? m_panels[p_panel + 1] : Panel::null_ptr;
    if ((panel_before->m_autohide && !panel_before->m_container.m_hook_active)
        || (b_have_next && panel_after->m_autohide && !panel_after->m_container.m_hook_active)) {
        bool a1 = false;
        bool a2 = false;
        if (panel_before->m_autohide && !panel_before->m_container.m_hook_active) {
            panel_before->m_hidden = false;
            a1 = true;
        }
        if (b_have_next && panel_after->m_autohide && !panel_after->m_container.m_hook_active) {
            panel_after->m_hidden = false;
            a2 = true;
        }
        if (a1 || a2) {
            get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
            on_size_changed();
            if (a1)
                panel_before->m_container.enter_autohide_hook();
            if (a2)
                panel_after->m_container.enter_autohide_hook();
        }
    }
}

void FlatSplitterPanel::get_supported_panels(
    const pfc::list_base_const_t<window::ptr>& p_windows, bit_array_var& p_mask_unsupported)
{
    service_ptr_t<service_base> temp;
    g_splitter_host_vert.instance_create(temp);
    uie::window_host_ptr ptr;
    if (temp->service_query_t(ptr))
        (static_cast<FlatSplitterPanelHost*>(ptr.get_ptr()))->set_window_ptr(this);
    t_size count = p_windows.get_count();
    for (t_size i = 0; i < count; i++)
        p_mask_unsupported.set(i, !p_windows[i]->is_available(ptr));
}

bool FlatSplitterPanel::is_point_ours(
    HWND wnd_point, const POINT& pt_screen, pfc::list_base_t<window::ptr>& p_hierarchy)
{
    if (wnd_point == get_wnd() || IsChild(get_wnd(), wnd_point)) {
        if (wnd_point == get_wnd()) {
            p_hierarchy.add_item(this);
            return true;
        }
        t_size count = m_panels.get_count();
        for (t_size i = 0; i < count; i++) {
            uie::splitter_window_v2_ptr sptr;
            if (m_panels[i]->m_child.is_valid()) {
                if (m_panels[i]->m_child->service_query_t(sptr)) {
                    pfc::list_t<window::ptr> temp;
                    temp.add_item(this);
                    if (sptr->is_point_ours(wnd_point, pt_screen, temp)) {
                        p_hierarchy.add_items(temp);
                        return true;
                    }
                } else if (wnd_point == m_panels[i]->m_wnd_child || IsChild(m_panels[i]->m_wnd_child, wnd_point)) {
                    p_hierarchy.add_item(this);
                    p_hierarchy.add_item(m_panels[i]->m_child);
                    return true;
                } else if (wnd_point == m_panels[i]->m_wnd) {
                    p_hierarchy.add_item(this);
                    return true;
                }
            }
        }
    }
    return false;
}

unsigned FlatSplitterPanel::get_panel_divider_size(unsigned index)
{
    return index + 1 < m_panels.get_count() ? settings::custom_splitter_divider_width : 0;
}

bool FlatSplitterPanel::set_config_item(
    unsigned index, const GUID& p_type, stream_reader* p_source, abort_callback& p_abort)
{
    if (is_index_valid(index)) {
        if (p_type == bool_show_caption) {
            p_source->read_object_t(m_panels[index]->m_show_caption, p_abort);
            if (get_wnd()) {
                get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
                on_size_changed();
                m_panels[index]->on_size();
            }
            return true;
        }
        if (p_type == bool_hidden) {
            if (!m_panels[index]->m_autohide)
                p_source->read_object_t(m_panels[index]->m_hidden, p_abort);
            return true;
        }
        if (p_type == bool_autohide) {
            p_source->read_object_t(m_panels[index]->m_autohide, p_abort);
            m_panels[index]->m_hidden = m_panels[index]->m_autohide;
            return true;
        }
        if (p_type == bool_locked) {
            if (get_wnd())
                save_sizes();
            p_source->read_object_t(m_panels[index]->m_locked, p_abort);
            return true;
        }
        if (p_type == uint32_orientation) {
            p_source->read_object_t(m_panels[index]->m_caption_orientation, p_abort);
            return true;
        }
        if (p_type == uint32_size) {
            uint32_t size;
            p_source->read_object_t(size, p_abort);
            m_panels[index]->m_size = size;
            return true;
        }
        if (p_type == size_and_dpi) {
            uie::size_and_dpi sad;
            p_source->read_object_t(sad.size, p_abort);
            p_source->read_object_t(sad.dpi, p_abort);
            m_panels[index]->m_size.value = sad.size;
            m_panels[index]->m_size.dpi = sad.dpi;
            return true;
        }
        if (p_type == bool_show_toggle_area && get_orientation() == horizontal) {
            p_source->read_object_t(m_panels[index]->m_show_toggle_area, p_abort);
            return true;
        }
        if (p_type == bool_use_custom_title) {
            p_source->read_object_t(m_panels[index]->m_use_custom_title, p_abort);
            return true;
        }
        if (p_type == string_custom_title) {
            p_source->read_string(m_panels[index]->m_custom_title, p_abort);
            return true;
        }
        return false;
    }
    return false;
}

bool FlatSplitterPanel::get_config_item(
    unsigned index, const GUID& p_type, stream_writer* p_out, abort_callback& p_abort) const
{
    if (is_index_valid(index)) {
        if (p_type == bool_show_caption) {
            p_out->write_object_t(m_panels[index]->m_show_caption, p_abort);
            return true;
        }
        if (p_type == bool_hidden) {
            p_out->write_object_t(m_panels[index]->m_hidden, p_abort);
            return true;
        }
        if (p_type == bool_autohide) {
            p_out->write_object_t(m_panels[index]->m_autohide, p_abort);
            return true;
        }
        if (p_type == bool_locked) {
            p_out->write_object_t(m_panels[index]->m_locked, p_abort);
            return true;
        }
        if (p_type == uint32_orientation) {
            p_out->write_object_t(m_panels[index]->m_caption_orientation, p_abort);
            return true;
        }
        if (p_type == uint32_size) {
            p_out->write_object_t(m_panels[index]->m_size.get_scaled_value(), p_abort);
            return true;
        }
        if (p_type == size_and_dpi) {
            p_out->write_object_t(m_panels[index]->m_size.value, p_abort);
            p_out->write_object_t(m_panels[index]->m_size.dpi, p_abort);
            return true;
        }
        if (p_type == bool_show_toggle_area && get_orientation() == horizontal) {
            p_out->write_object_t(m_panels[index]->m_show_toggle_area, p_abort);
            return true;
        }
        if (p_type == bool_use_custom_title) {
            p_out->write_object_t(m_panels[index]->m_use_custom_title, p_abort);
            return true;
        }
        if (p_type == string_custom_title) {
            p_out->write_string(m_panels[index]->m_custom_title, p_abort);
            return true;
        }
        return false;
    }
    return false;
}

bool FlatSplitterPanel::get_config_item_supported(unsigned index, const GUID& p_type) const
{
    if (is_index_valid(index)) {
        if (p_type == bool_show_caption || p_type == bool_locked || p_type == bool_hidden
            || p_type == uint32_orientation || p_type == bool_autohide
            || (p_type == bool_show_toggle_area && get_orientation() == horizontal) || p_type == uint32_size
            || p_type == size_and_dpi || p_type == bool_use_custom_title || p_type == string_custom_title)
            return true;
    }
    return false;
}

bool FlatSplitterPanel::is_index_valid(unsigned index) const
{
    return index < m_panels.get_count();
}

void FlatSplitterPanel::get_config(stream_writer* out, abort_callback& p_abort) const
{
    write_config(out, false, p_abort);
}

void FlatSplitterPanel::export_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    write_config(p_writer, true, p_abort);
}

void FlatSplitterPanel::write_config(stream_writer* p_writer, bool is_export, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(static_cast<t_uint32>(stream_version_current), p_abort);
    unsigned i;
    unsigned count = m_panels.get_count();
    p_writer->write_lendian_t(count, p_abort);
    for (i = 0; i < count; i++) {
        if (is_export)
            m_panels[i]->_export(p_writer, p_abort);
        else
            m_panels[i]->write(p_writer, p_abort);
    }
    // Extra data added in version 0.5.0
    for (i = 0; i < count; i++) {
        stream_writer_memblock extraData;
        m_panels[i]->write_extra(&extraData, p_abort);
        p_writer->write_lendian_t(gsl::narrow<uint32_t>(extraData.m_data.get_size()), p_abort);
        p_writer->write(extraData.m_data.get_ptr(), extraData.m_data.get_size(), p_abort);
    }
}

void FlatSplitterPanel::read_config(stream_reader* config, t_size p_size, bool is_import, abort_callback& p_abort)
{
    if (p_size) {
        t_uint32 version;
        config->read_lendian_t(version, p_abort);
        if (version <= stream_version_current) {
            PanelList panels;

            unsigned count;
            config->read_lendian_t(count, p_abort);

            unsigned i;
            for (i = 0; i < count; i++) {
                auto temp = std::make_shared<Panel>();
                if (is_import)
                    temp->import(config, p_abort);
                else
                    temp->read(config, p_abort);
                panels.add_item(temp);
            }

            // Extra data added in version 0.5.0
            for (i = 0; i < count; i++) {
                uint32_t extraDataSize;
                try {
                    config->read_lendian_t(extraDataSize, p_abort);
                } catch (const exception_io_data_truncation&) {
                    if (i == 0)
                        break;
                    throw;
                }
                pfc::array_staticsize_t<t_uint8> columnExtraData(extraDataSize);
                config->read(columnExtraData.get_ptr(), columnExtraData.get_size(), p_abort);
                stream_reader_memblock_ref columnReader(columnExtraData);
                panels[i]->read_extra(&columnReader, p_abort);
            }

            m_panels = panels;
        }
    }
}

void FlatSplitterPanel::import_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort)
{
    read_config(p_reader, p_size, true, p_abort);
}

void FlatSplitterPanel::set_config(stream_reader* config, t_size p_size, abort_callback& p_abort)
{
    read_config(config, p_size, false, p_abort);
}

uie::splitter_item_t* FlatSplitterPanel::get_panel(unsigned index) const
{
    if (index < m_panels.get_count()) {
        return m_panels[index]->create_splitter_item();
    }
    return nullptr;
}

unsigned FlatSplitterPanel::get_panel_count() const
{
    return m_panels.get_count();
}

void FlatSplitterPanel::remove_panel(unsigned index)
{
    if (index < m_panels.get_count()) {
        m_panels[index]->destroy();
        m_panels.remove_by_idx(index);

        if (get_wnd())
            refresh_children();
    }
}

unsigned FlatSplitterPanel::get_type() const
{
    return ui_extension::type_layout | uie::type_splitter;
}

void FlatSplitterPanel::get_category(pfc::string_base& p_out) const
{
    p_out = "Splitters";
}

int FlatSplitterPanel::g_get_caption_size()
{
    return gsl::narrow<int>(uGetFontHeight(g_font_menu_horizontal.get())) + 8_spx;
}

void FlatSplitterPanel::FlatSplitterPanelHost::relinquish_ownership(HWND wnd)
{
    unsigned index;
    if (m_this->m_panels.find_by_wnd_child(wnd, index)) {
        std::shared_ptr<Panel> p_ext = m_this->m_panels[index];

        {
            if (GetAncestor(wnd, GA_PARENT) == p_ext->m_wnd) {
                console::warning("window left by ui extension");
                SetParent(wnd, core_api::get_main_window());
            }

            DestroyWindow(p_ext->m_wnd);
            p_ext->m_wnd = nullptr;
            p_ext->m_child.release();
            m_this->m_panels.remove_by_idx(index);
            m_this->on_size_changed();
        }
    }
}

void FlatSplitterPanel::FlatSplitterPanelHost::set_window_ptr(FlatSplitterPanel* p_ptr)
{
    m_this = p_ptr;
}

bool FlatSplitterPanel::FlatSplitterPanelHost::set_window_visibility(HWND wnd, bool visibility)
{
    bool rv = false;
    if (!m_this->get_host()->is_visible(m_this->get_wnd()))
        return m_this->get_host()->set_window_visibility(m_this->get_wnd(), visibility);
    unsigned idx = 0;
    if (m_this->m_panels.find_by_wnd_child(wnd, idx)) {
        if (!m_this->m_panels[idx]->m_autohide) {
            m_this->m_panels[idx]->m_hidden = !visibility;
            m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);
            m_this->on_size_changed();
        }
    }
    return rv;
}

bool FlatSplitterPanel::FlatSplitterPanelHost::is_visibility_modifiable(HWND wnd, bool desired_visibility) const
{
    bool rv = false;

    if (!m_this->get_host()->is_visible(m_this->get_wnd()))
        return m_this->get_host()->is_visibility_modifiable(m_this->get_wnd(), desired_visibility);
    unsigned idx = 0;
    if (m_this->m_panels.find_by_wnd_child(wnd, idx)) {
        rv = !m_this->m_panels[idx]->m_autohide;
    }
    return rv;
}

bool FlatSplitterPanel::FlatSplitterPanelHost::is_visible(HWND wnd) const
{
    bool rv = false;

    if (!m_this->get_host()->is_visible(m_this->get_wnd())) {
        rv = false;
    } else {
        unsigned idx = 0;
        if (m_this->m_panels.find_by_wnd_child(wnd, idx)) {
            rv = !m_this->m_panels[idx]->m_hidden;
        }
    }
    return rv;
}

bool FlatSplitterPanel::FlatSplitterPanelHost::override_status_text_create(
    service_ptr_t<ui_status_text_override>& p_out)
{
    return m_this->get_host()->override_status_text_create(p_out);
}

bool FlatSplitterPanel::FlatSplitterPanelHost::request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height)
{
    bool rv = false;
    if (!(flags & (get_orientation() == horizontal ? ui_extension::size_height : uie::size_width))) {
        if (flags & (get_orientation() == vertical ? ui_extension::size_height : uie::size_width)) {
            unsigned index;
            if (m_this->m_panels.find_by_wnd_child(wnd, index)) {
                int delta = (get_orientation() == horizontal ? width : height) - m_this->m_panels[index]->m_size;
                m_this->override_size(index, delta);
                rv = true;
            }
        } else
            rv = true;
    }
    return rv;
}

unsigned FlatSplitterPanel::FlatSplitterPanelHost::is_resize_supported(HWND wnd) const
{
    return get_orientation() == vertical ? ui_extension::size_height : uie::size_width;
}

Orientation FlatSplitterPanel::FlatSplitterPanelHost::get_orientation() const
{
    return m_this.is_valid() ? m_this->get_orientation() : vertical;
}

void FlatSplitterPanel::FlatSplitterPanelHost::on_size_limit_change(HWND wnd, unsigned flags)
{
    unsigned index;
    if (m_this->m_panels.find_by_wnd_child(wnd, index)) {
        std::shared_ptr<Panel> p_ext = m_this->m_panels[index];
        MINMAXINFO mmi{};
        mmi.ptMaxTrackSize.x = MAXLONG;
        mmi.ptMaxTrackSize.y = MAXLONG;
        SendMessage(wnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
        p_ext->m_size_limits.min_width = std::min(mmi.ptMinTrackSize.x, static_cast<long>(MAXSHORT));
        p_ext->m_size_limits.min_height = std::min(mmi.ptMinTrackSize.y, static_cast<long>(MAXSHORT));
        p_ext->m_size_limits.max_height = std::min(mmi.ptMaxTrackSize.y, static_cast<long>(MAXSHORT));
        p_ext->m_size_limits.max_width = std::min(mmi.ptMaxTrackSize.x, static_cast<long>(MAXSHORT));
        pfc::string8 name;
        p_ext->m_child->get_name(name);
        // console::formatter() << "change: name: " << name << " min width: " << (t_int32)mmi.ptMinTrackSize.x;

        m_this->on_size_changed();

        m_this->get_host()->on_size_limit_change(m_this->get_wnd(), flags);
    }
}

void FlatSplitterPanel::FlatSplitterPanelHost::get_children(pfc::list_base_t<window::ptr>& p_out)
{
    if (m_this.is_valid()) {
        t_size count = m_this->m_panels.get_count();
        for (t_size i = 0; i < count; i++) {
            if (m_this->m_panels[i]->m_child.is_valid())
                p_out.add_item(m_this->m_panels[i]->m_child);
        }
    }
}

bool FlatSplitterPanel::FlatSplitterPanelHost::get_keyboard_shortcuts_enabled() const
{
    return m_this->get_host()->get_keyboard_shortcuts_enabled();
}

const GUID& FlatSplitterPanel::FlatSplitterPanelHost::get_host_guid() const
{
    // {FC0ED6EF-DCA2-4679-B7FE-48162DE321FC}
    static const GUID rv = {0xfc0ed6ef, 0xdca2, 0x4679, {0xb7, 0xfe, 0x48, 0x16, 0x2d, 0xe3, 0x21, 0xfc}};
    return rv;
}

void FlatSplitterPanel::g_on_size_change()
{
    for (t_size index = 0; index < g_instances.get_count(); index++) {
        g_instances[index]->on_size_changed();
    }
}

}
