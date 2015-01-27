#include "foo_ui_columns.h"

#define COLUMNSPNGAPI PNGAPI

typedef const char * ( __cdecl * zlibVersionProc)(void);
typedef png_charp ( COLUMNSPNGAPI * png_get_libpng_ver_proc)(png_structp png_ptr);


unsigned get_libpng_version(pfc::string8 & version, pfc::string8 & path) 
{
	unsigned rv = LIBPNG_NOTFOUND;
	
	{
#if 0
		try 
		{
			png_structp png_ptr = NULL;
			const char * ptr = png_get_libpng_ver(png_ptr);
			if (ptr)
			{
				rv = LIBPNG_FOUND;
				version = ptr;
			}
		}
		catch (const exception_delayload & p_err)
		{
			console::printf(p_err.what());
			
		}
#else
		
		HINSTANCE  hinstDll = LoadLibrary(_T("libpng13.dll"));
		if (!hinstDll) hinstDll = LoadLibrary(_T("libpng12.dll"));
		
		if (hinstDll)
		{
			uGetModuleFileName(hinstDll, path);
			png_get_libpng_ver_proc p_png_get_libpng_ver = (png_get_libpng_ver_proc)GetProcAddress(hinstDll, "png_get_header_ver"); //png_get_libpng_ver
			rv = LIBPNG_UNKNOWNVERSION;
			
			if (p_png_get_libpng_ver)
			{
				png_structp png_ptr = 0;
				
				
				const char * ptr  = p_png_get_libpng_ver(png_ptr);
				if (ptr)
				{
					rv = LIBPNG_FOUND;
					version = ptr;
				}
				
			}
			
			FreeLibrary(hinstDll);
		}
#endif
	}
	return (rv);
}





unsigned get_zlib_version(pfc::string8 & version, pfc::string8 & path) 
{
	unsigned rv = LIBPNG_NOTFOUND;
	
	{
		
		HINSTANCE  hinstDll = LoadLibrary(_T("zlib1.dll"));
//		if (!hinstDll) hinstDll = LoadLibrary("zlib.dll");

		
		
		if (hinstDll)
		{
			zlibVersionProc zlibVersion = (zlibVersionProc)GetProcAddress(hinstDll, "zlibVersion");

			uGetModuleFileName(hinstDll, path);
			rv = LIBPNG_UNKNOWNVERSION;

			if (zlibVersion)
			{
				
				
				const char * ptr  = zlibVersion();
				if (ptr)
				{
					rv = LIBPNG_FOUND;
					version = ptr;
				}
				
			}
			
			FreeLibrary(hinstDll);
		}
	}
	return (rv);
}
































const char * strchr_n(const char * src, char c, unsigned len)
{
	const char * ptr = src;
	const char * start = ptr;
	while (*ptr && ((unsigned)(ptr - start) < len))
	{
		if (*ptr == c) return ptr;
		ptr++;
	}
	return NULL;
}

void colour::set(COLORREF new_colour)
{
	B = LOBYTE(HIWORD(new_colour));
	G = HIBYTE(LOWORD(new_colour));
	R = LOBYTE(LOWORD(new_colour));
}

/*string_parser::string_parser(const char * ptr, char separator)
{
	while(*ptr)
	{
		const char * start = ptr;
		while(*ptr && *ptr!=separator) ptr++;
		if (ptr>start) data.add_item(pfc::strdup_n(start,ptr-start));
		while(*ptr==separator) ptr++;
	}
}

string_parser::~string_parser()
{
	data.free_all();
}

bool string_parser::is_in_list(const char * text)
{
	int n,t = data.get_count();
	for (n=0; n<t; n++)
	{
		if (!strcmp(text, data.get_item(n)))
			return true;
	}
	return false;
}*/

void set_sel_single(int idx, bool toggle, bool focus, bool single_only)
{
	static_api_ptr_t<playlist_manager> playlist_api;
	unsigned total = playlist_api->activeplaylist_get_item_count();
	unsigned idx_focus = playlist_api->activeplaylist_get_focus_item();

	bit_array_bittable mask(total);
//	if (!single_only) playlist_api->activeplaylist_get_selection_mask(mask);
		mask.set(idx, toggle ? !playlist_api->activeplaylist_is_item_selected(idx) : true);

//	if (single_only)
//		playlist_api->activeplaylist_set_selection(bit_array_one(idx), mask);
//	else
	if (single_only || toggle || !playlist_api->activeplaylist_is_item_selected(idx))
		playlist_api->activeplaylist_set_selection(single_only ? (bit_array&)bit_array_true() : (bit_array&)bit_array_one(idx), mask);
	if (focus && idx_focus != idx) playlist_api->activeplaylist_set_focus_item(idx);
}

void set_sel_range(int start, int end, bool keep, bool deselect)
{
//	console::info(pfc::string_printf("%i %i %i %i",start,end,(long)keep,(long)deselect));
	static_api_ptr_t<playlist_manager> playlist_api;
	unsigned total = playlist_api->activeplaylist_get_item_count();
	bit_array_bittable mask(total);
//	if (keep)
//		playlist_api->activeplaylist_get_selection_mask(mask);
	int n=start,t = end;
				
	if (n<t)
		for(;n<=t;n++) mask.set(n, !deselect);
	else if (n>=t)
		for(;n>=t;n--) mask.set(n, !deselect);
	
		int from = start < end  ? start+1 : end;
		int to = (start < end ? end+1 : start);

	if (keep)
		playlist_api->activeplaylist_set_selection(bit_array_range(from, to-from, true),mask);
	else
		playlist_api->activeplaylist_set_selection(bit_array_true(), mask);
}

int rebar_id_to_idx(HWND wnd, unsigned id)
{
	/* Avoid RB_IDTOINDEX for backwards compatibility */
	REBARBANDINFO  rbbi;
	memset(&rbbi,0,sizeof(rbbi));
	rbbi.cbSize = sizeof(rbbi);
	rbbi.fMask = RBBIM_ID;

	UINT count  = uSendMessage(wnd, RB_GETBANDCOUNT, 0, 0);
	unsigned n;
	for (n=0;n<count;n++)
	{
		uSendMessage(wnd, RB_GETBANDINFO , n, (long)&rbbi);
		if  (rbbi.wID == id) return n;
	}
	return -1;
}

void rebar_show_all_bands(HWND wnd)
{
	UINT count  = uSendMessage(wnd, RB_GETBANDCOUNT, 0, 0);
	unsigned n;
	for (n=0;n<count;n++)
	{
		uSendMessage(wnd, RB_SHOWBAND , n, TRUE);
	}
}

UINT GetNumScrollLines()
{
   HWND hdlMsWheel;
   UINT ucNumLines=3;  // 3 is the default
   OSVERSIONINFO osversion;
   UINT uiMsh_MsgScrollLines;
   

   memset(&osversion, 0, sizeof(osversion));
   osversion.dwOSVersionInfoSize =sizeof(osversion);
   GetVersionEx(&osversion);

   // In Windows 9x & Windows NT 3.51, query MSWheel for the
   // number of scroll lines. In Windows NT 4.0 and later,
   // use SystemParametersInfo. 

   if ((osversion.dwPlatformId ==
                        VER_PLATFORM_WIN32_WINDOWS) ||
       ( (osversion.dwPlatformId ==
                      VER_PLATFORM_WIN32_NT) && 
         (osversion.dwMajorVersion < 4) )   )
   {
        hdlMsWheel = FindWindow(MSH_WHEELMODULE_CLASS, 
                                MSH_WHEELMODULE_TITLE);
        if (hdlMsWheel)
        {
           uiMsh_MsgScrollLines = RegisterWindowMessage
                                     (MSH_SCROLL_LINES);
           if (uiMsh_MsgScrollLines)
                ucNumLines = (int)uSendMessage(hdlMsWheel,
                                    uiMsh_MsgScrollLines, 
                                                       0, 
                                                       0);
        }
   }
   else if ( (osversion.dwPlatformId ==
                         VER_PLATFORM_WIN32_NT) &&
             (osversion.dwMajorVersion >= 4) )
   {
      SystemParametersInfo(SPI_GETWHEELSCROLLLINES,
                                          0,
                                    &ucNumLines, 0);
   }
   return(ucNumLines);
}

bool is_winxp_or_newer()
{
	static OSVERSIONINFO ov;
	static bool blah = false;

	if (!blah)
	{
		ov.dwOSVersionInfoSize = sizeof(ov);
		GetVersionEx(&ov);
	}
	return (ov.dwMajorVersion > 5 || (ov.dwMajorVersion==5 && ov.dwMinorVersion >= 1));
}


/*
int move_playlist(int from, int to)
{
	playlist_switcher * g_switcher = playlist_switcher::get();
	int count = g_switcher->get_num_playlists();
	if (from < to)
	{
		int n = from;
		order_helper order(count);
		while (n<to && n < count)
		{
			order.swap(n,n+1);
			n++;
		}
		return n;
	}
	else if (from > to)
	{
		int n = from;
		order_helper order(count);
		while (n>to && n > 0)
		{
			order.swap(n,n-1);
			n--;
		}
		return n;
	}
	return from;
}
*/

#define PACKVERSION(major,minor) MAKELONG(minor,major)
DWORD GetCommctl32Version(DLLVERSIONINFO2 & dvi, pfc::string_base & p_out)
{
	 HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    /* For security purposes, LoadLibrary should be provided with a 
       fully-qualified path to the DLL. The lpszDllName variable should be
       tested to ensure that it is a fully qualified path before it is used. */
    hinstDll = uLoadLibrary("comctl32.dll");
	
    if(hinstDll)
    {
		uGetModuleFileName(hinstDll, p_out);
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");

        /* Because some DLLs might not implement this function, you
        must test for it explicitly. Depending on the particular 
        DLL, the lack of a DllGetVersion function can be a useful
        indicator of the version. */

        if(pDllGetVersion)
        {
     //       DLLVERSIONINFO dvi;
            HRESULT hr;

            memset(&dvi, 0, sizeof(DLLVERSIONINFO2));
            dvi.info1.cbSize = sizeof(DLLVERSIONINFO2);

            hr = (*pDllGetVersion)(&dvi.info1);

			if (FAILED(hr))
			{
				memset(&dvi, 0, sizeof(DLLVERSIONINFO));
				dvi.info1.cbSize = sizeof(DLLVERSIONINFO);

				hr = (*pDllGetVersion)(&dvi.info1);
			}

            if(SUCCEEDED(hr))
            {
               dwVersion = PACKVERSION(dvi.info1.dwMajorVersion, dvi.info1.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

//#include <uxtheme.h>
/*
typedef void (WINAPI * SetThemeAppPropertiesPROC)( DWORD dwFlags);

void uSetThemeAppProperties(DWORD flags)
{
	HINSTANCE                hinstDll;
	
	hinstDll = uLoadLibrary("uxtheme.dll");
	
	if(hinstDll)
    {
		  
		SetThemeAppPropertiesPROC pETDT = (SetThemeAppPropertiesPROC)GetProcAddress(hinstDll, "SetThemeAppProperties");
		
		if (pETDT)
		{
			 (*pETDT)(flags);
		}
		
		FreeLibrary(hinstDll);
	}
	
}*/


/*

typedef HRESULT  (WINAPI * SetWindowThemePROC)(      
    HWND hwnd,
    LPCWSTR pszSubAppName,
    LPCWSTR pszSubIdList
);

BOOL uSetWindowTheme(HWND wnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	HINSTANCE                hinstDll;
	BOOL							rv(FALSE);
	
	hinstDll = uLoadLibrary("uxtheme.dll");
	
	if(hinstDll)
    {
		  
		SetWindowThemePROC pETDT = (SetWindowThemePROC)GetProcAddress(hinstDll, "SetWindowTheme");
		
		if (pETDT)
		{
			HRESULT hr = (*pETDT)(wnd, pszSubAppName, pszSubIdList);
            if(SUCCEEDED(hr))
            {
				rv = TRUE;
			}
		}
		
		FreeLibrary(hinstDll);
	}
	
	return rv;
}
*/
string_pn::string_pn(metadb_handle_list_cref handles, const char * format, const char * def)
{
	pfc::string8_fast_aggressive a,b;
	a.prealloc(512);b.prealloc(512);
	unsigned n,count=handles.get_count(),f;
	bool use = false;

	pfc::ptr_list_t<char> specs;
	
	const char * ptr = format;
	while(*ptr)
	{
		const char * start = ptr;
		while(*ptr && *ptr!='\\') ptr++;
		if (ptr>start) specs.add_item(pfc::strdup_n(start,ptr-start));
		while(*ptr=='\\') ptr++;
	}
	
	unsigned fmt_count = specs.get_count();
	
	for (f=0;f<fmt_count;f++)
	{
		service_ptr_t<titleformat_object> to_temp;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_temp, specs[f]);
		for (n=0;n<count;n++)
		{
			if (n==0) 
			{
				handles[0]->format_title(0, a, to_temp, 0);
				use=true;
			}
			else 
			{
				handles[n]->format_title(0, b, to_temp, 0);
				if (strcmp(a, b))
				{
					use = false; 
					break;
				}
			}
		}
		
		if (use) break;
		
	}
	
	specs.free_all();
	
	if (use) set_string(a); else set_string(def);
}



menu_item_cache::menu_item_cache()
{
	service_enum_t<mainmenu_commands> e;
	service_ptr_t<mainmenu_commands> ptr;
	
	unsigned p_item_index=0,p_service_item_index;
	while(e.next(ptr))
	{
		//if (ptr->get_type() == menu_item::TYPE_MAIN)
		{
			unsigned p_service_item_count = ptr->get_command_count();
			for (p_service_item_index = 0; p_service_item_index < p_service_item_count; p_service_item_index++)
			{
				menu_item_info info;

				info.m_command = ptr->get_command(p_service_item_index);

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


				/*if (p_node.is_valid() && p_node->get_type() == menu_item_node::TYPE_POPUP)
				{
					unsigned child, child_count = p_node->get_children_count();
					for (child=0;child<child_count;child++)
					{
						menu_item_node * p_child = p_node->get_child(child);
						if (p_child->get_type() == menu_item_node::TYPE_COMMAND)
						{
							pfc::string8 subfull = full,subname;
							unsigned dummy;
							p_child->get_display_data(subname, dummy, metadb_handle_list(), pfc::guid_null);
							subfull.add_byte('/');
							subfull.add_string(subname);

							menu_item_info * p_info = new(std::nothrow) menu_item_info (info);
							p_info->m_subcommand = p_child->get_guid();
							p_child->get_description(p_info->m_desc);
							p_info->m_name = subfull;

							m_data.add_item(p_info);
						}
					}
				}
				else*/
				{
					menu_item_info * p_info = new(std::nothrow) menu_item_info (info);
					ptr->get_description(p_service_item_index, p_info->m_desc);
					p_info->m_name = full;

					m_data.add_item(p_info);
				}


			}
		}
	}
}

const menu_item_cache::menu_item_info & menu_item_cache::get_item(unsigned n) const
{
	return *m_data[n];
}

bool menu_helpers::run_command(const menu_item_identifier & p_command)
{
	return run_command_context(p_command.m_command,p_command.m_subcommand, metadb_handle_list());
}

void populate_menu_combo(HWND wnd, unsigned ID, unsigned ID_DESC, const menu_item_identifier & p_item, menu_item_cache & p_cache, bool insert_none)
{
	HWND wnd_combo = GetDlgItem(wnd, ID);

	unsigned n, count = p_cache.get_count();
	pfc::string8_fast_aggressive temp;
	unsigned idx_none = 0;
	if (insert_none) 
	{
		idx_none = uSendDlgItemMessageText(wnd,ID,CB_ADDSTRING,0,"(None)");
		uSendMessage(wnd_combo,CB_SETITEMDATA,idx_none,-1);
	}

	unsigned sel=-1;
	pfc::string8 desc;

	for (n=0;n<count;n++)
	{
		unsigned idx = uSendMessageText(wnd_combo,CB_ADDSTRING,0,p_cache.get_item(n).m_name);
		SendMessage(wnd_combo,CB_SETITEMDATA,idx,n);

		if (sel == -1 && p_cache.get_item(n) == p_item) {sel = idx; desc=p_cache.get_item(n).m_desc;}
		else if (sel != -1 && idx <= sel) sel++;

		if (insert_none && idx <= idx_none) idx_none++;
	}

	uSendMessageText(wnd_combo,CB_SETCURSEL,sel ==-1 && insert_none ? idx_none : sel,0);

	//menu_helpers::get_description(menu_item::TYPE_MAIN, item, desc);
	uSendDlgItemMessageText(wnd, ID_DESC, WM_SETTEXT, 0, desc);
				
}

bool operator==(const menu_item_identifier & p1, const menu_item_identifier & p2)
{
	return p1.m_command == p2.m_command && p1.m_subcommand == p2.m_subcommand;
}

bool operator!=(const menu_item_identifier & p1, const menu_item_identifier & p2)
{
    return !(p1 == p2);
}

void on_menu_combo_change(HWND wnd, LPARAM lp, cfg_menu_item & cfg_menu_store, menu_item_cache & p_cache, unsigned ID_DESC) 
{
	HWND wnd_combo = (HWND)lp;

	pfc::string8 temp;
	unsigned cache_idx =  uSendMessage(wnd_combo,CB_GETITEMDATA,uSendMessage(wnd_combo,CB_GETCURSEL,0,0),0);

	if (cache_idx == -1)
	{
		cfg_menu_store = menu_item_identifier();
	}
	else if (cache_idx < p_cache.get_count())
	{
		cfg_menu_store = p_cache.get_item(cache_idx);
	}
	
	pfc::string8 desc;
	//if (cfg_menu_store != pfc::guid_null) menu_helpers::get_description(menu_item::TYPE_MAIN, cfg_menu_store, desc);
	uSendDlgItemMessageText(wnd, ID_DESC, WM_SETTEXT, 0, cache_idx < p_cache.get_count() ? p_cache.get_item(cache_idx).m_desc.get_ptr() : "");
}

UINT_PTR CALLBACK choose_font_hook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			CHOOSEFONT * cf = reinterpret_cast<CHOOSEFONT*>(lp);
			reinterpret_cast<modal_dialog_scope*>(cf->lCustData)->initialize(FindOwningPopup(wnd));
		}
		return 0;
	default:
		return 0;
	}
}

BOOL font_picker(LOGFONT & p_font,HWND parent)
{
	modal_dialog_scope scope(parent);

	CHOOSEFONT cf;
	memset(&cf,0,sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner=parent;
	cf.lpLogFont=&p_font;
	cf.Flags=CF_SCREENFONTS|CF_FORCEFONTEXIST|CF_INITTOLOGFONTSTRUCT/*|CF_ENABLEHOOK*/;
	cf.nFontType=SCREEN_FONTTYPE;
	cf.lCustData = reinterpret_cast<LPARAM>(&scope);
	cf.lpfnHook = choose_font_hook;
	BOOL rv = ChooseFont(&cf);
	return rv;
}


void g_save_playlist(HWND wnd, const pfc::list_base_const_t<metadb_handle_ptr> & p_items, const char * p_name)
{
	pfc::string8 name = p_name;
	//name << p_name;

	pfc::string_formatter ext;
	service_enum_t<playlist_loader> e;
	service_ptr_t<playlist_loader> ptr;
	unsigned def_index=0,n=0;

	while(e.next(ptr))
	{
		if (ptr->can_write())
		{
			ext << ptr->get_extension() << " files|*." << ptr->get_extension() << "|";
			if (!stricmp_utf8(ptr->get_extension(), "fpl"))
				def_index=n;
			n++;
		}
	}
	if (uGetOpenFileName(wnd, ext, def_index, "fpl", "Save playlist...", NULL, name, TRUE))
	{
		try{
			playlist_loader::g_save_playlist(name, p_items, abort_callback_impl());
		}
		catch (pfc::exception & e)
		{
			popup_message::g_show(e.what(), "Error writing playlist", popup_message::icon_error);
		}
	}
}

