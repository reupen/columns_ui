#include "pch.h"

#include "system_appearance_manager.h"

#include "config_appearance.h"
#include "dark_mode.h"
#include "wcs.h"

namespace cui::system_appearance_manager {

namespace {

using namespace winrt::Windows::UI::ViewManagement;

std::optional<ModernColours> modern_colours_cached;
bool modern_colours_fetch_attempted{};
std::optional<bool> dark_mode_available_cached;
std::vector<std::shared_ptr<ModernColoursChangedHandler>> modern_colours_changed_callbacks;
std::vector<std::shared_ptr<DisplayChangedHandler>> display_changed_callbacks;
std::optional<bool> cleartype_enabled_cached;
std::optional<bool> font_smoothing_enabled_cached;

COLORREF winrt_color_to_colorref(const winrt::Windows::UI::Color& colour)
{
    return RGB(colour.R, colour.G, colour.B);
}

void log_winrt_error(std::string_view main_message, const winrt::hresult_error& ex)
{
    const pfc::stringcvt::string_utf8_from_wide error_message(ex.message().c_str());
    const auto message = fmt::format("Columns UI – {}: {}", main_message, error_message.get_ptr());
    console::warning(message.c_str());
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
        log_winrt_error("Error retrieving UISettings colours"sv, ex);
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
        log_winrt_error("Error retrieving HighContrast setting"sv, ex);
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

    void on_cleartype_parameters_changed()
    {
        if (m_window && !m_pending_cleartype_parameters_changed) {
            SetTimer(m_window->get_wnd(), TIMER_CLEARTYPE_PARAMETERS_CHANGED, 50, nullptr);
            m_pending_cleartype_parameters_changed = true;
        }
    }

private:
    static constexpr auto TIMER_CLEARTYPE_PARAMETERS_CHANGED = 1000;

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
                log_winrt_error("Error registering UISettings ColorValuesChanged event handler"sv, ex);
            }
            break;
        case WM_FONTCHANGE:
            g_font_manager_data.dispatch_all_fonts_changed();
            break;
        case WM_DISPLAYCHANGE:
            wcs::reset_colour_profiles();

            for (const auto& callback : display_changed_callbacks)
                (*callback)();
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
                font_smoothing_enabled_cached.reset();
                if (fonts::rendering_mode.get() == WI_EnumValue(fonts::RenderingMode::Automatic))
                    g_font_manager_data.dispatch_rendering_options_changed();
                else
                    g_font_manager_data.dispatch_all_fonts_changed();
                break;
            case SPI_SETFONTSMOOTHINGTYPE:
            case SPI_SETFONTSMOOTHINGCONTRAST:
            case SPI_SETFONTSMOOTHINGORIENTATION:
                // The ClearType tuner seems to send a WM_SETTINGCHANGE message only
                // for SPI_SETFONTSMOOTHINGORIENTATION (and at the end of the wizard at that)
                cleartype_enabled_cached.reset();
                on_cleartype_parameters_changed();
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
        case WM_TIMER:
            if (wp != TIMER_CLEARTYPE_PARAMETERS_CHANGED)
                break;

            if (m_pending_cleartype_parameters_changed) {
                m_pending_cleartype_parameters_changed = false;
                console::print("Columns UI – detected a change to the system ClearType parameters");
                g_font_manager_data.dispatch_all_fonts_changed();
            }

            KillTimer(wnd, TIMER_CLEARTYPE_PARAMETERS_CHANGED);
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
    bool m_pending_cleartype_parameters_changed{};
};

AppearanceMessageWindow message_window;

namespace registry_watcher {

wil::shared_event shutting_down_event;
std::optional<std::jthread> thread;

void start()
{
    if (thread)
        return;

    try {
        shutting_down_event.create(wil::EventOptions::ManualReset, nullptr);

        thread = std::jthread([shutting_down_event = shutting_down_event](auto&&... args) {
            TRACK_CALL_TEXT("cui::system_appearance_manager::registry_watcher::thread");

            (void)mmh::set_thread_description(GetCurrentThread(), L"[Columns UI] DirectWrite registry watcher");

            try {
                const auto hkey = wil::reg::open_unique_key(
                    HKEY_CURRENT_USER, LR"(Software\Microsoft\Avalon.Graphics)", wil::reg::key_access::read);
                wil::unique_event change_event(wil::EventOptions::None);

                auto subscribe = [&] {
                    return RegNotifyChangeKeyValue(hkey.get(), true,
                        REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, change_event.get(), true);
                };

                THROW_IF_WIN32_ERROR(subscribe());

                std::array events = {change_event.get(), shutting_down_event.get()};

                while (true) {
                    const auto wait_result = WaitForMultipleObjectsEx(2, events.data(), false, INFINITE, true);

                    switch (wait_result) {
                    case WAIT_OBJECT_0:
                        if (core_api::are_services_available())
                            fb2k::inMainThread([] { message_window.on_cleartype_parameters_changed(); });

                        THROW_IF_WIN32_ERROR(subscribe());
                        break;
                    case WAIT_IO_COMPLETION:
                        continue;
                    default:
                        return;
                    }
                }
            }
            CATCH_LOG()
        });
    }
    CATCH_LOG()
}

void shut_down()
{
    if (shutting_down_event)
        shutting_down_event.SetEvent();

    thread.reset();

    shutting_down_event.reset();
}

} // namespace registry_watcher

class InitQuit : public initquit {
    void on_init() override {}
    void on_quit() noexcept override
    {
        message_window.deinitialise();
        registry_watcher::shut_down();
    }
};

initquit_factory_t<InitQuit> _;

} // namespace

void initialise()
{
    if (core_api::is_shutting_down())
        return;

    message_window.initialise();
    registry_watcher::start();
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

struct DisplayChangedToken : EventToken {
    explicit DisplayChangedToken(std::shared_ptr<DisplayChangedHandler> event_handler_ptr)
        : m_event_handler_ptr(std::move(event_handler_ptr))
    {
    }
    ~DisplayChangedToken() override { std::erase(display_changed_callbacks, m_event_handler_ptr); }
    std::shared_ptr<DisplayChangedHandler> m_event_handler_ptr;
};

std::unique_ptr<EventToken> add_modern_colours_change_handler(ModernColoursChangedHandler event_handler)
{
    initialise();

    auto event_handler_ptr = std::make_shared<ModernColoursChangedHandler>(std::move(event_handler));
    modern_colours_changed_callbacks.emplace_back(event_handler_ptr);
    return {std::make_unique<ModernColoursChangedToken>(event_handler_ptr)};
}

std::unique_ptr<EventToken> add_display_changed_handler(DisplayChangedHandler event_handler)
{
    initialise();

    auto event_handler_ptr = std::make_shared<DisplayChangedHandler>(std::move(event_handler));
    display_changed_callbacks.emplace_back(event_handler_ptr);
    return {std::make_unique<DisplayChangedToken>(event_handler_ptr)};
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

bool is_font_smoothing_enabled()
{
    if (!font_smoothing_enabled_cached) {
        BOOL font_smoothing_enabled{true};
        SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &font_smoothing_enabled, 0);
        font_smoothing_enabled_cached = font_smoothing_enabled != 0;
    }

    return *font_smoothing_enabled_cached;
}

bool is_cleartype_enabled()
{
    if (!cleartype_enabled_cached) {
        BOOL font_smoothing_type{FE_FONTSMOOTHINGCLEARTYPE};
        SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &font_smoothing_type, 0);
        cleartype_enabled_cached = font_smoothing_type == FE_FONTSMOOTHINGCLEARTYPE;
    }

    return *cleartype_enabled_cached;
}

} // namespace cui::system_appearance_manager
