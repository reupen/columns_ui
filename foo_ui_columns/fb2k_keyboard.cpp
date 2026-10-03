#include "pch.h"

#include "fb2k_keyboard.h"
#include "win32.h"

namespace cui::fb2k_utils {

std::optional<LRESULT> SimpleKeyboardShortcutProcessor::handle_message(HWND wnd, UINT msg, WPARAM wp)
{
    switch (msg) {
    case WM_KEYDOWN:
        if (uie::window::g_process_keydown_keyboard_shortcuts(wp))
            return 0;

        if (wp == VK_TAB) {
            win32::handle_tab_key(wnd);
            return 0;
        }
        break;
    case WM_SYSKEYDOWN: {
        const auto is_destroyed = m_is_destroyed;
        const auto was_processed = uie::window::g_process_keydown_keyboard_shortcuts(wp);

        if (*is_destroyed)
            return 0;

        m_ignore_next_wm_syschar_message = was_processed;

        if (was_processed)
            return 0;

        break;
    }
    case WM_SYSCHAR:
        if (m_ignore_next_wm_syschar_message) {
            m_ignore_next_wm_syschar_message = false;
            return 0;
        }
        break;
    case WM_NCDESTROY:
        m_ignore_next_wm_syschar_message = false;
        break;
    }

    return {};
}

} // namespace cui::fb2k_utils
