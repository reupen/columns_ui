#include "pch.h"

#include "dark_mode.h"

namespace cui::dark::spin {

namespace {

enum class SpinType {
    UpDown,
    LeftRight,
};

enum class Direction {
    Up,
    Down,
    Left,
    Right,
};

enum class ButtonState {
    Default,
    Hot,
    Pressed,
    Disabled,
};

struct WindowState {
    WNDPROC wnd_proc{};
    SpinType spin_type{SpinType::UpDown};
    bool enabled{true};
    std::optional<Direction> render_hot_button;
    std::optional<Direction> actual_hot_button;
    std::optional<Direction> pressed_button;
};

struct RenderContext {
    RECT button_rect{};
    Direction direction{};
    ButtonState button_state{};
    bool has_buddy{};
};

namespace state {

std::optional<ULONG_PTR> gdiplus_token;
std::unordered_map<HWND, WindowState> state_map;

} // namespace state

void render_button_border_and_background(Gdiplus::Graphics& graphics, const RenderContext& context)
{
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias8x8);

    const auto background_colour_id = [button_state{context.button_state}, has_buddy{context.has_buddy}]() {
        if (button_state == ButtonState::Pressed)
            return has_buddy ? ColourID::SpinPressedBuddyButtonBackground : ColourID::SpinPressedButtonBackground;

        if (button_state == ButtonState::Hot)
            return has_buddy ? ColourID::SpinHotBuddyButtonBackground : ColourID::SpinHotButtonBackground;

        if (button_state == ButtonState::Disabled)
            return has_buddy ? ColourID::SpinDisabledBuddyButtonBackground : ColourID::SpinDisabledButtonBackground;

        return has_buddy ? ColourID::SpinBuddyButtonBackground : ColourID::SpinButtonBackground;
    }();

    const Gdiplus::SolidBrush background_brush(get_dark_gdiplus_colour(background_colour_id));

    const auto border_colour_id = [&context]() {
        if (!context.has_buddy)
            return ColourID::SpinButtonBorder;

        if (context.button_state == ButtonState::Disabled)
            return ColourID::SpinDisabledBuddyButtonBorder;

        return ColourID::SpinBuddyButtonBorder;
    }();

    const auto path_width = 1_spx;
    const Gdiplus::Pen border_pen(get_dark_gdiplus_colour(border_colour_id), static_cast<Gdiplus::REAL>(path_width));

    const auto half_path = (path_width + 1) / 2;
    const auto arc_radius = path_width;
    const auto arc_diameter = arc_radius * 2;

    const auto border_left = gsl::narrow<int>(context.button_rect.left);
    const auto border_top = gsl::narrow<int>(context.button_rect.top);
    const auto border_right = gsl::narrow<int>(context.button_rect.right) - half_path;
    const auto border_bottom = gsl::narrow<int>(context.button_rect.bottom) - half_path;
    const auto arc_ellipse_right = border_right - arc_diameter;
    const auto arc_ellipse_bottom = border_bottom - arc_diameter;

    Gdiplus::GraphicsPath border;

    switch (context.direction) {
    case Direction::Up:
    case Direction::Down:
        border.AddLine(border_left, border_bottom, border_left, border_top);
        border.AddLine(border_right, border_top, border_right, border_bottom);
        break;
    case Direction::Left:
        border.AddArc(border_left, arc_ellipse_bottom, arc_diameter, arc_diameter, 90, 90);
        border.AddArc(border_left, border_top, arc_diameter, arc_diameter, 180, 90);
        border.AddLine(border_right, border_top, border_right, border_bottom);
        break;
    case Direction::Right:
        border.AddArc(arc_ellipse_right, border_top, arc_diameter, arc_diameter, 270, 90);
        border.AddArc(arc_ellipse_right, arc_ellipse_bottom, arc_diameter, arc_diameter, 0, 90);
        border.AddLine(border_left, border_bottom, border_left, border_top);
        break;
    }

    border.CloseFigure();

    graphics.FillPath(&background_brush, &border);
    graphics.DrawPath(&border_pen, &border);

    graphics.SetSmoothingMode(Gdiplus::SmoothingModeDefault);
}

void render_button_arrow(Gdiplus::Graphics& graphics, const RenderContext& context)
{
    const auto direction = context.direction;
    const auto button_rect = context.button_rect;
    const auto is_up_down = direction == Direction::Up || direction == Direction::Down;
    const auto half_button_height = static_cast<float>(button_rect.bottom - button_rect.top) / 2.0f;
    const auto half_button_width = static_cast<float>(button_rect.right - button_rect.left) / 2.0f;
    const auto x_midpoint = static_cast<float>(button_rect.left) + half_button_width;
    const auto y_midpoint = static_cast<float>(button_rect.top) + half_button_height;

    // Internal height (actually horizontal in left and right arrows)
    const auto half_triangle_height = 0.15f * half_button_width;
    const auto half_triangle_base_length = 0.3f * (is_up_down ? half_button_width : half_button_height);

    const auto triangle_points = [=] {
        const auto coefficient = direction == Direction::Right || direction == Direction::Down ? 1.0f : -1.0f;

        if (is_up_down) {
            const auto triangle_left = x_midpoint - half_triangle_base_length;
            const auto triangle_right = x_midpoint + half_triangle_base_length;
            const auto triangle_start = y_midpoint - coefficient * half_triangle_height;
            const auto triangle_end = y_midpoint + coefficient * half_triangle_height;

            return std::vector<Gdiplus::PointF>{
                {half_button_width, triangle_end},
                {triangle_left, triangle_start},
                {triangle_right, triangle_start},
            };
        }

        const auto triangle_top = y_midpoint - half_triangle_base_length;
        const auto triangle_bottom = y_midpoint + half_triangle_base_length;
        const auto triangle_start = x_midpoint - coefficient * half_triangle_height;
        const auto triangle_end = x_midpoint + coefficient * half_triangle_height;

        return std::vector<Gdiplus::PointF>{
            {triangle_end, half_button_height},
            {triangle_start, triangle_top},
            {triangle_start, triangle_bottom},
        };
    }();

    const Gdiplus::SolidBrush arrow_brush(get_dark_gdiplus_colour(ColourID::SpinButtonArrow));
    graphics.FillPolygon(&arrow_brush, triangle_points.data(), gsl::narrow<INT>(triangle_points.size()));
}

ButtonState get_button_state(const WindowState& window_state, Direction direction, bool is_disabled)
{
    if (is_disabled)
        return ButtonState::Disabled;

    const auto direction_opt = std::make_optional(direction);

    if (window_state.pressed_button == direction_opt && window_state.actual_hot_button == direction_opt)
        return ButtonState::Pressed;

    if ((window_state.pressed_button == direction_opt && window_state.render_hot_button == direction_opt)
        || window_state.actual_hot_button == direction_opt)
        return ButtonState::Hot;

    return ButtonState::Default;
}

void render_button(Gdiplus::Graphics& graphics, RECT button_rect, const WindowState& window_state, Direction direction,
    bool is_disabled, bool has_buddy)
{
    const RenderContext context{
        button_rect, direction, get_button_state(window_state, direction, is_disabled), has_buddy};

    render_button_border_and_background(graphics, context);
    render_button_arrow(graphics, context);
}

std::tuple<RECT, RECT> get_button_rects(HWND wnd, SpinType spin_type)
{
    RECT client_rect{};
    GetClientRect(wnd, &client_rect);

    if (spin_type == SpinType::UpDown) {
        const RECT top_rect{0, 0, client_rect.right, client_rect.bottom / 2};
        const RECT bottom_rect{0, top_rect.bottom, client_rect.right, client_rect.bottom};

        return {top_rect, bottom_rect};
    }

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

std::optional<Direction> get_direction_at_point(HWND wnd, SpinType spin_type, POINT pt, bool strict = true)
{
    if (strict && !is_near_window(wnd, pt))
        return {};

    auto [first_rect, second_rect] = get_button_rects(wnd, spin_type);

    if (spin_type == SpinType::UpDown) {
        if (pt.y < second_rect.top)
            return Direction::Up;

        if (pt.y > second_rect.top)
            return Direction::Down;
    } else {
        if (pt.x < second_rect.left)
            return Direction::Left;

        if (pt.x > second_rect.left)
            return Direction::Right;
    }

    return {};
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
        window_state.pressed_button = get_direction_at_point(wnd, window_state.spin_type, pt);
        window_state.actual_hot_button = get_direction_at_point(wnd, window_state.spin_type, pt);
        window_state.render_hot_button = window_state.actual_hot_button;
        break;
    }
    case WM_LBUTTONUP: {
        const POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        window_state.pressed_button.reset();
        window_state.actual_hot_button = get_direction_at_point(wnd, window_state.spin_type, pt);
        window_state.render_hot_button = window_state.actual_hot_button;
        break;
    }
    case WM_MOUSEMOVE: {
        const POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        window_state.actual_hot_button = get_direction_at_point(wnd, window_state.spin_type, pt);
        window_state.render_hot_button
            = get_direction_at_point(wnd, window_state.spin_type, pt, (wp & MK_LBUTTON) == 0);
        break;
    }
    case WM_MOUSELEAVE:
        window_state.render_hot_button.reset();
        window_state.actual_hot_button.reset();
        break;
    case WM_ERASEBKGND:
        return FALSE;
    case WM_PAINT: {
        const auto buddy_wnd = reinterpret_cast<HWND>(SendMessage(wnd, UDM_GETBUDDY, 0, 0));
        const auto has_buddy = buddy_wnd != nullptr;
        const auto buddy_styles = has_buddy ? GetWindowLongPtr(buddy_wnd, GWL_STYLE) : 0;
        const auto is_disabled = (buddy_styles & WS_DISABLED) != 0;

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

        auto [first_rect, second_rect] = get_button_rects(wnd, window_state.spin_type);
        const auto first_direction = window_state.spin_type == SpinType::UpDown ? Direction::Up : Direction::Left;
        const auto second_direction = window_state.spin_type == SpinType::UpDown ? Direction::Down : Direction::Right;

        render_button(graphics, first_rect, window_state, first_direction, is_disabled, has_buddy);
        render_button(graphics, second_rect, window_state, second_direction, is_disabled, has_buddy);

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
    if (is_native_dark_spin_available())
        return;

    if (state::state_map.contains(wnd)) {
        state::state_map[wnd].enabled = true;
        return;
    }

    if (!state::gdiplus_token) {
        const Gdiplus::GdiplusStartupInput input;
        ULONG_PTR token{};

        if (GdiplusStartup(&token, &input, nullptr) == Gdiplus::Ok)
            state::gdiplus_token = token;
    }

    if (const auto wnd_proc = SubclassWindow(wnd, on_message)) {
        const auto spin_type = (GetWindowLongPtr(wnd, GWL_STYLE) & UDS_HORZ) ? SpinType::LeftRight : SpinType::UpDown;
        state::state_map.emplace(wnd, WindowState{wnd_proc, spin_type});
    }
}

void remove_window(HWND wnd)
{
    if (state::state_map.contains(wnd)) {
        state::state_map[wnd].enabled = false;
    }
}

} // namespace cui::dark::spin
