#pragma once

namespace cui::system_appearance_manager {

struct ModernColours {
    COLORREF background{};
    COLORREF foreground{};
    COLORREF accent{};
    COLORREF accent_light_1{};

    bool is_dark() const
    {
        return (GetRValue(background) + GetGValue(background) + GetBValue(background))
            < (GetRValue(foreground) + GetGValue(foreground) + GetBValue(foreground));
    }
};

struct EventToken {
    virtual ~EventToken() {}
};

using ModernColoursChangedHandler = std::function<void()>;

void initialise();
[[nodiscard]] std::optional<ModernColours> get_modern_colours();
[[nodiscard]] std::unique_ptr<EventToken> add_modern_colours_change_handler(ModernColoursChangedHandler event_handler);
[[nodiscard]] bool is_dark_mode_available();
[[nodiscard]] bool is_dark_mode_enabled();
}
