#include "pch.h"

#include "ng_playlist.h"

namespace cui::panels::playlist_view {

constexpr uint16_t STREAM_VERSION = 1;

void PlaylistView::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    if (get_wnd()) {
        const auto active_playlist = m_playlist_api->get_active_playlist();

        if (active_playlist != std::numeric_limits<size_t>::max())
            m_playlist_cache.set_item(active_playlist, save_scroll_position());
    }

    p_writer->write_lendian_t(STREAM_VERSION, p_abort);

    stream_writer_memblock writer_items;
    uint32_t count{};

    for (auto [playlist_id, saved_scroll_position] : m_playlist_cache) {
        if (!(saved_scroll_position && playlist_id))
            continue;

        stream_writer_memblock writer_item;
        writer_item.write_lendian_t(*playlist_id, p_abort);
        writer_item.write_lendian_t(saved_scroll_position->previous_item_index, p_abort);
        writer_item.write_lendian_t(saved_scroll_position->next_item_index, p_abort);
        writer_item.write_lendian_t(saved_scroll_position->proportional_position, p_abort);
        writer_items.write_lendian_t(gsl::narrow<uint32_t>(writer_item.m_data.size()), p_abort);
        writer_items.write(writer_item.m_data.get_ptr(), writer_item.m_data.size(), p_abort);
        ++count;
    }

    p_writer->write_lendian_t(count, p_abort);
    p_writer->write(writer_items.m_data.get_ptr(), writer_items.m_data.size(), p_abort);
}

void PlaylistView::set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    if (p_size == 0)
        return;

    const auto version = p_reader->read_lendian_t<uint16_t>(p_abort);

    if (version > STREAM_VERSION)
        throw exception_io_unsupported_format("Playlist view configuration format is not supported.");

    uint32_t count{};
    try {
        count = p_reader->read_lendian_t<uint32_t>(p_abort);
    } catch (const exception_io_data_truncation&) {
        return;
    }

    for (auto _ [[maybe_unused]] : std::ranges::views::iota(0u, count)) {
        const auto item_size = p_reader->read_lendian_t<uint32_t>(p_abort);
        std::vector<uint8_t> item_data(item_size);
        p_reader->read(item_data.data(), item_data.size(), p_abort);

        stream_reader_memblock_ref item_reader(item_data.data(), item_data.size());
        const auto playlist_id = item_reader.read_lendian_t<GUID>(p_abort);
        const auto previous_item_index = item_reader.read_lendian_t<int32_t>(p_abort);
        const auto next_item_index = item_reader.read_lendian_t<int32_t>(p_abort);
        const auto proportional_position = item_reader.read_lendian_t<double>(p_abort);
        m_initial_scroll_positions[playlist_id] = {previous_item_index, next_item_index, proportional_position};
    }
}

} // namespace cui::panels::playlist_view
