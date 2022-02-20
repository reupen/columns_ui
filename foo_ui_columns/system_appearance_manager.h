#pragma once

namespace cui::system_appearance_manager {

struct ModernColours {
    COLORREF background{};
    COLORREF foreground{};
    COLORREF accent{};
    COLORREF accent_light_1{};
};

struct EventToken {
    virtual ~EventToken() {}
};

using ModernColoursChangedHandler = std::function<void()>;

void initialise();
[[nodiscard]] std::optional<ModernColours> get_modern_colours();
[[nodiscard]] std::unique_ptr<EventToken> add_modern_colours_change_handler(ModernColoursChangedHandler event_handler);

}
