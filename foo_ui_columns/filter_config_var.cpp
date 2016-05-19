#include "stdafx.h"
#include "filter.h"

namespace filter_panel {

	void cfg_fields_t::set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort)
	{
		t_uint32 version;
		p_stream->read_lendian_t(version, p_abort);
		if (version <= stream_version_current)
		{
			t_uint32 count, i;
			p_stream->read_lendian_t(count, p_abort);
			set_count(count);
			for (i = 0; i<count; i++)
			{
				p_stream->read_string((*this)[i].m_name, p_abort);
				p_stream->read_string((*this)[i].m_field, p_abort);
			}

			uint32_t sub_stream_version;
			bool sub_stream_version_read = false;
			try {
				p_stream->read_lendian_t(sub_stream_version, p_abort);
				sub_stream_version_read = true;
			} catch (const exception_io_data_truncation&) {}

			if (sub_stream_version_read && sub_stream_version <= sub_stream_version_current) {
				for (i = 0; i < count; i++) {
					uint32_t extra_data_size;
					p_stream->read_lendian_t(extra_data_size, p_abort);
					
					pfc::array_staticsize_t<t_uint8> column_extra_data(extra_data_size);
					p_stream->read(column_extra_data.get_ptr(), column_extra_data.get_size(), p_abort);

					stream_reader_memblock_ref column_reader(column_extra_data);
					column_reader.read_lendian_t((*this)[i].m_last_sort_direction, p_abort);
				}
			}
		}
	}
	void cfg_fields_t::get_data_raw(stream_writer * p_stream, abort_callback & p_abort)
	{
		p_stream->write_lendian_t((t_uint32)stream_version_current, p_abort);
		t_uint32 i, count = pfc::downcast_guarded<uint32_t>(get_count());
		p_stream->write_lendian_t(count, p_abort);
		for (i = 0; i<count; i++)
		{
			const auto & field = (*this)[i];
			p_stream->write_string(field.m_name, p_abort);
			p_stream->write_string(field.m_field, p_abort);
		}

		p_stream->write_lendian_t(static_cast<uint32_t>(sub_stream_version_current), p_abort);
		for (i = 0; i<count; i++) {
			const auto & field = (*this)[i];
			stream_writer_memblock field_writer;
			field_writer.write_lendian_t(field.m_last_sort_direction, p_abort);

			uint32_t extra_size = field_writer.m_data.get_size();
			p_stream->write_lendian_t(extra_size, p_abort);
			p_stream->write(field_writer.m_data.get_ptr(), field_writer.m_data.get_size(), p_abort);
		}
	}
	void cfg_fields_t::reset()
	{
		set_count(3);
		t_size i = 0;
		(*this)[i].m_name = ((*this)[i].m_field = "Genre");
		i++;
		(*this)[i].m_field = "Album Artist;Artist";
		(*this)[i].m_name = "Artist";
		i++;
		(*this)[i].m_name = ((*this)[i].m_field = "Album");
	}

	bool cfg_fields_t::have_name(const char * p_name)
	{
		t_size i, count = get_count();
		for (i = 0; i<count; i++)
			if (!stricmp_utf8(p_name, (*this)[i].m_name))
				return true;
		return false;
	}

	bool cfg_fields_t::find_by_name(const char * p_name, size_t & p_index)
	{
		t_size i, count = get_count();
		for (i = 0; i<count; i++)
			if (!stricmp_utf8(p_name, (*this)[i].m_name)) {
				p_index = i;
				return true;
			}
		return false;
	}

	void cfg_fields_t::fix_name(const char * p_name, pfc::string8 & p_out)
	{
		t_size i = 0;
		p_out = p_name;
		while (have_name(p_out))
		{
			p_out.reset();
			p_out << p_name << " (" << (++i) << ")";
		}
	}
	void cfg_fields_t::fix_name(pfc::string8 & p_name)
	{
		fix_name(pfc::string8(p_name), p_name);
	}


	void cfg_favouriteslist::get_data_raw(stream_writer * p_stream, abort_callback & p_abort) {
		t_uint32 n, m = pfc::downcast_guarded<t_uint32>(get_count()), v = 0;
		p_stream->write_lendian_t(v, p_abort);
		p_stream->write_lendian_t(m, p_abort);
		for (n = 0; n<m; n++) p_stream->write_string(get_item(n), p_abort);
	}
	void cfg_favouriteslist::set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) {
		t_uint32 n, count, version;
		p_stream->read_lendian_t(version, p_abort);
		if (version <= 0)
		{
			p_stream->read_lendian_t(count, p_abort);
			pfc::string8_fast_aggressive temp;
			temp.prealloc(32);
			for (n = 0; n<count; n++)
			{
				p_stream->read_string(temp, p_abort);
				add_item(temp);
			}
		}
	}

	bool cfg_favouriteslist::have_item(const char * p_item)
	{
		t_size i, count = get_count();
		for (i = 0; i<count; i++)
			if (!strcmp(p_item, get_item(i))) return true;
		return false;
	}

	bool cfg_favouriteslist::find_item(const char * p_item, t_size & index)
	{
		t_size i, count = get_count();
		for (i = 0; i<count; i++)
			if (!strcmp(p_item, get_item(i))) { index = i; return true; }
		return false;
	}


}