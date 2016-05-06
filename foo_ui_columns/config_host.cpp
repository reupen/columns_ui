#include "stdafx.h"

#include "config_host.h"

void config_host_generic::make_child(HWND wnd)
{
	destroy_child();

	HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);

	RECT tab;

	GetWindowRect(wnd_tab, &tab);
	MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);

	TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

	if (m_active_tab >= (int)m_tab_count)
		m_active_tab = 0;

	if (m_active_tab < (int)m_tab_count && m_active_tab >= 0) {
		m_child = m_tabs[m_active_tab]->create(wnd);
	}

	//SetWindowPos(wnd_tab,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	if (m_child) {
		EnableThemeDialogTexture(m_child, ETDT_ENABLETAB);
	}

	SetWindowPos(m_child, 0, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
	SetWindowPos(wnd_tab, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	ShowWindow(m_child, SW_SHOWNORMAL);
	//UpdateWindow(child);
}

BOOL config_host_generic::g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	config_host_generic* p_instance;
	if (msg == WM_INITDIALOG) {
		p_instance = reinterpret_cast<config_host_generic*>(lp);
		SetWindowLongPtr(wnd, DWLP_USER, lp);
	} else
		p_instance = reinterpret_cast<config_host_generic*>(GetWindowLongPtr(wnd, DWLP_USER));
	return p_instance ? p_instance->on_message(wnd, msg, wp, lp) : FALSE;
}

BOOL config_host_generic::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
		case WM_INITDIALOG:
			{
				HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
				//SendMessage(wnd_tab, TCM_SETMINTABWIDTH, 0, 35);
				unsigned n, count = m_tab_count;
				for (n = 0; n < count; n++) {
					uTabCtrl_InsertItemText(wnd_tab, n, m_tabs[n]->get_name());
				}
				TabCtrl_SetCurSel(wnd_tab, m_active_tab);
				make_child(wnd);
			}
			break;
		case WM_DESTROY:
			break;
		case WM_WINDOWPOSCHANGED:
			{
				auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);
				// Temporary workaround for various bugs that occur due to foobar2000 1.0+ 
				// having a dislike for destroying preference pages
				if (lpwp->flags & SWP_HIDEWINDOW) {
					destroy_child();
				} else if (lpwp->flags & SWP_SHOWWINDOW && !m_child) {
					make_child(wnd);
				}
			}
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom) {
				case IDC_TAB1:
					switch (((LPNMHDR)lp)->code) {
						case TCN_SELCHANGE:
							{
								m_active_tab = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
								make_child(wnd);
							}
							break;
					}
					break;
			}
			break;


		case WM_PARENTNOTIFY:
			switch (wp) {
				case WM_DESTROY:
					{
						if (m_child && (HWND)lp == m_child)
							m_child = 0;
					}
					break;
			}
			break;
	}
	return 0;
}
