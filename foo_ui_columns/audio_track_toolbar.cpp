#include "pch.h"

#include "core_drop_down_list_toolbar.h"
#include "drop_down_list_toolbar.h"

namespace cui {

namespace {

struct AudioTrackCoreToolbarArgs {
    static constexpr GUID core_toolbar_id{0x6A6FF0B0, 0x3FB8, 0x4413, {0xBB, 0x76, 0xFF, 0x48, 0xC8, 0x8F, 0xF0, 0x57}};
    static constexpr const char* ignored_value{"not playing"};
    static constexpr auto no_items_text = "(not playing)"sv;
    static constexpr const wchar_t* class_name{L"columns_ui_audio_track_toolbar_xvkMz8coqQY"};
    static constexpr const char* name{"Audio track"};
    static constexpr GUID extension_guid{0xee6d2fed, 0x158, 0x4fa9, {0xaf, 0x1c, 0x72, 0xf2, 0x34, 0x56, 0xb7, 0x8c}};
    static constexpr GUID colour_client_id{0x91f80f2e, 0x4e58, 0x4600, {0xba, 0x4f, 0x13, 0xc7, 0xeb, 0x5, 0x7d, 0xf0}};
    static constexpr GUID font_client_id{0xe547f854, 0x1efe, 0x4fca, {0x8d, 0x42, 0xe4, 0x24, 0x99, 0x12, 0xbc, 0x9a}};
};

ui_extension::window_factory<DropDownListToolbar<CoreDropDownToolbarArgs<AudioTrackCoreToolbarArgs>>> _;

} // namespace

} // namespace cui
