#include "pch.h"
#include "buttons.h"
#include "list_view_drop_target.h"
#include "permutation_utils.h"

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
        this, [this](size_t old_index, size_t new_index) { move_item(old_index, new_index); }));

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

void ButtonsToolbar::ConfigParam::ButtonsList::move_selection(int delta)
{
    const auto selection_index = fbh::as_optional(get_selected_item_single());

    if (!selection_index)
        return;

    const auto new_index = std::clamp(
        gsl::narrow<ptrdiff_t>(*selection_index) + delta, ptrdiff_t{}, gsl::narrow<ptrdiff_t>(get_item_count() - 1));

    move_item(*selection_index, gsl::narrow<size_t>(new_index));
    set_item_selected_single(new_index, false);
    ensure_visible(new_index);
}

void ButtonsToolbar::ConfigParam::ButtonsList::move_item(size_t old_index, size_t new_index)
{
    auto permutation = utils::create_shift_item_permutation(old_index, new_index, get_item_count());
    mmh::destructive_reorder(m_param.m_buttons, permutation);

    m_param.m_selection = &m_param.m_buttons[new_index];

    const size_t first_index = std::min(old_index, new_index);
    const size_t last_index = std::max(old_index, new_index);
    m_param.refresh_buttons_list_items(first_index, last_index - first_index + 1);
}

} // namespace cui::toolbars::buttons
