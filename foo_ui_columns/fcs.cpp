#include "stdafx.h"


static const GUID fcs_header = 
{ 0x5d6e2a5, 0x9dd5, 0x23ca, { 0xbe, 0x5a, 0x74, 0x72, 0x3c, 0x6f, 0xc, 0x74 } };
static const GUID fcs_header_v2 = 
{ 0x5d6e2a5, 0x9dd5, 0x23ca, { 0xbe, 0x5a, 0x74, 0x72, 0x3c, 0x6f, 0xc, 0x75 } };

enum {FCS_VERSION = 4};

column_list_t fcs_config_columns;

namespace fcs
{
	class writer
	{
	public:
		template <typename t_item>
		void write_item (unsigned id, const t_item & item)
		{
			pfc::assert_raw_type<t_item>();
			m_output->write_lendian_t(id, m_abort);
			m_output->write_lendian_t(sizeof(item), m_abort);
			m_output->write_lendian_t(item, m_abort);
		};
		template <typename t_int_type>
		void write_item (unsigned id, const cfg_int_t<t_int_type> & item)
		{
			m_output->write_lendian_t(id, m_abort);
			m_output->write_lendian_t(sizeof(t_int_type), m_abort);
			m_output->write_lendian_t(t_int_type(item.get_value()), m_abort);
		};
		writer(stream_writer * p_out, abort_callback & p_abort)
			: m_abort(p_abort), m_output(p_out)
		{} ;
	private:
		stream_writer * m_output;
		abort_callback & m_abort;
	};

	class reader
	{
	public:
		template <typename t_item>
		void read_item (t_item & p_out)
		{
			pfc::assert_raw_type<t_item>();
			m_input->read_lendian_t(p_out, m_abort);
		};
		template <typename t_int_type>
		void read_item (cfg_int_t<t_int_type> & p_out)
		{
			pfc::assert_raw_type<t_int_type>();
			t_int_type temp;
			m_input->read_lendian_t(temp, m_abort);
			p_out = temp;
		};
		template <typename t_item>
		void read_item_force_bool (t_item & p_out)
		{
			pfc::assert_raw_type<t_item>();
			bool temp;
			m_input->read_lendian_t(temp, m_abort);
			t_item = temp;
		};
		template <typename t_int_type>
		void read_item_force_bool (cfg_int_t<t_int_type> & p_out)
		{
			bool temp;
			m_input->read_lendian_t(temp, m_abort);
			p_out = temp;
		};
		reader(stream_reader * p_input, abort_callback & p_abort)
			: m_abort(p_abort), m_input(p_input)
		{} ;
	private:
		stream_reader * m_input;
		abort_callback & m_abort;
	};
}

void read_bool_cfg(service_ptr_t<file> & p_file, cfg_int * cfg_out)
{
	bool temp;
	unsigned read = 0;
	read=p_file->read(&temp,1,abort_callback_impl());
	*cfg_out = temp;
}

void read_bool(service_ptr_t<file> & p_file, bool &out)
{
	bool temp;
	unsigned read = 0;
	read=p_file->read(&temp,1,abort_callback_impl());
	out = temp;
}

void read_dword_cfg(service_ptr_t<file> & p_file, cfg_int * cfg_out)
{
	DWORD temp;
	unsigned read = 0;
	p_file->read_lendian_t(temp, abort_callback_impl());
	*cfg_out = temp;
}

void read_dword(service_ptr_t<file> & p_file, DWORD &out)
{
	p_file->read_lendian_t(out, abort_callback_impl());
}

void read_string(service_ptr_t<file> & p_file, pfc::string8 & out, unsigned len)
{
	if (!len) return;

	pfc::array_t<char> temp;
	temp.set_size(len+1);

	temp.fill(0);

	{
		unsigned read = 0;
		read = p_file->read(temp.get_ptr(), len, abort_callback_impl());
		if (read != len) throw exception_io_data_truncation();
		out.set_string(temp.get_ptr(),temp.get_size());
	}
}

void read_string_cfg(service_ptr_t<file> & p_file, cfg_string * cfg_out, unsigned len)
{
	pfc::string8 out;
	read_string(p_file, out, len);
	*cfg_out = out;
}

void read_font_cfg(service_ptr_t<file> & p_file, cfg_struct_t<LOGFONT> * cfg_out)
{
	DWORD dword;

	uLOGFONT font;
	abort_callback_impl ac;
	
	p_file->read_lendian_t(dword, ac);
	font.lfHeight = dword;
	
	p_file->read_lendian_t(dword, ac);
	font.lfWidth = dword;
	
	p_file->read_lendian_t(dword, ac);
	font.lfEscapement = dword;
	
	p_file->read_lendian_t(dword, ac);
	font.lfOrientation = dword;
	
	p_file->read_lendian_t(dword, ac);
	font.lfWeight = dword;

	unsigned read = 0;
	
	read = p_file->read(&font.lfItalic,8 + 32, abort_callback_impl());

	LOGFONT temp;
	
	convert_logfont_utf8_to_os(font, temp);

	*cfg_out = temp;

}

void parse_column(service_ptr_t<file> & p_file, column_t::ptr & column, unsigned identifier, unsigned version)
{
	bool size_available = false;
	DWORD size = 0;
	if (version >= 3) 
	{
		p_file->read_lendian_t(size, abort_callback_impl());
		size_available = true;
	}

//	console::info(pfc::string_printf("-> %u         %u",identifier,size));

	switch(identifier)
	{
		
	case COLUMN_NAME:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 name;
			read_string(p_file, name, len);
			column->name = name;
		}
		break;
	case COLUMN_SPEC:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 name;
			read_string(p_file, name, len);
			column->spec = name;
		}
		break;
	case COLUMN_COLOUR:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 name;
			read_string(p_file, name, len);
			column->colour_spec = name;
		}
		break;
	case COLUMN_SORT:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 name;
			 read_string(p_file, name, len);
			column->sort_spec = name;
		}
		break;
	case COLUMN_EDIT_FIELD:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 name;
			 read_string(p_file, name, len);
			column->edit_field = name;
		}
		break;
	case COLUMN_FILTER:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 name;
			 read_string(p_file, name, len);
			 column->filter = name;
		}
		break;
	case COLUMN_USE_COLOUR:
		{
			bool temp;
			 read_bool(p_file, temp);
			 column->use_custom_colour = temp;
		}
		break;
	case COLUMN_USE_SORT:
		{
			bool temp;
			 read_bool(p_file, temp);
			 column->use_custom_sort = temp;
		}
		break;
	case COLUMN_SHOW:
		{
			bool temp;
			 read_bool(p_file, temp);
			column->show = temp;
		}
		break;
	case COLUMN_WIDTH:
		{
			DWORD temp;
			 read_dword(p_file, temp);
			column->width = temp;
		}
		break;
	case COLUMN_RESIZE:
		{
			DWORD temp;
			 read_dword(p_file, temp);
			column->parts = temp;
		}
		break;
	case COLUMN_ALIGNMENT:
		{
			DWORD temp;
			 read_dword(p_file, temp);
			column->align = ((alignment)temp);
		}
		break;
	case COLUMN_FILTER_TYPE:
		{
			DWORD temp;
			 read_dword(p_file, temp);
			 column->filter_type =  ((playlist_filter_type)temp);
		}
		break;
	default:
		{
			if (version >= 3)
			{
				console::warning(pfc::string_printf("unrecognised column element id %u found in file and was ignored",identifier));
				p_file->seek_ex(size, file::seek_from_current, abort_callback_impl());
			}
		}
		break;
	}
}

void parse(service_ptr_t<file> & p_file, unsigned identifier, unsigned version)
{
	bool size_available = false;
	DWORD size = 0;
	if (version >= 3) 
	{
		p_file->read_lendian_t(size, abort_callback_impl());
		size_available = true;
	}

	abort_callback_impl p_abort;

	fcs::reader reader(p_file.get_ptr(), p_abort);

//	console::info(pfc::string_printf("%u         %u",identifier,size));


	switch(identifier)
	{
	case CONFIG_COLUMN:
		{
			column_t::ptr item = new column_t;
			unsigned identifier_column = -2;

			for(;;)
			{
				p_file->read_lendian_t(identifier_column, abort_callback_impl());
				if (identifier_column == COLUMN_END)
					break;
				parse_column(p_file, item, identifier_column, version);
			}

			fcs_config_columns.add_item(new column_t(*item.get_ptr()));
		}
		break;
	case CONFIG_USE_GLOBAL:
		{
			 read_bool_cfg(p_file, &cfg_global);
		}
		break;
	case CONFIG_USE_LEGACY_GLOBAL:
		{
			bool temp;
			p_file->read_lendian_t(temp, abort_callback_impl());
			if (version <= 3)
			{
				cfg_oldglobal = true;
				cfg_global = temp;
			}
			else
				cfg_oldglobal = temp;
		}
		break;
	case CONFIG_USE_OLD_GLOBAL:
		{
			bool dummy;
			 read_bool(p_file, dummy);
			if (dummy) console::warning("old old style global string no longer supported");
		}
		break;
	case CONFIG_INCLUDE_DATE:
		{
			 read_bool_cfg(p_file, &cfg_playlist_date);
		}
		break;
	case CONFIG_MAP_COLOUR_CODES:
		{
			bool dummy;
			 read_bool(p_file, dummy);
		}
		break;
	case CONFIG_GLOBAL:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			read_string_cfg(p_file, &cfg_globalstring, len);
		}
		break;
	case CONFIG_COLOUR:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			read_string_cfg(p_file, &cfg_colour, len);
			break;
		}
	case CONFIG_SHOW_HEADER:
		{
			read_bool_cfg(p_file, &cfg_header);
			break;
		}
	case CONFIG_SHOW_PLIST:
		{
			p_file->seek_ex(1, file::seek_from_current, abort_callback_impl());
			break;
		}
	case CONFIG_SHOW_TABS:
		{
			bool temp;
			read_bool(p_file, temp);
			break;
		}
	case CONFIG_COLOUR_BACK:
		{
			read_dword_cfg(p_file, &cfg_back);
			break;
		}
	case CONFIG_NOHSCROLL:
		{
			read_dword_cfg(p_file, &cfg_nohscroll);
			break;
		}
	case CONFIG_USE_GLOBAL_SORT:
		{
			read_dword_cfg(p_file, &cfg_global_sort);
			break;
		}
	case CONFIG_HEIGHT:
		{
			read_dword_cfg(p_file, &cfg_height);
			break;
		}
	case CONFIG_PLHEIGHT:
		{
			read_dword_cfg(p_file, &cfg_plheight);
			break;
		}
	case CONFIG_COLOUR_FRAME:
		{
			read_dword_cfg(p_file, &cfg_focus);
			break;
		}
	case CONFIG_COLOUR_INACTIVE_SELECTED_BACK:
		reader.read_item(cfg_pv_selceted_back_no_focus);
		break;
	case CONFIG_COLOUR_INACTIVE_SELECTED_TEXT:
		reader.read_item(cfg_pv_selected_text_no_focus);
		break;
	case CONFIG_COLOUR_SELECTED_BACK:
		reader.read_item(cfg_pv_selected_back);
		break;
	case CONFIG_COLOUR_SELECTED_TEXT:
		reader.read_item(cfg_pv_selected_text_colour);
		break;
	case CONFIG_COLOUR_TEXT:
		reader.read_item(cfg_pv_text_colour);
		break;
	case CONFIG_USE_CUSTOM_COLOURS:
		reader.read_item_force_bool(cfg_pv_use_custom_colours);
		break;
	case CONFIG_USE_SYSTEM_FOCUSED_ITEM_FRAME:
		reader.read_item_force_bool(cfg_pv_use_system_frame);
		break;
	case CONFIG_COLOUR_PLIST_INACTIVE_SELECTED_TEXT:
		COLORREF temp;
		reader.read_item(temp);
		playlist_switcher::colours::config_inactive_selection_text.set(temp);
		break;
	case CONFIG_COLOUR_PLIST_FORE:
		{
			read_dword_cfg(p_file, &cfg_plist_text);
			break;
		}
	case CONFIG_COLOUR_PLIST_BACK:
		{
			read_dword_cfg(p_file, &cfg_plist_bk);
			break;
		}
	case CONFIG_COLOUR_PLIST_SELECTED_TEXT:
		{
			read_dword_cfg(p_file, &cfg_plist_selected_text);
			break;
		}
	case CONFIG_COLOUR_PLIST_SELECTED_FRAME:
		{
			read_dword_cfg(p_file, &cfg_plist_selected_frame);
			break;
		}
	case CONFIG_COLOUR_PLIST_SELECTED_BACK:
		{
			read_dword_cfg(p_file, &cfg_plist_selected_back);
			break;
		}
	case CONFIG_COLOUR_PLIST_SELECTED_BACK_NO_FOCUS:
		{
			read_dword_cfg(p_file, &cfg_plist_selected_back_no_focus);
			break;
		}
	case CONFIG_COLOUR_VIS_FORE:
		{
			read_dword_cfg(p_file, &cfg_vis2);
			break;
		}
	case CONFIG_COLOUR_VIS_BACK:
		{
			read_dword_cfg(p_file, &cfg_vis);
			break;
		}
	case CONFIG_FONT_PLAYLIST:
		read_font_cfg(p_file, &cfg_font);
		break;
	case CONFIG_FONT_HEADER:
		read_font_cfg(p_file, &cfg_header_font);
		break;
	case CONFIG_FONT_STATUS:
		{
			if (cfg_import_titles)  read_font_cfg(p_file, &cfg_status_font);
			else p_file->seek_ex(sizeof(uLOGFONT), file::seek_from_current, abort_callback_impl()) ;


	//	uLOGFONT temp = cfg_status_font;
	//	if (cfg_import_titles) 
	//		bool   /**/true;
	//		console::info(pfc::string_printf("%u %u",sizeof(uLOGFONT),size));
	//	else if false;
			
			

//			if (!cfg_import_titles) cfg_status_font = temp;
			break;
		}

	case CONFIG_FONT_PLIST:
		read_font_cfg(p_file, &cfg_plist_font);
		break;
	case CONFIG_FB2K_STATUS:
		{
			
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 temp;
			 read_string(p_file, temp, len);
			if (cfg_import_titles)
			{
				main_window::config_status_bar_script.set(temp);
			}
		}
		break;
	case CONFIG_FB2K_SYSTRAY:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 temp;
			 read_string(p_file, temp, len);
			if (cfg_import_titles)
			{
				main_window::config_notification_icon_script.set(temp);
			}
		}
		break;
	case CONFIG_FB2K_WTITLE:
		{
			DWORD len;
			if (!size_available) read_dword(p_file, len);
			else len = size;

			pfc::string8 temp;
			 read_string(p_file, temp, len);
			if (cfg_import_titles)
			{
				main_window::config_main_window_title_script.set(temp);			
			}
		}
		break;
/*	case CONFIG_EXTRA:
		{
			pfc::array_t<t_uint8> data;
			data.set_mem_logic(pfc::array_t<t_uint8>::ALLOC_FAST_DONTGODOWN);
			DWORD temp;
			p_file->read_dword_le(temp);

			data.set_size(temp);

			p_file->read(data, data.get_size());
			g_extra.set_data_raw(data, data.get_size());
			}
			return true;*/
	default:
		{
			if (version >= 3)
			{
				console::warning(pfc::string_printf("unrecognised element id %u found in file and was ignotred",identifier));
				p_file->seek_ex(size, file::seek_from_current, abort_callback_impl());
			}
		}
		break;
	}
}

bool g_import(const char * path)
{
	bool rv = false;
	service_ptr_t<file> p_file;
	try
	{

		filesystem::g_open_read(p_file, path,abort_callback_impl());
		if (!p_file.is_valid()) console::error(uStringPrintf("Unable to open file: \"%s\"",path));
		else
		{
			GUID temp;
			p_file->read_lendian_t(temp, abort_callback_impl());
			if (temp != fcs_header_v2 && temp != fcs_header) throw pfc::exception("Invalid file");
			else 
			{
				DWORD ver = 1;
				if (temp == fcs_header_v2) 	p_file->read_lendian_t(ver, abort_callback_impl());
				if (ver > FCS_VERSION)
					throw pfc::exception("please update columns ui in order to use this fcs file");
				else 
				{
					fcs_config_columns.remove_all();
					unsigned identifier = -2;
					enum t_end_signal {p_end_signal};
					try 
					{
						for (;;)
						{
							p_file->read_lendian_t(identifier, abort_callback_impl());
							if (identifier == CONFIG_END)
								throw p_end_signal;
							parse(p_file, identifier, ver);
						}
					}
					catch (t_end_signal)
					{
					}
					catch (pfc::exception & e)
					{
						popup_message::g_show(e.what(), "Error", popup_message::icon_error);
					}

					if (ver <= 3)
						cfg_oldglobal = TRUE;

					/* update ui */
					g_columns.set_entries_ref(fcs_config_columns);
					//g_on_tabs_font_change();
					//on_switcher_font_change();
					//on_header_font_change();
					//on_playlist_font_change();
					//pvt::ng_playlist_view_t::g_on_font_change();
					//pvt::ng_playlist_view_t::g_on_header_font_change();
					g_import_pv_colours_to_unified_global();
					g_import_fonts_to_unified();
					refresh_all_playlist_views();	
					pvt::ng_playlist_view_t::g_on_columns_change();
					if (g_main_window)
					{
					//	if (g_pl_back_brush) {DeleteObject(g_pl_back_brush); g_pl_back_brush=0;}
						//							if (g_plist) RedrawWindow(g_plist, 0, 0, RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
						//							if (g_back_brush) {DeleteObject(g_back_brush); g_back_brush=0;}
						//							g_flush_vis();
						//on_show_sidebar_change();
					//	on_status_font_change();
						//							bool rm = create_plist();
						//							create_tabs();


						size_windows();
						//							if (rm) RedrawWindow(g_main_window, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
					}

					rv = true;
				}
			}

			p_file.release();
		}
	}
	catch (pfc::exception & e)
	{
		popup_message::g_show(e.what(), "Error", popup_message::icon_error);
	}
	return rv;
}

void import(HWND wnd)
{
	pfc::string8 path = cfg_import;
	if (uGetOpenFileName(wnd, "Columns UI Settings (*.fcs)|*.fcs|All Files (*.*)|*.*", 0, "fcs", "Import from", NULL, path, FALSE))
	{
/*		unsigned stress = 0;
		for (stress = 0; stress < 1000000; stress++)
		{*/
		if (g_import(path))
			cfg_import = path;
//		}
	}
}

void write_font(service_ptr_t<file> & p_file, const LOGFONT & font_os)
{
	logfont_utf8_from_os font(font_os);

	unsigned face_len = strlen(font.lfFaceName);
	
	if (face_len < sizeof(font.lfFaceName))
	{
		char * ptr = font.lfFaceName;
		ptr += face_len;
		memset(ptr, 0, sizeof(font.lfFaceName)-face_len);
	}

	abort_callback_impl ac;
	
	p_file->write_lendian_t(sizeof(font), ac);

	p_file->write_lendian_t(font.lfHeight, ac);
	p_file->write_lendian_t(font.lfWidth, ac);
	p_file->write_lendian_t(font.lfEscapement, ac);
	p_file->write_lendian_t(font.lfOrientation, ac);
	p_file->write_lendian_t(font.lfWeight, ac);
	unsigned written = 0;
	p_file->write(&font.lfItalic,8 + 32,ac);
}


#if 0
bool g_export(const char * path)
{
	bool rv = false;

	try
	{
	service_ptr_t<file> p_file;
	abort_callback_impl ac;
	filesystem::g_open_write_new(p_file,path,ac);
		if (p_file.is_empty()) console::error(uStringPrintf("Unable to open file: \"%s\"",path));
		else
		{

			p_file->write_lendian_t(fcs_header_v2, ac);
			p_file->write_lendian_t((t_uint32)FCS_VERSION, ac);
			
			int n,t = fcs_config_columns.get_count();
			pfc::string8 temp;
			unsigned long len;
			
			
			
			
			for (n=0;n<t;n++)
			{
				stream_writer_memblock w;


				w.write_lendian_t(COLUMN_NAME, ac);
				fcs_config_columns.get_string(n, temp, STRING_NAME);
				len = temp.length();
				w.write_lendian_t(len, ac);
				w.write(temp.get_ptr(),len, ac);
				
				w.write_lendian_t(COLUMN_SPEC, ac);
				fcs_config_columns.get_string(n, temp, STRING_DISPLAY);
				len = temp.length();
				w.write_lendian_t(len, ac);
				w.write(temp.get_ptr(),len, ac);
				
				w.write_lendian_t(COLUMN_USE_COLOUR, ac);
				w.write_lendian_t(sizeof(bool), ac);
				bool use_colour = fcs_config_columns.get_bool(n, BOOL_CUSTOM_COLOUR);
				w.write(&use_colour, sizeof(bool), ac);
				
				w.write_lendian_t(COLUMN_COLOUR, ac);
				fcs_config_columns.get_string(n, temp, STRING_COLOUR);
				len = temp.length();
				w.write_lendian_t(len, ac);
				w.write(temp.get_ptr(),len, ac);
				
				w.write_lendian_t(COLUMN_USE_SORT, ac);
				w.write_lendian_t(sizeof(bool), ac);
				bool use_sort = fcs_config_columns.get_bool(n, BOOL_CUSTOM_SORT);
				w.write(&use_sort, sizeof(bool), ac);
				
				w.write_lendian_t(COLUMN_SORT, ac);
				fcs_config_columns.get_string(n, temp, STRING_SORT);
				len = temp.length();
				w.write_lendian_t(len, ac);
				w.write(temp.get_ptr(),len, ac);
				
				w.write_lendian_t(COLUMN_EDIT_FIELD, ac);
				fcs_config_columns.get_string(n, temp, STRING_EDIT_FIELD);
				len = temp.length();
				w.write_lendian_t(len, ac);
				w.write(temp.get_ptr(),len, ac);

				w.write_lendian_t(COLUMN_WIDTH, ac);
				w.write_lendian_t(4, ac);
				unsigned long width = fcs_config_columns.get_long(n, LONG_WIDTH);
				w.write_lendian_t(width, ac);
				
				w.write_lendian_t(COLUMN_ALIGNMENT, ac);
				w.write_lendian_t(4, ac);
				alignment align = fcs_config_columns.get_alignment(n);
				w.write_lendian_t(align, ac);
				
				w.write_lendian_t(COLUMN_FILTER_TYPE, ac);
				w.write_lendian_t(4, ac);
				playlist_filter_type type = fcs_config_columns.get_playlist_filter_type(n);
				w.write_lendian_t(type, ac);
				
				w.write_lendian_t(COLUMN_FILTER, ac);
				fcs_config_columns.get_string(n, temp, STRING_FILTER);
				len = temp.length();
				w.write_lendian_t(len, ac);
				w.write(temp.get_ptr(),len, ac);
				
				w.write_lendian_t(COLUMN_RESIZE, ac);
				w.write_lendian_t(4, ac);
				unsigned long parts = fcs_config_columns.get_long(n, LONG_PARTS);
				w.write_lendian_t(parts, ac);
				
				w.write_lendian_t(COLUMN_SHOW, ac);
				w.write_lendian_t(sizeof(bool), ac);
				bool show = fcs_config_columns.get_bool(n, BOOL_SHOW);
				w.write(&show, sizeof(bool), ac);
				
				w.write_lendian_t(COLUMN_END, ac);

				p_file->write_lendian_t(CONFIG_COLUMN, ac);
				p_file->write_lendian_t(w.m_data.get_size(), ac);

				unsigned written = 0;
				p_file->write(w.m_data.get_ptr(), w.m_data.get_size(), ac);

			}
			
			p_file->write_lendian_t(CONFIG_USE_GLOBAL, ac);
			p_file->write_lendian_t(1, ac);
			bool use_global = (cfg_global != 0);
			p_file->write(&use_global, 1, ac);
			
			p_file->write_lendian_t(CONFIG_USE_LEGACY_GLOBAL, ac);
			p_file->write_lendian_t(1, ac);
			bool use_old_global = (cfg_oldglobal != 0);
			p_file->write(&use_old_global, 1, ac);

			p_file->write_lendian_t(CONFIG_INCLUDE_DATE, ac);
			p_file->write_lendian_t(1, ac);
			bool include_date = (cfg_playlist_date != 0);
			p_file->write(&include_date, 1, ac);

//			p_file->write_lendian_t(CONFIG_MAP_COLOUR_CODES, ac);
//			p_file->write_lendian_t(1, ac);
//			bool map_codes = (cfg_global_map_codes != 0);
//			p_file->write(&map_codes, 1, ac);

			p_file->write_lendian_t(CONFIG_GLOBAL, ac);
			temp = cfg_globalstring;
			len = temp.length();
			p_file->write_lendian_t(len, ac);
			p_file->write(temp.get_ptr(),len, ac);
			
			p_file->write_lendian_t(CONFIG_COLOUR, ac);
			temp = cfg_colour;
			len = temp.length();
			p_file->write_lendian_t(len, ac);
			p_file->write(temp.get_ptr(),len, ac);
			
			p_file->write_lendian_t(CONFIG_SHOW_HEADER, ac);
			p_file->write_lendian_t(1, ac);
			bool show_header = (cfg_header != 0);
			p_file->write(&show_header, 1,ac);
			
			p_file->write_lendian_t(CONFIG_SHOW_TABS, ac);
			p_file->write_lendian_t(1, ac);
			bool show_tabs = (cfg_tabs != 0);
			p_file->write(&show_tabs, 1, ac);

			p_file->write_lendian_t(CONFIG_COLOUR_BACK, ac);
			p_file->write_lendian_t(4, ac);
			DWORD colour_back = cfg_back;
			p_file->write_lendian_t(colour_back,ac);

			p_file->write_lendian_t(CONFIG_COLOUR_FRAME, ac);
			p_file->write_lendian_t(4, ac);
			DWORD colour_frame = cfg_focus;
			p_file->write_lendian_t(colour_frame, ac);

			fcs::writer writer(p_file.get_ptr(), ac);

			writer.write_item(CONFIG_USE_CUSTOM_COLOURS, bool(cfg_pv_use_custom_colours != 0));
			writer.write_item(CONFIG_USE_SYSTEM_FOCUSED_ITEM_FRAME, bool(cfg_pv_use_system_frame != 0));
			writer.write_item(CONFIG_COLOUR_TEXT, cfg_pv_text_colour);
			writer.write_item(CONFIG_COLOUR_SELECTED_TEXT, cfg_pv_selected_text_colour);
			writer.write_item(CONFIG_COLOUR_SELECTED_BACK, cfg_pv_selected_back);
			writer.write_item(CONFIG_COLOUR_INACTIVE_SELECTED_BACK, cfg_pv_selceted_back_no_focus);
			writer.write_item(CONFIG_COLOUR_INACTIVE_SELECTED_TEXT, cfg_pv_selected_text_no_focus);

			writer.write_item(CONFIG_COLOUR_PLIST_INACTIVE_SELECTED_TEXT, playlist_switcher::colours::config_inactive_selection_text.get());

			p_file->write_lendian_t(CONFIG_COLOUR_PLIST_FORE, ac);
			p_file->write_lendian_t(4, ac);
			DWORD colour_plist_fore = cfg_plist_text;
			p_file->write_lendian_t(colour_plist_fore, ac);

			p_file->write_lendian_t(CONFIG_COLOUR_PLIST_BACK, ac);
			p_file->write_lendian_t(4, ac);
			DWORD colour_plist_back = cfg_plist_bk;
			p_file->write_lendian_t(colour_plist_back, ac);

			p_file->write_lendian_t(CONFIG_COLOUR_PLIST_SELECTED_TEXT, ac);
			p_file->write_lendian_t(4, ac);
			DWORD colour_plist_sel_text = cfg_plist_selected_text;
			p_file->write_lendian_t(colour_plist_sel_text, ac);

			p_file->write_lendian_t(CONFIG_COLOUR_PLIST_SELECTED_FRAME, ac);
			p_file->write_lendian_t(4, ac);
			DWORD colour_plist_sel_frame = cfg_plist_selected_frame;
			p_file->write_lendian_t(colour_plist_sel_frame, ac);

			p_file->write_lendian_t(CONFIG_COLOUR_PLIST_SELECTED_BACK, ac);
			p_file->write_lendian_t(4, ac);
			DWORD colour_plist_sel_back = cfg_plist_selected_back;
			p_file->write_lendian_t(colour_plist_sel_back, ac);

			p_file->write_lendian_t(CONFIG_COLOUR_PLIST_SELECTED_BACK_NO_FOCUS, ac);
			p_file->write_lendian_t(4, ac);
			DWORD colour_plist_sel_back_no_focus = cfg_plist_selected_back_no_focus;
			p_file->write_lendian_t(colour_plist_sel_back_no_focus, ac);

			p_file->write_lendian_t(CONFIG_COLOUR_VIS_FORE, ac);
			p_file->write_lendian_t(4, ac);
			DWORD vis_fore = cfg_vis2;
			p_file->write_lendian_t(vis_fore, ac);

			p_file->write_lendian_t(CONFIG_COLOUR_VIS_BACK, ac);
			p_file->write_lendian_t(4, ac);
			DWORD vis_back = cfg_vis;
			p_file->write_lendian_t(vis_back, ac);

			DWORD temp_dword;

			p_file->write_lendian_t(CONFIG_NOHSCROLL, ac);
			p_file->write_lendian_t(4, ac);
			temp_dword = cfg_nohscroll;
			p_file->write_lendian_t(temp_dword, ac);

			p_file->write_lendian_t(CONFIG_USE_GLOBAL_SORT, ac);
			p_file->write_lendian_t(4, ac);
			temp_dword = cfg_global_sort;
			p_file->write_lendian_t(temp_dword, ac);

			p_file->write_lendian_t(CONFIG_HEIGHT, ac);
			p_file->write_lendian_t(4, ac);
			temp_dword = cfg_height;
			p_file->write_lendian_t(temp_dword, ac);

			p_file->write_lendian_t(CONFIG_PLHEIGHT, ac);
			p_file->write_lendian_t(4, ac);
			temp_dword = cfg_plheight;
			p_file->write_lendian_t(temp_dword, ac);

			{
				p_file->write_lendian_t(CONFIG_FONT_PLAYLIST, ac);
				LOGFONT font = cfg_font;

				write_font(p_file, font);

			}
			{
				p_file->write_lendian_t(CONFIG_FONT_HEADER, ac);
				LOGFONT font = cfg_header_font;

				write_font(p_file, font);
			}

			if (cfg_export_titles)
			{
				p_file->write_lendian_t(CONFIG_FONT_STATUS, ac);
				LOGFONT font = cfg_status_font;
				write_font(p_file, font);
			}

			{
				p_file->write_lendian_t(CONFIG_FONT_PLIST, ac);
				LOGFONT font = cfg_plist_font;
				write_font(p_file, font);
			}

			if (cfg_export_titles)
			{
				titleformat_config::g_get_data(titleformat_config::config_statusbar, temp);
				p_file->write_lendian_t(CONFIG_FB2K_STATUS, ac);
				len = temp.length();
				p_file->write_lendian_t(len, ac);
				p_file->write(temp.get_ptr(),len, ac);

				titleformat_config::g_get_data(titleformat_config::config_systray, temp);
				p_file->write_lendian_t(CONFIG_FB2K_SYSTRAY, ac);
				len = temp.length();
				p_file->write_lendian_t(len, ac);
				p_file->write(temp.get_ptr(),len, ac);

				titleformat_config::g_get_data(titleformat_config::config_windowtitle, temp);
				p_file->write_lendian_t(CONFIG_FB2K_WTITLE, ac);
				len = temp.length();
				p_file->write_lendian_t(len, ac);
				p_file->write(temp.get_ptr(),len, ac);

			}


			p_file->write_lendian_t(CONFIG_END,ac);

			rv = true;
		}
				}
				catch (const pfc::exception & p_error)
				{
					popup_message::g_show_ex(p_error.what(), pfc_infinite, "Le erreur", pfc_infinite);;
				}
return rv;
}

void export(HWND wnd)
{
				
	pfc::string8 path = cfg_export;
	if (uGetOpenFileName(wnd, "Columns UI Settings (*.fcs)|*.fcs|All Files (*.*)|*.*", 0, "fcs", "Export to", NULL, path, TRUE))
	{
		g_export(path);
			cfg_export = path;
	}
					
}
#endif