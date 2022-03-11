#include "pch.h"

#include "config_appearance.h"
#include "dark_mode.h"
#include "system_appearance_manager.h"

namespace cui {

namespace {

int get_system_colour_id(const colours::colour_identifier_t colour_id)
{
    switch (colour_id) {
    case colours::colour_text:
        return COLOR_WINDOWTEXT;
    case colours::colour_selection_text:
        return COLOR_HIGHLIGHTTEXT;
    case colours::colour_background:
        return COLOR_WINDOW;
    case colours::colour_selection_background:
        return COLOR_HIGHLIGHT;
    case colours::colour_inactive_selection_text:
        return COLOR_BTNTEXT;
    case colours::colour_inactive_selection_background:
        return COLOR_BTNFACE;
    case colours::colour_active_item_frame:
        return COLOR_WINDOWFRAME;
    default:
        uBugCheck();
    }
}

class ColourManagerInstance : public colours::manager_instance {
public:
    explicit ColourManagerInstance(const GUID& p_client_guid)
    {
        g_colour_manager_data.find_by_guid(p_client_guid, m_entry);
        g_colour_manager_data.find_by_guid(pfc::guid_null, m_global_entry);
    }
    COLORREF get_colour(const colours::colour_identifier_t& p_identifier) const override
    {
        system_appearance_manager::initialise();
        ColourManagerData::entry_ptr_t p_entry
            = m_entry->colour_mode == colours::colour_mode_global ? m_global_entry : m_entry;
        if (p_entry->colour_mode == colours::colour_mode_system
            || p_entry->colour_mode == colours::colour_mode_themed) {
            const auto system_colour_id = get_system_colour_id(p_identifier);
            return dark::get_system_colour(system_colour_id, colours::is_dark_mode_active());
        }
        switch (p_identifier) {
        case colours::colour_text:
            return p_entry->text;
        case colours::colour_selection_text:
            return p_entry->selection_text;
        case colours::colour_background:
            return p_entry->background;
        case colours::colour_selection_background:
            return p_entry->selection_background;
        case colours::colour_inactive_selection_text:
            return p_entry->inactive_selection_text;
        case colours::colour_inactive_selection_background:
            return p_entry->inactive_selection_background;
        case colours::colour_active_item_frame:
            return p_entry->active_item_frame;
        default:
            return 0;
        }
    }

    bool get_bool(const colours::bool_identifier_t& p_identifier) const override
    {
        ColourManagerData::entry_ptr_t p_entry
            = m_entry->colour_mode == colours::colour_mode_global ? m_global_entry : m_entry;
        switch (p_identifier) {
        case colours::bool_use_custom_active_item_frame:
            return p_entry->use_custom_active_item_frame;
        case colours::bool_dark_mode_enabled:
            if (!system_appearance_manager::is_dark_mode_available())
                return false;

            if (colours::dark_mode_status.get() == WI_EnumValue(cui::colours::DarkModeStatus::Enabled))
                return true;

            if (colours::dark_mode_status.get() == WI_EnumValue(cui::colours::DarkModeStatus::UseSystemSetting))
                return system_appearance_manager::is_dark_mode_enabled();

            return false;
        default:
            return false;
        }
    }

    bool get_themed() const override
    {
        return m_entry->colour_mode == colours::colour_mode_themed
            || (m_entry->colour_mode == colours::colour_mode_global
                && m_global_entry->colour_mode == colours::colour_mode_themed);
    }

private:
    ColourManagerData::entry_ptr_t m_entry;
    ColourManagerData::entry_ptr_t m_global_entry;
};

class ColourManager : public colours::manager {
public:
    void create_instance(const GUID& p_client_guid, colours::manager_instance::ptr& p_out) override
    {
        p_out = new service_impl_t<ColourManagerInstance>(p_client_guid);
    }
    void register_common_callback(colours::common_callback* p_callback) override
    {
        g_colour_manager_data.register_common_callback(p_callback);
    }
    void deregister_common_callback(colours::common_callback* p_callback) override
    {
        g_colour_manager_data.deregister_common_callback(p_callback);
    }
};

service_factory_single_t<ColourManager> _;

} // namespace

} // namespace cui
