#include "stdafx.h"
#include "playlist_switcher_v2.h"
#include "main_window.h"

//cfg_int cfg_playlist_panel_delete(create_guid(0x264215a8,0x1f5e,0x58cd,0x80,0x4c,0x12,0xc1,0x2c,0xf2,0xff,0xe3),1);
cfg_struct_t<LOGFONT> cfg_plist_font(create_guid(0xd61b2f01,0xa845,0xc1f5,0x99,0x37,0x26,0xab,0x74,0x70,0xf5,0x0f),get_icon_font());

// {70A5C273-67AB-4bb6-B61C-F7975A6871FD}
const GUID g_guid_playlist_switcher_font = 
{ 0x70a5c273, 0x67ab, 0x4bb6, { 0xb6, 0x1c, 0xf7, 0x97, 0x5a, 0x68, 0x71, 0xfd } };

class font_client_switcher : public cui::fonts::client
{
public:
	const GUID & get_client_guid() const override
	{
		return g_guid_playlist_switcher_font;
	}
	void get_name (pfc::string_base & p_out) const override
	{
		p_out = "Playlist Switcher";
	}

	cui::fonts::font_type_t get_default_font_type() const override
	{
		return cui::fonts::font_type_items;
	}

	void on_font_changed() const override 
	{
		playlist_switcher_t::g_on_font_items_change();
	}
};

font_client_switcher::factory<font_client_switcher> g_font_client_switcher;

// {EB38A997-3B5F-4126-8746-262AA9C1F94B}
const GUID appearance_client_ps_impl::g_guid = 
{ 0xeb38a997, 0x3b5f, 0x4126, { 0x87, 0x46, 0x26, 0x2a, 0xa9, 0xc1, 0xf9, 0x4b } };

appearance_client_ps_impl::factory<appearance_client_ps_impl> g_appearance_client_ps_impl;


namespace playlist_switcher
{

	namespace colours 
	{
		COLORREF config_inactive_selection_text_t::get_default_value ()
		{
			return ::get_default_colour(::colours::COLOUR_SELECTED_TEXT_NO_FOCUS);
		}
		void config_inactive_selection_text_t::on_change(){};
		const GUID & config_inactive_selection_text_t::get_guid()
		{
			// {4262FCDA-C345-40fa-9DCA-1A1AA91B4C1C}
			static const GUID ret = 
			{ 0x4262fcda, 0xc345, 0x40fa, { 0x9d, 0xca, 0x1a, 0x1a, 0xa9, 0x1b, 0x4c, 0x1c } };
			return ret;
		}
		config_inactive_selection_text_t::config_inactive_selection_text_t()
			: config_item_t(get_guid(), get_default_value())
		{};
		config_inactive_selection_text_t config_inactive_selection_text;
	};
}

void appearance_client_ps_impl::on_colour_changed(t_size mask) const
{
	playlist_switcher_t::g_redraw_all();
}
