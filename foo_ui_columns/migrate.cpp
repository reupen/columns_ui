#include "pch.h"

#include "migrate.h"
#include "config_appearance.h"
#include "layout.h"
#include "main_window.h"
#include "vis_spectrum.h"

namespace cui::migrate {

namespace v100 {

cfg_bool has_migrated_to_v100({0xba7516e5, 0xd1f1, 0x4784, {0xa6, 0x93, 0x62, 0x72, 0x37, 0xc3, 0x7e, 0x9c}}, false);

bool replace_legacy_playlist(uie::splitter_item_t* splitter_item)
{
#pragma warning(suppress : 4996)
    if (splitter_item->get_panel_guid() == panels::guid_playlist_view) {
        splitter_item->set_panel_guid(panels::guid_playlist_view_v2);
        return true;
    }

    uie::window_ptr window;
    uie::splitter_window_ptr splitter;

    if (!uie::window::create_by_guid(splitter_item->get_panel_guid(), window))
        return false;

    if (!window->service_query_t(splitter))
        return false;

    auto modified = false;
    stream_writer_memblock writer;
    abort_callback_dummy aborter;
    splitter_item->get_panel_config(&writer);
    window->set_config_from_ptr(writer.m_data.get_ptr(), writer.m_data.get_size(), aborter);

    const auto child_count = splitter->get_panel_count();
    for (size_t i{0}; i < child_count; ++i) {
        uie::splitter_item_ptr child_splitter_item;
        splitter->get_panel(i, child_splitter_item);

        if (replace_legacy_playlist(child_splitter_item.get_ptr())) {
            splitter->replace_panel(i, child_splitter_item.get_ptr());
            modified = true;
        }
    }

    if (modified) {
        writer.m_data.set_size(0);
        splitter->get_config(&writer, aborter);
        splitter_item->set_panel_config_from_ptr(writer.m_data.get_ptr(), writer.m_data.get_size());
    }
    return modified;
}

bool replace_legacy_playlist_in_all_presets()
{
    auto preset_modified = false;
    auto&& presets = cfg_layout.get_presets();
    const auto count = presets.get_count();
    for (size_t i{0}; i < count; ++i) {
        uie::splitter_item_ptr splitter_item;
        presets[i].get(splitter_item);
        if (replace_legacy_playlist(splitter_item.get_ptr())) {
            cfg_layout.set_preset(i, splitter_item.get_ptr());
            preset_modified = true;
        }
    }
    return preset_modified;
}

void migrate()
{
    if (!has_migrated_to_v100) {
        if (!main_window::config_get_is_first_run())
            replace_legacy_playlist_in_all_presets();
        has_migrated_to_v100 = true;
    }
}

} // namespace v100

namespace v200 {

cfg_bool has_migrated_spectrum_analyser_colours(
    {0x2ce47e0, 0xd964, 0x4f16, {0x83, 0x57, 0xd1, 0x1f, 0xb4, 0x43, 0xf2, 0x58}}, false);

cfg_int cfg_legacy_spectrum_analyser_background_colour(
    GUID{0x2bb960d2, 0xb1a8, 0x5741, {0x55, 0xb6, 0x13, 0x3f, 0xb1, 0x80, 0x37, 0x88}},
    get_default_colour(::colours::COLOUR_BACK));
cfg_int cfg_legacy_spectrum_analyser_foreground_colour(
    GUID{0x421d3d3f, 0x5289, 0xb1e4, {0x9b, 0x91, 0xab, 0x51, 0xd3, 0xad, 0xbc, 0x4d}},
    get_default_colour(::colours::COLOUR_TEXT));

void migrate()
{
    if (has_migrated_spectrum_analyser_colours)
        return;

    has_migrated_spectrum_analyser_colours = true;

    if (main_window::config_get_is_first_run())
        return;

    if (cfg_legacy_spectrum_analyser_background_colour == get_default_colour(::colours::COLOUR_BACK)
        && cfg_legacy_spectrum_analyser_foreground_colour == get_default_colour(::colours::COLOUR_TEXT))
        return;

    ColourManagerData::entry_ptr_t colours_entry;
    g_colour_manager_data.find_by_guid(toolbars::spectrum_analyser::colour_client_id, colours_entry);

    colours_entry->colour_mode = colours::colour_mode_custom;
    colours_entry->background = cfg_legacy_spectrum_analyser_background_colour;
    colours_entry->text = cfg_legacy_spectrum_analyser_foreground_colour;
}

} // namespace v200

void migrate_all()
{
    v100::migrate();
    v200::migrate();
}

} // namespace cui::migrate
