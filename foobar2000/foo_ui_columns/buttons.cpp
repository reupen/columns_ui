#include "foo_ui_columns.h"

#define ID_BUTTONS  2001

void _check_hresult (HRESULT hr) {if (FAILED(hr)) throw pfc::exception(pfc::string8() << "WIC error: " << format_win32_error(hr));}

#if 0
HBITMAP g_load_png_wic(HDC dc, const char * fn)
{
	HBITMAP bm = 0;
	try {
	//coinitialise_scope m_coinit;


	mmh::comptr_t<IWICImagingFactory> pWICImagingFactory;
	_check_hresult(pWICImagingFactory.instantiate(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER));

	abort_callback_dummy p_abort;
	file:: ptr p_file;
	filesystem::g_open_read(p_file, fn, p_abort);
	t_size fsize = pfc::downcast_guarded<t_size>(p_file->get_size_ex(p_abort));
	pfc::array_staticsize_t<t_uint8> buffer(fsize);
	p_file->read(buffer.get_ptr(), fsize, p_abort);
	p_file.release();

	IStream *pStream = NULL;
	HGLOBAL gd = GlobalAlloc(GMEM_FIXED|GMEM_MOVEABLE, buffer.get_size());
	if (gd == NULL)
		throw exception_win32(GetLastError());
	void * p_data = GlobalLock(gd);
	if (p_data == NULL)
		throw exception_win32(GetLastError());

	memcpy(p_data, buffer.get_ptr(), buffer.get_size());
	GlobalUnlock(gd);

	HRESULT hr = CreateStreamOnHGlobal (gd, TRUE, &pStream);
	if (FAILED(hr))
		throw exception_win32(hr);

	mmh::comptr_t<IWICBitmapFrameDecode> pWICBitmapFrameDecode;

	mmh::comptr_t<IWICBitmapDecoder> pWICBitmapDecoder;
	_check_hresult(pWICImagingFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, pWICBitmapDecoder));
	
	pStream->Release();

	_check_hresult(pWICBitmapDecoder->GetFrame(0, pWICBitmapFrameDecode));

#if 1
	mmh::comptr_t<IWICFormatConverter> pWICFormatConverter;
	_check_hresult(pWICImagingFactory->CreateFormatConverter(pWICFormatConverter));
	
	_check_hresult(pWICFormatConverter->Initialize(pWICBitmapFrameDecode, GUID_WICPixelFormat32bppBGRA,
		WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeCustom));
#else
	mmh::comptr_t<IWICBitmapSource> pWICFormatConverter;
	_check_hresult(WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, pWICBitmapFrameDecode, pWICFormatConverter));
#endif

	UINT cx = 0, cy = 0;
	pWICFormatConverter->GetSize(&cx, &cy);

	t_size row_bytes = cx*4, stride = ((row_bytes%4)?(4-(row_bytes%4)):0) + row_bytes;
	pfc::array_t<t_uint8> bitmapData;
	bitmapData.set_size(stride*cy);
	pWICFormatConverter->CopyPixels(NULL, stride, stride*cy, bitmapData.get_ptr());

	pfc::array_t<t_uint8> bm_data;
	bm_data.set_size(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 0);
	bm_data.fill(0);
	
	BITMAPINFOHEADER bmi;
	memset(&bmi, 0, sizeof(bmi));
	
	bmi.      biSize = sizeof(bmi);
	bmi.      biWidth = cx;
	bmi.      biHeight = cy;
	bmi.      biPlanes = 1;
	bmi.      biBitCount = 32;
	bmi.      biCompression = BI_RGB;
	bmi.biClrUsed = 0;
	bmi.biClrImportant = 0;

	BITMAPINFO & bi = *(BITMAPINFO*)bm_data.get_ptr();
	
	bi.bmiHeader = bmi;
	
	void * data = 0;
	bm = CreateDIBSection(0,&bi,DIB_RGB_COLORS,&data,0,0);
	
	if (data)
	{
		char * ptr = (char*)data;
		
		GdiFlush();

		memcpy(ptr, bitmapData.get_ptr(), bitmapData.get_size());
	}

	} catch (pfc::exception const & ex)
	{
		console::formatter() << "Failed to load image \"" << fn << "\" - " << ex.what();
	}

	return bm;
}

HBITMAP read_png(HDC dc, const char * fn/*, unsigned int sig_read*/)  /* file is already open */
{
	FILE * fp = ufsopen(fn, "rb", _SH_DENYNO);
	unsigned int sig_read = 0;
	HBITMAP bmp = 0;
	
	if (fp)
	{
		
		if (p_libpng.is_valid())
		{
			{
				png_structp png_ptr = png_structp_NULL;
				png_infop info_ptr = 0;
				try
				{
					
					png_ptr = p_libpng->png_create_read_struct(PNG_LIBPNG_VER_STRING, (void*)fn, user_error_fn, user_warning_fn);
					
					if (png_ptr == NULL)
					{
						throw "error creating png read struct";
					}
					
					/* Allocate/initialize the memory for image information.  REQUIRED. */
					info_ptr = p_libpng->png_create_info_struct(png_ptr);
					if (info_ptr == NULL)
					{
						throw "error creating png info struct";
					}
					
					/* Set up the input control if you are using standard C streams */
					p_libpng->png_set_read_fn(png_ptr, (void*)fp,&png_read_file);
					
					/* If we have already read some of the signature */
					p_libpng->png_set_sig_bytes(png_ptr, sig_read);
					
					p_libpng->png_read_png(png_ptr, info_ptr, /**/PNG_TRANSFORM_BGR|PNG_TRANSFORM_STRIP_16|PNG_TRANSFORM_PACKING|PNG_TRANSFORM_EXPAND|PNG_TRANSFORM_SHIFT, png_voidp_NULL);

					png_bytepp image_rows;
					
					image_rows = p_libpng->png_get_rows(png_ptr, info_ptr);
					
					unsigned rowbytes = p_libpng->png_get_rowbytes(png_ptr, info_ptr);

					//png_colorp palette = NULL;
					//int num_palette = 0;

					//bool b_palette = (p_libpng->png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette) & PNG_INFO_PLTE) != 0;

					png_byte color_type = p_libpng->png_get_color_type(png_ptr, info_ptr);

					unsigned bpp = p_libpng->png_get_bit_depth (png_ptr, info_ptr) * p_libpng->png_get_channels(png_ptr, info_ptr);
					//console::formatter() << num_palette << " " << bpp;

					unsigned color_table_size = 0;
					if (bpp <= 8)
					{
						color_table_size = 1; 
						unsigned j;
						for (j=0; j<bpp; j++)
							color_table_size *= 2;
					}

					if ( color_type == PNG_COLOR_TYPE_GRAY && bpp > 8)
						throw "Unexpected image type (greyscale, bpp per channel > 8; expected libpng to convert)";
					
					pfc::array_t<t_uint8> bm_data;
					bm_data.set_size(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * color_table_size);
					bm_data.fill(0);
					
					BITMAPINFOHEADER bmi;
					memset(&bmi, 0, sizeof(bmi));
					
					bmi.      biSize = sizeof(bmi);
					bmi.      biWidth = p_libpng->png_get_image_width(png_ptr, info_ptr) ;
					bmi.      biHeight = p_libpng->png_get_image_height(png_ptr, info_ptr);
					bmi.      biPlanes = 1;
					bmi.      biBitCount = bpp;
					bmi.      biCompression = BI_RGB;
					bmi.biClrUsed = 0;
					bmi.biClrImportant = 0;

					BITMAPINFO & bi = *(BITMAPINFO*)bm_data.get_ptr();
					//memset(&bi, 0, sizeof(bi));
					
					bi.bmiHeader = bmi;

					if (bpp <= 8)
					{
						if (color_type == PNG_COLOR_TYPE_GRAY)
						{
							unsigned i;
							for (i=0; i<color_table_size;i++)
							{
								bi.bmiColors[i].rgbBlue = i * color_table_size/256;
								bi.bmiColors[i].rgbGreen = i * color_table_size/256;
								bi.bmiColors[i].rgbRed = i * color_table_size/256;
							}
						}
						else throw "Unexpected images type (bpp <= 8; expected libpng to convert)";
					}
					
					void * data = 0;
					bmp = CreateDIBSection(0,&bi,DIB_RGB_COLORS,&data,0,0);
					
					if (data)
					{
						char * ptr = (char*)data;
						
						GdiFlush();

						unsigned last_dword_size = rowbytes % sizeof(DWORD);
						if (!last_dword_size) last_dword_size=sizeof(DWORD);

						unsigned rowbytesaligned = rowbytes + (sizeof(DWORD)-last_dword_size);
						
						unsigned n;
						for (n=bmi.biHeight ; n; n--,ptr += rowbytesaligned)
						{
							memcpy(ptr, image_rows[n-1], rowbytes);
							memset(ptr+rowbytes, 0, rowbytesaligned-rowbytes);
						}
					}
					
					/* clean up after the read, and free any memory allocated - REQUIRED */
					//p_libpng->png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
					
					/* close the file */
					
					/* that's it */
				}
				catch (const char * ptr)
				{
					console::printf("aborting loading image \"%s\": %s",fn,ptr);
				}
				if (png_ptr)
					p_libpng->png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : png_infopp_NULL, png_infopp_NULL);
			}
			
		}
		else
			console::print("Failed to load the libpng library. Check that libpng13.dll and zlib1.dll are present in your foobar2000 installation directory. See \"libraries\" in preferences for further details.");
		
		fclose(fp);
	}
	
	return (bmp);
}
#endif


HBITMAP LoadMonoBitmap(INT_PTR uid, COLORREF cr_btntext)
{
	HBITMAP rv = 0;
	HRSRC rs = FindResource(core_api::get_my_instance(), MAKEINTRESOURCE(uid), RT_BITMAP);
	HGLOBAL glb = LoadResource(core_api::get_my_instance(), rs);
	void * p_res = LockResource(glb);
	BITMAPINFO * p_bih = (LPBITMAPINFO)p_res;
	if (p_bih)
	{
		unsigned num_colours = p_bih->bmiHeader.biClrUsed;
		if(!num_colours && p_bih->bmiHeader.biBitCount <= 8)
			num_colours = 1<<p_bih->bmiHeader.biBitCount;

		pfc::array_t<t_uint8> bmi;
		bmi.append_fromptr((t_uint8*)p_bih, p_bih->bmiHeader.biSize + sizeof(RGBQUAD)*num_colours);

		BITMAPINFO * lpbi = (LPBITMAPINFO)bmi.get_ptr();

		if (num_colours == 2)
		{
			lpbi->bmiColors[0].rgbRed = LOBYTE(LOWORD(cr_btntext));
			lpbi->bmiColors[0].rgbGreen = HIBYTE(LOWORD(cr_btntext));
			lpbi->bmiColors[0].rgbBlue = LOBYTE(HIWORD(cr_btntext));
			lpbi->bmiColors[1].rgbRed = 0xFF;
			lpbi->bmiColors[1].rgbGreen = 0xFF;
			lpbi->bmiColors[1].rgbBlue = 0xFF;
		}

//		BITMAPINFOHEADER bmh = lpbi->bmiHeader;

		void * p_bits = &p_bih->bmiColors[num_colours];

		HDC dc = GetDC(0);
		rv = CreateDIBitmap(dc,&lpbi->bmiHeader,CBM_INIT,p_bits,lpbi,DIB_RGB_COLORS);
		ReleaseDC(0, dc);
	}
	return rv;
}

template <const GUID & MenutItemID, INT IconID>
class button_menu_item_with_bitmap : public uie::button_v2
{
	virtual const GUID & get_item_guid ()const
	{
		return MenutItemID;
	}

	virtual HANDLE get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, unsigned cx_hint, unsigned cy_hint, unsigned & handle_type) const
	{
		HICON icon = (HICON)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IconID), IMAGE_ICON, cx_hint, cy_hint, NULL);
		handle_type = uie::button_v2::handle_type_icon;
		return (HANDLE) icon;
	}
};

uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_stop, IDI_STOP> > g_button_stop;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_open, IDI_OPEN> > g_button_open;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_play, IDI_PLAY> > g_button_play;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_pause, IDI_PAUSE> > g_button_pause;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_next, IDI_NEXT> > g_button_next;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_previous, IDI_PREV> > g_button_previous;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_random, IDI_RAND> > g_button_random;


#if 0
class button_stop : public ui_extension::button//, play_callback
{
	virtual const GUID & get_item_guid ()const
	{
		return standard_commands::guid_main_stop;
	}
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
	{
		
		//COLORMAP map;
		//map.from = 0x0;
		//map.to = cr_btntext;

		HBITMAP bm = NULL;

		DLLVERSIONINFO2 dvi;
		if (SUCCEEDED(win32_helpers::get_comctl32_version(dvi)) && dvi.info1.dwMajorVersion >= 6)
		{
			p_mask_type = uie::MASK_NONE;
			bm = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP32), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_CREATEDIBSECTION);
		}
		else
		{
			p_mask_type = ui_extension::MASK_BITMAP;
			bm = //CreateMappedBitmap(core_api::get_my_instance(), IDB_STOP, 0, &map, 1);
				//(HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
				LoadMonoBitmap(IDB_STOP, cr_btntext);
			if (bm)
				bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE); 
		}

		return bm;
	}
#if 0
	virtual void register_callback(uie::button_callback & p_callback)
	{
		m_callback = &p_callback;
		static_api_ptr_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_all, false);
	};
	virtual void deregister_callback(uie::button_callback & p_callback)
	{
		assert (m_callback == &p_callback);
		m_callback = NULL;
		static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
	};
	virtual void FB2KAPI on_playback_starting(play_control::t_track_command p_command,bool p_paused){};
	virtual void FB2KAPI on_playback_new_track(metadb_handle_ptr p_track)
	{
		if (m_callback) m_callback->on_button_state_change(uie::BUTTON_STATE_DEFAULT);
	};
	virtual void FB2KAPI on_playback_stop(play_control::t_stop_reason p_reason)
	{
		if (m_callback) m_callback->on_button_state_change(NULL);
	};
	virtual void FB2KAPI on_playback_seek(double p_time){};
	virtual void FB2KAPI on_playback_pause(bool p_state){};
	virtual void FB2KAPI on_playback_edited(metadb_handle_ptr p_track){};
	virtual void FB2KAPI on_playback_dynamic_info(const file_info & p_info){};
	virtual void FB2KAPI on_playback_dynamic_info_track(const file_info & p_info){};
	virtual void FB2KAPI on_playback_time(double p_time){};
	virtual void FB2KAPI on_volume_change(float p_new_val){};
	uie::button_callback * m_callback;
public:
	button_stop() : m_callback(NULL) {};
#endif
};

ui_extension::button_factory<button_stop> g_stop;
#endif



class button_blank : public ui_extension::custom_button
{
	virtual const GUID & get_item_guid ()const
	{
		// {A8FE61BA-A055-4a53-A588-9DDA92ED7312}
		static const GUID guid = 
		{ 0xa8fe61ba, 0xa055, 0x4a53, { 0xa5, 0x88, 0x9d, 0xda, 0x92, 0xed, 0x73, 0x12 } };
		return guid;
	}
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
	{
		COLORMAP map;
		map.from = 0x0;
		map.to = cr_btntext;

		p_mask_type = ui_extension::MASK_BITMAP;
		HBITMAP bm = LoadMonoBitmap(IDB_BLANK, cr_btntext);
		if (bm)
			bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_BLANK), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_MONOCHROME); 
		return bm;
	}
	virtual void get_name(pfc::string_base & p_out) const
	{
		p_out = "Blanking button";
	}
	virtual unsigned get_button_state() const
	{
		return NULL;
	}
	void execute(const pfc::list_base_const_t<metadb_handle_ptr> & p_items) {};
	virtual uie::t_button_guid get_guid_type() const
	{
		return uie::BUTTON_GUID_BUTTON;
	}

};

ui_extension::button_factory<button_blank> g_blank;

#if 0
class button_next : public ui_extension::button
{
	virtual const GUID & get_item_guid ()const
	{
		return standard_commands::guid_main_next;
	}
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
	{
		COLORMAP map;
		map.from = 0x0;
		map.to = cr_btntext;

		HBITMAP bm = NULL;

		DLLVERSIONINFO2 dvi;
		if (SUCCEEDED(win32_helpers::get_comctl32_version(dvi)) && dvi.info1.dwMajorVersion >= 6)
		{
			p_mask_type = uie::MASK_NONE;
			bm = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NEXT32), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_CREATEDIBSECTION);
		}
		else
		{
		p_mask_type = ui_extension::MASK_BITMAP;
		bm = LoadMonoBitmap(IDB_NEXT, cr_btntext);//CreateMappedBitmap(core_api::get_my_instance(), IDB_NEXT, 0, &map, 1);
		if (bm)
		bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NEXT), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_MONOCHROME); 
		}return bm;
	}
#if 0
	virtual void get_menu_items(uie::menu_hook_t & p_out)
	{
		class menu_node_play_item : public ui_extension::menu_node_leaf
		{
			unsigned m_index, m_playlist;
			service_ptr_t<playlist_manager> m_playlist_api;
			service_ptr_t<play_control> m_play_api;
		public:
			virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags)
			{
				metadb_handle_ptr p_handle;
				m_playlist_api->playlist_get_item_handle(p_handle, m_playlist, m_index);
				if (p_handle.is_valid())
					p_handle->query_meta_field("TITLE", 0, p_out);
				p_displayflags= 0;
				return true;
			}
			virtual bool get_description(pfc::string_base & p_out)
			{
				return false;
			}
			virtual void execute()
			{
				m_playlist_api->playlist_set_playback_cursor(m_playlist, m_index);
				m_play_api->play_start(play_control::TRACK_COMMAND_SETTRACK);
			}
			menu_node_play_item(unsigned pl, unsigned p_index)
				: m_playlist(pl), m_index(p_index)
			{
				playlist_manager::g_get(m_playlist_api);
				play_control::g_get(m_play_api);
			};
		};
		unsigned cursor, playlist;
		static_api_ptr_t<playlist_manager> api;
		if (!api->get_playing_item_location(&playlist, &cursor))
		{
			playlist = api->get_active_playlist();
			cursor = api->playlist_get_playback_cursor(playlist);
		}
		if (config_object::g_get_data_bool_simple(standard_config_objects::bool_playback_follows_cursor, false))
		{
			cursor = api->activeplaylist_get_focus_item();
			playlist = api->get_active_playlist();
		}

		unsigned n, count = api->playlist_get_item_count(playlist);
		for(n=0; n<10 && n+cursor<count; n++)
			p_out.add_node(uie::menu_node_ptr(new menu_node_play_item(playlist, cursor+n)));
	}
#endif
};

ui_extension::button_factory<button_next> g_next;
#endif

class toolbar_extension : public ui_extension::container_ui_extension
{
	static const TCHAR * class_name;
	int width;
	int height;

	enum t_config_version
	{
		VERSION_1,
		VERSION_2,
		VERSION_CURRENT = VERSION_2
	};

	enum t_filter
	{
		FILTER_NONE,
		FILTER_PLAYING,
		FILTER_PLAYLIST,
		FILTER_ACTIVE_SELECTION
	};

	enum t_type
	{
		TYPE_SEPARATOR,
		TYPE_BUTTON,
		TYPE_MENU_ITEM_CONTEXT,
		TYPE_MENU_ITEM_MAIN,
	};

	enum t_show
	{
		SHOW_IMAGE,
		SHOW_IMAGE_TEXT,
		SHOW_TEXT
	};

	enum t_appearance
	{
		APPEARANCE_NORMAL,
		APPEARANCE_FLAT,
		APPEARANCE_NOEDGE
	};

	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data_child_ex(class_name, true, false);
	}

	WNDPROC menuproc;
	bool initialised, m_gdiplus_initialised;
	ULONG_PTR m_gdiplus_instance;

public:
	class button {
	public:
		t_type m_type;
		t_filter m_filter;
		t_show m_show;
		types::t_guid m_guid;
		types::t_guid m_subcommand;
		bool m_use_custom;
		bool m_use_custom_hot;
		bool m_use_custom_text;
		pfc::string_simple m_text;
		class custom_image
		{
		public:
			pfc::string_simple m_path;
			pfc::string_simple m_mask_path;
			COLORREF m_mask_colour;
			ui_extension::t_mask m_mask_type;
			custom_image & operator = (const custom_image & p_source)
			{
				m_path = p_source.m_path;
				m_mask_type = p_source.m_mask_type;
//				if (m_mask_type == uie::MASK_BITMAP)
					m_mask_path = p_source.m_mask_path;
//				else if (m_mask_type == uie::MASK_COLOUR)
					m_mask_colour = p_source.m_mask_colour;
				return *this;
			}
			void get_path(pfc::string8 & p_out) const
			{
				p_out.reset();

				bool b_absolute = pfc::string_find_first(m_path, ':') != pfc_infinite || (m_path.length() > 1 && m_path.get_ptr()[0] == '\\' && m_path.get_ptr()[1] == '\\');
				bool b_relative_to_drive = !b_absolute && m_path.get_ptr()[0] == '\\';

				pfc::string8 fb2kexe;
				uGetModuleFileName(NULL, fb2kexe);
				//pfc::string8 fullPath;

				if (b_relative_to_drive)
				{
					t_size index_colon = fb2kexe.find_first(':');
					if (index_colon != pfc_infinite) 
						p_out.add_string(fb2kexe.get_ptr(), index_colon + 1);
				}
				else if (!b_absolute)
					p_out << pfc::string_directory(fb2kexe) << "\\";
				p_out += m_path;
			}
			void write(stream_writer * out, abort_callback & p_abort) const
			{
					out->write_string(m_path,p_abort);
					out->write_lendian_t(m_mask_type,p_abort);
					if (m_mask_type == uie::MASK_BITMAP)
					{
						out->write_string(m_mask_path,p_abort);
					}
					else if (m_mask_type == uie::MASK_COLOUR)
						out->write_lendian_t(m_mask_colour,p_abort);
			}
			void read(t_config_version p_version,stream_reader * reader, abort_callback & p_abort)
			{
					pfc::string8 temp;
					reader->read_string(temp, p_abort);
					m_path = temp;
					reader->read_lendian_t(m_mask_type, p_abort);
					if (m_mask_type == uie::MASK_BITMAP)
					{
						reader->read_string(temp, p_abort);
						m_mask_path = temp;
					}
					else if (m_mask_type == uie::MASK_COLOUR)
						reader->read_lendian_t(m_mask_colour, p_abort);
			}
			void write_to_file(stream_writer & p_file, bool b_paths, abort_callback & p_abort);
			void read_from_file(t_config_version p_version, const char * p_base, const char * p_name, stream_reader * p_file, unsigned p_size, abort_callback & p_abort);

		}
		m_custom_image, m_custom_hot_image;
		service_ptr_t<uie::button> m_interface;

		class callback_impl : public uie::button_callback
		{
			virtual void on_button_state_change(unsigned p_new_state) //see t_button_state
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
				SendMessage(m_this->wnd_toolbar, TB_SETSTATE, id, MAKELONG(state,0));
			};

			virtual void on_command_state_change(unsigned p_new_state){};

			service_ptr_t<toolbar_extension> m_this;
			unsigned id;
		public:
			callback_impl & operator = (const callback_impl & p_source)
			{
				m_this = p_source.m_this;
				return *this;
			}
			void set_wnd(toolbar_extension * p_source)
			{
				m_this = p_source;
			}
			void set_id(const unsigned i)
			{
				id = i;
			}
			callback_impl() : id(0){};
		}
		m_callback;

		inline void set(const button & p_source)
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
		inline button(
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
		inline button() {;}
		inline const button & operator= (const button & p_source) 
		{
			set(p_source);
			return *this;
		}
		void write(stream_writer * out, abort_callback & p_abort) const
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

		void read(t_config_version p_version,stream_reader * reader, abort_callback & p_abort)
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
					m_custom_image.read(p_version,reader, p_abort);
				}
				reader->read_lendian_t(m_use_custom_hot, p_abort);
				if (m_use_custom_hot)
				{
					m_custom_hot_image.read(p_version,reader, p_abort);
				}
				reader->read_lendian_t(m_use_custom_text, p_abort);
				if (m_use_custom_text)
				{
					pfc::string8 temp;
					reader->read_string(temp, p_abort);
					m_text = temp;
				}
		}
		void get_display_text(pfc::string_base & p_out) //display
		{
			p_out.reset();
			if (m_use_custom_text)
				p_out = m_text;
			else
				get_short_name(p_out);
		}
		void get_short_name(pfc::string_base & p_out) //tooltip
		{
			p_out.reset();
			if (m_type == TYPE_BUTTON)
				uie::custom_button::g_button_get_name(m_guid, p_out);
			else if (m_type == TYPE_SEPARATOR)
				p_out  = "Separator";
			else if (m_type == TYPE_MENU_ITEM_MAIN)
				menu_helpers::mainpath_from_guid(m_guid, m_subcommand, p_out, true);
			else
				menu_helpers::contextpath_from_guid(m_guid, m_subcommand, p_out, true);
		}

		void get_name_type(pfc::string_base & p_out) //config
		{
			p_out.reset();
			if (m_type == TYPE_BUTTON)
			{
				p_out = "Button";
			}
			else if (m_type == TYPE_SEPARATOR)
				p_out  = "Separator";
			else if (m_type == TYPE_MENU_ITEM_MAIN)
			{
				p_out = "Main menu item";
			}
			else
			{
				p_out = "Shortcut menu item";
			}
		}
		void get_name_name(pfc::string_base & p_out) //config
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
				p_out  = "-";
			else if (m_type == TYPE_MENU_ITEM_MAIN)
			{
				pfc::string8 temp;
				menu_helpers::mainpath_from_guid(m_guid, m_subcommand, temp);
				p_out+=temp;
			}
			else
			{
				pfc::string8 temp;
				menu_helpers::contextpath_from_guid(m_guid, m_subcommand, temp);
				p_out+=temp;
			}
		}
		void get_name(pfc::string_base & p_out) //config
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
				p_out  = "[Separator]";
			else if (m_type == TYPE_MENU_ITEM_MAIN)
			{
				pfc::string8 temp;
				p_out = "[Main menu item] ";
				menu_helpers::mainpath_from_guid(m_guid, m_subcommand, temp);
				p_out+=temp;
			}
			else
			{
				pfc::string8 temp;
				menu_helpers::contextpath_from_guid(m_guid, m_subcommand, temp);
				p_out = "[Shortcut menu item] ";
				p_out+=temp;
			}
		}
		void write_to_file(stream_writer & p_file, bool b_paths, abort_callback & p_abort);
		void read_from_file(t_config_version p_version, const char * p_base, const char * p_name, stream_reader * p_file, unsigned p_size, abort_callback & p_abort);
	};

	class button_image
	{
		HBITMAP m_bm;
		HICON m_icon;
		ui_extension::t_mask m_mask_type;
		HBITMAP m_bm_mask;
		COLORREF m_mask_colour;
	public:
		button_image() : m_bm(0), m_mask_type(uie::MASK_NONE), m_bm_mask(0), m_mask_colour(0), m_icon(0)
		{
		}
		~button_image()
		{
			if (m_bm)
				DeleteBitmap(m_bm);
			if (m_bm_mask)
				DeleteBitmap(m_bm_mask);
			if (m_icon)
				DestroyIcon(m_icon);
		}
		bool is_valid() {return m_bm != 0;}
		void load(const button::custom_image & p_image)
		{
			m_mask_type = p_image.m_mask_type;
			m_mask_colour = p_image.m_mask_colour;

			pfc::string8 fullPath;
			p_image.get_path(fullPath);

			if (!_stricmp(pfc::string_extension(fullPath),"bmp")) //Gdiplus vs 32bpp
				m_bm = (HBITMAP)uLoadImage(core_api::get_my_instance(), fullPath, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADFROMFILE);
			else if (!_stricmp(pfc::string_extension(fullPath),"ico")) 
				m_icon = (HICON)uLoadImage(core_api::get_my_instance(), fullPath, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE);
			else
			{
				m_bm = g_load_png_gdiplus(0, fullPath);
					//g_load_png_wic(0, p_image.m_path);
				//read_png(0, p_image.m_path);
			}
			if (m_bm)
			{
				switch (p_image.m_mask_type)
				{
				default:
					break;
				case ui_extension::MASK_COLOUR:
					break;
#if 0
				case ui_extension::MASK_BITMAP:
					{
						if (!_stricmp(pfc::string_extension(p_image.m_mask_path),"png"))
						{
							m_bm_mask = read_png(0, p_image.m_mask_path);
						}
						else
							m_bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), pfc::stringcvt::string_os_from_utf8(p_image.m_mask_path), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADFROMFILE|LR_MONOCHROME);
						if (!m_bm_mask)
							console::printf("failed loading image \"%s\"",p_image.m_mask_path.get_ptr());
					}
					break;
#endif
				}
			}
			else if (!m_icon)
				console::printf("failed loading image \"%s\"",fullPath.get_ptr());
		}
		void load(const service_ptr_t<uie::button> & p_in, COLORREF colour_btnface, unsigned cx, unsigned cy)
		{
			uie::button_v2::ptr inv2;
			if (p_in->service_query_t(inv2))
			{
				unsigned handle_type = 0;
				HANDLE image = inv2->get_item_bitmap(0, colour_btnface, cx, cy, handle_type);
				if (handle_type == uie::button_v2::handle_type_bitmap)
					m_bm = (HBITMAP)image;
				else if (handle_type == uie::button_v2::handle_type_icon)
					m_icon = (HICON)image;
			}
			else
				m_bm = p_in->get_item_bitmap(0, colour_btnface, m_mask_type, m_mask_colour, m_bm_mask);
		}
		unsigned add_to_imagelist(HIMAGELIST iml)
		{
			unsigned rv = -1;
			if (m_icon)
			{
				rv = ImageList_ReplaceIcon(iml, -1, m_icon);
			}
			else if (m_bm)
			{
				switch (m_mask_type)
				{
				default:
					rv = ImageList_Add(iml, m_bm, 0);
					break;
				case ui_extension::MASK_COLOUR:
					rv = ImageList_AddMasked(iml, m_bm, m_mask_colour);
					break;
				case ui_extension::MASK_BITMAP:
					{
						rv = ImageList_Add(iml, m_bm, m_bm_mask);
					}
					break;
				}
			}
			return rv;
		}
		void get_size(SIZE & p_out)
		{
			p_out.cx = 0;
			p_out.cy = 0;
			BITMAP bmi;
			memset(&bmi, 0, sizeof(BITMAP));
			if (m_icon)
			{
				ICONINFO ii;
				memset(&ii, 0, sizeof(ii));
				if (GetIconInfo(m_icon, &ii))
				{
					GetObject(ii.hbmColor ? ii.hbmColor : ii.hbmMask, sizeof(BITMAP), &bmi);
				}
			}
			else if (m_bm)
			{
				GetObject(m_bm, sizeof(BITMAP), &bmi);
			}
			p_out.cx = bmi.bmWidth;
			p_out.cy = bmi.bmHeight;
		}
	};

	static const button g_button_null;

	HWND wnd_toolbar;
	HWND wnd_host;

	virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	toolbar_extension();
	~toolbar_extension();

	static const GUID extension_guid;

	virtual const GUID & get_extension_guid() const
	{
		return extension_guid;
	}

	virtual void get_name(pfc::string_base & out)const;

	virtual void get_category(pfc::string_base & out)const;

	class config_param
	{
	public:
		//service_ptr_t<toolbar_extension> m_this;
		class t_button_list_view : public t_list_view
		{
			config_param & m_param;
			static CLIPFORMAT g_clipformat()
			{
				static CLIPFORMAT cf = (CLIPFORMAT)RegisterClipboardFormat(L"CUIListViewStandardClipFormat");
				return cf;
			}
			struct DDData
			{
				t_uint32 version;
				HWND wnd;
			};
			class IDropTarget_buttons_list : public IDropTarget
			{
				long drop_ref_count;
				bool last_rmb;
				t_button_list_view * m_button_list_view;
				mmh::comptr_t<IDataObject> m_DataObject;
				mmh::comptr_t<IDropTargetHelper> m_DropTargetHelper;
				//pfc::string
			public:
				virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR *ppvObject)
				{
					if (ppvObject == NULL) return E_INVALIDARG;
					*ppvObject = NULL;
					if (riid == IID_IUnknown) {AddRef();*ppvObject = (IUnknown*)this;return S_OK;}
					else if (riid == IID_IDropTarget) {AddRef();*ppvObject = (IDropTarget*)this;return S_OK;}
					else return E_NOINTERFACE;
				}
				virtual ULONG STDMETHODCALLTYPE   AddRef()
				{
					return InterlockedIncrement(&drop_ref_count); 
				}
				virtual ULONG STDMETHODCALLTYPE   Release()
				{
					LONG rv = InterlockedDecrement(&drop_ref_count);
					if (!rv)
						delete this;
					return rv;
				}
				bool check_do(IDataObject *pDO)
				{
					bool rv = false;
					FORMATETC fe;
					memset(&fe, 0, sizeof (fe));
					fe.cfFormat = g_clipformat();
					fe.dwAspect = DVASPECT_CONTENT;
					fe.lindex = -1;
					fe.tymed = TYMED_HGLOBAL;
					//console::formatter() << "check_do: " << (int)fe.cfFormat;
					STGMEDIUM sm;
					memset(&sm, 0, sizeof (sm));
					if (SUCCEEDED(pDO->GetData(&fe, &sm)))
					{
						DDData * pDDD = (DDData*)GlobalLock(sm.hGlobal);
						if (pDDD->version == 0 && pDDD->wnd == m_button_list_view->get_wnd()) rv = true;
						ReleaseStgMedium(&sm);
					}
					return rv;
				}
				virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
				{
					//console::formatter() << "DragEnter";
					m_DataObject = pDataObj;
					last_rmb = ((grfKeyState & MK_RBUTTON) != 0);
					POINT pt = {ptl.x, ptl.y};
					if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragEnter(m_button_list_view->get_wnd(), pDataObj, &pt, *pdwEffect);
					*pdwEffect = DROPEFFECT_NONE;
					if (check_do(pDataObj/*, pdwEffect*/))
					{
						mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_MOVE, "Move", "");
						*pdwEffect = DROPEFFECT_MOVE;
					}
					return S_OK; 	
				}
				virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
				{
					//console::formatter() << "DragOver";
					POINT pt = {ptl.x, ptl.y};
					if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragOver(&pt, *pdwEffect);

					last_rmb = ((grfKeyState & MK_RBUTTON) != 0);
					POINT pti;
					pti.y = ptl.y;
					pti.x = ptl.x;

					*pdwEffect = DROPEFFECT_NONE;
					if (check_do(m_DataObject/*, pdwEffect*/))
					{
						*pdwEffect = DROPEFFECT_MOVE;
						HRESULT hr = mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_MOVE, "Move", "");
						//console::formatter() << pfc::format_hex(hr);

					}
					if (m_button_list_view->get_wnd())
					{
						t_list_view::t_hit_test_result hi;

						{
							POINT ptt = pti;
							ScreenToClient(m_button_list_view->get_wnd(), &ptt);

							RECT rc_items;
							m_button_list_view->get_items_rect(&rc_items);

							rc_items.top += m_button_list_view->get_item_height();
							rc_items.bottom -= m_button_list_view->get_item_height();

							if (ptt.y < rc_items.top && ptt.y < rc_items.bottom)
							{
								m_button_list_view->create_timer_scroll_up();
							}
							else m_button_list_view->destroy_timer_scroll_up();

							if (ptt.y >= rc_items.top && ptt.y >= rc_items.bottom)
							{
								m_button_list_view->create_timer_scroll_down();
							}
							else m_button_list_view->destroy_timer_scroll_down();
							{
								m_button_list_view->hit_test_ex(ptt, hi, true);
								if (hi.result == t_list_view::hit_test_on || hi.result == t_list_view::hit_test_on_group
									|| hi.result == t_list_view::hit_test_obscured_below)
									m_button_list_view->set_insert_mark(hi.index);
								else if (hi.result == t_list_view::hit_test_below_items)
									m_button_list_view->set_insert_mark(hi.index+1);
								else
									m_button_list_view->remove_insert_mark();
							}
						}
					}

					return S_OK; 
				}
				virtual HRESULT STDMETHODCALLTYPE DragLeave()
				{
					//console::formatter() << "DragLeave";
					if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragLeave();
					mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");
					m_button_list_view->remove_insert_mark();
					m_button_list_view->destroy_timer_scroll_up();
					m_button_list_view->destroy_timer_scroll_down();
					m_DataObject.release();
					return S_OK;		
				}
				virtual HRESULT STDMETHODCALLTYPE Drop( IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
				{
					POINT pt = {ptl.x, ptl.y};
					if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->Drop(pDataObj, &pt, *pdwEffect);

					*pdwEffect = DROPEFFECT_NONE;
					if (check_do(pDataObj/*, pdwEffect*/))
					{
						*pdwEffect = DROPEFFECT_MOVE;
						t_size index = m_button_list_view->get_selected_item_single(); //meh

						t_list_view::t_hit_test_result hi;

						POINT ptt = pt;
						ScreenToClient(m_button_list_view->get_wnd(), &ptt);
						
						m_button_list_view->hit_test_ex(ptt, hi, true);

						t_size new_index = pfc_infinite;

						if (hi.result == t_list_view::hit_test_on || hi.result == t_list_view::hit_test_on_group
							|| hi.result == t_list_view::hit_test_obscured_below)
							new_index = hi.index;
						else if (hi.result == t_list_view::hit_test_below_items)
							new_index = hi.index+1;

						if (new_index != pfc_infinite && new_index > index) --new_index;

						if (new_index != pfc_infinite && index != pfc_infinite && new_index != index && index < m_button_list_view->m_param.m_buttons.get_count())
						{
							t_size button_count = m_button_list_view->m_param.m_buttons.get_count(), abs_delta = abs(int(index)-int(new_index));
							int direction = new_index>index?-1:1;
							mmh::permutation_t perm(button_count);
							for (t_size i = 0; i < abs_delta; i++)
								perm.swap_items(new_index+direction*(i), new_index+direction*(i+1));

							m_button_list_view->m_param.m_buttons.reorder(perm.get_ptr());

							//blaarrgg, designed in the dark ages
							m_button_list_view->m_param.m_selection = &m_button_list_view->m_param.m_buttons[new_index];

							m_button_list_view->m_param.refresh_buttons_list_items(min(index,new_index), abs_delta+1);
							m_button_list_view->set_item_selected_single(new_index);

						}

					}
					m_DataObject.release();
					m_button_list_view->remove_insert_mark();
					m_button_list_view->destroy_timer_scroll_up();
					m_button_list_view->destroy_timer_scroll_down();
					return S_OK;
				}
				IDropTarget_buttons_list(t_button_list_view * p_blv) : drop_ref_count(0), last_rmb(false), m_button_list_view(p_blv)
				{
					m_DropTargetHelper.instantiate(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER);
				}


			};
			void notify_on_initialisation()
			{
				set_single_selection(true);

				pfc::list_t<t_list_view::t_column> columns;
				columns.add_item(t_list_view::t_column("Name", 300));
				columns.add_item(t_list_view::t_column("Type", 125));

				set_columns(columns);
			}
			void notify_on_create()
			{
				pfc::com_ptr_t<IDropTarget_buttons_list> IDT_blv = new IDropTarget_buttons_list(this);
				RegisterDragDrop(get_wnd(), IDT_blv.get_ptr());
			}
			void notify_on_destroy()
			{
				RevokeDragDrop(get_wnd());
			}
			void notify_on_selection_change(const bit_array & p_affected,const bit_array & p_status, notification_source_t p_notification_source)
			{
				t_size index = get_selected_item_single();
				m_param.on_selection_change(index);
			}
			virtual bool do_drag_drop(WPARAM wp)
			{
				UINT cf = g_clipformat();

				HGLOBAL glb = GlobalAlloc(GPTR, sizeof(DDData));
				if (glb)
				{
					DDData * pddd = (DDData*)glb;
					pddd->version = 0;
					pddd->wnd = get_wnd();

					FORMATETC fe;
					memset(&fe, 0, sizeof (fe));
					fe.cfFormat = cf;
					fe.dwAspect = DVASPECT_CONTENT;
					fe.lindex = -1;
					fe.tymed = TYMED_HGLOBAL;
					STGMEDIUM sm;
					memset(&sm, 0, sizeof (sm));
					sm.tymed = TYMED_HGLOBAL;
					sm.hGlobal = glb;

					mmh::comptr_t<IDataObject> pDO = new CDataObject;
					pDO->SetData(&fe, &sm, TRUE);

					DWORD blah = DROPEFFECT_NONE;

					if (g_test_os_version(6,0))
						SHDoDragDrop(get_wnd(), pDO, NULL, DROPEFFECT_MOVE, &blah);
					else
						SHDoDragDrop(get_wnd(), pDO, mmh::comptr_t<IDropSource>(new mmh::ole::IDropSource_Generic(get_wnd(), pDO, wp, true)), DROPEFFECT_MOVE, &blah);
					//DoDragDrop(pDO, mmh::comptr_t<IDropSource>(new mmh::ole::IDropSource_Generic(get_wnd(), pDO, wp, true)), DROPEFFECT_MOVE, &blah);
				}

				return true;
			}

		public:
			t_button_list_view(config_param & p_param) : m_param(p_param) {};
		} m_button_list;

		modal_dialog_scope m_scope;
		//t_list_view m_button_list;
		button * m_selection;
		HWND m_wnd, m_child;
		unsigned m_active;
		button :: custom_image * m_image;
		pfc::list_t<button> m_buttons;
		bool m_text_below;
		t_appearance m_appearance;
		void export_to_file(const char * p_path, bool b_paths = false);
		void import_from_file(const char * p_path, bool add);
		void export_to_stream(stream_writer * p_writer, bool b_paths, abort_callback & p_abort);
		void import_from_stream(stream_reader * p_reader, bool add, abort_callback & p_abort);

		static BOOL CALLBACK g_ConfigPopupProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
		BOOL ConfigPopupProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

		void on_selection_change(t_size index);
		void populate_buttons_list();
		void refresh_buttons_list_items(t_size index, t_size count, bool b_update_display = true);
		
		config_param() : m_button_list(*this), m_wnd(NULL), m_child(NULL) {};
	};

	//static BOOL CALLBACK toolbar_extension::ConfigPopupProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static BOOL CALLBACK toolbar_extension::ConfigChildProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static BOOL CALLBACK toolbar_extension::ConfigCommandProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	virtual bool have_config_popup()const{return true;}

	virtual bool show_config_popup(HWND wnd_parent);

	void create_toolbar();
	void destroy_toolbar();

	virtual void get_menu_items (ui_extension::menu_hook_t & p_hook)
	{
		ui_extension::menu_node_ptr p_node(new uie::menu_node_configure(this, "Buttons options"));
		p_hook.add_node(p_node);
	}

	virtual unsigned get_type  () const{return ui_extension::type_toolbar;};

	pfc::list_t<button> m_buttons;
	pfc::list_t<button> m_buttons_config;

	bool m_text_below;
	t_appearance m_appearance;

	static void reset_buttons(pfc::list_base_t<button> & p_buttons);

	virtual void get_config(stream_writer * data, abort_callback & p_abort) const;
	virtual void set_config(stream_reader * p_reader, t_size p_sizehint, abort_callback & p_abort);
	virtual void import_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort) 
	{
		config_param param;
		param.m_selection = 0;
		param.m_buttons = m_buttons_config;
		param.m_child = 0;
		param.m_active = 0;
		param.m_image = 0;
		param.m_text_below = m_text_below;
		param.m_appearance = m_appearance;
		param.import_from_stream(p_reader, false, p_abort);

		m_text_below = param.m_text_below;
		m_buttons_config = param.m_buttons;
		m_appearance = param.m_appearance;
		if (initialised)
		{
			destroy_toolbar();
			create_toolbar();
			get_host()->on_size_limit_change(wnd_host, ui_extension::size_limit_minimum_width);
		}
	};
	virtual void export_config(stream_writer * p_writer, abort_callback & p_abort) const 
	{
		config_param param;
		param.m_selection = 0;
		param.m_buttons = m_buttons_config;
		param.m_child = 0;
		param.m_active = 0;
		param.m_image = 0;
		param.m_text_below = m_text_below;
		param.m_appearance = m_appearance;
		param.export_to_stream(p_writer, false, p_abort);
	}

//	virtual void write_to_file(stream_writer * out);

	static const GUID g_guid_fcb;

	enum t_identifier {
		I_TEXT_BELOW,
		I_APPEARANCE,
		I_BUTTONS,
	};
	enum t_identifier_button {
		I_BUTTON_TYPE,I_BUTTON_FILTER,I_BUTTON_SHOW,I_BUTTON_GUID,I_BUTTON_CUSTOM,I_BUTTON_CUSTOM_DATA,I_BUTTON_CUSTOM_HOT,I_BUTTON_CUSTOM_HOT_DATA,
		I_BUTTON_MASK_TYPE,I_BUTTON_CUSTOM_IMAGE_DATA,I_BUTTON_CUSTOM_IMAGE_MASK_DATA,I_BUTTON_MASK_COLOUR,I_BUTTON_USE_CUSTOM_TEXT,I_BUTTON_TEXT,I_BUTTON_SUBCOMMAND
	};

	enum t_custom_image_identifiers
	{
		I_CUSTOM_BUTTON_PATH,
		I_CUSTOM_BUTTON_MASK_PATH,
		//I_BUTTON_MASK_TYPE=8
	};

	enum t_image_identifiers {
		IMAGE_NAME,
		IMAGE_DATA,
		IMAGE_PATH
	};


};
	namespace pfc
	{
		template<> class traits_t<toolbar_extension::t_image_identifiers> : public traits_rawobject {};
		template<> class traits_t<toolbar_extension::t_custom_image_identifiers> : public traits_rawobject {};
		template<> class traits_t<toolbar_extension::t_identifier_button> : public traits_rawobject {};
		template<> class traits_t<toolbar_extension::t_identifier> : public traits_rawobject {};
		template<> class traits_t<toolbar_extension::t_appearance> : public traits_rawobject {};
		template<> class traits_t<toolbar_extension::t_config_version> : public traits_rawobject {};
		template<> class traits_t<toolbar_extension::t_show> : public traits_rawobject {};
		template<> class traits_t<toolbar_extension::t_filter> : public traits_rawobject {};
		template<> class traits_t<toolbar_extension::t_type> : public traits_rawobject {};
		template<> class traits_t<uie::t_mask> : public traits_rawobject {};
	}

void toolbar_extension::config_param::export_to_stream(stream_writer * p_file, bool b_paths, abort_callback & p_abort)
{
	p_file->write_lendian_t(g_guid_fcb, p_abort);
	p_file->write_lendian_t(VERSION_CURRENT, p_abort);

	unsigned n,count = m_buttons.get_count();

	p_file->write_lendian_t(I_TEXT_BELOW, p_abort);
	p_file->write_lendian_t(sizeof(m_text_below), p_abort);
	p_file->write_lendian_t(m_text_below, p_abort);

	p_file->write_lendian_t(I_APPEARANCE, p_abort);
	p_file->write_lendian_t(sizeof(m_appearance), p_abort);
	p_file->write_lendian_t(m_appearance, p_abort);

	p_file->write_lendian_t(I_BUTTONS, p_abort);

	stream_writer_memblock p_write;
	//FIX

	p_file->write_lendian_t(p_write.m_data.get_size() + sizeof(count), p_abort);
	p_file->write_lendian_t(count, p_abort);

	for (n=0; n<count; n++)
	{
		m_buttons[n].write_to_file(p_write, b_paths, p_abort);
		p_file->write_lendian_t(p_write.m_data.get_size(), p_abort);
		p_file->write(p_write.m_data.get_ptr(), p_write.m_data.get_size(), p_abort);
		p_write.m_data.set_size(0);
	}
}

void toolbar_extension::config_param::export_to_file(const char * p_path, bool b_paths)
	{

		try {
			abort_callback_impl p_abort;
			service_ptr_t<file> p_file;
			filesystem::g_open(p_file, p_path, filesystem::open_mode_write_new, p_abort);
			export_to_stream(p_file.get_ptr(), b_paths, p_abort);
		}
		catch (const pfc::exception & p_error)
		{
			popup_message::g_show_ex(p_error.what(), pfc_infinite, "Error writing FCB file", pfc_infinite);;
			filesystem::g_remove(p_path, abort_callback_impl());
		}
	}

void toolbar_extension::config_param::import_from_stream(stream_reader * p_file, bool add, abort_callback & p_abort)
{
	const char * profilepath = core_api::get_profile_path();
	if (!profilepath) throw pfc::exception_bug_check("NULL profile path");
	if (!stricmp_utf8_max(profilepath, "file://", 7))
		profilepath += 7;
	pfc::string8 str_base = profilepath;
	t_size blen = str_base.get_length();
	if (blen && str_base[blen-1] == '\\')
		str_base.truncate(blen-1);
	//uGetModuleFileName(NULL, str_base);
	//unsigned pos = str_base.find_last('\\');
	//if (pos != -1)
	//	str_base.truncate(pos);
	if (!add)
		m_buttons.remove_all();

	{
		GUID temp;
		p_file->read_lendian_t(temp, p_abort);
		if (temp != g_guid_fcb)
			throw exception_io_data();
		t_config_version vers;
		p_file->read_lendian_t(vers, p_abort);
		if (vers > VERSION_CURRENT)
			throw "Fcb version is newer than component";
		while (1) //!p_file.is_eof(p_abort)
		{
			t_identifier id;
			try {
			p_file->read_lendian_t(id, p_abort);
			} catch (exception_io_data_truncation const &) {break;}
			unsigned size;
			p_file->read_lendian_t(size, p_abort);
			//if (size > p_file->get_size(p_abort) - p_file->get_position(p_abort))
			//	throw exception_io_data();
			switch (id)
			{
			case I_TEXT_BELOW:
				p_file->read_lendian_t(m_text_below, p_abort);
				break;
			case I_APPEARANCE:
				p_file->read_lendian_t(m_appearance, p_abort);
				break;
			case I_BUTTONS:
				{
					service_ptr_t<genrand_service> genrand = genrand_service::g_create();
					genrand->seed(GetTickCount());
					t_uint32 dirname = genrand->genrand(pfc_infinite);

					unsigned count, n;
					p_file->read_lendian_t(count, p_abort);
					for (n=0; n<count; n++)
					{
						button temp = g_button_null;
						unsigned size_button;
						p_file->read_lendian_t(size_button, p_abort);
						temp.read_from_file(vers, str_base, pfc::string_formatter() << dirname, p_file, size_button, p_abort);
//						assert(n < 7);
						m_buttons.add_item(temp);
					}

				}
				break;
			}
		}
	}
}
void toolbar_extension::config_param::import_from_file(const char * p_path, bool add)
{
	try {
		abort_callback_impl p_abort;
		service_ptr_t<file> p_file;
		filesystem::g_open(p_file, p_path, filesystem::open_mode_read, p_abort);
		import_from_stream(p_file.get_ptr(), add, p_abort);
	}
	catch (const pfc::exception & p_error)
	{
		popup_message::g_show_ex(p_error.what(), pfc_infinite, "Error reading FCB file", pfc_infinite);
	}
	catch (const char * p_error)
	{
		popup_message::g_show_ex(p_error, pfc_infinite, "Error reading FCB file", pfc_infinite);;
	}
}

void toolbar_extension::button::read_from_file(t_config_version p_version, const char * p_base, const char * p_name, stream_reader * p_file, unsigned p_size, abort_callback & p_abort) throw (const exception_io &)
{
	//t_filesize p_start = p_file->get_position(p_abort);
	t_size read=0;
	while (read < p_size/* && !p_file->is_eof(p_abort)*/)
	{
		t_identifier_button id;
		p_file->read_lendian_t(id, p_abort);
		unsigned size;
		p_file->read_lendian_t(size, p_abort);
		//if (size > p_file->get_size(p_abort) - p_file->get_position(p_abort))
		//	throw exception_io_data();
		read += sizeof (t_uint32) + sizeof (t_uint32) + size;
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

void toolbar_extension::button::custom_image::read_from_file(t_config_version p_version, const char * p_base, const char * p_name, stream_reader * p_file, unsigned p_size, abort_callback & p_abort) throw (const exception_io &)
{
	//t_filesize p_start = p_file->get_position(p_abort);
	t_filesize read=0;
	while (/*p_file->get_position(p_abort) - p_start*/ read < p_size /* && !p_file->is_eof(p_abort)*/)
	{
		t_identifier id;
		p_file->read_lendian_t(id, p_abort);
		unsigned size;
		p_file->read_lendian_t(size, p_abort);
		//if (size > p_file->get_size(p_abort) - p_file->get_position(p_abort))
		//	throw exception_io_data();
		pfc::array_t<char> path, maskpath;
		read += 8 + size;
		switch (id)
		{
		case I_BUTTON_MASK_TYPE:
			p_file->read_lendian_t(m_mask_type, p_abort);
			break;
		case I_BUTTON_MASK_COLOUR:
			p_file->read_lendian_t(m_mask_colour, p_abort);
			break;
		case I_CUSTOM_BUTTON_MASK_PATH:
			maskpath.set_size(size);
			p_file->read(maskpath.get_ptr(), maskpath.get_size(), p_abort);
			m_mask_path.set_string(maskpath.get_ptr(), maskpath.get_size());
			break;
		case I_CUSTOM_BUTTON_PATH:
			{
				path.set_size(size);
				p_file->read(path.get_ptr(), path.get_size(), p_abort);
				m_path.set_string(path.get_ptr(), path.get_size());
			}
			break;
		case I_BUTTON_CUSTOM_IMAGE_DATA:
			{
				t_filesize read2=0;
				//t_filesize p_start_data = p_file->get_position(p_abort);
				pfc::array_t<char> name;
				pfc::array_t<t_uint8> data;
				while (read2 /*p_file->get_position(p_abort) - p_start_data*/ < size/* && !p_file->is_eof(p_abort)*/)
				{
					DWORD size_data;
					t_identifier id_data;
					p_file->read_lendian_t(id_data, p_abort);
					p_file->read_lendian_t(size_data, p_abort);
					//if (size_data > p_file->get_size(p_abort) - p_file->get_position(p_abort))
						//throw exception_io_data();
					read2 += 8 + size_data;
					switch (id_data)
					{
					case IMAGE_NAME:
						name.set_size(size_data);
						p_file->read(name.get_ptr(), name.get_size(), p_abort);
						break;
					case IMAGE_DATA:
						data.set_size(size_data);
						p_file->read(data.get_ptr(), data.get_size(), p_abort);
						break;
					default:
						if (p_file->skip(size_data, p_abort) != size_data)
							throw exception_io_data_truncation();
						break;
					}
				}
				{
					pfc::string_printf dir1("%s\\images",p_base);
					pfc::string_printf dir2("%s\\images\\%s",p_base,p_name);
					if (!filesystem::g_exists(dir1, p_abort))
						filesystem::g_create_directory(dir1, p_abort);
					if (!filesystem::g_exists(dir2, p_abort))
						filesystem::g_create_directory(dir2, p_abort);

					service_ptr_t<file> p_image;
					pfc::string8 curdir, wname;

					curdir = pfc::string_printf("%s\\images",p_base);

					wname = curdir;
					wname.add_byte('\\');

					wname.add_string(p_name);
					wname.add_byte('\\');

					wname.add_string(name.get_ptr(), name.get_size());
					pfc::string8 name_only = pfc::string_filename(wname);
					pfc::string8 ext = pfc::string_extension(wname);
					unsigned n=0;

					bool b_write = true;
					{
						bool b_continue = false;
						do
						{
							bool b_exists = filesystem::g_exists(wname, p_abort);
							if (b_exists)
							{
								b_continue = true;
								service_ptr_t<file> p_temp;
								try
								{
								filesystem::g_open(p_temp, wname, filesystem::open_mode_read, p_abort);
								{
									bool b_same = false;
									g_compare_file_with_bytes(p_temp, data, b_same, p_abort);
									if(b_same)
									{
										b_write = false;
										b_continue = false;
									}
								}
								}
								catch (pfc::exception & e)
								{
								}
							}
							else b_continue = false;

							if (b_continue && n<100 )
								wname = pfc::string_printf("%s\\%s\\%s %02u.%s",curdir.get_ptr(),p_name,name_only.get_ptr(), n, ext.get_ptr());n++;

						}
						while(b_continue && n<100);
					}

					if (b_write)
					{
						filesystem::g_open(p_image, wname, filesystem::open_mode_write_new, p_abort);
						p_image->write(data.get_ptr(), data.get_size(), p_abort);
					}
					m_path = wname;
				}
			}
			break;
		case I_BUTTON_CUSTOM_IMAGE_MASK_DATA:
			{
				t_filesize read2=0;
				//t_filesize p_start_data = p_file->get_position(p_abort);
				pfc::array_t<char> name;
				pfc::array_t<t_uint8> data;
				while (read2 /*p_file->get_position(p_abort) - p_start_data*/ < size/* && !p_file->is_eof(p_abort)*/)
				{
					DWORD size_data;
					t_identifier id_data;
					p_file->read_lendian_t(id_data, p_abort);
					p_file->read_lendian_t(size_data, p_abort);
					//if (size_data > p_file->get_size(p_abort) - p_file->get_position(p_abort))
					//	throw exception_io_data();
					read2 += 8 + size_data;
					switch (id_data)
					{
					case IMAGE_NAME:
						name.set_size(size_data);
						p_file->read(name.get_ptr(), name.get_size(), p_abort);
						break;
					case IMAGE_DATA:
						data.set_size(size_data);
						p_file->read(data.get_ptr(), data.get_size(), p_abort);
						break;
					default:
						if (p_file->skip(size_data, p_abort) != size_data)
							throw exception_io_data_truncation();
						break;
					}
				}
				{
					pfc::string_printf dir1("%s\\images",p_base);
					pfc::string_printf dir2("%s\\images\\%s",p_base,p_name);
					if (!filesystem::g_exists(dir1, p_abort))
						filesystem::g_create_directory(dir1, p_abort);
					if (!filesystem::g_exists(dir2, p_abort))
						filesystem::g_create_directory(dir2, p_abort);
					service_ptr_t<file> p_image;
					pfc::string8 curdir, wname;

					curdir = pfc::string_printf("%s\\images",p_base);

					wname = curdir;
					wname.add_byte('\\');

					wname.add_string(p_name);
					wname.add_byte('\\');

					wname.add_string(name.get_ptr(), name.get_size());

					pfc::string8 name_only = pfc::string_filename(wname);
					pfc::string8 ext = pfc::string_extension(wname);
					unsigned n=0;
					bool b_write = true;
					{
						bool b_continue = false;
						do
						{
							bool b_exists = filesystem::g_exists(wname, p_abort);
							if (b_exists)
							{
								b_continue = true;
								service_ptr_t<file> p_temp;
								try
								{
									filesystem::g_open(p_temp, wname, filesystem::open_mode_read, p_abort);
									{
										bool b_same = false;
										g_compare_file_with_bytes(p_temp, data, b_same, p_abort);
										if (b_same)
										{
											b_write = false;
											b_continue = false;
										}
									}
								}
								catch (pfc::exception & e)
								{
								}
							}
							else b_continue = false;

							if (b_continue && n<100 )
								wname = pfc::string_printf("%s\\%s\\%s %02u.%s",curdir.get_ptr(),p_name,name_only.get_ptr(), n, ext.get_ptr());n++;

						}
						while(b_continue && n<100);
					}


					if (b_write)
					{
						filesystem::g_open(p_image, wname, filesystem::open_mode_write_new, p_abort);
						p_image->write(data.get_ptr(), data.get_size(), p_abort);
					}
					m_mask_path = wname;
				}
			}
			break;
		default:
			if (p_file->skip(size, p_abort) != size)
				throw exception_io_data_truncation();
			break;
		}
	}
}


void toolbar_extension::button::custom_image::write_to_file(stream_writer &p_file, bool b_paths, abort_callback & p_abort) throw (const exception_io &)
{
	p_file.write_lendian_t(I_BUTTON_MASK_TYPE, p_abort);
	p_file.write_lendian_t(sizeof(m_mask_type), p_abort);
	p_file.write_lendian_t(m_mask_type, p_abort);

	if (b_paths)
	{
		p_file.write_lendian_t(I_CUSTOM_BUTTON_PATH, p_abort);
		p_file.write_lendian_t(m_path.length(), p_abort);
		p_file.write(m_path.get_ptr(), m_path.length(), p_abort);
	}
	else
	{
		pfc::string8 realPath, canPath;
		try{
		p_file.write_lendian_t(I_BUTTON_CUSTOM_IMAGE_DATA, p_abort);

		{
			service_ptr_t<file> p_image;
			get_path(realPath);
			filesystem::g_get_canonical_path(realPath, canPath);
			filesystem::g_open(p_image, canPath, filesystem::open_mode_read, p_abort);

			pfc::string_filename_ext name(m_path);

			t_filesize imagesize = p_image->get_size(p_abort);

			if (imagesize >= MAXLONG)
				throw exception_io_device_full();

			unsigned size = (unsigned)imagesize + name.length() + 4*sizeof(t_uint32);
			p_file.write_lendian_t(size, p_abort);

			p_file.write_lendian_t(IMAGE_NAME, p_abort);
			p_file.write_lendian_t(name.length(), p_abort);
			p_file.write(name.get_ptr(), name.length(), p_abort);

			p_file.write_lendian_t(IMAGE_DATA, p_abort);
			p_file.write_lendian_t((unsigned)imagesize, p_abort);
			pfc::array_t<t_uint8> temp;
			temp.set_size((unsigned)imagesize);
			p_image->read(temp.get_ptr(), temp.get_size(), p_abort);
			p_file.write(temp.get_ptr(), temp.get_size(), p_abort);
		}
		}
		catch (const pfc::exception & err)
		{
			throw pfc::exception(pfc::string_formatter() << "Error reading file \"" << realPath << "\" : " <<err.what());
		}
	}
	if (m_mask_type == uie::MASK_BITMAP)
	{
		if (b_paths)
		{
			p_file.write_lendian_t(I_CUSTOM_BUTTON_MASK_PATH, p_abort);
			p_file.write_lendian_t(m_mask_path.length(), p_abort);
			p_file.write(m_mask_path.get_ptr(), m_mask_path.length(), p_abort);
		}
		else
		{
			try {
			p_file.write_lendian_t(I_BUTTON_CUSTOM_IMAGE_MASK_DATA, p_abort);

			service_ptr_t<file> p_image;
			filesystem::g_open(p_image, m_mask_path, filesystem::open_mode_read, p_abort);

			pfc::string_filename_ext name(m_mask_path);

			t_filesize imagesize = p_image->get_size(p_abort);

			if (imagesize >= MAXLONG)
				throw exception_io_device_full();

			unsigned size = (unsigned)imagesize + name.length() + sizeof(IMAGE_NAME) + sizeof(IMAGE_DATA);
			p_file.write_lendian_t(size, p_abort);

			p_file.write_lendian_t(IMAGE_NAME, p_abort);
			p_file.write_lendian_t(name.length(), p_abort);
			p_file.write(name.get_ptr(), name.length(), p_abort);

			p_file.write_lendian_t(IMAGE_DATA, p_abort);
			p_file.write_lendian_t((unsigned)imagesize, p_abort);
			pfc::array_t<t_uint8> temp;
			temp.set_size((unsigned)imagesize);
			p_image->read(temp.get_ptr(), temp.get_size(),p_abort);
			p_file.write(temp.get_ptr(), temp.get_size(), p_abort);
			}
			catch (const pfc::exception & err)
			{
				throw pfc::exception(pfc::string_formatter() << "Error reading file \"" << m_mask_path << "\" : " <<err.what());
			}
		}
	}

	if (m_mask_type == uie::MASK_COLOUR)
	{
		p_file.write_lendian_t(I_BUTTON_MASK_COLOUR, p_abort);
		p_file.write_lendian_t(sizeof(m_mask_colour), p_abort);
		p_file.write_lendian_t(m_mask_colour, p_abort);
	}
}

// {AFD89390-8E1F-434c-B9C5-A4C1261BB792}
const GUID toolbar_extension::g_guid_fcb = 
{ 0xafd89390, 0x8e1f, 0x434c, { 0xb9, 0xc5, 0xa4, 0xc1, 0x26, 0x1b, 0xb7, 0x92 } };

const toolbar_extension::button toolbar_extension::g_button_null(pfc::guid_null, false, "", "", 0, ui_extension::MASK_NONE);

void toolbar_extension::reset_buttons(pfc::list_base_t<button> & p_buttons)
{
	p_buttons.remove_all();
	button temp = g_button_null;

	temp.m_type = TYPE_MENU_ITEM_MAIN;
	temp.m_show = SHOW_IMAGE;

	temp.m_guid = standard_commands::guid_main_stop;
	p_buttons.add_item(temp);
	temp.m_guid = standard_commands::guid_main_pause;
	p_buttons.add_item(temp);
	temp.m_guid = standard_commands::guid_main_play;
	p_buttons.add_item(temp);
	temp.m_guid = standard_commands::guid_main_previous;
	p_buttons.add_item(temp);
	temp.m_guid = standard_commands::guid_main_next;
	p_buttons.add_item(temp);
	temp.m_guid = standard_commands::guid_main_random;
	p_buttons.add_item(temp);
	temp.m_guid = pfc::guid_null;
	temp.m_type = TYPE_SEPARATOR;
	p_buttons.add_item(temp);
	temp.m_guid = standard_commands::guid_main_open;
	temp.m_type = TYPE_MENU_ITEM_MAIN;
	p_buttons.add_item(temp);
}

toolbar_extension::toolbar_extension() :  wnd_toolbar(0), initialised(false), wnd_host(0), m_text_below(false),
m_appearance(APPEARANCE_NORMAL), width(0), height(0), m_gdiplus_initialised(false)
{
	reset_buttons(m_buttons_config);
	memset(&m_gdiplus_instance, 0, sizeof(m_gdiplus_instance));
};

toolbar_extension::~toolbar_extension()
{
}

const TCHAR * toolbar_extension::class_name = _T("{D75D4E2D-603B-4699-9C49-64DDFFE56A16}");

void toolbar_extension::create_toolbar()
{
	m_buttons.add_items(m_buttons_config);

	pfc::array_t<TBBUTTON> tbb;
	tbb.set_size(m_buttons.get_count());

	pfc::array_t<button_image> images;
	images.set_size(m_buttons.get_count());

	pfc::array_t<button_image> images_hot;
	images_hot.set_size(m_buttons.get_count());

	memset(tbb.get_ptr(), 0, tbb.get_size()*sizeof(*tbb.get_ptr()));

	RECT rc;
	GetClientRect(wnd_host, &rc);

	wnd_toolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, 0, 
		WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT | (!m_text_below && m_appearance != APPEARANCE_NOEDGE ? TBSTYLE_LIST : 0) | TBSTYLE_TRANSPARENT |TBSTYLE_TOOLTIPS | CCS_NORESIZE| CCS_NOPARENTALIGN| CCS_NODIVIDER, 
		0, 0, rc.right, rc.bottom, wnd_host, (HMENU) ID_BUTTONS, core_api::get_my_instance(), NULL); 

	COLORREF colour_3dface = GetSysColor(COLOR_3DFACE);
	COLORREF colour_btntext = GetSysColor(COLOR_BTNTEXT);

	if (wnd_toolbar)
	{
		//			uSetWindowLong(p_this->wnd_toolbar,GWL_USERDATA,(LPARAM)(p_this));

		HIMAGELIST il = 0;
		HIMAGELIST iml_hot = 0;

		//libpng_handle::g_create(p_libpng);

		bool b_need_hot = false;

		unsigned n, count = tbb.get_size();
		for (n=0; n<count; n++)
		{
			if (m_buttons[n].m_use_custom_hot)
			{
				b_need_hot = true;
				break;
			}
		}

		SIZE sz = {0,0};

		bit_array_bittable mask(count);

		for (n=0; n<count; n++)
		{
			if (m_buttons[n].m_type != TYPE_SEPARATOR)
			{
				m_buttons[n].m_callback.set_wnd(this);
				m_buttons[n].m_callback.set_id(n);
				service_enum_t<ui_extension::button> e;
				service_ptr_t<ui_extension::button> ptr;
				while(e.next(ptr))
				{
					if (ptr->get_item_guid() == m_buttons[n].m_guid)
					{
						m_buttons[n].m_interface = ptr;
						break;
					}
				}

				if (m_buttons[n].m_show == SHOW_IMAGE || m_buttons[n].m_show == SHOW_IMAGE_TEXT)
				{
					if (m_buttons[n].m_use_custom_hot)
						images_hot[n].load(m_buttons[n].m_custom_hot_image);
					if (!m_buttons[n].m_use_custom)
					{
						if (m_buttons[n].m_interface.is_valid())
						{
							mask.set(n, true);
							//images[n].load(m_buttons[n].m_interface, colour_btntext);
						}
					}
					else
					{
						images[n].load(m_buttons[n].m_custom_image);

						SIZE szt;
						images[n].get_size(szt);
						sz.cx = max(sz.cx, szt.cx);
						sz.cy = max(sz.cy, szt.cy);
					}

				}
			}
		}
		if (sz.cx == 0 && sz.cy == 0)
		{
			sz.cx = GetSystemMetrics(SM_CXSMICON);
			sz.cy = GetSystemMetrics(SM_CYSMICON);
		}
		for (n=0; n<count; n++)
		{
			if (mask[n])
				images[n].load(m_buttons[n].m_interface, colour_btntext, sz.cx, sz.cy);
		}

		width = sz.cx;
		height = sz.cy;
		il = ImageList_Create(width, height, ILC_COLOR32|ILC_MASK, 7, 0);
		if (b_need_hot)
			iml_hot = ImageList_Create(width, height, ILC_COLOR32|ILC_MASK, 7, 0);

		//SendMessage(wnd_toolbar, TB_ADDSTRING,  NULL, (LPARAM)_T("\0")); //Add a empty string at index 0

		for (n=0; n<count; n++)
		{
			tbb[n].iString = -1; //"It works"

			if (m_buttons[n].m_type == TYPE_SEPARATOR)
			{
				tbb[n].idCommand = n; 
				tbb[n].fsState = TBSTATE_ENABLED; 
				tbb[n].fsStyle = BTNS_SEP ; 
			}
			else
			{
				m_buttons[n].m_callback.set_wnd(this);
				m_buttons[n].m_callback.set_id(n);
				if (m_buttons[n].m_show == SHOW_IMAGE || m_buttons[n].m_show == SHOW_IMAGE_TEXT)
				{
					tbb[n].iBitmap = images[n].add_to_imagelist(il);
					if (!m_buttons[n].m_use_custom_hot || !images_hot[n].is_valid())
						images[n].add_to_imagelist(iml_hot);
					else
						images_hot[n].add_to_imagelist(iml_hot);
				}
				else
					tbb[n].iBitmap = I_IMAGENONE;

				tbb[n].idCommand = n; 
				tbb[n].fsState = 0; 
				tbb[n].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON ; 
				if (!m_text_below && m_appearance != APPEARANCE_NOEDGE && (m_buttons[n].m_show == SHOW_TEXT || m_buttons[n].m_show == SHOW_IMAGE_TEXT))
					tbb[n].fsStyle |= BTNS_SHOWTEXT; 

				if ( /*m_text_below || (tbb[n].fsStyle & BTNS_SHOWTEXT) */ m_buttons[n].m_show == SHOW_TEXT || m_buttons[n].m_show == SHOW_IMAGE_TEXT)
				{
					pfc::string8 temp;
					m_buttons[n].get_display_text(temp);
					pfc::stringcvt::string_os_from_utf8 str_conv(temp);
					pfc::array_t<TCHAR, pfc::alloc_fast_aggressive> name;
					name.prealloc(str_conv.length()+4);
					name.append_fromptr(str_conv.get_ptr(), str_conv.length());
					name.append_single(0);
					name.append_single(0);
					tbb[n].iString = SendMessage(wnd_toolbar, TB_ADDSTRING,  NULL, (LPARAM)name.get_ptr());
				}
				
				if (m_buttons[n].m_interface.is_valid())
				{
					unsigned state = m_buttons[n].m_interface->get_button_state();
					if (m_buttons[n].m_interface->get_button_type() == uie::BUTTON_TYPE_DROPDOWN_ARROW)
						tbb[n].fsStyle |= BTNS_DROPDOWN;
					if (state & uie::BUTTON_STATE_ENABLED)
						tbb[n].fsState |= TBSTATE_ENABLED;
					if (state & uie::BUTTON_STATE_PRESSED)
						tbb[n].fsState |= TBSTATE_PRESSED;
					//m_buttons[n].m_interface->register_callback(m_buttons[n].m_callback);
				}
				else
				{
					tbb[n].fsState |= TBSTATE_ENABLED;
				}
			}
		}
		//p_libpng.release();

		unsigned ex_style = SendMessage(wnd_toolbar, TB_GETEXTENDEDSTYLE, 0, 0);
		SendMessage(wnd_toolbar, TB_SETEXTENDEDSTYLE, 0, ex_style | TBSTYLE_EX_DRAWDDARROWS | (!m_text_below ? TBSTYLE_EX_MIXEDBUTTONS : 0));

		SendMessage(wnd_toolbar, TB_SETBITMAPSIZE, (WPARAM) 0, MAKELONG(width,height));
		//SendMessage(wnd_toolbar, TB_SETBUTTONSIZE, (WPARAM) 0, MAKELONG(width,height));

		//todo: custom padding
		unsigned padding = SendMessage(wnd_toolbar, TB_GETPADDING, (WPARAM) 0, 0);
		if (m_appearance == APPEARANCE_NOEDGE)
		{
			SendMessage(wnd_toolbar, TB_SETPADDING, (WPARAM) 0, MAKELPARAM(0,0));
			DLLVERSIONINFO2 dvi;
			HRESULT hr = g_get_comctl32_version(dvi);
			if (SUCCEEDED(hr) && dvi.info1.dwMajorVersion >= 6)
			{
				/*
				HTHEME thm;
				uxtheme_api_ptr p_uxtheme;
				uxtheme_handle::g_create(p_uxtheme);
				thm = p_uxtheme->OpenThemeData(wnd_toolbar, L"Toolbar");
				MARGINS mg;
				p_uxtheme->GetThemeMargins(thm, NULL, 1, 1, 3602, NULL, &mg);
				p_uxtheme->CloseThemeData(thm);
				TBMETRICS temp;
				memset(&temp, 0, sizeof(temp));
				temp.cbSize = sizeof(temp);
				temp.dwMask = TBMF_BUTTONSPACING;
				temp.cxButtonSpacing =-4;
				temp.cyButtonSpacing=0;
				//SendMessage(wnd_toolbar, TB_SETMETRICS, 0, (LPARAM)&temp);
				temp.dwMask = TBMF_BUTTONSPACING|TBMF_BARPAD|TBMF_PAD;*/
			}
		}
		else if (m_appearance == APPEARANCE_FLAT)
			SendMessage(wnd_toolbar, TB_SETPADDING, (WPARAM) 0, MAKELPARAM(5,HIWORD(padding)));

		if (il)
			SendMessage(wnd_toolbar, TB_SETIMAGELIST, (WPARAM) 0, (LPARAM) il);
		if (iml_hot)
			SendMessage(wnd_toolbar, TB_SETHOTIMAGELIST, (WPARAM) 0, (LPARAM) iml_hot);

		SendMessage(wnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 

		SendMessage(wnd_toolbar, TB_ADDBUTTONS, (WPARAM) tbb.get_size(), (LPARAM) tbb.get_ptr());

		for (n=0; n<count; n++)
			if (m_buttons[n].m_interface.is_valid()) m_buttons[n].m_interface->register_callback(m_buttons[n].m_callback);

		ShowWindow(wnd_toolbar, SW_SHOWNORMAL);
		SendMessage(wnd_toolbar, TB_AUTOSIZE, 0, 0);
	}
}

void toolbar_extension::destroy_toolbar()
{
	t_size i, count = m_buttons.get_count();
	for (i=0; i<count; i++)
		if (m_buttons[i].m_interface.is_valid()) m_buttons[i].m_interface->deregister_callback(m_buttons[i].m_callback);
	HIMAGELIST iml = (HIMAGELIST)SendMessage(wnd_toolbar, TB_GETIMAGELIST, (WPARAM) 0, (LPARAM) 0);
	HIMAGELIST iml_hot = (HIMAGELIST)SendMessage(wnd_toolbar, TB_GETHOTIMAGELIST, (WPARAM) 0, (LPARAM) 0);
	DestroyWindow(wnd_toolbar);
	wnd_toolbar=0;
	if (iml)
		ImageList_Destroy(iml);
	if (iml_hot)
		ImageList_Destroy(iml_hot);
	m_buttons.remove_all();
}

LRESULT toolbar_extension::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	if(msg == WM_CREATE)
	{
		wnd_host=wnd;
		m_gdiplus_initialised = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplus_instance, &Gdiplus::GdiplusStartupInput(), NULL));
		initialised=true;
		create_toolbar();

	}
	else if (msg == WM_DESTROY)
	{
		destroy_toolbar();
		wnd_host=0;
		initialised=false;
		if (m_gdiplus_initialised)
		{
			Gdiplus::GdiplusShutdown(m_gdiplus_instance);
			m_gdiplus_initialised = false;
		}
	}
	else if (msg == WM_WINDOWPOSCHANGED)
	{
		LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
		if (!(lpwp->flags & SWP_NOSIZE))
		{
			//SIZE sz = {0,0};
			//SendMessage(wnd_menu, TB_GETMAXSIZE, NULL, (LPARAM)&sz);

			RECT rc = {0,0,0,0};
			t_size count = m_buttons.get_count();
			int cx = lpwp->cx;
			int cy = lpwp->cy;
			int extra = 0;
			if (count && (BOOL)SendMessage(wnd_toolbar, TB_GETITEMRECT, count - 1, (LPARAM)(&rc)))
			{
				cx = min(cx, rc.right);
				cy = min(cy, rc.bottom);
				extra = (lpwp->cy - rc.bottom)/2;
			}
			SetWindowPos(wnd_toolbar, 0, 0, extra, cx, cy, SWP_NOZORDER);
		}
	}
	else if (msg==WM_SIZE)
	{
	}
	else if (msg == WM_GETMINMAXINFO)
	{
		if ( m_buttons.get_count() )
		{
			LPMINMAXINFO mmi = LPMINMAXINFO(lp);

			RECT rc = {0,0,0,0};

			if (SendMessage(wnd_toolbar, TB_GETITEMRECT, m_buttons.get_count() - 1, (LPARAM)(&rc)))
			{
				mmi->ptMinTrackSize.x = rc.right;
				mmi->ptMinTrackSize.y = rc.bottom;
				mmi->ptMaxTrackSize.y = rc.bottom;
				return 0;
			}
		}
	}

	else if (msg == WM_USER + 2)
	{
		if (wnd_toolbar && wp < m_buttons.get_count() &&  m_buttons[wp].m_interface.is_valid())
		{
			unsigned state = m_buttons[wp].m_interface->get_button_state(), tbstate = 0;
			if (state & uie::BUTTON_STATE_PRESSED)
			{
				PostMessage(wnd_toolbar, TB_PRESSBUTTON, wp, MAKELONG(TRUE,0));
			}
		}
	}

	else if (msg== WM_NOTIFY && ((LPNMHDR)lp)->idFrom == ID_BUTTONS)
	{
		switch (((LPNMHDR)lp)->code)
		{
		case TBN_ENDDRAG:
			{
				LPNMTOOLBAR lpnmtb = (LPNMTOOLBAR)lp;
				PostMessage(wnd, WM_USER+2, lpnmtb->iItem, NULL);
			}
			break;
		case TBN_GETINFOTIP:
			{
				LPNMTBGETINFOTIP lpnmtbgit = (LPNMTBGETINFOTIP) lp;
				if (!m_buttons[lpnmtbgit->iItem].m_interface.is_valid() || (m_buttons[lpnmtbgit->iItem].m_interface->get_button_state() & uie::BUTTON_STATE_SHOW_TOOLTIP))
				{
					pfc::string8 temp;
					m_buttons[lpnmtbgit->iItem].get_short_name(temp);
					StringCchCopy(lpnmtbgit->pszText,lpnmtbgit->cchTextMax,pfc::stringcvt::string_wide_from_utf8(temp));
				}
			}
			break;
		case TBN_DROPDOWN:
			{
				LPNMTOOLBAR lpnmtb = (LPNMTOOLBAR)lp;
				pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> menu_items = new ui_extension::menu_hook_impl;

				m_buttons[lpnmtb->iItem].m_interface->get_menu_items(*menu_items.get_ptr());
				HMENU menu = CreatePopupMenu();
				menu_items->win32_build_menu(menu, 1, pfc_infinite);
				POINT pt = {lpnmtb->rcButton.left, lpnmtb->rcButton.bottom};
				MapWindowPoints(lpnmtb->hdr.hwndFrom, HWND_DESKTOP, &pt, 1);
				int cmd = TrackPopupMenuEx(menu, TPM_LEFTBUTTON|TPM_RETURNCMD, pt.x, pt.y, wnd, 0);
				if (cmd)
					menu_items->execute_by_id(cmd);
				DestroyMenu(menu);
				
				return TBDDRET_DEFAULT;
			}
		case NM_CUSTOMDRAW:
			{
				LPNMTBCUSTOMDRAW lptbcd = (LPNMTBCUSTOMDRAW) lp;
				switch ((lptbcd)->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					return (CDRF_NOTIFYITEMDRAW);
				case CDDS_ITEMPREPAINT:
					{
						if (m_appearance != APPEARANCE_NOEDGE && !m_text_below && lptbcd->nmcd.dwItemSpec >= 0 && lptbcd->nmcd.dwItemSpec<m_buttons.get_count() && m_buttons[lptbcd->nmcd.dwItemSpec].m_show == SHOW_TEXT)
						{
							DLLVERSIONINFO2 dvi;
							HRESULT hr = g_get_comctl32_version(dvi);
							if (SUCCEEDED(hr) && dvi.info1.dwMajorVersion >= 6)
								lptbcd->rcText.left-=LOWORD(SendMessage(wnd_toolbar, TB_GETPADDING, (WPARAM) 0, 0)) + 2;  //Hack for commctrl6
						}
						if (m_appearance == APPEARANCE_FLAT)
						{
							LRESULT rv = TBCDRF_NOEDGES|TBCDRF_NOOFFSET;
							if (lptbcd->nmcd.uItemState & CDIS_HOT)
							{
								lptbcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
							}
							else
							{
							}
							lptbcd->clrHighlightHotTrack = GetSysColor(COLOR_HIGHLIGHT);
							rv |=TBCDRF_HILITEHOTTRACK;
							return rv;
						}
						else if (m_appearance == APPEARANCE_NOEDGE)
						{
							return TBCDRF_NOEDGES|TBCDRF_NOBACKGROUND;
						}
					}
					break;
				}
			}
			break;
		}
	}
	else if (msg == WM_COMMAND)
	{
		if (wp >=0 && wp < m_buttons.get_count())
		{
			GUID caller = pfc::guid_null;
			metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
			switch (m_buttons[wp].m_filter)
			{
			case FILTER_PLAYLIST:
				{
					static_api_ptr_t<playlist_manager> api;
					data.prealloc(api->activeplaylist_get_selection_count(pfc_infinite));
					api->activeplaylist_get_selected_items(data);
					caller = contextmenu_item::caller_active_playlist_selection;
				}
				break;
			case FILTER_ACTIVE_SELECTION:
				{
					static_api_ptr_t<ui_selection_manager> api;
					if (api->get_selection_type() != contextmenu_item::caller_now_playing)
					{
						api->get_selection(data);
					}
					caller = contextmenu_item::caller_undefined;
				}
				break;
			case FILTER_PLAYING:
				{
					metadb_handle_ptr hdle;
					if (static_api_ptr_t<play_control>()->get_now_playing(hdle))
					data.add_item(hdle);
					caller = contextmenu_item::caller_now_playing;
				}
				break;
			}

			switch (m_buttons[wp].m_type)
			{
			case TYPE_MENU_ITEM_CONTEXT:
				menu_helpers::run_command_context_ex(m_buttons[wp].m_guid, m_buttons[wp].m_subcommand, data, caller);
				break;
			case TYPE_MENU_ITEM_MAIN:
				if (m_buttons[wp].m_subcommand != pfc::guid_null)
					mainmenu_commands::g_execute_dynamic(m_buttons[wp].m_guid, m_buttons[wp].m_subcommand);
				else
					mainmenu_commands::g_execute(m_buttons[wp].m_guid);
				break;
			case TYPE_BUTTON:
				{
					service_ptr_t<uie::custom_button> p_button;
					if (m_buttons[wp].m_interface.is_valid() && m_buttons[wp].m_interface->service_query_t(p_button))
						p_button->execute(data);
				}
				break;
			}
		}
		else
			console::print("buttons toolbar: error index out of range!");
	}
	else if (msg == WM_CONTEXTMENU)
	{
		if (HWND(wp) == wnd_toolbar)
		{
			if (lp != -1)
			{
				POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
				POINT pts = pt;
				ScreenToClient(wnd_toolbar, &pt);
				int lresult = SendMessage(wnd_toolbar, TB_HITTEST, 0, (LPARAM)&pt);
				if (lresult >= 0 && //not a separator
					(unsigned)lresult < m_buttons.get_count() && //safety
					m_buttons[lresult].m_interface.is_valid())

				{
					pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> menu_items = new ui_extension::menu_hook_impl;

					m_buttons[lresult].m_interface->get_menu_items(*menu_items.get_ptr());
					if (menu_items->get_children_count())
					{
						HMENU menu = CreatePopupMenu();
						menu_items->win32_build_menu(menu, 1, pfc_infinite);
						int cmd = TrackPopupMenuEx(menu, TPM_LEFTBUTTON|TPM_RETURNCMD, pts.x, pts.y, wnd, 0);
						if (cmd)
							menu_items->execute_by_id(cmd);
						DestroyMenu(menu);
						return 0;
					}

				}
			}
		}
	}

	return uDefWindowProc(wnd, msg, wp, lp);
}

void toolbar_extension::get_name(pfc::string_base & out)const
{
	out.set_string("Buttons");
}
void toolbar_extension::get_category(pfc::string_base & out)const
{
	out.set_string("Toolbars");
}



void toolbar_extension::get_config(stream_writer * out, abort_callback & p_abort) const
{
		unsigned n,count = m_buttons_config.get_count();
		out->write_lendian_t(VERSION_CURRENT, p_abort);
		out->write_lendian_t(m_text_below, p_abort);
		out->write_lendian_t(m_appearance, p_abort);
		out->write_lendian_t(count, p_abort);
		for (n=0; n<count; n++)
		{
			m_buttons_config[n].write(out, p_abort);
		}
}


void toolbar_extension::set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort)
{
	if (p_size)
	{
		t_config_version p_version;
		unsigned n,count = m_buttons_config.get_count();
		p_reader->read_lendian_t(p_version, p_abort);
		if (p_version <= VERSION_CURRENT)
		{
			p_reader->read_lendian_t(m_text_below, p_abort);
			p_reader->read_lendian_t(m_appearance, p_abort);
			p_reader->read_lendian_t(count, p_abort);
			m_buttons_config.remove_all();
			for (n=0; n<count; n++)
			{
				button temp;
				temp.read(p_version, p_reader, p_abort);
				m_buttons_config.add_item(temp);
			}
		}
	}
}

enum {
	MSG_BUTTON_CHANGE = WM_USER + 2,
	MSG_COMMAND_CHANGE = WM_USER + 3
};

class command_picker_param
{
public:
	types::t_guid m_guid, m_subcommand;
	unsigned m_group;
	unsigned m_filter;
	command_picker_param() {};
	command_picker_param (types::t_guid p_guid, types::t_guid p_subcommand, unsigned p_group, unsigned p_filter)
		: m_guid(p_guid), m_subcommand(p_subcommand), m_group(p_group), m_filter(p_filter) {};
};

class command_picker_data
{
	modal_dialog_scope m_scope;
	class command_data
	{
	public:
		types::t_guid m_guid;
		types::t_guid m_subcommand;
		pfc::string8 m_desc;
	};
	ptr_list_autodel_t<command_data> m_data;
	HWND m_wnd;
	HWND wnd_group;
	HWND wnd_filter;
	HWND wnd_command;
	unsigned m_group;
	types::t_guid m_guid;
	types::t_guid m_subcommand;
	unsigned m_filter;
	bool __populate_mainmenu_dynamic_recur(command_data & data, const mainmenu_node::ptr & ptr_node, pfc::string_base & full, bool b_root)
	{
		if (ptr_node.is_valid())
		{
			switch (ptr_node->get_type())
			{
			case mainmenu_node::type_command:
				{
					pfc::string8 subfull = full,subname;
					t_uint32 flags;
					ptr_node->get_display(subname, flags);

					if (subfull.length() && subfull.get_ptr()[subfull.length()-1] != '/')
						subfull.add_byte('/');
					subfull.add_string(subname);

					command_data * p_data = new command_data (data);
					p_data->m_subcommand = ptr_node->get_guid();
					ptr_node->get_description(p_data->m_desc);

					m_data.add_item(p_data);

					unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, subfull);
					SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
				}
				return true;
			case mainmenu_node::type_group:
				{

					pfc::string8 name, subfull=full;
					if (!b_root)
					{
						t_uint32 flags;
						ptr_node->get_display(name, flags);
						if (subfull.length() && subfull.get_ptr()[subfull.length()-1] != '/')
							subfull.add_byte('/');
						subfull.add_string(name);
					}

					mainmenu_node::ptr ptr_child;
					for (t_size i = 0, count = ptr_node->get_children_count(); i<count; i++)
					{
						ptr_child = ptr_node->get_child(i);
						__populate_mainmenu_dynamic_recur(data, ptr_child, subfull, false);
					}
				}
				return true;
			default:
				return false;
			};
		}
		return false;
	}
	bool __populate_commands_recur(command_data & data, pfc::string_base & full, contextmenu_item_node * p_node, bool b_root)
	{
		if (p_node)
		{
			if (p_node->get_type() == contextmenu_item_node::TYPE_POPUP)
			{
				pfc::string8 name, subfull=full;
				unsigned dummy;
				p_node->get_display_data(name, dummy, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list);
				if (subfull.length() && subfull.get_ptr()[subfull.length()-1] != '/')
					subfull.add_byte('/');
				subfull.add_string(name);

				unsigned child, child_count = p_node->get_children_count();
				for (child=0;child<child_count;child++)
				{
					contextmenu_item_node * p_child = p_node->get_child(child);
					__populate_commands_recur(data, subfull, p_child, false);
				}
				return true;
			}
			else if (p_node->get_type() == contextmenu_item_node::TYPE_COMMAND && !b_root)
			{
				pfc::string8 subfull = full,subname;
				unsigned dummy;
				p_node->get_display_data(subname, dummy, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list);
				if (subfull.length() && subfull.get_ptr()[subfull.length()-1] != '/')
					subfull.add_byte('/');
				subfull.add_string(subname);

				command_data * p_data = new command_data (data);
				p_data->m_subcommand = p_node->get_guid();
				p_node->get_description(p_data->m_desc);

				m_data.add_item(p_data);

				unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, subfull);
				SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
				return true;
			}
		}
		return false;
	}
	void populate_commands()
	{
		SendMessage(wnd_command, LB_RESETCONTENT, 0, 0);
		m_data.delete_all();
		SendMessage(wnd_command, WM_SETREDRAW, FALSE, 0);
		if (m_group == 2)
		{
			service_enum_t<contextmenu_item> e;
			service_ptr_t<contextmenu_item> ptr;

			unsigned p_item_index=0,p_service_item_index;//,n=0;
			while(e.next(ptr))
			{
				{
					unsigned p_service_item_count = ptr->get_num_items();
					for (p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++)
					{
						pfc::ptrholder_t<contextmenu_item_node_root> p_node (ptr->instantiate_item(p_service_item_index, metadb_handle_list(), contextmenu_item::caller_keyboard_shortcut_list));

						command_data data;
						data.m_guid = ptr->get_item_guid(p_service_item_index);

						pfc::string8 name,full;
						ptr->get_item_default_path(p_service_item_index,full);

						if (p_node.is_valid() && __populate_commands_recur(data, full, p_node.get_ptr(), true))
						{
						}
						else
						{
							ptr->get_item_name(p_service_item_index, name);
							if (full.length() && full[full.length()-1] != '/')
								full.add_byte('/');
							full.add_string(name);

							command_data * p_data = new command_data (data);
							ptr->get_item_description(p_service_item_index, p_data->m_desc);
							m_data.add_item(p_data);

							unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, full);
							SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
//							n++;
						}
					}
				}
			}
		}
		else if (m_group == 3)
		{
			service_enum_t<mainmenu_commands> e;
			service_ptr_t<mainmenu_commands> ptr;

			unsigned p_item_index=0,p_service_item_index;//,n=0;
			while(e.next(ptr))
			{
				service_ptr_t<mainmenu_commands_v2> ptr_v2;
				ptr->service_query_t(ptr_v2);
				{
					unsigned p_service_item_count = ptr->get_command_count();
					for (p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++)
					{
						command_data data;
						data.m_guid = ptr->get_command(p_service_item_index);
						pfc::string8 name,full;
						ptr->get_name(p_service_item_index, name);
						{
							pfc::list_t<pfc::string8> levels;
							GUID parent = ptr->get_parent();
							while (parent != pfc::guid_null)
							{
								pfc::string8 parentname;
								if (menu_helpers::maingroupname_from_guid(GUID(parent), parentname, parent))
									levels.insert_item(parentname, 0);
							}
							unsigned i, count = levels.get_count();
							for (i=0; i<count; i++)
							{
								full.add_string(levels[i]);
								full.add_byte('/');

							}
						}
						full.add_string(name);

						if (ptr_v2.is_valid() && ptr_v2->is_command_dynamic(p_service_item_index))
						{
							mainmenu_node::ptr ptr_node = ptr_v2->dynamic_instantiate(p_service_item_index);
							__populate_mainmenu_dynamic_recur(data, ptr_node, full, true);
						}
						else
						{
							command_data * p_data = new command_data (data);
							ptr->get_description(p_service_item_index, p_data->m_desc);
							m_data.add_item(p_data);
							unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, full);
							SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
						}
					}
				}
			}
		}
		else if (m_group == 1)
		{
			service_enum_t<uie::button> e;
			service_ptr_t<uie::button> ptr;
			while(e.next(ptr))
			{
				service_ptr_t<uie::custom_button> p_button;
				if (ptr->get_guid_type() == uie::BUTTON_GUID_BUTTON && ptr->service_query_t(p_button))
				{
					command_data * p_data = new command_data;
					p_data->m_guid = ptr->get_item_guid();
					p_button->get_description(p_data->m_desc);
					m_data.add_item(p_data);
					pfc::string8 temp;
					p_button->get_name(temp);
					unsigned idx = uSendMessageText(wnd_command, LB_ADDSTRING, 0, temp);
					SendMessage(wnd_command, LB_SETITEMDATA, idx, (LPARAM)p_data);
				}
			}
		}
		unsigned n, count = SendMessage(wnd_command, LB_GETCOUNT, 0,0);
		for (n=0; n<count; n++)
		{
			LRESULT ret = SendMessage(wnd_command, LB_GETITEMDATA, n, 0);
			command_data* p_data = ((command_data*)ret);

			if (ret != LB_ERR && p_data->m_guid == m_guid && p_data->m_subcommand == m_subcommand)
			{
				SendMessage(wnd_command, LB_SETCURSEL, n, 0);
				update_description();
				break;
			}
		}
		SendMessage(wnd_command, WM_SETREDRAW, TRUE, 0);
	}
	void update_description()
	{
		LRESULT p_command = SendMessage(wnd_command, LB_GETCURSEL,0,0);
		if (p_command != LB_ERR)
		{
			LRESULT p_data = SendMessage(wnd_command, LB_GETITEMDATA, p_command, 0);
			if (p_data != LB_ERR)
				uSendDlgItemMessageText(m_wnd,IDC_DESC, WM_SETTEXT, 0, ((command_data*)p_data)->m_desc); 
			else
				uSendDlgItemMessageText(m_wnd,IDC_DESC, WM_SETTEXT, 0, ""); 
		}
		else
			uSendDlgItemMessageText(m_wnd,IDC_DESC, WM_SETTEXT, 0, ""); 
	}
public:
	void set_data(const command_picker_param & p_data)
	{
		m_group = p_data.m_group;
		m_guid = p_data.m_guid;
		m_subcommand = p_data.m_subcommand;
		m_filter = p_data.m_filter;
	}
	void get_data(command_picker_param & p_data) const
	{
		p_data.m_group = m_group;
		p_data.m_guid = m_guid;
		p_data.m_subcommand = m_subcommand;
		p_data.m_filter = m_filter;
	}
	void initialise(HWND wnd)
	{
		m_wnd = wnd;
		wnd_group = GetDlgItem(wnd, IDC_GROUP);
		wnd_filter = GetDlgItem(wnd, IDC_ITEM);
		wnd_command = GetDlgItem(wnd, IDC_COMMAND);

		SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM)_T("Separator"));
		SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM)_T("Buttons"));
		SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM)_T("Shortcut menu items"));
		SendMessage(wnd_group, LB_ADDSTRING, 0, (LPARAM)_T("Main menu items"));

		SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM)_T("None"));
		SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM)_T("Now playing item"));
		SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM)_T("Current playlist selection"));
		SendMessage(wnd_filter, LB_ADDSTRING, 0, (LPARAM)_T("Active selection"));

		SendMessage(wnd_group, LB_SETCURSEL, m_group,0);

	}
	void deinitialise(HWND wnd)
	{
		SendMessage(wnd_group, LB_RESETCONTENT, 0, 0);
		SendMessage(wnd_filter, LB_RESETCONTENT, 0, 0);
		SendMessage(wnd_command, LB_RESETCONTENT, 0, 0);
		m_data.delete_all();
	}
	BOOL on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				m_scope.initialize(FindOwningPopup(wnd));
				initialise(wnd);
				populate_commands();
				SendMessage(wnd_filter, LB_SETCURSEL, (WPARAM)m_filter, 0);
			}
			return TRUE;
		case WM_DESTROY:
			deinitialise(wnd);
			return TRUE;
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
		case WM_COMMAND:
			switch(wp)
			{
			case IDC_GROUP|(LBN_SELCHANGE<<16):
				m_group = SendMessage(wnd_group, LB_GETCURSEL,0,0);
				m_guid.reset();
				m_subcommand.reset();
				populate_commands();
				return TRUE;
			case IDC_ITEM|(LBN_SELCHANGE<<16):
				{
					LRESULT p_filter = SendMessage(wnd_filter, LB_GETCURSEL,0,0);
					if (p_filter != LB_ERR)
						m_filter = p_filter;
				}
				return TRUE;
			case IDC_COMMAND|(LBN_SELCHANGE<<16):
				{
					m_guid.reset();
					m_subcommand.reset();

					LRESULT p_command = SendMessage(wnd_command, LB_GETCURSEL,0,0);
					if (p_command != LB_ERR)
					{
						LRESULT ret = SendMessage(wnd_command, LB_GETITEMDATA, p_command, 0);
						command_data* p_data = (command_data*)ret;
						if (ret != LB_ERR)
						{
							m_guid = p_data->m_guid;
							m_subcommand = p_data->m_subcommand;
						}
					}
					update_description();
				}
				return TRUE;
			case IDCANCEL:
				{
					EndDialog(wnd, 0);
				}
				return TRUE;
			case IDOK:
				{
					EndDialog(wnd,1);
				}
				return TRUE;
			}
			break;
		}
		return FALSE;
	}
};

void toolbar_extension::config_param::on_selection_change(t_size index)
{
	m_selection = index != pfc_infinite && index < m_buttons.get_count() ? &m_buttons[index] : NULL;
	m_image = m_selection ? (m_active ? &m_selection->m_custom_hot_image : &m_selection->m_custom_image) : 0;
	pfc::string8 temp;
	if (m_selection)
	{
		m_selection->get_name(temp);
	}
	uSendDlgItemMessageText(m_wnd, IDC_COMMAND_DESC, WM_SETTEXT, 0, temp);
	SendDlgItemMessage(m_wnd, IDC_SHOW, CB_SETCURSEL, m_selection ? m_selection->m_show : -1, 0);

	bool b_enable = index != pfc_infinite && m_selection && m_selection->m_type != TYPE_SEPARATOR;
	Button_SetCheck(GetDlgItem(m_wnd, IDC_USE_CUSTOM_TEXT), b_enable ? m_selection->m_use_custom_text : FALSE);
	SendDlgItemMessage(m_wnd, IDC_TEXT, WM_SETTEXT, 0, b_enable ? (LPARAM)pfc::stringcvt::string_os_from_utf8(m_selection->m_text).get_ptr() : (LPARAM)_T(""));
	EnableWindow(GetDlgItem(m_wnd, IDC_PICK), index != pfc_infinite);
	EnableWindow(GetDlgItem(m_wnd, IDC_SHOW), b_enable);
	EnableWindow(GetDlgItem(m_wnd, IDC_USE_CUSTOM_TEXT), b_enable);
	EnableWindow(GetDlgItem(m_wnd, IDC_TEXT), b_enable && m_selection->m_use_custom_text);
	SendMessage(m_child, MSG_BUTTON_CHANGE, 0, 0);

}

void toolbar_extension::config_param::populate_buttons_list()
{
	unsigned n, count = m_buttons.get_count();

	pfc::string8_fast_aggressive name;
	pfc::array_staticsize_t<t_list_view::t_item_insert> items(count);
	for(n=0; n<count; n++)
	{
		m_buttons[n].get_name_name(name);
		items[n].m_subitems.add_item(name);
		m_buttons[n].get_name_type(name);
		items[n].m_subitems.add_item(name);
	}
	m_button_list.insert_items(0, count, items.get_ptr());
}

void toolbar_extension::config_param::refresh_buttons_list_items(t_size index, t_size count, bool b_update_display)
{
	unsigned n, real_count = m_buttons.get_count();

	if (index+count > real_count) count = real_count-index;

	pfc::string8_fast_aggressive name;
	pfc::list_t<t_list_view::t_item_insert> items;
	items.set_count(count);
	for(n=index; n<index+count; n++)
	{
		m_buttons[n].get_name_name(name);
		items[n-index].m_subitems.add_item(name);
		m_buttons[n].get_name_type(name);
		items[n-index].m_subitems.add_item(name);
	}
	m_button_list.replace_items(index, items, b_update_display);
}

BOOL CALLBACK toolbar_extension::config_param::g_ConfigPopupProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	config_param * ptr = NULL;
	switch(msg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(wnd,DWL_USER,lp);
		ptr = reinterpret_cast<config_param*>(lp);
		break;
	default:
		ptr = reinterpret_cast<config_param*>(GetWindowLongPtr(wnd,DWL_USER));
		break;
	};
	return ptr ? ptr->ConfigPopupProc(wnd, msg, wp, lp) : FALSE;
}


BOOL toolbar_extension::config_param::ConfigPopupProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			m_wnd = wnd;
			m_scope.initialize(FindOwningPopup(wnd));

			HWND wnd_show = GetDlgItem(wnd, IDC_SHOW);

			SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM)_T("Image"));
			SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM)_T("Image and text"));
			SendMessage(wnd_show, CB_ADDSTRING, 0, (LPARAM)_T("Text"));

			HWND wnd_text = GetDlgItem(wnd, IDC_TEXT_LOCATION);

			SendMessage(wnd_text, CB_ADDSTRING, 0, (LPARAM)_T("Right"));
			SendMessage(wnd_text, CB_ADDSTRING, 0, (LPARAM)_T("Below"));

			HWND wnd_app = GetDlgItem(wnd, IDC_APPEARANCE);

			SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM)_T("Normal"));
			SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM)_T("Flat"));
			SendMessage(wnd_app, CB_ADDSTRING, 0, (LPARAM)_T("No edges"));

			HWND wnd_tab = GetDlgItem(wnd, IDC_TAB);
			uTabCtrl_InsertItemText(wnd_tab, 0, "Normal image");
			uTabCtrl_InsertItemText(wnd_tab, 1, "Hot image");

			RECT tab;
			
			GetWindowRect(wnd_tab,&tab);
			MapWindowPoints(HWND_DESKTOP, wnd, (LPPOINT)&tab, 2);
			
			TabCtrl_AdjustRect(wnd_tab,FALSE,&tab);

			m_child = CreateDialogParam(core_api::get_my_instance(), MAKEINTRESOURCE(IDD_IMAGE),wnd,ConfigChildProc, (LPARAM)this);

			SetWindowPos(m_child,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			
			if (m_child) 
			{
				{
					EnableThemeDialogTexture(m_child, ETDT_ENABLETAB);
				}
			}

			SetWindowPos(m_child, 0, tab.left, tab.top, tab.right-tab.left, tab.bottom-tab.top, SWP_NOZORDER);
			//SetWindowPos(wnd_tab,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			
			ShowWindow(m_child, SW_NORMAL);
			UpdateWindow(m_child);

			SendMessage(wnd_text, CB_SETCURSEL, m_text_below?1:0, 0);
			SendMessage(wnd_app, CB_SETCURSEL, m_appearance, 0);

			HWND wnd_button_list = m_button_list.create_in_dialog_units(wnd, ui_helpers::window_position_t(14, 16, 310, 106));
			populate_buttons_list();
			ShowWindow(wnd_button_list, SW_SHOWNORMAL);

		}
		return TRUE;
	case WM_DESTROY:
		m_button_list.destroy();
		break;
#if 0
	case WM_PAINT:
		ui_helpers::innerWMPaintModernBackground(wnd, GetDlgItem(wnd, IDOK));
		return TRUE;
	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
		SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
		return (BOOL)GetSysColorBrush(COLOR_WINDOW);
#endif
	//case WM_ERASEBKGND:
	//	SetWindowLongPtr(wnd, DWL_MSGRESULT, FALSE);
	//	return TRUE;
	//case WM_PAINT:
	//	ui_helpers::innerWMPaintModernBackground(wnd, GetDlgItem(wnd, IDOK));
	//	return TRUE;
	//case WM_CTLCOLORBTN:
	//case WM_CTLCOLORDLG:
	//case WM_CTLCOLORSTATIC:
		/*SetBkColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
		SetDCBrushColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
		SetDCPenColor((HDC)wp, GetSysColor(COLOR_3DDKSHADOW));
		SetBkMode((HDC)wp, TRANSPARENT);
		//SetROP2((HDC)wp, R2_BLACK);
		SelectBrush((HDC)wp, GetSysColorBrush(COLOR_3DDKSHADOW));
		return (BOOL)GetSysColorBrush(COLOR_3DDKSHADOW);*/
		//return FALSE;
	case WM_COMMAND:
		switch(wp)
		{
		case IDCANCEL:
			{
				EndDialog(wnd, 0);
			}
			return TRUE;
		case (CBN_SELCHANGE<<16)|IDC_SHOW:
			{
				if (m_selection)
				{
					m_selection->m_show = (t_show)SendMessage((HWND)lp, CB_GETCURSEL, 0,0);
				}
			}
			break;
		case (CBN_SELCHANGE<<16)|IDC_TEXT_LOCATION:
			{
				m_text_below = SendMessage((HWND)lp, CB_GETCURSEL, 0,0) != 0;
			}
			break;
		case (CBN_SELCHANGE<<16)|IDC_APPEARANCE:
			{
				m_appearance = (t_appearance)SendMessage((HWND)lp, CB_GETCURSEL, 0,0);
			}
			break;
		case IDC_ADD:
			{
				command_picker_data p_temp;
				command_picker_param p_data(g_button_null.m_guid, g_button_null.m_subcommand, 
					g_button_null.m_type, g_button_null.m_filter);
				p_temp.set_data(p_data);
				if (uDialogBox(IDD_COMMAND,wnd,ConfigCommandProc,reinterpret_cast<LPARAM>(&p_temp)))
				{
					t_size index = m_buttons.add_item(g_button_null);
					
					p_temp.get_data(p_data);
					m_buttons[index].m_type = (t_type)p_data.m_group;
					m_buttons[index].m_guid = p_data.m_guid;
					m_buttons[index].m_subcommand = p_data.m_subcommand;
					m_buttons[index].m_filter = (t_filter)p_data.m_filter;

					pfc::string8_fast_aggressive name;
					//m_buttons[index].get_name(name);
					//unsigned idx = uSendDlgItemMessageText(wnd, IDC_BUTTON_LIST, LB_ADDSTRING, 0, name);

					t_list_view::t_item_insert item;
					m_buttons[index].get_name_name(name);
					item.m_subitems.add_item(name);
					m_buttons[index].get_name_type(name);
					item.m_subitems.add_item(name);
					t_size index_list = m_button_list.get_item_count();
					m_button_list.insert_items(index_list, 1, &item);
					m_button_list.set_item_selected_single(index_list);
					m_button_list.ensure_visible(index_list);

					//SendDlgItemMessage(wnd, IDC_BUTTON_LIST, LB_SETCURSEL, idx, 0);
					//SendMessage(wnd, WM_COMMAND, (LBN_SELCHANGE<<16)|IDC_BUTTON_LIST, (LPARAM)GetDlgItem(wnd,IDC_BUTTON_LIST));
				}
			}
			break;
		case IDC_RESET:
			{
				if (win32_helpers::message_box(wnd, _T("This will reset all your buttons to the default buttons. Continue?"),_T("Reset buttons"),MB_YESNO) == IDYES)
				{
					m_button_list.remove_items(bit_array_true(), false);
					toolbar_extension::reset_buttons(m_buttons);
					populate_buttons_list();
				}
			}
			break;
		case IDC_REMOVE:
			{
				t_size index = m_button_list.get_selected_item_single();
				if (index != pfc_infinite)
				{
					m_button_list.remove_item(index);
					m_buttons.remove_by_idx(index);
					if (index < m_button_list.get_item_count())
						m_button_list.set_item_selected_single(index);
					else if (index)
						m_button_list.set_item_selected_single(index-1);
				}
			}
			break;
#if 0
		case IDC_UP:
			{
				t_size index = m_button_list.get_selected_item_single();

				if (index != pfc_infinite && index < m_buttons.get_count() && index)
				{
					m_buttons.swap_items(index, index-1);

					//blaarrgg, designed in the dark ages
					m_selection = &m_buttons[index-1];

					refresh_buttons_list_items(index-1, 2);
					m_button_list.set_item_selected_single(index-1);

				}
			}
			break;
		case IDC_DOWN:
			{
				t_size index = m_button_list.get_selected_item_single();
				if (index != pfc_infinite && index + 1 < m_buttons.get_count())
				{
					m_buttons.swap_items(index, index+1);
					
					//blaarrgg, designed in the dark ages
					m_selection = &m_buttons[index+1];

					refresh_buttons_list_items(index, 2);
					m_button_list.set_item_selected_single(index+1);
				}
			}
			break;
#endif
		case IDC_TOOLS:
			{
				RECT rc;
				GetWindowRect(HWND(lp), &rc);
				HMENU menu = CreatePopupMenu();
				enum {IDM_SET_MASK=1, IDM_EXPORT, IDM_SAVE, IDM_LOAD, IDM_ADD};
					
				//AppendMenu(menu,MF_SEPARATOR,0,0);
				AppendMenu(menu,MF_STRING,IDM_LOAD,_T("Load from file..."));
				AppendMenu(menu,MF_STRING,IDM_ADD,_T("Add from file..."));
				AppendMenu(menu,MF_STRING,IDM_EXPORT,_T("Save to file (embed images)..."));
				AppendMenu(menu,MF_STRING,IDM_SAVE,_T("Save to file (store image paths)..."));
					
				int cmd = TrackPopupMenu(menu,TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,rc.left,rc.bottom,0,wnd,0);
				DestroyMenu(menu);
				if (cmd == IDM_SAVE)
				{
					pfc::string8 path;
					if (uGetOpenFileName(wnd, "fcb files (*.fcb)|*.fcb|All files (*.*)|*.*", 0, "fcb", "Save as", NULL, path, TRUE))
					{
						export_to_file(path, true);
					}
				}
				else if (cmd == IDM_EXPORT)
				{
					pfc::string8 path;
					if (uGetOpenFileName(wnd, "fcb files (*.fcb)|*.fcb|All files (*.*)|*.*", 0, "fcb", "Save as", NULL, path, TRUE))
					{
						export_to_file(path);
					}
				}
				else if (cmd == IDM_LOAD || cmd == IDM_ADD)
				{
					pfc::string8 path;
					if (uGetOpenFileName(wnd, "fcb Files (*.fcb)|*.fcb|All Files (*.*)|*.*", 0, "fcb", "Open file", NULL, path, FALSE))
					{
						m_button_list.remove_items(bit_array_true(), false);

						HWND wnd_show = GetDlgItem(wnd, IDC_SHOW);
						HWND wnd_text = GetDlgItem(wnd, IDC_TEXT_LOCATION);
						HWND wnd_app = GetDlgItem(wnd, IDC_APPEARANCE);

						import_from_file(path, cmd == IDM_ADD);
						SendMessage(wnd_text, CB_SETCURSEL, m_text_below?1:0, 0);
						SendMessage(wnd_app, CB_SETCURSEL, m_appearance, 0);
						populate_buttons_list();
					}
				}
			}
			break;
		case IDC_USE_CUSTOM_TEXT:
			{
				if (m_selection)
				{
					m_selection -> m_use_custom_text = Button_GetCheck(HWND(lp)) != 0;
					bool b_enable = m_selection->m_type != TYPE_SEPARATOR;
					EnableWindow(GetDlgItem(wnd, IDC_TEXT), !b_enable || m_selection->m_use_custom_text);
				}
			}
			break;
		case IDC_TEXT|(EN_CHANGE<<16):
			{
				if (m_selection)
				{
					m_selection->m_text = string_utf8_from_window(HWND(lp));
				}
			}
			break;
		case IDC_PICK:
			{
				if (m_selection)
				{
					command_picker_data p_temp;
					command_picker_param p_data(m_selection->m_guid, m_selection->m_subcommand, 
						m_selection->m_type, m_selection->m_filter);
					p_temp.set_data(p_data);
					if (uDialogBox(IDD_COMMAND,wnd,ConfigCommandProc,reinterpret_cast<LPARAM>(&p_temp)))
					{
						p_temp.get_data(p_data);
						m_selection->m_type = (t_type)p_data.m_group;
						m_selection->m_guid = p_data.m_guid;
						m_selection->m_subcommand = p_data.m_subcommand;
						m_selection->m_filter = (t_filter)p_data.m_filter;

						unsigned idx = m_button_list.get_selected_item_single();
						if (idx != pfc_infinite)
						{
							refresh_buttons_list_items(idx, 1);

							pfc::string8 name;

							m_buttons[idx].get_name(name);

							uSendDlgItemMessageText(wnd, IDC_COMMAND_DESC, WM_SETTEXT, 0, name);
						}
						bool b_enable = m_selection->m_type != TYPE_SEPARATOR;
						EnableWindow(GetDlgItem(wnd, IDC_SHOW), m_selection->m_type != TYPE_SEPARATOR);
						EnableWindow(GetDlgItem(wnd, IDC_USE_CUSTOM_TEXT), b_enable);
						EnableWindow(GetDlgItem(wnd, IDC_TEXT), !b_enable || m_selection->m_use_custom_text);
						SendMessage(m_child, MSG_COMMAND_CHANGE, 0, 0);
					}
				}
			}
			break;
		case IDOK:
			{
				EndDialog(wnd,1);
			}
			return TRUE;
		default:
			return FALSE;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lp)->idFrom)
		{
		case IDC_TAB:
			switch (((LPNMHDR)lp)->code)
			{
			case TCN_SELCHANGE:
				{
					m_active = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB));
					m_image = m_selection ? (m_active ? &m_selection->m_custom_hot_image : &m_selection->m_custom_image) : 0;
					SendMessage(m_child, MSG_BUTTON_CHANGE, 0, 0);
				}
				break;
			}
			break;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
}


BOOL CALLBACK toolbar_extension::ConfigCommandProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			uSetWindowLong(wnd,DWL_USER,lp);
			command_picker_data * ptr = reinterpret_cast<command_picker_data*>(lp);
			return ptr->on_message(wnd, msg, wp, lp);
		}
	default:
		{
			command_picker_data * ptr = reinterpret_cast<command_picker_data*>(uGetWindowLong(wnd,DWL_USER));
			return ptr->on_message(wnd, msg, wp, lp);
		}
	}
}

BOOL CALLBACK toolbar_extension::ConfigChildProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		uSetWindowLong(wnd,DWL_USER,lp);
		{
			config_param * ptr = reinterpret_cast<config_param*>(lp);

			uSendDlgItemMessageText(wnd,IDC_IMAGE_TYPE,CB_ADDSTRING,0,"Default");
			uSendDlgItemMessageText(wnd,IDC_IMAGE_TYPE,CB_ADDSTRING,0,"Custom");

			SHAutoComplete(GetDlgItem(wnd, IDC_IMAGE_PATH), SHACF_FILESYSTEM);
		}
		return TRUE;
		case MSG_COMMAND_CHANGE:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				if (ptr->m_selection)
				{
					bool & b_custom = (ptr->m_active ? ptr->m_selection->m_use_custom_hot : ptr->m_selection->m_use_custom);
					bool b_enable = ptr->m_selection && ptr->m_selection->m_type != TYPE_SEPARATOR;
					EnableWindow(GetDlgItem(wnd, IDC_IMAGE_PATH), b_enable && b_custom);
					EnableWindow(GetDlgItem(wnd, IDC_IMAGE_TYPE), b_enable);
					EnableWindow(GetDlgItem(wnd, IDC_BROWSE), b_enable && b_custom);
				}
			}
			break;
		case MSG_BUTTON_CHANGE:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				bool b_custom = ptr->m_selection ? (ptr->m_active ? ptr->m_selection->m_use_custom_hot : ptr->m_selection->m_use_custom) : false;

				SendDlgItemMessage(wnd,IDC_IMAGE_TYPE,CB_SETCURSEL,ptr->m_selection && b_custom?1:0,0);
				uSendDlgItemMessageText(wnd, IDC_IMAGE_PATH, WM_SETTEXT, 0, (ptr->m_selection && b_custom) ? ptr->m_image->m_path : "");
				bool b_enable = ptr->m_selection && ptr->m_selection->m_type != TYPE_SEPARATOR;
				EnableWindow(GetDlgItem(wnd, IDC_IMAGE_PATH), b_enable && b_custom);
				EnableWindow(GetDlgItem(wnd, IDC_IMAGE_TYPE), b_enable);
				EnableWindow(GetDlgItem(wnd, IDC_BROWSE), b_enable && b_custom);
				if (!b_enable)
				{
					//SendDlgItemMessage(wnd, IDC_IMAGE_TYPE, CB_SETCURSEL, 
				}
			}
			break;
	case WM_COMMAND:
		switch(wp)
		{
		case (CBN_SELCHANGE<<16)|IDC_IMAGE_TYPE:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				if (ptr->m_selection && ptr->m_image)
				{
					unsigned idx = SendMessage((HWND)lp,CB_GETCURSEL,0,0);
					if (idx != CB_ERR && ptr->m_selection)
					{
						bool & b_custom = (ptr->m_active ? ptr->m_selection->m_use_custom_hot : ptr->m_selection->m_use_custom);
						b_custom = idx == 1;
						EnableWindow(GetDlgItem(wnd, IDC_IMAGE_PATH), b_custom);
						EnableWindow(GetDlgItem(wnd, IDC_BROWSE), b_custom);
						EnableWindow(GetDlgItem(wnd, IDC_MASK_TYPE), b_custom);
						EnableWindow(GetDlgItem(wnd, IDC_IMAGE_MASK_PATH), b_custom && ptr->m_image->m_mask_type == ui_extension::MASK_BITMAP);
						EnableWindow(GetDlgItem(wnd, IDC_BROWSE_MASK), b_custom && ptr->m_image->m_mask_type == ui_extension::MASK_BITMAP);
						EnableWindow(GetDlgItem(wnd, IDC_CHANGE_MASK_COLOUR), b_custom && ptr->m_image->m_mask_type == ui_extension::MASK_COLOUR);
						uSendDlgItemMessageText(wnd, IDC_IMAGE_PATH, WM_SETTEXT, 0, (ptr->m_selection && b_custom) ? ptr->m_image->m_path : "");
						uSendDlgItemMessageText(wnd, IDC_IMAGE_MASK_PATH, WM_SETTEXT, 0, (ptr->m_selection && b_custom && ptr->m_image->m_mask_type==ui_extension::MASK_BITMAP) ? ptr->m_image->m_mask_path : "");
						SendDlgItemMessage(wnd,IDC_MASK_TYPE,CB_SETCURSEL,(ptr->m_selection && b_custom)?(unsigned)ptr->m_image->m_mask_type:pfc_infinite,0);
					}
				}
			}
			break;
		case (EN_CHANGE<<16)|IDC_IMAGE_PATH:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				if (ptr->m_image)
				{
					ptr->m_image->m_path = string_utf8_from_window((HWND)lp);
				}
			}
			break;
		case IDC_BROWSE:
			{
				config_param * ptr = reinterpret_cast<config_param*>(uGetWindowLong(wnd,DWL_USER));
				bool b_custom = ptr->m_selection ? (ptr->m_active ? ptr->m_selection->m_use_custom_hot : ptr->m_selection->m_use_custom) : 0;
				if (ptr->m_image && b_custom)
				{
					pfc::string8 temp;
					if (!uGetFullPathName(ptr->m_selection->m_custom_image.m_path, temp) || (uGetFileAttributes(temp) & FILE_ATTRIBUTE_DIRECTORY))
						temp.reset();

					if (uGetOpenFileName(wnd, "Image Files (*.bmp;*.png;*.gif;*.tiff;*.ico)|*.bmp;*.png;*.gif;*.tiff;*.ico|All Files (*.*)|*.*", 0, "png", "Choose image", NULL, temp, FALSE))
					{
						ptr->m_image->m_path = temp;
						uSendDlgItemMessageText(wnd, IDC_IMAGE_PATH, WM_SETTEXT, 0, (1) ? ptr->m_image->m_path : "");
					}
				}
			}
			break;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return FALSE;
}


bool toolbar_extension::show_config_popup(HWND wnd_parent)
{
	config_param param;
	param.m_selection = 0;
	param.m_buttons = m_buttons_config;
	param.m_child = 0;
	param.m_active = 0;
	param.m_image = 0;
	param.m_text_below = m_text_below;
	param.m_appearance = m_appearance;
	bool rv = !!uDialogBox(IDD_BUTTONS,wnd_parent,config_param::g_ConfigPopupProc,reinterpret_cast<LPARAM>(&param));
	if (rv)
	{
		m_text_below = param.m_text_below;
		m_buttons_config = param.m_buttons;
		m_appearance = param.m_appearance;
		if (initialised)
		{
			destroy_toolbar();
			create_toolbar();
			get_host()->on_size_limit_change(wnd_host, ui_extension::size_limit_minimum_width);
		}
	}
	return rv;
}





// {D8E65660-64ED-42e7-850B-31D828C25294}
const GUID toolbar_extension::extension_guid = 
{ 0xd8e65660, 0x64ed, 0x42e7, { 0x85, 0xb, 0x31, 0xd8, 0x28, 0xc2, 0x52, 0x94 } };

ui_extension::window_factory<toolbar_extension> blah;

