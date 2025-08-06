
#pragma once
#include "config_host.h"

namespace cui::prefs {

PreferencesTab& get_setup_tab();

[[nodiscard]] std::unique_ptr<EventToken> add_use_hardware_acceleration_changed_handler(
    GenericEventHandler event_handler);

} // namespace cui::prefs
