#include "pch.h"

#include "core_drop_down_list_toolbar.h"
#include "drop_down_list_toolbar.h"

namespace cui {

namespace {

struct PlaylistSelectorCoreToolbarArgs {
    static constexpr GUID core_toolbar_id{0x6A38E690, 0x83DF, 0x45F1, {0x9C, 0x62, 0x2D, 0x6B, 0x0E, 0x62, 0x2A, 0x96}};
    static constexpr auto no_items_text = "(no playlists)"sv;
    static constexpr const wchar_t* class_name{L"columns_ui_playlist_selector_8o6ohpmHCGI"};
    static constexpr const char* name{"Playlist selector"};
    static constexpr GUID extension_guid{0xba030af9, 0xd806, 0x499b, {0x8a, 0x9c, 0x4b, 0x26, 0xb0, 0x54, 0x48, 0x22}};
    static constexpr GUID colour_client_id{
        0x427c33e1, 0x9568, 0x4a05, {0x9b, 0x4e, 0x6b, 0xde, 0xee, 0x3d, 0xf1, 0x57}};
    static constexpr GUID font_client_id{0xd42967bb, 0xd672, 0x41f3, {0x91, 0x1f, 0x45, 0x99, 0x8a, 0x46, 0x48, 0x84}};
};

ui_extension::window_factory<DropDownListToolbar<CoreDropDownToolbarArgs<PlaylistSelectorCoreToolbarArgs>>> _;

} // namespace

} // namespace cui
