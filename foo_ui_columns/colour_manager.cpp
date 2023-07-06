#include "pch.h"

#include "colour_utils.h"
#include "config_appearance.h"
#include "dark_mode.h"
#include "system_appearance_manager.h"

namespace cui {

namespace {

class ColourManagerInstance : public colours::manager_instance {
public:
    explicit ColourManagerInstance(const GUID& p_client_guid)
        : m_light_entry(g_colour_manager_data.get_entry(p_client_guid, false))
        , m_dark_entry(g_colour_manager_data.get_entry(p_client_guid, true))
        , m_global_light_entry(g_colour_manager_data.get_global_entry(false))
        , m_global_dark_entry(g_colour_manager_data.get_global_entry(true))
    {
    }

    COLORREF get_colour(const colours::colour_identifier_t& p_identifier) const override
    {
        system_appearance_manager::initialise();
        const auto entry = active_entry();

        const auto is_custom_active_item_frame
            = p_identifier == colours::colour_active_item_frame && entry->colour_set.use_custom_active_item_frame;

        if ((entry->colour_set.colour_scheme == colours::ColourSchemeSystem
                || entry->colour_set.colour_scheme == colours::ColourSchemeThemed)
            && !is_custom_active_item_frame) {
            const auto system_colour_id = get_system_colour_id(p_identifier);
            return dark::get_system_colour(system_colour_id, colours::is_dark_mode_active());
        }
        switch (p_identifier) {
        case colours::colour_text:
            return entry->colour_set.text;
        case colours::colour_selection_text:
            return entry->colour_set.selection_text;
        case colours::colour_background:
            return entry->colour_set.background;
        case colours::colour_selection_background:
            return entry->colour_set.selection_background;
        case colours::colour_inactive_selection_text:
            return entry->colour_set.inactive_selection_text;
        case colours::colour_inactive_selection_background:
            return entry->colour_set.inactive_selection_background;
        case colours::colour_active_item_frame:
            return entry->colour_set.active_item_frame;
        default:
            return 0;
        }
    }

    bool get_bool(const colours::bool_identifier_t& p_identifier) const override
    {
        switch (p_identifier) {
        case colours::bool_use_custom_active_item_frame: {
            return active_entry()->colour_set.use_custom_active_item_frame;
        }
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

    bool get_themed() const override { return active_entry()->colour_set.colour_scheme == colours::ColourSchemeThemed; }

private:
    [[nodiscard]] colours::Entry::Ptr active_entry() const
    {
        auto& global_entry = colours::is_dark_mode_active() ? m_global_dark_entry : m_global_light_entry;
        auto& entry = colours::is_dark_mode_active() ? m_dark_entry : m_light_entry;
        return entry->colour_set.colour_scheme == colours::ColourSchemeGlobal ? global_entry : entry;
    }

    colours::Entry::Ptr m_light_entry;
    colours::Entry::Ptr m_dark_entry;
    colours::Entry::Ptr m_global_light_entry;
    colours::Entry::Ptr m_global_dark_entry;
};

class ColourManager : public colours::manager {
public:
    void create_instance(const GUID& p_client_guid, colours::manager_instance::ptr& p_out) override
    {
        p_out = new service_impl_t<ColourManagerInstance>(p_client_guid);
    }
    void register_common_callback(colours::common_callback* p_callback) override
    {
        colours::common_colour_callback_manager.register_common_callback(p_callback);
    }
    void deregister_common_callback(colours::common_callback* p_callback) override
    {
        colours::common_colour_callback_manager.deregister_common_callback(p_callback);
    }
};

service_factory_single_t<ColourManager> _;

} // namespace

} // namespace cui
