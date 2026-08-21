#pragma once

namespace cui::utils {

namespace resize_flags {

constexpr auto move_x = 1u << 0;
constexpr auto move_y = 1u << 1;
constexpr auto move_x_y = move_x | move_y;
constexpr auto resize_width = 1u << 2;
constexpr auto resize_height = 1u << 3;
constexpr auto resize_width_height = resize_width | resize_height;
// Rudimentary handling for proportional resizing
// If this becomes more complicated, it can be replaced with something else
// e.g. the ability to specify weights
constexpr auto move_half_x = 1u << 4;
constexpr auto resize_half_width = 1u << 5;

} // namespace resize_flags

class WindowResizeHelper {
public:
    explicit WindowResizeHelper(HWND wnd, std::unordered_map<int, uint32_t> resize_mode_map);

    bool handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_GETMINMAXINFO:
            handle_wm_getminmaxinfo(reinterpret_cast<LPMINMAXINFO>(lp));
            return true;
        case WM_WINDOWPOSCHANGED:
            handle_wm_windowposchanged(reinterpret_cast<LPWINDOWPOS>(lp));
            return true;
        }

        return false;
    }

private:
    void save_state();
    void handle_wm_getminmaxinfo(LPMINMAXINFO lpmmi) const;
    void handle_wm_windowposchanged(LPWINDOWPOS lpwp);
    void handle_resize();

    HWND m_wnd{};
    std::unordered_map<int, uint32_t> m_resize_flags_map;
    std::unordered_map<int, RECT> m_saved_rect_map;
    RECT m_saved_client_rect{};
    int m_initial_window_width{};
    int m_initial_window_height{};
};

} // namespace cui::utils
