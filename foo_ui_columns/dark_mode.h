#pragma once

/**
 * Note: Dark mode is a work in progress. Functions may be moved to more appropriate homes
 * at a later date.
 */

namespace cui::dark {

/**
 * Temporary compile-time flag controlling whether dark mode is enabled.
 */
bool is_dark_mode_enabled();

void enable_dark_mode_for_app();
void enable_top_level_non_client_dark_mode(HWND wnd);
COLORREF get_system_colour(int system_colour_id, bool is_dark);
wil::unique_hbrush get_system_colour_brush(int system_colour_id, bool is_dark);

} // namespace cui::dark
