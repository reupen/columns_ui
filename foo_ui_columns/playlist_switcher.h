#ifndef _COLUMNS_UI_PLAYLIST_SWITCHER_H_
#define _COLUMNS_UI_PLAYLIST_SWITCHER_H_

class appearance_client_ps_impl : public cui::colours::client {
public:
	static const GUID g_guid;

	virtual const GUID & get_client_guid() const { return g_guid; };
	virtual void get_name(pfc::string_base & p_out) const { p_out = "Playlist Switcher"; };

	virtual t_size get_supported_colours() const { return cui::colours::colour_flag_all; }; //bit-mask
	virtual t_size get_supported_fonts() const { return 0; }; //bit-mask
	virtual t_size get_supported_bools() const { return cui::colours::bool_flag_use_custom_active_item_frame; }; //bit-mask
	virtual bool get_themes_supported() const { return true; };

	virtual void on_colour_changed(t_size mask) const;
	virtual void on_font_changed(t_size mask) const {};
	virtual void on_bool_changed(t_size mask) const {};
};

namespace playlist_switcher
{

	namespace colours 
	{
		class config_inactive_selection_text_t : public config_item_t<COLORREF>
		{
		public:
			virtual COLORREF get_default_value ();
			virtual void on_change();
			virtual const GUID & get_guid();
			config_inactive_selection_text_t();
		};

		extern config_inactive_selection_text_t config_inactive_selection_text;
	};
};

#endif