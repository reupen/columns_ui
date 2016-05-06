#pragma once

#include "fonts_manager_data.h"
#include "colours_manager_data.h"

class colours_client_list_entry_t
{
public:
	pfc::string8 m_name;
	GUID m_guid;
	cui::colours::client::ptr m_ptr;
};

class colours_client_list_t : public pfc::list_t < colours_client_list_entry_t >
{
public:
	static void g_get_list(colours_client_list_t & p_out)
	{
		service_enum_t<cui::colours::client> e;
		colours_client_list_entry_t entry;
		while (e.next(entry.m_ptr))
		{
			entry.m_guid = entry.m_ptr->get_client_guid();
			entry.m_ptr->get_name(entry.m_name);
			p_out.add_item(entry);
		};
		p_out.sort_t(g_compare);
	}
	static int g_compare(const colours_client_list_entry_t & p1, const colours_client_list_entry_t & p2)
	{
		return StrCmpLogicalW(pfc::stringcvt::string_os_from_utf8(p1.m_name), pfc::stringcvt::string_os_from_utf8(p2.m_name));
	}
};

class fonts_client_list_entry_t
{
public:
	pfc::string8 m_name;
	GUID m_guid;
	cui::fonts::client::ptr m_ptr;
};

class fonts_client_list_t : public pfc::list_t < fonts_client_list_entry_t >
{
public:
	static void g_get_list(fonts_client_list_t & p_out)
	{
		service_enum_t<cui::fonts::client> e;
		fonts_client_list_entry_t entry;
		while (e.next(entry.m_ptr))
		{
			entry.m_guid = entry.m_ptr->get_client_guid();
			entry.m_ptr->get_name(entry.m_name);
			p_out.add_item(entry);
		};
		p_out.sort_t(g_compare);
	}
	static int g_compare(const fonts_client_list_entry_t & p1, const fonts_client_list_entry_t & p2)
	{
		return StrCmpLogicalW(pfc::stringcvt::string_os_from_utf8(p1.m_name), pfc::stringcvt::string_os_from_utf8(p2.m_name));
	}
};

extern colours_manager_data g_colours_manager_data;
extern fonts_manager_data g_fonts_manager_data;
