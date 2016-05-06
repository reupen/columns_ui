#include "stdafx.h"
#include "item_details.h"
#include "config.h"

bool operator == (const font_data_t & item1, const font_data_t & item2) { return !font_data_t::g_compare(item1, item2); }

void font_change_info_t::reset(bool bKeepHandles /*= false*/)
{
	if (!bKeepHandles) m_fonts.set_size(0); m_font_changes.set_size(0);
}

bool font_change_info_t::find_font(const font_data_t & p_font, t_size & index)
{
	t_size i, count = m_fonts.get_count();
	for (i = 0; i < count; i++)
	{
		if (m_fonts[i]->m_data == p_font)
		{
			index = i;
			return true;
		}
	}

	return false;
}

titleformat_hook_change_font::titleformat_hook_change_font(const LOGFONT & lf)
{
	HDC dc = GetDC(0);
	m_default_font_size = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
	ReleaseDC(0, dc);

	m_default_font_face = pfc::stringcvt::string_utf8_from_wide(lf.lfFaceName, tabsize(lf.lfFaceName));
}

bool titleformat_hook_change_font::process_function(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag)
{
	p_found_flag = false;
	if (!stricmp_utf8_ex(p_name, p_name_length, "set_font", pfc_infinite))
	{
		switch (p_params->get_param_count())
		{
		case 2:
		case 3:
		{
			bool b_have_flags = p_params->get_param_count() == 3;
			const char * face, *pointsize, *flags;
			t_size face_length, pointsize_length, flags_length;
			p_params->get_param(0, face, face_length);
			p_params->get_param(1, pointsize, pointsize_length);
			if (b_have_flags)
				p_params->get_param(2, flags, flags_length);
			pfc::string8 temp;
			temp.add_byte('\x7');
			temp.add_string(face, face_length);
			temp.add_byte('\t');
			temp.add_string(pointsize, pointsize_length);
			temp.add_byte('\t');
			if (b_have_flags)
				temp.add_string(flags, flags_length);
			temp.add_byte('\x7');
			p_out->write(titleformat_inputtypes::unknown, temp);
			p_found_flag = true;
		}
		}
		return true;
	}
	else if (!stricmp_utf8_ex(p_name, p_name_length, "reset_font", pfc_infinite))
	{
		switch (p_params->get_param_count())
		{
		case 0:
		{
			p_out->write(titleformat_inputtypes::unknown, "\x07" "" "\x07", pfc_infinite);
			p_found_flag = true;
		}
		}
		return true;
	}
	else return false;
}

bool titleformat_hook_change_font::process_field(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, bool & p_found_flag)
{
	p_found_flag = false;
	if (!stricmp_utf8_ex(p_name, p_name_length, "default_font_face", pfc_infinite))
	{
		p_out->write(titleformat_inputtypes::unknown, m_default_font_face);
		p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name, p_name_length, "default_font_size", pfc_infinite))
	{
		p_out->write_int(titleformat_inputtypes::unknown, m_default_font_size);
		p_found_flag = true;
		return true;
	}
	return false;
}


void font_code_generator_t::initialise(const LOGFONT & p_lf_default, HWND parent, UINT edit)
{
	m_lf = p_lf_default;
	uSendDlgItemMessageText(parent, edit, WM_SETTEXT, 0, string_font_code(m_lf));
}

void font_code_generator_t::run(HWND parent, UINT edit)
{
	if (font_picker(m_lf, parent))
	{
		uSendDlgItemMessageText(parent, edit, WM_SETTEXT, 0, string_font_code(m_lf));
	}
}

font_code_generator_t::string_font_code::string_font_code(const LOGFONT & lf)
{
	prealloc(64);
	HDC dc = GetDC(0);
	unsigned pt = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
	ReleaseDC(0, dc);

	add_string("$set_font(");
	add_string(pfc::stringcvt::string_utf8_from_wide(lf.lfFaceName, tabsize(lf.lfFaceName)));
	add_byte(',');
	add_string(pfc::format_int(pt));
	add_string(",");
	if (lf.lfWeight == FW_BOLD)
		add_string("bold;");
	if (lf.lfItalic)
		add_string("italic;");
	add_string(")");
}

font_code_generator_t::string_font_code::operator const char *() const
{
	return get_ptr();
}



void g_parse_font_format_string(const char * str, t_size len, font_data_t & p_out)
{
	t_size ptr = 0;
	while (ptr < len)
	{
		t_size keyStart = ptr;
		while (ptr < len && str[ptr] != '=' && str[ptr] != ';') ptr++;
		t_size keyLen = ptr - keyStart;

		bool valueValid = false;
		t_size valueStart = 0;
		t_size valueLen = 0;

		if (str[ptr] == '=')
		{
			ptr++;
			valueStart = ptr;
			while (ptr < len && str[ptr] != ';') ptr++;
			valueLen = ptr - valueStart;
			ptr++;
			valueValid = true;
		}
		else if (ptr < len) ptr++;

		if (!stricmp_utf8_ex("bold", pfc_infinite, &str[keyStart], keyLen))
		{
			p_out.m_bold = (!valueValid || !stricmp_utf8_ex("true", pfc_infinite, &str[valueStart], valueLen));
		}
		else if (!stricmp_utf8_ex("italic", pfc_infinite, &str[keyStart], keyLen))
		{
			p_out.m_italic = (!valueValid || !stricmp_utf8_ex("true", pfc_infinite, &str[valueStart], valueLen));
		}
		else if (!stricmp_utf8_ex("underline", pfc_infinite, &str[keyStart], keyLen))
		{
			p_out.m_underline = (!valueValid || !stricmp_utf8_ex("true", pfc_infinite, &str[valueStart], valueLen));
		}
	}
}

class font_client_item_details : public cui::fonts::client
{
public:
	virtual const GUID & get_client_guid() const
	{
		return g_guid_item_details_font_client;
	}
	virtual void get_name(pfc::string_base & p_out) const
	{
		p_out = "Item Details";
	}

	virtual cui::fonts::font_type_t get_default_font_type() const
	{
		return cui::fonts::font_type_items;
	}

	virtual void on_font_changed() const
	{
		item_details_t::g_on_font_change();

	}
};

class colour_client_item_details : public cui::colours::client
{
public:
	virtual const GUID & get_client_guid() const
	{
		return g_guid_item_details_colour_client;
	}
	virtual void get_name(pfc::string_base & p_out) const
	{
		p_out = "Item Details";
	}

	virtual t_size get_supported_colours() const { return cui::colours::colour_flag_background | cui::colours::colour_flag_text; }; //bit-mask
	virtual t_size get_supported_bools() const { return 0; }; //bit-mask

	virtual bool get_themes_supported() const { return false; };

	virtual void on_bool_changed(t_size mask) const {};
	virtual void on_colour_changed(t_size mask) const
	{
		item_details_t::g_on_colours_change();
	};
};

namespace
{
	font_client_item_details::factory<font_client_item_details> g_font_client_item_details;
	colour_client_item_details::factory<colour_client_item_details> g_colour_client_item_details;
}
