#ifndef _COLUMNS_UI_H_
#define _COLUMNS_UI_H_


#define cfg_tabs 0
#define cfg_show_all_toolbars 0

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif

#define WM_CHANGEUISTATE                0x0127
#define WM_UPDATEUISTATE                0x0128
#define WM_QUERYUISTATE                 0x0129

//extern logfont_os_menu g_font_os_menu;
LOGFONT get_menu_font();
LOGFONT get_icon_font();

/*
 * LOWORD(wParam) values in WM_*UISTATE*
 */
#define UIS_SET                         1
#define UIS_CLEAR                       2
#define UIS_INITIALIZE                  3

/*
 * HIWORD(wParam) values in WM_*UISTATE*
 */
#define UISF_HIDEFOCUS                  0x1
#define UISF_HIDEACCEL                  0x2
//#if(_WIN32_WINNT >= 0x0501)
#define UISF_ACTIVE                     0x4


#define SEEK_TIMER_ID  669
#define SWITCH_TIMER_ID  670
#define DAY_TIMER_ID  671
#define HIDE_SIDEBAR_TIMER_ID  672


namespace colours
{
	enum t_colours
	{
		COLOUR_TEXT,
		COLOUR_SELECTED_TEXT,
		COLOUR_BACK,
		COLOUR_SELECTED_BACK,
		COLOUR_SELECTED_BACK_NO_FOCUS,
		COLOUR_SELECTED_TEXT_NO_FOCUS,
		COLOUR_FRAME,
	};
};

COLORREF get_default_colour(colours::t_colours index, bool themed = false);


BOOL uDrawPanelTitle(HDC dc, const RECT * rc_clip, const char * text, int len, bool vert, bool world);
LRESULT CALLBACK g_MainWindowProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);


/* UI IDs */
#define ID_REBAR	2100
#define ID_VIS		2000
#define ID_MENU		2001
#define ID_STATUS	2002
#define ID_ORDER	2004
#define ID_PLAYBACK_BUTTONS	2005
#define IDC_PLAYLISTS		1005

#define IDC_PLAYLISTS_TEST	666

#define ID_PLAYLIST_TOOLTIP	50
#define ID_SIDEBAR_TOOLTIP	51

enum {
	MSG_SET_AOT = WM_USER + 3,
	MSG_UPDATE_STATUS,
	MSG_UPDATE_TITLE,
	MSG_KILL_TIMER,
	MSG_SWITCH_PLAYLIST,
	MSG_SIZE,
	MSG_HIDE_SIDEBAR,
	MSG_SET_FOCUS,
	MSG_RUN_INITIAL_SETUP,
	MSG_UPDATE_THUMBBAR,
	MSG_NOTICATION_ICON
};

enum { MSG_DESTROY_PLAYLIST_TOOLTIP = WM_USER + 3, };

namespace status_bar
{
	enum t_parts
	{
		t_parts_none = 0,
		t_parts_all = 0xffffffff,
		t_part_main = 1 << 0,
		t_part_lock = 1 << 1,
		t_part_length = 1 << 2,
		t_part_volume = 1 << 3
	};

	void set_part_sizes(unsigned p_parts = t_parts_none);
};

//int get_pl_item_height();

//void create_balloon();
void update_titlebar();
void update_status();
void refresh_header(bool rebuild = true);

bool process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist = false, bool keyb = true);

void create_rebar();
void destroy_rebar(bool save_config = true);

void create_icon_handle();
void create_systray_icon();
void destroy_systray_icon();
void destroy_playlist_tooltip();

void set_main_window_text(const char * ptr);
void rename_playlist(unsigned idx);
void status_update_main(bool is_caller_menu_desc);
void update_systray(bool balloon = false, int btitle = 0, bool force_balloon = false);


void create_status();
void destroy_sidebar(bool save_config = true);

typedef void(*pma_action)(bool on_on_item, unsigned idx);
#ifndef  _DEBUG
#define DEBUG_TRACK_CALL_TEXT(X)
#define false_assert(exp) !exp
#else 
inline bool false_assert(bool exp)
{
	assert(exp);
	return !exp;
}
#define DEBUG_TRACK_CALL_TEXT(X) TRACK_CALL_TEXT(#X)
#endif

struct pma
{
	const char * name;
	unsigned id;
	pma_action p_run;
};

bool g_rename(pfc::string8 * param, HWND parent);

class playlist_mclick_actions
{
public:
	static pma g_pma_actions[];
	static unsigned id_to_idx(unsigned id);
	inline static bool run(unsigned id, bool on_item, unsigned idx)
	{
		g_pma_actions[id_to_idx(id)].p_run(on_item, idx);
		return true;
	}
	static unsigned get_count();
};

extern cfg_struct_t<LOGFONT> cfg_tab_font;

void g_update_taskbar_buttons_now(bool b_init = false);
void g_update_taskbar_buttons_delayed(bool b_init = false);

extern class status_pane g_status_pane;
extern class rebar_window * g_rebar_window;

namespace main_window
{
	extern user_interface::HookProc_t g_hookproc;
	extern mmh::comptr_t<ITaskbarList3> g_ITaskbarList3;
};

namespace taskbar_buttons {
	enum { ID_FIRST = 667, ID_STOP = ID_FIRST, ID_PREV, ID_PLAY_OR_PAUSE, ID_NEXT, ID_RAND };
}

namespace cui {
	class main_window_t
	{
	public:
		void initialise()
		{
			m_gdiplus_initialised = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplus_instance, &Gdiplus::GdiplusStartupInput(), NULL));
		}
		void deinitialise()
		{
			if (m_gdiplus_initialised)
				Gdiplus::GdiplusShutdown(m_gdiplus_instance);
			m_gdiplus_initialised = false;
		}

		main_window_t() : m_gdiplus_initialised(false), m_gdiplus_instance(NULL) {};
	private:
		ULONG_PTR m_gdiplus_instance;
		bool m_gdiplus_initialised;
	};

	extern main_window_t g_main_window;
}

void make_ui();
void size_windows();
extern advconfig_checkbox_factory g_advbool_notification_icon_x_buttons;


#endif