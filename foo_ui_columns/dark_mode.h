#pragma once

/**
 * Note: Dark mode is a work in progress. Functions may be moved to more appropriate homes
 * at a later date.
 */

namespace cui::dark {

enum class ColourID {
    CheckboxDisabledText,
    CheckboxText,
    ComboBoxEditBackground,
    ComboBoxEditText,
    EditBackground,
    LayoutBackground,
    PanelCaptionText,
    PanelCaptionBackground,
    RebarBackground,
    RebarBandBorder,
    SpinBackground,
    SpinBuddyButtonBackground,
    SpinBuddyButtonBorder,
    SpinButtonArrow,
    SpinButtonBackground,
    SpinButtonBorder,
    SpinDisabledButtonBackground,
    SpinDisabledBuddyButtonBackground,
    SpinDisabledBuddyButtonBorder,
    SpinHotBuddyButtonBackground,
    SpinHotButtonBackground,
    SpinPressedBuddyButtonBackground,
    SpinPressedButtonBackground,
    StatusBarBackground,
    StatusBarText,
    StatusPaneTopLine,
    StatusPaneBackground,
    StatusPaneText,
    TabControlBackground,
    TabControlItemBackground,
    TabControlItemText,
    TabControlItemBorder,
    TabControlActiveItemBackground,
    TabControlHotItemBackground,
    TabControlHotActiveItemBackground,
    ToolbarDivider,
    ToolbarFlatHotBackground,
    ToolbarFlatHotText,
    TrackbarChannel,
    TrackbarThumb,
    TrackbarHotThumb,
    TrackbarDisabledThumb,
    TreeViewBackground,
    TreeViewText,
    VolumeChannelTopEdge,
    VolumeChannelBottomAndRightEdge,
    VolumePopupBackground,
    VolumePopupBorder,
    VolumePopupText,
};

template <class Object>
class LazyObject {
public:
    explicit LazyObject(std::function<Object()> factory) : m_factory(factory) {}

    Object& operator*() const
    {
        if (!m_object)
            m_object = m_factory();

        return *m_object;
    }

private:
    mutable std::optional<Object> m_object;
    std::function<Object()> m_factory;
};

template <class Resource>
class LazyResource {
public:
    explicit LazyResource(std::function<Resource()> factory) : m_resource(factory) {}

    auto operator*() const { return (*m_resource).get(); }

private:
    mutable LazyObject<Resource> m_resource;
};

[[nodiscard]] bool does_os_support_dark_mode();
[[nodiscard]] bool are_private_apis_allowed();
[[nodiscard]] bool is_native_dark_spin_available();

enum class PreferredAppMode : int {
    NotSet,
    System,
    Dark,
    Light
};

void set_app_mode(PreferredAppMode mode);
void set_titlebar_mode(HWND wnd, bool is_dark);
void force_titlebar_redraw(HWND wnd);

[[nodiscard]] COLORREF get_dark_colour(ColourID colour_id);
[[nodiscard]] Gdiplus::Color get_dark_gdiplus_colour(ColourID colour_id);
[[nodiscard]] COLORREF get_colour(ColourID colour_id, bool is_dark);
[[nodiscard]] wil::unique_hbrush get_colour_brush(ColourID colour_id, bool is_dark);
[[nodiscard]] LazyResource<wil::unique_hbrush> get_colour_brush_lazy(ColourID colour_id, bool is_dark);

[[nodiscard]] COLORREF get_dark_system_colour(int system_colour_id);
[[nodiscard]] COLORREF get_system_colour(int system_colour_id, bool is_dark);
[[nodiscard]] wil::unique_hbrush get_system_colour_brush(int system_colour_id, bool is_dark);

void draw_layout_background(HWND wnd, HDC dc);
void handle_modern_background_paint(HWND wnd, HWND wnd_button, bool is_dark);

} // namespace cui::dark
