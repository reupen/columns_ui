#pragma once

/**
 * Note: Dark mode is a work in progress. Functions may be moved to more appropriate homes
 * at a later date.
 */

namespace cui::dark {

enum class ColourID {
    PanelCaptionBackground,
    TabControlBackground,
    TabControlItemBackground,
    TabControlItemText,
    TabControlItemBorder,
    TabControlActiveItemBackground,
    TabControlHotItemBackground,
    TabControlHotActiveItemBackground,
};

template <class Object>
class LazyObject {
public:
    LazyObject(std::function<Object()> factory) : m_factory(factory) {}

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
    LazyResource(std::function<Resource()> factory) : m_resource(factory) {}

    auto operator*() const { return (*m_resource).get(); }

private:
    mutable LazyObject<Resource> m_resource;
};

/**
 * Temporary compile-time flag controlling whether dark mode is enabled.
 */
[[nodiscard]] bool is_dark_mode_enabled();

void enable_dark_mode_for_app();
void enable_top_level_non_client_dark_mode(HWND wnd);

[[nodiscard]] COLORREF get_colour(ColourID colour_id, bool is_dark);
[[nodiscard]] wil::unique_hbrush get_colour_brush(ColourID colour_id, bool is_dark);
[[nodiscard]] LazyResource<wil::unique_hbrush> get_colour_brush_lazy(ColourID colour_id, bool is_dark);

[[nodiscard]] COLORREF get_system_colour(int system_colour_id, bool is_dark);
[[nodiscard]] wil::unique_hbrush get_system_colour_brush(int system_colour_id, bool is_dark);

} // namespace cui::dark
