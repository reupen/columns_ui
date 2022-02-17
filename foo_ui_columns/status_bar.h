#pragma once

#include "volume.h"

namespace cui::status_bar {

extern volume_popup_t volume_popup_window;

enum StatusBarPart : uint32_t {
    t_parts_none = 0,
    t_parts_all = 0xffffffff,
    t_part_main = 1 << 0,
    t_part_lock = 1 << 1,
    t_part_length = 1 << 2,
    t_part_count = 1 << 3,
    t_part_volume = 1 << 4
};

enum class StatusBarPartID : ULONG_PTR {
    PlaybackInformation,
    MenuItemDescription,
    PlaylistLock,
    TrackLength,
    TrackCount,
    Volume,
};

void set_part_sizes(unsigned p_parts = t_parts_none);

HICON get_icon();
void regenerate_text();
void set_playback_information(std::string_view text);
void set_menu_item_description(std::string_view text);
void clear_menu_item_description();
void create_window();
void destroy_window();
void on_status_font_change();
std::optional<LRESULT> handle_draw_item(const LPDRAWITEMSTRUCT lpdis);

} // namespace cui::status_bar
