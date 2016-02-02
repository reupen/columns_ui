#include "stdafx.h"

void toolbar_extension::button::callback_impl::on_button_state_change(unsigned p_new_state) //see t_button_state
{
	unsigned state = SendMessage(m_this->wnd_toolbar, TB_GETSTATE, id, 0);
	if (p_new_state & uie::BUTTON_STATE_ENABLED)
		state |= TBSTATE_ENABLED;
	else
		state = state & ~TBSTATE_ENABLED;
	if (p_new_state & uie::BUTTON_STATE_PRESSED)
		state |= TBSTATE_PRESSED;
	else
		state = state & ~TBSTATE_PRESSED;
	SendMessage(m_this->wnd_toolbar, TB_SETSTATE, id, MAKELONG(state, 0));
};


toolbar_extension::button::callback_impl & toolbar_extension::button::callback_impl::operator = (const callback_impl & p_source)
{
	m_this = p_source.m_this;
	return *this;
}
void toolbar_extension::button::callback_impl::set_wnd(toolbar_extension * p_source)
{
	m_this = p_source;
}
void toolbar_extension::button::callback_impl::set_id(const unsigned i)
{
	id = i;
}
toolbar_extension::button::callback_impl::callback_impl() : id(0){};


void toolbar_extension::button::set(const toolbar_extension::button & p_source)
{
	m_guid = p_source.m_guid;
	m_subcommand = p_source.m_subcommand;
	m_use_custom = p_source.m_use_custom;
	m_use_custom_hot = p_source.m_use_custom_hot;
	m_custom_image = p_source.m_custom_image;
	m_custom_hot_image = p_source.m_custom_hot_image;
	m_interface = p_source.m_interface;
	m_callback = p_source.m_callback;
	m_type = p_source.m_type;
	m_filter = p_source.m_filter;
	m_show = p_source.m_show;
	m_use_custom_text = p_source.m_use_custom_text;
	m_text = p_source.m_text;
}

//		inline button(const button & p_source) 
//		{
//			set(p_source);
//		}
toolbar_extension::button::button(
	GUID p_guid,
	bool p_custom,
	const char * p_custom_bitmap_path,
	const char * p_custom_bitmap_mask_path,
	COLORREF p_custom_bitmap_colour_mask,
	ui_extension::t_mask p_custom_bitmap_mask_type
	)
{
	m_guid = p_guid;
	m_use_custom = p_custom;
	if (p_custom_bitmap_path) m_custom_image.m_path = p_custom_bitmap_path;
	if (p_custom_bitmap_mask_path) m_custom_image.m_mask_path = p_custom_bitmap_mask_path;
	m_custom_image.m_mask_colour = p_custom_bitmap_colour_mask;
	m_custom_image.m_mask_type = p_custom_bitmap_mask_type;
	m_use_custom_hot = false;
	m_custom_hot_image.m_mask_type = uie::MASK_NONE;
	m_custom_hot_image.m_mask_colour = 0;
	m_type = TYPE_SEPARATOR;
	m_filter = FILTER_ACTIVE_SELECTION;
	m_show = SHOW_IMAGE;
	m_use_custom_text = false;

}
toolbar_extension::button::button() { ; }
const toolbar_extension::button & toolbar_extension::button::operator= (const toolbar_extension::button & p_source)
{
	set(p_source);
	return *this;
}
void toolbar_extension::button::write(stream_writer * out, abort_callback & p_abort) const
{
	out->write_lendian_t(m_type, p_abort);
	out->write_lendian_t(m_filter, p_abort);
	out->write_lendian_t((GUID&)m_guid, p_abort);
	out->write_lendian_t((GUID&)m_subcommand, p_abort);
	out->write_lendian_t(m_show, p_abort);
	out->write_lendian_t(m_use_custom, p_abort);
	if (m_use_custom)
	{
		m_custom_image.write(out, p_abort);
	}
	out->write_lendian_t(m_use_custom_hot, p_abort);
	if (m_use_custom_hot)
	{
		m_custom_hot_image.write(out, p_abort);
	}
	out->write_lendian_t(m_use_custom_text, p_abort);
	if (m_use_custom_text)
		out->write_string(m_text, p_abort);
}

void toolbar_extension::button::read(toolbar_extension::t_config_version p_version, stream_reader * reader, abort_callback & p_abort)
{
	*this = toolbar_extension::g_button_null;

	reader->read_lendian_t(m_type, p_abort);
	reader->read_lendian_t(m_filter, p_abort);
	reader->read_lendian_t((GUID&)m_guid, p_abort);
	if (p_version >= VERSION_2)
		reader->read_lendian_t((GUID&)m_subcommand, p_abort);
	reader->read_lendian_t(m_show, p_abort);
	reader->read_lendian_t(m_use_custom, p_abort);
	if (m_use_custom)
	{
		m_custom_image.read(p_version, reader, p_abort);
	}
	reader->read_lendian_t(m_use_custom_hot, p_abort);
	if (m_use_custom_hot)
	{
		m_custom_hot_image.read(p_version, reader, p_abort);
	}
	reader->read_lendian_t(m_use_custom_text, p_abort);
	if (m_use_custom_text)
	{
		pfc::string8 temp;
		reader->read_string(temp, p_abort);
		m_text = temp;
	}
}
void toolbar_extension::button::get_display_text(pfc::string_base & p_out) //display
{
	p_out.reset();
	if (m_use_custom_text)
		p_out = m_text;
	else
		get_short_name(p_out);
}
void toolbar_extension::button::get_short_name(pfc::string_base & p_out) //tooltip
{
	p_out.reset();
	if (m_type == TYPE_BUTTON)
		uie::custom_button::g_button_get_name(m_guid, p_out);
	else if (m_type == TYPE_SEPARATOR)
		p_out = "Separator";
	else if (m_type == TYPE_MENU_ITEM_MAIN)
		menu_helpers::mainpath_from_guid(m_guid, m_subcommand, p_out, true);
	else
		menu_helpers::contextpath_from_guid(m_guid, m_subcommand, p_out, true);
}

void toolbar_extension::button::get_name_type(pfc::string_base & p_out) //config
{
	p_out.reset();
	if (m_type == TYPE_BUTTON)
	{
		p_out = "Button";
	}
	else if (m_type == TYPE_SEPARATOR)
		p_out = "Separator";
	else if (m_type == TYPE_MENU_ITEM_MAIN)
	{
		p_out = "Main menu item";
	}
	else
	{
		p_out = "Shortcut menu item";
	}
}

void toolbar_extension::button::get_name_name(pfc::string_base & p_out) //config
{
	p_out.reset();
	if (m_type == TYPE_BUTTON)
	{
		pfc::string8 temp;
		if (uie::custom_button::g_button_get_name(m_guid, temp))
		{
			p_out += temp;
		}
	}
	else if (m_type == TYPE_SEPARATOR)
		p_out = "-";
	else if (m_type == TYPE_MENU_ITEM_MAIN)
	{
		pfc::string8 temp;
		menu_helpers::mainpath_from_guid(m_guid, m_subcommand, temp);
		p_out += temp;
	}
	else
	{
		pfc::string8 temp;
		menu_helpers::contextpath_from_guid(m_guid, m_subcommand, temp);
		p_out += temp;
	}
}
void toolbar_extension::button::get_name(pfc::string_base & p_out) //config
{
	p_out.reset();
	if (m_type == TYPE_BUTTON)
	{
		p_out = "[Button] ";
		pfc::string8 temp;
		if (uie::custom_button::g_button_get_name(m_guid, temp))
		{
			p_out += temp;
		}
	}
	else if (m_type == TYPE_SEPARATOR)
		p_out = "[Separator]";
	else if (m_type == TYPE_MENU_ITEM_MAIN)
	{
		pfc::string8 temp;
		p_out = "[Main menu item] ";
		menu_helpers::mainpath_from_guid(m_guid, m_subcommand, temp);
		p_out += temp;
	}
	else
	{
		pfc::string8 temp;
		menu_helpers::contextpath_from_guid(m_guid, m_subcommand, temp);
		p_out = "[Shortcut menu item] ";
		p_out += temp;
	}
}


void toolbar_extension::button::read_from_file(t_config_version p_version, const char * p_base, const char * p_name, stream_reader * p_file, unsigned p_size, abort_callback & p_abort)
{
	//t_filesize p_start = p_file->get_position(p_abort);
	t_size read = 0;
	while (read < p_size/* && !p_file->is_eof(p_abort)*/)
	{
		t_identifier_button id;
		p_file->read_lendian_t(id, p_abort);
		unsigned size;
		p_file->read_lendian_t(size, p_abort);
		//if (size > p_file->get_size(p_abort) - p_file->get_position(p_abort))
		//	throw exception_io_data();
		read += sizeof(t_uint32) + sizeof(t_uint32) + size;
		switch (id)
		{
		case I_BUTTON_TYPE:
			p_file->read_lendian_t(m_type, p_abort);
			break;
		case I_BUTTON_FILTER:
			p_file->read_lendian_t(m_filter, p_abort);
			break;
		case I_BUTTON_SHOW:
			p_file->read_lendian_t(m_show, p_abort);
			break;
		case I_BUTTON_GUID:
			p_file->read_lendian_t((GUID&)m_guid, p_abort);
			break;
		case I_BUTTON_SUBCOMMAND:
			p_file->read_lendian_t((GUID&)m_subcommand, p_abort);
			break;
		case I_BUTTON_CUSTOM:
			p_file->read_lendian_t(m_use_custom, p_abort);
			break;
		case I_BUTTON_CUSTOM_HOT:
			p_file->read_lendian_t(m_use_custom_hot, p_abort);
			break;
		case I_BUTTON_CUSTOM_DATA:
			m_custom_image.read_from_file(p_version, p_base, p_name, p_file, size, p_abort);
			break;
		case I_BUTTON_CUSTOM_HOT_DATA:
			m_custom_hot_image.read_from_file(p_version, p_base, p_name, p_file, size, p_abort);
			break;
		case I_BUTTON_USE_CUSTOM_TEXT:
			p_file->read_lendian_t(m_use_custom_text, p_abort);
			break;
		case I_BUTTON_TEXT:
		{
			pfc::array_t<char> name;
			name.set_size(size);
			p_file->read(name.get_ptr(), name.get_size(), p_abort);
			m_text.set_string(name.get_ptr(), name.get_size());
		}
		break;
		default:
			if (p_file->skip(size, p_abort) != size)
				throw exception_io_data_truncation();
			break;
		}
	}
}

void toolbar_extension::button::write_to_file(stream_writer &p_file, bool b_paths, abort_callback & p_abort)
{
	p_file.write_lendian_t(I_BUTTON_TYPE, p_abort);
	p_file.write_lendian_t(sizeof(m_type), p_abort);
	p_file.write_lendian_t(m_type, p_abort);

	p_file.write_lendian_t(I_BUTTON_FILTER, p_abort);
	p_file.write_lendian_t(sizeof(m_filter), p_abort);
	p_file.write_lendian_t(m_filter, p_abort);

	p_file.write_lendian_t(I_BUTTON_SHOW, p_abort);
	p_file.write_lendian_t(sizeof(m_show), p_abort);
	p_file.write_lendian_t(m_show, p_abort);

	p_file.write_lendian_t(I_BUTTON_GUID, p_abort);
	p_file.write_lendian_t(sizeof(m_guid), p_abort);
	p_file.write_lendian_t((GUID&)m_guid, p_abort);

	p_file.write_lendian_t(I_BUTTON_SUBCOMMAND, p_abort);
	p_file.write_lendian_t(sizeof(m_subcommand), p_abort);
	p_file.write_lendian_t((GUID&)m_subcommand, p_abort);

	p_file.write_lendian_t(I_BUTTON_CUSTOM, p_abort);
	p_file.write_lendian_t(sizeof(m_use_custom), p_abort);
	p_file.write_lendian_t(m_use_custom, p_abort);

	if (m_use_custom)
	{
		p_file.write_lendian_t(I_BUTTON_CUSTOM_DATA, p_abort);

		stream_writer_memblock p_write;
		m_custom_image.write_to_file(p_write, b_paths, p_abort);
		p_file.write_lendian_t(p_write.m_data.get_size(), p_abort);
		p_file.write(p_write.m_data.get_ptr(), p_write.m_data.get_size(), p_abort);
	}

	p_file.write_lendian_t(I_BUTTON_CUSTOM_HOT, p_abort);
	p_file.write_lendian_t(sizeof(m_use_custom_hot), p_abort);
	p_file.write_lendian_t(m_use_custom_hot, p_abort);

	if (m_use_custom_hot)
	{
		p_file.write_lendian_t(I_BUTTON_CUSTOM_HOT_DATA, p_abort);
		stream_writer_memblock p_write;
		m_custom_hot_image.write_to_file(p_write, b_paths, p_abort);
		p_file.write_lendian_t(p_write.m_data.get_size(), p_abort);
		p_file.write(p_write.m_data.get_ptr(), p_write.m_data.get_size(), p_abort);
	}

	p_file.write_lendian_t(I_BUTTON_USE_CUSTOM_TEXT, p_abort);
	p_file.write_lendian_t(sizeof(m_use_custom_text), p_abort);
	p_file.write_lendian_t(m_use_custom_text, p_abort);

	if (m_use_custom_text)
	{
		p_file.write_lendian_t(I_BUTTON_TEXT, p_abort);
		p_file.write_lendian_t(m_text.length(), p_abort);
		p_file.write(m_text, m_text.length(), p_abort);
	}
}
