#pragma once

namespace cui::playlist_item_helpers {

using MiddleClickFunction = void (*)(bool, unsigned int);

struct MiddleLickAction {
    const char* name;
    unsigned id;
    MiddleClickFunction p_run;
};

class MiddleClickActionManager {
public:
    static MiddleLickAction g_pma_actions[];
    static unsigned id_to_idx(unsigned id);

    static bool run(unsigned id, bool on_item, unsigned idx)
    {
        g_pma_actions[id_to_idx(id)].p_run(on_item, idx);
        return true;
    }
    static unsigned get_count();
};
} // namespace cui::playlist_item_helpers
