#include "stdafx.h"

#include "font_utils.h"
#include "fonts_manager_data.h"

FontsManagerData::FontsManagerData() : cfg_var(g_cfg_guid)
{
    m_common_items_entry = std::make_shared<Entry>();
    uGetIconFont(&m_common_items_entry->font_description.log_font);
    m_common_items_entry->font_description.estimate_point_size();

    m_common_labels_entry = std::make_shared<FontsManagerData::Entry>();
    uGetMenuFont(&m_common_labels_entry->font_description.log_font);
    m_common_labels_entry->font_description.estimate_point_size();
}

void FontsManagerData::g_on_common_font_changed(t_size mask)
{
    t_size count = m_callbacks.get_count();
    for (t_size i = 0; i < count; i++)
        m_callbacks[i]->on_font_changed(mask);
}

void FontsManagerData::deregister_common_callback(cui::fonts::common_callback* p_callback)
{
    m_callbacks.remove_item(p_callback);
}

void FontsManagerData::register_common_callback(cui::fonts::common_callback* p_callback)
{
    m_callbacks.add_item(p_callback);
}

void FontsManagerData::find_by_guid(const GUID& p_guid, entry_ptr_t& p_out)
{
    t_size count = m_entries.get_count();
    for (t_size i = 0; i < count; i++) {
        if (m_entries[i]->guid == p_guid) {
            p_out = m_entries[i];
            return;
        }
    }
    {
        p_out = std::make_shared<FontsManagerData::Entry>();
        p_out->guid = p_guid;
        cui::fonts::client::ptr ptr;
        if (cui::fonts::client::create_by_guid(p_guid, ptr)) {
            if (ptr->get_default_font_type() == cui::fonts::font_type_items)
                p_out->font_mode = cui::fonts::font_mode_common_items;
            else
                p_out->font_mode = cui::fonts::font_mode_common_labels;
        }
        m_entries.add_item(p_out);
    }
}

void FontsManagerData::set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort)
{
    t_uint32 version;
    p_stream->read_lendian_t(version, p_abort);
    if (version <= cfg_version) {
        m_common_items_entry->read(version, p_stream, p_abort);
        m_common_labels_entry->read(version, p_stream, p_abort);
        t_size count;
        p_stream->read_lendian_t(count, p_abort);
        m_entries.remove_all();
        for (t_size i = 0; i < count; i++) {
            entry_ptr_t ptr = std::make_shared<Entry>();
            ptr->read(version, p_stream, p_abort);
            m_entries.add_item(ptr);
        }
        try {
            m_common_items_entry->read_extra_data(p_stream, p_abort);
            m_common_labels_entry->read_extra_data(p_stream, p_abort);

            for (auto&& entry : m_entries) {
                entry->read_extra_data(p_stream, p_abort);
            }
        } catch (const exception_io_data_truncation&) {
        }
    }
}

void FontsManagerData::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
{
    pfc::list_t<GUID> clients;
    {
        service_enum_t<cui::fonts::client> client_enum;
        cui::fonts::client::ptr ptr;
        while (client_enum.next(ptr))
            clients.add_item(ptr->get_client_guid());
    }

    pfc::array_t<bool> mask;

    const auto count = m_entries.get_count();
    size_t counter = 0;
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
}

FontsManagerData::Entry::Entry()
{
    reset_fonts();
}

void FontsManagerData::Entry::reset_fonts()
{
    uGetIconFont(&font_description.log_font);
    font_description.estimate_point_size();
}

void FontsManagerData::Entry::import(
    stream_reader* p_reader, t_size stream_size, t_uint32 type, abort_callback& p_abort)
{
    fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
    t_uint32 element_id;
    t_uint32 element_size;

    while (reader.get_remaining()) {
        reader.read_item(element_id);
        reader.read_item(element_size);

        switch (element_id) {
        case identifier_guid:
            reader.read_item(guid);
            break;
        case identifier_mode:
            reader.read_item((t_uint32&)font_mode);
            break;
        case identifier_font:
            reader.read_item(font_description.log_font);
            font_description.estimate_point_size();
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

void FontsManagerData::Entry::_export(stream_writer* p_stream, abort_callback& p_abort)
{
    fbh::fcl::Writer out(p_stream, p_abort);
    out.write_item(identifier_guid, guid);
    out.write_item(identifier_mode, (t_uint32)font_mode);
    if (font_mode == cui::fonts::font_mode_custom) {
        out.write_item(identifier_font, font_description.log_font);
    }
    out.write_item(identifier_point_size_tenths, font_description.point_size_tenths);
}

void FontsManagerData::Entry::read(t_uint32 version, stream_reader* p_stream, abort_callback& p_abort)
{
    p_stream->read_lendian_t(guid, p_abort);
    p_stream->read_lendian_t((t_uint32&)font_mode, p_abort);
    font_description.log_font = cui::fonts::read_font(p_stream, p_abort);
    font_description.estimate_point_size();
}

void FontsManagerData::Entry::read_extra_data(stream_reader* stream, abort_callback& aborter)
{
    uint32_t size{};
    stream->read_lendian_t(size, aborter);
    stream_reader_limited_ref limited_reader(stream, size);

    limited_reader.read_lendian_t(font_description.point_size_tenths, aborter);
}

LOGFONT FontsManagerData::Entry::get_normalised_font(unsigned dpi)
{
    LOGFONT lf{font_description.log_font};
    lf.lfHeight = -MulDiv(font_description.point_size_tenths, dpi, 720);
    return lf;
}

void FontsManagerData::Entry::write(stream_writer* p_stream, abort_callback& p_abort)
{
    p_stream->write_lendian_t(guid, p_abort);
    p_stream->write_lendian_t((t_uint32)font_mode, p_abort);
    cui::fonts::write_font(p_stream, font_description.log_font, p_abort);
}

void FontsManagerData::Entry::write_extra_data(stream_writer* stream, abort_callback& aborter)
{
    stream_writer_memblock item_stream;
    item_stream.write_lendian_t(font_description.point_size_tenths, aborter);

    stream->write_lendian_t(gsl::narrow<uint32_t>(item_stream.m_data.get_size()), aborter);
    stream->write(item_stream.m_data.get_ptr(), item_stream.m_data.get_size(), aborter);
}
