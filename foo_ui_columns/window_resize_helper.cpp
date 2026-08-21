#include "pch.h"

#include "window_resize_helper.h"

namespace cui::utils {

WindowResizeHelper::WindowResizeHelper(HWND wnd, std::unordered_map<int, uint32_t> resize_mode_map)
    : m_wnd(wnd)
    , m_resize_flags_map(std::move(resize_mode_map))
{
    RECT window_rect{};
    GetWindowRect(m_wnd, &window_rect);
    m_initial_window_width = wil::rect_width(window_rect);
    m_initial_window_height = wil::rect_height(window_rect);

    save_state();
}

void WindowResizeHelper::save_state()
{
    GetClientRect(m_wnd, &m_saved_client_rect);

    for (const auto ctrl_id : m_resize_flags_map | std::views::keys) {
        const auto ctrl_wnd = GetDlgItem(m_wnd, ctrl_id);

        if (!ctrl_wnd)
            continue;

        RECT ctrl_rect{};
        GetWindowRect(GetDlgItem(m_wnd, ctrl_id), &ctrl_rect);

        MapWindowPoints(HWND_DESKTOP, m_wnd, reinterpret_cast<LPPOINT>(&ctrl_rect), 2);
        m_saved_rect_map[ctrl_id] = ctrl_rect;
    }
}

void WindowResizeHelper::handle_wm_getminmaxinfo(LPMINMAXINFO lpmmi) const
{
    lpmmi->ptMinTrackSize.x = m_initial_window_width;
    lpmmi->ptMinTrackSize.y = m_initial_window_height;
}

void WindowResizeHelper::handle_wm_windowposchanged(LPWINDOWPOS lpwp)
{
    if ((lpwp->flags & (SWP_NOMOVE | SWP_NOSIZE)) != (SWP_NOMOVE | SWP_NOSIZE))
        handle_resize();
}

void WindowResizeHelper::handle_resize()
{
    RECT new_client_rect{};
    GetClientRect(m_wnd, &new_client_rect);

    const auto width_delta = new_client_rect.right - m_saved_client_rect.right;
    const auto height_delta = new_client_rect.bottom - m_saved_client_rect.bottom;

    auto dwp = BeginDeferWindowPos(gsl::narrow<int>(m_resize_flags_map.size()));

    if (!dwp) {
        LOG_LAST_ERROR();
        return;
    }

    for (auto&& [ctrl_id, resize_mode] : m_resize_flags_map) {
        const auto ctrl_wnd = GetDlgItem(m_wnd, ctrl_id);

        if (!ctrl_wnd)
            continue;

        const auto saved_ctrl_rect_iter = m_saved_rect_map.find(ctrl_id);

        if (saved_ctrl_rect_iter == m_saved_rect_map.end())
            continue;

        const RECT& saved_ctrl_rect = saved_ctrl_rect_iter->second;

        const auto new_ctrl_x = [&] {
            if (resize_mode & resize_flags::move_x)
                return saved_ctrl_rect.left + width_delta;

            if (resize_mode & resize_flags::move_half_x)
                return saved_ctrl_rect.left + width_delta / 2;

            return saved_ctrl_rect.left;
        }();

        const auto new_ctrl_y = saved_ctrl_rect.top + ((resize_mode & resize_flags::move_y) ? height_delta : 0);

        const auto new_ctrl_width = [&] {
            const auto saved_width = wil::rect_width(saved_ctrl_rect);

            if (resize_mode & resize_flags::resize_width)
                return saved_width + width_delta;

            if (resize_mode & resize_flags::resize_half_width)
                return saved_width + width_delta / 2;

            return saved_width;
        }();

        const auto new_ctrl_height
            = wil::rect_height(saved_ctrl_rect) + ((resize_mode & resize_flags::resize_height) ? height_delta : 0);

        dwp = DeferWindowPos(
            dwp, ctrl_wnd, nullptr, new_ctrl_x, new_ctrl_y, new_ctrl_width, new_ctrl_height, SWP_NOZORDER);

        if (!dwp) {
            LOG_LAST_ERROR();
            return;
        }
    }

    if (!LOG_IF_WIN32_BOOL_FALSE(EndDeferWindowPos(dwp)))
        return;
}

} // namespace cui::utils
