#pragma once

namespace main_window {
extern fbh::ConfigString config_status_bar_script;
extern fbh::ConfigString config_notification_icon_script;
extern fbh::ConfigString config_main_window_title_script;

enum MetafieldEditingMode { mode_disabled, mode_columns };

void config_set_inline_metafield_edit_mode(uint32_t value);
uint32_t config_get_inline_metafield_edit_mode();
uint32_t config_get_inline_metafield_edit_mode_default_value();
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
} // namespace main_window
