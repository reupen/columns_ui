#include "stdafx.h"
#include "splitter.h"
#include "main_window.h"

#define HOST_AUTOHIDE_TIMER_ID 672

namespace cui::panels::splitter {

bool FlatSplitterPanel::Panel::PanelContainer::test_autohide_window(HWND wnd)
{
    return IsChild(get_wnd(), wnd) || wnd == get_wnd() || wnd == m_this->get_wnd();
}

void FlatSplitterPanel::Panel::PanelContainer::on_hooked_message(WPARAM msg, const MSLLHOOKSTRUCT& mllhs)
{
    if (msg == WM_MOUSEMOVE && m_this.is_valid() && MonitorFromPoint(mllhs.pt, MONITOR_DEFAULTTONULL)) {
        unsigned index = m_this->m_panels.find_item(m_panel->shared_from_this());
        if (index != pfc_infinite) {
            HWND wnd_capture = GetCapture();
            HWND wnd_pt = WindowFromPoint(mllhs.pt);
            POINT pt = mllhs.pt;
            ScreenToClient(m_this->get_wnd(), &pt);
            // if (!hwnd)
            // hwnd = uRecursiveChildWindowFromPointv2(m_this->get_wnd(), pt);
            // console::printf("pts: (%u, %u) pt: (%i, %i)  window: %x", mllhs.pt, pt.x, pt.y, hwnd);

            // We need to test wnd_pt when wnd_capture is non-NULL because during drag-and-drop operations wnd_capture
            // is an OLE window. Alternative fixes are checking the window with the keyboard focus (has side-effects)
            // and checking the window class of wnd_capture (a hack). (For future reference: the class of wnd_capture
            // during drag-and-drop operations is CLIPBRDWNDCLASS.)
            if (!(wnd_capture && test_autohide_window(wnd_capture)) && !(wnd_pt && test_autohide_window(wnd_pt))
                && !m_this->test_divider_pt(pt, index)) {
                if (!m_timer_active)
                    PostMessage(get_wnd(), MSG_AUTOHIDE_END, 0, 0);
            } else {
                if (m_timer_active) {
                    KillTimer(get_wnd(), HOST_AUTOHIDE_TIMER_ID);
                    m_timer_active = false;
                }
            }
        }
    }
}

LRESULT FlatSplitterPanel::Panel::PanelContainer::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_NCCREATE:
        break;
    case WM_CREATE: {
        m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"Rebar") : nullptr;
    } break;
    case WM_THEMECHANGED: {
        if (m_theme)
            CloseThemeData(m_theme);
        m_theme = IsThemeActive() && IsAppThemed() ? OpenThemeData(wnd, L"Rebar") : nullptr;
    } break;
    case WM_DESTROY: {
        if (m_theme)
            CloseThemeData(m_theme);
        m_theme = nullptr;
    }
        m_this.release();
        break;
    case WM_NCDESTROY:
        if (m_hook_active)
            fbh::LowLevelMouseHookManager::s_get_instance().deregister_hook(this);
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
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(wnd, &ps);
        if (m_this.is_valid()) {
            unsigned index = 0;
            if (m_this->m_panels.find_by_wnd(wnd, index) && m_this->m_panels[index]->m_show_caption) {
                RECT rc_client;
                RECT rc_dummy;
                GetClientRect(wnd, &rc_client);

                unsigned caption_size = g_get_caption_size();

                unsigned cx
                    = m_this->m_panels[index]->m_caption_orientation == vertical ? caption_size : rc_client.right;
                unsigned cy
                    = m_this->m_panels[index]->m_caption_orientation == vertical ? rc_client.bottom : caption_size;

                RECT rc_caption = {0, 0, gsl::narrow<long>(cx), gsl::narrow<long>(cy)};

                if (IntersectRect(&rc_dummy, &ps.rcPaint, &rc_caption)) {
                    {
#if 1
                        if (m_theme)
                            DrawThemeBackground(m_theme, dc, 0, 0, &rc_caption, nullptr);
                        else
                            FillRect(dc, &rc_caption, GetSysColorBrush(COLOR_BTNFACE));
#endif
                    }

                    pfc::string8 text;
                    uGetWindowText(wnd, text);

                    HFONT old = SelectFont(dc,
                        m_panel->m_caption_orientation == horizontal ? g_font_menu_horizontal.get()
                                                                     : g_font_menu_vertical.get());
                    // rc_caption.left += 11;
                    uDrawPanelTitle(dc, &rc_caption, text, text.length(),
                        m_this->m_panels[index]->m_caption_orientation == vertical, false);
                    SelectFont(dc, old);

#if 0
                    RECT rc_button = { cx - 15, 0, cx, cy };
                    HTHEME thm = OpenThemeData(wnd, L"ListView");
                    DrawThemeBackground(thm, dc, m_this->m_panels[index]->m_hidden ? LVP_EXPANDBUTTON : LVP_COLLAPSEBUTTON, LVCB_NORMAL, &rc_button, &rc_button);
                    CloseThemeData(thm);
#endif
                }
            }
        }
        EndPaint(wnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: {
        RECT rc_caption = {0, 0, 0, 0};
        RECT rc_fill;
        RECT rc_client;
        GetClientRect(wnd, &rc_client);
        if (m_this.is_valid()) {
            unsigned index = 0;
            if (m_this->m_panels.find_by_wnd(wnd, index) && m_this->m_panels[index]->m_show_caption) {
                unsigned caption_size = g_get_caption_size();

                unsigned cx
                    = m_this->m_panels[index]->m_caption_orientation == vertical ? caption_size : rc_client.right;
                unsigned cy
                    = m_this->m_panels[index]->m_caption_orientation == vertical ? rc_client.bottom : caption_size;

                // RECT rc_caption = {0, 0, cx, cy};
                rc_caption.right = cx;
                rc_caption.bottom = cy;
            }
        }
        SubtractRect(&rc_fill, &rc_client, &rc_caption);
        FillRect(HDC(wp), &rc_fill, GetSysColorBrush(COLOR_BTNFACE));
    }
        return TRUE;
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
    case WM_LBUTTONDBLCLK:
        if (m_this.is_valid()) {
            unsigned index = 0;
            if (m_this->m_panels.find_by_wnd(wnd, index) && m_this->get_orientation() != m_panel->m_caption_orientation
                && !m_panel->m_autohide) {
                POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
                if (ChildWindowFromPoint(wnd, pt) == wnd) {
                    m_this->m_panels[index]->m_hidden = !m_this->m_panels[index]->m_hidden;
                    m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);
                    m_this->on_size_changed();
                }
            }
        }
        return 0;
    case WM_MOUSEHOVER:
        if (m_this.is_valid() && m_panel->m_autohide) {
            if ((m_panel->m_hidden)) {
                m_panel->m_hidden = false;
                m_this->get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
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
                    m_this->get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
                    m_this->on_size_changed();
                    enter_autohide_hook();
                }
            } else {
                TRACKMOUSEEVENT tme;
                memset(&tme, 0, sizeof(TRACKMOUSEEVENT));
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
        enum { IDM_CLOSE = 1, IDM_MOVE_UP, IDM_MOVE_DOWN, IDM_LOCK, IDM_CAPTION, IDM_BASE };

        POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        if (pt.x == -1 && pt.y == -1)
            GetMessagePos(&pt);

        POINT pt_client = pt;

        ScreenToClient(wnd, &pt_client);

        HMENU menu = CreatePopupMenu();

        unsigned IDM_EXT_BASE = IDM_BASE;

        HWND child = ChildWindowFromPoint(wnd, pt_client);

        if (m_this.is_valid()) {
            unsigned index = 0;
            if (m_this->m_panels.find_by_wnd(wnd, index)) {
                std::shared_ptr<Panel> p_panel = m_this->m_panels[index];

                AppendMenu(
                    menu, (MF_STRING | (p_panel->m_show_caption ? MF_CHECKED : 0)), IDM_CAPTION, _T("Show &caption"));
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
                    p_this->m_panels.remove_by_idx(index);
                    p_this->get_host()->on_size_limit_change(p_this->get_wnd(), uie::size_limit_all);
                    p_this->on_size_changed();
                } else if (cmd == IDM_MOVE_UP) {
                    if (index) {
                        m_this->m_panels.swap_items(index, index - 1);
                        m_this->on_size_changed();
                    }
                } else if (cmd == IDM_MOVE_DOWN) {
                    if (index + 1 < m_this->m_panels.get_count()) {
                        m_this->m_panels.swap_items(index, index + 1);
                        m_this->on_size_changed();
                    }
                } else if (cmd == IDM_LOCK) {
                    m_this->save_sizes();
                    m_this->m_panels[index]->m_locked = m_this->m_panels[index]->m_locked == 0;
                } else if (cmd == IDM_CAPTION) {
                    // size limit chnge
                    m_this->m_panels[index]->m_show_caption = m_this->m_panels[index]->m_show_caption == 0;
                    m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);
                    m_this->on_size_changed();
                    m_this->m_panels[index]->on_size();
                }
            }
        }
    }
        return 0;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

FlatSplitterPanel::Panel::PanelContainer::class_data& FlatSplitterPanel::Panel::PanelContainer::get_class_data() const
{
    __implement_get_class_data_ex(_T("foo_ui_columns_splitter_panel_child_container"), _T(""), false, NULL,
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CONTROLPARENT, CS_DBLCLKS);
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

FlatSplitterPanel::Panel::PanelContainer::PanelContainer(Panel* p_panel)
    : m_theme(nullptr)
    , m_panel(p_panel)
    , m_hook_active(false)
    , m_timer_active(false)
{
}

} // namespace cui::panels::splitter
