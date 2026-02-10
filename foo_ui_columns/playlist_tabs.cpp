#include "pch.h"
#include "playlist_tabs.h"

#include "dark_mode_spin.h"
#include "dark_mode_tabs.h"
#include "playlist_manager_utils.h"

namespace cui::panels::playlist_tabs {

cfg_guid cfg_default_playlist(
    GUID{0x68527c89, 0xb0f7, 0xf653, {0x00, 0x53, 0x8c, 0xeb, 0x47, 0xe7, 0xa3, 0xb3}}, guid_playlist_view_v2);

// {942C36A4-4E28-4cea-9644-F223C9A838EC}
const GUID g_guid_playlist_switcher_tabs_font
    = {0x942c36a4, 0x4e28, 0x4cea, {0x96, 0x44, 0xf2, 0x23, 0xc9, 0xa8, 0x38, 0xec}};

void remove_playlist_helper(size_t index)
{
    const auto api = playlist_manager::get();
    if (index == api->get_active_playlist()) {
        if (index && index + 1 == api->get_playlist_count())
            api->set_active_playlist(index - 1);
    }
    api->remove_playlist_switch(index);
}

ui_extension::window_factory<PlaylistTabs> blah;
ui_extension::window_host_factory<PlaylistTabs::WindowHost> g_tab_host;

HFONT PlaylistTabs::g_font = nullptr;

void PlaylistTabs::get_supported_panels(
    const pfc::list_base_const_t<window::ptr>& p_windows, bit_array_var& p_mask_unsupported)
{
    const uie::window_host::ptr host = m_host.is_valid() ? m_host : fb2k::service_new<WindowHost>(this);
    const size_t count = p_windows.get_count();

    for (size_t i = 0; i < count; i++)
        p_mask_unsupported.set(i, !p_windows[i]->is_available(host));
}

bool PlaylistTabs::is_point_ours(HWND wnd_point, const POINT& pt_screen, pfc::list_base_t<window::ptr>& p_hierarchy)
{
    if (wnd_point == get_wnd() || IsChild(get_wnd(), wnd_point)) {
        if (wnd_point == get_wnd() || wnd_point == wnd_tabs) {
            p_hierarchy.add_item(this);
            return true;
        }
        {
            uie::splitter_window_v2_ptr sptr;
            if (m_child.is_valid()) {
                if (m_child->service_query_t(sptr)) {
                    pfc::list_t<window::ptr> temp;
                    temp.add_item(this);
                    if (sptr->is_point_ours(wnd_point, pt_screen, temp)) {
                        p_hierarchy.add_items(temp);
                        return true;
                    }
                } else if (wnd_point == m_child_wnd || IsChild(m_child_wnd, wnd_point)) {
                    p_hierarchy.add_item(this);
                    p_hierarchy.add_item(m_child);
                    return true;
                }
            }
        }
    }
    return false;
}

void PlaylistTabs::on_font_change()
{
    if (g_font != nullptr) {
        const auto count = list_wnd.get_count();
        for (unsigned n = 0; n < count; n++) {
            HWND wnd = list_wnd[n]->wnd_tabs;
            if (wnd)
                SendMessage(wnd, WM_SETFONT, (WPARAM)0, MAKELPARAM(0, 0));
        }
        DeleteObject(g_font);
    }

    g_font = fb2k::std_api_get<fonts::manager>()->get_font(g_guid_playlist_switcher_tabs_font);

    const auto count = list_wnd.get_count();
    for (unsigned n = 0; n < count; n++) {
        HWND wnd = list_wnd[n]->wnd_tabs;
        if (wnd) {
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_font, MAKELPARAM(1, 0));
            list_wnd[n]->on_size();
            list_wnd[n]->on_child_position_change();
        }
    }
}

pfc::ptr_list_t<PlaylistTabs> PlaylistTabs::list_wnd;

void PlaylistTabs::kill_switch_timer()
{
    if (m_switch_timer) {
        m_switch_timer = false;
        KillTimer(get_wnd(), SWITCH_TIMER_ID);
    }
}

void PlaylistTabs::switch_to_playlist_delayed2(unsigned idx)
{
    // if have a timer already and idxs re same dont bother
    if (!(m_switch_timer && idx == m_switch_playlist)) {
        if (m_switch_timer)
            kill_switch_timer();
        m_switch_playlist = idx;
        m_playlist_switched = false;
        m_switch_timer = (SetTimer(get_wnd(), SWITCH_TIMER_ID, cfg_autoswitch_delay, nullptr) != 0);
    }
}

PlaylistTabs::~PlaylistTabs() = default;

LRESULT WINAPI PlaylistTabs::main_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) noexcept
{
    auto p_this = reinterpret_cast<PlaylistTabs*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
    return p_this ? p_this->hook(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
}

void PlaylistTabs::create_child()
{
    destroy_child();
    if (m_host.is_valid()) {
        create_by_guid(m_child_guid, m_child);
        if (m_child.is_valid()) {
            try {
                abort_callback_dummy abortCallback;
                m_child->set_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size(), abortCallback);
            } catch (const exception_io&) {
            }
            m_child_wnd
                = m_child->create_or_transfer_window(m_host_wnd, ui_extension::window_host_ptr(m_host.get_ptr()));
            if (m_child_wnd) {
                SetWindowPos(m_child_wnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
        }
    }
    reset_size_limits();
    on_size();
    if (IsWindowVisible(get_wnd())) {
        get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
        ShowWindow(m_child_wnd, SW_SHOWNORMAL);
    }
}

void PlaylistTabs::destroy_child()
{
    if (m_child.is_valid()) {
        m_child->destroy_window();
        m_child_wnd = nullptr;
        m_child.release();
        reset_size_limits();
        if (IsWindowVisible(get_wnd()))
            get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
    }
}

bool PlaylistTabs::create_tabs()
{
    bool rv = false;
    bool force_close = false;

    const auto playlist_api = playlist_manager::get();

    if (cfg_pl_autohide) {
        force_close = (playlist_api->get_playlist_count() <= 1);
    }

    if (wnd_tabs && force_close) {
        DestroyWindow(wnd_tabs);
        wnd_tabs = nullptr;
        rv = true;
    } else if (!wnd_tabs && !force_close) {
        const auto t = playlist_api->get_playlist_count();
        pfc::string8 temp;

        int x = 0;
        int y = 0;
        int cx = 0; // bah
        int cy = 0;

        wnd_tabs = CreateWindowEx(0, WC_TABCONTROL, _T("Playlist switcher"),
            WS_CHILD | WS_TABSTOP | TCS_HOTTRACK | TCS_TABS | (t > 1 ? TCS_MULTILINE : 0) | (true ? WS_VISIBLE : 0)
                | WS_CLIPCHILDREN,
            x, y, cx, cy, m_host_wnd, HMENU(5002), core_api::get_my_instance(), nullptr);

        if (wnd_tabs) {
            SetWindowLongPtr(wnd_tabs, GWLP_USERDATA, (LPARAM)(this));

            if (g_font) {
                SendMessage(wnd_tabs, WM_SETFONT, (WPARAM)g_font, MAKELPARAM(0, 0));
            } else
                on_font_change();

            SetWindowPos(wnd_tabs, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

            tabproc = (WNDPROC)SetWindowLongPtr(wnd_tabs, GWLP_WNDPROC, (LPARAM)main_hook);

            pfc::string8 temp2;
            for (size_t i = 0; i < t; i++) {
                playlist_api->playlist_get_name(i, temp);
                uTabCtrl_InsertItemText(wnd_tabs, gsl::narrow<int>(i), temp);
            }

            TabCtrl_SetCurSel(wnd_tabs, playlist_api->get_active_playlist());
            set_styles();

            RECT rc;
            GetWindowRect(wnd_tabs, &rc);
            adjust_rect(FALSE, &rc);
            m_child_top = rc.top;

            rv = true;
        }
    }
    if (rv) {
        reset_size_limits();
        get_host()->on_size_limit_change(get_wnd(),
            uie::size_limit_maximum_height | uie::size_limit_maximum_width | uie::size_limit_minimum_height
                | uie::size_limit_minimum_width);
        on_size();
    }
    return rv;
}

const GUID& PlaylistTabs::get_extension_guid() const
{
    return extension_guid;
}

void PlaylistTabs::get_name(pfc::string_base& out) const
{
    out.set_string("Playlist tabs");
}
void PlaylistTabs::get_category(pfc::string_base& out) const
{
    out.set_string("Splitters");
}
bool PlaylistTabs::get_short_name(pfc::string_base& out) const
{
    out.set_string("Playlist tabs");
    return true;
}

// {ABB72D0D-DBF0-4bba-8C68-3357EBE07A4D}
const GUID PlaylistTabs::extension_guid
    = {0xabb72d0d, 0xdbf0, 0x4bba, {0x8c, 0x68, 0x33, 0x57, 0xeb, 0xe0, 0x7a, 0x4d}};

LRESULT WINAPI PlaylistTabs::hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_ERASEBKGND:
        return FALSE;
    case WM_PAINT:
        if (colours::is_dark_mode_active())
            dark::handle_tab_control_paint(wnd);
        else
            uih::paint_subclassed_window_with_buffering(wnd, tabproc);
        return 0;
    case WM_PARENTNOTIFY:
        switch (LOWORD(wp)) {
        case WM_CREATE: {
            const auto child_window = reinterpret_cast<HWND>(lp);
            std::array<wchar_t, 128> class_name{};
            GetClassName(child_window, class_name.data(), gsl::narrow<int>(class_name.size()));

            if (!wcsncmp(UPDOWN_CLASSW, class_name.data(), class_name.size())) {
                m_up_down_control_wnd = child_window;
                uih::subclass_window_and_paint_with_buffering(child_window);
                set_up_down_window_theme();
            }
            break;
        }
        case WM_DESTROY:
            if (m_up_down_control_wnd == reinterpret_cast<HWND>(lp))
                m_up_down_control_wnd = nullptr;
            break;
        }
        break;
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case WM_KEYDOWN: {
        if (wp != VK_LEFT && wp != VK_RIGHT && get_host()->get_keyboard_shortcuts_enabled()
            && g_process_keydown_keyboard_shortcuts(wp))
            return 0;
        if (wp == VK_TAB) {
            g_on_tab(wnd);
        }
        SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
        break;
    }
    case WM_SYSKEYDOWN:
        if (get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp))
            return 0;
        break;
    case WM_LBUTTONDOWN: {
        {
            TCHITTESTINFO hittest;
            hittest.pt.x = GET_X_LPARAM(lp);
            hittest.pt.y = GET_Y_LPARAM(lp);
            int idx = TabCtrl_HitTest(wnd_tabs, &hittest);
            if (idx >= 0) {
                const auto playlist_api = playlist_manager::get();
                if (cfg_drag_pl) {
                    SetCapture(wnd);
                    m_dragging = true;
                    m_dragging_idx = idx;
                    TabCtrl_GetItemRect(wnd, idx, &m_dragging_rect);
                }
            }
        }
        break;
    }
    case WM_MOUSEMOVE:
        if (m_dragging && (wp & MK_LBUTTON)) {
            TCHITTESTINFO hittest;
            hittest.pt.x = GET_X_LPARAM(lp);
            hittest.pt.y = GET_Y_LPARAM(lp);
            const auto hit_test_index = TabCtrl_HitTest(wnd_tabs, &hittest);
            if (hit_test_index >= 0 && !PtInRect(&m_dragging_rect, hittest.pt)) {
                const auto cur_idx = m_dragging_idx;
                const auto playlist_api = playlist_manager::get();
                const auto count = playlist_api->get_playlist_count();
                const auto target_index = gsl::narrow<size_t>(hit_test_index);

                auto n = cur_idx;
                order_helper order(count);
                if (n < target_index) {
                    while (n < target_index && n < count) {
                        order.swap(n, n + 1);
                        n++;
                    }
                } else if (n > target_index) {
                    while (n > target_index && n > 0) {
                        order.swap(n, n - 1);
                        n--;
                    }
                }
                if (n != cur_idx) {
                    TabCtrl_GetItemRect(wnd, gsl::narrow<int>(n), &m_dragging_rect);
                    playlist_api->reorder(order.get_ptr(), count);
                    m_dragging_idx = n;
                }
            }
        } else
            m_dragging = false;

        break;
    case WM_LBUTTONUP:
        if (m_dragging) {
            m_dragging = false;
            ReleaseCapture();
        }
        break;
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONUP: {
        TCHITTESTINFO hittest{};
        hittest.pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        const auto index = TabCtrl_HitTest(wnd_tabs, &hittest);
        const auto playlist_api = playlist_manager::get();
        if (index >= 0) {
            if (config::cfg_playlist_tabs_middle_click && msg == WM_MBUTTONUP) {
                remove_playlist_helper(index);
            }
            if (cfg_plm_rename && msg == WM_LBUTTONDBLCLK) {
                playlist_manager_utils::rename_playlist(index, get_wnd());
            }
        } else {
            const auto new_idx = playlist_api->create_playlist(
                pfc::string8("Untitled"), pfc_infinite, playlist_api->get_playlist_count());
            playlist_api->set_active_playlist(new_idx);
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        if ((GetWindowLongPtr(wnd, GWL_STYLE) & TCS_MULTILINE) != 0)
            return 0;

        if (!m_up_down_control_wnd || !IsWindowVisible(m_up_down_control_wnd))
            return 0;

        int min{};
        int max{};
        SendMessage(
            m_up_down_control_wnd, UDM_GETRANGE32, reinterpret_cast<WPARAM>(&min), reinterpret_cast<LPARAM>(&max));

        const auto index = gsl::narrow<int>(SendMessage(m_up_down_control_wnd, UDM_GETPOS32, NULL, NULL));

        if (max == 0)
            return 0;

        const int wheel_delta = GET_WHEEL_DELTA_WPARAM(wp);

        m_mousewheel_delta += wheel_delta;

        POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        ScreenToClient(wnd, &pt);

        if (abs(m_mousewheel_delta) < WHEEL_DELTA)
            return 0;

        if (m_mousewheel_delta > 0 && index > min) {
            SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, index - 1), NULL);
            SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
            SendMessage(m_up_down_control_wnd, UDM_SETPOS32, NULL, index - 1);
            SendMessage(wnd, WM_MOUSEMOVE, GET_KEYSTATE_WPARAM(wp), POINTTOPOINTS(pt));
        } else if (m_mousewheel_delta < 0 && index + 1 <= max) {
            SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, index + 1), NULL);
            SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
            SendMessage(m_up_down_control_wnd, UDM_SETPOS32, NULL, index + 1);
            SendMessage(wnd, WM_MOUSEMOVE, GET_KEYSTATE_WPARAM(wp), POINTTOPOINTS(pt));
        }

        m_mousewheel_delta = 0;
        return 0;
    }
    }
    return CallWindowProc(tabproc, wnd, msg, wp, lp);
}

void PlaylistTabs::on_child_position_change()
{
    reset_size_limits();
    get_host()->on_size_limit_change(get_wnd(),
        uie::size_limit_maximum_height | uie::size_limit_maximum_width | uie::size_limit_minimum_height
            | uie::size_limit_minimum_width);
}

void PlaylistTabs::set_up_down_window_theme() const
{
    if (!m_up_down_control_wnd)
        return;

    const auto is_dark = colours::is_dark_mode_active();

    if (is_dark)
        dark::spin::add_window(m_up_down_control_wnd);
    else
        dark::spin::remove_window(m_up_down_control_wnd);

    SetWindowTheme(m_up_down_control_wnd, is_dark ? L"DarkMode_Explorer" : nullptr, nullptr);
}

void PlaylistTabs::refresh_child_data(abort_callback& aborter) const
{
    if (m_child.is_valid())
        m_child_data = m_child->get_config_as_array(aborter);
}

void PlaylistTabs::get_config(stream_writer* out, abort_callback& p_abort) const
{
    out->write_lendian_t(m_child_guid, p_abort);

    refresh_child_data();
    out->write_lendian_t(gsl::narrow<uint32_t>(m_child_data.get_size()), p_abort);
    out->write(m_child_data.get_ptr(), m_child_data.get_size(), p_abort);
}

void PlaylistTabs::set_config(stream_reader* config, size_t p_size, abort_callback& p_abort)
{
    if (p_size) {
        config->read_lendian_t(m_child_guid, p_abort);
        unsigned size = 0;
        config->read_lendian_t(size, p_abort);
        m_child_data.set_size(0);
        m_child_data.set_size(size);
        config->read(m_child_data.get_ptr(), size, p_abort);
    }
}

void PlaylistTabs::export_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    abort_callback_dummy abortCallback;
    p_writer->write_lendian_t(m_child_guid, p_abort);
    uie::window_ptr ptr = m_child;
    if (!ptr.is_valid() && m_child_guid != pfc::guid_null) {
        if (create_by_guid(m_child_guid, ptr)) {
            try {
                ptr->set_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size(), abortCallback);
            } catch (const exception_io&) {
            }
        } else
            throw fcl::exception_missing_panel();
    }
    pfc::array_t<uint8_t> data;
    stream_writer_memblock_ref w(data);
    if (ptr.is_valid())
        ptr->export_config(&w, abortCallback);
    p_writer->write_lendian_t(gsl::narrow<uint32_t>(data.get_size()), p_abort);
    p_writer->write(data.get_ptr(), data.get_size(), p_abort);
}

void PlaylistTabs::import_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    if (p_size) {
        p_reader->read_lendian_t(m_child_guid, p_abort);
        unsigned size = 0;
        p_reader->read_lendian_t(size, p_abort);
        m_child_data.set_size(0);

        pfc::array_t<uint8_t> data;
        data.set_size(size);
        p_reader->read(data.get_ptr(), size, p_abort);
        uie::window_ptr ptr;
        if (create_by_guid(m_child_guid, ptr)) {
            try {
                ptr->import_config_from_ptr(data.get_ptr(), data.get_size(), p_abort);
            } catch (const exception_io&) {
            }
            ptr->get_config_to_array(m_child_data, p_abort, true);
        }
    }
}

uie::splitter_item_t* PlaylistTabs::get_panel(size_t index) const
{
    auto ptr = new uie::splitter_item_simple_t;
    ptr->set_panel_guid(m_child_guid);

    ptr->set_panel_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size());

    if (index == 0 && m_child_guid != pfc::guid_null) {
        if (m_child_wnd && m_child.is_valid())
            ptr->set_window_ptr(m_child);
    }
    return ptr;
}

size_t PlaylistTabs::get_maximum_panel_count() const
{
    return 1;
}

size_t PlaylistTabs::get_panel_count() const
{
    return m_child_guid != pfc::guid_null ? 1 : 0;
}

void PlaylistTabs::replace_panel(size_t index, const uie::splitter_item_t* p_item)
{
    if (index == 0 && m_child_guid != pfc::guid_null) {
        if (initialised)
            destroy_child();

        m_child_data.set_size(0);
        stream_writer_memblock_ref w(m_child_data);
        p_item->get_panel_config(&w);
        m_child_guid = p_item->get_panel_guid();

        if (initialised)
            create_child();
    }
}

void PlaylistTabs::remove_panel(size_t index)
{
    if (index == 0 && m_child_guid != pfc::guid_null) {
        if (initialised)
            destroy_child();

        m_child_guid = pfc::guid_null;
        m_child_data.set_size(0);

        if (initialised)
            create_child();
    }
}

void PlaylistTabs::insert_panel(size_t index, const uie::splitter_item_t* p_item)
{
    if (index == 0 && m_child_guid == pfc::guid_null) {
        if (initialised)
            destroy_child();

        m_child_data.set_size(0);
        stream_writer_memblock_ref w(m_child_data);
        p_item->get_panel_config(&w);
        m_child_guid = p_item->get_panel_guid();

        if (initialised)
            create_child();
    }
}

unsigned PlaylistTabs::get_type() const
{
    return ui_extension::type_layout | uie::type_splitter;
}

void PlaylistTabs::on_size(unsigned cx, unsigned cy)
{
    if (wnd_tabs) {
        SetWindowPos(wnd_tabs, nullptr, 0, 0, cx, cy, SWP_NOZORDER);
        RECT rc = {0, 0, gsl::narrow<long>(cx), gsl::narrow<long>(cy)};
        adjust_rect(FALSE, &rc);
        if (m_child_wnd)
            SetWindowPos(m_child_wnd, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
        unsigned old_top = m_child_top; // prevent braindead (partial) recursion
        m_child_top = rc.top;
        if (rc.top != old_top) {
            // Avoid causing the host to trigger a sequence of resizes within another sequence of resizes.
            PostMessage(get_wnd(), MSG_RESET_SIZE_LIMITS, 0, 0);
        }
    } else if (m_child_wnd) {
        SetWindowPos(m_child_wnd, nullptr, 0, 0, cx, cy, SWP_NOZORDER);
    }
}

void PlaylistTabs::on_size()
{
    RECT rc;
    GetWindowRect(get_wnd(), &rc);
    on_size(rc.right - rc.left, rc.bottom - rc.top);
}

void PlaylistTabs::adjust_rect(bool b_larger, RECT* rc)
{
    if (b_larger) {
        RECT rc_child = *rc;
        TabCtrl_AdjustRect(wnd_tabs, FALSE, &rc_child);
        rc_child.top = rc->top + 2;
        TabCtrl_AdjustRect(wnd_tabs, TRUE, &rc_child);
        *rc = rc_child;
    } else {
        RECT rc_tabs = *rc;
        TabCtrl_AdjustRect(wnd_tabs, FALSE, &rc_tabs);
        rc->top = rc_tabs.top - 2;
    }
}

void PlaylistTabs::reset_size_limits()
{
    constexpr auto max_value = static_cast<long>(USHRT_MAX);
    memset(&m_mmi, 0, sizeof(m_mmi));
    if (m_child_wnd) {
        m_mmi.ptMinTrackSize.x = 0;
        m_mmi.ptMinTrackSize.y = 0;
        m_mmi.ptMaxTrackSize.x = max_value;
        m_mmi.ptMaxTrackSize.y = max_value;
        SendMessage(m_child_wnd, WM_GETMINMAXINFO, 0, (LPARAM)&m_mmi);
        if (m_mmi.ptMinTrackSize.x != 0 || m_mmi.ptMinTrackSize.y != 0 || m_mmi.ptMaxTrackSize.x < max_value
            || m_mmi.ptMaxTrackSize.y < max_value) {
            RECT rc_min = {0, 0, m_mmi.ptMinTrackSize.x, m_mmi.ptMinTrackSize.y};
            RECT rc_max
                = {0, 0, std::min(m_mmi.ptMaxTrackSize.x, max_value), std::min(m_mmi.ptMaxTrackSize.y, max_value)};
            if (wnd_tabs) {
                adjust_rect(TRUE, &rc_min);
                adjust_rect(TRUE, &rc_max);
            }
            m_mmi.ptMinTrackSize.x = rc_min.right - rc_min.left;
            m_mmi.ptMinTrackSize.y = rc_min.bottom - rc_min.top;
            m_mmi.ptMaxTrackSize.x = rc_max.right - rc_max.left;
            m_mmi.ptMaxTrackSize.y = rc_max.bottom - rc_max.top;
        } else {
            if (wnd_tabs) {
                RECT rc;
                GetWindowRect(wnd_tabs, &rc);
                MapWindowPoints(HWND_DESKTOP, get_wnd(), (LPPOINT)&rc, 2);
                RECT rc_child = rc;
                adjust_rect(FALSE, &rc_child);
                m_mmi.ptMinTrackSize.x = rc_child.left - rc.left + rc.right - rc_child.right;
                m_mmi.ptMinTrackSize.y = rc_child.top - rc.top + rc.bottom - rc_child.bottom;
            }
            m_mmi.ptMaxTrackSize.x = MAXLONG;
            m_mmi.ptMaxTrackSize.y = MAXLONG;
        }
    } else {
        if (wnd_tabs) {
            RECT rc_tabs;
            GetWindowRect(wnd_tabs, &rc_tabs);
            MapWindowPoints(HWND_DESKTOP, get_wnd(), (LPPOINT)&rc_tabs, 2);
            RECT rc_child = rc_tabs;
            adjust_rect(FALSE, &rc_child);
            m_mmi.ptMinTrackSize.x = rc_child.left - rc_tabs.left + rc_tabs.right - rc_child.right;
            m_mmi.ptMinTrackSize.y = rc_child.top - rc_tabs.top + rc_tabs.bottom - rc_child.bottom;
            m_mmi.ptMaxTrackSize.y = m_mmi.ptMinTrackSize.y;
            m_mmi.ptMaxTrackSize.x = MAXLONG;
        } else {
            m_mmi.ptMaxTrackSize.x = MAXLONG;
            m_mmi.ptMaxTrackSize.y = 0;
        }
    }
}

void PlaylistTabs::set_styles(bool visible)
{
    if (wnd_tabs) {
        long flags = WS_CHILD | TCS_HOTTRACK | TCS_TABS
            | ((cfg_tabs_multiline && (TabCtrl_GetItemCount(wnd_tabs) > 1)) ? TCS_MULTILINE | TCS_RIGHTJUSTIFY
                                                                            : TCS_SINGLELINE)
            | (visible ? WS_VISIBLE : 0) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP;

        if (GetWindowLongPtr(wnd_tabs, GWL_STYLE) != flags)
            SetWindowLongPtr(wnd_tabs, GWL_STYLE, flags);
    }
}

void PlaylistTabs::on_playlist_renamed(size_t p_index, const char* p_new_name, size_t p_new_name_len) noexcept
{
    if (wnd_tabs) {
        uTabCtrl_InsertItemText(wnd_tabs, gsl::narrow<int>(p_index), pfc::string8(p_new_name, p_new_name_len), false);
        if (cfg_tabs_multiline)
            on_size();
    }
}

void PlaylistTabs::on_playlists_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) noexcept
{
    bool need_move = false;

    if (create_tabs())
        need_move = true;
    else if (wnd_tabs) {
        auto n = p_old_count;
        for (; n > 0; n--) {
            if (p_mask[n - 1])
                TabCtrl_DeleteItem(wnd_tabs, n - 1);
        }
        set_styles();
        if (cfg_tabs_multiline)
            on_size();
        // RedrawWindow(wnd_tabs, NULL, NULL, RDW_ERASE|RDW_INVALIDATE);
    }

    if (need_move)
        on_size();
}

void PlaylistTabs::on_playlist_created(size_t p_index, const char* p_name, size_t p_name_len) noexcept
{
    if (wnd_tabs) {
        uTabCtrl_InsertItemText(wnd_tabs, gsl::narrow<int>(p_index), pfc::string8(p_name, p_name_len));
        set_styles();
        if (cfg_tabs_multiline)
            on_size();
    } else if (create_tabs())
        on_size();
}

void PlaylistTabs::on_playlists_reorder(const size_t* p_order, size_t p_count) noexcept
{
    if (wnd_tabs) {
        const auto playlist_api = playlist_manager::get();
        const auto sel = playlist_api->get_active_playlist();

        for (size_t n = 0; n < p_count; n++) {
            if (n != p_order[n]) {
                pfc::string8 temp;
                pfc::string8 temp2;
                playlist_api->playlist_get_name(n, temp);

                uTabCtrl_InsertItemText(wnd_tabs, gsl::narrow<int>(n), temp, false);
            }
        }
        TabCtrl_SetCurSel(wnd_tabs, sel);
    }
}

void PlaylistTabs::on_playlist_activate(size_t p_old, size_t p_new) noexcept
{
    if (wnd_tabs) {
        TabCtrl_SetCurSel(wnd_tabs, p_new);
    }
}

void g_on_autohide_tabs_change()
{
    const auto count = PlaylistTabs::list_wnd.get_count();
    for (size_t n = 0; n < count; n++) {
        PlaylistTabs::list_wnd[n]->create_tabs();
    }
}

void g_on_multiline_tabs_change()
{
    const auto count = PlaylistTabs::list_wnd.get_count();
    for (size_t n = 0; n < count; n++) {
        PlaylistTabs* p_tabs = PlaylistTabs::list_wnd[n];
        p_tabs->set_styles();
        p_tabs->on_size();
    }
}

void g_on_tabs_font_change()
{
    PlaylistTabs::on_font_change();
}

class PlaylistTabsFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_playlist_switcher_tabs_font; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Playlist tabs"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_labels; }

    void on_font_changed() const override { PlaylistTabs::on_font_change(); }
};

PlaylistTabsFontClient::factory<PlaylistTabsFontClient> g_font_client_switcher_tabs;

void PlaylistTabs::WindowHost::relinquish_ownership(HWND wnd)
{
    m_this->m_child_wnd = nullptr;
    m_this->m_host.release();
    m_this->m_child.release();
    m_this->m_child_guid = pfc::guid_null;
    m_this->m_child_data.set_size(0);
    m_this->reset_size_limits();
    m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);
}

bool PlaylistTabs::WindowHost::override_status_text_create(service_ptr_t<ui_status_text_override>& p_out)
{
    const auto api = ui_control::get();
    return m_this->get_host()->override_status_text_create(p_out);
}

const GUID& PlaylistTabs::WindowHost::get_host_guid() const
{
    // {20789B52-4998-43ae-9B20-CCFD3BFBEEBD}
    static const GUID guid = {0x20789b52, 0x4998, 0x43ae, {0x9b, 0x20, 0xcc, 0xfd, 0x3b, 0xfb, 0xee, 0xbd}};
    return guid;
}

void PlaylistTabs::WindowHost::on_size_limit_change(HWND wnd, unsigned flags)
{
    m_this->on_child_position_change();
}

bool PlaylistTabs::WindowHost::get_show_shortcuts() const
{
    return m_this->get_host()->get_keyboard_shortcuts_enabled();
}

bool PlaylistTabs::WindowHost::get_keyboard_shortcuts_enabled() const
{
    return m_this->get_host()->get_keyboard_shortcuts_enabled();
}

bool PlaylistTabs::WindowHost::set_window_visibility(HWND wnd, bool visibility)
{
    return false;
}

bool PlaylistTabs::WindowHost::is_visibility_modifiable(HWND wnd, bool desired_visibility) const
{
    return false;
}

bool PlaylistTabs::WindowHost::is_visible(HWND wnd) const
{
    return true;
}

bool PlaylistTabs::WindowHost::request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height)
{
    if (flags == ui_extension::size_height && is_resize_supported(wnd)) {
        if (m_this->wnd_tabs) {
            RECT rc;
            GetWindowRect(m_this->m_child_wnd, &rc);
            MapWindowPoints(HWND_DESKTOP, m_this->get_wnd(), (LPPOINT)&rc, 2);
            rc.bottom = rc.top + height;
            m_this->adjust_rect(TRUE, &rc);
            // We would expect rc.top and rc.left to be 0.
            return m_this->get_host()->request_resize(m_this->get_wnd(), flags, rc.right, rc.bottom);
        }
        return m_this->get_host()->request_resize(m_this->get_wnd(), flags, width, height);
    }
    return false;
}

unsigned PlaylistTabs::WindowHost::is_resize_supported(HWND wnd) const
{
    // We won't support ui_extension::size_width since we can't reliably detect multiline tab shit
    return (m_this->get_host()->is_resize_supported(m_this->get_wnd()) & ui_extension::size_height);
}

} // namespace cui::panels::playlist_tabs
