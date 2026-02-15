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

void initialise();
[[nodiscard]] std::optional<ModernColours> get_modern_colours();
[[nodiscard]] mmh::EventToken::Ptr add_modern_colours_change_handler(mmh::GenericEventHandler event_handler);
[[nodiscard]] mmh::EventToken::Ptr add_display_changed_handler(mmh::GenericEventHandler event_handler);
[[nodiscard]] bool is_dark_mode_available();
[[nodiscard]] bool is_dark_mode_enabled();

[[nodiscard]] bool is_font_smoothing_enabled();
[[nodiscard]] bool is_cleartype_enabled();
} // namespace cui::system_appearance_manager
