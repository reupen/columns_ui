#include "pch.h"
#include "buttons.h"
#include "list_view_drop_target.h"

namespace cui::toolbars::buttons {

void ButtonsToolbar::ConfigParam::ButtonsList::notify_on_initialisation()
{
    CoreDarkListView::notify_on_initialisation();
    set_selection_mode(SelectionMode::SingleRelaxed);
    set_columns({{"Name", 225_spx}, {"Type", 150_spx}});
}
void ButtonsToolbar::ConfigParam::ButtonsList::notify_on_create()
{
    wil::com_ptr drop_target(new utils::SimpleListViewDropTarget(
        this, [this](mmh::Permutation& new_order, size_t old_index, size_t new_index) {
            mmh::destructive_reorder(m_param.m_buttons, new_order);

            m_param.m_selection = &m_param.m_buttons[new_index];

            const size_t first_index = std::min(old_index, new_index);
            const size_t last_index = std::max(old_index, new_index);
            m_param.refresh_buttons_list_items(first_index, last_index - first_index + 1);
        }));

    RegisterDragDrop(get_wnd(), drop_target.get());
}
void ButtonsToolbar::ConfigParam::ButtonsList::notify_on_destroy()
{
    RevokeDragDrop(get_wnd());
}
void ButtonsToolbar::ConfigParam::ButtonsList::notify_on_selection_change(
    const bit_array& p_affected, const bit_array& p_status, notification_source_t p_notification_source)
{
    size_t index = get_selected_item_single();
    m_param.on_selection_change(index);
}

bool ButtonsToolbar::ConfigParam::ButtonsList::do_drag_drop(WPARAM wp)
{
    DWORD drop_effect{DROPEFFECT_NONE};
    const auto data_object = utils::create_simple_list_view_data_object(get_wnd());

    LOG_IF_FAILED(uih::ole::do_drag_drop(get_wnd(), wp, data_object.get(), DROPEFFECT_MOVE, NULL, &drop_effect));

    return true;
}

} // namespace cui::toolbars::buttons
