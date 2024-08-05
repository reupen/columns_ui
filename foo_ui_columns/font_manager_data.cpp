#include "pch.h"

#include "font_utils.h"
#include "font_manager_data.h"

FontManagerData::FontManagerData() : cfg_var(g_cfg_guid)
{
    m_common_items_entry = std::make_shared<Entry>();
    uGetIconFont(&m_common_items_entry->font_description.log_font);
    m_common_items_entry->font_description.estimate_point_and_dip_size();

    m_common_labels_entry = std::make_shared<Entry>();
    uGetMenuFont(&m_common_labels_entry->font_description.log_font);
    m_common_labels_entry->font_description.estimate_point_and_dip_size();
}

void FontManagerData::g_on_common_font_changed(uint32_t mask)
{
    size_t count = m_callbacks.get_count();
    for (size_t i = 0; i < count; i++)
        m_callbacks[i]->on_font_changed(mask);
}

void FontManagerData::deregister_common_callback(cui::fonts::common_callback* p_callback)
{
    m_callbacks.remove_item(p_callback);
}

void FontManagerData::register_common_callback(cui::fonts::common_callback* p_callback)
{
    m_callbacks.add_item(p_callback);
}

FontManagerData::entry_ptr_t FontManagerData::find_by_guid(GUID id)
{
    for (auto&& entry : m_entries) {
        if (entry->guid == id) {
            return entry;
        }
    }

    auto entry = std::make_shared<Entry>();
    entry->guid = id;
    cui::fonts::client::ptr ptr;

    if (cui::fonts::client::create_by_guid(id, ptr)) {
        if (ptr->get_default_font_type() == cui::fonts::font_type_items)
            entry->font_mode = cui::fonts::font_mode_common_items;
        else
            entry->font_mode = cui::fonts::font_mode_common_labels;
    }

    m_entries.add_item(entry);

    return entry;
}

cui::fonts::FontDescription FontManagerData::resolve_font_description(const entry_ptr_t& entry)
{
    const auto is_common_items = entry->font_mode == cui::fonts::font_mode_common_items;
    const auto is_common_labels = entry->font_mode == cui::fonts::font_mode_common_labels;

    const auto resolved_entry = [&] {
        if (is_common_items)
            return m_common_items_entry;

        if (is_common_labels)
            return m_common_labels_entry;

        return entry;
    }();

    if (resolved_entry->font_mode == cui::fonts::font_mode_system) {
        const auto system_font = is_common_items ? cui::fonts::get_icon_font_for_dpi(USER_DEFAULT_SCREEN_DPI)
                                                 : cui::fonts::get_menu_font_for_dpi(USER_DEFAULT_SCREEN_DPI);

        cui::fonts::FontDescription description{system_font.log_font};
        description.fill_wss();
        const auto point_size = system_font.size * 72.0f / gsl::narrow_cast<float>(USER_DEFAULT_SCREEN_DPI);
        description.point_size_tenths = gsl::narrow_cast<int>(point_size * 10.0f + 0.5f);
        description.dip_size = system_font.size;
        return description;
    }

    resolved_entry->font_description.fill_wss();
    return resolved_entry->font_description;
}

void FontManagerData::set_data_raw(stream_reader* p_stream, size_t p_sizehint, abort_callback& p_abort)
{
    uint32_t version;
    p_stream->read_lendian_t(version, p_abort);
    if (version > cfg_version)
        return;

    m_common_items_entry->read(version, p_stream, p_abort);
    m_common_labels_entry->read(version, p_stream, p_abort);
    const auto count = p_stream->read_lendian_t<uint32_t>(p_abort);
    m_entries.remove_all();

    for (size_t i = 0; i < count; i++) {
        entry_ptr_t ptr = std::make_shared<Entry>();
        ptr->read(version, p_stream, p_abort);
        m_entries.add_item(ptr);
    }

    try {
        m_common_items_entry->read_extra_data(p_stream, p_abort);
    } catch (const exception_io_data_truncation&) {
        return;
    }

    m_common_labels_entry->read_extra_data(p_stream, p_abort);

    for (auto&& entry : m_entries) {
        entry->read_extra_data(p_stream, p_abort);
    }

    try {
        m_common_items_entry->read_extra_data_v2(p_stream, p_abort);
    } catch (const exception_io_data_truncation&) {
        return;
    }

    m_common_labels_entry->read_extra_data_v2(p_stream, p_abort);

    for (auto&& entry : m_entries) {
        entry->read_extra_data_v2(p_stream, p_abort);
    }
}

void FontManagerData::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
{
    pfc::list_t<GUID> clients;
    {
        service_enum_t<cui::fonts::client> client_enum;
        cui::fonts::client::ptr ptr;
        while (client_enum.next(ptr))
            clients.add_item(ptr->get_client_guid());
    }

    pfc::array_t<bool> mask;

    const auto count = gsl::narrow<uint32_t>(m_entries.get_count());
    uint32_t counter = 0;
    mask.set_count(count);

    for (auto&& i : ranges::views::iota(size_t{0}, count))
        if ((mask[i] = clients.have_item(m_entries[i]->guid)))
            counter++;

    p_stream->write_lendian_t(static_cast<uint32_t>(cfg_version), p_abort);
    m_common_items_entry->write(p_stream, p_abort);
    m_common_labels_entry->write(p_stream, p_abort);
    p_stream->write_lendian_t(counter, p_abort);

    for (auto&& i : ranges::views::iota(size_t{0}, count))
        if (mask[i])
            m_entries[i]->write(p_stream, p_abort);

    m_common_items_entry->write_extra_data(p_stream, p_abort);
    m_common_labels_entry->write_extra_data(p_stream, p_abort);

    for (auto&& i : ranges::views::iota(size_t{0}, count))
        if (mask[i])
            m_entries[i]->write_extra_data(p_stream, p_abort);

    m_common_items_entry->write_extra_data_v2(p_stream, p_abort);
    m_common_labels_entry->write_extra_data_v2(p_stream, p_abort);

    for (auto&& i : ranges::views::iota(size_t{0}, count))
        if (mask[i])
            m_entries[i]->write_extra_data_v2(p_stream, p_abort);
}

FontManagerData::Entry::Entry()
{
    reset_fonts();
}

void FontManagerData::Entry::reset_fonts()
{
    uGetIconFont(&font_description.log_font);
    font_description.estimate_point_and_dip_size();
}

void FontManagerData::Entry::import(stream_reader* p_reader, size_t stream_size, uint32_t type, abort_callback& p_abort)
{
    fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
    uint32_t element_id;
    uint32_t element_size;

    while (reader.get_remaining()) {
        reader.read_item(element_id);
        reader.read_item(element_size);

        switch (element_id) {
        case identifier_guid:
            reader.read_item(guid);
            break;
        case identifier_mode:
            reader.read_item((uint32_t&)font_mode);
            break;
        case identifier_font:
            reader.read_item(font_description.log_font);
            font_description.estimate_point_and_dip_size();
            break;
        case identifier_point_size_tenths:
            reader.read_item(font_description.point_size_tenths);
            break;
        default:
            reader.skip(element_size);
            break;
        }
    }
}

void FontManagerData::Entry::_export(stream_writer* p_stream, abort_callback& p_abort)
{
    fbh::fcl::Writer out(p_stream, p_abort);
    out.write_item(identifier_guid, guid);
    out.write_item(identifier_mode, (uint32_t)font_mode);
    if (font_mode == cui::fonts::font_mode_custom) {
        out.write_item(identifier_font, font_description.log_font);
    }
    out.write_item(identifier_point_size_tenths, font_description.point_size_tenths);
}

void FontManagerData::Entry::read(uint32_t version, stream_reader* p_stream, abort_callback& p_abort)
{
    p_stream->read_lendian_t(guid, p_abort);
    p_stream->read_lendian_t(reinterpret_cast<uint32_t&>(font_mode), p_abort);
    font_description.log_font = cui::fonts::read_font(p_stream, p_abort);
    font_description.estimate_point_and_dip_size();
}

void FontManagerData::Entry::read_extra_data(stream_reader* stream, abort_callback& aborter)
{
    const auto size = stream->read_lendian_t<uint32_t>(aborter);
    std::vector<uint8_t> data(size);
    stream->read(data.data(), data.size(), aborter);

    stream_reader_memblock_ref extra_reader(data.data(), data.size());

    extra_reader.read_lendian_t(font_description.point_size_tenths, aborter);
    font_description.estimate_dip_size();
}

void FontManagerData::Entry::read_extra_data_v2(stream_reader* stream, abort_callback& aborter)
{
    const auto size = stream->read_lendian_t<uint32_t>(aborter);
    std::vector<uint8_t> data(size);
    stream->read(data.data(), data.size(), aborter);

    stream_reader_memblock_ref extra_reader(data.data(), data.size());

    const auto has_wss = extra_reader.read_lendian_t<bool>(aborter);

    if (!has_wss)
        return;

    cui::fonts::WeightStretchStyle wss;
    wss.family_name = mmh::to_utf16(mmh::to_string_view(extra_reader.read_string(aborter)));
    wss.weight = static_cast<DWRITE_FONT_WEIGHT>(extra_reader.read_lendian_t<int32_t>(aborter));
    wss.stretch = static_cast<DWRITE_FONT_STRETCH>(extra_reader.read_lendian_t<int32_t>(aborter));
    wss.style = static_cast<DWRITE_FONT_STYLE>(extra_reader.read_lendian_t<int32_t>(aborter));
    font_description.dip_size = extra_reader.read_lendian_t<float>(aborter);
    font_description.wss = wss;
}

LOGFONT FontManagerData::Entry::get_normalised_font(unsigned dpi)
{
    LOGFONT lf{font_description.log_font};
    lf.lfHeight = -MulDiv(font_description.point_size_tenths, dpi, 720);
    return lf;
}

void FontManagerData::Entry::write(stream_writer* p_stream, abort_callback& p_abort)
{
    p_stream->write_lendian_t(guid, p_abort);
    p_stream->write_lendian_t((uint32_t)font_mode, p_abort);
    cui::fonts::write_font(p_stream, font_description.log_font, p_abort);
}

void FontManagerData::Entry::write_extra_data(stream_writer* stream, abort_callback& aborter) const
{
    stream_writer_memblock item_stream;
    item_stream.write_lendian_t(font_description.point_size_tenths, aborter);

    stream->write_lendian_t(gsl::narrow<uint32_t>(item_stream.m_data.get_size()), aborter);
    stream->write(item_stream.m_data.get_ptr(), item_stream.m_data.get_size(), aborter);
}

void FontManagerData::Entry::write_extra_data_v2(stream_writer* stream, abort_callback& aborter) const
{
    stream_writer_memblock item_stream;

    if (font_description.wss) {
        item_stream.write_lendian_t(true, aborter);
        const auto& wss = font_description.wss;
        const auto family_name = mmh::to_utf8(wss->family_name);
        item_stream.write_string(family_name, aborter);
        item_stream.write_lendian_t(static_cast<int32_t>(wss->weight), aborter);
        item_stream.write_lendian_t(static_cast<int32_t>(wss->stretch), aborter);
        item_stream.write_lendian_t(static_cast<int32_t>(wss->style), aborter);
        item_stream.write_lendian_t(font_description.dip_size, aborter);
    } else {
        item_stream.write_lendian_t(false, aborter);
    }

    stream->write_lendian_t(gsl::narrow<uint32_t>(item_stream.m_data.get_size()), aborter);
    stream->write(item_stream.m_data.get_ptr(), item_stream.m_data.get_size(), aborter);
}
