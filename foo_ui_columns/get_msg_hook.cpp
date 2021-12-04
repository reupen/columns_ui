#include "stdafx.h"
#include "get_msg_hook.h"
#include "layout.h"
#include "rebar.h"
#include "main_window.h"

extern cui::rebar::RebarWindow* g_rebar_window;

bool GetMsgHook::on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp)
{
    auto lpmsg = (LPMSG)lp;
    if ((lpmsg->message == WM_KEYUP || lpmsg->message == WM_SYSKEYDOWN || lpmsg->message == WM_KEYDOWN)
        && IsChild(cui::main_window.get_wnd(), lpmsg->hwnd)) {
        if (((HIWORD(lpmsg->lParam) & KF_ALTDOWN))) {
            if (!(HIWORD(lpmsg->lParam) & KF_REPEAT)) {
                if (lpmsg->wParam == VK_MENU) {
                    if ((g_rebar_window && !g_rebar_window->is_menu_focused()) && !g_layout_window.is_menu_focused()) {
                        if (g_rebar_window)
                            g_rebar_window->show_accelerators();
                        g_layout_window.show_menu_access_keys();
                    }
                } else {
                    if (((HIWORD(lpmsg->lParam) & KF_ALTDOWN) == 0)) {
                        if (g_rebar_window)
                            g_rebar_window->hide_accelerators();
                        g_layout_window.hide_menu_access_keys();
                    }
                }
            }
        } else if (((HIWORD(lpmsg->lParam) & KF_UP))) {
            if (lpmsg->wParam == VK_MENU) {
                if (g_rebar_window)
                    g_rebar_window->hide_accelerators();
                g_layout_window.hide_menu_access_keys();
            }
        } else if (lpmsg->wParam == VK_TAB) {
            unsigned flags = SendMessage(lpmsg->hwnd, WM_GETDLGCODE, 0, (LPARAM)lpmsg);
            if (!((flags & DLGC_WANTTAB) || (flags & DLGC_WANTMESSAGE))) {
                ui_extension::window::g_on_tab(lpmsg->hwnd);
                lpmsg->message = WM_NULL;
                lpmsg->lParam = 0;
                lpmsg->wParam = 0;
                SendMessage(lpmsg->hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
            }
        }
    } else if (lpmsg->message == WM_MOUSEWHEEL && IsChild(cui::main_window.get_wnd(), lpmsg->hwnd)) {
        /**
         * Redirects mouse wheel messages to window under the pointer.
         *
         * This implementation works with non-main message loops e.g. modal dialogs.
         */
        POINT pt = {GET_X_LPARAM(lpmsg->lParam), GET_Y_LPARAM(lpmsg->lParam)};
        ScreenToClient(cui::main_window.get_wnd(), &pt);
        HWND wnd = uRecursiveChildWindowFromPoint(cui::main_window.get_wnd(), pt);
        if (wnd)
            lpmsg->hwnd = wnd;
    }
    return false;
}

void GetMsgHook::register_hook()
{
    uih::register_message_hook(uih::MessageHookType::type_get_message, this);
}
void GetMsgHook::deregister_hook()
{
    uih::deregister_message_hook(uih::MessageHookType::type_get_message, this);
}
