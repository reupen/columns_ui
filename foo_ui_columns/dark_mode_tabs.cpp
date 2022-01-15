#include "stdafx.h"
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
    TCHITTESTINFO tchti{};
    GetMessagePos(&tchti.pt);
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
    tci_default.cchTextMax = buffer.size();

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
    const auto default_item_brush = get_colour_brush(ColourID::TabControlItemBackground, is_dark);
    const auto hot_item_brush = get_colour_brush(ColourID::TabControlHotItemBackground, is_dark);
    const auto hot_active_item_brush = get_colour_brush(ColourID::TabControlHotActiveItemBackground, is_dark);
    const auto active_item_brush = get_colour_brush(ColourID::TabControlActiveItemBackground, is_dark);

    PAINTSTRUCT ps{};
    const auto dc = wil::BeginPaint(wnd, &ps);
    const auto _select_font = wil::SelectObject(dc.get(), GetWindowFont(wnd));
    const auto _select_pen = wil::SelectObject(dc.get(), border_pen.get());

    SetTextColor(dc.get(), get_colour(ColourID::TabControlItemText, is_dark));
    SetBkMode(dc.get(), TRANSPARENT);

    RECT client_rect{};
    GetClientRect(wnd, &client_rect);

    if (ps.fErase)
        FillRect(dc.get(), &client_rect, *get_colour_brush(ColourID::TabControlBackground, is_dark));

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

        const auto item_back_brush = [&] {
            if (item.is_hot && item.is_active)
                return *hot_active_item_brush;

            if (item.is_hot)
                return *hot_item_brush;

            if (item.is_active)
                return *active_item_brush;

            return *default_item_brush;
        }();
        FillRect(dc.get(), &item_rect, item_back_brush);

        MoveToEx(dc.get(), item_rect.right, item_rect.bottom, nullptr);
        LineTo(dc.get(), item_rect.right, item_rect.top);
        LineTo(dc.get(), item_rect.left, item_rect.top);
        if (is_new_line || item.is_active) {
            LineTo(dc.get(), item_rect.left, item_rect.bottom);
        }

        SIZE sz{};
        GetTextExtentPoint32(dc.get(), item.text.data(), gsl::narrow<int>(item.text.length()), &sz);

        // Position using original rect, but shift up 1px if it's the active tab
        const auto x = item.rc.left + (RECT_CX(item.rc) - sz.cx) / 2;
        const auto y = item.rc.top + (RECT_CY(item.rc) - sz.cy) / 2 - (item.is_active ? 1_spx : 0);

        ExtTextOut(dc.get(), x, y, ETO_CLIPPED, &item_rect, item.text.data(), item.text.length(), nullptr);
    }
}

} // namespace cui::dark
