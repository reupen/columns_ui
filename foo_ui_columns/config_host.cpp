#include "stdafx.h"

#include "config_host.h"

void config_host_generic::make_child()
{
	destroy_child();

	RECT tab;

	GetWindowRect(m_wnd_tabs, &tab);
	MapWindowPoints(HWND_DESKTOP, m_wnd, (LPPOINT)&tab, 2);

	TabCtrl_AdjustRect(m_wnd_tabs, FALSE, &tab);

	if (m_active_tab >= (int)m_tab_count)
		m_active_tab = 0;

	if (m_active_tab < (int)m_tab_count && m_active_tab >= 0) {
		m_child = m_tabs[m_active_tab]->create(m_wnd);
	}

	//SetWindowPos(wnd_tab,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	if (m_child) {
		EnableThemeDialogTexture(m_child, ETDT_ENABLETAB);
	}

	SetWindowPos(m_child, nullptr, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, SWP_NOZORDER);
	SetWindowPos(m_wnd_tabs, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

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

void config_host_generic::show_tab(const char * tab_name)
{
	for (size_t n = 0; n < m_tab_count; n++) {
		if (!strcmp(m_tabs[n]->get_name(), tab_name)) {
			m_active_tab = n;
			break;
		}
	}
	
	if (m_wnd_tabs) {
		TabCtrl_SetCurSel(m_wnd_tabs, m_active_tab);
		make_child();
	}
	else {
		static_api_ptr_t<ui_control>()->show_preferences(get_guid());
	}
}

BOOL config_host_generic::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
		case WM_INITDIALOG:
			{
				m_wnd = wnd;
				m_wnd_tabs = GetDlgItem(wnd, IDC_TAB1);
				//SendMessage(wnd_tab, TCM_SETMINTABWIDTH, 0, 35);
				unsigned n, count = m_tab_count;
				for (n = 0; n < count; n++) {
					uTabCtrl_InsertItemText(m_wnd_tabs, n, m_tabs[n]->get_name());
				}
				TabCtrl_SetCurSel(m_wnd_tabs, m_active_tab);
				make_child();
			}
			break;
		case WM_DESTROY:
			m_wnd_tabs = nullptr;
			m_wnd = nullptr;
			break;
		case WM_WINDOWPOSCHANGED:
			{
				auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);
				// Temporary workaround for various bugs that occur due to foobar2000 1.0+ 
				// having a dislike for destroying preference pages
				if (lpwp->flags & SWP_HIDEWINDOW) {
					destroy_child();
				} else if (lpwp->flags & SWP_SHOWWINDOW && !m_child) {
					make_child();
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
								make_child();
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
							m_child = nullptr;
					}
					break;
			}
			break;
	}
	return 0;
}
