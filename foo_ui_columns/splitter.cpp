#include "pch.h"
#include "splitter.h"

namespace cui::panels::splitter {

pfc::ptr_list_t<FlatSplitterPanel> FlatSplitterPanel::g_instances;

class HorizontalSplitterPanel : public FlatSplitterPanel {
    uie::container_window_v3_config get_window_config() override
    {
        return {L"{72FACC90-BB7E-4733-8449-D7537232AD26}", true, CS_DBLCLKS};
    }
    void get_name(pfc::string_base& p_out) const override { p_out = "Row"; }
    const GUID& get_extension_guid() const override
    {
        // {8FA0BC24-882A-4fff-8A3B-215EA7FBD07F}
        static const GUID rv = {0x8fa0bc24, 0x882a, 0x4fff, {0x8a, 0x3b, 0x21, 0x5e, 0xa7, 0xfb, 0xd0, 0x7f}};
        return rv;
    }
    Orientation get_orientation() const override { return horizontal; }
};

class VerticalSplitterPanel : public FlatSplitterPanel {
    uie::container_window_v3_config get_window_config() override
    {
        return {L"{77653A44-66D1-49e0-9A7A-1C71898C0441}", true, CS_DBLCLKS};
    }
    void get_name(pfc::string_base& p_out) const override { p_out = "Column"; }
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

} // namespace cui::panels::splitter
