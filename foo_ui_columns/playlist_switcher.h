#ifndef _COLUMNS_UI_PLAYLIST_SWITCHER_H_
#define _COLUMNS_UI_PLAYLIST_SWITCHER_H_

class appearance_client_ps_impl : public cui::colours::client {
public:
    static const GUID g_guid;

    const GUID & get_client_guid() const override { return g_guid; };

    void get_name(pfc::string_base & p_out) const override { p_out = "Playlist Switcher"; };

    t_size get_supported_colours() const override { return cui::colours::colour_flag_all; }; //bit-mask
    t_size get_supported_bools() const override    { return cui::colours::bool_flag_use_custom_active_item_frame; }; //bit-mask
    bool get_themes_supported() const override { return true; };

    void on_colour_changed(t_size mask) const override;
    void on_bool_changed(t_size mask) const override {};
};

namespace playlist_switcher
{

    namespace colours 
    {
        class config_inactive_selection_text_t : public fbh::config_item_t<COLORREF>
        {
        public:
            COLORREF get_default_value () override;
            void on_change() override;
            const GUID & get_guid() override;
            config_inactive_selection_text_t();
        };

        extern config_inactive_selection_text_t config_inactive_selection_text;
    };
};

#endif