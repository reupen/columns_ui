#ifndef _COLUMNS_UI_H_
#define _COLUMNS_UI_H_

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <vector>
#include <complex>
#include <iostream>

#define OEMRESOURCE

#include "../SDK/foobar2000.h"
#include "../SDK/core_api.h"
#define UI_EXTENSION_LIBPNG_SUPPORT_ENABLED
#include "uxtheme.h"
#include "Wincodec.h"
#include "../columns_ui-sdk/ui_extension.h"
#include "../ui_helpers/stdafx.h"
#include "../mmh/stdafx.h"
#include "../helpers/helpers.h"
#include "resource.h"
#include "utf8api.h"
#include "helpers.h"

#define move_window_controls size_windows
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

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
/*#include <uxtheme.h>
#include <tmschema.h>*/
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

//#include "..\lpng128\png.h"
#include <io.h>
#include <share.h>

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

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

template <typename type_t>
class ptr_list_autodel_t : public pfc::ptr_list_t<type_t>
{
public:
	~ptr_list_autodel_t()
	{
		delete_all();
	}
};

BOOL uDrawPanelTitle(HDC dc, const RECT * rc_clip, const char * text, int len, bool vert, bool world);

#include "gdiplus.h"
#include "menu_helpers.h"
#include "prefs.h"
#include "config_vars.h"

#include "callback.h"
#include "common.h"
#include "status_bar.h"
#include "columns_v2.h"
#include "cache.h"
#include "rebar.h"
#include "font_notify.h"
#include "sort.h"
#include "config.h"
#include "splitter.h"
#include "layout.h"
extern cfg_columns_t g_columns;
#include "playlist_search.h"
#include "playlist_view.h"
#include "fcs.h"
#include "playlist_manager_utils.h"
#include "playlist_switcher_v2.h"
#include "playlist_tabs.h"
#include "seekbar.h"
#include "vis_gen_host.h"
#include "volume.h"
#include "splitter_tabs.h"
#include "filter.h"
#include "get_msg_hook.h"
#include "setup_dialog.h"
#include "buttons.h"
#include "item_details.h"

/* UI IDs */
#define ID_REBAR     2100
#define ID_VIS    2000
#define ID_MENU  2001
#define ID_STATUS    2002
#define ID_ORDER     2004
#define ID_PLAYBACK_BUTTONS   2005
#define IDC_PLAYLISTS                   1005

#define IDC_PLAYLISTS_TEST                   666

#define ID_PLAYLIST_TOOLTIP   50
#define ID_SIDEBAR_TOOLTIP   51

enum {MSG_SET_AOT = WM_USER+3,
MSG_UPDATE_STATUS , 
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

enum {MSG_DESTROY_PLAYLIST_TOOLTIP=WM_USER+3,};

void size_windows();

//void set_status_parts();

namespace status_bar
{
	enum t_parts
	{
		t_parts_none = 0,
		t_parts_all = 0xffffffff,
		t_part_main = 1<<0,
		t_part_lock = 1<<1,
		t_part_length = 1<<2,
		t_part_volume = 1<<3
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
void destroy_rebar(bool save_config  = true);

void create_icon_handle();
void create_systray_icon();
void destroy_systray_icon();
void destroy_playlist_tooltip();

void set_main_window_text(const char * ptr);
void rename_playlist (unsigned idx);
void status_update_main(bool is_caller_menu_desc);
void update_systray(bool balloon = false, int btitle = 0, bool force_balloon = false);


void create_status();
void destroy_sidebar(bool save_config  = true);

typedef void (*pma_action)(bool on_on_item, unsigned idx);
#ifndef  _DEBUG
#define DEBUG_TRACK_CALL_TEXT(X)
#define false_assert(exp) !exp
#else 
inline bool false_assert(bool exp)
{
	assert (exp);
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

bool g_rename(pfc::string8 * param,HWND parent);

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

namespace main_window
{

#if 0
	class config_inline_metafield_edit_mode_t : public config_item_t<t_uint32>
	{
	public:
		enum metafield_edit_mode_t
		{
			mode_disabled,
			mode_columns,
			mode_windows
		};
		virtual t_uint32 get_default_value ();
		virtual void on_change(){};
		virtual const GUID & get_guid();
		config_inline_metafield_edit_mode_t();
	};

	extern config_inline_metafield_edit_mode_t config_inline_metafield_edit_mode;
#endif

	class config_status_bar_script_t : public config_item_t<pfc::string8>
	{
	public:
		virtual const char * get_default_value () 
		{
			return "//This is the default script for the content of the main status bar pane during playback.\r\n\r\n"
			"$if(%is_status_pane%,%artist% - %title%$crlf(),$if(%ispaused%,Paused,Playing) | )%codec% | %bitrate% kbps | %samplerate% Hz | $caps(%channels%) | %playback_time%[ / %length%]";
		}
		virtual void on_change() 
		{
			if (g_main_window)
				SendMessage(g_main_window,MSG_UPDATE_STATUS,0,0);
		};
		virtual const GUID & get_guid() 
		{
			// {B5CA645B-A5E0-4c70-A598-CD625CF3CC37}
			static const GUID ret = 
			{ 0xb5ca645b, 0xa5e0, 0x4c70, { 0xa5, 0x98, 0xcd, 0x62, 0x5c, 0xf3, 0xcc, 0x37 } };
			return ret;
		};
		config_status_bar_script_t() : config_item_t<pfc::string8>(get_guid(), get_default_value()) {};
	};

	class config_notification_icon_script_t : public config_item_t<pfc::string8>
	{
	public:
		virtual const char * get_default_value () 
		{
			return "//This is the default script for the content of the notification area icon tooltip during playback.\r\n\r\n"
				"[%title%]$crlf()[%artist%][$crlf()%album%]";
		}
		virtual void on_change() {};
		virtual const GUID & get_guid() 
		{
			// {85D128CF-8B01-4ae9-B81C-6BC4BE67599F}
			static const GUID ret = 
			{ 0x85d128cf, 0x8b01, 0x4ae9, { 0xb8, 0x1c, 0x6b, 0xc4, 0xbe, 0x67, 0x59, 0x9f } };
			return ret;
		};
		config_notification_icon_script_t() : config_item_t<pfc::string8>(get_guid(), get_default_value()) {};
	};

	class config_main_window_title_script_t : public config_item_t<pfc::string8>
	{
	public:
		virtual const char * get_default_value () 
		{
			return "//This is the default script for the title of the main window during playback.\r\n\r\n"
				"[%title% - ]foobar2000";
		}
		virtual void on_change() 
		{
			if (g_main_window)
				SendMessage(g_main_window,MSG_UPDATE_TITLE,0,0);
		};
		virtual const GUID & get_guid() 
		{
			// {28B799FB-BC22-4e1c-B999-F1E6B1F26040}
			static const GUID ret = 
			{ 0x28b799fb, 0xbc22, 0x4e1c, { 0xb9, 0x99, 0xf1, 0xe6, 0xb1, 0xf2, 0x60, 0x40 } };
			return ret;
		};
		config_main_window_title_script_t() : config_item_t<pfc::string8>(get_guid(), get_default_value()) {};
	};

	extern config_status_bar_script_t config_status_bar_script;
	extern config_notification_icon_script_t config_notification_icon_script;
	extern config_main_window_title_script_t config_main_window_title_script;

	enum metafield_edit_mode_t
	{
		mode_disabled,
		mode_columns
	};

	void config_set_inline_metafield_edit_mode(t_uint32 value);
	t_uint32 config_get_inline_metafield_edit_mode();
	t_uint32 config_get_inline_metafield_edit_mode_default_value();
	void config_reset_inline_metafield_edit_mode();

	void on_transparency_enabled_change();
	void on_transparency_level_change();
	void config_set_transparency_enabled(bool b_val);
	bool config_get_transparency_enabled();
	bool config_get_transparency_enabled_default_value();
	void config_reset_transparency_enabled();
	void config_set_transparency_level(unsigned char b_val);
	unsigned char config_get_transparency_level();
	unsigned char config_get_transparency_level_default_value();
	void config_reset_transparency_level();
	void config_set_status_show_lock(bool b_val);
	bool config_get_status_show_lock();
	bool config_get_status_show_lock_default_value();
	void config_reset_status_show_lock();

	bool config_get_is_first_run();
	void config_set_is_first_run();

	void config_reset_activate_target_playlist_on_dropped_items();
	bool config_get_activate_target_playlist_on_dropped_items();
	bool config_get_activate_target_playlist_on_dropped_items_default_value();
	void config_set_activate_target_playlist_on_dropped_items(bool b_val);

	/*void config_reset_status_bar_script();
	const char * config_get_status_bar_script();
	const char * config_get_status_bar_script_default_value();
	void config_set_status_bar_script(const char * b_val);

	void config_reset_notification_icon_script();
	const char * config_get_notification_icon_script();
	const char * config_get_notification_icon_script_default_value();
	void config_set_notification_icon_script(const char * b_val);

	void config_reset_notification_icon_script();
	const char * config_get_notification_icon_script();
	const char * config_get_notification_icon_script_default_value();
	void config_set_notification_icon_script(const char * b_val);*/
};

#include "artwork.h"
#include "ng playlist/ng_playlist.h"

#include "fcl.h"
#include "menu_items.h"
#include "status_pane.h"

#endif