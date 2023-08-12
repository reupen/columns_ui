#include "pch.h"
#include "status_pane.h"
#include "status_bar.h"
#include "notification_area.h"
#include "main_window.h"

extern bool g_icon_created;

namespace cui {

namespace {

constexpr GUID core_colours_client_id = {0xbbd0468d, 0x48e6, 0x4cf3, {0xa4, 0xf4, 0xc7, 0x87, 0x4c, 0xb6, 0x1d, 0x8b}};
constexpr GUID core_console_font_client_id
    = {0xe9f4d060, 0x6feb, 0x4626, {0xb2, 0x5d, 0xba, 0x09, 0x0f, 0x75, 0xf7, 0xa5}};
constexpr GUID core_default_font_client_id
    = {0x1adbc094, 0x3b35, 0x4bab, {0x82, 0xd5, 0x3b, 0xdd, 0x4a, 0x5c, 0x6a, 0xf9}};
constexpr GUID core_lists_font_client_id
    = {0xfc3cb3a8, 0x72db, 0x4448, {0xb1, 0x2c, 0x2e, 0xf7, 0x6a, 0x15, 0xd7, 0x53}};

class UIConfigManagerImpl : public ui_config_manager {
public:
    void on_colours_changed()
    {
        for (const auto callback : m_callbacks)
            callback->ui_colors_changed();
    }

    void on_fonts_changed()
    {
        // Keep previous handles alive while callbacks are being handled
        auto _ = std::move(m_font_cache);

        for (const auto callback : m_callbacks)
            callback->ui_fonts_changed();
    }

    void add_callback(ui_config_callback* callback) override { m_callbacks.emplace_back(callback); }

    void remove_callback(ui_config_callback* callback) override { std::erase(m_callbacks, callback); }

    bool query_color(const GUID& p_what, t_ui_color& p_out) override
    {
        if (p_what == ui_color_darkmode) {
            p_out = m_colours.is_dark_mode_active() ? 0 : RGB(255, 255, 255);
            return true;
        }

        const auto colour_id = [&p_what]() -> std::optional<colours::colour_identifier_t> {
            if (p_what == ui_color_text)
                return colours::colour_text;

            if (p_what == ui_color_background)
                return colours::colour_background;

            if (p_what == ui_color_selection)
                return colours::colour_selection_background;

            return {};
        }();

        if (!colour_id)
            return false;

        p_out = m_colours.get_colour(*colour_id);

        return m_colours.is_dark_mode_active() || !m_colours.get_themed();
    }

    t_ui_font query_font(const GUID& p_what) override
    {
        if (m_font_cache.contains(p_what)) {
            return m_font_cache[p_what].get();
        }

        LOGFONT lf{};
        if (p_what == ui_font_lists || p_what == ui_font_playlists) {
            m_lists_font.get_font(lf);
        } else if (p_what == ui_font_console) {
            m_console_font.get_font(lf);
        } else {
            m_default_font.get_font(lf);
        }

        m_font_cache.insert_or_assign(p_what, wil::unique_hfont(CreateFontIndirect(&lf)));

        return m_font_cache.at(p_what).get();
    }

private:
    colours::helper m_colours{core_colours_client_id};
    fonts::helper m_default_font{core_default_font_client_id};
    fonts::helper m_console_font{core_console_font_client_id};
    fonts::helper m_lists_font{core_lists_font_client_id};
    std::unordered_map<GUID, wil::unique_hfont> m_font_cache;
    std::vector<ui_config_callback*> m_callbacks;
};

service_ptr_t<UIConfigManagerImpl> ui_config_manager_impl;

class CoreColoursClient : public colours::client {
public:
    const GUID& get_client_guid() const override { return core_colours_client_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Core"; }
    uint32_t get_supported_colours() const override
    {
        return colours::colour_flag_background | colours::colour_flag_selection_background | colours::colour_flag_text;
    }
    uint32_t get_supported_bools() const override { return 0; }
    bool get_themes_supported() const override { return false; }
    void on_colour_changed(uint32_t changed_items_mask) const override
    {
        if (ui_config_manager_impl.is_valid())
            ui_config_manager_impl->on_colours_changed();
    }
    void on_bool_changed(uint32_t changed_items_mask) const override
    {
        if (ui_config_manager_impl.is_valid())
            ui_config_manager_impl->on_colours_changed();
    }
};

class CoreConsoleFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return core_console_font_client_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Core: Console"; }
    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_labels; }
    void on_font_changed() const override
    {
        if (ui_config_manager_impl.is_valid())
            ui_config_manager_impl->on_fonts_changed();
    }
};

class CoreDefaultFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return core_default_font_client_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Core: Default"; }
    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_labels; }
    void on_font_changed() const override
    {
        if (ui_config_manager_impl.is_valid())
            ui_config_manager_impl->on_fonts_changed();
    }
};

class CoreListsFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return core_lists_font_client_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Core: List items"; }
    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }
    void on_font_changed() const override
    {
        if (ui_config_manager_impl.is_valid())
            ui_config_manager_impl->on_fonts_changed();
    }
};

class UserInterfaceImpl : public user_interface_v3 {
public:
    const char* get_name() override { return "Columns UI"; }

    HWND init(HookProc_t hook) override { return main_window.initialise(hook); }

    GUID get_guid() override { return {0xf12d0a24, 0xa8a4, 0x4618, {0x96, 0x59, 0x6f, 0x66, 0xde, 0x6, 0x75, 0x24}}; }

    void show_now_playing() override
    {
        auto play_api = play_control::get();
        update_systray(true, play_api->is_paused() ? 2 : 0, true);
    }
    void shutdown() override
    {
        main_window.shutdown();
        ui_config_manager_impl.reset();
    }

    void activate() override
    {
        if (main_window.get_wnd()) {
            cfg_go_to_tray = false;
            if (g_icon_created && !cfg_show_systray)
                destroy_systray_icon();

            if (!is_visible()) {
                ShowWindow(main_window.get_wnd(), SW_RESTORE);
                if ((GetWindowLong(main_window.get_wnd(), GWL_EXSTYLE) & WS_EX_LAYERED))
                    RedrawWindow(main_window.get_wnd(), nullptr, nullptr,
                        RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
            }
            SetForegroundWindow(main_window.get_wnd());
        }
    }
    void hide() override
    {
        if (main_window.get_wnd()) {
            ShowWindow(main_window.get_wnd(), SW_MINIMIZE);
        }
    }
    bool is_visible() override
    {
        bool rv = false;
        if (main_window.get_wnd()) {
            rv = IsWindowVisible(main_window.get_wnd()) && !IsIconic(main_window.get_wnd());
        }
        return rv;
    }
    void override_statusbar_text(const char* p_text) override
    {
        status_bar::set_menu_item_description(p_text);
        status_pane::g_status_pane.enter_menu_mode(p_text);
    }
    void revert_statusbar_text() override
    {
        status_bar::clear_menu_item_description();
        status_pane::g_status_pane.exit_menu_mode();
    }

    bool query_capability(const GUID& cap) override
    {
        main_window.on_query_capability();
        if (cap == cap_suppress_core_shellhook)
            return false;
        if (cap == cap_suppress_core_uvc)
            return false;
        // The SDK documentation does not say what to do when a GUID for an unknown capability is encountered.
        // We return false (which is apparently what the Default UI does).
        return false;
    }

    service_ptr_t<ui_config_manager> get_config_manager() override
    {
        if (ui_config_manager_impl.is_empty())
            ui_config_manager_impl = fb2k::service_new<UIConfigManagerImpl>();

        return ui_config_manager_impl;
    }
};

user_interface_factory<UserInterfaceImpl> _user_interface_impl;
colours::client::factory<CoreColoursClient> _core_colours_client;
fonts::client::factory<CoreDefaultFontClient> _core_default_font_client;
fonts::client::factory<CoreConsoleFontClient> _core_console_font_client;
fonts::client::factory<CoreListsFontClient> _core_lists_font_client;

} // namespace

} // namespace cui
