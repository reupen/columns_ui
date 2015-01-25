#include "foo_ui_columns.h"

/*
* Fonts: 
*  Tabs - playlist tabs, tab stack
*  Lists - Playlist switcher, Playlist items, Playlist items group header, playlist column titles, filter panel
*  Other - Status bar, , , , 
* 
* Colours:
*  [[inactive] selected] text, [[inactive] selected] item background, 
* 
*/

#if 1

class appearance_message_window_t : public ui_helpers::container_window_autorelease_t
{
public:
	virtual class_data & get_class_data() const 
	{
		__implement_get_class_data_ex(_T("{BDCEC7A3-7230-4671-A5F7-B19A989DCA81}"), _T(""), false, 0, 0, 0, 0);
	}
	static void g_initialise();
	static bool g_initialised;
	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
};
bool appearance_message_window_t::g_initialised = false;
//pfc::rcptr_t<appearance_message_window_t> g_appearance_message_window;

void appearance_message_window_t::g_initialise()
	{
		if (!g_initialised)
		{
			appearance_message_window_t * ptr = new appearance_message_window_t;
			ptr->create(HWND_MESSAGE);
			g_initialised = true;
		}
	}


class colours_manager_data : public cfg_var
{
public:
	static const GUID g_cfg_guid;
	enum {cfg_version = 0};
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort) 
	{
		pfc::list_t<GUID> clients;
		{
			service_enum_t<cui::colours::client> clientenum;
			cui::colours::client::ptr ptr;
			while (clientenum.next(ptr))
				clients.add_item(ptr->get_client_guid());
		}

		pfc::array_t<bool> mask;
		t_size i, count = m_entries.get_count(), counter=0;
		mask.set_count(count);
		for (i=0; i<count; i++)
			if (mask[i] = clients.have_item(m_entries[i]->guid)) counter++;

		p_stream->write_lendian_t((t_uint32)cfg_version,p_abort);
		m_global_entry->write(p_stream, p_abort);
		p_stream->write_lendian_t(counter, p_abort);
		for (i=0; i<count; i++)
			if (mask[i])
				m_entries[i]->write(p_stream, p_abort);
	}
	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort)
	{
		t_uint32 version;
		p_stream->read_lendian_t(version, p_abort);
		if (version <= cfg_version)
		{
			m_global_entry->read(version, p_stream, p_abort);
			t_size i, count;
			p_stream->read_lendian_t(count, p_abort);
			m_entries.remove_all();
			for (i=0; i<count; i++)
			{
				entry_ptr_t ptr = new entry_t;
				ptr->read(version, p_stream, p_abort);
				m_entries.add_item(ptr);
			}
		}
	}
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
		void write(stream_writer * p_stream,abort_callback & p_abort)
		{
			p_stream->write_lendian_t(guid, p_abort);
			p_stream->write_lendian_t((t_uint32)colour_mode, p_abort);
			p_stream->write_lendian_t(text, p_abort);
			p_stream->write_lendian_t(selection_text, p_abort);
			p_stream->write_lendian_t(inactive_selection_text, p_abort);
			p_stream->write_lendian_t(background, p_abort);
			p_stream->write_lendian_t(selection_background, p_abort);
			p_stream->write_lendian_t(inactive_selection_background, p_abort);
			p_stream->write_lendian_t(active_item_frame, p_abort);
			p_stream->write_lendian_t(use_custom_active_item_frame, p_abort);
		}
		void export(stream_writer * p_stream,abort_callback & p_abort)
		{
			fcl::writer out(p_stream, p_abort);
			out.write_item(identifier_guid, guid);
			out.write_item(identifier_mode, (t_uint32)colour_mode);
			if (colour_mode == cui::colours::colour_mode_custom)
			{
				out.write_item(identifier_text, text);
				out.write_item(identifier_selection_text, selection_text);
				out.write_item(identifier_inactive_selection_text, inactive_selection_text);
				out.write_item(identifier_background, background);
				out.write_item(identifier_selection_background, selection_background);
				out.write_item(identifier_inactive_selection_background, inactive_selection_background);
			}
			out.write_item(identifier_use_custom_active_item_frame, use_custom_active_item_frame);
			if (use_custom_active_item_frame)
				out.write_item(identifier_custom_active_item_frame, active_item_frame);
		}
		virtual void import (stream_reader * p_reader, t_size stream_size, t_uint32 type, abort_callback & p_abort)
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
					reader.read_item((t_uint32&)colour_mode);
					break;
				case identifier_text:
					reader.read_item(text);
					break;
				case identifier_selection_text:
					reader.read_item(selection_text);
					break;
				case identifier_inactive_selection_text:
					reader.read_item(inactive_selection_text);
					break;
				case identifier_background:
					reader.read_item(background);
					break;
				case identifier_selection_background:
					reader.read_item(selection_background);
					break;
				case identifier_inactive_selection_background:
					reader.read_item(inactive_selection_background);
					break;
				case identifier_custom_active_item_frame:
					reader.read_item(active_item_frame);
					break;
				case identifier_use_custom_active_item_frame:
					reader.read_item(use_custom_active_item_frame);
					break;
				default:
					reader.skip(element_size);
					break;
				};
			}
		}
		void read(t_uint32 version, stream_reader * p_stream,abort_callback & p_abort)
		{
			p_stream->read_lendian_t(guid, p_abort);
			p_stream->read_lendian_t((t_uint32&)colour_mode, p_abort);
			p_stream->read_lendian_t(text, p_abort);
			p_stream->read_lendian_t(selection_text, p_abort);
			p_stream->read_lendian_t(inactive_selection_text, p_abort);
			p_stream->read_lendian_t(background, p_abort);
			p_stream->read_lendian_t(selection_background, p_abort);
			p_stream->read_lendian_t(inactive_selection_background, p_abort);
			p_stream->read_lendian_t(active_item_frame, p_abort);
			p_stream->read_lendian_t(use_custom_active_item_frame, p_abort);
		}
		void reset_colors()
		{
			text = cui::colours::g_get_system_color(cui::colours::colour_text);
			selection_text = cui::colours::g_get_system_color(cui::colours::colour_selection_text);
			inactive_selection_text = cui::colours::g_get_system_color(cui::colours::colour_inactive_selection_text);
			background = cui::colours::g_get_system_color(cui::colours::colour_background);
			selection_background = cui::colours::g_get_system_color(cui::colours::colour_selection_background);
			inactive_selection_background = cui::colours::g_get_system_color(cui::colours::colour_inactive_selection_background);
			active_item_frame = cui::colours::g_get_system_color(cui::colours::colour_active_item_frame);
			group_foreground = NULL;
			group_background = NULL;
			use_custom_active_item_frame = false;
		}
		entry_t(bool b_global=false)
			: guid (pfc::guid_null), colour_mode(b_global ? (is_vista_or_newer() ? cui::colours::colour_mode_themed : cui::colours::colour_mode_system) : cui::colours::colour_mode_global)
		{
			reset_colors();
		}
	};
	typedef pfc::refcounted_object_ptr_t<entry_t> entry_ptr_t;
	pfc::list_t<entry_ptr_t> m_entries;
	entry_ptr_t m_global_entry;

	void find_by_guid(const GUID & p_guid, entry_ptr_t & p_out)
	{
		if (p_guid == pfc::guid_null)
		{
			p_out = m_global_entry;
			return;
		}
		t_size i, count = m_entries.get_count();
		for (i=0; i<count; i++)
		{
			if (m_entries[i]->guid == p_guid)
			{
				p_out = m_entries[i];
				return;
			}
		}
		p_out = new entry_t;
		p_out->guid = p_guid;
		m_entries.add_item(p_out);
	}

	colours_manager_data() : cfg_var(g_cfg_guid)
	{
		m_global_entry = new entry_t(true);
	}

	void register_common_callback (cui::colours::common_callback * p_callback) 
	{
		m_callbacks.add_item(p_callback);
	};
	void deregister_common_callback (cui::colours::common_callback * p_callback) 
	{
		m_callbacks.remove_item(p_callback);
	};

	void g_on_common_colour_changed(t_size mask)
	{
		t_size i, count = m_callbacks.get_count();
		for (i=0; i<count; i++)
			m_callbacks[i]->on_colour_changed(mask);
	}

	void g_on_common_bool_changed(t_size mask)
	{
		t_size i, count = m_callbacks.get_count();
		for (i=0; i<count; i++)
			m_callbacks[i]->on_bool_changed(mask);
	}

	pfc::ptr_list_t<cui::colours::common_callback> m_callbacks;
} g_colours_manager_data;

class fonts_manager_data : public cfg_var
{
public:
	static const GUID g_cfg_guid;
	enum {cfg_version = 0};
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort) 
	{
		pfc::list_t<GUID> clients;
		{
			service_enum_t<cui::fonts::client> clientenum;
			cui::fonts::client::ptr ptr;
			while (clientenum.next(ptr))
				clients.add_item(ptr->get_client_guid());
		}

		pfc::array_t<bool> mask;
		t_size i, count = m_entries.get_count(), counter=0;
		mask.set_count(count);
		for (i=0; i<count; i++)
			if (mask[i] = clients.have_item(m_entries[i]->guid)) counter++;

		p_stream->write_lendian_t((t_uint32)cfg_version,p_abort);
		m_common_items_entry->write(p_stream, p_abort);
		m_common_labels_entry->write(p_stream, p_abort);

		p_stream->write_lendian_t(counter, p_abort);
		for (i=0; i<count; i++)
			if (mask[i])
				m_entries[i]->write(p_stream, p_abort);
	}
	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort)
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
			for (i=0; i<count; i++)
			{
				entry_ptr_t ptr = new entry_t;
				ptr->read(version, p_stream, p_abort);
				m_entries.add_item(ptr);
			}
		}
	}
	static void g_write_font(stream_writer * m_output, const LOGFONT & lfc, abort_callback & m_abort)
	{
		LOGFONT lf = lfc;
		t_size face_len = pfc::wcslen_max(lf.lfFaceName, tabsize(lf.lfFaceName));
		
		if (face_len < tabsize(lf.lfFaceName))
		{
			WCHAR * ptr = lf.lfFaceName;
			ptr += face_len;
			memset(ptr, 0, sizeof(WCHAR)*(tabsize(lf.lfFaceName)-face_len));
		}

		m_output->write_lendian_t(lf.lfHeight, m_abort);
		m_output->write_lendian_t(lf.lfWidth, m_abort);
		m_output->write_lendian_t(lf.lfEscapement, m_abort);
		m_output->write_lendian_t(lf.lfOrientation, m_abort);
		m_output->write_lendian_t(lf.lfWeight, m_abort);

		//meh endianness
		m_output->write(&lf.lfItalic,8 + sizeof(lf.lfFaceName),m_abort);
	}
	static void g_read_font(stream_reader * p_reader, LOGFONT & lf_out, abort_callback & p_abort)
	{
		LOGFONT lf;

		p_reader->read_lendian_t(lf.lfHeight, p_abort);
		p_reader->read_lendian_t(lf.lfWidth, p_abort);
		p_reader->read_lendian_t(lf.lfEscapement, p_abort);
		p_reader->read_lendian_t(lf.lfOrientation, p_abort);
		p_reader->read_lendian_t(lf.lfWeight, p_abort);

		//meh endianness
		p_reader->read(&lf.lfItalic,8 + sizeof(lf.lfFaceName), p_abort);

		lf_out = lf;
	}
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
		void write(stream_writer * p_stream,abort_callback & p_abort)
		{
			p_stream->write_lendian_t(guid, p_abort);
			p_stream->write_lendian_t((t_uint32)font_mode, p_abort);
			g_write_font(p_stream, font, p_abort);
		}
		void read(t_uint32 version, stream_reader * p_stream,abort_callback & p_abort)
		{
			p_stream->read_lendian_t(guid, p_abort);
			p_stream->read_lendian_t((t_uint32&)font_mode, p_abort);
			g_read_font(p_stream, font, p_abort);
		}
		void export(stream_writer * p_stream,abort_callback & p_abort)
		{
			fcl::writer out(p_stream, p_abort);
			out.write_item(identifier_guid, guid);
			out.write_item(identifier_mode, (t_uint32)font_mode);
			if (font_mode == cui::fonts::font_mode_custom)
			{
				out.write_item(identifier_font, font);
			}
		}
		virtual void import (stream_reader * p_reader, t_size stream_size, t_uint32 type, abort_callback & p_abort)
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
		void reset_fonts()
		{
			uGetIconFont(&font);
		}
		entry_t(bool b_global=false)
			: guid (pfc::guid_null), font_mode(cui::fonts::font_mode_system)
		{
			reset_fonts();
		}
	};
	typedef pfc::refcounted_object_ptr_t<entry_t> entry_ptr_t;
	pfc::list_t<entry_ptr_t> m_entries;
	entry_ptr_t m_common_items_entry;
	entry_ptr_t m_common_labels_entry;

	void find_by_guid(const GUID & p_guid, entry_ptr_t & p_out)
	{
		t_size i, count = m_entries.get_count();
		for (i=0; i<count; i++)
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

	void register_common_callback (cui::fonts::common_callback * p_callback) 
	{
		m_callbacks.add_item(p_callback);
	};
	void deregister_common_callback (cui::fonts::common_callback * p_callback) 
	{
		m_callbacks.remove_item(p_callback);
	};

	void g_on_common_font_changed(t_size mask)
	{
		t_size i, count = m_callbacks.get_count();
		for (i=0; i<count; i++)
			m_callbacks[i]->on_font_changed(mask);
	}

	pfc::ptr_list_t<cui::fonts::common_callback> m_callbacks;


	fonts_manager_data() : cfg_var(g_cfg_guid)
	{
		m_common_items_entry = new entry_t(true);
		m_common_labels_entry = new entry_t(true);
		uGetIconFont(&m_common_items_entry->font);
		uGetMenuFont(&m_common_labels_entry->font);
	}
} g_fonts_manager_data;

class colours_manager_instance_impl : public cui::colours::manager_instance
{
public:
	colours_manager_instance_impl(const GUID & p_client_guid)
	{
		g_colours_manager_data.find_by_guid(p_client_guid, m_entry);
		g_colours_manager_data.find_by_guid(pfc::guid_null, m_global_entry);
	}
	virtual COLORREF get_colour(const cui::colours::colour_identifier_t & p_identifier) const
	{
		appearance_message_window_t::g_initialise();
		colours_manager_data::entry_ptr_t p_entry = m_entry->colour_mode == cui::colours::colour_mode_global ? m_global_entry : m_entry;
		if (p_entry->colour_mode == cui::colours::colour_mode_system || p_entry->colour_mode == cui::colours::colour_mode_themed)
			return g_get_system_color(p_identifier);
		switch (p_identifier)
		{
		case cui::colours::colour_text:
			return p_entry->text;
		case cui::colours::colour_selection_text:
			return p_entry->selection_text;
		case cui::colours::colour_background:
			return p_entry->background;
		case cui::colours::colour_selection_background:
			return p_entry->selection_background;
		case cui::colours::colour_inactive_selection_text:
			return p_entry->inactive_selection_text;
		case cui::colours::colour_inactive_selection_background:
			return p_entry->inactive_selection_background;
		case cui::colours::colour_active_item_frame:
			return p_entry->active_item_frame;
		default:
			return 0;
		}
	}
	virtual bool get_bool(const cui::colours::bool_identifier_t & p_identifier) const
	{
		colours_manager_data::entry_ptr_t p_entry = m_entry->colour_mode == cui::colours::colour_mode_global ? m_global_entry : m_entry;
		switch (p_identifier)
		{
		case cui::colours::bool_use_custom_active_item_frame:
			return p_entry->use_custom_active_item_frame;
		default:
			return false;
		}
	}
	virtual bool get_themed() const 
	{
		return m_entry->colour_mode == cui::colours::colour_mode_themed
			|| (m_entry->colour_mode == cui::colours::colour_mode_global && m_global_entry->colour_mode == cui::colours::colour_mode_themed);
	}
private:
	colours_manager_data::entry_ptr_t m_entry;
	colours_manager_data::entry_ptr_t m_global_entry;
};

class colours_manager_impl : public cui::colours::manager
{
public:
	virtual void create_instance (const GUID & p_client_guid, cui::colours::manager_instance::ptr & p_out)
	{
		p_out = new service_impl_t<colours_manager_instance_impl>(p_client_guid);
	}
	virtual void register_common_callback (cui::colours::common_callback * p_callback) 
	{
		g_colours_manager_data.register_common_callback(p_callback);
	};
	virtual void deregister_common_callback (cui::colours::common_callback * p_callback) 
	{
		g_colours_manager_data.deregister_common_callback(p_callback);
	};
private:
};

class fonts_manager_impl : public cui::fonts::manager
{
public:
	virtual void get_font(const GUID & p_guid, LOGFONT & p_out) const
	{
		appearance_message_window_t::g_initialise();
		fonts_manager_data::entry_ptr_t p_entry;
		g_fonts_manager_data.find_by_guid(p_guid, p_entry);
		if (p_entry->font_mode == cui::fonts::font_mode_common_items)
			get_font(cui::fonts::font_type_items, p_out);
		else if (p_entry->font_mode == cui::fonts::font_mode_common_labels)
			get_font(cui::fonts::font_type_labels, p_out);
		else
		{
			p_out = p_entry->font;
		}
	}
	virtual void get_font(const cui::fonts::font_type_t p_type, LOGFONT & p_out) const
	{
		fonts_manager_data::entry_ptr_t p_entry;
		if (p_type == cui::fonts::font_type_items)
			p_entry = g_fonts_manager_data.m_common_items_entry;
		else
			p_entry = g_fonts_manager_data.m_common_labels_entry;

		if (p_entry->font_mode == cui::fonts::font_mode_system)
		{
			if (p_type == cui::fonts::font_type_items)
				uGetIconFont(&p_out);
			else
				uGetMenuFont(&p_out);
		}
		else
		{
			p_out = p_entry->font;
		}
	}
	virtual void set_font(const GUID & p_guid, const LOGFONT & p_font)
	{
		fonts_manager_data::entry_ptr_t p_entry;
		g_fonts_manager_data.find_by_guid(p_guid, p_entry);
		p_entry->font_mode = cui::fonts::font_mode_custom;
		p_entry->font = p_font;
		cui::fonts::client::ptr ptr;
		if (cui::fonts::client::create_by_guid(p_guid, ptr))
			ptr->on_font_changed();
	}
	virtual void register_common_callback (cui::fonts::common_callback * p_callback) 
	{
		g_fonts_manager_data.register_common_callback(p_callback);
	};
	virtual void deregister_common_callback (cui::fonts::common_callback * p_callback) 
	{
		g_fonts_manager_data.deregister_common_callback(p_callback);
	};
private:
};

namespace {
	service_factory_single_t<colours_manager_impl> g_colours_manager;
service_factory_t<fonts_manager_impl> g_fonts_manager;
};

class colours_client_list_entry_t
{
public:
	pfc::string8 m_name;
	GUID m_guid;
	cui::colours::client::ptr m_ptr;
};

class colours_client_list_t : public pfc::list_t<colours_client_list_entry_t>
{
public:
	static void g_get_list(colours_client_list_t & p_out)
	{
		service_enum_t<cui::colours::client> e;
		colours_client_list_entry_t entry;
		while (e.next(entry.m_ptr))
		{
			entry.m_guid = entry.m_ptr->get_client_guid();
			entry.m_ptr->get_name(entry.m_name);
			p_out.add_item(entry);
		};
		p_out.sort_t(g_compare);
	}
	static int g_compare (const colours_client_list_entry_t & p1, const colours_client_list_entry_t & p2)
	{
		return StrCmpLogicalW(pfc::stringcvt::string_os_from_utf8(p1.m_name), pfc::stringcvt::string_os_from_utf8(p2.m_name));
	}
};

class fonts_client_list_entry_t
{
public:
	pfc::string8 m_name;
	GUID m_guid;
	cui::fonts::client::ptr m_ptr;
};

class fonts_client_list_t : public pfc::list_t<fonts_client_list_entry_t>
{
public:
	static void g_get_list(fonts_client_list_t & p_out)
	{
		service_enum_t<cui::fonts::client> e;
		fonts_client_list_entry_t entry;
		while (e.next(entry.m_ptr))
		{
			entry.m_guid = entry.m_ptr->get_client_guid();
			entry.m_ptr->get_name(entry.m_name);
			p_out.add_item(entry);
		};
		p_out.sort_t(g_compare);
	}
	static int g_compare (const fonts_client_list_entry_t & p1, const fonts_client_list_entry_t & p2)
	{
		return StrCmpLogicalW(pfc::stringcvt::string_os_from_utf8(p1.m_name), pfc::stringcvt::string_os_from_utf8(p2.m_name));
	}
};

	static int combobox_find_by_id(HWND wnd, t_size id)
	{
		t_size i, count = ComboBox_GetCount(wnd);
		for (i=0; i<count; i++)
			if (ComboBox_GetItemData(wnd, i) == id)
				return i;
		return -1;
	}

static class tab_appearance : public preferences_tab
{
	HWND m_wnd;
	HWND m_wnd_colours_mode;
	HWND m_wnd_colours_element;
	window_fill g_fill_text;
	window_fill g_fill_background;
	window_fill g_fill_selection_text;
	window_fill g_fill_selection_background;
	window_fill g_fill_selection_text_inactive;
	window_fill g_fill_selection_background_inactive;
	window_fill g_fill_active_item_frame;
	GUID m_element_guid;
	colours_manager_data::entry_ptr_t m_element_ptr;
	cui::colours::client::ptr m_element_api;
	colours_client_list_t m_colours_client_list;
public:
	tab_appearance() : initialising(false), m_wnd_colours_mode(NULL), m_element_guid(pfc::guid_null),
	m_wnd(NULL) {};

	void refresh_me(HWND wnd)
	{
		initialising = true;	
		initialising = false;	
	}


	static BOOL CALLBACK g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		tab_appearance * p_data = NULL;
		if (msg == WM_INITDIALOG)
		{
			p_data = reinterpret_cast<tab_appearance*>(lp);
			SetWindowLongPtr(wnd, DWL_USER, lp);
		}
		else
			p_data = reinterpret_cast<tab_appearance*>(GetWindowLongPtr(wnd, DWL_USER));
		return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
	}

	void update_fills()
	{
		cui::colours::helper p_manager(m_element_guid);
		if (p_manager.get_themed() && (!m_element_api.is_valid() || m_element_api->get_themes_supported()))
		{
			g_fill_selection_background.set_fill_themed(L"ListView", LVP_LISTITEM, LISS_SELECTED, p_manager.get_colour(cui::colours::colour_selection_background));
			g_fill_selection_background_inactive.set_fill_themed(L"ListView", LVP_LISTITEM, LISS_SELECTEDNOTFOCUS, p_manager.get_colour(cui::colours::colour_inactive_selection_background));
			g_fill_selection_text_inactive.set_fill_themed_colour(L"ListView", COLOR_BTNTEXT);
			g_fill_selection_text.set_fill_themed_colour(L"ListView", COLOR_BTNTEXT);
		}
		else
		{
			g_fill_selection_text.set_fill_colour(p_manager.get_colour(cui::colours::colour_selection_text));
			g_fill_selection_background.set_fill_colour(p_manager.get_colour(cui::colours::colour_selection_background));
			g_fill_selection_background_inactive.set_fill_colour(p_manager.get_colour(cui::colours::colour_inactive_selection_background));
			g_fill_selection_text_inactive.set_fill_colour(p_manager.get_colour(cui::colours::colour_inactive_selection_text));
		}
		g_fill_text.set_fill_colour(p_manager.get_colour(cui::colours::colour_text));
		g_fill_background.set_fill_colour(p_manager.get_colour(cui::colours::colour_background));
		g_fill_active_item_frame.set_fill_colour(p_manager.get_colour(cui::colours::colour_active_item_frame));

		Button_SetCheck(GetDlgItem(m_wnd, IDC_CUSTOM_FRAME), p_manager.get_bool(cui::colours::bool_use_custom_active_item_frame));
	}
	bool get_change_colour_enabled(cui::colours::colour_identifier_t p_identifier)
	{
		if (cui::colours::colour_active_item_frame == p_identifier)
			return m_element_ptr->colour_mode != cui::colours::colour_mode_global && (!m_element_api.is_valid() || m_element_api->get_supported_colours() & (1<<p_identifier)) && (  (!m_element_api.is_valid() || !(m_element_api->get_supported_bools() & cui::colours::bool_flag_use_custom_active_item_frame)) || cui::colours::helper(m_element_guid).get_bool(cui::colours::bool_use_custom_active_item_frame));
		else
			return (m_element_ptr->colour_mode == cui::colours::colour_mode_custom && (!m_element_api.is_valid() || (m_element_api->get_supported_colours() & (1<<p_identifier))));
	}
	bool get_colour_patch_enabled(cui::colours::colour_identifier_t p_identifier)
	{
		if (cui::colours::colour_active_item_frame == p_identifier)
			return cui::colours::helper(m_element_guid).get_bool(cui::colours::bool_use_custom_active_item_frame);
		else
			return (!m_element_api.is_valid() || (m_element_api->get_supported_colours() & (1<<p_identifier)));
	}
	void update_buttons()
	{
		EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_TEXT_BACK), get_change_colour_enabled(cui::colours::colour_background));
		EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_TEXT_FORE), get_change_colour_enabled(cui::colours::colour_text));
		EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_SELBACK), get_change_colour_enabled(cui::colours::colour_selection_background));
		EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_SEL_FORE), get_change_colour_enabled(cui::colours::colour_selection_text));
		EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_SEL_INACTIVE_BACK), get_change_colour_enabled(cui::colours::colour_inactive_selection_background));
		EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_SEL_INACTIVE_FORE), get_change_colour_enabled(cui::colours::colour_inactive_selection_text));
		EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_FRAME), get_change_colour_enabled(cui::colours::colour_active_item_frame));

		EnableWindow(g_fill_background.get_wnd(), get_colour_patch_enabled(cui::colours::colour_background));
		EnableWindow(g_fill_text.get_wnd(), get_colour_patch_enabled(cui::colours::colour_text));
		EnableWindow(g_fill_selection_background.get_wnd(), get_colour_patch_enabled(cui::colours::colour_selection_background));
		EnableWindow(g_fill_selection_text.get_wnd(), get_colour_patch_enabled(cui::colours::colour_selection_text));
		EnableWindow(g_fill_selection_background_inactive.get_wnd(), get_colour_patch_enabled(cui::colours::colour_inactive_selection_background));
		EnableWindow(g_fill_selection_text_inactive.get_wnd(), get_colour_patch_enabled(cui::colours::colour_inactive_selection_text));

		EnableWindow(g_fill_active_item_frame.get_wnd(), get_colour_patch_enabled(cui::colours::colour_active_item_frame));
		EnableWindow(GetDlgItem(m_wnd, IDC_CUSTOM_FRAME), m_element_ptr->colour_mode != cui::colours::colour_mode_global && (!m_element_api.is_valid() || (m_element_api->get_supported_bools() & cui::colours::bool_flag_use_custom_active_item_frame)));
	}
	void update_mode_combobox()
	{
		ComboBox_ResetContent(m_wnd_colours_mode);
		t_size index;
		if (m_element_guid != pfc::guid_null)
		{
			index = ComboBox_AddString(m_wnd_colours_mode, L"Global");
			ComboBox_SetItemData(m_wnd_colours_mode, index, cui::colours::colour_mode_global);
		}
		index = ComboBox_AddString(m_wnd_colours_mode, L"System");
		ComboBox_SetItemData(m_wnd_colours_mode, index, cui::colours::colour_mode_system);
		if (!m_element_api.is_valid() || m_element_api->get_themes_supported())
		{
			index = ComboBox_AddString(m_wnd_colours_mode, L"Themed");
			ComboBox_SetItemData(m_wnd_colours_mode, index, cui::colours::colour_mode_themed);
		}
		index = ComboBox_AddString(m_wnd_colours_mode, L"Custom");
		ComboBox_SetItemData(m_wnd_colours_mode, index, cui::colours::colour_mode_custom);

		ComboBox_SetCurSel(m_wnd_colours_mode, combobox_find_by_id(m_wnd_colours_mode,m_element_ptr->colour_mode));
	}

	void on_colour_changed()
	{
		if (m_element_api.is_valid())
			m_element_api->on_colour_changed(cui::colours::colour_flag_all);
		else if (m_element_guid == pfc::guid_null)
		{
			g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
			t_size i, count = m_colours_client_list.get_count();
			for (i=0; i<count; i++)
			{
				colours_manager_data::entry_ptr_t p_data;
				g_colours_manager_data.find_by_guid(m_colours_client_list[i].m_guid, p_data);
				if (p_data->colour_mode == cui::colours::colour_mode_global)
					m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
			}
		}
	}

	BOOL CALLBACK on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{

		switch(msg)
		{
		case WM_INITDIALOG:
			{
				m_wnd = wnd;
				m_wnd_colours_mode = GetDlgItem(wnd, IDC_COLOURS_MODE);
				m_wnd_colours_element = GetDlgItem(wnd, IDC_COLOURS_ELEMENT);

				t_size y = 85;
				t_size spacing = 5+14;

				g_fill_text.create_in_dialog_units(wnd, ui_helpers::window_position_t(120,y,18,14));
				g_fill_selection_text.create_in_dialog_units(wnd, ui_helpers::window_position_t(120,y+=spacing,18,14));
				g_fill_selection_text_inactive.create_in_dialog_units(wnd, ui_helpers::window_position_t(120,y+=spacing,18,14));

				g_fill_active_item_frame.create_in_dialog_units(wnd, ui_helpers::window_position_t(120,162,18,14));
				
				y=85;

				g_fill_background.create_in_dialog_units(wnd, ui_helpers::window_position_t(225,y,18,14));
				g_fill_selection_background.create_in_dialog_units(wnd, ui_helpers::window_position_t(225,y+=spacing,18,14));
				g_fill_selection_background_inactive.create_in_dialog_units(wnd, ui_helpers::window_position_t(225,y+=spacing,18,14));

				ComboBox_AddString(m_wnd_colours_element, L"Global");
				;
				m_colours_client_list.g_get_list(m_colours_client_list);
				t_size i, count = m_colours_client_list.get_count();
				for (i=0; i<count; i++)
					ComboBox_AddString(m_wnd_colours_element, pfc::stringcvt::string_os_from_utf8(m_colours_client_list[i].m_name));

				ComboBox_SetCurSel(m_wnd_colours_element, 0);

				g_colours_manager_data.find_by_guid(m_element_guid, m_element_ptr);

				update_mode_combobox();
				update_fills();
				update_buttons();

				refresh_me(wnd);
			}
			break;
		case WM_DESTROY:
			{
				m_colours_client_list.remove_all();
				m_element_guid = pfc::guid_null;
				m_element_ptr.release();
				m_element_api.release();
				m_wnd = NULL;
			}
			break;
		case WM_COMMAND:
			switch(wp)
			{
			/*case IDC_IMPORT:
				{
					g_import_pv_colours_to_unified_global();
				}
				break;*/
			case IDC_COLOURS_ELEMENT|(CBN_SELCHANGE<<16):
				{
					int idx = ComboBox_GetCurSel((HWND)lp);
					m_element_api.release();
					if (idx != -1)
					{
						if (idx ==0)
							m_element_guid = pfc::guid_null;
						else
						{
							m_element_guid = m_colours_client_list[idx-1].m_guid;
							m_element_api = m_colours_client_list[idx-1].m_ptr;
						}
						g_colours_manager_data.find_by_guid(m_element_guid, m_element_ptr);
					}
					update_fills();
					update_buttons();
					update_mode_combobox();
				}
				return 0;
			case IDC_COLOURS_MODE|(CBN_SELCHANGE<<16):
				{
					int idx = ComboBox_GetCurSel((HWND)lp);
					m_element_ptr->colour_mode = (cui::colours::colour_mode_t)ComboBox_GetItemData((HWND)lp, idx);;
					update_fills();
					update_buttons();
					on_colour_changed();
				}
				return 0;
			case IDC_CHANGE_TEXT_BACK:
			case IDC_CHANGE_FRAME:
			case IDC_CHANGE_TEXT_FORE:
			case IDC_CHANGE_SELBACK:
			case IDC_CHANGE_SEL_FORE:
			case IDC_CHANGE_SEL_INACTIVE_BACK:
			case IDC_CHANGE_SEL_INACTIVE_FORE:
			case IDC_CUSTOM_FRAME:
				bool b_changed = false;
				if (wp == IDC_CHANGE_TEXT_BACK)
					b_changed = colour_picker(wnd, m_element_ptr->background, get_default_colour(colours::COLOUR_BACK));
				if (wp == IDC_CHANGE_TEXT_FORE)
					b_changed = colour_picker(wnd, m_element_ptr->text, get_default_colour(colours::COLOUR_TEXT));
				if (wp == IDC_CHANGE_SELBACK)
					b_changed = colour_picker(wnd, m_element_ptr->selection_background, get_default_colour(colours::COLOUR_SELECTED_BACK));
				if (wp == IDC_CHANGE_SEL_FORE)
					b_changed = colour_picker(wnd, m_element_ptr->selection_text, get_default_colour(colours::COLOUR_SELECTED_TEXT));
				if (wp == IDC_CHANGE_SEL_INACTIVE_BACK)
					b_changed = colour_picker(wnd, m_element_ptr->inactive_selection_background, get_default_colour(colours::COLOUR_SELECTED_BACK_NO_FOCUS));
				if (wp == IDC_CHANGE_SEL_INACTIVE_FORE)
					b_changed = colour_picker(wnd, m_element_ptr->inactive_selection_text, get_default_colour(colours::COLOUR_SELECTED_TEXT_NO_FOCUS));
				if (wp == IDC_CHANGE_FRAME)
					b_changed = colour_picker(wnd, m_element_ptr->active_item_frame, get_default_colour(colours::COLOUR_FRAME));
				if (wp == IDC_CUSTOM_FRAME)
				{
					b_changed = true;
					m_element_ptr->use_custom_active_item_frame = (Button_GetCheck((HWND)lp) == BST_CHECKED);
				}
				if (b_changed)
				{
					update_fills();
					on_colour_changed();
					//apply();
				}
				if (wp == IDC_CUSTOM_FRAME)
					update_buttons();
				return 0;
			}
			break;
		}
		return 0;
	}
	void apply()
	{
	}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_COLOURS_GLOBAL,wnd,g_on_message, (LPARAM)this);}
	virtual const char * get_name() {return "Colours";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:colours_and_fonts:colours";
		return true;
	}
	bool is_active() {return m_wnd != 0;}

private:
	bool initialising;
} g_tab_appearance;	

static class tab_appearance_fonts : public preferences_tab
{
	HWND m_wnd;
	HWND m_wnd_colours_mode;
	HWND m_wnd_colours_element;
	fonts_manager_data::entry_ptr_t m_element_ptr;
	cui::fonts::client::ptr m_element_api;
	fonts_client_list_t m_fonts_client_list;
public:
	tab_appearance_fonts() : initialising(false), m_wnd_colours_mode(NULL), m_wnd_colours_element(NULL), m_wnd(NULL) {};

	void refresh_me(HWND wnd)
	{
		initialising = true;	
		initialising = false;	
	}


	static BOOL CALLBACK g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		tab_appearance_fonts * p_data = NULL;
		if (msg == WM_INITDIALOG)
		{
			p_data = reinterpret_cast<tab_appearance_fonts*>(lp);
			SetWindowLongPtr(wnd, DWL_USER, lp);
		}
		else
			p_data = reinterpret_cast<tab_appearance_fonts*>(GetWindowLongPtr(wnd, DWL_USER));
		return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
	}

	void update_mode_combobox()
	{
		ComboBox_ResetContent(m_wnd_colours_mode);
		t_size index;
		t_size index_element = ComboBox_GetCurSel(m_wnd_colours_element);
		if (index_element <= 1)
		{
			index = ComboBox_AddString(m_wnd_colours_mode, L"System");
			ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_system);
		}
		else
		{
			index = ComboBox_AddString(m_wnd_colours_mode, L"Common (list items)");
			ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_common_items);
			index = ComboBox_AddString(m_wnd_colours_mode, L"Common (labels)");
			ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_common_labels);
		}
		index = ComboBox_AddString(m_wnd_colours_mode, L"Custom");
		ComboBox_SetItemData(m_wnd_colours_mode, index, cui::fonts::font_mode_custom);

		ComboBox_SetCurSel(m_wnd_colours_mode, combobox_find_by_id(m_wnd_colours_mode,m_element_ptr->font_mode));
	}

	void get_font (LOGFONT & lf)
	{
		t_size index_element = ComboBox_GetCurSel(m_wnd_colours_element);
		if (index_element <= 1)
			static_api_ptr_t<cui::fonts::manager>()->get_font(cui::fonts::font_type_t(index_element), lf);
		else
			static_api_ptr_t<cui::fonts::manager>()->get_font(m_element_api->get_client_guid(), lf);
	}

	void update_change()
	{
		EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_FONT), m_element_ptr->font_mode == cui::fonts::font_mode_custom);
	}

	void update_font_desc()
	{
		LOGFONT lf;
		get_font(lf);
		uSetWindowText(GetDlgItem(m_wnd, IDC_FONT_DESC), string_font_desc(lf));
	}

	void on_font_changed()
	{
		if (m_element_api.is_valid())
			m_element_api->on_font_changed();
		else
		{
			t_size index_element = ComboBox_GetCurSel(m_wnd_colours_element);
			if (index_element <= 1)
			{
				g_fonts_manager_data.g_on_common_font_changed(1<<index_element);
				t_size i, count = m_fonts_client_list.get_count();
				for (i=0; i<count; i++)
				{
					fonts_manager_data::entry_ptr_t p_data;
					g_fonts_manager_data.find_by_guid(m_fonts_client_list[i].m_guid, p_data);
					if (index_element == 0 && p_data->font_mode == cui::fonts::font_mode_common_items)
					{
						m_fonts_client_list[i].m_ptr->on_font_changed();
					}
					else if (index_element == 1 && p_data->font_mode == cui::fonts::font_mode_common_labels)
						m_fonts_client_list[i].m_ptr->on_font_changed();
				}
			}
		}
	}

	BOOL CALLBACK on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{

		switch(msg)
		{
		case WM_INITDIALOG:
			{
				m_wnd = wnd;
				m_wnd_colours_mode = GetDlgItem(wnd, IDC_COLOURS_MODE);
				m_wnd_colours_element = GetDlgItem(wnd, IDC_COLOURS_ELEMENT);

				m_fonts_client_list.g_get_list(m_fonts_client_list);

				ComboBox_AddString(m_wnd_colours_element, L"Common (list items)");
				ComboBox_AddString(m_wnd_colours_element, L"Common (labels)");

				t_size i, count = m_fonts_client_list.get_count();
				for (i=0; i<count; i++)
					ComboBox_AddString(m_wnd_colours_element, pfc::stringcvt::string_os_from_utf8(m_fonts_client_list[i].m_name));

				ComboBox_SetCurSel(m_wnd_colours_element, 0);
				m_element_ptr = g_fonts_manager_data.m_common_items_entry;

				update_mode_combobox();
				update_font_desc();
				update_change();

				refresh_me(wnd);
			}
			break;
		case WM_DESTROY:
			{
				m_wnd = NULL;
				m_fonts_client_list.remove_all();
				m_element_ptr.release();
				m_element_api.release();
			}
			break;
		case WM_COMMAND:
			switch(wp)
			{
			case IDC_CHANGE_FONT:
				{
					LOGFONT lf;
					get_font(lf);
					if (font_picker(lf, wnd))
					{
						m_element_ptr->font = lf;
						update_font_desc();
						on_font_changed();
					}
				}
				break;
			case IDC_COLOURS_MODE|(CBN_SELCHANGE<<16):
				{
					int idx = ComboBox_GetCurSel((HWND)lp);
					m_element_ptr->font_mode = (cui::fonts::font_mode_t)ComboBox_GetItemData((HWND)lp, idx);;
					update_font_desc();
					update_change();
					on_font_changed();
				}
				break;
			case IDC_COLOURS_ELEMENT|(CBN_SELCHANGE<<16):
				{
					int idx = ComboBox_GetCurSel((HWND)lp);
					m_element_api.release();
					if (idx != -1)
					{
						if (idx ==0)
							m_element_ptr = g_fonts_manager_data.m_common_items_entry;
						else if (idx ==1)
							m_element_ptr = g_fonts_manager_data.m_common_labels_entry;
						else if (idx >= 2)
						{
							m_element_api = m_fonts_client_list[idx-2].m_ptr;
							g_fonts_manager_data.find_by_guid(m_fonts_client_list[idx-2].m_guid, m_element_ptr);
						}
					}
					update_mode_combobox();
					update_font_desc();
					update_change();
				}
				return 0;
			/*case IDC_IMPORT:
				g_import_fonts_to_unified();
				break;*/
			}
			break;
		}
		return 0;
	}
	void apply()
	{
	}
	virtual HWND create(HWND wnd) {return uCreateDialog(IDD_FONTS_GLOBAL,wnd,g_on_message, (LPARAM)this);}
	virtual const char * get_name() {return "Fonts";}
	bool get_help_url(pfc::string_base & p_out)
	{
		p_out = "http://yuo.be/wiki/columns_ui:config:colours_and_fonts:fonts";
		return true;
	}
	bool is_active() {return m_wnd != 0;}

private:
	bool initialising;
} g_tab_appearance_fonts;	

cui::colours::colour_mode_t g_get_global_colour_mode() 
{	
	colours_manager_data::entry_ptr_t ptr;
	g_colours_manager_data.find_by_guid(pfc::guid_null, ptr);
	return ptr->colour_mode;
}

void g_set_global_colour_mode(cui::colours::colour_mode_t mode) 
{	
	colours_manager_data::entry_ptr_t ptr;
	g_colours_manager_data.find_by_guid(pfc::guid_null, ptr);
	if (ptr->colour_mode != mode)
	{
		ptr->colour_mode = mode;
		g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
		if (g_tab_appearance.is_active())
		{
			g_tab_appearance.update_mode_combobox();
			g_tab_appearance.update_fills();
		}
		colours_client_list_t m_colours_client_list;
		m_colours_client_list.g_get_list(m_colours_client_list);
		t_size i, count = m_colours_client_list.get_count();
		for (i=0; i<count; i++)
		{
			colours_manager_data::entry_ptr_t p_data;
			g_colours_manager_data.find_by_guid(m_colours_client_list[i].m_guid, p_data);
			if (p_data->colour_mode == cui::colours::colour_mode_global)
				m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
		}
	}
}

void g_import_pv_colours_to_unified_global()
{
	colours_manager_data::entry_ptr_t ptr;
	g_colours_manager_data.find_by_guid(pfc::guid_null, ptr);
	ptr->inactive_selection_background = cfg_pv_selceted_back_no_focus;
	ptr->inactive_selection_text = cfg_pv_selected_text_no_focus;
	ptr->selection_text = cfg_pv_selected_text_colour;
	ptr->selection_background = cfg_pv_selected_back;
	ptr->text = cfg_pv_text_colour;
	ptr->background = cfg_back;
	ptr->active_item_frame = cfg_focus;
	ptr->use_custom_active_item_frame = !cfg_pv_use_system_frame;
	if (cfg_pv_use_custom_colours==2)
		ptr->colour_mode = cui::colours::colour_mode_themed;
	else if (cfg_pv_use_custom_colours==1)
		ptr->colour_mode = cui::colours::colour_mode_custom;
	else
		ptr->colour_mode = cui::colours::colour_mode_system;
	if (g_tab_appearance.is_active())
	{
		g_tab_appearance.update_mode_combobox();
		g_tab_appearance.update_fills();
	}
	g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
	colours_client_list_t m_colours_client_list;
	m_colours_client_list.g_get_list(m_colours_client_list);
	t_size i, count = m_colours_client_list.get_count();
	for (i=0; i<count; i++)
	{
		colours_manager_data::entry_ptr_t p_data;
		g_colours_manager_data.find_by_guid(m_colours_client_list[i].m_guid, p_data);
		if (p_data->colour_mode == cui::colours::colour_mode_global)
			m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
	}
}

namespace fonts
{
// {82196D79-69BC-4041-8E2A-E3B4406BB6FC}
	const GUID columns_playlist_items = 
	{ 0x82196d79, 0x69bc, 0x4041, { 0x8e, 0x2a, 0xe3, 0xb4, 0x40, 0x6b, 0xb6, 0xfc } };

	// {B9D5EA18-5827-40be-A896-302A71BCAA9C}
	const GUID status_bar = 
	{ 0xb9d5ea18, 0x5827, 0x40be, { 0xa8, 0x96, 0x30, 0x2a, 0x71, 0xbc, 0xaa, 0x9c } };

	// {C0D3B76C-324D-46d3-BB3C-E81C7D3BCB85}
	const GUID columns_playlist_header = 
	{ 0xc0d3b76c, 0x324d, 0x46d3, { 0xbb, 0x3c, 0xe8, 0x1c, 0x7d, 0x3b, 0xcb, 0x85 } };

	// {19F8E0B3-E822-4f07-B200-D4A67E4872F9}
	const GUID ng_playlist_items = 
	{ 0x19f8e0b3, 0xe822, 0x4f07, { 0xb2, 0x0, 0xd4, 0xa6, 0x7e, 0x48, 0x72, 0xf9 } };

	// {30FBD64C-2031-4f0b-A937-F21671A2E195}
	const GUID ng_playlist_header = 
	{ 0x30fbd64c, 0x2031, 0x4f0b, { 0xa9, 0x37, 0xf2, 0x16, 0x71, 0xa2, 0xe1, 0x95 } };

	// {6F000FC4-3F86-4fc5-80EA-F7AA4D9551E6}
	const GUID splitter_tabs = 
	{ 0x6f000fc4, 0x3f86, 0x4fc5, { 0x80, 0xea, 0xf7, 0xaa, 0x4d, 0x95, 0x51, 0xe6 } };

	// {942C36A4-4E28-4cea-9644-F223C9A838EC}
	const GUID playlist_tabs = 
	{ 0x942c36a4, 0x4e28, 0x4cea, { 0x96, 0x44, 0xf2, 0x23, 0xc9, 0xa8, 0x38, 0xec } };

	// {70A5C273-67AB-4bb6-B61C-F7975A6871FD}
	const GUID playlist_switcher = 
	{ 0x70a5c273, 0x67ab, 0x4bb6, { 0xb6, 0x1c, 0xf7, 0x97, 0x5a, 0x68, 0x71, 0xfd } };

	// {D93F1EF3-4AEE-4632-B5BF-0220CEC76DED}
	const GUID filter_items = 
	{ 0xd93f1ef3, 0x4aee, 0x4632, { 0xb5, 0xbf, 0x2, 0x20, 0xce, 0xc7, 0x6d, 0xed } };

	// {FCA8752B-C064-41c4-9BE3-E125C7C7FC34}
	const GUID filter_header = 
	{ 0xfca8752b, 0xc064, 0x41c4, { 0x9b, 0xe3, 0xe1, 0x25, 0xc7, 0xc7, 0xfc, 0x34 } };
}

void g_import_fonts_to_unified(bool b_pv, bool b_ps, bool b_status )
{
	static_api_ptr_t<cui::fonts::manager> api;
	if (b_pv)
	{
		api->set_font(fonts::columns_playlist_items, cfg_font);
		api->set_font(fonts::columns_playlist_header, cfg_header_font);
		api->set_font(fonts::ng_playlist_items, cfg_font);
		api->set_font(fonts::ng_playlist_header, cfg_header_font);
		api->set_font(fonts::filter_items, cfg_font);
		api->set_font(fonts::filter_header, cfg_header_font);
	}
	if (b_ps)
	{
		api->set_font(fonts::playlist_switcher, cfg_plist_font);
		api->set_font(fonts::playlist_tabs, cfg_tab_font);
		api->set_font(fonts::splitter_tabs, cfg_tab_font);
	}
	if (b_status)
		api->set_font(fonts::status_bar, cfg_status_font);
	if (g_tab_appearance_fonts.is_active())
	{
		g_tab_appearance_fonts.update_mode_combobox();
		g_tab_appearance_fonts.update_font_desc();
		g_tab_appearance_fonts.update_change();
	}
}

static preferences_tab * g_tabs_appearance[] = 
{
	&g_tab_appearance, &g_tab_appearance_fonts
};

// {FA25D859-C808-485d-8AB7-FCC10F29ECE5}
const GUID g_guid_cfg_child_appearance = 
{ 0xfa25d859, 0xc808, 0x485d, { 0x8a, 0xb7, 0xfc, 0xc1, 0xf, 0x29, 0xec, 0xe5 } };

cfg_int cfg_child_appearance(g_guid_cfg_child_appearance,0);

class config_tabs : public preferences_page
{
	typedef config_tabs t_self;
public:
	HWND m_wnd_child;
	HWND m_wnd;
private:


	void make_child()
	{
		//HWND wnd_destroy = child;
		if (m_wnd_child)
		{
			ShowWindow(m_wnd_child, SW_HIDE);
			DestroyWindow(m_wnd_child);
			m_wnd_child=NULL;
		}

		HWND wnd_tab = GetDlgItem(m_wnd, IDC_TAB1);
		
		RECT tab;
		
		GetWindowRect(wnd_tab,&tab);
		MapWindowPoints(HWND_DESKTOP, m_wnd, (LPPOINT)&tab, 2);
		
		TabCtrl_AdjustRect(wnd_tab,FALSE,&tab);
		
		unsigned count = tabsize(g_tabs_appearance);
		if (cfg_child_appearance >= count) cfg_child_appearance = 0;

		if (cfg_child_appearance < count && cfg_child_appearance >= 0)
		{
			m_wnd_child = g_tabs_appearance[cfg_child_appearance]->create(m_wnd);
		}

		//SetWindowPos(child,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
		if (m_wnd_child) 
		{
			EnableThemeDialogTexture(m_wnd_child, ETDT_ENABLETAB);
		}

		SetWindowPos(m_wnd_child, 0, tab.left, tab.top, tab.right-tab.left, tab.bottom-tab.top, SWP_NOZORDER);
		SetWindowPos(wnd_tab,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		
		ShowWindow(m_wnd_child, SW_SHOWNORMAL);
		//UpdateWindow(child);
	}

	static BOOL CALLBACK g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		t_self * p_data = NULL;
		if (msg == WM_INITDIALOG)
		{
			p_data = reinterpret_cast<t_self*>(lp);
			SetWindowLongPtr(wnd, DWL_USER, lp);
		}
		else
			p_data = reinterpret_cast<t_self*>(GetWindowLongPtr(wnd, DWL_USER));
		return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
	}

	BOOL on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				m_wnd = wnd;

				HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
				unsigned n, count = tabsize(g_tabs_appearance);


				for (n=0; n<count; n++)
				{
					uTabCtrl_InsertItemText(wnd_tab, n, g_tabs_appearance[n]->get_name());
				}
				
				TabCtrl_SetCurSel(wnd_tab, cfg_child_appearance);
				
				make_child();
				
			}
			
			break;
		case WM_DESTROY:
			m_wnd = NULL;
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lp)->idFrom)
			{
			case IDC_TAB1:
				switch (((LPNMHDR)lp)->code)
				{
				case TCN_SELCHANGE:
					{
						cfg_child_appearance = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
						make_child();
					}
					break;
				}
				break;
			}
			break;
			
			
			case WM_PARENTNOTIFY:
				switch(wp)
				{
				case WM_DESTROY:
					{
						if (m_wnd_child && (HWND)lp == m_wnd_child)
							m_wnd_child = NULL;
					}
					break;	
				}
				break;
		}
		return 0;
	}
	



public:
	HWND create(HWND parent)
	{
		return uCreateDialog(IDD_HOST,parent,g_on_message, (LPARAM)this);
	}
	const char * get_name() {return "Colours and Fonts";}
	static const GUID guid;

	virtual bool get_help_url(pfc::string_base & p_out)
	{
		if (!(cfg_child_appearance < tabsize (g_tabs_appearance) && g_tabs_appearance[cfg_child_appearance]->get_help_url(p_out)))
			p_out = "http://yuo.be/wiki/columns_ui:manual";
		return true;
	}

	virtual GUID get_guid()
	{
		return guid;
	}
	virtual GUID get_parent_guid() 
	{
		static const GUID ret = 
		{ 0xdf6b9443, 0xdcc5, 0x4647, { 0x8f, 0x8c, 0xd6, 0x85, 0xbf, 0x25, 0xbd, 0x9 } };
		return ret;
	}
	virtual bool reset_query()	{return false;}
	virtual void reset() {};

};

// {41E6D7ED-A1DC-4d84-9BC9-352DAF7788B0}
const GUID config_tabs::guid = 
{ 0x41e6d7ed, 0xa1dc, 0x4d84, { 0x9b, 0xc9, 0x35, 0x2d, 0xaf, 0x77, 0x88, 0xb0 } };

preferences_page_factory_t<config_tabs> g_config_tabs;






class fcl_colours_t : public cui::fcl::dataset
{
	enum {stream_version=0};
	virtual void get_name (pfc::string_base & p_out) const
	{
		p_out = "Colours (unified)";
	}
	virtual const GUID & get_group () const
	{
		return cui::fcl::groups::colours_and_fonts;
	}
	virtual const GUID & get_guid () const
	{
		// {165946E7-6165-4680-A08E-84B5768458E8}
		static const GUID guid = 
		{ 0x165946e7, 0x6165, 0x4680, { 0xa0, 0x8e, 0x84, 0xb5, 0x76, 0x84, 0x58, 0xe8 } };
		return guid;
	}
	enum identifiers_t 
	{
		identifier_global_entry,
		identifier_client_entries,
		identifier_client_entry=0,
	};
	virtual void get_data (stream_writer * p_writer, t_uint32 type, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort) const
	{
		fcl::writer out(p_writer, p_abort);
		//p_writer->write_lendian_t(stream_version, p_abort);
		{
			stream_writer_memblock mem;
			g_colours_manager_data.m_global_entry->export(&mem, p_abort);
			out.write_item(identifier_global_entry, mem.m_data.get_ptr(), mem.m_data.get_size());
		}
		{
			stream_writer_memblock mem;
			fcl::writer out2(&mem, p_abort);
			t_size i, count = g_colours_manager_data.m_entries.get_count();
			mem.write_lendian_t(count, p_abort);
			for (i=0; i<count; i++)
			{
				stream_writer_memblock mem2;
				g_colours_manager_data.m_entries[i]->export(&mem2, p_abort);
				out2.write_item(identifier_client_entry, mem2.m_data.get_ptr(), mem2.m_data.get_size());
			}
			out.write_item(identifier_client_entries, mem.m_data.get_ptr(), mem.m_data.get_size());
		}
	}
	virtual void set_data (stream_reader * p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback & feedback, abort_callback & p_abort)
	{
		fcl::reader reader(p_reader, stream_size, p_abort);
		t_uint32 element_id;
		t_uint32 element_size;

		while (reader.get_remaining())
		{
			reader.read_item(element_id);
			reader.read_item(element_size);

			pfc::array_t<t_uint8> data;
			data.set_size(element_size);
			reader.read(data.get_ptr(), data.get_size());
			

			switch (element_id)
			{
			case identifier_global_entry:
				g_colours_manager_data.m_global_entry->import(&stream_reader_memblock_ref(data), data.get_size(), type, p_abort);
				break;
			case identifier_client_entries:
				{
					stream_reader_memblock_ref stream2(data);
					fcl::reader reader2(&stream2, data.get_size(), p_abort);

					t_size count, i;
					reader2.read_item(count);

					g_colours_manager_data.m_entries.remove_all();
					g_colours_manager_data.m_entries.set_count(count);

					for (i=0; i<count; i++)
					{
						t_uint32 element_id2;
						t_uint32 element_size2;
						reader2.read_item(element_id2);
						reader2.read_item(element_size2);
						if (element_id2 == identifier_client_entry)
						{
							pfc::array_t<t_uint8> data2;
							data2.set_size(element_size2);
							reader2.read(data2.get_ptr(), data2.get_size());
							g_colours_manager_data.m_entries[i] = new colours_manager_data::entry_t;
							g_colours_manager_data.m_entries[i]->import(&stream_reader_memblock_ref(data2), data2.get_size(), type, p_abort);
						}
						else
							reader2.skip(element_size2);
					}
				}
				break;
			default:
				reader.skip(element_size);
				break;
			};
		}
		if (g_tab_appearance.is_active())
		{
			g_tab_appearance.update_mode_combobox();
			g_tab_appearance.update_fills();
		}
		g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
		service_enum_t<cui::colours::client> colour_enum;
		cui::colours::client::ptr ptr;
		while (colour_enum.next(ptr))
			ptr->on_colour_changed(cui::colours::colour_flag_all);
	}
};

namespace {
service_factory_t<fcl_colours_t> g_fcl_colours_t;
};

class fcl_fonts_t : public cui::fcl::dataset
{
	enum {stream_version=0};
	virtual void get_name (pfc::string_base & p_out) const
	{
		p_out = "Fonts (unified)";
	}
	virtual const GUID & get_group () const
	{
		return cui::fcl::groups::colours_and_fonts;
	}
	virtual const GUID & get_guid () const
	{
		// {A806A9CD-4117-43da-805E-FE4EB348C90C}
		static const GUID guid = 
		{ 0xa806a9cd, 0x4117, 0x43da, { 0x80, 0x5e, 0xfe, 0x4e, 0xb3, 0x48, 0xc9, 0xc } };
		return guid;
	}
	enum identifiers_t 
	{
		identifier_global_items,
		identifier_global_labels,
		identifier_client_entries,
		identifier_client_entry=0,
	};
	virtual void get_data (stream_writer * p_writer, t_uint32 type, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort) const
	{
		fcl::writer out(p_writer, p_abort);
		//p_writer->write_lendian_t(stream_version, p_abort);
		{
			stream_writer_memblock mem;
			g_fonts_manager_data.m_common_items_entry->export(&mem, p_abort);
			out.write_item(identifier_global_items, mem.m_data.get_ptr(), mem.m_data.get_size());
		}
		{
			stream_writer_memblock mem;
			g_fonts_manager_data.m_common_labels_entry->export(&mem, p_abort);
			out.write_item(identifier_global_labels, mem.m_data.get_ptr(), mem.m_data.get_size());
		}
		{
			stream_writer_memblock mem;
			fcl::writer out2(&mem, p_abort);
			t_size i, count = g_fonts_manager_data.m_entries.get_count();
			mem.write_lendian_t(count, p_abort);
			for (i=0; i<count; i++)
			{
				stream_writer_memblock mem2;
				g_fonts_manager_data.m_entries[i]->export(&mem2, p_abort);
				out2.write_item(identifier_client_entry, mem2.m_data.get_ptr(), mem2.m_data.get_size());
			}
			out.write_item(identifier_client_entries, mem.m_data.get_ptr(), mem.m_data.get_size());
		}
	}
	virtual void set_data (stream_reader * p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback & feedback, abort_callback & p_abort)
	{
		fcl::reader reader(p_reader, stream_size, p_abort);
		t_uint32 element_id;
		t_uint32 element_size;

		while (reader.get_remaining())
		{
			reader.read_item(element_id);
			reader.read_item(element_size);

			pfc::array_t<t_uint8> data;
			data.set_size(element_size);
			reader.read(data.get_ptr(), data.get_size());
			

			switch (element_id)
			{
			case identifier_global_items:
				g_fonts_manager_data.m_common_items_entry->import(&stream_reader_memblock_ref(data), data.get_size(), type, p_abort);
				break;
			case identifier_global_labels:
				g_fonts_manager_data.m_common_labels_entry->import(&stream_reader_memblock_ref(data), data.get_size(), type, p_abort);
				break;
			case identifier_client_entries:
				{
					stream_reader_memblock_ref stream2(data);
					fcl::reader reader2(&stream2, data.get_size(), p_abort);

					t_size count, i;
					reader2.read_item(count);

					g_fonts_manager_data.m_entries.remove_all();
					g_fonts_manager_data.m_entries.set_count(count);

					for (i=0; i<count; i++)
					{
						t_uint32 element_id2;
						t_uint32 element_size2;
						reader2.read_item(element_id2);
						reader2.read_item(element_size2);
						if (element_id2 == identifier_client_entry)
						{
							pfc::array_t<t_uint8> data2;
							data2.set_size(element_size2);
							reader2.read(data2.get_ptr(), data2.get_size());
							g_fonts_manager_data.m_entries[i] = new fonts_manager_data::entry_t;
							g_fonts_manager_data.m_entries[i]->import(&stream_reader_memblock_ref(data2), data2.get_size(), type, p_abort);
						}
						else
							reader2.skip(element_size2);
					}
				}
				break;
			default:
				reader.skip(element_size);
				break;
			};
		}
		if (g_tab_appearance_fonts.is_active())
		{
			g_tab_appearance_fonts.update_mode_combobox();
			g_tab_appearance_fonts.update_font_desc();
			g_tab_appearance_fonts.update_change();
		}
		g_fonts_manager_data.g_on_common_font_changed(pfc_infinite);
		service_enum_t<cui::fonts::client> font_enum;
		cui::fonts::client::ptr ptr;
		while (font_enum.next(ptr))
			ptr->on_font_changed();
	}
};

namespace {
service_factory_t<fcl_fonts_t> g_fcl_fonts_t;
};

// {15FD4FF9-0622-4077-BFBB-DF0102B6A068}
const GUID colours_manager_data::g_cfg_guid = 
{ 0x15fd4ff9, 0x622, 0x4077, { 0xbf, 0xbb, 0xdf, 0x1, 0x2, 0xb6, 0xa0, 0x68 } };

// {6B71F91C-6B7E-4dbe-B27B-C493AA513FD0}
const GUID fonts_manager_data::g_cfg_guid = 
{ 0x6b71f91c, 0x6b7e, 0x4dbe, { 0xb2, 0x7b, 0xc4, 0x93, 0xaa, 0x51, 0x3f, 0xd0 } };

LRESULT appearance_message_window_t::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch (msg)
		{
		case WM_SYSCOLORCHANGE:
			{
				colours_client_list_t m_colours_client_list;
				m_colours_client_list.g_get_list(m_colours_client_list);
				t_size i, count = m_colours_client_list.get_count();
				bool b_global_custom = g_colours_manager_data.m_global_entry->colour_mode == cui::colours::colour_mode_custom;
				if (!b_global_custom)
					g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
				for (i=0; i<count; i++)
				{
					colours_manager_data::entry_ptr_t p_data;
					g_colours_manager_data.find_by_guid(m_colours_client_list[i].m_guid, p_data);
					if (p_data->colour_mode != cui::colours::colour_mode_custom && (!p_data->colour_mode == cui::colours::colour_mode_global || b_global_custom))
						m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
				}
			}
			break;
		case WM_SETTINGCHANGE:
			if (
				(wp == SPI_GETICONTITLELOGFONT && g_fonts_manager_data.m_common_items_entry.is_valid() && g_fonts_manager_data.m_common_items_entry->font_mode == cui::fonts::font_mode_system)
				|| (wp == SPI_GETNONCLIENTMETRICS && g_fonts_manager_data.m_common_labels_entry.is_valid() && g_fonts_manager_data.m_common_labels_entry->font_mode == cui::fonts::font_mode_system))
			{

				fonts_client_list_t m_fonts_client_list;
				m_fonts_client_list.g_get_list(m_fonts_client_list);
				t_size i, count = m_fonts_client_list.get_count();
				g_fonts_manager_data.g_on_common_font_changed(wp == SPI_GETICONTITLELOGFONT ? cui::fonts::font_type_flag_items : cui::fonts::font_type_flag_labels);
				for (i=0; i<count; i++)
				{
					fonts_manager_data::entry_ptr_t p_data;
					g_fonts_manager_data.find_by_guid(m_fonts_client_list[i].m_guid, p_data);
					if (wp == SPI_GETNONCLIENTMETRICS && p_data->font_mode == cui::fonts::font_mode_common_items)
						m_fonts_client_list[i].m_ptr->on_font_changed();
					else if (wp == SPI_GETICONTITLELOGFONT && p_data->font_mode == cui::fonts::font_mode_common_labels)
						m_fonts_client_list[i].m_ptr->on_font_changed();
				}
			}
			break;
		case WM_CLOSE:
			destroy();
			delete this;
			return 0;
		};
		return DefWindowProc(wnd, msg, wp, lp);
	}

#endif