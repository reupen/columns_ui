#include "stdafx.h"
#include "buttons.h"

CLIPFORMAT toolbar_extension::config_param::t_button_list_view::g_clipformat()
{
    static CLIPFORMAT cf = (CLIPFORMAT)RegisterClipboardFormat(L"CUIuih::ListViewStandardClipFormat");
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
void toolbar_extension::config_param::t_button_list_view::notify_on_selection_change(const bit_array & p_affected, const bit_array & p_status, notification_source_t p_notification_source)
{
    t_size index = get_selected_item_single();
    m_param.on_selection_change(index);
}
bool toolbar_extension::config_param::t_button_list_view::do_drag_drop(WPARAM wp)
{
    UINT cf = g_clipformat();

    HGLOBAL glb = GlobalAlloc(GPTR, sizeof(DDData));
    if (glb)
    {
        DDData * pddd = (DDData*)glb;
        pddd->version = 0;
        pddd->wnd = get_wnd();

        FORMATETC fe;
        memset(&fe, 0, sizeof(fe));
        fe.cfFormat = cf;
        fe.dwAspect = DVASPECT_CONTENT;
        fe.lindex = -1;
        fe.tymed = TYMED_HGLOBAL;
        STGMEDIUM sm;
        memset(&sm, 0, sizeof(sm));
        sm.tymed = TYMED_HGLOBAL;
        sm.hGlobal = glb;

        mmh::ComPtr<IDataObject> pDO = new CDataObject;
        pDO->SetData(&fe, &sm, TRUE);

        DWORD blah = DROPEFFECT_NONE;

        uih::ole::do_drag_drop(get_wnd(), wp, pDO, DROPEFFECT_MOVE, NULL, &blah);
    }

    return true;
}
