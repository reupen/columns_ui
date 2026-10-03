#pragma once

namespace cui::fb2k_utils {

class SimpleKeyboardShortcutProcessor {
public:
    std::optional<LRESULT> handle_message(HWND wnd, UINT msg, WPARAM wp);

    ~SimpleKeyboardShortcutProcessor() { *m_is_destroyed = true; }

private:
    bool m_ignore_next_wm_syschar_message{};
    std::shared_ptr<bool> m_is_destroyed{std::make_shared<bool>()};
};

} // namespace cui::fb2k_utils
