#include "pch.h"

#include "config_utils.h"

namespace cui::config {

WINDOWPLACEMENT WindowPlacementAndDpi::get_adjusted_placement() const
{
    WINDOWPLACEMENT adjusted_placement{placement};

    adjusted_placement.rcNormalPosition.top = uih::scale_dpi_value(placement.rcNormalPosition.top, dpi);
    adjusted_placement.rcNormalPosition.left = uih::scale_dpi_value(placement.rcNormalPosition.left, dpi);
    adjusted_placement.rcNormalPosition.bottom = uih::scale_dpi_value(placement.rcNormalPosition.bottom, dpi);
    adjusted_placement.rcNormalPosition.right = uih::scale_dpi_value(placement.rcNormalPosition.right, dpi);

    return adjusted_placement;
}

void write_window_placement_and_dpi(
    stream_writer& stream, const WindowPlacementAndDpi& window_placement_and_dpi, abort_callback& aborter)
{
    stream.write_lendian_t(window_placement_and_dpi.placement.flags, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.showCmd, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.ptMinPosition.x, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.ptMinPosition.y, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.ptMaxPosition.x, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.ptMaxPosition.y, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.rcNormalPosition.left, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.rcNormalPosition.top, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.rcNormalPosition.right, aborter);
    stream.write_lendian_t(window_placement_and_dpi.placement.rcNormalPosition.bottom, aborter);
    stream.write_lendian_t(window_placement_and_dpi.dpi, aborter);
}

WindowPlacementAndDpi read_window_placement_and_dpi(stream_reader& stream, abort_callback& aborter)
{
    WindowPlacementAndDpi placement_and_dpi{};
    stream.read_lendian_t(placement_and_dpi.placement.flags, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.showCmd, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.ptMinPosition.x, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.ptMinPosition.y, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.ptMaxPosition.x, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.ptMaxPosition.y, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.rcNormalPosition.left, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.rcNormalPosition.top, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.rcNormalPosition.right, aborter);
    stream.read_lendian_t(placement_and_dpi.placement.rcNormalPosition.bottom, aborter);
    stream.read_lendian_t(placement_and_dpi.dpi, aborter);
    return placement_and_dpi;
}

} // namespace cui::config
