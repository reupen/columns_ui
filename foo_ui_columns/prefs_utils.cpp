#include "stdafx.h"
#include "playlist_view.h"
#include "tab_colours.h"
#include "prefs_utils.h"

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
		playlist_view::g_get_cache().active_make_extra(idx, extra_items, b_date ? &st : nullptr, b_legacy);
		service_ptr_t<titleformat_object> to_temp;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_temp, spec);

		titleformat_hook_set_global<false, true> tf_hook_set_global(extra_items, b_legacy);
		titleformat_hook_date tf_hook_date(&st);

		titleformat_hook_impl_splitter tf_hook(&tf_hook_set_global, b_date ? &tf_hook_date : nullptr);

		//0.9 fallout
		playlist_api->activeplaylist_item_format_title(idx, &tf_hook, temp, to_temp, nullptr, play_control::display_level_all);
		//	if (map) temp.replace_char(6, 3);
		pfc::string_formatter formatter;
		popup_message::g_show(temp, formatter << "Preview of track " << (idx + 1));
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

UINT_PTR CALLBACK choose_font_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
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

BOOL font_picker(LOGFONT & p_font, HWND parent)
{
	modal_dialog_scope scope(parent);

	CHOOSEFONT cf;
	memset(&cf, 0, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = parent;
	cf.lpLogFont = &p_font;
	cf.Flags = CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT/*|CF_ENABLEHOOK*/;
	cf.nFontType = SCREEN_FONTTYPE;
	cf.lCustData = reinterpret_cast<LPARAM>(&scope);
	cf.lpfnHook = choose_font_hook;
	BOOL rv = ChooseFont(&cf);
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

void populate_menu_combo(HWND wnd, unsigned ID, unsigned ID_DESC, const menu_item_identifier & p_item, menu_item_cache & p_cache, bool insert_none)
{
	HWND wnd_combo = GetDlgItem(wnd, ID);

	unsigned n, count = p_cache.get_count();
	pfc::string8_fast_aggressive temp;
	unsigned idx_none = 0;
	if (insert_none)
	{
		idx_none = uSendDlgItemMessageText(wnd, ID, CB_ADDSTRING, 0, "(None)");
		SendMessage(wnd_combo, CB_SETITEMDATA, idx_none, -1);
	}

	unsigned sel = -1;
	pfc::string8 desc;

	for (n = 0; n < count; n++)
	{
		unsigned idx = uSendMessageText(wnd_combo, CB_ADDSTRING, 0, p_cache.get_item(n).m_name);
		SendMessage(wnd_combo, CB_SETITEMDATA, idx, n);

		if (sel == -1 && p_cache.get_item(n) == p_item) { sel = idx; desc = p_cache.get_item(n).m_desc; }
		else if (sel != -1 && idx <= sel) sel++;

		if (insert_none && idx <= idx_none) idx_none++;
	}

	uSendMessageText(wnd_combo, CB_SETCURSEL, sel == -1 && insert_none ? idx_none : sel, nullptr);

	//menu_helpers::get_description(menu_item::TYPE_MAIN, item, desc);
	uSendDlgItemMessageText(wnd, ID_DESC, WM_SETTEXT, 0, desc);

}

void on_menu_combo_change(HWND wnd, LPARAM lp, cfg_menu_item & cfg_menu_store, menu_item_cache & p_cache, unsigned ID_DESC)
{
	HWND wnd_combo = (HWND)lp;

	pfc::string8 temp;
	unsigned cache_idx = SendMessage(wnd_combo, CB_GETITEMDATA, SendMessage(wnd_combo, CB_GETCURSEL, 0, 0), 0);

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


