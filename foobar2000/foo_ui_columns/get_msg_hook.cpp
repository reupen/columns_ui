#include "stdafx.h"

extern rebar_window * g_rebar_window;

bool get_msg_hook_t::on_hooked_message(message_hook_manager::t_message_hook_type p_type, int code, WPARAM wp, LPARAM lp)
{
	LPMSG lpmsg = (LPMSG)lp;
	if ((lpmsg->message == WM_KEYUP || lpmsg->message == WM_SYSKEYDOWN || lpmsg->message == WM_KEYDOWN) && IsChild(g_main_window, lpmsg->hwnd))
	{
		if (((HIWORD(lpmsg->lParam) & KF_ALTDOWN)))
		{
			if (!(HIWORD(lpmsg->lParam) & KF_REPEAT))
			{
				if (lpmsg->wParam == VK_MENU)
				{
					if ((g_rebar_window && !g_rebar_window->is_menu_focused()) || !g_layout_window.is_menu_focused())
					{
						if (g_rebar_window) g_rebar_window->on_alt_down();
						g_layout_window.show_menu_access_keys();
					}
				}
				else
				{
					if (((HIWORD(lpmsg->lParam) & KF_ALTDOWN) == 0))
					{
						if (g_rebar_window) g_rebar_window->hide_accelerators();
						g_layout_window.hide_menu_access_keys();
					}
				}
			}
		}
		else if (((HIWORD(lpmsg->lParam) & KF_UP)))
		{
			if (lpmsg->wParam == VK_MENU)
			{
				if (g_rebar_window) g_rebar_window->hide_accelerators();
				g_layout_window.hide_menu_access_keys();
			}
		}
		else if (lpmsg->wParam == VK_TAB)
		{
			unsigned flags = uSendMessage(lpmsg->hwnd, WM_GETDLGCODE, 0, (LPARAM)lpmsg);
			if (!((flags & DLGC_WANTTAB) || (flags & DLGC_WANTMESSAGE)))
			{
				ui_extension::window::g_on_tab(lpmsg->hwnd);
				lpmsg->message = WM_NULL;
				lpmsg->lParam = 0;
				lpmsg->wParam = 0;
				SendMessage(lpmsg->hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
			}
		}
	}
	else if (lpmsg->message == WM_MOUSEWHEEL && IsChild(g_main_window, lpmsg->hwnd))
	{
		/**
		* Redirects mouse wheel messages to window under the pointer.
		*
		* This implementation works with non-main message loops e.g. modal dialogs.
		*/
		POINT pt = { GET_X_LPARAM(lpmsg->lParam), GET_Y_LPARAM(lpmsg->lParam) };
		ScreenToClient(g_main_window, &pt);
		HWND wnd = uRecursiveChildWindowFromPoint(g_main_window, pt);
		if (wnd) lpmsg->hwnd = wnd;
	}
	return false;
}


void get_msg_hook_t::register_hook()
{
	message_hook_manager::register_hook(message_hook_manager::type_get_message, this);
}
void get_msg_hook_t::deregister_hook()
{
	message_hook_manager::deregister_hook(message_hook_manager::type_get_message, this);
}

