#include "pch.h"

#include "core_dark_list_view.h"
#include "dark_mode.h"

namespace cui::helpers {

void CoreDarkListView::notify_on_initialisation()
{
    set_use_dark_mode(dark::is_active_ui_dark(m_allow_cui_fallback));
    set_dark_edit_colours(dark::get_dark_system_colour(COLOR_WINDOWTEXT), dark::get_dark_system_colour(COLOR_WINDOW));
    m_dark_mode_status_callback = dark::add_status_callback(
        [this] { set_use_dark_mode(dark::is_active_ui_dark(m_allow_cui_fallback)); }, m_allow_cui_fallback);
}

void CoreDarkListView::notify_on_destroy()
{
    m_dark_mode_status_callback.reset();
}

void CoreDarkListView::render_get_colour_data(ColourData& p_out)
{
    if (!dark::is_active_ui_dark(m_allow_cui_fallback)) {
        ListView::render_get_colour_data(p_out);
        return;
    }

    p_out.m_themed = true;
    p_out.m_use_custom_active_item_frame = false;
    p_out.m_text = dark::get_dark_system_colour(COLOR_WINDOWTEXT);
    p_out.m_selection_text = dark::get_dark_system_colour(COLOR_HIGHLIGHTTEXT);
    p_out.m_background = dark::get_dark_system_colour(COLOR_WINDOW);
    p_out.m_selection_background = dark::get_dark_system_colour(COLOR_HIGHLIGHT);
    p_out.m_inactive_selection_text = dark::get_dark_system_colour(COLOR_BTNTEXT);
    p_out.m_inactive_selection_background = dark::get_dark_system_colour(COLOR_BTNFACE);
    p_out.m_active_item_frame = dark::get_dark_system_colour(COLOR_WINDOWFRAME);
    p_out.m_group_text = get_group_text_colour_default();
    p_out.m_group_background = p_out.m_background;
}

} // namespace cui::helpers
