#include "pch.h"
#include "item_properties.h"
#include "list_view_drop_target.h"
#include "permutation_utils.h"

namespace cui::panels::item_properties {

void FieldsList::notify_save_inline_edit(const char* value)
{
    if (m_edit_index < m_fields.get_count()) {
        (m_edit_column ? m_fields[m_edit_index].m_name : m_fields[m_edit_index].m_name_friendly) = value;
        pfc::list_t<SizedInsertItem<2, 0>> items;
        items.set_count(1);
        items[0].m_subitems[0] = m_fields[m_edit_index].m_name_friendly;
        items[0].m_subitems[1] = m_fields[m_edit_index].m_name;
        replace_items(m_edit_index, items);
    }
    m_edit_column = pfc_infinite;
    m_edit_index = pfc_infinite;
}

bool FieldsList::notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
    pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries)
{
    size_t indices_count = indices.get_count();
    if (indices_count == 1 && indices[0] < m_fields.get_count()) {
        m_edit_index = indices[0];
        m_edit_column = column;

        p_text = m_edit_column ? m_fields[m_edit_index].m_name : m_fields[m_edit_index].m_name_friendly;

        if (m_edit_column == 1)
            p_flags = inline_edit_uppercase;

        return true;
    }
    return false;
}

bool FieldsList::notify_before_create_inline_edit(
    const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse)
{
    return column <= 1 && indices.get_count() == 1;
}

void FieldsList::notify_on_create()
{
    set_selection_mode(SelectionMode::SingleRelaxed);
    set_columns({{"Name", 125_spx}, {"Field", 125_spx}});

    size_t count = m_fields.get_count();
    pfc::list_t<InsertItem> items;
    get_insert_items(0, count, items);
    insert_items(0, count, items.get_ptr());

    wil::com_ptr drop_target(new utils::SimpleListViewDropTarget(
        this, [this](size_t old_index, size_t new_index) { reorder_item(old_index, new_index); }));

    RegisterDragDrop(get_wnd(), drop_target.get());
}

bool FieldsList::do_drag_drop(WPARAM wp)
{
    DWORD drop_effect{DROPEFFECT_NONE};
    const auto data_object = utils::create_simple_list_view_data_object(get_wnd());
    LOG_IF_FAILED(uih::ole::do_drag_drop(get_wnd(), wp, data_object.get(), DROPEFFECT_MOVE, NULL, &drop_effect));
    return true;
}

void FieldsList::move_selection(int delta)
{
    const auto selection_index = fbh::as_optional(get_selected_item_single());

    if (!selection_index)
        return;

    const auto new_index = std::clamp(
        gsl::narrow<ptrdiff_t>(*selection_index) + delta, ptrdiff_t{}, gsl::narrow<ptrdiff_t>(get_item_count() - 1));

    reorder_item(*selection_index, gsl::narrow<size_t>(new_index));
    set_item_selected_single(new_index, false);
    ensure_visible(new_index);
}

void FieldsList::execute_default_action(size_t index, size_t column, bool b_keyboard, bool b_ctrl)
{
    activate_inline_editing(index, column);
}

void FieldsList::get_insert_items(size_t base, size_t count, pfc::list_t<InsertItem>& items)
{
    items.set_count(count);
    for (size_t i = 0; i < count; i++) {
        items[i].m_subitems.resize(2);
        items[i].m_subitems[0] = m_fields[base + i].m_name_friendly;
        items[i].m_subitems[1] = m_fields[base + i].m_name;
    }
}

FieldsList::FieldsList(pfc::list_t<Field>& p_fields)
    : helpers::CoreDarkListView(true)
    , m_edit_index(pfc_infinite)
    , m_edit_column(pfc_infinite)
    , m_fields(p_fields)
{
}

void FieldsList::reorder_item(size_t old_index, size_t new_index)
{
    auto permutation = utils::create_shift_item_permutation(old_index, new_index, get_item_count());
    m_fields.reorder(permutation.data());

    const auto first_affected = std::min(old_index, new_index);
    const auto last_affected = std::max(old_index, new_index);
    const auto affected_count = last_affected - first_affected + 1;

    pfc::list_t<InsertItem> insert_items;
    get_insert_items(first_affected, affected_count, insert_items);

    replace_items(first_affected, insert_items.size(), insert_items.get_ptr());
}

} // namespace cui::panels::item_properties
