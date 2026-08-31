#pragma once

namespace cui::config {

struct WindowPlacementAndDpi {
    WINDOWPLACEMENT placement{.length = sizeof(WINDOWPLACEMENT)};
    int32_t dpi{USER_DEFAULT_SCREEN_DPI};

    WINDOWPLACEMENT get_adjusted_placement() const;
};

void write_window_placement_and_dpi(
    stream_writer& stream, const WindowPlacementAndDpi& window_placement_and_dpi, abort_callback& aborter);
WindowPlacementAndDpi read_window_placement_and_dpi(stream_reader& stream, abort_callback& aborter);

} // namespace cui::config
