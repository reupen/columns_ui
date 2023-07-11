#pragma once
#include "resource_utils.h"
#include "svg.h"

namespace cui::icons {

struct IconConfig {
    WORD light_icon_id;
    WORD dark_icon_id;
    WORD light_svg_id;
    WORD dark_svg_id;

    constexpr IconConfig(WORD light_icon_id, WORD dark_icon_id, WORD light_svg_id, WORD dark_svg_id)
        : light_icon_id(light_icon_id)
        , dark_icon_id(dark_icon_id)
        , light_svg_id(light_svg_id)
        , dark_svg_id(dark_svg_id)
    {
    }

    constexpr WORD svg_id(bool is_dark = colours::is_dark_mode_active()) const
    {
        return is_dark ? dark_svg_id : light_svg_id;
    }

    constexpr WORD icon_id(bool is_dark = colours::is_dark_mode_active()) const
    {
        return is_dark ? dark_icon_id : light_icon_id;
    }
};

#define MAKE_ICON_CONFIG_CTOR_ARGS(NAME) IDI_LIGHT_##NAME, IDI_DARK_##NAME, IDV_LIGHT_##NAME, IDV_DARK_##NAME

bool is_standard_size(int width, int height);
bool use_svg_icon(int width, int height);
wil::unique_hbitmap render_svg(IconConfig icon_config, int width, int height,
    svg_services::PixelFormat pixel_format = svg_services::PixelFormat::BGRA);
wil::unique_hicon load_icon(IconConfig icon_config, int width, int height);

namespace built_in {

constexpr IconConfig gold_star(MAKE_ICON_CONFIG_CTOR_ARGS(STARON));
constexpr IconConfig grey_star(MAKE_ICON_CONFIG_CTOR_ARGS(STAROFF));
constexpr IconConfig next(MAKE_ICON_CONFIG_CTOR_ARGS(NEXT));
constexpr IconConfig open(MAKE_ICON_CONFIG_CTOR_ARGS(OPEN));
constexpr IconConfig padlock(MAKE_ICON_CONFIG_CTOR_ARGS(PADLOCK));
constexpr IconConfig pause(MAKE_ICON_CONFIG_CTOR_ARGS(PAUSE));
constexpr IconConfig play(MAKE_ICON_CONFIG_CTOR_ARGS(PLAY));
constexpr IconConfig previous(MAKE_ICON_CONFIG_CTOR_ARGS(PREV));
constexpr IconConfig random(MAKE_ICON_CONFIG_CTOR_ARGS(RAND));
constexpr IconConfig reset(MAKE_ICON_CONFIG_CTOR_ARGS(RESET));
constexpr IconConfig stop(MAKE_ICON_CONFIG_CTOR_ARGS(STOP));
constexpr IconConfig stop_after_current(MAKE_ICON_CONFIG_CTOR_ARGS(STOP_AFTER_CURRENT));

} // namespace built_in

} // namespace cui::icons
