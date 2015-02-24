#include "stdafx.h"




fonts_manager_data::fonts_manager_data() : cfg_var(g_cfg_guid)
{
	m_common_items_entry = new entry_t(true);
	m_common_labels_entry = new entry_t(true);
	uGetIconFont(&m_common_items_entry->font);
	uGetMenuFont(&m_common_labels_entry->font);
}

void fonts_manager_data::g_on_common_font_changed(t_size mask)
{
	t_size i, count = m_callbacks.get_count();
	for (i = 0; i < count; i++)
		m_callbacks[i]->on_font_changed(mask);
}

void fonts_manager_data::deregister_common_callback(cui::fonts::common_callback * p_callback)
{
	m_callbacks.remove_item(p_callback);
}

void fonts_manager_data::register_common_callback(cui::fonts::common_callback * p_callback)
{
	m_callbacks.add_item(p_callback);
}

void fonts_manager_data::find_by_guid(const GUID & p_guid, entry_ptr_t & p_out)
{
	t_size i, count = m_entries.get_count();
	for (i = 0; i < count; i++)
	{
		if (m_entries[i]->guid == p_guid)
		{
			p_out = m_entries[i];
			return;
		}
	}
		{
			p_out = new entry_t;
			p_out->guid = p_guid;
			cui::fonts::client::ptr ptr;
			if (cui::fonts::client::create_by_guid(p_guid, ptr))
			{
				if (ptr->get_default_font_type() == cui::fonts::font_type_items)
					p_out->font_mode = cui::fonts::font_mode_common_items;
				else
					p_out->font_mode = cui::fonts::font_mode_common_labels;
			}
			m_entries.add_item(p_out);
		}
}

void fonts_manager_data::g_read_font(stream_reader * p_reader, LOGFONT & lf_out, abort_callback & p_abort)
{
	LOGFONT lf;

	p_reader->read_lendian_t(lf.lfHeight, p_abort);
	p_reader->read_lendian_t(lf.lfWidth, p_abort);
	p_reader->read_lendian_t(lf.lfEscapement, p_abort);
	p_reader->read_lendian_t(lf.lfOrientation, p_abort);
	p_reader->read_lendian_t(lf.lfWeight, p_abort);

	//meh endianness
	p_reader->read(&lf.lfItalic, 8 + sizeof(lf.lfFaceName), p_abort);

	lf_out = lf;
}

void fonts_manager_data::g_write_font(stream_writer * m_output, const LOGFONT & lfc, abort_callback & m_abort)
{
	LOGFONT lf = lfc;
	t_size face_len = pfc::wcslen_max(lf.lfFaceName, tabsize(lf.lfFaceName));

	if (face_len < tabsize(lf.lfFaceName))
	{
		WCHAR * ptr = lf.lfFaceName;
		ptr += face_len;
		memset(ptr, 0, sizeof(WCHAR)*(tabsize(lf.lfFaceName) - face_len));
	}

	m_output->write_lendian_t(lf.lfHeight, m_abort);
	m_output->write_lendian_t(lf.lfWidth, m_abort);
	m_output->write_lendian_t(lf.lfEscapement, m_abort);
	m_output->write_lendian_t(lf.lfOrientation, m_abort);
	m_output->write_lendian_t(lf.lfWeight, m_abort);

	//meh endianness
	m_output->write(&lf.lfItalic, 8 + sizeof(lf.lfFaceName), m_abort);
}

void fonts_manager_data::set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort)
{
	t_uint32 version;
	p_stream->read_lendian_t(version, p_abort);
	if (version <= cfg_version)
	{
		m_common_items_entry->read(version, p_stream, p_abort);
		m_common_labels_entry->read(version, p_stream, p_abort);
		t_size i, count;
		p_stream->read_lendian_t(count, p_abort);
		m_entries.remove_all();
		for (i = 0; i < count; i++)
		{
			entry_ptr_t ptr = new entry_t;
			ptr->read(version, p_stream, p_abort);
			m_entries.add_item(ptr);
		}
	}
}

void fonts_manager_data::get_data_raw(stream_writer * p_stream, abort_callback & p_abort)
{
	pfc::list_t<GUID> clients;
	{
		service_enum_t<cui::fonts::client> clientenum;
		cui::fonts::client::ptr ptr;
		while (clientenum.next(ptr))
			clients.add_item(ptr->get_client_guid());
	}

	pfc::array_t<bool> mask;
	t_size i, count = m_entries.get_count(), counter = 0;
	mask.set_count(count);
	for (i = 0; i < count; i++)
		if (mask[i] = clients.have_item(m_entries[i]->guid)) counter++;

	p_stream->write_lendian_t((t_uint32)cfg_version, p_abort);
	m_common_items_entry->write(p_stream, p_abort);
	m_common_labels_entry->write(p_stream, p_abort);

	p_stream->write_lendian_t(counter, p_abort);
	for (i = 0; i < count; i++)
		if (mask[i])
			m_entries[i]->write(p_stream, p_abort);
}

fonts_manager_data::entry_t::entry_t(bool b_global /*= false*/) : guid(pfc::guid_null), font_mode(cui::fonts::font_mode_system)
{
	reset_fonts();
}

void fonts_manager_data::entry_t::reset_fonts()
{
	uGetIconFont(&font);
}

void fonts_manager_data::entry_t::import(stream_reader * p_reader, t_size stream_size, t_uint32 type, abort_callback & p_abort)
{
	fcl::reader reader(p_reader, stream_size, p_abort);
	t_uint32 element_id;
	t_uint32 element_size;

	while (reader.get_remaining())
	{
		reader.read_item(element_id);
		reader.read_item(element_size);

		switch (element_id)
		{
		case identifier_guid:
			reader.read_item(guid);
			break;
		case identifier_mode:
			reader.read_item((t_uint32&)font_mode);
			break;
		case identifier_font:
			reader.read_item(font);
			break;
		default:
			reader.skip(element_size);
			break;
		};
	}
}

void fonts_manager_data::entry_t::export(stream_writer * p_stream, abort_callback & p_abort)
{
	fcl::writer out(p_stream, p_abort);
	out.write_item(identifier_guid, guid);
	out.write_item(identifier_mode, (t_uint32)font_mode);
	if (font_mode == cui::fonts::font_mode_custom)
	{
		out.write_item(identifier_font, font);
	}
}

void fonts_manager_data::entry_t::read(t_uint32 version, stream_reader * p_stream, abort_callback & p_abort)
{
	p_stream->read_lendian_t(guid, p_abort);
	p_stream->read_lendian_t((t_uint32&)font_mode, p_abort);
	g_read_font(p_stream, font, p_abort);
}

void fonts_manager_data::entry_t::write(stream_writer * p_stream, abort_callback & p_abort)
{
	p_stream->write_lendian_t(guid, p_abort);
	p_stream->write_lendian_t((t_uint32)font_mode, p_abort);
	g_write_font(p_stream, font, p_abort);
}
