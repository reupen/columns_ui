#include "pch.h"

#include "icons.h"
#include "resource_utils.h"
#include "svg.h"

namespace cui::button_items {

template <const GUID& MenuItemID, const icons::IconConfig* icon_config>
class MenuItemButtonWithIcon : public uie::button_v2 {
    const GUID& get_item_guid() const override { return MenuItemID; }

    HANDLE get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, unsigned cx_hint, unsigned cy_hint,
        unsigned& handle_type) const override
    {
        if (!icon_config)
            return nullptr;

        const auto cx = gsl::narrow<int>(cx_hint);
        const auto cy = gsl::narrow<int>(cy_hint);

        if (icons::use_svg_icon(cx, cy)) {
            handle_type = handle_type_bitmap;
            return render_svg(*icon_config, cx, cy).release();
        }

        handle_type = handle_type_icon;
        return load_icon(*icon_config, cx, cy).release();
    }
};

uie::button_factory<MenuItemButtonWithIcon<standard_commands::guid_main_open, &icons::built_in::open>> _button_open;
uie::button_factory<MenuItemButtonWithIcon<standard_commands::guid_main_play, &icons::built_in::play>> _button_play;
uie::button_factory<MenuItemButtonWithIcon<standard_commands::guid_main_pause, &icons::built_in::pause>> _button_pause;
uie::button_factory<MenuItemButtonWithIcon<standard_commands::guid_main_next, &icons::built_in::next>> _button_next;
uie::button_factory<MenuItemButtonWithIcon<standard_commands::guid_main_previous, &icons::built_in::previous>>
    _button_previous;
uie::button_factory<MenuItemButtonWithIcon<standard_commands::guid_main_random, &icons::built_in::random>>
    _button_random;
uie::button_factory<MenuItemButtonWithIcon<standard_commands::guid_main_stop, &icons::built_in::stop>> _button_stop;

template <const GUID& MenuItemID, const GUID& ConfigObjectID, const cui::icons::IconConfig* icon_config>
class MenuItemToggleButton : public MenuItemButtonWithIcon<MenuItemID, icon_config> {
public:
    class ConfigObjectNotify : public config_object_notify {
    public:
        size_t get_watched_object_count() override { return 1; }
        GUID get_watched_object(size_t p_index) override { return ConfigObjectID; }

        void on_watched_object_changed(const service_ptr_t<config_object>& p_object) override
        {
            try {
                bool value{};
                p_object->get_data_bool(value);
                s_on_state_change(value);
            } catch (exception_io const&) {
            }
        }
    };

    static void s_on_state_change(bool b_enabled)
    {
        for (auto&& button : m_buttons) {
            for (auto&& callback : button->m_callbacks) {
                callback->on_button_state_change(
                    b_enabled ? uie::BUTTON_STATE_PRESSED | uie::BUTTON_STATE_ENABLED : uie::BUTTON_STATE_DEFAULT);
            }
        }
    }

    MenuItemToggleButton() { m_buttons.emplace_back(this); }
    ~MenuItemToggleButton() { std::erase(m_buttons, this); }

    MenuItemToggleButton(const MenuItemToggleButton&) = delete;
    MenuItemToggleButton& operator=(const MenuItemToggleButton&) = delete;
    MenuItemToggleButton(MenuItemToggleButton&&) = delete;
    MenuItemToggleButton& operator=(MenuItemToggleButton&&) = delete;

private:
    unsigned get_button_state() const override
    {
        return config_object::g_get_data_bool_simple(ConfigObjectID, false)
            ? uie::BUTTON_STATE_PRESSED | uie::BUTTON_STATE_ENABLED
            : uie::BUTTON_STATE_DEFAULT;
    }

    void register_callback(uie::button_callback& p_callback) override { m_callbacks.emplace_back(&p_callback); }
    void deregister_callback(uie::button_callback& p_callback) override { std::erase(m_callbacks, &p_callback); }

    std::vector<uie::button_callback*> m_callbacks;
    inline static std::vector<MenuItemToggleButton*> m_buttons;
};

template <const GUID& MenuItemID, const GUID& ConfigObjectID, const cui::icons::IconConfig* icon_config = nullptr>
class MenuItemToggleButtonFactory {
    uie::button_factory<MenuItemToggleButton<MenuItemID, ConfigObjectID, icon_config>> button;
    service_factory_t<typename MenuItemToggleButton<MenuItemID, ConfigObjectID, icon_config>::ConfigObjectNotify>
        notify;
};

MenuItemToggleButtonFactory<standard_commands::guid_main_stop_after_current,
    standard_config_objects::bool_playlist_stop_after_current, &icons::built_in::stop_after_current>
    _stop_after_current_1;
MenuItemToggleButtonFactory<standard_commands::guid_main_playback_follows_cursor,
    standard_config_objects::bool_playback_follows_cursor>
    _playback_follows_cursor;
MenuItemToggleButtonFactory<standard_commands::guid_main_cursor_follows_playback,
    standard_config_objects::bool_cursor_follows_playback>
    _cursor_follows_playback;
MenuItemToggleButtonFactory<standard_commands::guid_main_always_on_top, standard_config_objects::bool_ui_always_on_top>
    _always_on_top;

class ButtonBlank : public ui_extension::custom_button {
    const GUID& get_item_guid() const override
    {
        // {A8FE61BA-A055-4a53-A588-9DDA92ED7312}
        static const GUID guid = {0xa8fe61ba, 0xa055, 0x4a53, {0xa5, 0x88, 0x9d, 0xda, 0x92, 0xed, 0x73, 0x12}};
        return guid;
    }
    HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask& p_mask_type,
        COLORREF& cr_mask, HBITMAP& bm_mask) const override
    {
        COLORMAP map;
        map.from = 0x0;
        map.to = cr_btntext;

        p_mask_type = ui_extension::MASK_BITMAP;
        HBITMAP bm = LoadMonoBitmap(IDB_BLANK, cr_btntext);
        if (bm)
            bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_BLANK), IMAGE_BITMAP, 0, 0,
                LR_DEFAULTSIZE | LR_MONOCHROME);
        return bm;
    }
    void get_name(pfc::string_base& p_out) const override { p_out = "Blanking button"; }
    unsigned get_button_state() const override { return NULL; }
    void execute(const pfc::list_base_const_t<metadb_handle_ptr>& p_items) override {}
    uie::t_button_guid get_guid_type() const override { return uie::BUTTON_GUID_BUTTON; }
};

ui_extension::button_factory<ButtonBlank> g_blank;

#if 0
/** Prototype stop button that was disabled when not playing. Was abandoned. */
class button_stop : public ui_extension::button//, play_callback
{
    virtual const GUID & get_item_guid()const
    {
        return standard_commands::guid_main_stop;
    }
    virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
    {

        //COLORMAP map;
        //map.from = 0x0;
        //map.to = cr_btntext;

        HBITMAP bm = NULL;

        DLLVERSIONINFO2 dvi;
        if (SUCCEEDED(uih::get_comctl32_version(dvi)) && dvi.info1.dwMajorVersion >= 6)
        {
            p_mask_type = uie::MASK_NONE;
            bm = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP32), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
        }
        else
        {
            p_mask_type = ui_extension::MASK_BITMAP;
            bm = //CreateMappedBitmap(core_api::get_my_instance(), IDB_STOP, 0, &map, 1);
                //(HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
                LoadMonoBitmap(IDB_STOP, cr_btntext);
            if (bm)
                bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
        }

        return bm;
    }
#if 0
    virtual void register_callback(uie::button_callback & p_callback)
    {
        m_callback = &p_callback;
        play_callback_manager::get()->register_callback(this, play_callback::flag_on_playback_all, false);
    };
    virtual void deregister_callback(uie::button_callback & p_callback)
    {
        assert(m_callback == &p_callback);
        m_callback = NULL;
        play_callback_manager::get()->unregister_callback(this);
    };
    virtual void FB2KAPI on_playback_starting(play_control::t_track_command p_command, bool p_paused){};
    virtual void FB2KAPI on_playback_new_track(metadb_handle_ptr p_track)
    {
        if (m_callback) m_callback->on_button_state_change(uie::BUTTON_STATE_DEFAULT);
    };
    virtual void FB2KAPI on_playback_stop(play_control::t_stop_reason p_reason)
    {
        if (m_callback) m_callback->on_button_state_change(NULL);
    };
    virtual void FB2KAPI on_playback_seek(double p_time){};
    virtual void FB2KAPI on_playback_pause(bool p_state){};
    virtual void FB2KAPI on_playback_edited(metadb_handle_ptr p_track){};
    virtual void FB2KAPI on_playback_dynamic_info(const file_info & p_info){};
    virtual void FB2KAPI on_playback_dynamic_info_track(const file_info & p_info){};
    virtual void FB2KAPI on_playback_time(double p_time){};
    virtual void FB2KAPI on_volume_change(float p_new_val){};
    uie::button_callback * m_callback;
public:
    button_stop() : m_callback(NULL) {};
#endif
};

ui_extension::button_factory<button_stop> g_stop;
#endif

#if 0
/** Prototype next button with drop-down list for next 10 tracks. Was abandoned. */
class button_next : public ui_extension::button
{
    virtual const GUID & get_item_guid()const
    {
        return standard_commands::guid_main_next;
    }
    virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
    {
        COLORMAP map;
        map.from = 0x0;
        map.to = cr_btntext;

        HBITMAP bm = NULL;

        DLLVERSIONINFO2 dvi;
        if (SUCCEEDED(uih::get_comctl32_version(dvi)) && dvi.info1.dwMajorVersion >= 6)
        {
            p_mask_type = uie::MASK_NONE;
            bm = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NEXT32), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
        }
        else
        {
            p_mask_type = ui_extension::MASK_BITMAP;
            bm = LoadMonoBitmap(IDB_NEXT, cr_btntext);//CreateMappedBitmap(core_api::get_my_instance(), IDB_NEXT, 0, &map, 1);
            if (bm)
                bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NEXT), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_MONOCHROME);
        }return bm;
    }
#if 0
    virtual void get_menu_items(uie::menu_hook_t & p_out)
    {
        class menu_node_play_item : public ui_extension::menu_node_leaf
        {
            unsigned m_index, m_playlist;
            service_ptr_t<playlist_manager> m_playlist_api;
            service_ptr_t<play_control> m_play_api;
        public:
            virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)
            {
                metadb_handle_ptr p_handle;
                m_playlist_api->playlist_get_item_handle(p_handle, m_playlist, m_index);
                if (p_handle.is_valid())
                    p_handle->query_meta_field("TITLE", 0, p_out);
                p_displayflags = 0;
                return true;
            }
            virtual bool get_description(pfc::string_base & p_out)
            {
                return false;
            }
            virtual void execute()
            {
                m_playlist_api->playlist_set_playback_cursor(m_playlist, m_index);
                m_play_api->play_start(play_control::TRACK_COMMAND_SETTRACK);
            }
            menu_node_play_item(unsigned pl, size_t p_index)
                : m_playlist(pl), m_index(p_index)
            {
                playlist_manager::g_get(m_playlist_api);
                play_control::g_get(m_play_api);
            };
        };
        unsigned cursor, playlist;
        const auto api = playlist_manager::get();
        if (!api->get_playing_item_location(&playlist, &cursor))
        {
            playlist = api->get_active_playlist();
            cursor = api->playlist_get_playback_cursor(playlist);
        }
        if (config_object::g_get_data_bool_simple(standard_config_objects::bool_playback_follows_cursor, false))
        {
            cursor = api->activeplaylist_get_focus_item();
            playlist = api->get_active_playlist();
        }

        unsigned n, count = api->playlist_get_item_count(playlist);
        for (n = 0; n<10 && n + cursor<count; n++)
            p_out.add_node(uie::menu_node_ptr(new menu_node_play_item(playlist, cursor + n)));
    }
#endif
};

ui_extension::button_factory<button_next> g_next;
#endif

} // namespace cui::button_items
