#pragma once

class fonts_manager_data : public cfg_var
{
public:
	static const GUID g_cfg_guid;
	enum { cfg_version = 0 };
	void get_data_raw(stream_writer * p_stream, abort_callback & p_abort) override;
	void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) override;
	static void g_write_font(stream_writer * m_output, const LOGFONT & lfc, abort_callback & m_abort);
	static void g_read_font(stream_reader * p_reader, LOGFONT & lf_out, abort_callback & p_abort);
	class entry_t : public pfc::refcounted_object_root
	{
	public:
		enum identifier_t
		{
			identifier_guid,
			identifier_mode,
			identifier_font,
		};
		GUID guid;
		LOGFONT font;
		cui::fonts::font_mode_t font_mode;
		void write(stream_writer * p_stream, abort_callback & p_abort);
		void read(t_uint32 version, stream_reader * p_stream, abort_callback & p_abort);
		void _export(stream_writer * p_stream, abort_callback & p_abort);
		virtual void import(stream_reader * p_reader, t_size stream_size, t_uint32 type, abort_callback & p_abort);
		void reset_fonts();
		entry_t(bool b_global = false);
	};
	typedef pfc::refcounted_object_ptr_t<entry_t> entry_ptr_t;
	pfc::list_t<entry_ptr_t> m_entries;
	entry_ptr_t m_common_items_entry;
	entry_ptr_t m_common_labels_entry;

	void find_by_guid(const GUID & p_guid, entry_ptr_t & p_out);

	void register_common_callback(cui::fonts::common_callback * p_callback);;
	void deregister_common_callback(cui::fonts::common_callback * p_callback);;

	void g_on_common_font_changed(t_size mask);

	pfc::ptr_list_t<cui::fonts::common_callback> m_callbacks;


	fonts_manager_data();
};
