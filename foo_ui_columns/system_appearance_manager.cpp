#include "pch.h"

#include "system_appearance_manager.h"

#include "config_appearance.h"
#include "dark_mode.h"

namespace cui::system_appearance_manager {

namespace {

using namespace winrt::Windows::UI::ViewManagement;

std::optional<ModernColours> modern_colours_cached;
bool modern_colours_fetch_attempted{};
std::optional<bool> dark_mode_available_cached;
std::vector<std::shared_ptr<ModernColoursChangedHandler>> modern_colours_changed_callbacks;

COLORREF winrt_color_to_colorref(const winrt::Windows::UI::Color& colour)
{
    return RGB(colour.R, colour.G, colour.B);
}

void log_winrt_error(std::basic_string_view<char8_t> main_message, const winrt::hresult_error& ex)
{
    const pfc::stringcvt::string_utf8_from_wide error_message(ex.message().c_str());
    const auto message
        = fmt::format(u8"Columns UI – {}: {}", main_message, reinterpret_cast<const char8_t*>(error_message.get_ptr()));
    console::warning(reinterpret_cast<const char*>(message.c_str()));
}

std::optional<ModernColours> fetch_modern_colours()
{
    try {
        const UISettings settings;
        const auto background = winrt_color_to_colorref(settings.GetColorValue(UIColorType::Background));
        const auto foreground = winrt_color_to_colorref(settings.GetColorValue(UIColorType::Foreground));
        const auto accent = winrt_color_to_colorref(settings.GetColorValue(UIColorType::Accent));
        const auto light_accent = winrt_color_to_colorref(settings.GetColorValue(UIColorType::AccentLight1));

        return ModernColours{background, foreground, accent, light_accent};
    } catch (const winrt::hresult_error& ex) {
        log_winrt_error(u8"Error retrieving UISettings colours"sv, ex);
        return {};
    }
}

void reset_modern_colours()
{
    modern_colours_fetch_attempted = false;
    modern_colours_cached.reset();
}

bool fetch_dark_mode_available()
{
    if (!dark::does_os_support_dark_mode() || !IsThemeActive() || !IsAppThemed())
        return false;

    try {
        if (AccessibilitySettings().HighContrast())
            return false;
    } catch (const winrt::hresult_error& ex) {
        log_winrt_error(u8"Error retrieving HighContrast setting"sv, ex);
    }

    return true;
}

void reset_dark_mode_available()
{
    dark_mode_available_cached.reset();
}

void handle_modern_colours_changed()
{
    std::optional<bool> old_dark_mode_available;
    std::optional<bool> old_dark_mode_enabled;

    if (dark_mode_available_cached)
        old_dark_mode_available = dark_mode_available_cached;

    if (modern_colours_cached && dark_mode_available_cached)
        old_dark_mode_enabled = is_dark_mode_enabled();

    reset_modern_colours();
    reset_dark_mode_available();

    if (old_dark_mode_enabled && *old_dark_mode_enabled != is_dark_mode_enabled()
        && colours::handle_system_dark_mode_status_change()) {
        // A dark mode status change was handled, so don't bother sending any
        // other notifications
        return;
    }

    if (old_dark_mode_available && *old_dark_mode_available != is_dark_mode_available()
        && colours::handle_system_dark_mode_availability_change()) {
        // A dark mode status change was handled, so don't bother sending any
        // other notifications
        return;
    }

    for (auto&& callback : modern_colours_changed_callbacks)
        (*callback)();
}

class AppearanceMessageWindow {
public:
    void initialise()
    {
        if (!m_window) {
            uie::container_window_v3_config config(L"{BDCEC7A3-7230-4671-A5F7-B19A989DCA81}", false);
            config.window_styles = 0;
            config.extended_window_styles = 0;

            m_window = std::make_unique<uie::container_window_v3>(
                config, [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });

            m_window->create(nullptr);
        }
    }

    void deinitialise()
    {
        if (m_window)
            m_window->destroy();
        m_window.reset();
    }

private:
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_CREATE:
            if (!dark::does_os_support_dark_mode())
                break;

            try {
                m_ui_settings = UISettings();
                m_colours_changed_token = m_ui_settings->ColorValuesChanged(
                    [](auto&&, auto&&) { fb2k::inMainThread([] { handle_modern_colours_changed(); }); });
            } catch (const winrt::hresult_error& ex) {
                log_winrt_error(u8"Error registering UISettings ColorValuesChanged event handler"sv, ex);
            }
            break;
        case WM_FONTCHANGE:
            g_font_manager_data.dispatch_all_fonts_changed();
            break;
        case WM_SYSCOLORCHANGE: {
            if (colours::is_dark_mode_active())
                break;

            ColoursClientList m_colours_client_list;
            ColoursClientList::g_get_list(m_colours_client_list);
            size_t count = m_colours_client_list.get_count();
            bool b_global_custom
                = g_colour_manager_data.get_global_entry()->colour_set.colour_scheme == colours::ColourSchemeCustom;
            if (!b_global_custom)
                colours::common_colour_callback_manager.s_on_common_colour_changed(colours::colour_flag_all);
            for (size_t i = 0; i < count; i++) {
                const auto p_data = g_colour_manager_data.get_entry(m_colours_client_list[i].m_guid);
                if (p_data->colour_set.colour_scheme == colours::ColourSchemeSystem
                    || p_data->colour_set.colour_scheme == colours::ColourSchemeThemed
                    || (p_data->colour_set.colour_scheme == colours::ColourSchemeGlobal && !b_global_custom)) {
                    m_colours_client_list[i].m_ptr->on_colour_changed(colours::colour_flag_all);
                }
            }
        } break;
        case WM_SETTINGCHANGE:
            switch (wp) {
            case SPI_SETFONTSMOOTHING:
                if (fonts::rendering_mode.get() == WI_EnumValue(fonts::RenderingMode::Automatic))
                    g_font_manager_data.dispatch_rendering_options_changed();
                break;
            case SPI_SETICONTITLELOGFONT:
                if (!g_font_manager_data.m_common_items_entry
                    || g_font_manager_data.m_common_items_entry->font_mode != fonts::FontMode::System)
                    break;

                g_font_manager_data.dispatch_common_font_changed(fonts::font_type_flag_items);

                for (auto client_ptr : fonts::client::enumerate()) {
                    const auto entry = g_font_manager_data.find_by_id(client_ptr->get_client_guid());

                    if (entry->font_mode == fonts::FontMode::CommonItems)
                        g_font_manager_data.dispatch_client_font_changed(client_ptr);
                }
                break;
            case SPI_SETNONCLIENTMETRICS: {
                if (!g_font_manager_data.m_common_labels_entry
                    || g_font_manager_data.m_common_labels_entry->font_mode != fonts::FontMode::System)
                    break;

                g_font_manager_data.dispatch_common_font_changed(fonts::font_type_flag_labels);

                for (auto client_ptr : fonts::client::enumerate()) {
                    const auto entry = g_font_manager_data.find_by_id(client_ptr->get_client_guid());

                    if (entry->font_mode == fonts::FontMode::CommonLabels)
                        g_font_manager_data.dispatch_client_font_changed(client_ptr);
                }
                break;
            }
            }
            break;
        case WM_NCDESTROY:
            if (m_colours_changed_token) {
                m_ui_settings->ColorValuesChanged(m_colours_changed_token);
                m_colours_changed_token = {};
            }
            m_ui_settings.reset();
            break;
        }
        return DefWindowProc(wnd, msg, wp, lp);
    }

    std::unique_ptr<uie::container_window_v3> m_window;
    std::optional<UISettings> m_ui_settings;
    winrt::event_token m_colours_changed_token;
};

AppearanceMessageWindow message_window;

class InitQuit : public initquit {
    void on_init() override {}
    void on_quit() noexcept override { message_window.deinitialise(); }
};

initquit_factory_t<InitQuit> _;

} // namespace

void initialise()
{
    message_window.initialise();
}

std::optional<ModernColours> get_modern_colours()
{
    initialise();

    if (!modern_colours_fetch_attempted)
        modern_colours_cached = fetch_modern_colours();

    modern_colours_fetch_attempted = true;
    return modern_colours_cached;
}

struct ModernColoursChangedToken : EventToken {
    explicit ModernColoursChangedToken(std::shared_ptr<ModernColoursChangedHandler> event_handler_ptr)
        : m_event_handler_ptr(std::move(event_handler_ptr))
    {
    }
    ~ModernColoursChangedToken() override { std::erase(modern_colours_changed_callbacks, m_event_handler_ptr); }
    std::shared_ptr<ModernColoursChangedHandler> m_event_handler_ptr;
};

std::unique_ptr<EventToken> add_modern_colours_change_handler(ModernColoursChangedHandler event_handler)
{
    initialise();

    auto event_handler_ptr = std::make_shared<ModernColoursChangedHandler>(std::move(event_handler));
    modern_colours_changed_callbacks.emplace_back(event_handler_ptr);
    return {std::make_unique<ModernColoursChangedToken>(event_handler_ptr)};
}

bool is_dark_mode_available()
{
    initialise();

    if (!dark_mode_available_cached)
        dark_mode_available_cached = fetch_dark_mode_available();

    return *dark_mode_available_cached;
}

bool is_dark_mode_enabled()
{
    initialise();

    if (!is_dark_mode_available())
        return false;

    const auto modern_colours = get_modern_colours();

    if (!modern_colours)
        return false;

    // Infer dark mode status as there is no public API for Win32 apps to determine it
    return modern_colours->is_dark();
}

} // namespace cui::system_appearance_manager
