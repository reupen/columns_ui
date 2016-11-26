#pragma once

namespace filter_panel {
	class SummaryField {
	public:
		pfc::string8 name;
		pfc::string8 script;

		void write(stream_writer * p_stream, abort_callback & p_abort) const;
		void read(stream_reader * p_stream, abort_callback & p_abort);
	};

	class CfgSummaryFields : public cfg_var, public std::vector<SummaryField> {
	public:
		enum class StreamVersion {
			StreamVersion0 = 0,
			Current = StreamVersion0
		};

		void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) override;
		void get_data_raw(stream_writer * p_stream, abort_callback & p_abort) override;

		CfgSummaryFields(const GUID & guid): cfg_var(guid) {}
	};
}
