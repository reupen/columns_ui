#include "stdafx.h"
#include "splitter.h"

pfc::ptr_list_t<FlatSplitterPanel> FlatSplitterPanel::g_instances;

void g_get_panel_list(uie::window_info_list_simple& p_out, uie::window_host_ptr& p_host)
{
    service_enum_t<ui_extension::window> e;
    uie::window_ptr l;

    if (e.first(l))
        do {
            uie::window_info_simple info;

            if (l->is_available(p_host)) {
                l->get_name(info.name);
                l->get_category(info.category);
                info.guid = l->get_extension_guid();
                info.prefer_multiple_instances = l->get_prefer_multiple_instances();
                info.type = l->get_type();
                p_out.add_item(info);
            }
        } while (e.next(l));

    p_out.sort_by_category_and_name();
}

void g_append_menu_panels(HMENU menu, const uie::window_info_list_simple& panels, UINT base)
{
    HMENU popup = nullptr;
    unsigned count = panels.get_count();
    for (unsigned n = 0; n < count; n++) {
        if (!n || uStringCompare(panels[n - 1].category, panels[n].category)) {
            if (n)
                uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)popup, panels[n - 1].category);
            popup = CreatePopupMenu();
        }
        uAppendMenu(popup, (MF_STRING), base + n, panels[n].name);
        if (n == count - 1)
            uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)popup, panels[n].category);
    }
}

void g_append_menu_splitters(HMENU menu, const uie::window_info_list_simple& panels, UINT base)
{
    unsigned count = panels.get_count();
    for (unsigned n = 0; n < count; n++) {
        if (panels[n].type & uie::type_splitter)
            uAppendMenu(menu, (MF_STRING), base + n, panels[n].name);
    }
}

void clip_minmaxinfo(MINMAXINFO& mmi)
{
    mmi.ptMinTrackSize.x = min(mmi.ptMinTrackSize.x, MAXSHORT);
    mmi.ptMinTrackSize.y = min(mmi.ptMinTrackSize.y, MAXSHORT);
    mmi.ptMaxTrackSize.y = min(mmi.ptMaxTrackSize.y, MAXSHORT);
    mmi.ptMaxTrackSize.x = min(mmi.ptMaxTrackSize.x, MAXSHORT);
}

class HorizontalSplitterPanel : public FlatSplitterPanel {
    class_data& get_class_data() const override
    {
        __implement_get_class_data_ex(_T("{72FACC90-BB7E-4733-8449-D7537232AD26}"), _T(""), false, 0,
            WS_CHILD | WS_CLIPCHILDREN, WS_EX_CONTROLPARENT, CS_DBLCLKS);
    }
    void get_name(pfc::string_base& p_out) const override { p_out = "Horizontal splitter"; }
    const GUID& get_extension_guid() const override
    {
        // {8FA0BC24-882A-4fff-8A3B-215EA7FBD07F}
        static const GUID rv = {0x8fa0bc24, 0x882a, 0x4fff, {0x8a, 0x3b, 0x21, 0x5e, 0xa7, 0xfb, 0xd0, 0x7f}};
        return rv;
    }
    Orientation get_orientation() const override { return horizontal; }
};

class VerticalSplitterPanel : public FlatSplitterPanel {
    class_data& get_class_data() const override
    {
        __implement_get_class_data_ex(_T("{77653A44-66D1-49e0-9A7A-1C71898C0441}"), _T(""), false, 0,
            WS_CHILD | WS_CLIPCHILDREN, WS_EX_CONTROLPARENT, CS_DBLCLKS);
    }
    void get_name(pfc::string_base& p_out) const override { p_out = "Vertical splitter"; }
    const GUID& get_extension_guid() const override
    {
        // {77653A44-66D1-49e0-9A7A-1C71898C0441}
        static const GUID rv = {0x77653a44, 0x66d1, 0x49e0, {0x9a, 0x7a, 0x1c, 0x71, 0x89, 0x8c, 0x4, 0x41}};
        return rv;
    }
    Orientation get_orientation() const override { return vertical; }
};

uie::window_factory<HorizontalSplitterPanel> g_splitter_window_horizontal;
uie::window_factory<VerticalSplitterPanel> g_splitter_window_vertical;

FlatSplitterPanel::Panel::ptr FlatSplitterPanel::Panel::null_ptr = FlatSplitterPanel::Panel::ptr();

#if 0
template <orientation_t t_orientation>
class dummy_class
{
    static ui_helpers::container_window::class_data myint;
};

template <orientation_t t_orientation>
int dummy_class<t_orientation>::myint = {_T("dummy"), _T(""), 0, false, false, 0, WS_CHILD|WS_CLIPCHILDREN, WS_EX_CONTROLPARENT, 0};
#endif

//#endif
