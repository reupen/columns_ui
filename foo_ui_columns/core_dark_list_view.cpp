#include "pch.h"

#include "core_dark_list_view.h"
#include "dark_mode.h"

namespace cui::helpers {

void CoreDarkListView::notify_on_initialisation()
{
    const auto manager = ui_config_manager::tryGet();

    if (!manager.is_valid())
        return;

    set_use_dark_mode(manager->is_dark_mode());
    set_dark_edit_colours(
        cui::dark::get_dark_system_colour(COLOR_WINDOWTEXT), cui::dark::get_dark_system_colour(COLOR_WINDOW));
    m_dark_mode_status_callback
        = dark::add_status_callback([this] { set_use_dark_mode(dark::is_active_ui_dark(false)); }, false);
}

void CoreDarkListView::notify_on_destroy()
{
    m_dark_mode_status_callback.reset();
}

void CoreDarkListView::render_get_colour_data(ColourData& p_out)
{
    const auto manager = ui_config_manager::tryGet();

    if (!manager.is_valid() || !manager->is_dark_mode()) {
        ListView::render_get_colour_data(p_out);
        return;
    }

    p_out.m_themed = true;
    p_out.m_use_custom_active_item_frame = false;
    p_out.m_text = cui::dark::get_dark_system_colour(COLOR_WINDOWTEXT);
    p_out.m_selection_text = cui::dark::get_dark_system_colour(COLOR_HIGHLIGHTTEXT);
    p_out.m_background = cui::dark::get_dark_system_colour(COLOR_WINDOW);
    p_out.m_selection_background = cui::dark::get_dark_system_colour(COLOR_HIGHLIGHT);
    p_out.m_inactive_selection_text = cui::dark::get_dark_system_colour(COLOR_BTNTEXT);
    p_out.m_inactive_selection_background = cui::dark::get_dark_system_colour(COLOR_BTNFACE);
    p_out.m_active_item_frame = cui::dark::get_dark_system_colour(COLOR_WINDOWFRAME);
    p_out.m_group_text = get_group_text_colour_default();
    p_out.m_group_background = p_out.m_background;
}

} // namespace cui::helpers
