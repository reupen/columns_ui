/**
* Columns UI foobar2000 component
* 
* \author musicmusic
*/

#include "stdafx.h"

const TCHAR * main_window_class_name = _T("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}");

GUID null_guid = 
{ 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

#include <strsafe.h>

rebar_window * g_rebar_window = 0;

layout_window g_layout_window;


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
	g_status=0;

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

//UINT g_seek_timer = 0;

unsigned g_switch_playlist = 0;


HICON g_icon=0;

static RECT tooltip;

pfc::string8
	statusbartext,
	windowtext
	;


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
	//cfg_playlist_sidebar_left_sep(create_guid(0x9e812607,0xb455,0xe066,0xfc,0xfb,0x0a,0x2b,0x9c,0xef,0x2f,0x3c),0),
	cfg_playlist_sidebar_tooltips(create_guid(0x0f6ad5ac,0x7409,0x151e,0xe0,0xd7,0x61,0xe5,0xd8,0x53,0x9b,0x4c),1),
	cfg_playlist_switcher_use_tagz(create_guid(0x43d94259,0x4fde,0x8779,0x8e,0xde,0x18,0xca,0xa7,0x2f,0xc5,0x7b),0),
	cfg_sidebar_use_custom_show_delay(create_guid(0x59e44486,0x2dcc,0x1276,0x83,0x84,0xcf,0xcb,0xc0,0x4f,0xb4,0x1c),0),
	cfg_sidebar_show_delay(create_guid(0xad46e438,0x269d,0xe1c8,0x4a,0xe0,0x08,0x29,0xb5,0x5d,0x2f,0xc3),0),
	cfg_sidebar_hide_delay(create_guid(0xab13b5dc,0x1baa,0x937a,0x85,0x33,0x2d,0x58,0x35,0xc8,0xe7,0xa8),0),
	cfg_toolbars(create_guid(0xfd1f165e,0xeb6e,0xbb2d,0xe0,0xd7,0xc5,0x03,0xee,0xf1,0x6d,0xd7),1),
	cfg_show_sort_arrows(create_guid(0xa9f1111e,0x68d6,0xc707,0xd4,0xad,0x09,0x63,0x72,0x0b,0xe9,0xfa),1),
	cfg_playlist_date(create_guid(0xe0f9c009,0x89f2,0x4a6e,0xdd,0xb5,0x10,0x30,0x0c,0xc3,0x74,0xce),0),
	cfg_autoswitch_delay(create_guid(0x11645dab,0xf8a6,0xdc4e,0x1d,0xc0,0xce,0x50,0xb1,0x27,0xb5,0xbb),500),
	//cfg_scar_hidden(create_guid(0x59a3a04e,0xfd16,0x2345,0xe3,0x30,0x8b,0x4d,0xb5,0x96,0x77,0x3f),0),
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
	/* There was some misbehaviour with the newer messages. So we don't use them. */
	//	if (!g_icon_created)
	//		win32_helpers::uShellNotifyIcon(NIM_SETVERSION, g_main_window, 1, NOTIFYICON_VERSION, MSG_NOTICATION_ICON, g_icon, "foobar2000"/*core_version_info::g_get_version_string()*/);
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

user_interface::HookProc_t main_window::g_hookproc;



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

mmh::comptr_t<ITaskbarList3> main_window::g_ITaskbarList3;


/** FUCKO: ITaskbarList3::ThumbBarUpdateButtons calls SendMessageTimeout without SMTO_BLOCK flag */
void g_update_taskbar_buttons_delayed(bool b_init)
{
	if (g_main_window)
		PostMessage(g_main_window, MSG_UPDATE_THUMBBAR, b_init, NULL);
}

void g_update_taskbar_buttons_now(bool b_init)
{
	if (main_window::g_ITaskbarList3.is_valid())
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
			tb[i].iId = taskbar_buttons::ID_FIRST + i;
			tb[i].iBitmap =bitmap_indices[i];
			wcscpy_s(tb[i].szTip, tabsize(tb[i].szTip), ttips[i]);
			//if (tb[i].iId == ID_STOP && !b_is_playing)
			//	tb[i].dwFlags |= THBF_DISABLED;
		}

		if (b_init)
			main_window::g_ITaskbarList3->ThumbBarAddButtons(g_main_window, tabsize(tb), tb);
		else
			main_window::g_ITaskbarList3->ThumbBarUpdateButtons(g_main_window, tabsize(tb), tb);
	}
}

cui::main_window_t cui::g_main_window;

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

		main_window::g_hookproc = hook;

	
		WNDCLASS  wc;
		memset(&wc,0,sizeof(wc));
		
		create_icon_handle();
		
		wc.lpfnWndProc = (WNDPROC)g_MainWindowProc;
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
		status_bar::menudesc = p_text;
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