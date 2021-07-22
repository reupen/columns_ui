#include "stdafx.h"

#include "rebar_band.h"

namespace cui::rebar {

void RebarBandState::export_to_fcl_stream(stream_writer* writer, t_uint32 fcl_type, abort_callback& aborter) const
{
    uie::window_ptr ptr;

    if (uie::window::create_by_guid(m_guid, ptr)) {
        // if (fcl_type==cui::fcl::type_public)
        try {
            ptr->set_config_from_ptr(m_config.get_ptr(), m_config.get_size(), aborter);
        } catch (const exception_io&) {
        } // FIXME: Why?
    } else
        throw cui::fcl::exception_missing_panel();

    stream_writer_memblock w;
    if (fcl_type == cui::fcl::type_public)
        ptr->export_config(&w, aborter);
    else
        ptr->get_config(&w, aborter);
    writer->write_lendian_t(m_guid, aborter);
    writer->write_lendian_t(m_width.get_scaled_value(), aborter);
    writer->write_lendian_t(m_break_before_band, aborter);
    const auto size = gsl::narrow<uint32_t>(w.m_data.get_size());
    writer->write_lendian_t(size, aborter);
    writer->write(w.m_data.get_ptr(), size, aborter);
}

void RebarBandState::import_from_fcl_stream(stream_reader* reader, t_uint32 fcl_type, abort_callback& aborter)
{
    reader->read_lendian_t(m_guid, aborter);
    uint32_t width_;
    reader->read_lendian_t(width_, aborter);
    m_width = width_;
    reader->read_lendian_t(m_break_before_band, aborter);
    uint32_t mem_size;
    reader->read_lendian_t(mem_size, aborter);

    if (mem_size) {
        pfc::array_t<t_uint8> data;
        data.set_size(mem_size);
        reader->read(data.get_ptr(), mem_size, aborter);

        if (fcl_type == cui::fcl::type_public) {
            uie::window_ptr ptr;

            if (uie::window::create_by_guid(m_guid, ptr)) {
                stream_reader_memblock_ref panel_config_reader(data.get_ptr(), data.get_size());
                ptr->import_config(&panel_config_reader, data.get_size(), aborter);
                stream_writer_memblock_ref writer(m_config, true);
                ptr->get_config(&writer, aborter);
            }
        } else
            m_config = data;
    }
}

void RebarBandState::write_to_stream(stream_writer* writer, abort_callback& aborter) const
{
    writer->write_lendian_t(m_guid, aborter);
    writer->write_lendian_t(m_width.get_scaled_value(), aborter);
    writer->write_lendian_t(m_break_before_band, aborter);
    const auto size = gsl::narrow<uint32_t>(m_config.get_size());
    writer->write_lendian_t(size, aborter);
    writer->write(m_config.get_ptr(), size, aborter);
}

void RebarBandState::read_from_stream(stream_reader* reader, abort_callback& aborter)
{
    reader->read_lendian_t(m_guid, aborter);
    uint32_t width_;
    reader->read_lendian_t(width_, aborter);
    m_width = width_;
    reader->read_lendian_t(m_break_before_band, aborter);

    uint32_t mem_size;
    reader->read_lendian_t(mem_size, aborter);

    if (mem_size) {
        m_config.set_size(mem_size);
        reader->read(m_config.get_ptr(), mem_size, aborter);
    }
}

void RebarBandState::write_extra(stream_writer* writer, abort_callback& aborter) const
{
    writer->write_lendian_t(m_width.value, aborter);
    writer->write_lendian_t(m_width.dpi, aborter);
}

void RebarBandState::read_extra(stream_reader* reader, abort_callback& aborter)
{
    reader->read_lendian_t(m_width.value, aborter);
    reader->read_lendian_t(m_width.dpi, aborter);
}

} // namespace cui::rebar
