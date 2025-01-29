#include "pch.h"

#include "migrate.h"

#include "artwork.h"
#include "config_appearance.h"
#include "layout.h"
#include "status_pane.h"

namespace cui::migrate {

namespace {

namespace v100 {

cfg_bool has_migrated_to_v100({0xba7516e5, 0xd1f1, 0x4784, {0xa6, 0x93, 0x62, 0x72, 0x37, 0xc3, 0x7e, 0x9c}}, false);

constexpr GUID legacy_playlist_view_id{0xf20bed8f, 0x225b, 0x46c3, {0x9f, 0xc7, 0x45, 0x4c, 0xed, 0xb6, 0xcd, 0xad}};

bool replace_legacy_playlist(uie::splitter_item_t* splitter_item)
{
    if (splitter_item->get_panel_guid() == legacy_playlist_view_id) {
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

cfg_bool has_migrated_status_pane(
    {0x1577e396, 0xf6bb, 0x4086, {0x93, 0xfc, 0x46, 0xd7, 0x4f, 0xc5, 0xac, 0xc4}}, false);

void migrate_status_pane()
{
    if (has_migrated_status_pane)
        return;

    has_migrated_status_pane = true;

    if (main_window::config_get_is_first_run())
        return;

    status_pane::double_click_action = cfg_statusdbl;
    status_pane::status_pane_script = main_window::config_status_bar_script.get();
}

cfg_bool has_migrated_custom_colours(
    {0x6541170b, 0xc305, 0x4ae5, {0xa4, 0x84, 0x3c, 0x2, 0xcb, 0xf6, 0x2c, 0x7e}}, false);

void migrate_custom_colours_entry(const colours::Entry::Ptr& light_entry)
{
    if (light_entry->colour_set.colour_scheme != colours::ColourSchemeCustom)
        return;

    auto default_colour_set = create_default_colour_set(false, colours::ColourSchemeCustom);
    default_colour_set.use_custom_active_item_frame = light_entry->colour_set.use_custom_active_item_frame;

    if (light_entry->colour_set == default_colour_set)
        return;

    const auto dark_entry = g_colour_manager_data.get_entry(light_entry->id, true);

    dark_entry->colour_set = light_entry->colour_set;
}

void migrate_custom_colours()
{
    if (has_migrated_custom_colours)
        return;

    has_migrated_custom_colours = true;

    if (main_window::config_get_is_first_run())
        return;

    migrate_custom_colours_entry(g_colour_manager_data.m_global_light_entry);

    for (auto&& entry : g_colour_manager_data.m_light_entries)
        migrate_custom_colours_entry(entry);
}

} // namespace v200

namespace v300 {

fbh::ConfigInt32 legacy_rendering_mode(
    {0x3168bfe2, 0x5c40, 0x4d5b, {0x96, 0xe7, 0xd3, 0x95, 0xa7, 0xf0, 0x4d, 0x67}}, DWRITE_RENDERING_MODE_DEFAULT);

cfg_bool has_migrated_rendering_mode(
    {0xbfa0d1d0, 0x138f, 0x4a33, {0xa8, 0x4e, 0xd5, 0x79, 0xfe, 0xcd, 0x7e, 0xde}}, false);

void migrate_rendering_mode()
{
    if (has_migrated_rendering_mode)
        return;

    has_migrated_rendering_mode = true;

    const auto new_value = [] {
        switch (static_cast<DWRITE_RENDERING_MODE>(legacy_rendering_mode.get())) {
        default:
            return fonts::RenderingMode::Automatic;
        case DWRITE_RENDERING_MODE_ALIASED:
            return fonts::RenderingMode::GdiAliased;
        case DWRITE_RENDERING_MODE_GDI_CLASSIC:
            return fonts::RenderingMode::GdiClassic;
        case DWRITE_RENDERING_MODE_GDI_NATURAL:
            return fonts::RenderingMode::GdiNatural;
        case DWRITE_RENDERING_MODE_NATURAL:
            return fonts::RenderingMode::Natural;
        case DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC:
            return fonts::RenderingMode::NaturalSymmetric;
        }
    }();

    fonts::rendering_mode = WI_EnumValue(new_value);
}

} // namespace v300

} // namespace

void migrate_all()
{
    v100::migrate();
    v200::migrate_status_pane();
    v200::migrate_custom_colours();
    v300::migrate_rendering_mode();
}

void apply_first_run_defaults()
{
    if (!main_window::config_get_is_first_run())
        return;

    colours::dark_mode_status.set(WI_EnumValue(colours::DarkModeStatus::UseSystemSetting));

    if (!cfg_layout.get_presets().get_count())
        cfg_layout.reset_presets();

    if (fb2k::imageViewer::ptr api; fb2k::imageViewer::tryGet(api)) {
        artwork_panel::click_action = WI_EnumValue(artwork_panel::ClickAction::open_image_viewer);
    }
}

} // namespace cui::migrate
