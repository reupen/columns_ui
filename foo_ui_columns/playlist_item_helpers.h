#pragma once

namespace cui::playlist_item_helpers {

using pma_action = void (*)(bool, unsigned int);

struct pma {
    const char* name;
    unsigned id;
    pma_action p_run;
};

class mclick_action {
public:
    static pma g_pma_actions[];
    static unsigned id_to_idx(unsigned id);

    static bool run(unsigned id, bool on_item, unsigned idx)
    {
        g_pma_actions[id_to_idx(id)].p_run(on_item, idx);
        return true;
    }
    static unsigned get_count();
};
} // namespace cui::playlist_item_helpers
