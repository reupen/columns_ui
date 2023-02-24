#include "pch.h"

#include "dark_mode.h"

namespace cui::dark::spin {

namespace {

enum class Direction {
    Left,
    Right,
};

enum class ButtonState {
    Default,
    Hot,
    Pressed,
};

struct WindowState {
    WNDPROC wnd_proc{};
    bool enabled{true};
    std::optional<Direction> render_hot_button;
    std::optional<Direction> actual_hot_button;
    std::optional<Direction> pressed_button;
};

namespace state {

std::optional<ULONG_PTR> gdiplus_token;
std::unordered_map<HWND, WindowState> state_map;

} // namespace state

void render_button_border_and_background(Gdiplus::Graphics& graphics, RECT rect, Direction direction, ButtonState state)
{
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias8x8);

    const auto background_colour_id = [state]() {
        if (state == ButtonState::Pressed)
            return ColourID::SpinPressedButtonBackground;

        if (state == ButtonState::Hot)
            return ColourID::SpinHotButtonBackground;

        return ColourID::SpinButtonBackground;
    }();

    const auto path_width = 1_spx;

    const Gdiplus::SolidBrush background_brush(get_dark_gdiplus_colour(background_colour_id));
    const Gdiplus::Pen border_pen(
        get_dark_gdiplus_colour(ColourID::SpinButtonBorder), static_cast<Gdiplus::REAL>(path_width));

    const auto half_path = (path_width + 1) / 2;
    const auto arc_radius = path_width;
    const auto arc_diameter = arc_radius * 2;

    const auto border_left = gsl::narrow<int>(rect.left);
    const auto border_top = gsl::narrow<int>(rect.top);
    const auto border_right = gsl::narrow<int>(rect.right) - half_path;
    const auto border_bottom = gsl::narrow<int>(rect.bottom) - half_path;
    const auto arc_ellipse_right = border_right - arc_diameter;
    const auto arc_ellipse_bottom = border_bottom - arc_diameter;

    Gdiplus::GraphicsPath border;

    if (direction == Direction::Right) {
        border.AddArc(arc_ellipse_right, border_top, arc_diameter, arc_diameter, 270, 90);
        border.AddArc(arc_ellipse_right, arc_ellipse_bottom, arc_diameter, arc_diameter, 0, 90);
        border.AddLine(border_left, border_bottom, border_left, border_top);
    } else {
        border.AddArc(border_left, arc_ellipse_bottom, arc_diameter, arc_diameter, 90, 90);
        border.AddArc(border_left, border_top, arc_diameter, arc_diameter, 180, 90);
        border.AddLine(border_right, border_top, border_right, border_bottom);
    }

    border.CloseFigure();

    graphics.FillPath(&background_brush, &border);
    graphics.DrawPath(&border_pen, &border);

    graphics.SetSmoothingMode(Gdiplus::SmoothingModeDefault);
}

void render_button_arrow(Gdiplus::Graphics& graphics, RECT rect, Direction direction)
{
    const auto button_height_px = static_cast<float>(rect.bottom);
    const auto button_left_px = static_cast<float>(rect.left);
    const auto button_width_px = static_cast<float>(rect.right - rect.left);

    constexpr auto triangle_half_width = 0.75f;
    constexpr auto triangle_half_height = 1.5f;
    constexpr auto divisor = 10.0f;
    constexpr auto half_divisor = divisor / 2.0f;

    auto narrow_side_coeff = half_divisor + (direction == Direction::Right ? 1.0f : -1.0f) * triangle_half_width;
    auto wide_side_coeff = half_divisor + (direction == Direction::Right ? -1.0f : 1.0f) * triangle_half_width;

    const auto triangle_top = (half_divisor - triangle_half_height) * button_height_px / divisor;
    const auto triangle_bottom = (half_divisor + triangle_half_height) * button_height_px / divisor;
    const auto triangle_start = button_left_px + wide_side_coeff * button_width_px / divisor;
    const auto triangle_end = button_left_px + narrow_side_coeff * button_width_px / divisor;

    const Gdiplus::PointF triangle_points[] = {
        {triangle_end, button_height_px / 2.0f},
        {triangle_start, triangle_top},
        {triangle_start, triangle_bottom},
    };

    const Gdiplus::SolidBrush arrow_brush(get_dark_gdiplus_colour(ColourID::SpinButtonArrow));
    graphics.FillPolygon(&arrow_brush, triangle_points, 3);
}

void render_button(Gdiplus::Graphics& graphics, RECT rect, Direction direction, ButtonState state)
{
    render_button_border_and_background(graphics, rect, direction, state);
    render_button_arrow(graphics, rect, direction);
}

std::tuple<RECT, RECT> get_button_rects(HWND wnd)
{
    RECT client_rect{};
    GetClientRect(wnd, &client_rect);

    const RECT left_rect{0, 0, client_rect.right / 2, client_rect.bottom};
    const RECT right_rect{left_rect.right, 0, client_rect.right, client_rect.bottom};

    return {left_rect, right_rect};
}

bool is_near_window(HWND wnd, POINT pt)
{
    RECT client_rect{};
    GetClientRect(wnd, &client_rect);

    InflateRect(&client_rect, (GetSystemMetrics(SM_CXSIZEFRAME) + 1) / 2, (GetSystemMetrics(SM_CYSIZEFRAME) + 1) / 2);

    return PtInRect(&client_rect, pt) != 0;
}

std::optional<Direction> get_direction_at_point(HWND wnd, POINT pt, bool strict = true)
{
    if (strict && !is_near_window(wnd, pt))
        return {};

    auto [left_rect, right_rect] = get_button_rects(wnd);

    if (pt.x < right_rect.left)
        return Direction::Left;

    if (pt.x > right_rect.left)
        return Direction::Right;

    return {};
}

ButtonState get_button_state(const WindowState& window_state, Direction direction)
{
    const auto direction_opt = std::make_optional(direction);

    if (window_state.pressed_button == direction_opt && window_state.actual_hot_button == direction_opt)
        return ButtonState::Pressed;

    if ((window_state.pressed_button == direction_opt && window_state.render_hot_button == direction_opt)
        || window_state.actual_hot_button == direction_opt)
        return ButtonState::Hot;

    return ButtonState::Default;
}

LRESULT WINAPI on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto& window_state = state::state_map.at(wnd);
    auto call_next_window_proc
        = [wnd_proc{window_state.wnd_proc}, wnd, msg, wp, lp] { return CallWindowProc(wnd_proc, wnd, msg, wp, lp); };

    if (msg != WM_NCDESTROY && !(state::gdiplus_token && window_state.enabled))
        return call_next_window_proc();

    switch (msg) {
    case WM_LBUTTONDOWN: {
        const POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        window_state.pressed_button = get_direction_at_point(wnd, pt);
        window_state.actual_hot_button = get_direction_at_point(wnd, pt);
        window_state.render_hot_button = window_state.actual_hot_button;
        break;
    }
    case WM_LBUTTONUP: {
        const POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        window_state.pressed_button.reset();
        window_state.actual_hot_button = get_direction_at_point(wnd, pt);
        window_state.render_hot_button = window_state.actual_hot_button;
        break;
    }
    case WM_MOUSEMOVE: {
        const POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        window_state.actual_hot_button = get_direction_at_point(wnd, pt);
        window_state.render_hot_button = get_direction_at_point(wnd, pt, (wp & MK_LBUTTON) == 0);
        break;
    }
    case WM_MOUSELEAVE:
        window_state.render_hot_button.reset();
        window_state.actual_hot_button.reset();
        break;
    case WM_ERASEBKGND:
        return FALSE;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        const auto paint_dc = wil::BeginPaint(wnd, &ps);
        const uih::BufferedDC buffered_dc(paint_dc.get(), ps.rcPaint);

        Gdiplus::Graphics graphics(buffered_dc.get());

        POINT pt{};
        GetMessagePos(&pt);
        ScreenToClient(wnd, &pt);

        RECT client_rect{};
        GetClientRect(wnd, &client_rect);

        const Gdiplus::Rect rect_plus(gsl::narrow<int>(client_rect.left), gsl::narrow<int>(client_rect.top),
            gsl::narrow<int>(client_rect.right - client_rect.left),
            gsl::narrow<int>(client_rect.bottom - client_rect.top));
        const Gdiplus::SolidBrush background_brush(get_dark_gdiplus_colour(ColourID::SpinBackground));
        graphics.FillRectangle(&background_brush, rect_plus);

        auto [left_rect, right_rect] = get_button_rects(wnd);
        render_button(graphics, right_rect, Direction::Right, get_button_state(window_state, Direction::Right));
        render_button(graphics, left_rect, Direction::Left, get_button_state(window_state, Direction::Left));

        return 0;
    }
    case WM_NCDESTROY:
        state::state_map.erase(wnd);

        if (state::state_map.empty() && state::gdiplus_token) {
            Gdiplus::GdiplusShutdown(*state::gdiplus_token);
            state::gdiplus_token.reset();
        }
        break;
    }

    return call_next_window_proc();
}

} // namespace

void add_window(HWND wnd)
{
    if (is_native_dark_mode_spin_available())
        return;

    if (state::state_map.contains(wnd)) {
        state::state_map[wnd].enabled = true;
        return;
    }

    if (!state::gdiplus_token) {
        console::print("GdiplusStartup");
        const Gdiplus::GdiplusStartupInput input;
        ULONG_PTR token{};

        if (GdiplusStartup(&token, &input, nullptr) == Gdiplus::Ok)
            state::gdiplus_token = token;
    }

    if (const auto wnd_proc = SubclassWindow(wnd, on_message)) {
        state::state_map.emplace(wnd, WindowState{wnd_proc});
    }
}

void remove_window(HWND wnd)
{
    if (state::state_map.contains(wnd)) {
        state::state_map[wnd].enabled = false;
    }
}

} // namespace cui::dark::spin
