#pragma once

#include "config.h"

class config_host_generic : public preferences_page {
public:
	config_host_generic(const char* p_name, preferences_tab*const * const p_tabs, size_t p_tab_count, const GUID& p_guid, const GUID& p_parent_guid, cfg_int* const p_active_tab)
		: m_child(nullptr), m_name(p_name), m_guid(p_guid), m_parent_guid(p_parent_guid), m_tabs(p_tabs), m_tab_count(p_tab_count), m_active_tab(*p_active_tab) {}

	HWND create(HWND parent) override
	{
		return uCreateDialog(IDD_HOST, parent, g_on_message, reinterpret_cast<LPARAM>(this));
	}

	const char* get_name() override
	{
		return m_name;
	}

	GUID get_guid() override
	{
		return m_guid;
	}

	GUID get_parent_guid() override
	{
		return m_parent_guid;
	}

	bool reset_query() override
	{
		return false;
	}

	void reset() override { };

	bool get_help_url(pfc::string_base& p_out) override
	{
		if (!(m_active_tab < (int)m_tab_count && m_tabs[m_active_tab]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}

private:
	void destroy_child()
	{
		if (m_child) {
			ShowWindow(m_child, SW_HIDE);
			DestroyWindow(m_child);
			m_child = nullptr;
		}
	}

	void make_child(HWND wnd);

	static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

	BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

	HWND m_child;
	const char* m_name;
	const GUID &m_guid, &m_parent_guid;
	preferences_tab*const * const m_tabs;
	const size_t m_tab_count;
	cfg_int& m_active_tab;
};
