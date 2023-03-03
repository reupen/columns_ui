#include "pch.h"
#include "dark_mode.h"

namespace cui::dark {

namespace {

struct Tab {
    std::wstring text;
    RECT rc{};
    bool is_active{};
    bool is_hot{};
};

auto get_tab_hot_item_index(HWND wnd)
{
    POINT pt{};
    GetMessagePos(&pt);

    if (const auto up_down_window = GetFirstChild(wnd)) {
        RECT rect{};
        GetWindowRect(up_down_window, &rect);

        if (IsWindowVisible(up_down_window) && pt.x >= rect.left)
            return -1;
    }

    TCHITTESTINFO tchti{};
    tchti.pt = pt;
    MapWindowPoints(HWND_DESKTOP, wnd, &tchti.pt, 1);

    return TabCtrl_HitTest(wnd, &tchti);
}

std::vector<Tab> get_tabs(HWND wnd)
{
    const auto active_item_index = TabCtrl_GetCurSel(wnd);
    const auto hot_item_index = get_tab_hot_item_index(wnd);
    const auto item_count = TabCtrl_GetItemCount(wnd);
    std::array<wchar_t, 1024> buffer{};

    TCITEM tci_default{};
    tci_default.mask = TCIF_TEXT;
    tci_default.pszText = buffer.data();
    tci_default.cchTextMax = gsl::narrow<int>(buffer.size());

    // clang-format off
    return ranges::views::iota(0, item_count)
        | ranges::views::transform([&](auto index) {
            TCITEM tci{tci_default};
            TabCtrl_GetItem(wnd, index, &tci);

            RECT rc{};
            TabCtrl_GetItemRect(wnd, index, &rc);

            const bool is_active = index == active_item_index;
            const bool is_hot = index == hot_item_index;

            return Tab{{buffer.data(), wcsnlen(buffer.data(), buffer.size())}, rc, is_active, is_hot};})
        | ranges::to<std::vector>()
        | ranges::actions::stable_sort([](auto&& left, auto&& right) {
            if (left.rc.top == right.rc.top)
                return left.is_active < right.is_active;
            return left.rc.top < right.rc.top;
        });
    // clang-format on
}

} // namespace

void handle_tab_control_paint(HWND wnd)
{
    const auto items = get_tabs(wnd);

    constexpr auto is_dark = true;
    const auto border_colour = get_colour(ColourID::TabControlItemBorder, is_dark);
    const wil::unique_hpen border_pen(CreatePen(PS_SOLID, 1_spx, border_colour));
    const auto default_item_brush = get_colour_brush_lazy(ColourID::TabControlItemBackground, is_dark);
    const auto hot_item_brush = get_colour_brush_lazy(ColourID::TabControlHotItemBackground, is_dark);
    const auto hot_active_item_brush = get_colour_brush_lazy(ColourID::TabControlHotActiveItemBackground, is_dark);
    const auto active_item_brush = get_colour_brush_lazy(ColourID::TabControlActiveItemBackground, is_dark);

    PAINTSTRUCT ps{};
    const auto paint_dc = wil::BeginPaint(wnd, &ps);
    const auto buffered_dc = uih::BufferedDC(paint_dc.get(), ps.rcPaint);
    const auto _select_font = wil::SelectObject(buffered_dc.get(), GetWindowFont(wnd));
    const auto _select_pen = wil::SelectObject(buffered_dc.get(), border_pen.get());

    SetTextColor(buffered_dc.get(), get_colour(ColourID::TabControlItemText, is_dark));
    SetBkMode(buffered_dc.get(), TRANSPARENT);

    if (ps.fErase)
        FillRect(buffered_dc.get(), &ps.rcPaint, *get_colour_brush_lazy(ColourID::TabControlBackground, is_dark));

    for (auto&& [index, item] : ranges::views::enumerate(items)) {
        const auto is_new_line = index == 0 || items[index - 1].rc.top != item.rc.top;

        auto item_rect = item.rc;
        if (item.is_active) {
            item_rect.top -= 1_spx;
            // Scale and round 0.5 for these two
            item_rect.left -= uih::scale_dpi_value(1, USER_DEFAULT_SCREEN_DPI * 2);
            item_rect.right += uih::scale_dpi_value(1, USER_DEFAULT_SCREEN_DPI * 2);
        }

        RECT _intersect_rect{};
        if (!IntersectRect(&_intersect_rect, &ps.rcPaint, &item_rect))
            continue;

        // &item = item is currently required by Clang, see https://github.com/llvm/llvm-project/issues/48582
        const auto item_back_brush = [&, &item = item] {
            if (item.is_hot && item.is_active)
                return *hot_active_item_brush;

            if (item.is_hot)
                return *hot_item_brush;

            if (item.is_active)
                return *active_item_brush;

            return *default_item_brush;
        }();
        FillRect(buffered_dc.get(), &item_rect, item_back_brush);

        MoveToEx(buffered_dc.get(), item_rect.right, item_rect.bottom, nullptr);
        LineTo(buffered_dc.get(), item_rect.right, item_rect.top);
        LineTo(buffered_dc.get(), item_rect.left, item_rect.top);
        if (is_new_line || item.is_active) {
            LineTo(buffered_dc.get(), item_rect.left, item_rect.bottom);
        }

        // Position using original rect, but shift up 1px if it's the active tab
        RECT text_rect = {item.rc.left, item.rc.top - (item.is_active ? 1_spx : 0), item.rc.right,
            item.rc.bottom - (item.is_active ? 1_spx : 0)};

        DrawTextEx(buffered_dc.get(), const_cast<wchar_t*>(item.text.data()), gsl::narrow<int>(item.text.length()),
            &text_rect, DT_CENTER | DT_HIDEPREFIX | DT_SINGLELINE | DT_VCENTER, nullptr);
    }
}

} // namespace cui::dark
