#include "pch.h"

#include "dark_mode.h"
#include "splitter.h"

#define HOST_AUTOHIDE_TIMER_ID 672

namespace cui::panels::splitter {

void FlatSplitterPanel::Panel::PanelContainer::reopen_theme()
{
    close_theme();

    if (IsThemeActive() && IsAppThemed())
        m_theme.reset(OpenThemeData(m_wnd, L"Rebar"));
}

void FlatSplitterPanel::Panel::PanelContainer::close_theme()
{
    m_theme.reset();
}

bool FlatSplitterPanel::Panel::PanelContainer::test_autohide_window(HWND wnd)
{
    return IsChild(m_wnd, wnd) || wnd == m_wnd || wnd == m_this->get_wnd();
}

void FlatSplitterPanel::Panel::PanelContainer::on_hooked_message(WPARAM msg, const MSLLHOOKSTRUCT& mllhs)
{
    if (msg == WM_MOUSEMOVE && m_this.is_valid() && MonitorFromPoint(mllhs.pt, MONITOR_DEFAULTTONULL)) {
        const auto iter = std::ranges::find(m_this->m_panels, m_panel->shared_from_this());
        if (iter == m_this->m_panels.end())
            return;

        HWND wnd_capture = GetCapture();
        HWND wnd_pt = WindowFromPoint(mllhs.pt);
        POINT pt = mllhs.pt;
        ScreenToClient(m_this->get_wnd(), &pt);

        // We need to test wnd_pt when wnd_capture is non-NULL because during drag-and-drop operations wnd_capture
        // is an OLE window. Alternative fixes are checking the window with the keyboard focus (has side-effects)
        // and checking the window class of wnd_capture (a hack). (For future reference: the class of wnd_capture
        // during drag-and-drop operations is CLIPBRDWNDCLASS.)
        if (!(wnd_capture && test_autohide_window(wnd_capture)) && !(wnd_pt && test_autohide_window(wnd_pt))
            && !m_this->test_divider_pt(pt, std::distance(m_this->m_panels.begin(), iter))) {
            if (!m_timer_active)
                PostMessage(m_wnd, MSG_AUTOHIDE_END, 0, 0);
        } else {
            if (m_timer_active) {
                KillTimer(m_wnd, HOST_AUTOHIDE_TIMER_ID);
                m_timer_active = false;
            }
        }
    }
}

LRESULT FlatSplitterPanel::Panel::PanelContainer::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_NCCREATE:
        m_wnd = wnd;
        break;
    case WM_CREATE:
        reopen_theme();
        m_dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>(
            [wnd] { RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE); });
        break;
    case WM_THEMECHANGED:
        reopen_theme();
        RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
        break;
    case WM_DESTROY:
        m_dark_mode_notifier.reset();
        close_theme();
        m_this.release();
        break;
    case WM_NCDESTROY:
        if (m_hook_active)
            fbh::LowLevelMouseHookManager::s_get_instance().deregister_hook(this);
        m_wnd = nullptr;
        break;
    case MSG_AUTOHIDE_END:
        if (!cfg_sidebar_hide_delay) {
            if (m_this.is_valid()) {
                m_panel->m_hidden = true;
                m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);
                m_this->on_size_changed();
                if (m_hook_active) {
                    fbh::LowLevelMouseHookManager::s_get_instance().deregister_hook(this);
                    m_hook_active = false;
                }
            }
        } else {
            if (!m_timer_active) {
                m_timer_active = true;
                SetTimer(wnd, HOST_AUTOHIDE_TIMER_ID, cfg_sidebar_hide_delay, nullptr);
            }
        }
        return 0;
    case WM_TIMER: {
        if (wp == HOST_AUTOHIDE_TIMER_ID) {
            if (m_this.is_valid()) {
                m_panel->m_hidden = true;
                m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);
                m_this->on_size_changed();
                if (m_hook_active) {
                    fbh::LowLevelMouseHookManager::s_get_instance().deregister_hook(this);
                    m_hook_active = false;
                }
            }
            KillTimer(wnd, wp);
            m_timer_active = false;
        }
    } break;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        const auto paint_dc = wil::BeginPaint(wnd, &ps);
        const uih::BufferedPaint buffered_dc(paint_dc.get(), ps.rcPaint);

        uie::win32::paint_background_using_parent(wnd, buffered_dc.get(), false);

        if (!m_this.is_valid())
            return 0;

        const auto iter = m_this->find_panel_by_container_wnd(wnd);

        if (iter == m_this->m_panels.end())
            return 0;

        const auto panel = *iter;

        if (!panel->m_show_caption)
            return 0;

        const auto orientation = panel->m_caption_orientation;

        RECT rc_client{};
        GetClientRect(wnd, &rc_client);

        const auto caption_size = g_get_caption_size();

        auto cx = orientation == vertical ? caption_size : rc_client.right;
        auto cy = orientation == vertical ? rc_client.bottom : caption_size;

        RECT rc_caption = {0, 0, cx, cy};

        RECT rc_dummy{};
        if (!IntersectRect(&rc_dummy, &ps.rcPaint, &rc_caption))
            return 0;

        const auto is_dark = colours::is_dark_mode_active();
        if (m_theme && !is_dark) {
            DrawThemeBackground(m_theme.get(), buffered_dc.get(), 0, 0, &rc_caption, nullptr);
        } else {
            const auto back_brush = get_colour_brush(dark::ColourID::PanelCaptionBackground, is_dark);
            FillRect(buffered_dc.get(), &rc_caption, back_brush.get());
        }

        pfc::string8 text;
        uGetWindowText(wnd, text);

        auto _ = wil::SelectObject(buffered_dc.get(),
            m_panel->m_caption_orientation == horizontal ? g_font_menu_horizontal.get() : g_font_menu_vertical.get());
        uDrawPanelTitle(
            buffered_dc.get(), &rc_caption, text, gsl::narrow<int>(text.length()), orientation == vertical, is_dark);
        return 0;
    }
    case WM_ERASEBKGND: {
        const auto dc = reinterpret_cast<HDC>(wp);
        if (WindowFromDC(dc) == wnd)
            return FALSE;

        uie::win32::paint_background_using_parent(wnd, dc, false);
        return TRUE;
    }
    case WM_WINDOWPOSCHANGED:
        if (m_this.is_valid()) {
            auto lpwp = (LPWINDOWPOS)lp;
            if (!(lpwp->flags & SWP_NOSIZE)) {
                m_panel->on_size(lpwp->cx, lpwp->cy);
            }
        }
        break;
        /*    case WM_SIZE:
        if (m_this.is_valid())
        {
        m_panel->on_size(LOWORD(lp),HIWORD(lp));
        }
        break;*/
    case WM_LBUTTONDOWN:
        if (m_this.is_valid()) {
            if (m_panel->m_show_toggle_area && !m_panel->m_autohide) {
                POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
                if (pt.x == 0) {
                    m_panel->set_hidden(!m_panel->m_hidden);
                }
            }
        }
        return 0;
    case WM_LBUTTONDBLCLK: {
        if (!m_this.is_valid())
            return 0;

        const auto iter = m_this->find_panel_by_container_wnd(wnd);

        if (iter == m_this->m_panels.end())
            return 0;

        const auto panel = *iter;

        if (m_this->get_orientation() != m_panel->m_caption_orientation && !m_panel->m_autohide) {
            POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            if (ChildWindowFromPoint(wnd, pt) == wnd) {
                panel->m_hidden = !panel->m_hidden;
                m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);
                m_this->on_size_changed();
            }
        }
        return 0;
    }
    case WM_MOUSEHOVER:
        if (m_this.is_valid() && m_panel->m_autohide) {
            if ((m_panel->m_hidden)) {
                m_panel->m_hidden = false;
                m_this->get_host()->on_size_limit_change(m_wnd, uie::size_limit_all);
                m_this->on_size_changed();
                enter_autohide_hook();
            }
        }
        break;
    case WM_MOUSEMOVE:
        if (m_this.is_valid() && m_panel->m_autohide) {
            if (cfg_sidebar_use_custom_show_delay && !cfg_sidebar_show_delay) {
                if ((m_panel->m_hidden)) {
                    m_panel->m_hidden = false;
                    m_this->get_host()->on_size_limit_change(m_wnd, uie::size_limit_all);
                    m_this->on_size_changed();
                    enter_autohide_hook();
                }
            } else {
                TRACKMOUSEEVENT tme{};
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_QUERY;
                tme.hwndTrack = wnd;
                _TrackMouseEvent(&tme);

                if (!(tme.dwFlags & TME_HOVER)) {
                    memset(&tme, 0, sizeof(TRACKMOUSEEVENT));
                    tme.cbSize = sizeof(TRACKMOUSEEVENT);
                    tme.hwndTrack = wnd;
                    tme.dwHoverTime = cfg_sidebar_use_custom_show_delay ? cfg_sidebar_show_delay : HOVER_DEFAULT;
                    tme.dwFlags = TME_HOVER;
                    _TrackMouseEvent(&tme);
                }
            }
        }

        break;
    case WM_CONTEXTMENU: {
        enum {
            IDM_CLOSE = 1,
            IDM_MOVE_UP,
            IDM_MOVE_DOWN,
            IDM_LOCK,
            IDM_CAPTION,
            IDM_BASE
        };

        POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        if (pt.x == -1 && pt.y == -1)
            GetMessagePos(&pt);

        POINT pt_client = pt;

        ScreenToClient(wnd, &pt_client);

        HMENU menu = CreatePopupMenu();

        unsigned IDM_EXT_BASE = IDM_BASE;

        if (!m_this.is_valid())
            return 0;

        const auto iter = m_this->find_panel_by_container_wnd(wnd);

        if (iter == m_this->m_panels.end())
            return 0;

        const auto p_panel = *iter;

        AppendMenu(menu, (MF_STRING | (p_panel->m_show_caption ? MF_CHECKED : 0)), IDM_CAPTION, _T("Show &caption"));
        AppendMenu(menu, (MF_STRING | (p_panel->m_locked ? MF_CHECKED : 0)), IDM_LOCK, _T("&Lock panel"));
        AppendMenu(menu, (MF_SEPARATOR), 0, _T(""));
        AppendMenu(menu, (MF_STRING), IDM_MOVE_UP, _T("Move &up"));
        AppendMenu(menu, (MF_STRING), IDM_MOVE_DOWN, _T("Move &down"));
        AppendMenu(menu, (MF_STRING), IDM_CLOSE, _T("&Close panel"));

        pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> extension_menu_nodes
            = new ui_extension::menu_hook_impl;

        if (p_panel->m_child.is_valid()) {
            //                p_ext->build_menu(menu, IDM_EXT_BASE, pt, true, user_data);
            p_panel->m_child->get_menu_items(*extension_menu_nodes.get_ptr());
            if (extension_menu_nodes->get_children_count() > 0)
                AppendMenu(menu, MF_SEPARATOR, 0, nullptr);

            extension_menu_nodes->win32_build_menu(menu, IDM_EXT_BASE, pfc_infinite - IDM_EXT_BASE);
        }
        menu_helpers::win32_auto_mnemonics(menu);

        //            menu_ext_base = IDM_EXT_BASE;

        const auto cmd = static_cast<unsigned>(
            TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, nullptr));

        //            menu_ext_base=0;

        if (cmd >= IDM_EXT_BASE) {
            extension_menu_nodes->execute_by_id(cmd);
        }

        DestroyMenu(menu);

        if (cmd == IDM_CLOSE && p_panel->m_child.is_valid()) {
            service_ptr_t<FlatSplitterPanel> p_this = m_this;
            p_panel->destroy();
            std::erase(p_this->m_panels, p_panel);
            p_this->get_host()->on_size_limit_change(p_this->get_wnd(), uie::size_limit_all);
            p_this->on_size_changed();
        } else if (cmd == IDM_MOVE_UP) {
            if (auto new_iter = std::ranges::find(m_this->m_panels, p_panel);
                new_iter != m_this->m_panels.begin() && new_iter != m_this->m_panels.end()) {
                std::iter_swap(new_iter, std::prev(new_iter));
                m_this->on_size_changed();
            }
        } else if (cmd == IDM_MOVE_DOWN) {
            if (auto new_iter = std::ranges::find(m_this->m_panels, p_panel);
                new_iter != m_this->m_panels.end() && std::next(new_iter) != m_this->m_panels.end()) {
                std::iter_swap(new_iter, std::next(new_iter));
                m_this->on_size_changed();
            }
        } else if (cmd == IDM_LOCK) {
            m_this->save_sizes();
            p_panel->m_locked = p_panel->m_locked == 0;
        } else if (cmd == IDM_CAPTION) {
            // size limit chnge
            p_panel->m_show_caption = p_panel->m_show_caption == 0;
            m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);
            m_this->on_size_changed();
            p_panel->on_size();
        }
        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

void FlatSplitterPanel::Panel::PanelContainer::enter_autohide_hook()
{
    if (!m_hook_active) {
        fbh::LowLevelMouseHookManager::s_get_instance().register_hook(this);
        m_hook_active = true;
    }
}

void FlatSplitterPanel::Panel::PanelContainer::set_window_ptr(FlatSplitterPanel* p_ptr)
{
    m_this = p_ptr;
}

FlatSplitterPanel::Panel::PanelContainer::~PanelContainer() = default;

HWND FlatSplitterPanel::Panel::PanelContainer::create(HWND parent) const
{
    return m_window->create(parent);
}

void FlatSplitterPanel::Panel::PanelContainer::destroy() const
{
    m_window->destroy();
}

FlatSplitterPanel::Panel::PanelContainer::PanelContainer(Panel* p_panel)
    : m_panel(p_panel)
    , m_hook_active(false)
    , m_timer_active(false)
{
    uie::container_window_v3_config config{class_name, false, CS_DBLCLKS};
    m_window = std::make_unique<uie::container_window_v3>(
        config, [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
}

} // namespace cui::panels::splitter
