#include "stdafx.h"
#include "NG Playlist/ng_playlist.h"


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
			int32_t item_padding_temp;
			reader.read_item(item_padding_temp);
			settings::playlist_view_item_padding = item_padding_temp;
			break;
		}
	case CONFIG_PLHEIGHT:
		{
			int32_t item_padding_temp;
			reader.read_item(item_padding_temp);
			settings::playlist_switcher_item_padding = item_padding_temp;
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
