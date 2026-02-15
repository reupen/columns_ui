#include "pch.h"

#include "dark_mode_active_ui.h"

#include "main_window.h"

namespace cui::dark {

namespace {

class DarkModeStatusChangedToken : public mmh::EventToken {
public:
    DarkModeStatusChangedToken(std::function<void()> callback, bool allow_cui_fallback);
    void on_dark_mode_status_change() const { m_callback(); }

private:
    std::function<void()> m_callback;
    std::unique_ptr<class UIConfigCallback> m_ui_config_callback;
    std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;
};

class UIConfigCallback : public ui_config_callback {
public:
    explicit UIConfigCallback(DarkModeStatusChangedToken* callback_object) : m_callback_object(callback_object)
    {
        if (m_manager.is_valid())
            m_manager->add_callback(this);
    }

    ~UIConfigCallback()
    {
        if (m_manager.is_valid())
            m_manager->remove_callback(this);
    }

    bool is_valid() const { return m_manager.is_valid(); }
    void ui_fonts_changed() override {}
    void ui_colors_changed() override { m_callback_object->on_dark_mode_status_change(); }

private:
    DarkModeStatusChangedToken* m_callback_object{};
    ui_config_manager::ptr m_manager{ui_config_manager::tryGet()};
};

DarkModeStatusChangedToken::DarkModeStatusChangedToken(std::function<void()> callback, bool allow_cui_fallback)
    : m_callback(std::move(callback))
{
    m_ui_config_callback = std::make_unique<UIConfigCallback>(this);

    if (!m_ui_config_callback->is_valid() && allow_cui_fallback)
        m_dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>([this] { m_callback(); });
}

} // namespace

bool is_active_ui_dark(bool allow_cui_fallback)
{
    const auto manager = ui_config_manager::tryGet();

    if (manager.is_valid())
        return manager->is_dark_mode();

    return allow_cui_fallback && main_window.get_wnd() && colours::is_dark_mode_active();
}

mmh::EventToken::Ptr add_status_callback(std::function<void()> callback, bool allow_cui_fallback)
{
    return std::make_unique<DarkModeStatusChangedToken>(std::move(callback), allow_cui_fallback);
}

} // namespace cui::dark
