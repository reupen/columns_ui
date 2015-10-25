#pragma once

class colours_manager_data : public cfg_var
{
public:
	static const GUID g_cfg_guid;
	enum { cfg_version = 0 };
	void get_data_raw(stream_writer * p_stream, abort_callback & p_abort);
	void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort);
	class entry_t : public pfc::refcounted_object_root
	{
	public:
		enum t_export_identifiers
		{
			identifier_guid,
			identifier_mode,
			identifier_background,
			identifier_selection_background,
			identifier_inactive_selection_background,
			identifier_text,
			identifier_selection_text,
			identifier_inactive_selection_text,
			identifier_custom_active_item_frame,
			identifier_use_custom_active_item_frame,
		};
		GUID guid;
		COLORREF
			text,
			selection_text,
			inactive_selection_text,
			background,
			selection_background,
			inactive_selection_background,
			active_item_frame,
			group_foreground,
			group_background
			;
		bool use_custom_active_item_frame;
		/*LOGFONT
		font_content,
		font_content_group,
		font_header;*/
		cui::colours::colour_mode_t colour_mode;
		void write(stream_writer * p_stream, abort_callback & p_abort);
		void _export(stream_writer * p_stream, abort_callback & p_abort);
		virtual void import(stream_reader * p_reader, t_size stream_size, t_uint32 type, abort_callback & p_abort);
		void read(t_uint32 version, stream_reader * p_stream, abort_callback & p_abort);
		void reset_colors();
		entry_t(bool b_global = false);
	};
	typedef pfc::refcounted_object_ptr_t<entry_t> entry_ptr_t;
	pfc::list_t<entry_ptr_t> m_entries;
	entry_ptr_t m_global_entry;

	void find_by_guid(const GUID & p_guid, entry_ptr_t & p_out);

	colours_manager_data();

	void register_common_callback(cui::colours::common_callback * p_callback);;
	void deregister_common_callback(cui::colours::common_callback * p_callback);;

	void g_on_common_colour_changed(t_size mask);

	void g_on_common_bool_changed(t_size mask);

	pfc::ptr_list_t<cui::colours::common_callback> m_callbacks;
};