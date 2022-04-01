#include "pch.h"

namespace cui::colours {

int get_system_colour_id(const colour_identifier_t colour_id)
{
    switch (colour_id) {
    case colour_text:
        return COLOR_WINDOWTEXT;
    case colour_selection_text:
        return COLOR_HIGHLIGHTTEXT;
    case colour_background:
        return COLOR_WINDOW;
    case colour_selection_background:
        return COLOR_HIGHLIGHT;
    case colour_inactive_selection_text:
        return COLOR_BTNTEXT;
    case colour_inactive_selection_background:
        return COLOR_BTNFACE;
    case colour_active_item_frame:
        return COLOR_WINDOWFRAME;
    default:
        uBugCheck();
    }
}

} // namespace cui::colours
