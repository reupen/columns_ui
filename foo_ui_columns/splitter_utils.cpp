#include "pch.h"

#include "splitter_utils.h"

namespace cui::splitter_utils {

auto normalise_splitter_item(const uie::splitter_item_t* item)
{
    auto normalised_item = std::make_unique<uie::splitter_item_full_v3_impl_t>();

    normalised_item->set_panel_guid(item->get_panel_guid());
    normalised_item->set_window_ptr(item->get_window_ptr());

    stream_writer_memblock panel_data;
    item->get_panel_config(&panel_data);
    normalised_item->set_panel_config_from_ptr(panel_data.m_data.get_ptr(), panel_data.m_data.get_size());

    const uie::splitter_item_full_t* item_full{};
    if (item->query(item_full)) {
        normalised_item->m_autohide = item_full->m_autohide;
        normalised_item->m_caption_orientation = item_full->m_caption_orientation;
        normalised_item->m_locked = item_full->m_locked;
        normalised_item->m_hidden = item_full->m_hidden;
        normalised_item->m_show_caption = item_full->m_show_caption;
        normalised_item->m_size = item_full->m_size;
        normalised_item->m_show_toggle_area = item_full->m_show_toggle_area;
        normalised_item->m_custom_title = item_full->m_custom_title;
        pfc::string8 title;
        item_full->get_title(title);
        normalised_item->set_title(title, title.get_length());
    }

    const uie::splitter_item_full_v2_t* item_full_v2{};
    if (item->query(item_full_v2)) {
        normalised_item->m_size_v2 = item_full_v2->m_size_v2;
        normalised_item->m_size_v2_dpi = item_full_v2->m_size_v2_dpi;
    } else {
        normalised_item->m_size_v2 = normalised_item->m_size;
        normalised_item->m_size_v2_dpi = uih::get_system_dpi_cached().cx;
    }

    const uie::splitter_item_full_v3_base_t* item_full_v3{};
    if (item->query(item_full_v3)) {
        normalised_item->m_extra_data_format_id = item_full_v3->get_extra_data_format_id();
        stream_writer_memblock_ref writer{normalised_item->m_extra_data};
        item_full_v3->get_extra_data(&writer);
    }

    return normalised_item;
}

pfc::array_t<uint8_t> serialise_splitter_item(const uie::splitter_item_full_v3_impl_t* item)
{
    stream_writer_memblock writer;
    abort_callback_dummy aborter;

    writer.write_lendian_t(item->get_panel_guid(), aborter);
    writer.write_lendian_t(item->m_autohide, aborter);
    writer.write_lendian_t(item->m_caption_orientation, aborter);
    writer.write_lendian_t(item->m_locked, aborter);
    writer.write_lendian_t(item->m_hidden, aborter);
    writer.write_lendian_t(item->m_show_caption, aborter);
    writer.write_lendian_t(item->m_show_toggle_area, aborter);
    writer.write_lendian_t(item->m_custom_title, aborter);
    writer.write_lendian_t(item->m_size_v2, aborter);
    writer.write_lendian_t(item->m_size_v2_dpi, aborter);

    pfc::string8 title;
    item->get_title(title);
    writer.write_string(title.get_ptr(), aborter);

    auto panel_data = item->get_panel_config_to_array(true);
    writer.write_lendian_t(gsl::narrow<uint32_t>(panel_data.get_size()), aborter);
    writer.write(panel_data.get_ptr(), panel_data.get_size(), aborter);

    writer.write_lendian_t(item->m_extra_data_format_id, aborter);
    writer.write_lendian_t(gsl::narrow<uint32_t>(item->m_extra_data.get_size()), aborter);
    writer.write(item->m_extra_data.get_ptr(), item->m_extra_data.get_size(), aborter);

    return writer.m_data;
}

pfc::array_t<uint8_t> serialise_splitter_item(const uie::splitter_item_t* item)
{
    auto normalised_item = normalise_splitter_item(item);
    return serialise_splitter_item(normalised_item.get());
}

std::unique_ptr<uie::splitter_item_full_v3_impl_t> deserialise_splitter_item(std::span<const uint8_t> data)
{
    auto item = std::make_unique<uie::splitter_item_full_v3_impl_t>();
    stream_reader_memblock_ref reader(data.data(), data.size());
    abort_callback_dummy aborter;

    GUID panel_guid{};
    reader.read_lendian_t(panel_guid, aborter);
    item->set_panel_guid(panel_guid);

    reader.read_lendian_t(item->m_autohide, aborter);
    reader.read_lendian_t(item->m_caption_orientation, aborter);
    reader.read_lendian_t(item->m_locked, aborter);
    reader.read_lendian_t(item->m_hidden, aborter);
    reader.read_lendian_t(item->m_show_caption, aborter);
    reader.read_lendian_t(item->m_show_toggle_area, aborter);
    reader.read_lendian_t(item->m_custom_title, aborter);
    reader.read_lendian_t(item->m_size_v2, aborter);
    reader.read_lendian_t(item->m_size_v2_dpi, aborter);

    pfc::string8 title;
    reader.read_string(title, aborter);
    item->set_title(title, title.get_length());

    uint32_t panel_data_size{};
    reader.read_lendian_t(panel_data_size, aborter);

    pfc::array_staticsize_t<uint8_t> panel_data{panel_data_size};
    reader.read(panel_data.get_ptr(), panel_data_size, aborter);
    item->set_panel_config_from_ptr(panel_data.get_ptr(), panel_data.get_size());

    reader.read_lendian_t(item->m_extra_data_format_id, aborter);
    uint32_t extra_data_size{};
    reader.read_lendian_t(extra_data_size, aborter);

    item->m_extra_data.set_size(extra_data_size);
    try {
        reader.read(item->m_extra_data.get_ptr(), extra_data_size, aborter);
    } catch (const exception_io&) {
        item->m_extra_data.set_size(0);
        throw;
    }

    return item;
}

CLIPFORMAT get_splitter_item_clipboard_format()
{
    static auto clipboard_format{
        gsl::narrow<CLIPFORMAT>(RegisterClipboardFormat(L"columns_ui_splitter_item_LvM_6LmP"))};
    return clipboard_format;
}

bool is_splitter_item_in_clipboard()
{
    return IsClipboardFormatAvailable(get_splitter_item_clipboard_format()) != 0;
}

std::unique_ptr<uie::splitter_item_full_v3_impl_t> get_splitter_item_from_clipboard()
{
    if (!is_splitter_item_in_clipboard())
        throw exception_io("Clipboard does not contain a panel");
    const auto data = uih::get_clipboard_data(get_splitter_item_clipboard_format());
    if (!data) {
        auto message = "Error getting data from clipboard: "s + helpers::get_last_win32_error_message().get_ptr();
        throw exception_io(message.c_str());
    }
    return deserialise_splitter_item({data->data(), data->size()});
}

std::unique_ptr<uie::splitter_item_full_v3_impl_t> get_splitter_item_from_clipboard_safe(HWND wnd)
{
    try {
        return get_splitter_item_from_clipboard();
    } catch (const exception_io& ex) {
        uMessageBox(wnd, ex.what(), u8"Error – Paste Panel"_pcc, MB_OK | MB_ICONERROR);
    }
    return {};
}

} // namespace cui::splitter_utils
