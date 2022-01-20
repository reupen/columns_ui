#include "stdafx.h"
#include "playlist_item_helpers.h"

namespace cui::playlist_item_helpers {

void action_remove_track(bool on_item, unsigned idx)
{
    if (on_item) {
        static_api_ptr_t<playlist_manager> api;
        api->activeplaylist_undo_backup();
        api->activeplaylist_remove_items(bit_array_one(idx));
    }
}

void action_add_to_queue(bool on_item, unsigned idx)
{
    if (on_item) {
        static_api_ptr_t<playlist_manager> api;
        unsigned active = api->get_active_playlist();
        if (active != -1)
            api->queue_add_item_playlist(active, idx);
    }
}

void action_none(bool on_on_item, unsigned idx) {}

MiddleLickAction MiddleClickActionManager::g_pma_actions[] = {
    {"(None)", 0, action_none},
    {"Remove track from playlist", 1, action_remove_track},
    {"Add to playback queue", 2, action_add_to_queue},
};

unsigned MiddleClickActionManager::get_count()
{
    return tabsize(g_pma_actions);
}

unsigned MiddleClickActionManager::id_to_idx(unsigned id)
{
    unsigned count = tabsize(g_pma_actions);
    for (unsigned n = 0; n < count; n++) {
        if (g_pma_actions[n].id == id)
            return n;
    }
    return 0;
}
} // namespace cui::playlist_item_helpers
