#include "pch.h"
#include "playlist_item_helpers.h"

namespace cui::playlist_item_helpers {

void action_remove_track(bool on_item, size_t idx)
{
    if (on_item) {
        const auto api = playlist_manager::get();
        api->activeplaylist_undo_backup();
        api->activeplaylist_remove_items(bit_array_one(idx));
    }
}

void action_add_to_queue(bool on_item, size_t idx)
{
    if (on_item) {
        const auto api = playlist_manager::get();
        const auto active = api->get_active_playlist();
        if (active != -1)
            api->queue_add_item_playlist(active, idx);
    }
}

void action_none(bool on_on_item, size_t idx) {}

MiddleLickAction MiddleClickActionManager::g_pma_actions[] = {
    {"(None)", 0, action_none},
    {"Remove track from playlist", 1, action_remove_track},
    {"Add to playback queue", 2, action_add_to_queue},
};

size_t MiddleClickActionManager::get_count()
{
    return std::size(g_pma_actions);
}

size_t MiddleClickActionManager::id_to_idx(unsigned id)
{
    constexpr auto count = std::size(g_pma_actions);
    for (size_t n = 0; n < count; n++) {
        if (g_pma_actions[n].id == id)
            return n;
    }
    return 0;
}
} // namespace cui::playlist_item_helpers
