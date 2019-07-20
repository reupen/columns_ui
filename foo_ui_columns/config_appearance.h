#pragma once

#include "fonts_manager_data.h"
#include "colours_manager_data.h"

class ColoursClientListEntry {
public:
    pfc::string8 m_name;
    GUID m_guid{};
    cui::colours::client::ptr m_ptr;
};

class ColoursClientList : public pfc::list_t<ColoursClientListEntry> {
public:
    static void g_get_list(ColoursClientList& p_out)
    {
        service_enum_t<cui::colours::client> e;
        cui::colours::client::ptr ptr;

        while (e.next(ptr)) {
            pfc::string8 name;
            ptr->get_name(name);
            if (name.is_empty())
                name = "(unnamed item)";

            p_out.add_item({name, ptr->get_client_guid(), std::move(ptr)});
        }
        p_out.sort_t(g_compare);
    }
    static int g_compare(const ColoursClientListEntry& p1, const ColoursClientListEntry& p2)
    {
        return StrCmpLogicalW(
            pfc::stringcvt::string_os_from_utf8(p1.m_name), pfc::stringcvt::string_os_from_utf8(p2.m_name));
    }
};

class FontsClientListEntry {
public:
    pfc::string8 m_name;
    GUID m_guid{};
    cui::fonts::client::ptr m_ptr;
};

class FontsClientList : public pfc::list_t<FontsClientListEntry> {
public:
    static void g_get_list(FontsClientList& p_out)
    {
        service_enum_t<cui::fonts::client> e;
        cui::fonts::client::ptr ptr;

        while (e.next(ptr)) {
            pfc::string8 name;
            ptr->get_name(name);
            if (name.is_empty())
                name = "(unnamed item)";

            p_out.add_item({name, ptr->get_client_guid(), std::move(ptr)});
        }
        p_out.sort_t(g_compare);
    }
    static int g_compare(const FontsClientListEntry& p1, const FontsClientListEntry& p2)
    {
        return StrCmpLogicalW(
            pfc::stringcvt::string_os_from_utf8(p1.m_name), pfc::stringcvt::string_os_from_utf8(p2.m_name));
    }
};

extern ColoursManagerData g_colours_manager_data;
extern FontsManagerData g_fonts_manager_data;
