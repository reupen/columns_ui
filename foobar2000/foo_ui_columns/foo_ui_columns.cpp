/**
* Columns UI foobar2000 component
* 
* \author musicmusic
*/

#include "foo_ui_columns.h"

const TCHAR * main_window_class_name = _T("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}");

GUID null_guid = 
{ 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

#include <strsafe.h>

rebar_window * g_rebar_window = 0;

layout_window g_layout_window;
get_msg_hook_t g_get_msg_hook;

#define VERSION "0.3.8.8"

cfg_rebar g_cfg_rebar(create_guid(0xd26d3aa5,0x9157,0xbf8e,0xd5,0x9f,0x44,0x86,0x1c,0x7a,0x82,0xc7));
cfg_band_cache_t cfg_band_cache(create_guid(0x76e74192,0x6932,0x2671,0x90,0x12,0xcf,0x18,0xca,0x02,0x06,0xe0));

VALIDATE_COMPONENT_FILENAME("foo_ui_columns.dll");

DECLARE_COMPONENT_VERSION("Columns UI",
						  
						  VERSION,
						  
						  "Columns UI\n"
						  "Version " VERSION ", Date "__DATE__"\n"
						  "Written by musicmusic\n"
						  "Copyright (C) 2003-2011\n"
						  "Current version at: yuo.be\n\n"

						  "Columns UI SDK version: " UI_EXTENSION_VERSION
						  
						  );

HIMAGELIST  g_imagelist = NULL;

cache_manager g_cache_manager;

extern cfg_int cfg_child;

// {D2DD7FFC-F780-4fa3-8952-38D82C8C1E4B}
static const GUID g_guid_advbranch_cui = 
{ 0xd2dd7ffc, 0xf780, 0x4fa3, { 0x89, 0x52, 0x38, 0xd8, 0x2c, 0x8c, 0x1e, 0x4b } };

// {CEE3F8CC-7CA6-4277-AAD4-CEC7B9D05A63}
static const GUID g_guid_advbool_notification_icon_x_buttons = 
{ 0xcee3f8cc, 0x7ca6, 0x4277, { 0xaa, 0xd4, 0xce, 0xc7, 0xb9, 0xd0, 0x5a, 0x63 } };

advconfig_branch_factory advBranchCUI("Columns UI", g_guid_advbranch_cui, advconfig_branch::guid_branch_display, 0);

advconfig_checkbox_factory g_advbool_notification_icon_x_buttons("Enable notification icon support for back/forward mouse buttons", g_guid_advbool_notification_icon_x_buttons, g_guid_advbranch_cui, 0, false);

HWND g_main_window=0,
	g_tooltip=0,
	g_rebar=0,
	g_status=0,
	g_buttons=0,
	g_plist_tooltip=0;

int active_item=0,
	actual_active=0,
	scroll_item_offset = 0, 
	horizontal_offset=0;

bool redrop=true,
	ui_initialising=false,
	g_minimised = false,
	g_dragging=false,
	g_drag_lmb=false,
	g_dragging1=false;

bool g_playing = false;

UINT g_seek_timer = 0;

unsigned g_switch_playlist = 0;

namespace statusbar_contextmenus
{
	enum { ID_BASE = 1 };
	service_ptr_t<contextmenu_manager> g_main_nowplaying;
}


namespace systray_contextmenus
{
	enum { ID_ACTIVATE=1, ID_NOW_PLAYING_BASE = 2, ID_BASE_FILE_PREFS = 0x2000, ID_BASE_FILE_EXIT = 0x3000, ID_BASE_PLAYBACK = 0x4000 };
	service_ptr_t<mainmenu_manager> g_menu_file_prefs;
	service_ptr_t<mainmenu_manager> g_menu_file_exit;
	service_ptr_t<mainmenu_manager> g_menu_playback;
	service_ptr_t<contextmenu_manager> g_main_nowplaying;
}


HICON g_icon=0;

static RECT tooltip;

pfc::string8
	statusbartext,
	windowtext,
	menudesc;


HFONT g_font=0;

HHOOK msghook = 0;

COLORREF get_default_colour(colours::t_colours index, bool themed)
{
	switch (index)
	{
	case colours::COLOUR_TEXT:
		return GetSysColor(COLOR_WINDOWTEXT);
	case colours::COLOUR_SELECTED_TEXT:
		return GetSysColor(COLOR_HIGHLIGHTTEXT);
	case colours::COLOUR_BACK:
		return GetSysColor(COLOR_WINDOW);
	case colours::COLOUR_SELECTED_BACK:
		return GetSysColor(COLOR_HIGHLIGHT);
	case colours::COLOUR_FRAME:
		return GetSysColor(COLOR_WINDOWFRAME);
	case colours::COLOUR_SELECTED_BACK_NO_FOCUS:
		return GetSysColor(COLOR_BTNFACE);
	case colours::COLOUR_SELECTED_TEXT_NO_FOCUS:
		return GetSysColor(COLOR_BTNTEXT);
	default:
		return 0x0000FF;
	}
}

inline WINDOWPLACEMENT get_def_window_pos()
{
	WINDOWPLACEMENT rv;
	memset(&rv, 0, sizeof(rv));
	rv.showCmd = SW_SHOWNORMAL;
	rv.length = sizeof(rv);
	rv.ptMaxPosition.x = -1;
	rv.ptMaxPosition.y = -1;
	rv.ptMinPosition.x = -1;
	rv.ptMinPosition.y = -1;
	rv.rcNormalPosition.right = 700;
	rv.rcNormalPosition.bottom = 500;
	return rv;
}

LOGFONT get_menu_font()
{
	static logfont_os_menu font;
	return font;
}

LOGFONT get_icon_font()
{
	static logfont_os_icon font;
	return font;
}

cfg_struct_t<LOGFONT> cfg_font(create_guid(0x2465a5af,0xd5e3,0x29f6,0xae,0x12,0x1f,0x39,0x4e,0x9b,0xff,0xf3),get_icon_font());
cfg_struct_t<LOGFONT> cfg_status_font(create_guid(0x93681691,0xd7ed,0x7567,0x63,0xdf,0xcb,0xc3,0x86,0x9f,0x2f,0x9e),get_menu_font());
cfg_struct_t<LOGFONT> cfg_header_font(create_guid(0x1ebb1ab5,0xefd6,0xf6c9,0xbd,0xfe,0xa6,0x5c,0x1d,0xb9,0x8a,0xec),get_icon_font());
cfg_struct_t<LOGFONT> cfg_tab_font(create_guid(0xa8568cb6,0x771a,0x4180,0xc0,0x4a,0xee,0xd2,0xe1,0x9a,0x4b,0x5f),get_menu_font());

inline bool remember_window_pos()
{
	return config_object::g_get_data_bool_simple(standard_config_objects::bool_remember_window_positions,false);
}

class cfg_window_placement_t : public cfg_struct_t<WINDOWPLACEMENT>
{
	using cfg_struct_t<WINDOWPLACEMENT>::operator=;
	virtual void get_data_raw(stream_writer * out, abort_callback & p_abort)
	{
		if (g_main_window && remember_window_pos())
		{
			WINDOWPLACEMENT wp;
			memset(&wp, 0, sizeof(wp));
			wp.length = sizeof(wp);
			if (GetWindowPlacement(g_main_window, &wp))
				*this = wp;
		}
		const WINDOWPLACEMENT & wp = get_value();

		out->write(&wp,sizeof(wp), p_abort);
	}
public:
	cfg_window_placement_t(const GUID & p_guid) : cfg_struct_t<WINDOWPLACEMENT>(p_guid, get_def_window_pos()) {};
} cfg_window_placement_columns(create_guid(0x8bdb3caa,0x6544,0x07a6,0x89,0x67,0xf8,0x13,0x3a,0x80,0x75,0xbb));


cfg_int cfg_back(create_guid(0x7446b27e,0xb23d,0xb28a,0x05,0x02,0x3a,0x2b,0x28,0xbd,0xb5,0x29),get_default_colour(colours::COLOUR_BACK)),
	cfg_global(create_guid(0xf939e07d,0x0944,0x0f40,0xbf,0x1f,0x6d,0x31,0xaa,0x37,0x10,0x5f),0),
	cfg_focus(create_guid(0x74f0b12b,0x98a6,0x12bc,0xfc,0x92,0x1b,0xbb,0x3a,0xd2,0x6c,0xa5),get_default_colour(colours::COLOUR_FRAME)),
	cfg_vis(create_guid(0x2bb960d2,0xb1a8,0x5741,0x55,0xb6,0x13,0x3f,0xb1,0x80,0x37,0x88),get_default_colour(colours::COLOUR_BACK)),
	cfg_plheight(create_guid(0xc8f7e065,0xeb66,0xe282,0xbd,0xe3,0x70,0xaa,0xf4,0x3a,0x10,0x97),2),
	cfg_vis_edge(create_guid(0x57cd2544,0xd765,0xef88,0x30,0xce,0xd9,0x9b,0x47,0xe4,0x09,0x94),1),
	cfg_vis2(create_guid(0x421d3d3f,0x5289,0xb1e4,0x9b,0x91,0xab,0x51,0xd3,0xad,0xbc,0x4d),get_default_colour(colours::COLOUR_TEXT)),
	cfg_lock(create_guid(0x93843978,0xae88,0x5ba2,0x3c,0xa6,0xbc,0x00,0xb0,0x78,0x74,0xa5),0),
	cfg_header(create_guid(0x7b92fba5,0x91a8,0x479e,0x3e,0xb9,0x0a,0x26,0x44,0x8b,0xef,0x1c),1),
	cfg_show_seltime(create_guid(0x7e70339b,0x877f,0xc4a5,0x02,0xbc,0xee,0x6e,0x9d,0x9d,0x01,0xc9),1),
	cfg_header_hottrack(create_guid(0xc61d7603,0xfb21,0xf362,0xb9,0xcd,0x95,0x2c,0x82,0xf9,0xe5,0x8e),1),
	cfg_drop_at_end(create_guid(0xd72ab7ee,0x271c,0xa72f,0x69,0x47,0xb7,0x26,0x2c,0x38,0xb1,0x60),0),
	cfg_global_sort(create_guid(0xdf68f654,0x4c01,0x9d16,0x0d,0xb5,0xc2,0x77,0x95,0x8d,0x8f,0xfa),0),
	cfg_mclick(create_guid(0xfab4859d,0x72ae,0xd27e,0xb1,0x19,0xd6,0xfe,0x88,0x4b,0x67,0x93),0),
	cfg_mclick2(create_guid(0x41fbf3da,0x4026,0x2505,0xbd,0x7a,0x53,0x76,0x73,0x51,0x0f,0x3e),1),
	cfg_plm_rename(create_guid(0xc793fce9,0xa0e8,0xc235,0xd4,0x3b,0xe4,0xdd,0x2c,0xd0,0x45,0xb7),1),
	cfg_balloon(create_guid(0x55a67634,0x1a85,0xf736,0x5f,0xce,0x00,0xdf,0xd4,0xd9,0x10,0x02),0),
	cfg_ellipsis(create_guid(0xb6fe16e9,0x0502,0x2b66,0x45,0xd1,0xda,0xfb,0xfc,0x37,0xe0,0x33),1),
	cfg_plist_text(create_guid(0x649c0bdc,0x8420,0x5e4e,0x18,0xc5,0x03,0x41,0x32,0xbe,0xba,0x7d),get_default_colour(colours::COLOUR_TEXT)),
	cfg_plist_bk(create_guid(0x9615e179,0xe7d0,0xe2f2,0x76,0x10,0x30,0x56,0x4d,0xd0,0xd5,0x44),get_default_colour(colours::COLOUR_BACK)),
	cfg_frame(create_guid(0x992ece57,0xc20d,0x167c,0x34,0x40,0x80,0x5e,0x08,0x1f,0x91,0x14),2),
	cfg_plistframe(create_guid(0xdbfdc16f,0x8cfa,0xcc63,0xa2,0xec,0x17,0xde,0xa8,0xc6,0xdc,0xa1),0),
	cfg_tooltip(create_guid(0x69db592b,0x8c95,0x810d,0xca,0xb7,0x9b,0x61,0xa1,0xa4,0xaa,0x3a),0),
	cfg_tooltips_clipped(create_guid(0x8181afae,0x404e,0xa880,0xe2,0x46,0x38,0x69,0xbd,0xc9,0xd3,0xef),0),
	cfg_np(create_guid(0x2e590774,0xc50e,0xbcd0,0x73,0xa2,0x75,0x2f,0x70,0x9b,0xb1,0x2e),1),
	cfg_show_systray(create_guid(0x19cf6962,0x3d1e,0x10b7,0xcd,0x91,0x48,0x66,0x8b,0x72,0xe8,0x22),1),
	cfg_minimise_to_tray(create_guid(0x4c4bf8f0,0x1f76,0x65f4,0x52,0xe4,0x78,0x01,0x45,0x13,0xf9,0x79),1),
	cfg_show_vol(create_guid(0xa325cde8,0xc0df,0x96f4,0x90,0xe9,0x55,0xbd,0xe7,0xd0,0x32,0xb9),1),
	cfg_custom_icon(create_guid(0xb45bfb84,0x3ecd,0xe706,0x6e,0x13,0xa3,0x2a,0xd0,0x2f,0xac,0x5f),0),
	cfg_drag_autoswitch(create_guid(0x593b498d,0x4cc7,0x1ec8,0xb9,0x20,0x3c,0x95,0xb2,0xde,0x70,0x8c),0),
	cfg_plist_width(create_guid(0x05aba8c0,0x3460,0x3c58,0x20,0xa0,0x00,0x6a,0x54,0x23,0x5c,0x3a),135),
	cfg_drag_pl(create_guid(0x56a9c8d8,0x51fe,0xac6f,0x29,0xe6,0x29,0xb3,0x58,0x82,0xa7,0x26),1),
	cfg_pl_autohide(create_guid(0x70cb66a7,0xbe89,0x8589,0x53,0x6c,0x2b,0x81,0xdb,0xa6,0x72,0x79),0),
	cfg_height(create_guid(0x032abfcb,0x6cab,0x25ee,0x9d,0xe8,0x27,0x89,0xa9,0x0a,0x72,0x36),2),
	cfg_sel_dp(create_guid(0x3586f434,0x1896,0xecaa,0xee,0x61,0x65,0x61,0xc1,0x09,0xd4,0xcb),0),
	cfg_replace_drop_underscores(create_guid(0x73612560,0xa7f0,0x3d30,0xb3,0x2b,0x8d,0xd7,0xe6,0x5f,0xfa,0x91),0),
	cfg_tabs_multiline(create_guid(0xbb86aba9,0x8402,0xb932,0x7a,0xfd,0xf3,0xc9,0xd5,0x40,0xfe,0x2e),1),
	cfg_status(create_guid(0x79829634,0x9b73,0xa8bb,0x89,0x39,0xf1,0x70,0xf7,0x38,0xe8,0x5e),0),
	cfg_pgen_tf(create_guid(0xdc10bfe5,0xe91b,0x513e,0xb9,0xcb,0xc4,0xef,0xd9,0xed,0xac,0x25),0),
	cfg_cur_prefs_col(create_guid(0x6b77648d,0x35ec,0xb125,0x49,0xea,0xa2,0xe6,0xd8,0xa8,0xed,0x0c),0),
	cfg_plist_selected_back(create_guid(0xc8256ac6,0xe25a,0x2518,0x0f,0x58,0xed,0x52,0xe6,0x3b,0xcc,0x5b),get_default_colour(colours::COLOUR_SELECTED_BACK)),
	cfg_plist_selected_frame(create_guid(0x054296d9,0x132d,0x9767,0xbc,0x5f,0x46,0x82,0x75,0xec,0x9e,0x67),get_default_colour(colours::COLOUR_FRAME)),
	cfg_plist_selected_back_no_focus(create_guid(0x88a7c39d,0x2e10,0x5556,0x5a,0x70,0x23,0xac,0x48,0x29,0xcd,0x55),get_default_colour(colours::COLOUR_SELECTED_BACK_NO_FOCUS)),
	cfg_plist_selected_text(create_guid(0xfffa38df,0x3dd4,0x9239,0x82,0x4f,0x19,0x55,0xbd,0xe1,0x9c,0x52),get_default_colour(colours::COLOUR_SELECTED_TEXT)),
	cfg_playlist_sidebar_left_sep(create_guid(0x9e812607,0xb455,0xe066,0xfc,0xfb,0x0a,0x2b,0x9c,0xef,0x2f,0x3c),0),
	cfg_playlist_sidebar_tooltips(create_guid(0x0f6ad5ac,0x7409,0x151e,0xe0,0xd7,0x61,0xe5,0xd8,0x53,0x9b,0x4c),1),
	cfg_playlist_switcher_use_tagz(create_guid(0x43d94259,0x4fde,0x8779,0x8e,0xde,0x18,0xca,0xa7,0x2f,0xc5,0x7b),0),
	cfg_sidebar_use_custom_show_delay(create_guid(0x59e44486,0x2dcc,0x1276,0x83,0x84,0xcf,0xcb,0xc0,0x4f,0xb4,0x1c),0),
	cfg_sidebar_show_delay(create_guid(0xad46e438,0x269d,0xe1c8,0x4a,0xe0,0x08,0x29,0xb5,0x5d,0x2f,0xc3),0),
	cfg_sidebar_hide_delay(create_guid(0xab13b5dc,0x1baa,0x937a,0x85,0x33,0x2d,0x58,0x35,0xc8,0xe7,0xa8),0),
	cfg_toolbars(create_guid(0xfd1f165e,0xeb6e,0xbb2d,0xe0,0xd7,0xc5,0x03,0xee,0xf1,0x6d,0xd7),1),
	cfg_show_sort_arrows(create_guid(0xa9f1111e,0x68d6,0xc707,0xd4,0xad,0x09,0x63,0x72,0x0b,0xe9,0xfa),1),
	cfg_playlist_date(create_guid(0xe0f9c009,0x89f2,0x4a6e,0xdd,0xb5,0x10,0x30,0x0c,0xc3,0x74,0xce),0),
	cfg_autoswitch_delay(create_guid(0x11645dab,0xf8a6,0xdc4e,0x1d,0xc0,0xce,0x50,0xb1,0x27,0xb5,0xbb),500),
	cfg_scar_hidden(create_guid(0x59a3a04e,0xfd16,0x2345,0xe3,0x30,0x8b,0x4d,0xb5,0x96,0x77,0x3f),0),
	cfg_alternative_sel(create_guid(0xfd0cdf1f,0x588a,0x1a2a,0xdc,0x70,0x31,0x06,0x79,0x41,0x52,0xe2),0),
	cfg_oldglobal(create_guid(0x512eace5,0x25c3,0xb722,0x28,0x8b,0xb3,0x4a,0xc5,0x80,0xf4,0xbf),0),
	cfg_playlist_middle_action(create_guid(0xbda32fa2,0xfb5a,0xb715,0x3f,0x00,0xcf,0xaf,0x9b,0x57,0xcd,0x2c),0),
	cfg_nohscroll(create_guid(0x75ede7f7,0x8c57,0x03d9,0x51,0xa2,0xe4,0xe3,0xdd,0x7c,0x8c,0x74),1);


cfg_string cfg_tray_icon_path(create_guid(0x4845fc42,0x5c4c,0x4e80,0xa3,0xae,0x9b,0xdc,0x33,0x2f,0x8d,0x32),"");
cfg_string cfg_export(create_guid(0x834b613e,0x2157,0x5300,0x78,0x33,0x24,0x0b,0x7c,0xda,0x42,0x7f),"");
cfg_string cfg_import(create_guid(0x79587d33,0x97e4,0x72cb,0x92,0xce,0x30,0x7a,0x2f,0x0a,0x34,0x83),"");
cfg_string cfg_custom_buttons_path(create_guid(0x3d077fbb,0x47f2,0x7c15,0x7a,0x3d,0xa1,0x09,0x20,0xa8,0xd5,0x85),"");
cfg_string cfg_globalstring(create_guid(0x355320ea,0xf39e,0x0d97,0xa5,0x94,0xe0,0x40,0x57,0x66,0x51,0x25),"");
cfg_menu_item cfg_statusdbl(create_guid(0x21440b3f,0x4c1d,0xb049,0x46,0xe1,0x37,0xa2,0x7e,0xc1,0xe6,0x93),mainmenu_activate_now_playing_t::g_guid);
cfg_string cfg_pgenstring(create_guid(0x07bee8c2,0xc6f1,0x9db3,0x52,0x55,0x43,0x28,0x1f,0xb3,0xf1,0xe6),"%album%\\$directory(%_path%,2)");

const char * g_default_colour = "$if(%_themed%,,$if($and(%isplaying%,$not(%_is_group%)),\r\n"
"\r\n"
"$puts(back,$offset_colour(%_back%,$offset_colour($calculate_blend_target(%_back%),ff0000,20),25))\r\n"
"$puts(back-selected,$offset_colour(%_selected_back%,$offset_colour($calculate_blend_target(%_selected_back%),ff0000,20),25))\r\n"
"$puts(back-selected-no-focus,$offset_colour(%_selected_back_no_focus%,$offset_colour($calculate_blend_target(%_selected_back_no_focus%),ff0000,20),25))\r\n"
",\r\n"
"\r\n"
"$ifequal($mod($if2(%_display_index%,%list_index%),2),0,\r\n"
"$puts(back,$offset_colour(%_back%,$calculate_blend_target(%_back%),12))\r\n"
"$puts(back-selected,%_selected_back%)\r\n"
"$puts(back-selected-no-focus,%_selected_back_no_focus%)\r\n"
",\r\n"
"$puts(back-selected,$offset_colour(%_selected_back%,$calculate_blend_target(%_selected_back%),7))\r\n"
"$puts(back-selected-no-focus,$offset_colour(%_selected_back_no_focus%,$calculate_blend_target(%_selected_back_no_focus%),7))\r\n"
"$puts(back,%_back%)\r\n"
")\r\n"
"\r\n"
")\r\n"
"$set_style(back,$get(back),$get(back-selected),$get(back-selected-no-focus)))";

cfg_string cfg_colour(create_guid(0xa41b3a98,0x3834,0x3b7c,0x58,0xae,0x1d,0x46,0xb0,0xf9,0x4b,0x0d),g_default_colour);

cfg_menu_item cfg_playlist_double(create_guid(0xffc47d9d,0xb43d,0x8fad,0x8f,0xb3,0x42,0x84,0xbf,0x9a,0x22,0x2a));
cfg_string cfg_playlist_switcher_tagz(create_guid(0x13f4b9ae,0x5db5,0xb083,0x15,0x36,0x08,0x4d,0x55,0xe3,0xb5,0x64),"%title%");

// {F006EC50-7F52-4037-9D48-7447BBF742AA}
static const GUID guid_columns = 
{ 0xf006ec50, 0x7f52, 0x4037, { 0x9d, 0x48, 0x74, 0x47, 0xbb, 0xf7, 0x42, 0xaa } };

cfg_columns_t g_columns(guid_columns);

void action_remove_track(bool on_item, unsigned idx)
{
	if (on_item) 
	{
		static_api_ptr_t<playlist_manager> api;
		api->activeplaylist_undo_backup();
		api->activeplaylist_remove_items(bit_array_one(idx));
	}
}

void action_add_to_queue(bool on_item, unsigned idx)
{
	if (on_item)
	{
		static_api_ptr_t<playlist_manager> api;
		unsigned active = api->get_active_playlist();
		if (active != -1)
			api->queue_add_item_playlist(active, idx);
	}
}

void action_none(bool on_on_item, unsigned idx)
{
}

pma playlist_mclick_actions::g_pma_actions[] =
{
	{"(None)",0,action_none},
	{"Remove track from playlist",1,action_remove_track},
	{"Add to playback queue",2,action_add_to_queue},
};

unsigned playlist_mclick_actions::get_count()
{
	return tabsize(g_pma_actions);
}

unsigned playlist_mclick_actions::id_to_idx(unsigned id)
{
	unsigned n, count = tabsize(g_pma_actions);
	for (n=0;n<count;n++)
	{
		if (g_pma_actions[n].id == id) return n;
	}
	return 0;
}

void rename_playlist (unsigned idx);

bool process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist, bool keyb)
{
	static_api_ptr_t<keyboard_shortcut_manager_v2> keyboard_api;

	if (msg == WM_SYSKEYDOWN)
	{
		if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp)) 
		{
			return true;
		}
	}
	else if (msg == WM_KEYDOWN)
	{
		if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp)) 
		{
			return true;
		}
		if (wp == VK_TAB)
		{
			uie::window::g_on_tab(GetFocus());
		}
	}
	return false;
}


void set_main_window_text(const char * ptr)
{
	if (ptr)
	{
		if (strcmp(windowtext,ptr)) uSetWindowText(g_main_window,ptr);
		windowtext = ptr;
	}
}

bool g_icon_created = false;

void destroy_systray_icon()
{
	if (g_icon_created)
	{
		uShellNotifyIcon(NIM_DELETE, g_main_window, 1, MSG_NOTICATION_ICON, 0, 0);
		g_icon_created = false;
	}
}

void create_systray_icon()
{
	uShellNotifyIcon(g_icon_created ? NIM_MODIFY : NIM_ADD, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, "foobar2000"/*core_version_info::g_get_version_string()*/);
	g_icon_created = true;
}

void create_icon_handle()
{
	if (g_icon) {DestroyIcon(g_icon); g_icon=0;}
	if (cfg_custom_icon)
		g_icon = (HICON)uLoadImage(core_api::get_my_instance(), cfg_tray_icon_path, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	if (!g_icon)
		g_icon = static_api_ptr_t<ui_control>()->load_main_icon(16, 16);
}

static user_interface::HookProc_t g_hookproc;



class playlist_callback_single_columns : public playlist_callback_single_static
{
public:

	virtual void on_items_added(unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection)//inside any of these methods, you can call IPlaylist APIs to get exact info about what happened (but only methods that read playlist state, not those that modify it)
	{
		if (g_main_window) 
		{
			status_bar::set_part_sizes(status_bar::t_part_length);
		}
	}
	virtual void on_items_reordered(const unsigned * order,unsigned count){};//changes selection too; doesnt actually change set of items that are selected or item having focus, just changes their order
	virtual void FB2KAPI on_items_removing(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count){};//called before actually removing them
	virtual void FB2KAPI on_items_removed(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count)
	{
		if (g_main_window) 
		{
			status_bar::set_part_sizes(status_bar::t_part_length);
		}
	};
	virtual void on_items_selection_change(const bit_array & affected,const bit_array & state)
	{
		if (g_main_window) 
		{
			status_bar::set_part_sizes(status_bar::t_part_length);
		}
	}
	virtual void on_item_focus_change(unsigned from,unsigned to){};//focus may be -1 when no item has focus; reminder: focus may also change on other callbacks
	virtual void FB2KAPI on_items_modified(const bit_array & p_mask){;}
	virtual void FB2KAPI on_items_modified_fromplayback(const bit_array & p_mask,play_control::t_display_level p_level){};
	virtual void on_items_replaced(const bit_array & p_mask,const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & p_data){};
	virtual void on_item_ensure_visible(unsigned idx){};

	virtual void on_playlist_switch() 
	{
		if (g_main_window) 
		{
			status_bar::set_part_sizes(status_bar::t_parts_all);
		}
	};
	virtual void on_playlist_renamed(const char * p_new_name,unsigned p_new_name_len) {};
	virtual void on_playlist_locked(bool p_locked) 
	{
		if (g_main_window)
			if (g_status && main_window::config_get_status_show_lock())
				status_bar::set_part_sizes(status_bar::t_parts_all);
	};

	virtual void on_default_format_changed() {};
	virtual void on_playback_order_changed(unsigned p_new_index) {};

	unsigned get_flags() {return playlist_callback_single::flag_all;}

};
static service_factory_single_t<playlist_callback_single_columns> asdf2;


void g_split_string_by_crlf(const char * text, pfc::string_list_impl & p_out)
{
	const char * ptr = text;
	while (*ptr)
	{
		const char * start = ptr;
		t_size counter = 0;
		while (*ptr && *ptr != '\r' && *ptr != '\n') 
		{
			ptr++;
		}

		p_out.add_item(pfc::string8(start, ptr-start));

		if (*ptr == '\r') ptr++;
		if (*ptr == '\n') ptr++;
	}
}

status_pane g_status_pane;

void make_ui()
{
	ui_initialising = true;
	
	RECT rc;
	GetWindowRect(g_main_window, &rc);
	
	long flags = 0;
	if (cfg_frame == 1) flags |= WS_EX_CLIENTEDGE;
	if (cfg_frame == 2) flags |= WS_EX_STATICEDGE;

	g_layout_window.create(g_main_window);
	
	create_rebar();
	create_status();
	if (settings::show_status_pane) g_status_pane.create(g_main_window);
	
	g_layout_window.set_focus();
	ui_initialising = false;
}

void size_windows()
{
	if (!/*g_minimised*/IsIconic(g_main_window) && !ui_initialising)
	{		
		RECT rc_main_client;
		GetClientRect(g_main_window, &rc_main_client);
		
		HDWP dwp = BeginDeferWindowPos(7);
		if (dwp)
		{
			
			int status_height = 0;
			if (g_status) 
			{

				//uSendMessage(g_status, WM_SETREDRAW, FALSE, 0);
				uSendMessage(g_status,WM_SIZE,0,0);
				RECT rc_status;
				GetWindowRect(g_status, &rc_status);
				
				status_height += rc_status.bottom-rc_status.top;
				
				//dwp = DeferWindowPos(dwp, g_status, 0, 0, rc_main_client.bottom-status_height, rc_main_client.right-rc_main_client.left, status_height, SWP_NOZORDER|SWP_NOREDRAW);
				
			}
			if (g_status_pane.get_wnd()) 
			{
				int cy = g_status_pane.get_ideal_height();
				RedrawWindow(g_status_pane.get_wnd(), 0, 0, RDW_INVALIDATE);
				dwp = DeferWindowPos(dwp, g_status_pane.get_wnd(), 0, 0, rc_main_client.bottom-status_height-cy, rc_main_client.right-rc_main_client.left, cy, SWP_NOZORDER);
				status_height += cy;
			}
			int rebar_height=0;
			
			if (g_rebar) 
			{
				RECT rc_rebar;
				GetWindowRect(g_rebar, &rc_rebar);
				rebar_height = rc_rebar.bottom-rc_rebar.top;
			}
			if (g_layout_window.get_wnd())
				dwp = DeferWindowPos(dwp, g_layout_window.get_wnd(), 0, 0, rebar_height, rc_main_client.right-rc_main_client.left, rc_main_client.bottom-rc_main_client.top-rebar_height-status_height, SWP_NOZORDER);
			if (g_rebar) 
			{
				dwp = DeferWindowPos(dwp, g_rebar, 0, 0, 0, rc_main_client.right-rc_main_client.left, rebar_height, SWP_NOZORDER);
			}
			
			EndDeferWindowPos(dwp);

			if (g_status)
			{
				status_bar::set_part_sizes(status_bar::t_parts_none);
			}
			
			
		}
	}
}

bool g_last_rmb = false;

class drop_handler_interface : public IDropTarget
{
	long drop_ref_count;
	POINTL last_over; 
	mmh::comptr_t<IDropTargetHelper> m_DropTargetHelper;
	mmh::comptr_t<IDataObject> m_DataObject;
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
		{
#ifdef _DEBUG
			OutputDebugString(_T("deleting drop_handler_interface"));
#endif
			delete this;
		}
		return rv;
	}
	
	virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
	{
		POINT pt = {ptl.x, ptl.y};
		if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragEnter(g_main_window, pDataObj, &pt, *pdwEffect);

		m_DataObject = pDataObj;

		last_over.x = 0;
		last_over.y = 0;
		g_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

		HWND wnd = WindowFromPoint(pt);
		bool b_fuck_death = wnd != g_main_window && !IsChild(g_main_window, wnd);

		if (ui_drop_item_callback::g_is_accepted_type(pDataObj, pdwEffect) || (!b_fuck_death && static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check(pDataObj)))
		{
			*pdwEffect = DROPEFFECT_COPY;
			pfc::string8 name;
			static_api_ptr_t<playlist_manager>()->activeplaylist_get_name(name);
			mmh::ole::SetDropDescription(pDataObj, DROPIMAGE_COPY, "Add to %1", name);
			return S_OK; 	
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
			mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");
		}
		return S_OK; //??
	}
	

	virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
	{
		POINT pt = {ptl.x, ptl.y};
		if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragOver(&pt, *pdwEffect);

		g_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

		HWND wnd = WindowFromPoint(pt);
		bool b_fuck_death = wnd != g_main_window && !IsChild(g_main_window, wnd);

		//if (last_over.x != pt.x || last_over.y != pt.y)
		if (ui_drop_item_callback::g_is_accepted_type(m_DataObject, pdwEffect) || (!b_fuck_death && static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check(m_DataObject)))
		{
			*pdwEffect = DROPEFFECT_COPY;

			pfc::string8 name;
			static_api_ptr_t<playlist_manager>()->activeplaylist_get_name(name);
			mmh::ole::SetDropDescription(m_DataObject, DROPIMAGE_COPY, "Add to %1", name);
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
			mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");
		}

		last_over = ptl;
		

		return S_OK; 
	}
	
	virtual HRESULT STDMETHODCALLTYPE DragLeave( void)
	{
		if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragLeave();
		mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");

		last_over.x = 0;
		last_over.y = 0;
		m_DataObject.release();
		return S_OK;		
	}
	
	virtual HRESULT STDMETHODCALLTYPE Drop( IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
	{ 
		POINT pt = {ptl.x, ptl.y};
		if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->Drop(pDataObj, &pt, *pdwEffect);

		static_api_ptr_t<playlist_manager> playlist_api;
		static_api_ptr_t<playlist_incoming_item_filter> incoming_api;

		HWND wnd = WindowFromPoint(pt);
		bool b_fuck_death = wnd != g_main_window && !IsChild(g_main_window, wnd);

		*pdwEffect = b_fuck_death ? DROPEFFECT_NONE : DROPEFFECT_COPY;

		bool process = !ui_drop_item_callback::g_on_drop(pDataObj) && !b_fuck_death;
		
		if (process && g_last_rmb)
		{
			process = false;
			enum {ID_DROP = 1, ID_CANCEL };
			
			HMENU menu = CreatePopupMenu();
			
			uAppendMenu(menu,(MF_STRING),ID_DROP,"&Add files here");
			uAppendMenu(menu,MF_SEPARATOR,0,0);
			uAppendMenu(menu,MF_STRING,ID_CANCEL,"&Cancel");
			
			int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,g_main_window,0);
			DestroyMenu(menu);
			
			if (cmd)
			{
				switch(cmd)
				{
				case ID_DROP:
					process = true;						
					break;
				}
			}
		}

		if (process)
		{
			metadb_handle_list data;
			
			incoming_api->process_dropped_files(pDataObj, data,true,g_main_window);
			
			bool send_new_playlist = false;

			int idx = -1;

			playlist_api->activeplaylist_undo_backup();
			playlist_api->activeplaylist_clear_selection();
			playlist_api->activeplaylist_insert_items(idx, data, bit_array_true());
			
			data.remove_all();
		}
		

		if (m_DropTargetHelper.is_valid()) m_DropTargetHelper->DragLeave();
		mmh::ole::SetDropDescription(m_DataObject.get_ptr(), DROPIMAGE_INVALID, "", "");

		m_DataObject.release();
		
		return S_OK;		
	}
	drop_handler_interface() : drop_ref_count(0)
	{
		last_over.x=0;last_over.y=0;
		m_DropTargetHelper.instantiate(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER);
	}
	
};

int start_plist=0;
bool pl_drag=false;

class rename_param
{
public:
	modal_dialog_scope m_scope;
	pfc::string8 * m_text;
};

static BOOL CALLBACK RenameProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		uSetWindowLong(wnd,DWL_USER,lp);
		{
			rename_param * ptr = (rename_param *)lp;
			ptr->m_scope.initialize(FindOwningPopup(wnd));
			uSetWindowText(wnd,uStringPrintf("Rename playlist: \"%s\"",ptr->m_text->get_ptr()));
			uSetDlgItemText(wnd,IDC_EDIT,ptr->m_text->get_ptr());
		}
		return 1;
	case WM_COMMAND:
		switch(wp)
		{
		case IDOK:
			{
				rename_param * ptr = (rename_param *)GetWindowLong(wnd,DWL_USER);
				uGetDlgItemText(wnd,IDC_EDIT,*ptr->m_text);
				EndDialog(wnd,1);
			}
			break;
		case IDCANCEL:
			EndDialog(wnd,0);
			break;
		}
		break;
		case WM_CLOSE:
			EndDialog(wnd,0);
			break;
	}
	return 0;
}

bool g_rename(pfc::string8 * text,HWND parent)
{
	rename_param param;
	param.m_text = text;
	return !!uDialogBox(IDD_RENAME_PLAYLIST,parent,RenameProc,(LPARAM)(&param));
}

void rename_playlist (unsigned idx)
{
	static_api_ptr_t<playlist_manager> playlist_api;
	pfc::string8 temp;
	if (playlist_api->playlist_get_name(idx,temp))
	{
		if (g_rename(&temp,g_main_window))
		{//fucko: dialogobx has a messgeloop, someone might have called switcher api funcs in the meanwhile
//			idx = ((HWND)wp == g_tab) ? idx : uSendMessage(g_plist,LB_GETCURSEL,0,0);
			unsigned num = playlist_api->get_playlist_count();
			if (idx<num)
			{
				playlist_api->playlist_rename(idx,temp,temp.length());
			}
		}
	}
}


HHOOK mouse_hook = 0;
static HWND wnd_last;



void RegisterShellHookWindowHelper(HWND wnd)
{
	typedef BOOL (WINAPI * RegisterShellHookWindowProc)(HWND);
	HINSTANCE inst = LoadLibrary(L"user32.dll");
	if (inst)
	{
		RegisterShellHookWindowProc pRegisterShellHookWindow = (RegisterShellHookWindowProc)GetProcAddress(inst, "RegisterShellHookWindow");
		if (pRegisterShellHookWindow)
			pRegisterShellHookWindow(wnd);
		FreeLibrary(inst);
	}
}

void DeregisterShellHookWindowHelper(HWND wnd)
{
	typedef BOOL (WINAPI * DeregisterShellHookWindowProc)(HWND);
	HINSTANCE inst = LoadLibrary(L"user32.dll");
	if (inst)
	{
		DeregisterShellHookWindowProc pDeregisterShellHookWindow = (DeregisterShellHookWindowProc)GetProcAddress(inst, "DeregisterShellHookWindow");
		if (pDeregisterShellHookWindow)
			pDeregisterShellHookWindow(wnd);
		FreeLibrary(inst);
	}
}

bool g_get_resource_data (INT_PTR id, pfc::array_t<t_uint8> & p_out)
{
	bool ret = false;
	HRSRC rsrc = FindResource(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NOCOVER), L"PNG");
	HGLOBAL handle = LoadResource(core_api::get_my_instance(), rsrc);
	DWORD size = SizeofResource(core_api::get_my_instance(), rsrc);
	LPVOID ptr = LockResource(handle);
	if (ptr && size)
	{
		p_out.append_fromptr((t_uint8*)ptr, size);
		ret = true;
	}
	FreeResource(handle);
	return ret;
}

static mmh::comptr_t<ITaskbarList3> g_ITaskbarList3;

enum {ID_FIRST = 667, ID_STOP = ID_FIRST, ID_PREV, ID_PLAY_OR_PAUSE, ID_NEXT, ID_RAND};
INT_PTR bitmaps[] = {IDI_STOP, IDI_PREV, IDI_PAUSE, IDI_PLAY, IDI_NEXT, IDI_RAND};

/** FUCKO: ITaskbarList3::ThumbBarUpdateButtons calls SendMessageTimeout without SMTO_BLOCK flag */
void g_update_taskbar_buttons_delayed(bool b_init)
{
	if (g_main_window)
		PostMessage(g_main_window, MSG_UPDATE_THUMBBAR, b_init, NULL);
}

void g_update_taskbar_buttons_now(bool b_init)
{
	if (g_ITaskbarList3.is_valid())
	{
		static_api_ptr_t<playback_control> play_api;

		bool b_is_playing = play_api->is_playing();
		bool b_is_paused = play_api->is_paused();

		const WCHAR * ttips[6] = {L"Stop", L"Previous", (b_is_playing && !b_is_paused ? L"Pause" : L"Play"), L"Next", L"Random"};
		INT_PTR bitmap_indices[] = {0, 1, (b_is_playing && !b_is_paused ? 2 : 3), 4, 5};

		THUMBBUTTON tb[tabsize(bitmap_indices)];
		memset(&tb, 0, sizeof(tb));

		size_t i;
		for (i=0; i<tabsize(bitmap_indices); i++)
		{
			tb[i].dwMask = THB_BITMAP|THB_TOOLTIP/*|THB_FLAGS*/;
			tb[i].iId = ID_FIRST + i;
			tb[i].iBitmap =bitmap_indices[i];
			wcscpy_s(tb[i].szTip, tabsize(tb[i].szTip), ttips[i]);
			//if (tb[i].iId == ID_STOP && !b_is_playing)
			//	tb[i].dwFlags |= THBF_DISABLED;
		}

		if (b_init)
			g_ITaskbarList3->ThumbBarAddButtons(g_main_window, tabsize(tb), tb);
		else
			g_ITaskbarList3->ThumbBarUpdateButtons(g_main_window, tabsize(tb), tb);
	}
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
			m_gdiplus_initialised=false;
		}

		main_window_t() : m_gdiplus_initialised(false), m_gdiplus_instance(NULL) {};
	private:
		ULONG_PTR m_gdiplus_instance;
		bool m_gdiplus_initialised;
	} g_main_window;
}

static LRESULT CALLBACK MainProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{

	static UINT WM_TASKBARCREATED;
	static UINT WM_TASKBARBUTTONCREATED;
	static UINT WM_SHELLHOOKMESSAGE;
	static bool g_last_sysray_r_down; 
	static bool g_last_sysray_l_down; 
	static bool g_last_sysray_x1_down; 
	static bool g_last_sysray_x2_down; 
	static bool g_sep_toggle; 

	static HIMAGELIST g_imagelist_taskbar;


	if (g_hookproc)
	{
		LRESULT ret;
		if (g_hookproc(wnd,msg,wp,lp,&ret))
		{
			return ret;
		}
	}

	//ermm we should probably use some kind of class so we can initialise this as a const value
	if (WM_TASKBARCREATED && msg == WM_TASKBARCREATED)
	{
		if (g_icon_created) 
		{
			g_icon_created = false;
			create_systray_icon();
			update_systray();
		}
//		return 0;
	}

	if (WM_TASKBARBUTTONCREATED && msg==WM_TASKBARBUTTONCREATED)
	{
		mmh::comptr_t<ITaskbarList> p_ITaskbarList;
		if (SUCCEEDED (p_ITaskbarList.instantiate(CLSID_TaskbarList)))
		{
			g_ITaskbarList3 = p_ITaskbarList;
			if (g_ITaskbarList3.is_valid() && SUCCEEDED(g_ITaskbarList3->HrInit()))
			{
				const unsigned cx = GetSystemMetrics(SM_CXSMICON), cy = GetSystemMetrics(SM_CYSMICON);

				g_imagelist_taskbar = ImageList_Create(cx, cy, ILC_COLOR32, 0, 6);

				t_size i = 0;

				for (i=0; i<6; i++)
				{
					HICON icon = (HICON)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(bitmaps[i]), IMAGE_ICON, cx, cy, NULL);
					ImageList_ReplaceIcon(g_imagelist_taskbar, -1, icon);
					DestroyIcon(icon);
				}

				if (SUCCEEDED(g_ITaskbarList3->ThumbBarSetImageList(wnd, g_imagelist_taskbar)))
				{
					g_update_taskbar_buttons_delayed(true);
				}
				else g_ITaskbarList3.release();
			}
		}
	}

	if (WM_SHELLHOOKMESSAGE && msg == WM_SHELLHOOKMESSAGE)
	{
		if (wp == HSHELL_APPCOMMAND)
		{
			short cmd  = GET_APPCOMMAND_LPARAM(lp);
			WORD uDevice = GET_DEVICE_LPARAM(lp);
			WORD dwKeys = GET_KEYSTATE_LPARAM(lp);
			switch (cmd)
			{
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
				standard_commands::main_play_or_pause();
				return TRUE;
			case APPCOMMAND_MEDIA_PLAY:
				standard_commands::main_play();
				return TRUE;
			case APPCOMMAND_MEDIA_PAUSE:
				standard_commands::main_pause();
				return TRUE;
			case APPCOMMAND_MEDIA_STOP:
				standard_commands::main_stop();
				return TRUE;
			case APPCOMMAND_MEDIA_NEXTTRACK:
				standard_commands::main_next();
				return TRUE;
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
				standard_commands::main_previous();
				return TRUE;
			default:
				break;
			}
		}
	}
	
	switch(msg)
	{
	case WM_COMMAND:
		{
			switch (wp)
			{
			case ID_STOP|(THBN_CLICKED<<16):
				standard_commands::main_stop();
				break;
			case ID_PLAY_OR_PAUSE|(THBN_CLICKED<<16):
				standard_commands::main_play_or_pause();
				break;
			case ID_PREV|(THBN_CLICKED<<16):
				standard_commands::main_previous();
				break;
			case ID_NEXT|(THBN_CLICKED<<16):
				standard_commands::main_next();
				break;
			case ID_RAND|(THBN_CLICKED<<16):
				standard_commands::main_random();
				break;
			}
		}
		break;
	case WM_MENUSELECT:
		{
			if (HIWORD(wp) & MF_POPUP)
			{
				status_set_menu(false);
			}
			else 
			{
				if (systray_contextmenus::g_menu_file_prefs.is_valid() ||
					systray_contextmenus::g_menu_file_exit.is_valid() ||
					systray_contextmenus::g_menu_playback.is_valid() ||
					systray_contextmenus::g_main_nowplaying.is_valid() ||
					statusbar_contextmenus::g_main_nowplaying.is_valid()
					)
				{
					unsigned id = LOWORD(wp);

					bool set = false;
					if (statusbar_contextmenus::g_main_nowplaying.is_valid())
					{
						contextmenu_node * node = statusbar_contextmenus::g_main_nowplaying->find_by_id(id - statusbar_contextmenus::ID_BASE);
						if (node) set = node->get_description(menudesc);
					}

					if (systray_contextmenus::g_main_nowplaying.is_valid() && id < systray_contextmenus::ID_BASE_FILE_PREFS && id >= systray_contextmenus::ID_NOW_PLAYING_BASE)
					{
						contextmenu_node * node = systray_contextmenus::g_main_nowplaying->find_by_id(id - systray_contextmenus::ID_NOW_PLAYING_BASE);
						if (node) set = node->get_description(menudesc);
					}
					else if (systray_contextmenus::g_menu_file_prefs.is_valid() && id < systray_contextmenus::ID_BASE_FILE_EXIT)
					{
						set = systray_contextmenus::g_menu_file_prefs->get_description(id - systray_contextmenus::ID_BASE_FILE_PREFS, menudesc);
					}
					else if (systray_contextmenus::g_menu_file_exit.is_valid() && id < systray_contextmenus::ID_BASE_PLAYBACK)
					{
						set = systray_contextmenus::g_menu_file_exit->get_description(id - systray_contextmenus::ID_BASE_FILE_EXIT, menudesc);
					}
					else if (systray_contextmenus::g_menu_playback.is_valid())
					{
						set = systray_contextmenus::g_menu_playback->get_description(id - systray_contextmenus::ID_BASE_PLAYBACK, menudesc);
					}

					status_set_menu(set);
				}
			}
		}
		break;
	case WM_SYSCOMMAND:
		if (wp == SC_KEYMENU && !lp) 
		{
			bool processed = false;
			if (g_rebar_window) processed = g_rebar_window->on_alt_up();
			if (!processed) g_layout_window.set_menu_focus();
			else g_layout_window.hide_menu_access_keys();
			return 0;
		}
		break;
	case WM_CONTEXTMENU:
		if (g_status && (HWND)wp == g_status)
		{
			POINT pt = { (short)(LOWORD(lp)), (short)(HIWORD(lp)) };
			enum { ID_CUSTOM_BASE = 1 };
			HMENU menu = CreatePopupMenu();
			service_ptr_t<contextmenu_manager> p_manager;
			contextmenu_manager::g_create(p_manager);
			if (p_manager.is_valid())
			{
				statusbar_contextmenus::g_main_nowplaying = p_manager;

				const keyboard_shortcut_manager::shortcut_type shortcuts[] = {keyboard_shortcut_manager::TYPE_CONTEXT_NOW_PLAYING};
				p_manager->set_shortcut_preference(shortcuts, tabsize(shortcuts));
				if (p_manager->init_context_now_playing(standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0))
				{
					
					p_manager->win32_build_menu(menu,ID_CUSTOM_BASE,-1);
				}

				menu_helpers::win32_auto_mnemonics(menu);

				int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);
				DestroyMenu(menu);
				statusbar_contextmenus::g_main_nowplaying.release();

				if (cmd)
				{
					p_manager->execute_by_id(cmd - ID_CUSTOM_BASE);
				}

				
			}
			
		}
		else if ((HWND)wp == g_rebar)
		{
			if (g_rebar_window)
			{
				enum {IDM_LOCK = 1, IDM_CLOSE, IDM_BASE};

				ui_extension::window_info_list_simple moo;
			
				service_enum_t<ui_extension::window> e;
				uie::window_ptr l;

				POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};

				if (pt.x == -1 && pt.y == -1)
				{
					RECT rc;
					GetWindowRect(GetFocus(), &rc);
					pt.x = rc.left;
					pt.y = rc.bottom;
				}
			
				POINT pt_client = pt;
			
				ScreenToClient(g_rebar_window->wnd_rebar, &pt_client);

				unsigned IDM_EXT_BASE=IDM_BASE+1;
				unsigned n;
			
				HWND child = RealChildWindowFromPoint(g_rebar_window->wnd_rebar, pt_client);

				RBHITTESTINFO rbht;
				rbht.pt = pt_client;

				int idx_hit = uSendMessage(g_rebar, RB_HITTEST, 0, (LPARAM)&rbht);
			
				uie::window_ptr p_ext;

				if (idx_hit < g_rebar_window->bands.get_count())
					p_ext = g_rebar_window->bands[idx_hit]->p_ext; 

				pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> extension_menu_nodes = new ui_extension::menu_hook_impl;

				HMENU menu = CreatePopupMenu();

				if (e.first(l))
				do
				{
					if (g_rebar_window->check_band(l->get_extension_guid()) || (((cfg_show_all_toolbars || ( l->get_type() & ui_extension::type_toolbar))) && l->is_available(&get_rebar_host())))
					{
						ui_extension::window_info_simple info;
					
						l->get_name(info.name);
						l->get_category(info.category);
						info.guid = l->get_extension_guid();
						info.prefer_multiple_instances = l->get_prefer_multiple_instances();
					
						moo.add_item(info);
					
						l.release();
					}
				}
				while (e.next(l));

				moo.sort();

				unsigned count_exts = moo.get_count();
				HMENU popup;
				for(n=0;n<count_exts;n++)
				{
					if (!n || uStringCompare(moo[n-1].category, moo[n].category))
					{
						if (n) uAppendMenu(menu,MF_STRING|MF_POPUP,(UINT)popup,moo[n-1].category);
						popup = CreatePopupMenu();
					}
					uAppendMenu(popup,(MF_STRING| (g_rebar_window->check_band(moo[n].guid) ? MF_CHECKED : 0)),IDM_BASE+n,moo[n].name);
					if (n == count_exts-1) uAppendMenu(menu,MF_STRING|MF_POPUP,(UINT)popup,moo[n].category);
					IDM_EXT_BASE++;
				}

				uAppendMenu(menu,MF_SEPARATOR,0,"");
				uAppendMenu(menu,(((cfg_lock) ? MF_CHECKED : 0)| MF_STRING),IDM_LOCK,"Lock the toolbars");
				if (idx_hit != -1) uAppendMenu(menu,(MF_STRING),IDM_CLOSE,"Remove toolbar");

				if (p_ext.is_valid()) 
				{
					p_ext->get_menu_items(*extension_menu_nodes.get_ptr()); 
					if (extension_menu_nodes->get_children_count() > 0)
						uAppendMenu(menu,MF_SEPARATOR,0,0);
					
					extension_menu_nodes->win32_build_menu(menu, IDM_EXT_BASE, pfc_infinite - IDM_EXT_BASE);
				}

				menu_helpers::win32_auto_mnemonics(menu);
			
				int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);

				if (g_rebar_window)
				{

					if (cmd >= IDM_EXT_BASE)
					{
						extension_menu_nodes->execute_by_id(cmd);
					}

		//			if (p_ext.is_valid()) p_ext->menu_action(menu, IDM_EXT_BASE, cmd, user_data);

					DestroyMenu(menu);


					if (cmd == IDM_LOCK) 
					{
						cfg_lock = (cfg_lock == 0);
						g_rebar_window->update_bands();
					}
					if (cmd == IDM_CLOSE) 
					{
						g_rebar_window->delete_band(idx_hit);
					}
					else if (cmd > 0 && cmd-IDM_BASE < moo.get_count())
					{
						bool shift_down = (GetAsyncKeyState(VK_SHIFT) & (1 << 31)) != 0;
		//				bool ctrl_down = (GetAsyncKeyState(VK_CONTROL) & (1 << 31)) != 0;

						if (!shift_down && !moo[cmd-IDM_BASE].prefer_multiple_instances && g_rebar_window->check_band(moo[cmd-IDM_BASE].guid))
						{
							g_rebar_window->delete_band(moo[cmd-IDM_BASE].guid);
						}
						else
						{
							if (idx_hit != -1)
							g_rebar_window->insert_band(idx_hit+1, moo[cmd-IDM_BASE].guid, g_rebar_window->cache.get_width(moo[cmd-IDM_BASE].guid));
								else
							g_rebar_window->add_band(moo[cmd-IDM_BASE].guid, g_rebar_window->cache.get_width(moo[cmd-IDM_BASE].guid));
						}
					}
				}

			}
		} 
		break;
	case MSG_UPDATE_THUMBBAR:
		g_update_taskbar_buttons_now(wp != 0);
		break;
	case MSG_SET_AOT:
		SetWindowPos(wnd,wp?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		break;
	case MSG_UPDATE_STATUS:
		update_status();
		break;
	case MSG_UPDATE_TITLE:
		update_titlebar();
		break;
	case MSG_RUN_INITIAL_SETUP:
		setup_dialog_t::g_run();
		return 0;
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_DRAWITEM:
		{
			
			if (((LPDRAWITEMSTRUCT) lp)->CtlID == ID_STATUS)
			{
				RECT rc = ((LPDRAWITEMSTRUCT) lp)->rcItem;
	//			rc.right -= 3;
				if (!cfg_show_vol && !cfg_show_seltime && !IsZoomed(g_main_window))
				{
					RECT rc_main;
					GetClientRect(g_main_window, &rc_main);
					rc.right = rc_main.right - GetSystemMetrics(SM_CXVSCROLL);
				}
				else
				{
					int blah[3];
					uSendMessage(g_status, SB_GETBORDERS, 0, (LPARAM)&blah);
					rc.right -= blah[2];
				}

				if (rc.left > rc.right) rc.right = rc.left;

				if (((LPDRAWITEMSTRUCT) lp)->itemData)
				{
#if 0
				HDC dc_mem = CreateCompatibleDC(((LPDRAWITEMSTRUCT) lp)->hDC);

				HBITMAP bm_mem = CreateCompatibleBitmap(((LPDRAWITEMSTRUCT) lp)->hDC, rc.right - rc.left, rc.bottom - rc.top);
 				HBITMAP bm_old = (HBITMAP)SelectObject(dc_mem, bm_mem);

				RECT rc2 = {0,0,rc.right-rc.left, rc.bottom-rc.top};

				OffsetWindowOrgEx(((LPDRAWITEMSTRUCT) lp)->hDC, 0-rc.left, 0-rc.top,0);

				BitBlt(dc_mem,	0, 0, rc.right-rc.left, rc.bottom-rc.top,
					((LPDRAWITEMSTRUCT) lp)->hDC, 0, 0, SRCCOPY);

				OffsetWindowOrgEx(((LPDRAWITEMSTRUCT) lp)->hDC, rc.left, rc.top,0);
			

					ui_helpers::text_out_colours_tab(dc_mem, *(pfc::string8 *)((LPDRAWITEMSTRUCT) lp)->itemData, 
						((pfc::string8 *)((LPDRAWITEMSTRUCT) lp)->itemData)->length(), 2, 0, &rc, FALSE, GetSysColor(COLOR_MENUTEXT), 
						TRUE, true, false, ui_helpers::ALIGN_LEFT);

				BitBlt(((LPDRAWITEMSTRUCT) lp)->hDC, rc.left, rc.top, rc.right, rc.bottom,
					dc_mem, 0, 0, SRCCOPY);
				
				SelectObject(dc_mem, bm_old);
				DeleteObject(bm_mem);
				DeleteDC(dc_mem);
#else
				ui_helpers::text_out_colours_tab(((LPDRAWITEMSTRUCT) lp)->hDC, *(pfc::string8 *)((LPDRAWITEMSTRUCT) lp)->itemData, 
						((pfc::string8 *)((LPDRAWITEMSTRUCT) lp)->itemData)->length(), 2, 0, &rc, FALSE, GetSysColor(COLOR_MENUTEXT), 
						TRUE, true, false, ui_helpers::ALIGN_LEFT);
#endif
				}
				
				return TRUE;
			}

			
		}
		break;
	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
			if (!(lpwp->flags & SWP_NOSIZE))
			{
				ULONG_PTR styles = GetWindowLongPtr(wnd, GWL_STYLE);
				if (styles & WS_MINIMIZE)
				{
					g_minimised = true;
					if (!g_icon_created && cfg_minimise_to_tray) create_systray_icon();
					if (g_icon_created && cfg_minimise_to_tray) ShowWindow(wnd, SW_HIDE);
				} 
				else 
				{
					g_minimised = false;
					size_windows();
				}
			}
		}
		break;
				
	
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONUP:
		if (pl_drag)
		{
			pl_drag = false;ReleaseCapture();SetCursor(LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW)));
		}
		else if (g_sep_toggle && cfg_playlist_sidebar_left_sep && (GET_X_LPARAM(lp) < 1) ) {cfg_scar_hidden = (cfg_scar_hidden==0) ; move_window_controls();}
		break;
	case WM_MENUCHAR:
		{
			unsigned short chr = LOWORD(wp);
			bool processed = false;
			if (g_rebar_window)
			{
					processed = g_rebar_window->on_menu_char(chr);
			}
			if (!processed)
				g_layout_window.on_menu_char(chr);
		}
		return (MNC_CLOSE<<16);
	case WM_SHOWWINDOW:
		break;
	case WM_KILLFOCUS:
		break;
	case WM_CREATE:
		{
			/* initialise ui */

			OSVERSIONINFOEX  vi;
			vi.dwOSVersionInfoSize = sizeof(vi);

			GetVersionEx((OSVERSIONINFO*) &vi);

//			modeless_dialog_manager::add(wnd);

			WM_TASKBARCREATED = RegisterWindowMessage(L"TaskbarCreated");
			WM_SHELLHOOKMESSAGE = RegisterWindowMessage(TEXT("SHELLHOOK"));
			WM_TASKBARBUTTONCREATED = RegisterWindowMessage(L"TaskbarButtonCreated");

			RegisterShellHookWindowHelper (wnd);
		
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icex.dwICC  = ICC_BAR_CLASSES|ICC_COOL_CLASSES|ICC_LISTVIEW_CLASSES|ICC_TAB_CLASSES|ICC_WIN95_CLASSES;
			InitCommonControlsEx(&icex);

			cui::g_main_window.initialise();

			//g_gdiplus_initialised = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&g_gdiplus_instance, &Gdiplus::GdiplusStartupInput(), NULL));
			
			if (!g_keyboard_cues_enabled())
				SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEFOCUS), NULL);

			g_main_window = wnd;
			statusbartext = core_version_info::g_get_version_string();
			set_main_window_text("foobar2000"/*core_version_info::g_get_version_string()*/);
			if (cfg_show_systray) create_systray_icon();

			HRESULT hr = OleInitialize(0);
			pfc::com_ptr_t<drop_handler_interface> drop_handler = new drop_handler_interface;
			RegisterDragDrop(g_main_window, drop_handler.get_ptr());
			
			/* end of initialisation */

//			ShowWindow(wnd, SW_SHOWNORMAL);

			make_ui();

			g_get_msg_hook.register_hook();

			//lets try recursively in wm_showwindow

//			SetWindowPos(wnd, 0, cfg_window_placement_columns.get_value().rcNormalPosition.left, cfg_window_placement_columns.get_value().rcNormalPosition.top, cfg_window_placement_columns.get_value().rcNormalPosition.right - cfg_window_placement_columns.get_value().rcNormalPosition.left, cfg_window_placement_columns.get_value().rcNormalPosition.bottom - cfg_window_placement_columns.get_value().rcNormalPosition.top, SWP_NOZORDER);
//			ShowWindow(wnd, SW_SHOWNORMAL);
			if (config_object::g_get_data_bool_simple(standard_config_objects::bool_ui_always_on_top, false))
				SendMessage(wnd, MSG_SET_AOT, TRUE, 0);


		}
		
		return 0;
	case WM_PARENTNOTIFY:
		{
			if (wp == WM_DESTROY)
			{
			}				
		}
		break;
	case WM_SYSCOLORCHANGE:
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_TIMECHANGE:
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_SETTINGCHANGE:
		if (wp == SPI_SETNONCLIENTMETRICS)
		{
		}
		else if (wp == SPI_SETKEYBOARDCUES)
		{
			bool cues = g_keyboard_cues_enabled();
			SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(cues ? UIS_CLEAR : UIS_SET, UISF_HIDEFOCUS), NULL);
			//SendMessage(wnd, WM_UPDATEUISTATE, MAKEWPARAM(cues ? UIS_CLEAR : UIS_SET, UISF_HIDEFOCUS), NULL);
			//return 0;
		}
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
		break;
	case WM_THEMECHANGED:
		if (is_winxp_or_newer())
		{
			if (g_rebar_window) g_rebar_window->on_themechanged();
			if (g_status)
			{
				status_bar::destroy_theme_handle();
				status_bar::create_theme_handle();
				status_bar::set_part_sizes(status_bar::t_parts_none);
			}
		}
		break;
				
	case WM_KEYDOWN:
		if (process_keydown(msg, lp, wp)) return 0;
		break;
	case WM_SYSKEYUP:
		if (process_keydown(msg, lp, wp)) return 0;
		break;
	case WM_SYSKEYDOWN:
		if (process_keydown(msg, lp, wp)) return 0;
			break;
	case MSG_NOTICATION_ICON:
		if (lp == WM_LBUTTONDOWN) 
		{
			g_last_sysray_l_down = true;
		}
		else if (lp == WM_LBUTTONUP) 
		{
			//bool b_wasDown = g_last_sysray_l_down;
			g_last_sysray_l_down = false;
			//if (b_wasDown) 
				standard_commands::main_activate_or_hide();
		}
		else if (lp == WM_RBUTTONDOWN) 
		{
			g_last_sysray_r_down = true;
		}
#if 0		
		/* There was some misbehaviour with the newer messages. So we don't use them. */
		if (lp == NIN_SELECT || lp == NIN_KEYSELECT) 
		{
			standard_commands::main_activate_or_hide();
			//PostMessage(wnd, WM_NULL, 0, 0);
			return TRUE;
		}
#endif
		else if (lp == WM_XBUTTONDOWN)
		{
			g_last_sysray_x1_down = HIBYTE(GetKeyState(VK_XBUTTON1)) != 0;
			g_last_sysray_x2_down = HIBYTE(GetKeyState(VK_XBUTTON2)) != 0;
			return TRUE;
		}
		else if (lp == WM_MOUSEMOVE)
		{
		}
		else if (lp == WM_XBUTTONUP)
		{
			if (g_advbool_notification_icon_x_buttons.get_static_instance().get_state())
			{
				if (g_last_sysray_x1_down && !g_last_sysray_x2_down)
					standard_commands::main_previous();
				if (g_last_sysray_x2_down && !g_last_sysray_x1_down)
					standard_commands::main_next();
			}
			g_last_sysray_x1_down = false;
			g_last_sysray_x2_down = false;
			return TRUE;
		}
		//else if (lp == WM_CONTEXTMENU) 
		else if (lp == WM_RBUTTONUP) 
		{
			if (g_last_sysray_r_down)
			{
			
				SetForegroundWindow(wnd);
				
				POINT pt;// = {(short)LOWORD(lp),(short)HIWORD(lp)};
				GetCursorPos(&pt);
				
				HMENU menu = CreatePopupMenu();
				HMENU menu_now_playing = 0;
				
				service_ptr_t<contextmenu_manager> p_manager_selection;
				
				if (cfg_np)
				{
					contextmenu_manager::g_create(p_manager_selection);
					if (p_manager_selection.is_valid())
					{
						const keyboard_shortcut_manager::shortcut_type shortcuts[] = {keyboard_shortcut_manager::TYPE_CONTEXT_NOW_PLAYING};
						p_manager_selection->set_shortcut_preference(shortcuts, tabsize(shortcuts));

						if (p_manager_selection->init_context_now_playing(standard_config_objects::query_show_keyboard_shortcuts_in_menus() ? contextmenu_manager::FLAG_SHOW_SHORTCUTS_GLOBAL : 0))
						{
							menu_now_playing = CreatePopupMenu();
							
							
							p_manager_selection->win32_build_menu(menu_now_playing,systray_contextmenus::ID_NOW_PLAYING_BASE,systray_contextmenus::ID_BASE_FILE_PREFS-1);

							pfc::string8_fast_aggressive title,name,title2,title3;
							static_api_ptr_t<play_control> play_api;
							metadb_handle_ptr track;
							if (play_api->get_now_playing(track)) 
							{
								const file_info * info;
								track->metadb_lock();
								if (track->get_info_async_locked(info) || track->get_info_locked(info))
								{
									const char * ptr = info->meta_get("TITLE", 0);
									if (ptr)
										title2 = ptr;
								}
								track->metadb_unlock();
							}
							if (title2.length() > 25) {title2.truncate(24); title2 += "\xe2\x80\xa6";}

							title.prealloc(14+25);
							title.set_string(uStringPrintf("Now Playing: %s",title2.get_ptr()));
					//			play_control::get()->playback_format_title_ex(track, title, "$puts(title,Now Playing: %title%)$ifgreater($len($get(title)),25,$cut($get(title),24)..,$get(title))",0,0,true);
							track.release();
							uFixAmpersandChars_v2(title,name);
							
							uAppendMenu(menu,MF_STRING|MF_POPUP,(UINT)menu_now_playing,name);
							
							uAppendMenu(menu,MF_SEPARATOR,0,"");
						}
					}
					
				}
				

				service_ptr_t<mainmenu_manager> p_manager_context = standard_api_create_t<mainmenu_manager>();
				service_ptr_t<mainmenu_manager> p_manager_playback = standard_api_create_t<mainmenu_manager>();
				systray_contextmenus::g_menu_file_exit = standard_api_create_t<mainmenu_manager>();

				bool b_shortcuts = standard_config_objects::query_show_keyboard_shortcuts_in_menus();

				if (p_manager_playback.is_valid())
				{
					p_manager_playback->instantiate(mainmenu_groups::playback_controls);
					p_manager_playback->generate_menu_win32(menu,systray_contextmenus::ID_BASE_PLAYBACK,pfc_infinite,b_shortcuts ? mainmenu_manager::flag_show_shortcuts_global : 0);

					AppendMenu(menu, MF_SEPARATOR, 0, 0);
				}
				if (p_manager_context.is_valid())
				{

					p_manager_context->instantiate(mainmenu_groups::file_etc_preferences);
					p_manager_context->generate_menu_win32(menu,systray_contextmenus::ID_BASE_FILE_PREFS,systray_contextmenus::ID_BASE_FILE_EXIT-1,b_shortcuts ? mainmenu_manager::flag_show_shortcuts_global : 0);
				}

				AppendMenu(menu, MF_SEPARATOR, 0, 0);
				t_size insert_point = GetMenuItemCount(menu);
				if (systray_contextmenus::g_menu_file_exit.is_valid())
				{

					systray_contextmenus::g_menu_file_exit->instantiate(mainmenu_groups::file_etc_exit);
					systray_contextmenus::g_menu_file_exit->generate_menu_win32(menu,systray_contextmenus::ID_BASE_FILE_EXIT,systray_contextmenus::ID_BASE_PLAYBACK-1,b_shortcuts ? mainmenu_manager::flag_show_shortcuts_global : 0);
				}

				bool b_visible = static_api_ptr_t<ui_control>()->is_visible();
				InsertMenu(menu,insert_point,MF_STRING|MF_BYPOSITION,systray_contextmenus::ID_ACTIVATE,b_visible?_T("Hide foobar2000"):_T("Show foobar2000"));

				systray_contextmenus::g_menu_file_prefs = p_manager_context;
				systray_contextmenus::g_menu_playback = p_manager_playback;
				systray_contextmenus::g_main_nowplaying = p_manager_selection;

				menu_helpers::win32_auto_mnemonics(menu);

				MENUITEMINFO mi;
				memset(&mi, 0, sizeof(mi));
				mi.cbSize = sizeof(MENUITEMINFO);
				mi.fMask = MIIM_STATE;
				mi.fState = MFS_DEFAULT;

				SetMenuItemInfo(menu, systray_contextmenus::ID_ACTIVATE, FALSE, &mi);
				int cmd = TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);



				DestroyMenu(menu);

				systray_contextmenus::g_menu_file_prefs.release();
				systray_contextmenus::g_menu_playback.release();
				systray_contextmenus::g_main_nowplaying.release();

				if (cmd)
				{
					if (cmd == systray_contextmenus::ID_ACTIVATE)
					{
						if (b_visible)
							static_api_ptr_t<ui_control>()->hide();
						else
							static_api_ptr_t<ui_control>()->activate();
					}
					else if (cmd<systray_contextmenus::ID_BASE_FILE_PREFS)
					{
						if (p_manager_selection.is_valid())
						{
							p_manager_selection->execute_by_id(cmd - systray_contextmenus::ID_NOW_PLAYING_BASE);
						}
					}
					else if (cmd < systray_contextmenus::ID_BASE_FILE_EXIT)
					{
						if (p_manager_context.is_valid())
						{
							p_manager_context->execute_command(cmd - systray_contextmenus::ID_BASE_FILE_PREFS);
						}
					}
					else if (cmd < systray_contextmenus::ID_BASE_PLAYBACK)
					{
						if (systray_contextmenus::g_menu_file_exit.is_valid())
						{
							systray_contextmenus::g_menu_file_exit->execute_command(cmd - systray_contextmenus::ID_BASE_FILE_EXIT);
						}
					}
					else
					{
						if (p_manager_playback.is_valid())
						{
							p_manager_playback->execute_command(cmd - systray_contextmenus::ID_BASE_PLAYBACK);
						}
					}
					systray_contextmenus::g_menu_file_exit.release();
				}
				
				PostMessage(wnd, WM_NULL, 0, 0);
			}
			return TRUE;
		}
		break;
	case WM_ACTIVATE:
		{
			if ((LOWORD(wp) == WA_INACTIVE))
			{
				if (!g_keyboard_cues_enabled())
					SendMessage(wnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), NULL);
				wnd_last = GetFocus();
				//if (is_win2k_or_newer())
				{
					if (g_rebar_window) g_rebar_window->hide_accelerators();
					g_layout_window.hide_menu_access_keys();
				}
			}
		}
		break;
	case WM_SETFOCUS:
		{
			if (wnd_last && IsWindow(wnd_last)) SetFocus(wnd_last);
			else g_layout_window.set_focus();

			// wnd_last = 0; // meh minimised fuko
		}
		break;
	case MSG_SET_FOCUS:
		{
		}
		break;
	case WM_ACTIVATEAPP:
		{
		}
		break;
	case MSG_SIZE: 
		move_window_controls();
		return 0;


	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONUP:
		break;
	case WM_DESTROY:
		{
			g_get_msg_hook.deregister_hook();
			g_layout_window.destroy();
			DeregisterShellHookWindowHelper(wnd);


			destroy_rebar(false);
			status_bar::destroy_status_window();
			g_ITaskbarList3.release();
			RevokeDragDrop(g_main_window);
			destroy_systray_icon();
			cui::g_main_window.deinitialise();
			OleUninitialize();
		}
		break;
	case WM_NCDESTROY:
		if (g_imagelist_taskbar)
			ImageList_Destroy(g_imagelist_taskbar);
		break;
	case WM_CLOSE:
		{
			standard_commands::main_exit();
		}
		return 0;
	case WM_TIMER:
		{

		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lp)->idFrom)
		{
		case ID_REBAR:
			switch (((LPNMHDR)lp)->code)
			{
			case RBN_HEIGHTCHANGE:
				{
					move_window_controls();
				}
				break;
			case RBN_LAYOUTCHANGED:
				{
					if (g_rebar_window) 
					{
						g_rebar_window->save_bands();
					}
				}
				break;
			}
			break;
		case ID_STATUS:
			switch (((LPNMHDR)lp)->code)
			{
			case NM_RCLICK:
			case NM_CLICK:
				{
					LPNMMOUSE lpnm =  (LPNMMOUSE)lp;
					unsigned u_parts = SendMessage(lpnm->hdr.hwndFrom, SB_GETPARTS, 0, 0);
					pfc::array_t<unsigned> parts;
					parts.set_size(u_parts);
					SendMessage(lpnm->hdr.hwndFrom, SB_GETPARTS, parts.get_size(), (LPARAM)parts.get_ptr());
					u_parts = parts.get_size();
					if (!IsZoomed(wnd) && u_parts && parts[u_parts-1] == pfc_infinite)
					{
						RECT rc;
						GetClientRect(lpnm->hdr.hwndFrom, &rc);
						parts[u_parts-1] = rc.right - GetSystemMetrics(SM_CXVSCROLL);
					}

					unsigned part = -1, n= 0;
					for (n=0; n<u_parts; n++)
					{
						if ((unsigned)lpnm->pt.x < parts[n]) 
						{
							part = n;
							break;
						}
					}
					if ( cfg_show_vol && /*lpnm->dwItemSpec*/ part == status_bar::u_vol_pos )
					{

						if (!status_bar::volume_popup_window.get_wnd())
						{
							//caption vertical. alt-f4 send crazy with two??
							RECT rc_status;
							GetWindowRect(lpnm->hdr.hwndFrom, &rc_status);
							HWND wndvol = status_bar::volume_popup_window.create(wnd);
							POINT pt = lpnm->pt;
							ClientToScreen(lpnm->hdr.hwndFrom, &pt);
							int cx = volume_control::g_get_caption_size() + 28;
							int cy = 150;
							int x = pt.x;
							int y = pt.y;
							HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
							if (mon)
							{

								MONITORINFO mi;
								memset(&mi, 0, sizeof(MONITORINFO));
								mi.cbSize = sizeof(MONITORINFO);
								if (GetMonitorInfo(mon, &mi))
								{
									if (x + cx > mi.rcMonitor.right)
										x = x-cx > mi.rcMonitor.left ? x-cx : mi.rcMonitor.left;

									if (x < mi.rcMonitor.left)
										x = mi.rcMonitor.left;

									if (y + cy > mi.rcMonitor.bottom)
										y = y-cy > mi.rcMonitor.top ? y-cy : mi.rcMonitor.top;

									if (y < mi.rcMonitor.top)
										y = mi.rcMonitor.top;
								}
							}

							SetWindowPos(wndvol, 0, x, y, cx, cy, SWP_NOZORDER);
							ShowWindow(wndvol, SW_SHOWNORMAL);
						}
						return TRUE;
					}
				}
				break;
			case NM_DBLCLK:
				{
					unsigned long part = ((LPNMMOUSE)lp)->dwItemSpec;

					if (part == 0)
						mainmenu_commands::g_execute(cfg_statusdbl.get_value().m_command);
					//standard_commands::main_highlight_playing();
					else if (cfg_show_vol && part == status_bar::u_vol_pos) 
					{
						//static_api_ptr_t<ui_control>()->show_preferences(preferences_page::guid_playback);
					}
					else if (cfg_show_seltime && part == status_bar::u_length_pos) 
					{
						static_api_ptr_t<playlist_manager>()->activeplaylist_set_selection(bit_array_true(), bit_array_true());
					}

				}
				break;
			}
			break;
		}
		break;
	}
	return uDefWindowProc(wnd, msg, wp, lp);
}

// {27DFB9B0-2621-4935-B670-02576945C012}
const GUID g_guid_colours_fonts_imported = 
{ 0x27dfb9b0, 0x2621, 0x4935, { 0xb6, 0x70, 0x2, 0x57, 0x69, 0x45, 0xc0, 0x12 } };

cfg_bool g_colours_fonts_imported(g_guid_colours_fonts_imported, false);

class ui_test : public user_interface
{
public:
	
	virtual const char * get_name() {return "Columns UI";}
	
	virtual HWND init(HookProc_t hook)
	{
		{
			OSVERSIONINFOEX osvi;
			memset(&osvi,0,sizeof(osvi));
			osvi.dwOSVersionInfoSize = sizeof (osvi);
			if (GetVersionEx((LPOSVERSIONINFO)&osvi))
			{
				if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 && osvi.wServicePackMajor  == 0)
				{
					pfc::string_formatter message;
					message << "Sorry, your operating system Windows XP ";
					if (!osvi.wServicePackMajor)
						message << "(no service pack installed)";
					else
						message << "Service Pack " << osvi.wServicePackMajor;
					message << " is not supported by Columns UI. Please upgrade to Service Pack 1 or newer and try again.\n\nOtherwise, uninstall the Columns UI component to return to the Default User Interface.",
					MessageBox(NULL, uT(message), _T("Columns UI - Unsupported operating system"), MB_OK|MB_ICONEXCLAMATION);
					return NULL;
						
				}
			}
		}
//		performance_counter startup;

		if (main_window::config_get_is_first_run())
		{
			if (!cfg_layout.get_presets().get_count())
				cfg_layout.reset_presets();
		}

		g_hookproc = hook;

	
		WNDCLASS  wc;
		memset(&wc,0,sizeof(wc));
		
		create_icon_handle();
		
		wc.lpfnWndProc    = (WNDPROC)MainProc;
		wc.style    = CS_DBLCLKS;
		wc.hInstance      = core_api::get_my_instance();
		wc.hIcon          = static_api_ptr_t<ui_control>()->get_main_icon();//g_main_icon;
		wc.hCursor        = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
		wc.lpszClassName  = main_window_class_name;
		
		ATOM cls = RegisterClass(&wc);

		RECT rc_work;
		SystemParametersInfo(SPI_GETWORKAREA, NULL, &rc_work, NULL);

		const unsigned cx = (rc_work.right - rc_work.left)*80/100;
		const unsigned cy = (rc_work.bottom - rc_work.top)*80/100;

		unsigned left = (rc_work.right - rc_work.left - cx) / 2;
		unsigned top = (rc_work.bottom - rc_work.top - cy) / 2;

		if (main_window::config_get_is_first_run())
		{
			cfg_plist_width = cx * 10 / 100;
		}
		else if (!g_colours_fonts_imported)
		{
			g_import_pv_colours_to_unified_global();
			g_import_fonts_to_unified();
		}

		g_colours_fonts_imported=true;

		g_main_window = CreateWindowEx(  main_window::config_get_transparency_enabled() ? WS_EX_LAYERED : 0 /*WS_EX_TOOLWINDOW*/, main_window_class_name, _T("foobar2000"), WS_OVERLAPPED | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
			WS_THICKFRAME, left, top, cx, cy, 0, 0, core_api::get_my_instance(), NULL);

		main_window::on_transparency_enabled_change();

		bool rem_pos = remember_window_pos();

		if (rem_pos && !main_window::config_get_is_first_run())
		{
			SetWindowPlacement(g_main_window, &cfg_window_placement_columns.get_value());
			size_windows();
			ShowWindow(g_main_window,cfg_window_placement_columns.get_value().showCmd);
			
			if (g_icon_created && (cfg_window_placement_columns.get_value().showCmd == SW_SHOWMINIMIZED) && cfg_minimise_to_tray)
				ShowWindow(g_main_window,SW_HIDE);
		}
		else
		{
			size_windows();
			ShowWindow(g_main_window,SW_SHOWNORMAL);
		}
		
		if (g_rebar) ShowWindow(g_rebar, SW_SHOWNORMAL);
		if (g_status) ShowWindow(g_status, SW_SHOWNORMAL);
		if (g_status_pane.get_wnd()) ShowWindow(g_status_pane.get_wnd(), SW_SHOWNORMAL);
		g_layout_window.show_window();
		
		RedrawWindow(g_main_window, 0, 0, RDW_UPDATENOW|RDW_ALLCHILDREN);

		if (main_window::config_get_is_first_run())
			SendMessage(g_main_window, MSG_RUN_INITIAL_SETUP, NULL, NULL);

		main_window::config_set_is_first_run();

		return g_main_window;
	}

	GUID get_guid()
	{
		// {F12D0A24-A8A4-4618-9659-6F66DE067524}
		static const GUID guid_columns = 
		{ 0xf12d0a24, 0xa8a4, 0x4618, { 0x96, 0x59, 0x6f, 0x66, 0xde, 0x6, 0x75, 0x24 } };

		return guid_columns;
	}
	
	virtual void show_now_playing()
	{
		static_api_ptr_t<play_control> play_api;
		update_systray(true, play_api->is_paused() ? 2 : 0, true);
	}
	virtual void shutdown()
	{

		//if (!endsession) 
		{
			//if (IsWindow(g_main_window))
			DestroyWindow(g_main_window);
			UnregisterClass(main_window_class_name, core_api::get_my_instance());
			status_bar::volume_popup_window.class_release();
		}
		g_main_window = 0;
		g_status = 0;
		if (g_imagelist) {ImageList_Destroy(g_imagelist); g_imagelist=0;}
		if (g_icon) DestroyIcon(g_icon); g_icon=0;
		status_bar::destroy_icon();

		font_cleanup();
	}
	
	virtual void activate()
	{
		if (g_main_window)
		{
			if (g_icon_created && !cfg_show_systray) destroy_systray_icon();

			if (!is_visible()) 
			{
				ShowWindow(g_main_window,SW_RESTORE);
				if ((GetWindowLong(g_main_window, GWL_EXSTYLE) & WS_EX_LAYERED))
				RedrawWindow(g_main_window, 
					NULL, 
					NULL, 
					RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
			}
			SetForegroundWindow(g_main_window);
		}
	}
	virtual void hide()
	{
		if (g_main_window)
		{
//			if (is_visible()) 
				ShowWindow(g_main_window,SW_MINIMIZE);
		}
	}
	virtual bool is_visible()
	{
		bool rv = false;
		if (g_main_window)
		{
			rv = IsWindowVisible(g_main_window) && !IsIconic(g_main_window);
		}
		return rv;
	}
	virtual void override_statusbar_text(const char * p_text)
	{
		menudesc = p_text;
		status_set_menu(true);
		g_status_pane.enter_menu_mode(p_text);
	};
	virtual void revert_statusbar_text()
	{
		status_set_menu(false);
		g_status_pane.exit_menu_mode();
	}
};


void update_titlebar()
{
	metadb_handle_ptr track;
	static_api_ptr_t<play_control> play_api;
	play_api->get_now_playing(track);
	if (track.is_valid())
	{
		pfc::string8 title;
		service_ptr_t<titleformat_object> to_wtitle;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_wtitle, main_window::config_main_window_title_script.get());
		play_api->playback_format_title_ex(track, 0, title, to_wtitle, NULL, play_control::display_level_all);
		set_main_window_text(title);
		track.release();
	}
	else 
	{
		set_main_window_text("foobar2000"/*core_version_info::g_get_version_string()*/);
	}
	
	
}

void update_systray(bool balloon, int btitle, bool force_balloon)
{
	if (g_icon_created)
	{
		metadb_handle_ptr track;
		static_api_ptr_t<play_control> play_api;
		play_api->get_now_playing(track);
		pfc::string8 sys, title;

		if (track.is_valid())
		{
			
			service_ptr_t<titleformat_object> to_systray;
			static_api_ptr_t<titleformat_compiler>()->compile_safe(to_systray, main_window::config_notification_icon_script.get());
			play_api->playback_format_title_ex(track, 0, title, to_systray, 0, play_control::display_level_titles);
			
			track.release();
			
		}
		else
		{
			title = "foobar2000";//core_version_info::g_get_version_string();
		}

		uFixAmpersandChars(title,sys);
		
		if (balloon && (cfg_balloon||force_balloon))
		{
			uShellNotifyIconEx(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys, "", "");
			uShellNotifyIconEx(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys, (btitle == 0 ? "Now playing:" : (btitle == 1 ? "Unpaused:" : "Paused:")), title);
		}
		else
		uShellNotifyIcon(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys);
		
	}

}

void update_status()
{
	metadb_handle_ptr track;
	static_api_ptr_t<play_control> play_api;
	play_api->get_now_playing(track);
		if (track.is_valid())
		{
			
		service_ptr_t<titleformat_object> to_status;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_status, main_window::config_status_bar_script.get());
		play_api->playback_format_title_ex(track, 0, statusbartext, to_status, NULL, play_control::display_level_all);
		
		track.release();
	} 
	else 
	{
		statusbartext = core_version_info::g_get_version_string();
	}
	status_update_main(false);
}

namespace main_window
{
	config_status_bar_script_t config_status_bar_script;
	config_notification_icon_script_t config_notification_icon_script;
	config_main_window_title_script_t config_main_window_title_script;


	// {2B6EAF5C-970A-4432-B809-12E8CEF6DCDE}
	static const GUID guid_inline_metafield_edit_mode = 
	{ 0x2b6eaf5c, 0x970a, 0x4432, { 0xb8, 0x9, 0x12, 0xe8, 0xce, 0xf6, 0xdc, 0xde } };

	cfg_int_t<t_uint32> cfg_inline_metafield_edit_mode(guid_inline_metafield_edit_mode, config_get_inline_metafield_edit_mode_default_value());

	// {737954D1-1814-4d94-852D-A5CB8D7025ED}
	static const GUID guid_activate_target_playlist_on_dropped_items = 
	{ 0x737954d1, 0x1814, 0x4d94, { 0x85, 0x2d, 0xa5, 0xcb, 0x8d, 0x70, 0x25, 0xed } };

	cfg_bool
		cfg_firstrun(create_guid(0xf7c0139, 0x8698, 0x42bd, 0xb2, 0xe0, 0x3f, 0x59, 0x94, 0xaa, 0xde, 0x43), true),
		cfg_activate_target_playlist_on_dropped_items(guid_activate_target_playlist_on_dropped_items, config_get_activate_target_playlist_on_dropped_items_default_value());

	cfg_int
		cfg_transparency_enabled(create_guid(0xd5ab3806,0x8670,0xf09d,0x47,0xdd,0x1f,0x45,0x05,0x8d,0x83,0x8f), config_get_transparency_enabled_default_value()),
		cfg_transparency_level(create_guid(0x7d50b4ac,0xfcd4,0x0a98,0xd3,0x64,0x58,0xc8,0x2e,0x56,0x19,0xd7), config_get_transparency_level_default_value()),
		cfg_status_show_lock(create_guid(0x0ec529ab,0xa4cb,0x0fa9,0xa1,0xbc,0x7d,0x24,0x56,0xdc,0xb5,0x40), config_get_status_show_lock_default_value());

	void on_transparency_enabled_change()
	{
		if (g_main_window)
		{
			LONG current_style = GetWindowLong(g_main_window, GWL_EXSTYLE);
			if (cfg_transparency_enabled/* && IsWindowVisible(g_main_window)*/)
			{
				if (!(current_style & WS_EX_LAYERED))
				{
					SetWindowLong(g_main_window, 
								GWL_EXSTYLE, 
								current_style | WS_EX_LAYERED);
				}
				on_transparency_level_change();
			}
			else if (current_style & WS_EX_LAYERED)
			{
				SetWindowLong(g_main_window, 
							GWL_EXSTYLE, 
							current_style ^ WS_EX_LAYERED);
				RedrawWindow(g_main_window, 
					NULL, 
					NULL, 
					RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
			}
		}
	}
	void on_transparency_level_change()
	{
		if (g_main_window && cfg_transparency_enabled)
		{
			SetLayeredWindowAttributes(g_main_window, 0, cfg_transparency_level, LWA_ALPHA);
		}
	}
	void config_set_transparency_enabled(bool b_val)
	{
		cfg_transparency_enabled = b_val;
		on_transparency_enabled_change();
	}
	bool config_get_transparency_enabled()
	{
		return cfg_transparency_enabled != 0;
	}
	bool config_get_transparency_enabled_default_value()
	{
		return false;
	}
	bool config_get_is_first_run()
	{
		return cfg_firstrun;
	}
	void config_set_is_first_run()
	{
		cfg_firstrun = false;
	}
	void config_reset_transparency_enabled()
	{
		config_set_transparency_enabled(config_get_transparency_enabled_default_value());
	}
	void config_set_transparency_level(unsigned char b_val)
	{
		cfg_transparency_level = b_val;
		on_transparency_level_change();
	}
	unsigned char config_get_transparency_level()
	{
		return (unsigned char)cfg_transparency_level;
	}
	unsigned char config_get_transparency_level_default_value()
	{
		return 255;
	}
	void config_reset_transparency_level()
	{
		config_set_transparency_level(config_get_transparency_level_default_value());
	}
	void config_set_status_show_lock(bool b_val)
	{
		cfg_status_show_lock = b_val;
		status_bar::set_part_sizes(status_bar::t_part_lock|status_bar::t_part_length|status_bar::t_part_volume);
	}
	bool config_get_status_show_lock()
	{
		return cfg_status_show_lock!=0;
	}
	bool config_get_status_show_lock_default_value()
	{
		return true;
	}
	void config_reset_status_show_lock()
	{
		config_set_status_show_lock(config_get_status_show_lock_default_value());
	}
	bool config_get_activate_target_playlist_on_dropped_items(){return cfg_activate_target_playlist_on_dropped_items;}
	bool config_get_activate_target_playlist_on_dropped_items_default_value(){return true;}
	void config_set_activate_target_playlist_on_dropped_items(bool b_val){cfg_activate_target_playlist_on_dropped_items=b_val;}
	void config_reset_activate_target_playlist_on_dropped_items(){config_set_activate_target_playlist_on_dropped_items(config_get_activate_target_playlist_on_dropped_items_default_value());}

	void config_set_inline_metafield_edit_mode(t_uint32 value)
	{
		cfg_inline_metafield_edit_mode = value;
	}
	t_uint32 config_get_inline_metafield_edit_mode()
	{
		return cfg_inline_metafield_edit_mode;
	}
	void config_reset_inline_metafield_edit_mode()
	{
		config_set_inline_metafield_edit_mode(config_get_inline_metafield_edit_mode_default_value());
	}
	t_uint32 config_get_inline_metafield_edit_mode_default_value(){return mode_columns;};
};


static user_interface_factory<ui_test> fooui;

class control_impl : public columns_ui::control
{
public:
	virtual bool get_string(const GUID & p_guid, pfc::string_base & p_out) const
	{
		if (p_guid == columns_ui::strings::guid_global_variables)
		{
			p_out = cfg_globalstring;
			return true;
		}
		return false;
	}
};

service_factory_single_t<control_impl> g_control_impl;