#include "stdafx.h"
#include "buttons.h"

CLIPFORMAT toolbar_extension::config_param::t_button_list_view::g_clipformat()
{
    static auto cf = (CLIPFORMAT)RegisterClipboardFormat(L"CUIListViewStandardClipFormat");
    return cf;
}

void toolbar_extension::config_param::t_button_list_view::notify_on_initialisation()
{
    set_single_selection(true);

    pfc::list_t<uih::ListView::Column> columns;
    columns.add_item(uih::ListView::Column("Name", 300));
    columns.add_item(uih::ListView::Column("Type", 150));

    set_columns(columns);
}
void toolbar_extension::config_param::t_button_list_view::notify_on_create()
{
    pfc::com_ptr_t<IDropTarget_buttons_list> IDT_blv = new IDropTarget_buttons_list(this);
    RegisterDragDrop(get_wnd(), IDT_blv.get_ptr());
}
void toolbar_extension::config_param::t_button_list_view::notify_on_destroy()
{
    RevokeDragDrop(get_wnd());
}
void toolbar_extension::config_param::t_button_list_view::notify_on_selection_change(
    const pfc::bit_array& p_affected, const pfc::bit_array& p_status, notification_source_t p_notification_source)
{
    t_size index = get_selected_item_single();
    m_param.on_selection_change(index);
}

bool toolbar_extension::config_param::t_button_list_view::do_drag_drop(WPARAM wp)
{
    UINT cf = g_clipformat();
    mmh::ComPtr<IDataObject> pDO = new CDataObject;

    DDData data = {0, get_wnd()};
    uih::ole::set_blob(pDO, cf, &data, sizeof(data));

    DWORD drop_effect = DROPEFFECT_NONE;
    uih::ole::do_drag_drop(get_wnd(), wp, pDO, DROPEFFECT_MOVE, NULL, &drop_effect);

    return true;
}
