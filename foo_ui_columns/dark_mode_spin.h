#pragma once

namespace cui::dark::spin {

/**
 * \brief Enable dark mode support for a spin (up-down) window.
 *
 * Supports only left/right standalone and up/down buddy modes.
 */
void add_window(HWND wnd);
void remove_window(HWND wnd);

} // namespace cui::dark::spin
