#include "stdafx.h"
#include "filter_statistics.h"

namespace filter_panel {
	void SummaryField::write(stream_writer* p_stream, abort_callback& p_abort) const
	{
		p_stream->write_lendian_t(guid, p_abort);
		p_stream->write_string(name, p_abort);
		p_stream->write_string(script, p_abort);
	}

	void SummaryField::read(stream_reader* p_stream, abort_callback& p_abort)
	{
		p_stream->read_lendian_t(guid, p_abort);
		p_stream->read_string(name, p_abort);
		p_stream->read_string(script, p_abort);
	}

	namespace default_guids {
		// {B39BB41D-CD8D-4EE6-8E48-FCE072EEA26C}
		static const GUID size =
		{ 0xb39bb41d, 0xcd8d, 0x4ee6,{ 0x8e, 0x48, 0xfc, 0xe0, 0x72, 0xee, 0xa2, 0x6c } };

		// {B4E11056-5016-4EC3-8690-3CD878EAB38C}
		static const GUID file_size =
		{ 0xb4e11056, 0x5016, 0x4ec3,{ 0x86, 0x90, 0x3c, 0xd8, 0x78, 0xea, 0xb3, 0x8c } };

		// {000A4BDC-FB5E-400A-BA95-4195380B0FB1}
		static const GUID length =
		{ 0xa4bdc, 0xfb5e, 0x400a,{ 0xba, 0x95, 0x41, 0x95, 0x38, 0xb, 0xf, 0xb1 } };
	}


	void CfgSummaryFields::set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort)
	{
		if (p_sizehint == 0) {
			reset();
		} else {
			m_data.clear();
			uint32_t streamVersion_;
			p_stream->read_lendian_t(streamVersion_, p_abort);
			StreamVersion stream_version = static_cast<StreamVersion>(streamVersion_);

			if (stream_version <= StreamVersion::Current) {
				uint32_t entry_count;
				p_stream->read_lendian_t(entry_count, p_abort);

				for (uint32_t entry_index = 0; entry_index < entry_count; entry_index++) {
					std::shared_ptr<SummaryField> entry = std::make_shared<SummaryField>();
					uint32_t entry_data_size;
					p_stream->read_lendian_t(entry_data_size, p_abort);
					pfc::array_staticsize_t<t_uint8> entry_data(entry_data_size);
					p_stream->read(entry_data.get_ptr(), entry_data_size, p_abort);
					stream_reader_memblock_ref entry_reader(entry_data);
					entry->read(&entry_reader, p_abort);
					m_data.emplace_back(entry);
				}

			}
		}
	}

	void CfgSummaryFields::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
	{
		p_stream->write_lendian_t(static_cast<uint32_t>(StreamVersion::StreamVersion0), p_abort);
		p_stream->write_lendian_t(pfc::downcast_guarded<uint32_t>(m_data.size()), p_abort);
		for (const auto& entry : *this) {
			stream_writer_memblock entry_data;
			entry->write(&entry_data, p_abort);
			p_stream->write_lendian_t(pfc::downcast_guarded<uint32_t>(entry_data.m_data.get_size()), p_abort);
			p_stream->write(entry_data.m_data.get_ptr(), entry_data.m_data.get_size(), p_abort);
		}
	}

	void CfgSummaryFields::reset()
	{
		m_data.clear();
		m_data.emplace_back(std::make_shared<SummaryField>(default_guids::size, "Items", "%size%"));
		m_data.emplace_back(std::make_shared<SummaryField>(default_guids::file_size, "File size", "%filesize%"));
		m_data.emplace_back(std::make_shared<SummaryField>(default_guids::length, "Duration", "%length%"));
	}

	TitleformatHookSummaryFields::TitleformatHookSummaryFields(metadb_handle_list_cref handles)
		: m_handles(handles)
	{
	}

	bool TitleformatHookSummaryFields::process_field(titleformat_text_out* p_out, const char* p_name, t_size p_name_length, bool& p_found_flag)
	{
		p_found_flag = false;
		if (!stricmp_utf8_ex(p_name, p_name_length, "size", pfc_infinite)) {
			return process_size(p_out, p_found_flag);
		}
		if (!stricmp_utf8_ex(p_name, p_name_length, "length", pfc_infinite)) {
			return process_length(p_out, p_found_flag);
		}
		if (!stricmp_utf8_ex(p_name, p_name_length, "filesize", pfc_infinite)) {
			return process_file_size(p_out, p_found_flag);
		}

		return false;
	}

	bool TitleformatHookSummaryFields::process_function(titleformat_text_out* p_out, const char* p_name, t_size p_name_length, titleformat_hook_function_params* p_params, bool& p_found_flag)
	{
		p_found_flag = false;
		return false;
	}

	bool TitleformatHookSummaryFields::process_size(titleformat_text_out * p_out, bool & p_found_flag) const
	{
		p_found_flag = true;
		p_out->write_int(titleformat_inputtypes::unknown, m_handles.get_count());
		return true;
	}

	bool TitleformatHookSummaryFields::process_length(titleformat_text_out * p_out, bool & p_found_flag) const
	{
		p_found_flag = true;
		// TODO: Cache?
		double total_duration = metadb_handle_list_helper::calc_total_duration(m_handles);
		pfc::format_time_ex formatted_time(total_duration, 0);
		p_out->write(titleformat_inputtypes::unknown, formatted_time);
		return true;
	}

	bool TitleformatHookSummaryFields::process_file_size(::titleformat_text_out* p_out, bool& p_found_flag) const
	{
		p_found_flag = true;
		// TODO: Cache?
		t_filesize total_size = metadb_handle_list_helper::calc_total_size(m_handles);
		mmh::format_file_size formatted_total_size(total_size);
		p_out->write(titleformat_inputtypes::unknown, formatted_total_size);
		return true;
	}
}
