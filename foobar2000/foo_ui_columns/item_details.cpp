#include "foo_ui_columns.h"
//#include <Richedit.h>

// {59B4F428-26A5-4a51-89E5-3945D327B4CB}
static const GUID g_guid_item_details = 
{ 0x59b4f428, 0x26a5, 0x4a51, { 0x89, 0xe5, 0x39, 0x45, 0xd3, 0x27, 0xb4, 0xcb } };

// {A834BCF6-7230-4ff6-8F30-2ED826EEE1D3}
static const GUID g_guid_item_details_tracking_mode = 
{ 0xa834bcf6, 0x7230, 0x4ff6, { 0x8f, 0x30, 0x2e, 0xd8, 0x26, 0xee, 0xe1, 0xd3 } };

// {16345DC1-2B8B-4351-A2B4-64736F667B22}
static const GUID g_guid_item_details_script = 
{ 0x16345dc1, 0x2b8b, 0x4351, { 0xa2, 0xb4, 0x64, 0x73, 0x6f, 0x66, 0x7b, 0x22 } };

// {77F3FA70-E39C-46f8-8E8A-6ECC64DDE234}
static const GUID g_guid_item_details_font_client = 
{ 0x77f3fa70, 0xe39c, 0x46f8, { 0x8e, 0x8a, 0x6e, 0xcc, 0x64, 0xdd, 0xe2, 0x34 } };

// {4E20CEED-42F6-4743-8EB3-610454457E19}
static const GUID g_guid_item_details_colour_client = 
{ 0x4e20ceed, 0x42f6, 0x4743, { 0x8e, 0xb3, 0x61, 0x4, 0x54, 0x45, 0x7e, 0x19 } };

// {3E3D189A-8154-4c9f-8E68-B278301271CD}
static const GUID g_guid_item_details_hscroll = 
{ 0x3e3d189a, 0x8154, 0x4c9f, { 0x8e, 0x68, 0xb2, 0x78, 0x30, 0x12, 0x71, 0xcd } };

// {AE00D056-AACB-4ca0-A2EC-FD2BAB599C1B}
static const GUID g_guid_item_details_horizontal_alignment = 
{ 0xae00d056, 0xaacb, 0x4ca0, { 0xa2, 0xec, 0xfd, 0x2b, 0xab, 0x59, 0x9c, 0x1b } };

// {07526EBA-2E7A-4e03-83ED-7BDE8FF79E8E}
static const GUID g_guid_item_details_vertical_alignment = 
{ 0x7526eba, 0x2e7a, 0x4e03, { 0x83, 0xed, 0x7b, 0xde, 0x8f, 0xf7, 0x9e, 0x8e } };

// {41753F1F-F2D6-4385-BEFA-B4BEC44A4167}
static const GUID g_guid_item_details_word_wrapping = 
{ 0x41753f1f, 0xf2d6, 0x4385, { 0xbe, 0xfa, 0xb4, 0xbe, 0xc4, 0x4a, 0x41, 0x67 } };

// {E944E1BF-0822-4141-B417-1F259D738ABC}
static const GUID g_guid_item_details_edge_style = 
{ 0xe944e1bf, 0x822, 0x4141, { 0xb4, 0x17, 0x1f, 0x25, 0x9d, 0x73, 0x8a, 0xbc } };


cfg_uint cfg_item_details_tracking_mode(g_guid_item_details_tracking_mode, 0);
cfg_uint cfg_item_details_edge_style(g_guid_item_details_edge_style, 2);
#if 0
cfg_string cfg_item_details_script(g_guid_item_details_script, "[%artist%]$crlf()[%title%]$crlf()[%album%][$crlf()$crlf()%lyrics%]");
#else
pfc::string8 cfg_item_details_script = "$set_font(%default_font_face%,$add(%default_font_size%,4),)[%artist%]$crlf()[%title%]$crlf()[%album%][$crlf()$crlf()%lyrics%]";
#endif
cfg_bool cfg_item_details_hscroll(g_guid_item_details_hscroll, false);
cfg_uint cfg_item_details_horizontal_alignment(g_guid_item_details_horizontal_alignment, ui_helpers::ALIGN_CENTRE);
cfg_uint cfg_item_details_vertical_alignment(g_guid_item_details_vertical_alignment, ui_helpers::ALIGN_CENTRE);
cfg_bool cfg_item_details_word_wrapping(g_guid_item_details_word_wrapping, true);

class line_info_t
{
public:
	t_size m_bytes;
	t_size m_raw_bytes;

	line_info_t() : m_bytes(NULL), m_raw_bytes(NULL) {};
};

class display_line_info_t : public line_info_t
{
public:
	t_size m_width;
	t_size m_height;
};

typedef pfc::list_t<line_info_t, pfc::alloc_fast_aggressive> line_info_list_t;
typedef pfc::list_t<display_line_info_t, pfc::alloc_fast_aggressive> display_line_info_list_t;

class font_data_t
{
public:
	pfc::string8 m_face;
	t_size m_point;
	bool m_bold,m_underline,m_italic;

	font_data_t() : m_point(10), m_bold(false), m_underline(false), m_italic(false) {};

	static int g_compare (const font_data_t & item1, const font_data_t & item2)
	{
		int ret = stricmp_utf8(item1.m_face, item2.m_face);
		if (ret == 0) ret = pfc::compare_t(item1.m_point, item2.m_point);
		if (ret == 0) ret = pfc::compare_t(item1.m_bold, item2.m_bold);
		if (ret == 0) ret = pfc::compare_t(item1.m_underline, item2.m_underline);
		if (ret == 0) ret = pfc::compare_t(item1.m_italic, item2.m_italic);
		return ret;
	}
};

bool operator == (const font_data_t & item1, const font_data_t & item2) {return !font_data_t::g_compare(item1, item2);}
class font_change_data_t
{
public:
	font_data_t m_font_data;
	bool m_reset;
	t_size m_character_index;

	font_change_data_t() : m_reset(false), m_character_index(NULL) {};
};

typedef pfc::list_t<font_change_data_t, pfc::alloc_fast_aggressive> font_change_data_list_t;

void g_parse_font_format_string (const char * str, t_size len, font_data_t & p_out)
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
			valueLen = ptr-valueStart;
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

void g_get_text_font_data (const char * p_text, pfc::string8_fast_aggressive & p_new_text, font_change_data_list_t & p_out)
{
	p_out.prealloc(32);

	const char * ptr = p_text;

	while (*ptr)
	{
		const char * addStart = ptr;
		while (*ptr && *ptr != '\x7') ptr++;
		p_new_text.add_string(addStart, ptr - addStart);
		if (*ptr == '\x7')
		{
			ptr++;
			const char * start = ptr;

			while (*ptr && *ptr != '\x7' && *ptr != '\t') ptr++;

			bool b_tab = false;

			if ( (b_tab = *ptr == '\t') || *ptr == '\x7')
			{
				font_change_data_t temp;
				t_size count = ptr - start;
				ptr++;

				if (b_tab)
				{
					const char * pSizeStart = ptr;
					while (*ptr && *ptr != '\x7' && *ptr != '\t') ptr++;
					t_size sizeLen = ptr - pSizeStart;

					temp.m_font_data.m_point = strtoul_n(pSizeStart, sizeLen);
					if ( (b_tab = *ptr == '\t') || *ptr == '\x7') ptr++;

					if (b_tab)
					{
						//ptr++;
						const char * pFormatStart = ptr;
						while (*ptr && *ptr != '\x7') ptr++;
						t_size formatLen = ptr - pFormatStart;
						if (*ptr == '\x7') 
						{
							ptr++;
							g_parse_font_format_string(pFormatStart, formatLen, temp.m_font_data);
						}
					}
				}
				else if (count == 0)
					temp.m_reset = true;

				temp.m_font_data.m_face.set_string(start, count);
				temp.m_character_index = p_new_text.get_length();//start-p_text-1;
				p_out.add_item(temp);
			}
		}
	}
}


class font_t : public pfc::refcounted_object_root
{
public:
	typedef pfc::refcounted_object_ptr_t<font_t> ptr_t;


	font_data_t m_data;

	gdi_object_t<HFONT>::ptr_t m_font;
	t_size m_height;
	//font_t (const font_t & p_font) : m_font(p_font.m_font), m_height(p_font.m_height), m_data(p_font.m_data) {};
};

typedef pfc::list_t<font_t::ptr_t> font_list_t;

class font_change_info_t
{
public:
	class font_change_entry_t
	{
	public:
		t_size m_text_index;
		font_t::ptr_t m_font;
	};

	font_list_t m_fonts;
	pfc::array_t<font_change_entry_t> m_font_changes;
	font_t::ptr_t m_default_font;

	bool find_font (const font_data_t & p_font, t_size & index)
	{
		t_size i, count = m_fonts.get_count();
		for (i=0; i<count; i++)
		{
			if (m_fonts[i]->m_data == p_font)
			{
				index = i;
				return true;
			}
		}
		
		return false;
	}

	void reset(bool bKeepHandles = false) {if (!bKeepHandles) m_fonts.set_size(0); m_font_changes.set_size(0);}
};

void g_get_text_font_info (const font_change_data_list_t & p_data, font_change_info_t & p_info)
{
	t_size i, count = p_data.get_count();
	if (count)
	{
		pfc::list_t<bool> maskKeepFonts;

		//maskKeepFonts.set_count(p_info.m_fonts.get_count());
		maskKeepFonts.add_items_repeat(false, p_info.m_fonts.get_count());

		//p_info.m_fonts.set_count (count);
		p_info.m_font_changes.set_count (count);

		HDC dc = GetDC(NULL);
		LOGFONT lf_base;
		memset (&lf_base, 0, sizeof(lf_base));

		pfc::stringcvt::string_wide_from_utf8_fast wideconv;

		for (i=0; i<count; i++)
		{
			if (p_data[i].m_reset)
			{
				p_info.m_font_changes[i].m_font = p_info.m_default_font;
			}
			else
			{
				wideconv.convert(p_data[i].m_font_data.m_face);
				LOGFONT lf = lf_base;
				wcsncpy_s(lf.lfFaceName, wideconv.get_ptr(), _TRUNCATE);
				lf.lfHeight = -MulDiv(p_data[i].m_font_data.m_point, GetDeviceCaps(dc, LOGPIXELSY), 72);
				if (p_data[i].m_font_data.m_bold)
					lf.lfWeight = FW_BOLD;
				if (p_data[i].m_font_data.m_underline)
					lf.lfUnderline = TRUE;
				if (p_data[i].m_font_data.m_italic)
					lf.lfItalic = TRUE;

				t_size index;
				if (p_info.find_font(p_data[i].m_font_data, index))
				{
					if (index < maskKeepFonts.get_count())
						maskKeepFonts[index] = true;
				}
				else
				{
					font_t::ptr_t font = new font_t;
					font->m_font = CreateFontIndirect(&lf);
					font->m_height = uGetFontHeight(font->m_font);
					font->m_data = p_data[i].m_font_data;
					index = p_info.m_fonts.add_item(font);
					//maskKeepFonts.add_item(true);
				}

				p_info.m_font_changes[i].m_font = p_info.m_fonts[index];
			}
			p_info.m_font_changes[i].m_text_index = p_data[i].m_character_index;

		}
		p_info.m_fonts.remove_mask(bit_array_not(bit_array_table(maskKeepFonts.get_ptr(), maskKeepFonts.get_count(), true)));

		ReleaseDC(NULL, dc);
	}
}

//void get_multiline_text_dimensions(HDC dc, pfc::string8_fast_aggressive & text, line_info_list_t & indices, t_size line_height, SIZE & sz, bool b_word_wrapping = false, t_size max_width = pfc_infinite);
void get_multiline_text_dimensions(HDC dc, pfc::string8_fast_aggressive & text_new, const line_info_list_t & rawLines, display_line_info_list_t & displayLines, const font_change_info_t & p_font_data, t_size line_height, SIZE & sz, bool b_word_wrapping = false, t_size max_width = pfc_infinite);
//void get_multiline_text_dimensions_const(HDC dc, const char * text, const line_info_list_t & indices, t_size line_height, SIZE & sz, bool b_word_wrapping = false, t_size max_width = pfc_infinite);
void get_multiline_text_dimensions_const(HDC dc, const char * text, const line_info_list_t & indices, const font_change_info_t & p_font_data, t_size line_height, SIZE & sz, bool b_word_wrapping = false, t_size max_width = pfc_infinite);
void g_get_text_multiline_data(const char * text, pfc::string8_fast_aggressive & p_out, pfc::list_t<line_info_t, pfc::alloc_fast_aggressive> & indices);

	class titleformat_hook_change_font : public titleformat_hook
	{
	public:
		virtual bool process_field(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,bool & p_found_flag)
		{
			p_found_flag = false;
			if (!stricmp_utf8_ex(p_name,p_name_length,"default_font_face",pfc_infinite))
			{
				p_out->write(titleformat_inputtypes::unknown, m_default_font_face);
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name,p_name_length,"default_font_size",pfc_infinite))
			{
				p_out->write_int(titleformat_inputtypes::unknown, m_default_font_size);
				p_found_flag = true;
				return true;
			}
			return false;
		}

		virtual bool process_function(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag)
		{
			p_found_flag = false;
			if (!stricmp_utf8_ex(p_name,p_name_length,"set_font",pfc_infinite))
			{
				switch(p_params->get_param_count())
				{
				case 2:
				case 3:
					{
						bool b_have_flags = p_params->get_param_count() == 3;
						const char * face,*pointsize,*flags;
						t_size face_length,pointsize_length,flags_length;
						p_params->get_param(0,face,face_length);
						p_params->get_param(1,pointsize,pointsize_length);
						if (b_have_flags)
							p_params->get_param(2,flags,flags_length);
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
			else if (!stricmp_utf8_ex(p_name,p_name_length,"reset_font",pfc_infinite))
			{
				switch(p_params->get_param_count())
				{
				case 0:
					{
						p_out->write(titleformat_inputtypes::unknown, "\x07" "" "\x07",pfc_infinite);
						p_found_flag = true;
					}
				}
				return true;
			}
			else return false;
		}

		titleformat_hook_change_font(const LOGFONT & lf)
		{
			HDC dc = GetDC(0);		
			m_default_font_size = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
			ReleaseDC(0, dc);

			m_default_font_face = pfc::stringcvt::string_utf8_from_wide(lf.lfFaceName, tabsize(lf.lfFaceName));
		}
	private:
		pfc::string8 m_default_font_face;
		t_size m_default_font_size;
	};


class item_details_t : 
	public uie::container_ui_extension,
	public ui_selection_callback,
	public play_callback,
	public playlist_callback_single,
	public metadb_io_callback_dynamic
{
	class message_window_t : public ui_helpers::container_window
	{
		virtual class_data & get_class_data() const;
		virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	};

	static message_window_t g_message_window;

	enum 
	{
		MSG_REFRESH = WM_USER+2,
	};

	virtual class_data & get_class_data()const 
	{
		long flags = 0;
		if (m_edge_style == 1) flags |= WS_EX_CLIENTEDGE;
		if (m_edge_style == 2) flags |= WS_EX_STATICEDGE;
		__implement_get_class_data_ex(_T("\r\nCUI_Item_Details_Panel"), _T(""), false, 0, WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CONTROLPARENT|flags, 0);
		//__implement_get_class_data(L"", false);
	}

public:
	enum track_mode_t
	{
		track_auto_playlist_playing,
		track_playlist,
		track_playing,
		track_auto_selection_playing,
		track_selection,
	};

	const bool g_track_mode_includes_now_playing(t_size mode) 
	{return mode == track_auto_playlist_playing || mode == track_auto_selection_playing || mode == track_playing;}

	const bool g_track_mode_includes_plalist(t_size mode) 
	{return mode == track_auto_playlist_playing || mode == track_playlist;}

	const bool g_track_mode_includes_auto(t_size mode) 
	{return mode == track_auto_playlist_playing || mode == track_auto_selection_playing;}

	const bool g_track_mode_includes_selection(t_size mode) 
	{return mode == track_auto_selection_playing || mode == track_selection;}

	class menu_node_track_mode : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
		t_size m_source;
	public:
		static const char * get_name(t_size source)
		{
			if (source == track_playing) return "Playing item";
			if (source == track_playlist) return "Playlist selection";
			if (source == track_auto_selection_playing) return "Automatic (current selection/playing item)";
			if (source == track_selection) return "Current selection";
			return "Automatic (playlist selection/playing item)";
		}
		virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags)const
		{
			p_out = get_name(m_source);
			p_displayflags= (m_source == p_this->m_tracking_mode) ? ui_extension::menu_node_t::state_radiochecked : 0;
			return true;
		}
		virtual bool get_description(pfc::string_base & p_out)const
		{
			return false;
		}
		virtual void execute()
		{
			p_this->m_tracking_mode = m_source;
			cfg_item_details_tracking_mode = m_source;
			p_this->on_tracking_mode_change();
		}
		menu_node_track_mode(item_details_t * p_wnd, t_size p_value) : p_this(p_wnd), m_source(p_value) {};
	};

	class menu_node_source_popup : public ui_extension::menu_node_popup_t
	{
		pfc::list_t<ui_extension::menu_node_ptr> m_items;
	public:
		virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags)const
		{
			p_out = "Displayed track";
			p_displayflags= 0;
			return true;
		}
		virtual unsigned get_children_count()const{return m_items.get_count();}
		virtual void get_child(unsigned p_index, uie::menu_node_ptr & p_out)const{p_out = m_items[p_index].get_ptr();}
		menu_node_source_popup(item_details_t * p_wnd)
		{
			m_items.add_item(new menu_node_track_mode(p_wnd, 3));
			m_items.add_item(new menu_node_track_mode(p_wnd, 0));
			m_items.add_item(new uie::menu_node_separator_t());
			m_items.add_item(new menu_node_track_mode(p_wnd, 2));
			m_items.add_item(new menu_node_track_mode(p_wnd, 4));
			m_items.add_item(new menu_node_track_mode(p_wnd, 1));
		};
	};

	class menu_node_alignment : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
		t_size m_type;
	public:
		static const char * get_name(t_size source)
		{
			if (source == 0) return "Left";
			else if (source == 1) return "Centre";
			else /*if (source == 2)*/ return "Right";
		
		}
		virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags)const
		{
			p_out = get_name(m_type);
			p_displayflags= (m_type == p_this->m_horizontal_alignment) ? ui_extension::menu_node_t::state_radiochecked : 0;
			return true;
		}
		virtual bool get_description(pfc::string_base & p_out)const
		{
			return false;
		}
		virtual void execute()
		{
			p_this->m_horizontal_alignment = m_type;
			cfg_item_details_horizontal_alignment = m_type;
			p_this->invalidate_all(false);
			p_this->update_scrollbar_range();
			p_this->update_now();
		}
		menu_node_alignment(item_details_t * p_wnd, t_size p_value) : p_this(p_wnd), m_type(p_value) {};
	};

	class menu_node_alignment_popup : public ui_extension::menu_node_popup_t
	{
		pfc::list_t<ui_extension::menu_node_ptr> m_items;
	public:
		virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags)const
		{
			p_out = "Alignment";
			p_displayflags= 0;
			return true;
		}
		virtual unsigned get_children_count()const{return m_items.get_count();}
		virtual void get_child(unsigned p_index, uie::menu_node_ptr & p_out)const{p_out = m_items[p_index].get_ptr();}
		menu_node_alignment_popup(item_details_t * p_wnd)
		{
			m_items.add_item(new menu_node_alignment(p_wnd, 0));
			m_items.add_item(new menu_node_alignment(p_wnd, 1));
			m_items.add_item(new menu_node_alignment(p_wnd, 2));
		};
	};

	class menu_node_options : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
	public:
		virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags)const
		{
			p_out = "Options";
			p_displayflags= 0;
			return true;
		}
		virtual bool get_description(pfc::string_base & p_out)const
		{
			return false;
		}
		virtual void execute();
		menu_node_options(item_details_t * p_wnd) : p_this(p_wnd) {};
	};
	class menu_node_hscroll : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
	public:
		virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags)const
		{
			p_out = "Allow horizontal scrolling";
			p_displayflags= (p_this->m_hscroll) ? ui_extension::menu_node_t::state_checked : 0;
			return true;
		}
		virtual bool get_description(pfc::string_base & p_out)const
		{
			return false;
		}
		virtual void execute()
		{
			p_this->m_hscroll = !p_this->m_hscroll;
			cfg_item_details_hscroll = p_this->m_hscroll;
			p_this->reset_display_info();
			p_this->invalidate_all(false);
			p_this->update_scrollbar_range();
			p_this->update_now();
		}
		menu_node_hscroll(item_details_t * p_wnd) : p_this(p_wnd) {};
	};

	class menu_node_wwrap : public ui_extension::menu_node_command_t
	{
		service_ptr_t<item_details_t> p_this;
	public:
		virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags)const
		{
			p_out = "Word wrapping";
			p_displayflags= (p_this->m_word_wrapping) ? ui_extension::menu_node_t::state_checked : 0;
			return true;
		}
		virtual bool get_description(pfc::string_base & p_out)const
		{
			return false;
		}
		virtual void execute()
		{
			p_this->m_word_wrapping = !p_this->m_word_wrapping;
			cfg_item_details_word_wrapping = p_this->m_word_wrapping;
			p_this->reset_display_info();
			p_this->invalidate_all(false);
			p_this->update_scrollbar_range();
			p_this->update_now();
		}
		menu_node_wwrap(item_details_t * p_wnd) : p_this(p_wnd) {};
	};

	//UIE funcs
	virtual const GUID & get_extension_guid() const;
	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;
	unsigned get_type () const;

	enum {stream_version_current=2};
	void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort);
	void get_config(stream_writer * p_writer, abort_callback & p_abort) const;
	bool have_config_popup()const;
	bool show_config_popup(HWND wnd_parent) ;

	void get_menu_items (ui_extension::menu_hook_t & p_hook);

	//UI SEL API
	void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr> & p_selection);

	//PC
	void on_playback_starting(play_control::t_track_command p_command,bool p_paused) {}
	void on_playback_new_track(metadb_handle_ptr p_track);
	void on_playback_stop(play_control::t_stop_reason p_reason);
	void on_playback_seek(double p_time);
	void on_playback_pause(bool p_state);
	void on_playback_edited(metadb_handle_ptr p_track);
	void on_playback_dynamic_info(const file_info & p_info);
	void on_playback_dynamic_info_track(const file_info & p_info);
	void on_playback_time(double p_time);
	void on_volume_change(float p_new_val) {}

	//PL
	enum {playlist_callback_flags = 
		playlist_callback_single::flag_on_items_selection_change
		|playlist_callback_single::flag_on_playlist_switch
	};
	void on_playlist_switch();
	void on_item_focus_change(t_size p_from,t_size p_to) {};

	void on_items_added(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection) {}
	void on_items_reordered(const t_size * p_order,t_size p_count) {}
	void on_items_removing(const bit_array & p_mask,t_size p_old_count,t_size p_new_count) {}
	void on_items_removed(const bit_array & p_mask,t_size p_old_count,t_size p_new_count) {}
	void on_items_selection_change(const bit_array & p_affected,const bit_array & p_state);
	void on_items_modified(const bit_array & p_mask) {}
	void on_items_modified_fromplayback(const bit_array & p_mask,play_control::t_display_level p_level) {}
	void on_items_replaced(const bit_array & p_mask,const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & p_data) {}
	void on_item_ensure_visible(t_size p_idx) {}

	void on_playlist_renamed(const char * p_new_name,t_size p_new_name_len) {}
	void on_playlist_locked(bool p_locked) {}

	void on_default_format_changed() {}
	void on_playback_order_changed(t_size p_new_index) {}

	//ML
	void on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook);

	static void g_on_app_activate(bool b_activated);
	static void g_redraw_all();
	static void g_on_font_change();
	static void g_on_colours_change();

	void set_horizontal_alignment(t_size horizontal_alignment)
	{
		if (get_wnd())
		{
			m_horizontal_alignment = horizontal_alignment;
			invalidate_all(false);
			update_scrollbar_range();
			update_now();
		}
	}
	void set_vertical_alignment(t_size vertical_alignment)
	{
		if (get_wnd())
		{
			m_vertical_alignment = vertical_alignment;
			invalidate_all(false);
			update_scrollbar_range();
			update_now();
		}
	}

	void set_edge_style(t_size edge_style)
	{
		m_edge_style = edge_style;
		if (get_wnd())
		{
			on_edge_style_change();
		}
	}

	void on_edge_style_change()
	{
		long flags = 0;

		if (m_edge_style == 1) flags |= WS_EX_CLIENTEDGE;
		if (m_edge_style == 2) flags |= WS_EX_STATICEDGE;
		SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, flags);
		SetWindowPos(get_wnd(),0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
	}

	void set_script(const char * p_script)
	{
		m_script = p_script;

		if (get_wnd())
		{
			m_to.release();
			static_api_ptr_t<titleformat_compiler>()->compile_safe(m_to, m_script);

			on_edge_style_change();
			refresh_contents();
		}
	}

	void set_config_wnd(HWND wnd) {m_wnd_config = wnd;}

	item_details_t() : m_callback_registered(false), m_tracking_mode(cfg_item_details_tracking_mode),
		m_script(cfg_item_details_script), m_horizontal_alignment(cfg_item_details_horizontal_alignment), m_hscroll(cfg_item_details_hscroll),
		m_nowplaying_active(false), m_word_wrapping(cfg_item_details_word_wrapping), m_font_change_info_valid(false),
		m_display_info_valid(false), m_edge_style(cfg_item_details_edge_style), 
		m_vertical_alignment(cfg_item_details_vertical_alignment),
		m_wnd_config(NULL)
		//, m_update_scrollbar_range_in_progress(false)
		//m_library_richedit(NULL), m_wnd_richedit(NULL)
	{
		m_display_sz.cx = (NULL); m_display_sz.cy = (NULL);
	};
private:
	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	void register_callback();
	void deregister_callback();
	void on_app_activate(bool b_activated);
	void refresh_contents(bool b_new_track = true);
	void update_font_change_info();
	void reset_font_change_info();
	void update_display_info(HDC dc);
	void update_display_info();
	void reset_display_info();
	void on_tracking_mode_change();
	bool check_process_on_selection_changed();

	void scroll(INT sb, int position, bool b_absolute);

	void invalidate_all(bool b_update=true);
	void update_now();
	void update_scrollbar_range(bool b_set_pos = true);

	void on_size ();
	void on_size (t_size cx, t_size cy);

	void on_font_change();
	void on_colours_change();

	static pfc::ptr_list_t<item_details_t> g_windows;

	ui_selection_holder::ptr m_selection_holder;
	metadb_handle_list m_handles, m_selection_handles;
	bool m_callback_registered, m_nowplaying_active;//, m_update_scrollbar_range_in_progress;
	t_size m_tracking_mode;

	bool m_font_change_info_valid;
	font_change_info_t m_font_change_info;
	font_change_data_list_t m_font_change_data;

	line_info_list_t m_line_info;
	display_line_info_list_t m_display_line_info;
	bool m_display_info_valid;

	SIZE m_display_sz;

	pfc::string8 m_current_text, m_script, m_current_text_raw;
	pfc::string8_fast_aggressive m_current_display_text;
	titleformat_object::ptr m_to;

	t_size m_horizontal_alignment, m_vertical_alignment;
	t_size m_edge_style;
	bool m_hscroll, m_word_wrapping;

	HWND m_wnd_config;

	//gdi_object_t<HFONT>::ptr_t m_default_font;

	//HINSTANCE m_library_richedit;
	//HWND m_wnd_richedit;
};

class font_code_generator_t
{
	class string_font_code : private pfc::string8_fast_aggressive
	{
	public:
		operator const char * () const {return get_ptr();}
		string_font_code(const LOGFONT & lf)
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
	};
public:
	void run(HWND parent, UINT edit)
	{
		if (font_picker(m_lf, parent))
		{
			uSendDlgItemMessageText(parent, edit, WM_SETTEXT, 0, string_font_code(m_lf));
		}
	}
	void initialise(const LOGFONT & p_lf_default, HWND parent, UINT edit)
	{
		m_lf = p_lf_default;
		uSendDlgItemMessageText(parent, edit, WM_SETTEXT, 0, string_font_code(m_lf));
	}
private:
	LOGFONT m_lf;
};

class item_details_config_t
{
public:
	pfc::string8 m_script;
	t_size m_edge_style, m_horizontal_alignment, m_vertical_alignment;
	font_code_generator_t m_font_code_generator;
	bool m_modal, m_timer_active;
	service_ptr_t<item_details_t> m_this;
	HWND m_wnd;

	enum {timer_id = 100};

	static BOOL CALLBACK g_DialogProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		item_details_config_t * p_data = NULL;
		if (msg == WM_INITDIALOG)
		{
			p_data = reinterpret_cast<item_details_config_t*>(lp);
			SetWindowLongPtr(wnd, DWL_USER, lp);
		}
		else
			p_data = reinterpret_cast<item_details_config_t*>(GetWindowLongPtr(wnd, DWL_USER));
		return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
	}
	item_details_config_t(const char * p_text, t_size edge_style, t_size halign, t_size valign)
		: m_script(p_text), m_edge_style(edge_style), m_horizontal_alignment(halign),
		m_vertical_alignment(valign), m_modal(true), m_timer_active(false), m_wnd(NULL) {};

	bool run_modal(HWND wnd) {m_modal = true; return uDialogBox(IDD_ITEMDETAILS_CONFIG, wnd, g_DialogProc, (LPARAM)this) != 0;}
	void run_modeless(HWND wnd, item_details_t* p_this) {m_modal = false; m_this = p_this; if (!uCreateDialog(IDD_ITEMDETAILS_CONFIG, wnd, g_DialogProc, (LPARAM)this)) delete this;}
private:
	void kill_timer()
	{
		if (m_timer_active)
		{
			KillTimer(m_wnd, timer_id);
			m_timer_active = false;
		}
	}
	void start_timer() 
	{
		kill_timer();
	
		SetTimer(m_wnd, timer_id, 667, NULL);
		m_timer_active = true;
	}
	void on_timer()
	{
		m_this->set_script(m_script);
		kill_timer();
	}
	BOOL CALLBACK on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		/*case DM_GETDEFID:
			SetWindowLongPtr(wnd, DWL_MSGRESULT, MAKELONG(m_modal ? IDOK : IDCANCEL, DC_HASDEFID));
			return TRUE;*/
		case WM_INITDIALOG:
			{
				m_wnd = wnd;

				if (!m_modal)
				{
					modeless_dialog_manager::g_add(wnd);
					m_this->set_config_wnd(wnd);
					ShowWindow(GetDlgItem(wnd, IDOK), SW_HIDE);
					SetWindowText(GetDlgItem(wnd, IDCANCEL), L"Close");
				}

				uSetWindowText(GetDlgItem(wnd, IDC_SCRIPT), m_script);
				HWND wnd_combo = GetDlgItem(wnd, IDC_EDGESTYLE);
				ComboBox_AddString(wnd_combo, L"None");
				ComboBox_AddString(wnd_combo, L"Sunken");
				ComboBox_AddString(wnd_combo, L"Grey");
				ComboBox_SetCurSel(wnd_combo, m_edge_style);

				wnd_combo = GetDlgItem(wnd, IDC_HALIGN);
				ComboBox_AddString(wnd_combo, L"Left");
				ComboBox_AddString(wnd_combo, L"Centre");
				ComboBox_AddString(wnd_combo, L"Right");
				ComboBox_SetCurSel(wnd_combo, m_horizontal_alignment);

				wnd_combo = GetDlgItem(wnd, IDC_VALIGN);
				ComboBox_AddString(wnd_combo, L"Top");
				ComboBox_AddString(wnd_combo, L"Centre");
				ComboBox_AddString(wnd_combo, L"Bottom");
				ComboBox_SetCurSel(wnd_combo, m_vertical_alignment);

				LOGFONT lf;
				static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_item_details_font_client, lf);
				m_font_code_generator.initialise(lf, wnd, IDC_FONT_CODE);

				colour_code_gen(wnd, IDC_COLOUR_CODE, false, true);

				if (!m_modal)
				{
					SendMessage(wnd, DM_SETDEFID, IDCANCEL, NULL);
					SetFocus (GetDlgItem(wnd, IDCANCEL));
					return FALSE;
				}
				else
					return FALSE;

			}
			return FALSE;// m_modal ? FALSE : TRUE;
		case WM_DESTROY:
			if (m_timer_active)
				on_timer();
			if (!m_modal)
				m_this->set_config_wnd(NULL);
			break;
		case WM_NCDESTROY:
			m_wnd = NULL;
			if (!m_modal)
			{
				modeless_dialog_manager::g_remove(wnd);
				SetWindowLongPtr(wnd, DWL_USER, NULL);
				delete this;
			}
			break;
		case WM_ERASEBKGND:
			SetWindowLongPtr(wnd, DWL_MSGRESULT, TRUE);
			return TRUE;
		case WM_PAINT:
			ui_helpers::innerWMPaintModernBackground(wnd, GetDlgItem(wnd, IDOK));
			return TRUE;
		case WM_CTLCOLORSTATIC:
			SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
			SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
			return (BOOL)GetSysColorBrush(COLOR_WINDOW);
		case WM_CLOSE:
			if (m_modal)
			{
				SendMessage(wnd, WM_COMMAND, IDCANCEL, NULL);
				return TRUE;
			}
			break;
		case WM_TIMER:
			if (wp == timer_id) on_timer();
			break;
		case WM_COMMAND:
			switch (LOWORD(wp))
			{
			case IDOK:
				if (m_modal)
					EndDialog(wnd, 1);
				return TRUE;
			case IDCANCEL:
				if (m_modal)
				EndDialog(wnd, 0);
				else
				{
					DestroyWindow(wnd);
				}
				return TRUE;
			case IDC_GEN_COLOUR:
				colour_code_gen(wnd, IDC_COLOUR_CODE, false, false);
				break;
			case IDC_GEN_FONT:
				m_font_code_generator.run(wnd, IDC_FONT_CODE);
				break;
			case IDC_SCRIPT:
				switch (HIWORD(wp))
				{
				case EN_CHANGE:
					m_script = string_utf8_from_window(HWND(lp));
					if (!m_modal)
						start_timer();
					break;
				}
				break;
			case IDC_EDGESTYLE:
				switch (HIWORD(wp))
				{
				case CBN_SELCHANGE:
					m_edge_style = ComboBox_GetCurSel((HWND)lp);
					if (!m_modal)
					{
						m_this->set_edge_style(m_edge_style);
						cfg_item_details_edge_style = m_edge_style;
					}
					break;
				}
				break;
			case IDC_HALIGN:
				switch (HIWORD(wp))
				{
				case CBN_SELCHANGE:
					m_horizontal_alignment = ComboBox_GetCurSel((HWND)lp);
					if (!m_modal)
					{
						m_this->set_horizontal_alignment(m_horizontal_alignment);
						cfg_item_details_horizontal_alignment = m_horizontal_alignment;
					}
					break;
				}
				break;
			case IDC_VALIGN:
				switch (HIWORD(wp))
				{
				case CBN_SELCHANGE:
					m_vertical_alignment = ComboBox_GetCurSel((HWND)lp);
					if (!m_modal)
					{
						m_this->set_vertical_alignment(m_vertical_alignment);
						cfg_item_details_vertical_alignment = m_vertical_alignment;
					}
					break;
				}
				break;
			}
			break;
		}
		return FALSE;
	}
};

void item_details_t::menu_node_options::execute()
{
	if (p_this->m_wnd_config)
		SetActiveWindow (p_this->m_wnd_config);
	else
	{
		item_details_config_t * p_dialog = new item_details_config_t(p_this->m_script, p_this->m_edge_style, p_this->m_horizontal_alignment, p_this->m_vertical_alignment);
		p_dialog->run_modeless(GetAncestor(p_this->get_wnd(), GA_ROOT), p_this.get_ptr());
	}
}

bool item_details_t::have_config_popup()const {return true;}
bool item_details_t::show_config_popup(HWND wnd_parent) 
{
	item_details_config_t dialog(m_script, m_edge_style, m_horizontal_alignment, m_vertical_alignment);
	if (dialog.run_modal(wnd_parent))
	{
		m_script = dialog.m_script;
		m_edge_style = dialog.m_edge_style;
		cfg_item_details_edge_style = m_edge_style;
		m_horizontal_alignment = dialog.m_horizontal_alignment;
		m_vertical_alignment = dialog.m_vertical_alignment;
		cfg_item_details_horizontal_alignment = m_horizontal_alignment;
		cfg_item_details_vertical_alignment = m_vertical_alignment;

		if (get_wnd())
		{
			m_to.release();
			static_api_ptr_t<titleformat_compiler>()->compile_safe(m_to, m_script);

			on_edge_style_change();
			refresh_contents();
		}
		return true;
	}
	return false;
}

void item_details_t::set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort)
{
	if (p_size)
	{
		t_size version;
		p_reader->read_lendian_t(version, p_abort);
		if (version <= stream_version_current)
		{
			p_reader->read_string(m_script, p_abort);
			p_reader->read_lendian_t(m_tracking_mode, p_abort);
			p_reader->read_lendian_t(m_hscroll, p_abort);
			p_reader->read_lendian_t(m_horizontal_alignment, p_abort);
			if (version >= 1)
			{
				p_reader->read_lendian_t(m_word_wrapping, p_abort);
				if (version >= 2)
				{
					p_reader->read_lendian_t(m_edge_style, p_abort);
					p_reader->read_lendian_t(m_vertical_alignment, p_abort);
				}
			}
		}
	}
}

void item_details_t::get_config(stream_writer * p_writer, abort_callback & p_abort) const
{
	p_writer->write_lendian_t((t_size)stream_version_current, p_abort);
	p_writer->write_string(m_script, p_abort);
	p_writer->write_lendian_t(m_tracking_mode, p_abort);
	p_writer->write_lendian_t(m_hscroll, p_abort);
	p_writer->write_lendian_t(m_horizontal_alignment, p_abort);
	p_writer->write_lendian_t(m_word_wrapping, p_abort);
	p_writer->write_lendian_t(m_edge_style, p_abort);
	p_writer->write_lendian_t(m_vertical_alignment, p_abort);
}

void item_details_t::get_menu_items (ui_extension::menu_hook_t & p_hook)
{
	ui_extension::menu_node_ptr p_node;
	p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_source_popup(this)));
	//p_node = new menu_node_alignment_popup(this);
	//p_hook.add_node(p_node);
	p_node = new menu_node_hscroll(this);
	p_hook.add_node(p_node);
	p_node = new menu_node_wwrap(this);
	p_hook.add_node(p_node);
	p_node = new menu_node_options(this);
	p_hook.add_node(p_node);
}

item_details_t::message_window_t item_details_t::g_message_window;

pfc::ptr_list_t<item_details_t> item_details_t::g_windows;

item_details_t::message_window_t::class_data & item_details_t::message_window_t::get_class_data() const 
{
	__implement_get_class_data_ex(_T("\r\n{6EB3EA81-7C5E-468d-B507-E5527F52361B}"), _T(""), false, 0, 0, 0, 0);
}

LRESULT item_details_t::message_window_t::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_CREATE:
		break;
	case WM_ACTIVATEAPP:
		item_details_t::g_on_app_activate(wp != 0);
		break;
	case WM_DESTROY:
		break;

	}
	return uDefWindowProc(wnd, msg, wp, lp);
}

void item_details_t::g_on_app_activate(bool b_activated)
{
	t_size i, count = g_windows.get_count();
	for (i=0; i<count; i++)
		g_windows[i]->on_app_activate(b_activated);
}
void item_details_t::on_app_activate(bool b_activated)
{
	if (b_activated)
	{
		if (GetFocus() != get_wnd())
			register_callback();
	}
	else
	{
		deregister_callback();
	}
}

const GUID & item_details_t::get_extension_guid() const {return g_guid_item_details;}
void item_details_t::get_name(pfc::string_base & p_out)const {p_out = "Item details";}
void item_details_t::get_category(pfc::string_base & p_out)const {p_out = "Panels";}
unsigned item_details_t::get_type () const {return uie::type_panel;}

void item_details_t::register_callback()
{
	if (!m_callback_registered)
		g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
	m_callback_registered = true;
}
void item_details_t::deregister_callback()
{
	if (m_callback_registered)
		static_api_ptr_t<ui_selection_manager>()->unregister_callback(this);
	m_callback_registered = false;
}

void item_details_t::update_scrollbar_range(bool b_set_pos)
{
	//if (m_update_scrollbar_range_in_progress) return;

	//pfc::vartoggle_t<bool> vart(m_update_scrollbar_range_in_progress, true);
	SCROLLINFO si, si2;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si2 = si;

	RECT rc;
	GetClientRect(get_wnd(), &rc);

	RECT rc_old = rc;

	//SIZE & sz = m_display_sz;
	{
		update_font_change_info();

		update_display_info();
	}

	int vMax = 1;
	
	vMax = m_display_sz.cy ? m_display_sz.cy-1 : 0;
	vMax = max (vMax, 1);

	si.fMask = SIF_RANGE|SIF_PAGE;
	si.nPage = RECT_CY(rc);
	si.nPage = max (si.nPage, 1);

	si.nMin = 0;
	si.nMax = vMax;
	SetScrollInfo(get_wnd(), SB_VERT, &si, TRUE);

#if 1
	GetClientRect(get_wnd(), &rc);
	update_display_info();

	vMax = m_display_sz.cy ? m_display_sz.cy-1 : 0;
	vMax = max (vMax, 1);
#else
	GetClientRect(get_wnd(), &rc);
	if (!EqualRect(&rc_old, &rc))
	{
		HDC dc = GetDC(get_wnd());
		HFONT fnt_old = SelectFont(dc, m_font_change_info.m_default_font->m_font.get());
		get_multiline_text_dimensions_const(dc, m_current_text, m_line_info, m_font_change_info, uGetTextHeight(dc)+2, sz, m_word_wrapping, RECT_CX(rc)>4 ? RECT_CX(rc)-4:0);
		SelectFont(dc, fnt_old);
		ReleaseDC(get_wnd(), dc);

		rc_old = rc;

		vMax = sz.cy ? sz.cy-1 : 0;
		vMax = max (vMax, 1);
	}
#endif

	si2.fMask = SIF_PAGE;
	GetScrollInfo(get_wnd(), SB_VERT, &si2);
	//bool b_has_vscrollbar = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_VSCROLL) != 0;
	bool b_need_vscrollbar = vMax >= (int)si2.nPage;
	//if (b_need_vscrollbar != b_has_vscrollbar)
		ShowScrollBar(get_wnd(), SB_VERT, b_need_vscrollbar);

	GetClientRect(get_wnd(), &rc);
	update_display_info();

	/*GetClientRect(get_wnd(), &rc);
	if (!EqualRect(&rc_old, &rc))
	{
		HDC dc = GetDC(get_wnd());
		HFONT fnt_old = SelectFont(dc, m_font_change_info.m_default_font->m_font.get());
		get_multiline_text_dimensions_const(dc, m_current_text, m_line_info, m_font_change_info, uGetTextHeight(dc)+2, sz, m_word_wrapping, RECT_CX(rc)>4 ? RECT_CX(rc)-4:0);
		SelectFont(dc, fnt_old);
		ReleaseDC(get_wnd(), dc);

		rc_old = rc;
	}*/
	int hMax = 1;
	hMax = (m_hscroll && m_display_sz.cx) ? (m_display_sz.cx + 4 - 1) : 0;
	hMax = max (hMax, 1);

	if (b_set_pos)
		si.fMask |= SIF_POS;
	si.nPage = RECT_CX(rc);
	si.nPage = max (si.nPage, 1);
	si.nMin = 0;
	si.nMax = hMax;

	if (b_set_pos)
	{
		if (m_horizontal_alignment == ui_helpers::ALIGN_RIGHT)
			si.nPos = si.nMax - (si.nPage ? si.nPage - 1 : 0);
		else if (m_horizontal_alignment == ui_helpers::ALIGN_CENTRE)
			si.nPos = (si.nMax - (si.nPage ? si.nPage - 1 : 0))/2;
		else
			si.fMask &= ~SIF_POS;
	}
	
	
	SetScrollInfo(get_wnd(), SB_HORZ, &si, TRUE);

#if 1

	GetClientRect(get_wnd(), &rc);
	update_display_info();

	hMax = (m_hscroll && m_display_sz.cx) ? (m_display_sz.cx + 4 - 1) : 0;
	hMax = max (hMax, 1);

	/*GetClientRect(get_wnd(), &rc);
	if (!EqualRect(&rc_old, &rc))
	{
		HDC dc = GetDC(get_wnd());
		HFONT fnt_old = SelectFont(dc, m_font_change_info.m_default_font->m_font.get());
		get_multiline_text_dimensions_const(dc, m_current_text, m_line_info, m_font_change_info, uGetTextHeight(dc)+2, sz, m_word_wrapping, RECT_CX(rc)>4 ? RECT_CX(rc)-4:0);
		SelectFont(dc, fnt_old);
		ReleaseDC(get_wnd(), dc);

		rc_old = rc;

		hMax = (m_hscroll && sz.cx) ? (sz.cx + 4 - 1) : 0;
		hMax = max (hMax, 1);
	}*/


	GetScrollInfo(get_wnd(), SB_HORZ, &si2);
	//bool b_has_hscrollbar = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_VSCROLL) != 0;
	bool b_need_hscrollbar = hMax >= (int)si2.nPage;
	//if (b_need_hscrollbar != b_has_hscrollbar)
		ShowScrollBar(get_wnd(), SB_HORZ, b_need_hscrollbar);
#endif
}

void item_details_t::refresh_contents(bool b_new_track)
{
	//disable_redrawing_t noRedraw(get_wnd());
	bool b_Update = true;
	if (m_handles.get_count())
	{
		LOGFONT lf;
		static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_item_details_font_client, lf);

		pfc::string8_fast_aggressive temp, temp2;
		temp.prealloc(2048);
		temp2.prealloc(2048);
		if (m_nowplaying_active)
			static_api_ptr_t<playback_control>()->playback_format_title(&titleformat_hook_change_font(lf),temp, m_to, NULL, playback_control::display_level_all);
		else
			m_handles[0]->format_title(&titleformat_hook_change_font(lf),temp, m_to, NULL);

		if (strcmp(temp, m_current_text_raw))
		{
			m_current_text_raw = temp;

			m_font_change_data.set_size(0);
			m_font_change_info.reset(true);
			m_font_change_info_valid = false;

			g_get_text_multiline_data(temp, temp2, m_line_info);
			temp.reset();

			g_get_text_font_data(temp2, temp, m_font_change_data);
			m_current_text = temp;
		}
		else b_Update = false;
	}
	else 
	{
		m_current_text.reset();
		m_current_text_raw.reset();
		reset_font_change_info();
		m_line_info.remove_all();
	}

	if (b_Update)
	{
		reset_display_info();

		update_scrollbar_range(b_new_track);

		invalidate_all();
	}
}

void item_details_t::update_display_info(HDC dc)
{
	if (!m_display_info_valid)
	{
		RECT rc;
		GetClientRect(get_wnd(), &rc);
		t_size widthMax = rc.right>4 ? rc.right-4 : 0;
		m_current_display_text = m_current_text;
		get_multiline_text_dimensions(dc, m_current_display_text, m_line_info, m_display_line_info, m_font_change_info, m_font_change_info.m_default_font->m_height, m_display_sz, m_word_wrapping, widthMax);
		m_display_info_valid = true;
	}
}

void item_details_t::update_display_info()
{
	if (!m_display_info_valid)
	{
		HDC dc = GetDC(get_wnd());
		HFONT fnt_old = SelectFont(dc, m_font_change_info.m_default_font->m_font.get());
		update_display_info(dc);
		SelectFont(dc, fnt_old);
		ReleaseDC(get_wnd(), dc);
	}
}
void item_details_t::reset_display_info()
{
	m_current_display_text.force_reset();
	m_display_line_info.remove_all();
	m_display_sz.cy = (m_display_sz.cx = 0);
	m_display_info_valid = false;
}

void item_details_t::update_font_change_info()
{
	if (!m_font_change_info_valid)
	{
		g_get_text_font_info(m_font_change_data, m_font_change_info);
		m_font_change_data.set_size(0);
		m_font_change_info_valid = true;
	}
}

void item_details_t::reset_font_change_info()
{
	m_font_change_data.remove_all();
	m_font_change_info.reset();
	m_font_change_info_valid = false;
}

void item_details_t::on_playback_new_track(metadb_handle_ptr p_track)
{
	if ( g_track_mode_includes_now_playing(m_tracking_mode))
	{
		m_handles.remove_all();
		m_handles.add_item(p_track);
		m_nowplaying_active = true;
		refresh_contents();
	}
}

void item_details_t::on_playback_seek(double p_time) 
{
	if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_pause(bool p_state)
{
	if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_edited(metadb_handle_ptr p_track)
{
	if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_dynamic_info(const file_info & p_info)
{
	if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_dynamic_info_track(const file_info & p_info)
{
	if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_time(double p_time)
{
	if (m_nowplaying_active) refresh_contents(false);
}


void item_details_t::on_playback_stop(play_control::t_stop_reason p_reason)
{
		if ( g_track_mode_includes_now_playing(m_tracking_mode) && p_reason != play_control::stop_reason_starting_another && p_reason != play_control::stop_reason_shutting_down)
		{
			m_nowplaying_active = false;

			metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
			if (m_tracking_mode == track_auto_playlist_playing)
			{
				static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
			}
			else if (m_tracking_mode == track_auto_selection_playing)
			{
				handles = m_selection_handles;
			}
			m_handles = handles;
			refresh_contents();
		}
	
}

void item_details_t::on_playlist_switch()
{
	if ( g_track_mode_includes_plalist(m_tracking_mode) && (!g_track_mode_includes_auto(m_tracking_mode) || !static_api_ptr_t<play_control>()->is_playing()))
	{
		metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
		static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
		m_handles = handles;
		refresh_contents();
	}
}
void item_details_t::on_items_selection_change(const bit_array & p_affected,const bit_array & p_state)
{
	if ( g_track_mode_includes_plalist(m_tracking_mode) && (!g_track_mode_includes_auto(m_tracking_mode) || !static_api_ptr_t<play_control>()->is_playing()))
	{
		metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
		static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
		m_handles = handles;
		refresh_contents();
	}
}

void item_details_t::on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook)
{
	if (!p_fromhook && !m_nowplaying_active)
	{
		bool b_refresh = false;
		t_size i, count = m_handles.get_count();
		for (i=0; i<count && !b_refresh; i++)
		{
			t_size index = pfc_infinite;
			if (p_items_sorted.bsearch_t(pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, m_handles[i], index))
				b_refresh = true;
		}
		if (b_refresh)
		{
			refresh_contents(false);
		}
	}
}

bool item_details_t::check_process_on_selection_changed()
{
	HWND wnd_focus = GetFocus();
	if (wnd_focus == NULL)
		return false ;

	DWORD processid = NULL;
	GetWindowThreadProcessId (wnd_focus, &processid);
	if (processid != GetCurrentProcessId())
		return false;

	return true;
}

void item_details_t::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr> & p_selection)
{

	if (check_process_on_selection_changed())
	{
			if (g_ui_selection_manager_is_now_playing_fallback())
				m_selection_handles.remove_all();
			else
				m_selection_handles = p_selection;

			if ( g_track_mode_includes_selection(m_tracking_mode) && (!g_track_mode_includes_auto(m_tracking_mode) || !static_api_ptr_t<play_control>()->is_playing()))
			{
				m_handles = m_selection_handles;
				refresh_contents();
			}

	}

	//pfc::hires_timer timer;
	//timer.start();


	//console::formatter() << "Selection properties panel refreshed in: " << timer.query() << " seconds";
}

void item_details_t::on_tracking_mode_change()
{
	m_handles.remove_all();

	m_nowplaying_active = false;

	if (g_track_mode_includes_now_playing(m_tracking_mode) && static_api_ptr_t<play_control>()->is_playing())
	{
		metadb_handle_ptr item;
		if (static_api_ptr_t
			<playback_control>()->get_now_playing(item))
			m_handles.add_item(item);
		m_nowplaying_active = true;
	}
	else if ( g_track_mode_includes_plalist(m_tracking_mode) )
	{
		static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(m_handles);
	}
	else if (g_track_mode_includes_selection(m_tracking_mode))
	{
		m_handles = m_selection_handles;
	}

	refresh_contents();
}

void item_details_t::update_now()
{
	RedrawWindow(get_wnd(), NULL, NULL, RDW_UPDATENOW);
}

void item_details_t::invalidate_all(bool b_update)
{
	RedrawWindow(get_wnd(), NULL, NULL, RDW_INVALIDATE|(b_update?RDW_UPDATENOW:NULL));
}

t_size text_get_multiline_lines(const char * text)
{
	t_size count = 0;
	const char * ptr = text;
	while (*ptr)
	{
		const char * start = ptr;
		while (*ptr && *ptr != '\r' && *ptr != '\n') ptr++;

		count++;

		if (*ptr == '\r') ptr++;
		if (*ptr == '\n') ptr++;
	}
	return count;
}

bool g_text_ptr_skip_font_change (const char * & ptr)
{
	//while (*ptr && *ptr != '\x7') ptr++;
	if (*ptr == '\x7')
	{
		ptr++;

		while (*ptr && *ptr != '\x7') ptr++;

		if (*ptr == '\x7')
			ptr++;

		return true;
	}
	return false;
}

void g_get_text_multiline_data(const char * text, pfc::string8_fast_aggressive & p_out, pfc::list_t<line_info_t, pfc::alloc_fast_aggressive> & indices)
{
	indices.remove_all();
	p_out.prealloc(strlen(text));
	indices.prealloc(256);

	const char * ptr = text;
	while (*ptr)
	{
		const char * start = ptr;
		t_size counter = 0;
		while (*ptr && *ptr != '\r' && *ptr != '\n') 
		{
			if (!g_text_ptr_skip_font_change(ptr))
			{
				ptr++;
				counter++;
			}
		}

		line_info_t temp;
		temp.m_raw_bytes = counter;
		//if (*ptr)
			indices.add_item(temp/*p_out.get_length()*/);

		p_out.add_string(start, ptr-start);

		if (*ptr == '\r') ptr++;
		if (*ptr == '\n') ptr++;
	}
}

t_size get_text_ptr_length_colour (const char * ptr, t_size ptr_length)
{
	t_size i = 0, charCount=0;

	while (ptr[i] && i<ptr_length)
	{
		while (ptr[i] && i<ptr_length && ptr[i] != '\x3')
		{
			charCount++;
			i++;
		}
		while (i<ptr_length && ptr[i] == '\x3')
		{
			i++;

			while (ptr[i] && i<ptr_length && ptr[i] != '\x3') i++;

			if (i<ptr_length && ptr[i] == '\x3') i++;
		}
	}
	return charCount;
}

t_size get_text_ptr_characters_colour (const char * ptr, t_size ptr_length)
{
	t_size i = 0, charCount=0;

	while (ptr[i] && i<ptr_length)
	{
		while (ptr[i] && i<ptr_length && ptr[i] != '\x3')
		{
			charCount++;
			unsigned cLen = uCharLength(&ptr[i]);
			if (cLen == 0)
				return charCount;
			i += cLen;
		}
		while (i<ptr_length && ptr[i] == '\x3')
		{
			i++;

			while (ptr[i] && i<ptr_length && ptr[i] != '\x3') i++;

			if (i<ptr_length && ptr[i] == '\x3') i++;
		}
	}
	return charCount;
}

t_size increase_text_ptr_colour(const char * ptr, t_size ptr_length, t_size count)
{
	t_size i = 0, charCount=0;

	while (ptr[i] && i<ptr_length && charCount<count)
	{
		while (ptr[i] && i<ptr_length && charCount<count && ptr[i] != '\x3')
		{
			charCount++;
			i++;
		}
		while (i<ptr_length && ptr[i] == '\x3')
		{
			i++;

			while (ptr[i] && i<ptr_length && ptr[i] != '\x3') i++;

			if (i<ptr_length && ptr[i] == '\x3') i++;
		}
	}
	return i;
}

t_size utf8_char_prev_len (const char * str, t_size ptr)
{
	t_size ret = 0;

	while (ptr && (str[ptr-1]&0xC0) == 0x80) {ptr--; ret++;}

	if (ptr) ret++;

	return ret;
}

bool text_ptr_find_break(const char * ptr, t_size length, t_size desiredPositionChar, t_size & positionChar, t_size & positionByte)
{
	t_size i = 0, charPos = 0, prevSpaceByte = pfc_infinite, prevSpaceChar = pfc_infinite;

	while (i<length/* && ptr[i] != ' '*/)
	{
		if (i< length && ptr[i] == ' ')
		{
			if (charPos >= desiredPositionChar)
			{
				positionChar = charPos;
				positionByte = i;
				return true;
			}
			prevSpaceChar = charPos;
			prevSpaceByte = i;
		}
		if (charPos >= desiredPositionChar)
		{
			if (prevSpaceByte != pfc_infinite)
			{
				positionChar = prevSpaceChar;
				positionByte = prevSpaceByte;
				return true;
			}
		}
		if (i<length && ptr[i] != '\x3')
		{
			t_size cLen = uCharLength(&ptr[i]);
			if (!cLen) break;
			i+=cLen;
			charPos++;
		}
		else
		{
			while (i<length && ptr[i] == '\x3')
			{
				i++;

				while (ptr[i] && i<length && ptr[i] != '\x3') i++;

				if (i<length && ptr[i] == '\x3') i++;
			}
		}
	}


	positionChar = charPos;
	positionByte = i;
	return false;
}

#define ELLIPSIS "\xe2\x80\xa6"//"\x85"
#define ELLIPSIS_LEN 3

void get_multiline_text_dimensions(HDC dc, pfc::string8_fast_aggressive & text_new, const line_info_list_t & rawLines, display_line_info_list_t & displayLines, const font_change_info_t & p_font_data, t_size line_height, SIZE & sz, bool b_word_wrapping, t_size max_width)
{
	displayLines.remove_all();
	displayLines.prealloc(rawLines.get_count()*2);
	sz.cx = 0;
	sz.cy = 0;
	t_size i, count = rawLines.get_count();
	displayLines.set_count(count);
	for (i=0; i<count; i++)
	{
		displayLines[i].m_raw_bytes = rawLines[i].m_raw_bytes;
	}
	t_size ptr = 0;
	//if (b_word_wrapping && max_width)
	{
		t_size heightDefault = uGetTextHeight(dc);
		t_size heightCurrent = heightDefault;

		t_size fontChangesCount = p_font_data.m_font_changes.get_count();
		t_size fontPtr = 0;
		bool b_fontChanged = false;
		HFONT fnt_old = NULL;

		pfc::string8 text = text_new;
		text_new.reset();

		ui_helpers::uGetTextExtentExPoint_helper pGetTextExtentExPoint;
		pfc::array_t<INT, pfc::alloc_fast_aggressive> widths;

		for (i=0; i<count; i++)
		{
			t_size thisFontChangeCount = 0;
			t_size ptrRemaining = displayLines[i].m_raw_bytes;
			t_size thisLineHeight = heightCurrent;

			{
				while ((fontPtr + thisFontChangeCount < fontChangesCount && (ptr+ptrRemaining) > p_font_data.m_font_changes[fontPtr + thisFontChangeCount].m_text_index) )
				{
					thisLineHeight = max (thisLineHeight, (heightCurrent = p_font_data.m_font_changes[fontPtr+thisFontChangeCount].m_font->m_height));
					thisFontChangeCount++;
				}
			}

			t_size widthCuml = 0, ptrStart = ptr;

			bool bWrapped = false;
			t_size textWrappedPtr = 0, ptrLengthNoColours = 0, ptrLength = 0; // no colour codes
			t_size ptrTextWidth = 0, ptrCharacterExtent = 0;

			t_size lineTotalChars = get_text_ptr_characters_colour(&text[ptr], ptrRemaining);
			pfc::array_t<INT, pfc::alloc_fast_aggressive> character_extents;
			character_extents.prealloc(lineTotalChars);

			while (true && thisFontChangeCount)
			{
				unsigned width = NULL;
				t_size ptrThisCount = ptrRemaining;
				if (ptr < p_font_data.m_font_changes[fontPtr].m_text_index)
					ptrThisCount = p_font_data.m_font_changes[fontPtr].m_text_index - (ptr);
				else if (thisFontChangeCount>1) 
					ptrThisCount = p_font_data.m_font_changes[fontPtr+1].m_text_index - (ptr);

				if ((ptr) >= p_font_data.m_font_changes[fontPtr].m_text_index)
				{
					HFONT fnt = SelectFont(dc, p_font_data.m_font_changes[fontPtr].m_font->m_font.get());
					if (!b_fontChanged)
					{
						fnt_old = fnt;
						b_fontChanged = true;
					}
					fontPtr++;
					thisFontChangeCount--;
				}

				SIZE sz2;
				t_size length_chars_no_colours = get_text_ptr_characters_colour(&text[ptr], ptrThisCount);
				INT max_chars = length_chars_no_colours;

				character_extents.increase_size(length_chars_no_colours);
				//widths.set_size(length_chars_no_colours);
				pGetTextExtentExPoint.run(dc, &text[ptr], ptrThisCount, ptrTextWidth > max_width ? 0 : max_width - ptrTextWidth, b_word_wrapping ? &max_chars : NULL, character_extents.get_ptr()+ptrCharacterExtent, &sz2, NULL, false);

				if ((unsigned)max_chars < length_chars_no_colours)
				{
					textWrappedPtr = max_chars;
					bWrapped = true;
					ptrLengthNoColours = length_chars_no_colours;
					ptrLength = ptrThisCount;
					ptrCharacterExtent += length_chars_no_colours;
					break;
				}

				ptr += ptrThisCount;
				ptrRemaining -= ptrThisCount;

				t_size ptrTextWidthPrev = ptrTextWidth;

				if (max_chars)
				{
					ptrTextWidth += character_extents[ptrCharacterExtent+max_chars-1];
					widthCuml += character_extents[ptrCharacterExtent+max_chars-1];
				}

				for (t_size k = 0; k < max_chars; k++)
					character_extents[ptrCharacterExtent+k] += ptrTextWidthPrev;

				ptrCharacterExtent += length_chars_no_colours;
			}

			if (!bWrapped && ptrRemaining)
			{
				t_size ptrThisCount = ptrRemaining;
				SIZE sz2;
				t_size length_chars_no_colours = get_text_ptr_characters_colour(&text[ptr], ptrThisCount);
				INT max_chars = length_chars_no_colours;

				character_extents.increase_size(length_chars_no_colours);
				pGetTextExtentExPoint.run(dc, &text[ptr], ptrThisCount, ptrTextWidth > max_width ? 0 : max_width - ptrTextWidth, b_word_wrapping ? &max_chars : NULL, character_extents.get_ptr()+ptrCharacterExtent, &sz2, NULL, false);

#if 0
				{
					pfc::stringcvt::string_wide_from_utf8 wstr(&text[ptr], ptrThisCount);
					DRAWTEXTPARAMS dtp;
					memset(&dtp, 0, sizeof(dtp));
					dtp.cbSize = sizeof(dtp);
					RECT rc = {0};
					rc.right = 100;
					rc.bottom = 1;
					DrawTextEx(dc, const_cast<wchar_t*>(wstr.get_ptr()), wstr.length(), &rc, DT_TOP|DT_NOPREFIX|DT_WORDBREAK|DT_EDITCONTROL|DT_SINGLELINE, &dtp);

					HRESULT hr = DrawThemeText (NULL, dc, NULL, NULL, wstr.get_ptr(), wstr.length(), NULL, NULL, &rc);

				}
#endif

				t_size ptrTextWidthPrev = ptrTextWidth;
				if ((unsigned)max_chars < length_chars_no_colours)
				{
					textWrappedPtr = max_chars;
					bWrapped = true;
					ptrLengthNoColours = length_chars_no_colours;
					ptrLength = ptrThisCount;
					//break;
				}
				else
				{
					ptr += ptrThisCount; //FIXME
					ptrRemaining -= ptrThisCount;
					if (max_chars)
					{
						ptrTextWidth += character_extents[ptrCharacterExtent+max_chars-1]; //well it's too late now but meh
						widthCuml += character_extents[ptrCharacterExtent+max_chars-1];
					}
				}
				for (t_size k = 0; k < max_chars; k++)
					character_extents[ptrCharacterExtent+k] += ptrTextWidthPrev;
				ptrCharacterExtent += length_chars_no_colours;
			}

			bool b_skipped = false;
			t_size wrapChar = 0, wrapByte = 0;
			if (bWrapped)
			{
				if (text_ptr_find_break(&text[ptrStart], ptrLength + (ptr - ptrStart), (textWrappedPtr ? textWrappedPtr-1 : 0) + get_text_ptr_characters_colour(&text[ptrStart], ptr - ptrStart), wrapChar, wrapByte))
				{
					//textWrappedPtr = (INT)wrapChar; /??
					//wrapByte++;
					text_new.add_string(text + ptrStart, wrapByte + 1 );
					//displayLines[i].m_raw_bytes--;
					b_skipped = true;
				}
				else
				{
					if (wrapByte == 0) //invalid utf-8
						break;
#if 0
					if (wrapByte >= ptrLength + (ptr - ptrStart) 
						&& text[ptrLength + (ptr - ptrStart)]
						&& text[ptrLength + (ptr - ptrStart)] != ' '
					)
					{
						if (textWrappedPtr)
							widthCuml = character_extents[textWrappedPtr-1]; //doesn't include clipped part... meh!
						bWrapped = false;
					}
#endif
					text_new.add_string(text + ptrStart, wrapByte);
				}
			}

			if (bWrapped)
			{
				while (fontPtr 
					&& p_font_data.m_font_changes[fontPtr-1].m_text_index >= ptrStart
					&& p_font_data.m_font_changes[fontPtr-1].m_text_index - ptrStart > wrapByte)
					fontPtr--;

				if (fontPtr)
				{
					SelectFont(dc, p_font_data.m_font_changes[fontPtr-1].m_font->m_font.get());
				}

				//if (wrapByte > (ptr - ptrStart))
				//	widthCuml += ui_helpers::get_text_width_color(dc, &text[ptr], wrapByte - (ptr - ptrStart));// widths[wrapChar-1];

				t_size widthChar = min(textWrappedPtr ? textWrappedPtr-1 :0, wrapChar);
				if (widthChar < character_extents.get_size())
					widthCuml = character_extents[widthChar];

				//max(sz.cx, ui_helpers::get_text_width_color(dc, text + ptr, wrapByte));

				//if (b_skipped) 
				//	wrapByte++;
				t_size inc = wrapByte ;//increase_text_ptr_colour (&text[ptr], indices[i], wrapChar);
				ptr = ptrStart + wrapByte;
				displayLines[i].m_raw_bytes -= inc;
				if (b_skipped) 
				{
					ptr++;
					displayLines[i].m_raw_bytes--;
					//inc--;
				}

				display_line_info_t temp;
				temp.m_raw_bytes = inc;
				temp.m_bytes = temp.m_raw_bytes;
				if (b_skipped)
				{
					temp.m_raw_bytes++;
					//temp.m_bytes--;
				}
				temp.m_width = widthCuml;
				temp.m_height = thisLineHeight + 2;
				displayLines.insert_item(temp,i);
				//text.remove_chars()
				count++;
			}
			else
			{
				displayLines[i].m_width = widthCuml;
				displayLines[i].m_height = thisLineHeight + 2;
				displayLines[i].m_bytes = displayLines[i].m_raw_bytes;
				text_new.add_string(text + ptrStart, displayLines[i].m_raw_bytes);
				ptr = ptrStart + displayLines[i].m_raw_bytes;
			}

			sz.cx = max(sz.cx, widthCuml);;
			sz.cy += thisLineHeight + 2;
		
		}
		if (b_fontChanged)
			SelectFont(dc, fnt_old);
	}
	/*else
	{
		for (i=0; i<count; i++)
		{
			sz.cx = max(sz.cx, ui_helpers::get_text_width_color(dc, &text_new[ptr], displayLines[i].m_byte_count));
			ptr += displayLines[i].m_byte_count;
		}
	}*/
}

#if 0
void get_multiline_text_dimensions(HDC dc, pfc::string8_fast_aggressive & text_new, const line_info_list_t & rawLines, display_line_info_list_t & displayLines, t_size line_height, SIZE & sz, bool b_word_wrapping, t_size max_width)
{
	displayLines.remove_all();
	displayLines.prealloc(rawLines.get_count()*2);
	sz.cx = 0;
	t_size i, count = rawLines.get_count();
	displayLines.set_count(count);
	for (i=0; i<count; i++)
	{
		displayLines[i].m_byte_count = rawLines[i].m_byte_count;
	}
	t_size ptr = 0;
	if (b_word_wrapping && max_width)
	{
		pfc::string8 text = text_new;
		text_new.reset();
		ui_helpers::uGetTextExtentExPoint_helper pGetTextExtentExPoint;
		pfc::array_t<INT, pfc::alloc_fast_aggressive> widths;
		for (i=0; i<count; i++)
		{
			SIZE sz2;
			INT max_chars = NULL;
			t_size length_colours = displayLines[i].m_byte_count;
			t_size length_chars_no_colours = get_text_ptr_characters_colour(&text[ptr], displayLines[i].m_byte_count);

			widths.set_size(length_chars_no_colours);
			//widths.fill_null();

			pGetTextExtentExPoint.run(dc, &text[ptr], length_colours, max_width, &max_chars, widths.get_ptr(), &sz2, NULL, false);

			 //FIXME! Chars not bytes
			if ((unsigned)max_chars < length_chars_no_colours)
			{
				bool b_skipped = false;
				t_size wrapChar = 0, wrapByte = 0;
				if (/*max_chars && */text_ptr_find_break(&text[ptr], length_colours, max_chars ? max_chars-1 : 0, wrapChar, wrapByte))
				{
					max_chars = (INT)wrapChar+1;
					text_new.add_string(text + ptr, wrapByte);
					//text.remove_chars(ptr + max_chars2, 1);
					displayLines[i].m_byte_count--;
					b_skipped = true;
				}
				else
					text_new.add_string(text + ptr, wrapByte);

				sz.cx = max(sz.cx, ui_helpers::get_text_width_color(dc, text + ptr, wrapByte));

				t_size inc = wrapByte;//increase_text_ptr_colour (&text[ptr], indices[i], wrapChar);
				ptr += inc;
				if (b_skipped) ptr++;
				displayLines[i].m_byte_count -= inc;
				display_line_info_t temp;
				temp.m_byte_count = inc;
				displayLines.insert_item(temp,i);
				//text.remove_chars()
				count++;
			}
			else
			{
				text_new.add_string(text + ptr, displayLines[i].m_byte_count);
				sz.cx = max(sz.cx, sz2.cx);
				ptr += displayLines[i].m_byte_count;
			}
		
		}
	}
	else
	{
		for (i=0; i<count; i++)
		{
			sz.cx = max(sz.cx, ui_helpers::get_text_width_color(dc, &text_new[ptr], displayLines[i].m_byte_count));
			ptr += displayLines[i].m_byte_count;
		}
	}
	sz.cy = line_height * displayLines.get_count();
}
#endif

#if 0
void get_multiline_text_dimensions_const(HDC dc, const char * text, const line_info_list_t & newLineData, t_size line_height, SIZE & sz, bool b_word_wrapping, t_size width)
{
	pfc::string8_fast_aggressive rawText = text;
	display_line_info_list_t newLineDataWrapped;
	get_multiline_text_dimensions(dc, rawText, newLineData, newLineDataWrapped, line_height, sz, b_word_wrapping, width);
}
#endif

void get_multiline_text_dimensions_const(HDC dc, const char * text, const line_info_list_t & newLineData, const font_change_info_t & p_font_data, t_size line_height, SIZE & sz, bool b_word_wrapping, t_size width)
{
	pfc::string8_fast_aggressive rawText = text;
	display_line_info_list_t newLineDataWrapped;
	get_multiline_text_dimensions(dc, rawText, newLineData, newLineDataWrapped, p_font_data, line_height, sz, b_word_wrapping, width);
}

void text_out_multiline_font(HDC dc, const RECT & rc_topleft, t_size line_height, const char * text, const font_change_info_t & p_font_data, const display_line_info_list_t & newLineDataWrapped, const SIZE & sz, COLORREF cr_text, ui_helpers::alignment align, bool b_hscroll, bool word_wrapping)
{
	pfc::string8_fast_aggressive rawText = text;

	RECT rc = rc_topleft;

	t_size widthMax = rc.right>4 ? rc.right-4 : 0;

	int newRight = rc.left + sz.cx + 4;
	if (b_hscroll && newRight > rc.right)
		rc.right = newRight;
	rc.bottom = rc.top + sz.cy;

	COLORREF cr_old = GetTextColor(dc);

	SetTextColor(dc, cr_text);

	t_size fontChangesCount = p_font_data.m_font_changes.get_count();
	t_size fontPtr = 0;

	t_size i, count = newLineDataWrapped.get_count(),start=0;//(rc.top<0?(0-rc.top)/line_height : 0);

	RECT rc_line = rc;
	const t_size ySkip = rc.top < 0 ? 0-rc.top : 0; //Hackish - meh

	{
		t_size yCuml = 0;
		for (i= 0; i<count; i++)
		{
			yCuml += newLineDataWrapped[i].m_height;
			if (yCuml > ySkip)
				break;
			start = i;
			if (i)
				rc_line.top += newLineDataWrapped[i-1].m_height;
		}
	}

	const char * ptr = rawText;
	for (i= 0; i<start/*+1*/; i++)
	{
		if (i < count)
		{
			ptr += newLineDataWrapped[i].m_raw_bytes;
			while (fontPtr < fontChangesCount && (ptr-rawText) > p_font_data.m_font_changes[fontPtr].m_text_index) fontPtr++;
		}
	}
	bool b_fontChanged = false;
	HFONT fnt_old = NULL;


	if (fontPtr)
	{
		HFONT fnt = SelectFont(dc, p_font_data.m_font_changes[fontPtr-1].m_font->m_font.get());
		if (!b_fontChanged)
		{
			fnt_old = fnt;
			b_fontChanged = true;
		}
	}

	if (start)
	{
		if (*ptr != '\x3')
		{
			const char * ptrC = ptr;
			while (--ptrC > rawText)
			{
				if (*ptrC == '\x3')
				{
					const char * ptrCEnd = ptrC;
					do {ptrC--;}
					while (ptrC >= rawText && *ptrC != '\x3');
					if (ptrC >= rawText &&  *ptrC == '\x3')
					{
						ui_helpers::text_out_colours_tab(dc, ptrC, ptrCEnd-ptrC+1, 0, 0, &rc_line, false, cr_text, false, false, false && !b_hscroll, ui_helpers::ALIGN_LEFT, NULL, false, false);
					}
					break;
				}
			}
		}
	}

	for (i= start; i<count/*+1*/; i++)
	{
		const char * ptrStart = ptr;

		if (rc_line.top > rc_topleft.bottom) break;

		t_size thisFontChangeCount = 0;
		t_size ptrRemaining = newLineDataWrapped[i].m_bytes;
		t_size thisLineHeight = newLineDataWrapped[i].m_height;

		{
			while ((fontPtr + thisFontChangeCount < fontChangesCount && (ptr-rawText.get_ptr()+ptrRemaining) > (p_font_data.m_font_changes[fontPtr + thisFontChangeCount].m_text_index)) )
			{
				thisFontChangeCount++;
			}
		}

		rc_line.bottom = rc_line.top + thisLineHeight;
		rc_line.left = rc.left + 2;
		rc_line.right = rc.right - 2;

		t_size widthLine = RECT_CX(rc_line), widthLineText = newLineDataWrapped[i].m_width;

		if (widthLineText < widthLine)
		{
			if (align == ui_helpers::ALIGN_CENTRE)
				rc_line.left += (widthLine - widthLineText)/2;
			else if (align == ui_helpers::ALIGN_RIGHT)
				rc_line.left += (widthLine - widthLineText);
		}

		while (thisFontChangeCount)
		{
				unsigned width = NULL;
				t_size ptrThisCount = ptrRemaining;
				if (ptr-rawText < p_font_data.m_font_changes[fontPtr].m_text_index)
					ptrThisCount = p_font_data.m_font_changes[fontPtr].m_text_index - (ptr-rawText);
				else if (thisFontChangeCount>1) 
					ptrThisCount = p_font_data.m_font_changes[fontPtr+1].m_text_index - (ptr-rawText);

				//rc_line.top = rc_line.bottom - 4 - p_font_data.m_fonts[p_font_data.m_font_changes[fontPtr].m_font_index]->m_height;

				if ((ptr-rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index)
				{
					HFONT fnt = SelectFont(dc, p_font_data.m_font_changes[fontPtr].m_font->m_font.get());
					if (!b_fontChanged)
					{
						fnt_old = fnt;
						b_fontChanged = true;
					}
					fontPtr++;
					thisFontChangeCount--;
				}
				RECT rc_font = rc_line;
				int extra = RECT_CY(rc_font) - uGetTextHeight(dc);
				rc_font.bottom -= 2;//extra/4;
				BOOL ret = ui_helpers::text_out_colours_tab(dc, ptr, ptrThisCount, 0, 0, &rc_font, false, cr_text, false, false, false && !b_hscroll, ui_helpers::ALIGN_LEFT, NULL, false, false, &width);
				rc_line.left = width; //width == position actually!!
				ptr += ptrThisCount;
				ptrRemaining -= ptrThisCount;
		}

		if (ptrRemaining)
		{
			RECT rc_font = rc_line;
			int extra = RECT_CY(rc_font) - uGetTextHeight(dc);
			rc_font.bottom -= 2;
			ui_helpers::text_out_colours_tab(dc, ptr, ptrRemaining, 0, 0, &rc_font, false, cr_text, false, false, false && !b_hscroll, ui_helpers::ALIGN_LEFT, NULL, false, false);
		}

#if 0
		if (fontPtr < fontChangesCount && (ptr-rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index)
		{
			do 
			{
				unsigned width = NULL;
				t_size ptrThisCount = ptrRemaining;
				if (fontPtr + 1 < fontChangesCount) 
					ptrThisCount = min(p_font_data.m_font_changes[fontPtr+1].m_text_index - (ptr-rawText), ptrThisCount);
				if (fontPtr < fontChangesCount && (ptr-rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index)
				{
					HFONT fnt = SelectFont(dc, p_font_data.m_fonts[p_font_data.m_font_changes[fontPtr].m_font_index]->m_font);
					if (!b_fontChanged)
					{
						fnt_old = fnt;
						b_fontChanged = true;
					}
				}
				ui_helpers::text_out_colours_tab(dc, ptr, ptrThisCount, 0, 2, &rc_line, false, cr_text, false, false, !b_hscroll, align, &width, false);
				rc_line.left += width;
				if (fontPtr < fontChangesCount && (ptr-rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index)
					fontPtr++;
				ptr += ptrThisCount;
				ptrRemaining -= ptrThisCount;
			}
			while (ptrRemaining || (fontPtr < fontChangesCount && (ptr-rawText) >= p_font_data.m_font_changes[fontPtr].m_text_index) );

		}
		else
			ui_helpers::text_out_colours_tab(dc, ptr, newLinePositions[i], 0, 2, &rc_line, false, cr_text, false, false, !b_hscroll, align, NULL, false);
#endif

		if (i < count)
			ptr = ptrStart + newLineDataWrapped[i].m_raw_bytes;
		
		rc_line.top = rc_line.bottom;
		//rc_line.left = rc.left;
	}

	SetTextColor(dc, cr_old);
	if (b_fontChanged)
		SelectFont(dc, fnt_old);
}

#if 0
void text_out_multiline(HDC dc, const RECT & rc_topleft, t_size line_height, const char * text, COLORREF cr_text, ui_helpers::alignment align, bool b_hscroll, bool word_wrapping)
{
	pfc::string8_fast_aggressive rawText;
	pfc::list_t<t_size, pfc::alloc_fast_aggressive> newLinePositions;

	text_get_multiline_data(text, rawText, newLinePositions);

	RECT rc = rc_topleft;

	SIZE sz;
	get_multiline_text_dimensions(dc, rawText, newLinePositions, line_height, sz, word_wrapping, rc.right>4 ? rc.right-4 : 0);
	int newRight = rc.left + sz.cx + 4;
	if (b_hscroll && newRight > rc.right)
		rc.right = newRight;
	rc.bottom = rc.top + sz.cy;

	COLORREF cr_old = GetTextColor(dc);

	SetTextColor(dc, cr_text);

	t_size i, count = newLinePositions.get_count(),start=(rc.top<0?(0-rc.top)/line_height : 0);
	const char * ptr = rawText;
	for (i= 0; i<start/*+1*/; i++)
	{
		if (i < count)
			ptr += newLinePositions[i];
	}
	for (i= start; i<count/*+1*/; i++)
	{
		RECT rc_line = rc;
		rc_line.top = rc.top + line_height * i;
		rc_line.bottom = rc_line.top + line_height;

		if (rc_line.top > rc_topleft.bottom) break;

		ui_helpers::text_out_colours_tab(dc, ptr, i<count ? newLinePositions[i] : strlen(ptr), 0, 2, &rc_line, false, cr_text, false, false, !b_hscroll, align, NULL, false);
		if (i < count)
			ptr += newLinePositions[i];
	}

	SetTextColor(dc, cr_old);
}
#endif

void item_details_t::on_size ()
{
	RECT rc;
	GetClientRect(get_wnd(), &rc);
	on_size (RECT_CX(rc), RECT_CY(rc));
}

void item_details_t::on_size (t_size cx, t_size cy)
{
	reset_display_info();

	invalidate_all(false);

	if (m_word_wrapping)
	{
		update_scrollbar_range();
	}
	else
	{
		SCROLLINFO si;
		memset(&si, 0, sizeof(si));
		si.cbSize = sizeof(si);

		si.fMask = SIF_PAGE;
		si.nPage = max (cy, 1);
		SetScrollInfo(get_wnd(), SB_VERT, &si, TRUE);

		RECT rc;
		GetClientRect(get_wnd(), &rc); //SetScrollInfo above may trigger a WM_SIZE
		si.nPage = RECT_CX(rc);
		si.nPage = max (si.nPage, 1);
		SetScrollInfo(get_wnd(), SB_HORZ, &si, TRUE);
	}
}

void item_details_t::scroll(INT SB, int position, bool b_absolute)
{
	SCROLLINFO si, si2;
	memset(&si, 0, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
	GetScrollInfo(get_wnd(), SB, &si);

	int new_pos = si.nPos;

	if (b_absolute)
		new_pos = si.nPos + position;
	else
		new_pos = position;

	if (new_pos < si.nMin)
		new_pos = si.nMin;
	if (new_pos > si.nMax)
		new_pos = si.nMax;

	if (new_pos != si.nPos)
	{
		memset(&si2, 0, sizeof(SCROLLINFO));
		si2.cbSize = sizeof(si);
		si2.fMask = SIF_POS;
		si2.nPos = new_pos;
		new_pos = SetScrollInfo(get_wnd(), SB, &si2, TRUE);

		RECT rc;
		GetClientRect(get_wnd(), &rc);
		int dy = SB == SB_VERT ? si.nPos - new_pos : 0;
		int dx = SB == SB_HORZ ? si.nPos - new_pos : 0;
		ScrollWindowEx(get_wnd(), dx, dy, &rc, &rc, 0, 0, SW_INVALIDATE);
		RedrawWindow(get_wnd(),0,0,RDW_UPDATENOW);
	}

}

LRESULT item_details_t::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		{
			register_callback();
			static_api_ptr_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_all & ~(play_callback::flag_on_volume_change|play_callback::flag_on_playback_starting), false);
			static_api_ptr_t<playlist_manager_v3>()->register_callback(this, playlist_callback_flags);
			static_api_ptr_t<metadb_io_v3>()->register_callback(this);

			m_font_change_info.m_default_font = new font_t;
			m_font_change_info.m_default_font->m_font = static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_item_details_font_client);
			m_font_change_info.m_default_font->m_height = uGetFontHeight (m_font_change_info.m_default_font->m_font);


			if (0 == g_windows.add_item(this))
				g_message_window.create(NULL);

			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lp;

			static_api_ptr_t<titleformat_compiler>()->compile_safe(m_to, m_script);

			on_size (/*lpcs->cx, lpcs->cy*/);
			on_tracking_mode_change();
			refresh_contents();

			//FIXME
			//m_library_richedit = LoadLibrary(L"Msftedit.dll");
			//m_wnd_richedit = CreateWindowEx(NULL, MSFTEDIT_CLASS, L"MooMooCoCoBananas", WS_VISIBLE| WS_CHILD | WS_BORDER, 0, 0, 200, 200, wnd, HMENU(1001), core_api::get_my_instance(), NULL);
		}
		break;
	case WM_DESTROY:
		{
			g_windows.remove_item(this);
			if (g_windows.get_count() == 0)
				g_message_window.destroy();

			m_font_change_info.m_default_font.release();

			static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
			static_api_ptr_t<metadb_io_v3>()->unregister_callback(this);
			static_api_ptr_t<playlist_manager_v3>()->unregister_callback(this);
			deregister_callback();
			m_handles.remove_all();
			m_selection_handles.remove_all();
			m_selection_holder.release();
			m_to.release();
		}
		break;
	case WM_SETFOCUS:
		deregister_callback();
		m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
		m_selection_holder->set_selection(m_handles);
		break;
	case WM_KILLFOCUS:
		m_selection_holder.release();
		register_callback();
		break;
	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
			if (!(lpwp->flags & SWP_NOSIZE) || (lpwp->flags & SWP_FRAMECHANGED))
			{
				on_size(lpwp->cx, lpwp->cy);
			}
		}
		break;
	case WM_MOUSEWHEEL:
		{
			LONG_PTR style = GetWindowLongPtr(get_wnd(),GWL_STYLE);
			bool b_horz = (!(style & WS_VSCROLL) || ((wp & MK_CONTROL))) && (style & WS_HSCROLL);

			SCROLLINFO si;
			memset(&si, 0, sizeof(SCROLLINFO));
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
			GetScrollInfo(get_wnd(), b_horz ? SB_HORZ : SB_VERT, &si);

			UINT scroll_lines = 3; //3 is default
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scroll_lines, 0); //we don't support Win9X

			int line_height = m_font_change_info.m_default_font->m_height + 2;

			if (!si.nPage) si.nPage++;

			if (scroll_lines == -1)
				scroll_lines = si.nPage-1;
			else
				scroll_lines *= line_height;

			int zDelta = short(HIWORD(wp));

			if (scroll_lines == 0)
				scroll_lines = 1;

			//console::formatter() << zDelta;

			int delta = -MulDiv(zDelta, scroll_lines, 120);

			// Limit scrolling to one page ?!?!?! It was in Columns Playlist code...
			if (delta < 0 && (UINT)(delta*-1) > si.nPage)
			{
				delta = si.nPage*-1;
				if (delta <-1) delta++;
			}
			else if (delta > 0 && (UINT)delta > si.nPage)
			{
				delta = si.nPage;
				if (delta >1) delta--;
			}

			scroll(b_horz ? SB_HORZ : SB_VERT, delta, true);

		}
		return 0;
	case WM_HSCROLL:
	case WM_VSCROLL:
		{
			UINT SB = msg == WM_VSCROLL ? SB_VERT : SB_HORZ;
			SCROLLINFO si, si2;
			memset(&si, 0, sizeof(SCROLLINFO));
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
			GetScrollInfo(get_wnd(), SB, &si);

			int new_pos = si.nPos;

			int line_height = m_font_change_info.m_default_font->m_height + 2;


			WORD p_value = LOWORD(wp);

			if (p_value == SB_LINEDOWN)
				new_pos = si.nPos + line_height;
			if (p_value == SB_LINEUP)
				new_pos = si.nPos - line_height;
			if (p_value == SB_PAGEUP) 
				new_pos = si.nPos - si.nPage;
			if (p_value == SB_PAGEDOWN) 
				new_pos = si.nPos + si.nPage;
			if (p_value == SB_THUMBTRACK) 
				new_pos = si.nTrackPos;
			if (p_value == SB_BOTTOM) 
				new_pos = si.nMax;
			if (p_value == SB_TOP) 
				new_pos = si.nMin;

			if (new_pos < si.nMin)
				new_pos = si.nMin;
			if (new_pos > si.nMax)
				new_pos = si.nMax;

			if (new_pos != si.nPos)
			{
				memset(&si2, 0, sizeof(SCROLLINFO));
				si2.cbSize = sizeof(si);
				si2.fMask = SIF_POS;
				si2.nPos = new_pos;
				new_pos = SetScrollInfo(wnd, SB, &si2, TRUE);

				RECT rc;
				GetClientRect(get_wnd(), &rc);
				int dy = SB == SB_VERT ? si.nPos - new_pos : 0;
				int dx = SB == SB_HORZ ? si.nPos - new_pos : 0;
				ScrollWindowEx(get_wnd(), dx, dy, &rc, &rc, 0, 0, SW_INVALIDATE);
				RedrawWindow(get_wnd(),0,0,RDW_UPDATENOW);
			}

		}
		return 0;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(wnd, &ps);

			SCROLLINFO siv, sih;
			memset(&siv, 0, sizeof(siv));
			siv.cbSize = sizeof(siv);
			siv.fMask = SIF_POS;
			sih = siv;
			GetScrollInfo(wnd, SB_VERT, &siv);
			GetScrollInfo(wnd, SB_HORZ, &sih);

			HDC dc_mem = CreateCompatibleDC(ps.hdc);

			RECT & rc = ps.rcPaint;
			HBITMAP bm_mem = CreateCompatibleBitmap(ps.hdc, RECT_CX(rc), RECT_CY(rc));
			HBITMAP bm_old = SelectBitmap(dc_mem, bm_mem);

			OffsetWindowOrgEx(dc_mem, +rc.left, +rc.top, NULL);

			cui::colours::helper p_helper(g_guid_item_details_colour_client);

			HFONT fnt_old = SelectFont(dc_mem, m_font_change_info.m_default_font->m_font.get());
			RECT rc_client;
			GetClientRect(wnd, &rc_client);

			const int client_height = RECT_CY(rc_client);
			
			FillRect(dc_mem, &rc, gdi_object_t<HBRUSH>::ptr_t(
				CreateSolidBrush(p_helper.get_colour(cui::colours::colour_background))
				));

			int line_height = uGetTextHeight(dc_mem)+2;

			rc_client.top -= siv.nPos;
			rc_client.left -= sih.nPos;

			update_font_change_info();
			update_display_info(dc_mem);

			if (m_display_sz.cy < client_height)
			{
				int extra = client_height - m_display_sz.cy;
				if (m_vertical_alignment == ui_helpers::ALIGN_CENTRE)
					rc_client.top += extra/2;
				else if (m_vertical_alignment == ui_helpers::ALIGN_RIGHT)
					rc_client.top += extra;
			}

			//ui_helpers::text_out_colours_tab(dc_mem, m_current_text, m_current_text.get_length(), 0, 2, &rc, NULL, p_helper.get_colour(cui::colours::colour_text), true, true, false, ui_helpers::ALIGN_LEFT);
			//text_out_multiline(dc_mem, rc_client, line_height, m_current_text, p_helper.get_colour(cui::colours::colour_text), (ui_helpers::alignment)m_alignment, m_hscroll, m_word_wrapping);
			text_out_multiline_font(dc_mem, rc_client, line_height, m_current_text, m_font_change_info, m_display_line_info, m_display_sz, p_helper.get_colour(cui::colours::colour_text), (ui_helpers::alignment)m_horizontal_alignment, m_hscroll, m_word_wrapping);
			SelectFont(dc_mem, fnt_old);

			BitBlt(ps.hdc,	rc.left, rc.top, RECT_CX(rc), RECT_CY(rc), dc_mem, rc.left, rc.top, SRCCOPY);

			SelectBitmap(dc_mem, bm_old);
			DeleteObject(bm_mem);
			DeleteDC(dc_mem);

			EndPaint(wnd, &ps);
		}
		return 0;
	};
	return DefWindowProc(wnd, msg, wp, lp);
}

class font_client_item_details : public cui::fonts::client
{
public:
	virtual const GUID & get_client_guid() const
	{
		return g_guid_item_details_font_client;
	}
	virtual void get_name (pfc::string_base & p_out) const
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
	virtual void get_name (pfc::string_base & p_out) const
	{
		p_out = "Item Details";
	}

	virtual t_size get_supported_colours() const {return cui::colours::colour_flag_background|cui::colours::colour_flag_text;}; //bit-mask
	virtual t_size get_supported_bools() const {return 0;}; //bit-mask

	virtual bool get_themes_supported() const {return false;};

	virtual void on_bool_changed(t_size mask) const {};
	virtual void on_colour_changed(t_size mask) const 
	{
		item_details_t::g_on_colours_change();
	};
};

void item_details_t::g_on_font_change()
{
	t_size i, count = g_windows.get_count();
	for (i=0; i<count; i++)
	{
		g_windows[i]->on_font_change();
	}
}

void item_details_t::g_on_colours_change()
{
	t_size i, count = g_windows.get_count();
	for (i=0; i<count; i++)
	{
		g_windows[i]->on_colours_change();
	}
}

void item_details_t::on_font_change()
{
	m_font_change_info.m_default_font->m_font = static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_item_details_font_client);
	refresh_contents(false);
	/*
	invalidate_all(false);
	update_scrollbar_range();
	update_now();
	*/
}
void item_details_t::on_colours_change()
{
	invalidate_all();
}

namespace
{
	font_client_item_details::factory<font_client_item_details> g_font_client_item_details;
	colour_client_item_details::factory<colour_client_item_details> g_colour_client_item_details;
}

uie::window_factory<item_details_t> g_item_details;