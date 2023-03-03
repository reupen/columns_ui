#pragma once

namespace cui::dark::spin {
/**
 * \brief Enable dark mode support for a spin (up-down) window.
 *
 * Only supports left/right spin windows, as used in the tab control.
 */
void add_window(HWND wnd);
void remove_window(HWND wnd);

} // namespace cui::dark::spin
