#pragma once

namespace cui::playlist_item_helpers {

using MiddleClickFunction = void (*)(bool, size_t);

struct MiddleLickAction {
    const char* name;
    unsigned id;
    MiddleClickFunction p_run;
};

class MiddleClickActionManager {
public:
    static MiddleLickAction g_pma_actions[];
    static size_t id_to_idx(unsigned id);

    static bool run(unsigned id, bool on_item, size_t idx)
    {
        g_pma_actions[id_to_idx(id)].p_run(on_item, idx);
        return true;
    }
    static size_t get_count();
};
} // namespace cui::playlist_item_helpers
