#include "stdafx.h"

void preview_to_console(const char * spec, bool extra)
{
	static_api_ptr_t<playlist_manager> playlist_api;

	int count = playlist_api->activeplaylist_get_item_count();

	if (!count) popup_message::g_show("Activate a non-empty playlist and try again", "No track to preview");
	else
	{
		int idx = playlist_api->activeplaylist_get_focus_item();
		if (idx >= count) idx = count - 1;
		if (idx < 0) idx = 0;

		pfc::string8 temp;

		bool b_legacy = cfg_oldglobal != 0;
		bool b_date = cfg_playlist_date != 0;
		SYSTEMTIME st;
		if (b_date) GetLocalTime(&st);

		global_variable_list extra_items;
		playlist_view::g_get_cache().active_make_extra(idx, extra_items, b_date ? &st : 0, b_legacy);
		service_ptr_t<titleformat_object> to_temp;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_temp, spec);

		//0.9 fallout
		playlist_api->activeplaylist_item_format_title(idx, &titleformat_hook_impl_splitter(extra ? &titleformat_hook_set_global<false, true>(extra_items, b_legacy) : 0, b_date ? &titleformat_hook_date(&st) : 0), temp, to_temp, 0, play_control::display_level_all);
		//	if (map) temp.replace_char(6, 3);
		popup_message::g_show(temp, pfc::string8() << "Preview of track " << (idx + 1));
	}
	//console::popup();
}

void colour_code_gen(HWND parent, UINT edit, bool markers, bool init)
{
	COLORREF COLOR = g_last_colour;
	COLORREF COLORS[16] = { g_last_colour, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	if (init || uChooseColor(&COLOR, parent, &COLORS[0]))
	{
		g_last_colour = COLOR;

		pfc::string_formatter text;
		text << "$rgb(" << unsigned(COLOR & 0xff) << "," << unsigned(COLOR >> 8 & 0xff) << "," << unsigned(COLOR >> 16 & 0xff) << ")";

		uSendDlgItemMessageText(parent, edit, WM_SETTEXT, 0, text);
	}
}

bool colour_picker(HWND wnd, cfg_int & out, COLORREF custom)
{
	bool rv = false;
	COLORREF COLOR = out;
	COLORREF COLORS[16] = { custom, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	if (uChooseColor(&COLOR, wnd, &COLORS[0]))
	{
		out = COLOR;
		rv = true;
	}
	return rv;
}

bool colour_picker(HWND wnd, COLORREF & out, COLORREF custom)
{
	bool rv = false;
	COLORREF COLOR = out;
	COLORREF COLORS[16] = { custom, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	if (uChooseColor(&COLOR, wnd, &COLORS[0]))
	{
		out = COLOR;
		rv = true;
	}
	return rv;
}

bool colour_picker2(HWND wnd, config_item_t<COLORREF> & p_out, COLORREF custom)
{
	bool rv = false;
	COLORREF COLOR = p_out.get();
	COLORREF COLORS[16] = { custom, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	if (uChooseColor(&COLOR, wnd, &COLORS[0]))
	{
		p_out.set(COLOR);
		rv = true;
	}
	return rv;
}

bool font_picker(HWND wnd, cfg_struct_t<LOGFONT> & out)
{
	bool rv = false;
	LOGFONT temp = out;
	if (font_picker(temp, wnd))
	{
		out = temp;
		rv = true;
	}
	return rv;
}

