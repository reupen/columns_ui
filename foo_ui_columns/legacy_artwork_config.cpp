#include "pch.h"
#include "config.h"
#include "legacy_artwork_config.h"

namespace cui::artwork::legacy {

enum Fb2KArtworkMode {
    fb2k_artwork_disabled,
    fb2k_artwork_embedded,
    fb2k_artwork_embedded_and_external,
};

constexpr GUID g_guid_fb2k_artwork_mode
    = {0xa24038c7, 0xc055, 0x45ed, {0xb6, 0x31, 0xcc, 0x8f, 0xd2, 0xa2, 0x24, 0x73}};

constexpr GUID g_guid_cfg_front_scripts = {0xf6e92fcd, 0x7e02, 0x4329, {0x9d, 0xa3, 0xd0, 0x3a, 0xed, 0xd6, 0x6d, 0x7}};

constexpr GUID g_guid_cfg_back_scripts = {0xbd2474fc, 0x2cf9, 0x475f, {0xac, 0xb, 0x26, 0x13, 0x5, 0x41, 0x52, 0x6c}};

constexpr GUID g_guid_cfg_disc_scripts = {0x70d71df4, 0xd1ff, 0x4d19, {0x94, 0x12, 0xb9, 0x49, 0x69, 0xe, 0xd4, 0x3e}};

constexpr GUID g_guid_cfg_artist_scripts
    = {0xc1e7da7e, 0x1d3a, 0x4f30, {0x83, 0x84, 0xe, 0x47, 0xc4, 0x9b, 0x6d, 0xd9}};

cfg_uint cfg_fb2k_artwork_mode(g_guid_fb2k_artwork_mode, fb2k_artwork_embedded_and_external);

cfg_objList<pfc::string8> cfg_front_scripts(g_guid_cfg_front_scripts);
cfg_objList<pfc::string8> cfg_back_scripts(g_guid_cfg_back_scripts);
cfg_objList<pfc::string8> cfg_disc_scripts(g_guid_cfg_disc_scripts);
cfg_objList<pfc::string8> cfg_artist_scripts(g_guid_cfg_artist_scripts);

cfg_bool has_been_asked_to_reconfigure(
    {0xb2becac4, 0x9a81, 0x465d, {0x9a, 0x4f, 0x8c, 0x7c, 0x16, 0x86, 0x4e, 0xfb}}, false);

bool any_legacy_sources()
{
    auto is_non_empty = [](auto&& source) {
        auto&& [scripts, _] = source;
        return scripts->get_count() > 0;
    };
    return ranges::any_of(legacy_sources, is_non_empty);
}

void prompt_to_reconfigure()
{
    if (any_legacy_sources() && !has_been_asked_to_reconfigure) {
        has_been_asked_to_reconfigure = true;
        prefs::page_main.get_static_instance().show_tab("Artwork");
    }
}

} // namespace cui::artwork::legacy
