#include "foo_ui_columns.h"
#if 0

#define ID_PLAYBACK_BUTTONS  2005
enum {MSG_HIDE_MENUACC = WM_USER + 1, MSG_SHOW_MENUACC = WM_USER + 2, MSG_CREATE_MENU = WM_USER + 3};

#define BTN_STOP 11
#define BTN_PLAY 12
#define BTN_PAUSE 13
#define BTN_RANDOM 14
#define BTN_BACK 15
#define BTN_NEXT 16
#define BTN_OPEN 17


extern cfg_int cfg_back,
	cfg_lock,
	cfg_header,
	cfg_plist,
	cfg_drop_at_end,
	cfg_ellipsis,
	cfg_minimise_to_tray,
	cfg_show_vol,
	cfg_custom_icon,
	cfg_custom_buttons,
	cfg_custom_buttons_over,
//	cfg_custom_buttons_transparency,
	cfg_nohscroll;

extern cfg_string cfg_tray_icon_path,cfg_export,cfg_import,cfg_custom_buttons_path,cfg_globalstring,cfg_colour,cfg_pgenstring;



class toolbar_extension : public ui_extension::container_ui_extension
{
	static const char * class_name;
	static HIMAGELIST g_toolbar_images;
	static HIMAGELIST g_toolbar_images_mouse_over;
	static int width;
	static int height;
	
	static void create_toolbar_imagelist();
	static void destroy_toolbar_imagelist();
	static inline void ensure_images_created()
	{
		if (!g_toolbar_images) create_toolbar_imagelist();
	}
	
	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data(class_name, true);
	}

	WNDPROC menuproc;
	bool initialised;

public:
	
	HWND wnd_toolbar;

	bool disable_dd;
	virtual LRESULT WINAPI on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	
	toolbar_extension();
	~toolbar_extension();
	
	static const GUID extension_guid;
	
	virtual const GUID & get_extension_guid() const
	{
		return extension_guid;
	}
	
	virtual void get_name(pfc::string_base & out)const;
	
	virtual void get_category(pfc::string_base & out)const;

	virtual unsigned get_type  () const{return ui_extension::type_toolbar;};
	
};

HIMAGELIST toolbar_extension::g_toolbar_images = 0;
HIMAGELIST toolbar_extension::g_toolbar_images_mouse_over = 0;
int toolbar_extension::width(13);
int toolbar_extension::height(14);

void toolbar_extension::create_toolbar_imagelist()
{
	
	if (g_toolbar_images) {ImageList_Destroy(g_toolbar_images); g_toolbar_images=0;}
	
	HBITMAP bm_buttons = 0;
	
	bool mouse_over = (cfg_custom_buttons_over && cfg_custom_buttons);
	bool transparency  = 0;//(cfg_custom_buttons_transparency!=0);
	
	if (cfg_custom_buttons)
	{
		if (!_stricmp(pfc::string_extension(cfg_custom_buttons_path),"png"))
		{
	//		HDC dc = GetDC(0);
			bm_buttons = read_png(0, cfg_custom_buttons_path);
	//		ReleaseDC(0, dc);
		}
		else
			bm_buttons = (HBITMAP)uLoadImage(core_api::get_my_instance(), cfg_custom_buttons_path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION|(transparency ? LR_LOADMAP3DCOLORS|LR_LOADTRANSPARENT : 0)); 
	}
	COLORREF colour_3dface = GetSysColor(COLOR_3DFACE);
	if (!bm_buttons)
	{
		mouse_over = false;
		COLORMAP map[2];
		memset(&map, 0, sizeof(map));
		map[0].from = 0x000000;
		map[0].to = colour_3dface;
		map[1].from = 0xffffff;
		map[1].to = GetSysColor(COLOR_BTNTEXT);
		
		bm_buttons = CreateMappedBitmap(core_api::get_my_instance(), IDB_BUTTONS16, 0, &map[0], 2);
		//	buttons_images = (HBITMAP)uLoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_BUTTONS256), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS|LR_DEFAULTSIZE|LR_LOADTRANSPARENT); 
	}
	
	width = 13;
	height = 14;
	
	BITMAP info;
	memset(&info, 0, sizeof(info));
	if (GetObject (bm_buttons, sizeof(info), &info))
	{
		width = info.bmWidth/(mouse_over ? 14 : 7);
		height = info.bmHeight;
		//		if (mouse_over) width /= 2;
	}
	
	
	g_toolbar_images = ImageList_Create(width, height, ILC_COLOR32|ILC_MASK, 7, 0);
	
	ImageList_AddMasked(g_toolbar_images, bm_buttons, colour_3dface); 
	//	ImageList_Add(g_toolbar_images, bm_buttons, 0); 
	
	DeleteObject(bm_buttons);
	
	if (ImageList_GetImageCount(g_toolbar_images) == 14)
	{
		g_toolbar_images_mouse_over = ImageList_Duplicate(g_toolbar_images);
		if (g_toolbar_images_mouse_over)
		{
			int n = 7;
			for (;n; n--) ImageList_Remove(g_toolbar_images_mouse_over, 0);
		}
	}
	
	int n, count= ImageList_GetImageCount(g_toolbar_images);
	if (count >7)
	{
		for(n=7; n<count; n++)
		{
			ImageList_Remove(g_toolbar_images_mouse_over, 7);
		}
	}
	
}
void toolbar_extension::destroy_toolbar_imagelist()
{
	if (g_toolbar_images)
	{
		ImageList_Destroy(g_toolbar_images);
		g_toolbar_images=0;
	}
	if (g_toolbar_images_mouse_over)
	{
		ImageList_Destroy(g_toolbar_images_mouse_over);
		g_toolbar_images_mouse_over=0;
	}
}


toolbar_extension::toolbar_extension() :  wnd_toolbar(0), initialised(false), disable_dd(false)
{
};

toolbar_extension::~toolbar_extension()
{
}

const char * toolbar_extension::class_name = "{D8E65660-64ED-42e7-850B-31D828C25294}";

LRESULT WINAPI toolbar_extension::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	if(msg == WM_CREATE)
	{
		initialised=true;
		
		TBBUTTON tbb[7]; 
		memset(&tbb, 0, sizeof(tbb));
		
		wnd_toolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, 0, 
			WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |TBSTYLE_FLAT | TBSTYLE_TRANSPARENT |TBSTYLE_TOOLTIPS | CCS_NORESIZE| CCS_NOPARENTALIGN| CCS_NODIVIDER, 
			0, 0, 0, 0, wnd, (HMENU) ID_PLAYBACK_BUTTONS, core_api::get_my_instance(), NULL); 
		
		if (wnd_toolbar)
		{
			//			uSetWindowLong(p_this->wnd_toolbar,GWL_USERDATA,(LPARAM)(p_this));
			
			ensure_images_created();
			
			uSendMessage(wnd_toolbar, TB_SETBITMAPSIZE, (WPARAM) 0, MAKELONG(width,height));
			uSendMessage(wnd_toolbar, TB_SETBUTTONSIZE, (WPARAM) 0, MAKELONG(width,height));

			extern cfg_int cfg_toolbar_disable_default_drawing ;

			disable_dd = cfg_toolbar_disable_default_drawing != 0;

			if (disable_dd) uSendMessage(wnd_toolbar, TB_SETPADDING, (WPARAM) 0, MAKELPARAM(0,0));
			
			uSendMessage(wnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
			
			//			uSendMessage(hwndTB, TB_ADDBITMAP, (WPARAM) 7, (LPARAM) &buttons);
			
			uSendMessage(wnd_toolbar, TB_SETIMAGELIST, (WPARAM) 0, (LPARAM) g_toolbar_images);
			
			if (g_toolbar_images_mouse_over)
			{
				uSendMessage(wnd_toolbar, TB_SETHOTIMAGELIST, (WPARAM) 0, (LPARAM) g_toolbar_images_mouse_over);
			}
			
			//			uSendMessage(hwndTB, TB_SETPADDING, (WPARAM) 0, (LPARAM) 0);
			
			
			
			tbb[0].iBitmap = 0; 
			tbb[0].idCommand = BTN_STOP; 
			tbb[0].fsState = TBSTATE_ENABLED; 
			tbb[0].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON ; 
			
			tbb[1].iBitmap = 1; 
			tbb[1].idCommand = BTN_PAUSE; 
			tbb[1].fsState = TBSTATE_ENABLED; 
			tbb[1].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON ;
			tbb[1].dwData = 0; 
			
			tbb[2].iBitmap = 2; 
			tbb[2].idCommand = BTN_PLAY; 
			tbb[2].fsState = TBSTATE_ENABLED; 
			tbb[2].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON ;
			tbb[2].dwData = 0; 
			
			tbb[3].iBitmap = 3; 
			tbb[3].idCommand = BTN_BACK; 
			tbb[3].fsState = TBSTATE_ENABLED; 
			tbb[3].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON ;
			tbb[3].dwData = 0; 
			
			tbb[4].iBitmap = 4; 
			tbb[4].idCommand = BTN_NEXT; 
			tbb[4].fsState = TBSTATE_ENABLED; 
			tbb[4].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON ;
			tbb[4].dwData = 0; 
			
			tbb[5].iBitmap = 5; 
			tbb[5].idCommand = BTN_RANDOM; 
			tbb[5].fsState = TBSTATE_ENABLED; 
			tbb[5].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON ;
			tbb[5].dwData = 0; 
			
			tbb[6].iBitmap = 6; 
			tbb[6].idCommand = BTN_OPEN; 
			tbb[6].fsState = TBSTATE_ENABLED; 
			tbb[6].fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON ;
			tbb[6].dwData = 0; 
			
			
			uSendMessage(wnd_toolbar, TB_ADDBUTTONS, (WPARAM) 7, (LPARAM) &tbb);
			
			ShowWindow(wnd_toolbar, SW_SHOWNORMAL);
			uSendMessage(wnd_toolbar, TB_AUTOSIZE, 0, 0);
		}
		
		
	}
	else if (msg == WM_DESTROY)
	{
		initialised=false;
	}
	else if (msg==WM_SIZE)
	{
		SetWindowPos(wnd_toolbar, 0, 0, 0, LOWORD(lp), HIWORD(lp), SWP_NOZORDER);
	}
	else if (msg == WM_GETMINMAXINFO)
	{
		LPMINMAXINFO mmi = LPMINMAXINFO(lp);

		RECT rc;

		uSendMessage(wnd_toolbar,  TB_GETITEMRECT, 6, (LPARAM)(&rc));

		mmi->ptMinTrackSize.x = rc.right;
		mmi->ptMinTrackSize.y = rc.bottom;
		mmi->ptMaxTrackSize.y = rc.bottom;
		return 0;
	}

	else if (msg== WM_NOTIFY && ((LPNMHDR)lp)->idFrom == ID_PLAYBACK_BUTTONS)
	{
		switch (((LPNMHDR)lp)->code)
		{
		case TBN_GETINFOTIPA:
			{
				switch (((LPNMTBGETINFOTIPA) lp)->iItem)
				{
				case BTN_STOP:
					StringCchCopyA(((LPNMTBGETINFOTIPA) lp)->pszText,((LPNMTBGETINFOTIPA) lp)->cchTextMax,"Stop");
					break;
				case BTN_PAUSE:
					StringCchCopyA(((LPNMTBGETINFOTIPA) lp)->pszText,((LPNMTBGETINFOTIPA) lp)->cchTextMax,"Pause");
					break;
				case BTN_PLAY:
					StringCchCopyA(((LPNMTBGETINFOTIPA) lp)->pszText,((LPNMTBGETINFOTIPA) lp)->cchTextMax,"Play");
					break;
				case BTN_RANDOM:
					StringCchCopyA(((LPNMTBGETINFOTIPA) lp)->pszText,((LPNMTBGETINFOTIPA) lp)->cchTextMax,"Random");
					break;
				case BTN_BACK:
					StringCchCopyA(((LPNMTBGETINFOTIPA) lp)->pszText,((LPNMTBGETINFOTIPA) lp)->cchTextMax,"Previous");
					break;
				case BTN_NEXT:
					StringCchCopyA(((LPNMTBGETINFOTIPA) lp)->pszText,((LPNMTBGETINFOTIPA) lp)->cchTextMax,"Next");
					break;
				case BTN_OPEN:
					StringCchCopyA(((LPNMTBGETINFOTIPA) lp)->pszText,((LPNMTBGETINFOTIPA) lp)->cchTextMax,"Open...");
					break;
				}
			}
			break;
		case TBN_GETINFOTIPW:
			{
				switch (((LPNMTBGETINFOTIPA) lp)->iItem)
				{
				case BTN_STOP:
					StringCchCopyW(((LPNMTBGETINFOTIPW) lp)->pszText,((LPNMTBGETINFOTIPW) lp)->cchTextMax,L"Stop");
					break;
				case BTN_PAUSE:
					StringCchCopyW(((LPNMTBGETINFOTIPW) lp)->pszText,((LPNMTBGETINFOTIPW) lp)->cchTextMax,L"Pause");
					break;
				case BTN_PLAY:
					StringCchCopyW(((LPNMTBGETINFOTIPW) lp)->pszText,((LPNMTBGETINFOTIPW) lp)->cchTextMax,L"Play");
					break;
				case BTN_RANDOM:
					StringCchCopyW(((LPNMTBGETINFOTIPW) lp)->pszText,((LPNMTBGETINFOTIPW) lp)->cchTextMax,L"Random");
					break;
				case BTN_BACK:
					StringCchCopyW(((LPNMTBGETINFOTIPW) lp)->pszText,((LPNMTBGETINFOTIPW) lp)->cchTextMax,L"Previous");
					break;
				case BTN_NEXT:
					StringCchCopyW(((LPNMTBGETINFOTIPW) lp)->pszText,((LPNMTBGETINFOTIPW) lp)->cchTextMax,L"Next");
					break;
				case BTN_OPEN:
					StringCchCopyW(((LPNMTBGETINFOTIPW) lp)->pszText,((LPNMTBGETINFOTIPW) lp)->cchTextMax,L"Open...");
					break;
				}
			}
			break;
		case NM_CUSTOMDRAW:
			{
				switch (((LPNMCUSTOMDRAW) lp)->dwDrawStage)
				{
				case CDDS_PREPAINT:
					return (disable_dd) ? CDRF_NOTIFYITEMDRAW : CDRF_DODEFAULT;
				case CDDS_ITEMPREPAINT:
					return TBCDRF_NOEDGES|/*TBCDRF_NOOFFSET|*/TBCDRF_NOBACKGROUND;
				}
			}
			break;
		}
	}
	else if (msg == WM_COMMAND)
	{
		switch(wp)
		{
			
		case BTN_STOP:
			standard_commands::main_stop();
			break;
		case BTN_PAUSE:
			standard_commands::main_pause();
			break;
		case BTN_PLAY:
			standard_commands::main_play();
			break;
		case BTN_RANDOM:
			standard_commands::main_random();
			break;
		case BTN_BACK:
			standard_commands::main_previous();
			break;
		case BTN_NEXT:
			standard_commands::main_next();
			break;
		case BTN_OPEN:
			standard_commands::main_open();
			break;
		}
	}
	
	return uDefWindowProc(wnd, msg, wp, lp);
	}
	
	
	
	void toolbar_extension::get_name(pfc::string_base & out)const
	{
		out.set_string("Playback buttons");
	}
	void toolbar_extension::get_category(pfc::string_base & out)const
	{
		out.set_string("Toolbars");
	}
	
	// {D8E65660-64ED-42e7-850B-31D828C25294}
	const GUID toolbar_extension::extension_guid = 
	{ 0xd8e65660, 0x64ed, 0x42e7, { 0x85, 0xb, 0x31, 0xd8, 0x28, 0xc2, 0x52, 0x94 } };
	
	ui_extension::window_factory<toolbar_extension> blah;
	
	
#endif