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

} // namespace cui::dark
