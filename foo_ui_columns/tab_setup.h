
#pragma once
#include "config_host.h"

namespace cui::prefs {

PreferencesTab& get_setup_tab();

[[nodiscard]] mmh::EventToken::Ptr add_use_hardware_acceleration_changed_handler(
    mmh::GenericEventHandler event_handler);

} // namespace cui::prefs
