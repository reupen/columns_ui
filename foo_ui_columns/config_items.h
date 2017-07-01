#pragma once

namespace main_window
{

#if 0
    class config_inline_metafield_edit_mode_t : public config_item_t < t_uint32 >
    {
    public:
        enum metafield_edit_mode_t
        {
            mode_disabled,
            mode_columns,
            mode_windows
        };
        virtual t_uint32 get_default_value();
        virtual void on_change(){};
        virtual const GUID & get_guid();
        config_inline_metafield_edit_mode_t();
    };

    extern config_inline_metafield_edit_mode_t config_inline_metafield_edit_mode;
#endif

    class config_status_bar_script_t : public fbh::config_item_t < pfc::string8 >
    {
    public:
        const char * get_default_value() override;
        void on_change() override;;
        const GUID & get_guid() override;;
        config_status_bar_script_t();;
    };

    class config_notification_icon_script_t : public fbh::config_item_t < pfc::string8 >
    {
    public:
        const char * get_default_value() override;
        void on_change() override;;
        const GUID & get_guid() override;;
        config_notification_icon_script_t();;
    };

    class config_main_window_title_script_t : public fbh::config_item_t < pfc::string8 >
    {
    public:
        const char * get_default_value() override;
        void on_change() override;;
        const GUID & get_guid() override;;
        config_main_window_title_script_t();;
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
};
