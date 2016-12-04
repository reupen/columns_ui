#pragma once

namespace filter_panel {
	class SummaryField {
	public:
		GUID guid;
		pfc::string8 name;
		pfc::string8 script;

		void write(stream_writer * p_stream, abort_callback & p_abort) const;
		void read(stream_reader * p_stream, abort_callback & p_abort);

		SummaryField() : guid(pfc::guid_null) {}
		SummaryField(GUID guid_, const char * name_, const char * script_)
			: guid(guid_), name(name_), script(script_) {}
	};

	class CfgSummaryFields : public cfg_var, public std::vector<std::shared_ptr<SummaryField>> {
	public:
		enum class StreamVersion {
			StreamVersion0 = 0,
			Current = StreamVersion0
		};

		CfgSummaryFields(const GUID & guid) : cfg_var(guid) {}

		void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) override;
		void get_data_raw(stream_writer * p_stream, abort_callback & p_abort) override;

		void reset();
	};

	class TitleformatHookSummaryFields : public titleformat_hook {
	public:
		TitleformatHookSummaryFields(size_t size);

		bool process_field(titleformat_text_out* p_out, const char* p_name, t_size p_name_length, bool& p_found_flag) override;
		bool process_function(titleformat_text_out* p_out, const char* p_name, t_size p_name_length, titleformat_hook_function_params* p_params, bool& p_found_flag) override;
	private:
		bool process_size(titleformat_text_out* p_out, bool& p_found_flag);

		/** Item count */
		size_t m_size;
	};
}
