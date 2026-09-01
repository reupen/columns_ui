#include "pch.h"

#include "dialog_placement.h"
#include "main_window.h"

namespace cui::config {

DialogPlacementManager dialog_placement_manager(
    {0xac661954, 0x0a16, 0x47f8, {0x90, 0x66, 0xc3, 0x80, 0x9c, 0x45, 0x6f, 0x1c}});

void DialogPlacementManager::get_data_raw(stream_writer* stream, abort_callback& aborter)
{
    for (const auto& [wnd, id] : m_open_windows)
        save_placement(id, wnd);

    stream->write_lendian_t(gsl::narrow<uint32_t>(m_placements_and_dpis.size()), aborter);

    for (const auto& [id, placement_and_dpi] : m_placements_and_dpis) {
        stream_writer_memblock placement_writer;
        placement_writer.write_lendian_t(id, aborter);
        write_window_placement_and_dpi(placement_writer, placement_and_dpi, aborter);
        stream->write_lendian_t(gsl::narrow<uint32_t>(placement_writer.m_data.get_size()), aborter);
        stream->write(placement_writer.m_data.get_ptr(), placement_writer.m_data.get_size(), aborter);
    }
}

void DialogPlacementManager::set_data_raw(stream_reader* stream, size_t size, abort_callback& aborter)
{
    m_placements_and_dpis.clear();

    const auto count = stream->read_lendian_t<uint32_t>(aborter);

    m_placements_and_dpis.reserve(count);

    for (const auto _ : ranges::views::iota(0u, count)) {
        const auto item_size = stream->read_lendian_t<uint32_t>(aborter);
        stream_reader_limited_ref placement_reader(stream, item_size);

        const auto id = placement_reader.read_lendian_t<GUID>(aborter);
        const auto placement_and_dpi = read_window_placement_and_dpi(placement_reader, aborter);
        placement_reader.flush_remaining(aborter);

        m_placements_and_dpis.insert_or_assign(id, placement_and_dpi);
    }
}

void DialogPlacementManager::register_window(const GUID& id, HWND wnd)
{
    m_open_windows.emplace(wnd, id);

    const auto iter = m_placements_and_dpis.find(id);

    if (iter == m_placements_and_dpis.end()) {
        ShowWindow(wnd, SW_SHOWNORMAL);
        return;
    }

    auto adjusted_wp = iter->second.get_adjusted_placement();
    adjusted_wp.showCmd = SW_SHOWNORMAL;

    SetWindowPlacement(wnd, &adjusted_wp);
}

void DialogPlacementManager::deregister_window(HWND wnd)
{
    const auto iter = m_open_windows.find(wnd);

    if (iter == m_open_windows.end())
        return;

    save_placement(iter->second, wnd);
    m_open_windows.erase(iter);
}

void DialogPlacementManager::save_placement(const GUID& id, HWND wnd)
{
    WindowPlacementAndDpi placement_and_dpi{};

    if (GetWindowPlacement(wnd, &placement_and_dpi.placement)) {
        placement_and_dpi.dpi = gsl::narrow<int32_t>(uih::get_system_dpi_cached().cx);
        m_placements_and_dpis.insert_or_assign(id, placement_and_dpi);
    }
}

} // namespace cui::config
