#pragma once

namespace cui::main_menu {

struct MainMenuCommand {
    GUID guid;
    const char* name;
    const char* description;
    std::function<void()> execute_callback;
    std::function<bool()> is_ticked_callback{};
    bool hide_without_shift_key{};
};

namespace commands {
constexpr GUID show_groups_id = {0x8fa126be, 0xda26, 0x43de, {0xa5, 0x25, 0xb6, 0x4d, 0x17, 0x79, 0x3c, 0xf0}};
constexpr GUID show_artwork_id = {0x1fba5c30, 0xc6ed, 0x4649, {0x84, 0xa0, 0x8f, 0xf6, 0xab, 0x71, 0x85, 0x97}};
constexpr GUID activate_now_playing_id{0x450c17b0, 0x70e0, 0x42cc, {0xaa, 0x7d, 0xdc, 0x5b, 0x7a, 0xdf, 0xf5, 0xa8}};
constexpr GUID toggle_live_editing_id = {0xf2c92eb9, 0xb5f1, 0x4cec, {0xb2, 0x4e, 0xc1, 0x40, 0x83, 0x1f, 0x7b, 0x9}};
} // namespace commands
} // namespace cui::main_menu
