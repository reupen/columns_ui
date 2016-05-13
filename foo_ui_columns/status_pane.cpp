#include "stdafx.h"

#include "main_window.h"
#include "status_pane.h"

// {522E01C6-EA7C-49f2-AE5E-702B8C6B4B24}
const GUID status_pane::g_guid_font = 
{ 0x522e01c6, 0xea7c, 0x49f2, { 0xae, 0x5e, 0x70, 0x2b, 0x8c, 0x6b, 0x4b, 0x24 } };

class font_client_status_pane : public cui::fonts::client
{
public:
	const GUID & get_client_guid() const override
	{
		return status_pane::g_guid_font;
	}
	void get_name (pfc::string_base & p_out) const override
	{
		p_out = "Status Pane";
	}

	cui::fonts::font_type_t get_default_font_type() const override
	{
		return cui::fonts::font_type_labels;
	}

	void on_font_changed() const override 
	{
		g_status_pane.on_font_changed();
	}
};

font_client_status_pane::factory<font_client_status_pane> g_font_client_status_pane;

void status_pane::on_font_changed()
{
	m_font = cui::fonts::helper(g_guid_font).get_font();
	size_windows();
}

void status_pane::render_background (HDC dc, const RECT & rc)
{
	RECT rc_top = rc, rc_bottom= rc;
	rc_top.top += 4;
	rc_top.bottom = rc_top.top + (RECT_CY(rc)-6) / 2;
	rc_bottom.top = rc_top.bottom;
	rc_bottom.bottom -= 2;

	COLORREF cr = GetSysColor(COLOR_BTNFACE);
	COLORREF cr2 = GetSysColor(COLOR_3DDKSHADOW);
	if (0 && m_theme)
	{
		//GetThemeColor(m_theme, 1, 0, TMT_EDGEHIGHLIGHTCOLOR, &cr);
		//GetThemeColor(m_theme, 1, 0, TMT_EDGEDKSHADOWCOLOR, &cr2);
		//GetThemeColor(m_theme, 1, 0, TMT_BTNFACE, &cr);
	}
	FillRect(dc, &rc, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(cr)));

	if (m_theme)
	{
		COLORREF cr_back = cr2;
		Gdiplus::Color cr_end(0,LOBYTE(LOWORD(cr_back)),HIBYTE(LOWORD(cr_back)),LOBYTE(HIWORD(cr_back)));
		Gdiplus::Color cr_start(33,LOBYTE(LOWORD(cr_back)),HIBYTE(LOWORD(cr_back)),LOBYTE(HIWORD(cr_back)));
		Gdiplus::Rect rect(rc.left, rc.top, RECT_CX(rc), 2);
		Gdiplus::LinearGradientBrush lgb(rect, cr_start, cr_end, Gdiplus::LinearGradientModeVertical);
		Gdiplus::Graphics(dc).FillRectangle(&lgb, rect);
	}
	else
	{
		RECT rcl = {0, 0, rc.right, 1};
		FillRect(dc, &rcl, GetSysColorBrush(COLOR_3DLIGHT));
	}
}

void status_pane::get_length_data(bool & p_selection, t_size & p_count, pfc::string_base & p_out)
{
	metadb_handle_list_t<pfc::alloc_fast_aggressive> sels;
	double length=0;

	static_api_ptr_t<playlist_manager> playlist_api;
	static_api_ptr_t<metadb> metadb_api;

	unsigned count = playlist_api->activeplaylist_get_selection_count(pfc_infinite);
	bool b_selection = count > 0;
	if (count == 0)
		count = playlist_api->activeplaylist_get_item_count();

	sels.prealloc(count);

	if (b_selection)
		playlist_api->activeplaylist_get_selected_items(sels);
	else
		playlist_api->activeplaylist_get_all_items(sels);

	length = sels.calc_total_duration();

	p_out = pfc::format_time_ex(length,0);
	p_count = count;
	p_selection = b_selection;
}
