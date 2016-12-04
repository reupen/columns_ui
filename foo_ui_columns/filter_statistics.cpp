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

	void CfgSummaryFields::set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort)
	{
		clear();
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
				emplace_back(entry);
			}

		}
	}

	void CfgSummaryFields::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
	{
		p_stream->write_lendian_t(static_cast<uint32_t>(StreamVersion::StreamVersion0), p_abort);
		p_stream->write_lendian_t(pfc::downcast_guarded<uint32_t>(size()), p_abort);
		for (const auto& entry : *this) {
			stream_writer_memblock entry_data;
			entry->write(&entry_data, p_abort);
			p_stream->write_lendian_t(pfc::downcast_guarded<uint32_t>(entry_data.m_data.get_size()), p_abort);
			p_stream->write(entry_data.m_data.get_ptr(), entry_data.m_data.get_size(), p_abort);
		}
	}

	void CfgSummaryFields::reset()
	{
		clear();
		emplace_back(std::make_shared<SummaryField>( pfc::guid_null, "Size", "%size%" ));
	}

	TitleformatHookSummaryFields::TitleformatHookSummaryFields(size_t size)
		: m_size(size)
	{
	}

	bool TitleformatHookSummaryFields::process_field(titleformat_text_out* p_out, const char* p_name, t_size p_name_length, bool& p_found_flag)
	{
		p_found_flag = false;
		if (!stricmp_utf8_ex(p_name, p_name_length, "size", pfc_infinite)) {
			return process_size(p_out, p_found_flag);
		}
		return false;
	}

	bool TitleformatHookSummaryFields::process_function(titleformat_text_out* p_out, const char* p_name, t_size p_name_length, titleformat_hook_function_params* p_params, bool& p_found_flag)
	{
		p_found_flag = false;
		return false;
	}

	bool TitleformatHookSummaryFields::process_size(titleformat_text_out * p_out, bool & p_found_flag)
	{
		p_found_flag = true;
		p_out->write_int(titleformat_inputtypes::unknown, 0);
		return false;
	}
}
