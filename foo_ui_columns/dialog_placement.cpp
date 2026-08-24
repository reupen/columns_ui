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

    stream->write_lendian_t(gsl::narrow<uint32_t>(m_dialog_placements.size()), aborter);

    for (const auto& [id, dialog_placement] : m_dialog_placements) {
        stream_writer_memblock placement_writer;
        placement_writer.write_lendian_t(id, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.flags, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.showCmd, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.ptMinPosition.x, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.ptMinPosition.y, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.ptMaxPosition.x, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.ptMaxPosition.y, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.rcNormalPosition.left, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.rcNormalPosition.top, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.rcNormalPosition.right, aborter);
        placement_writer.write_lendian_t(dialog_placement.placement.rcNormalPosition.bottom, aborter);
        placement_writer.write_lendian_t(dialog_placement.dpi, aborter);

        stream->write_lendian_t(gsl::narrow<uint32_t>(placement_writer.m_data.get_size()), aborter);
        stream->write(placement_writer.m_data.get_ptr(), placement_writer.m_data.get_size(), aborter);
    }
}

void DialogPlacementManager::set_data_raw(stream_reader* stream, size_t size, abort_callback& aborter)
{
    m_dialog_placements.clear();

    const auto count = stream->read_lendian_t<uint32_t>(aborter);

    m_dialog_placements.reserve(count);

    for (const auto _ : ranges::views::iota(0u, count)) {
        const auto item_size = stream->read_lendian_t<uint32_t>(aborter);
        stream_reader_limited_ref placement_reader(stream, item_size);

        DialogPlacement dialog_placement{};
        const auto id = placement_reader.read_lendian_t<GUID>(aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.flags, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.showCmd, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.ptMinPosition.x, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.ptMinPosition.y, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.ptMaxPosition.x, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.ptMaxPosition.y, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.rcNormalPosition.left, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.rcNormalPosition.top, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.rcNormalPosition.right, aborter);
        placement_reader.read_lendian_t(dialog_placement.placement.rcNormalPosition.bottom, aborter);
        placement_reader.read_lendian_t(dialog_placement.dpi, aborter);
        placement_reader.flush_remaining(aborter);

        m_dialog_placements.insert_or_assign(id, dialog_placement);
    }
}

void DialogPlacementManager::register_window(const GUID& id, HWND wnd)
{
    m_open_windows.emplace(wnd, id);

    if (!remember_window_pos()) {
        ShowWindow(wnd, SW_SHOWNORMAL);
        return;
    }

    const auto iter = m_dialog_placements.find(id);

    if (iter == m_dialog_placements.end()) {
        ShowWindow(wnd, SW_SHOWNORMAL);
        return;
    }

    const auto& original_wp = iter->second.placement;
    const auto original_dpi = iter->second.dpi;

    WINDOWPLACEMENT adjusted_wp{original_wp};

    adjusted_wp.rcNormalPosition.top = uih::scale_dpi_value(original_wp.rcNormalPosition.top, original_dpi);
    adjusted_wp.rcNormalPosition.left = uih::scale_dpi_value(original_wp.rcNormalPosition.left, original_dpi);
    adjusted_wp.rcNormalPosition.bottom = uih::scale_dpi_value(original_wp.rcNormalPosition.bottom, original_dpi);
    adjusted_wp.rcNormalPosition.right = uih::scale_dpi_value(original_wp.rcNormalPosition.right, original_dpi);
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
    if (!remember_window_pos())
        return;

    DialogPlacement dialog_placement{};

    if (GetWindowPlacement(wnd, &dialog_placement.placement)) {
        dialog_placement.dpi = gsl::narrow<int32_t>(uih::get_system_dpi_cached().cx);
        m_dialog_placements.insert_or_assign(id, dialog_placement);
    }
}

} // namespace cui::config
