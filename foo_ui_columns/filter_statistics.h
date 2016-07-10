#pragma once

namespace filter_panel {
	class SummaryField {
	public:
		pfc::string8 name;
		pfc::string8 script;
	};

	class CfgSummaryFields : public cfg_var {
	public:
		enum class StreamVersion {
			Current = 0
		};

		void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) override;
		void get_data_raw(stream_writer * p_stream, abort_callback & p_abort) override;

	private:
		pfc::list_t<SummaryField> m_data;

	};
}
