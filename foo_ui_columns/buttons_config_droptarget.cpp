#include "stdafx.h"
#include "buttons.h"

namespace cui::toolbars::buttons {

HRESULT STDMETHODCALLTYPE ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::QueryInterface(
    REFIID riid, LPVOID FAR* ppvObject)
{
    if (ppvObject == nullptr)
        return E_INVALIDARG;
    *ppvObject = nullptr;
    if (riid == IID_IUnknown) {
        AddRef();
        *ppvObject = (IUnknown*)this;
        return S_OK;
    }
    if (riid == IID_IDropTarget) {
        AddRef();
        *ppvObject = (IDropTarget*)this;
        return S_OK;
    }
    return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::AddRef()
{
    return InterlockedIncrement(&drop_ref_count);
}
ULONG STDMETHODCALLTYPE ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::Release()
{
    LONG rv = InterlockedDecrement(&drop_ref_count);
    if (!rv)
        delete this;
    return rv;
}
bool ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::check_do(IDataObject* pDO)
{
    bool rv = false;
    FORMATETC fe;
    memset(&fe, 0, sizeof(fe));
    fe.cfFormat = g_clipformat();
    fe.dwAspect = DVASPECT_CONTENT;
    fe.lindex = -1;
    fe.tymed = TYMED_HGLOBAL;
    // console::formatter() << "check_do: " << (int)fe.cfFormat;
    STGMEDIUM sm;
    memset(&sm, 0, sizeof(sm));
    if (SUCCEEDED(pDO->GetData(&fe, &sm))) {
        auto* pDDD = (DDData*)GlobalLock(sm.hGlobal);
        if (pDDD && pDDD->version == 0 && pDDD->wnd == m_button_list_view->get_wnd())
            rv = true;
        ReleaseStgMedium(&sm);
    }
    return rv;
}
HRESULT STDMETHODCALLTYPE ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::DragEnter(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    // console::formatter() << "DragEnter";
    m_DataObject = pDataObj;
    last_rmb = ((grfKeyState & MK_RBUTTON) != 0);
    POINT pt = {ptl.x, ptl.y};
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragEnter(m_button_list_view->get_wnd(), pDataObj, &pt, *pdwEffect);
    *pdwEffect = DROPEFFECT_NONE;
    if (check_do(pDataObj /*, pdwEffect*/)) {
        uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_MOVE, "Move", "");
        *pdwEffect = DROPEFFECT_MOVE;
    }
    return S_OK;
}
HRESULT STDMETHODCALLTYPE ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::DragOver(
    DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    // console::formatter() << "DragOver";
    POINT pt = {ptl.x, ptl.y};
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragOver(&pt, *pdwEffect);

    last_rmb = ((grfKeyState & MK_RBUTTON) != 0);
    POINT pti;
    pti.y = ptl.y;
    pti.x = ptl.x;

    *pdwEffect = DROPEFFECT_NONE;
    if (check_do(m_DataObject.get() /*, pdwEffect*/)) {
        *pdwEffect = DROPEFFECT_MOVE;
        HRESULT hr = uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_MOVE, "Move", "");
        // console::formatter() << pfc::format_hex(hr);
    }
    if (m_button_list_view->get_wnd()) {
        uih::ListView::HitTestResult hi;

        {
            POINT ptt = pti;
            ScreenToClient(m_button_list_view->get_wnd(), &ptt);

            RECT rc_items = m_button_list_view->get_items_rect();

            rc_items.top += m_button_list_view->get_item_height();
            rc_items.bottom -= m_button_list_view->get_item_height();

            if (ptt.y < rc_items.top && ptt.y < rc_items.bottom) {
                m_button_list_view->create_timer_scroll_up();
            } else
                m_button_list_view->destroy_timer_scroll_up();

            if (ptt.y >= rc_items.top && ptt.y >= rc_items.bottom) {
                m_button_list_view->create_timer_scroll_down();
            } else
                m_button_list_view->destroy_timer_scroll_down();
            {
                m_button_list_view->hit_test_ex(ptt, hi);
                if (hi.category == HitTestCategory::OnUnobscuredItem || hi.category == HitTestCategory::OnGroupHeader
                    || hi.category == HitTestCategory::OnItemObscuredBelow || hi.category == HitTestCategory::NotOnItem)
                    m_button_list_view->set_insert_mark(hi.insertion_index);
                else
                    m_button_list_view->remove_insert_mark();
            }
        }
    }

    return S_OK;
}
HRESULT STDMETHODCALLTYPE ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::DragLeave()
{
    // console::formatter() << "DragLeave";
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragLeave();
    uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_INVALID, "", "");
    m_button_list_view->remove_insert_mark();
    m_button_list_view->destroy_timer_scroll_up();
    m_button_list_view->destroy_timer_scroll_down();
    m_DataObject.reset();
    return S_OK;
}
HRESULT STDMETHODCALLTYPE ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::Drop(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    POINT pt = {ptl.x, ptl.y};
    if (m_DropTargetHelper)
        m_DropTargetHelper->Drop(pDataObj, &pt, *pdwEffect);

    *pdwEffect = DROPEFFECT_NONE;
    if (check_do(pDataObj /*, pdwEffect*/)) {
        *pdwEffect = DROPEFFECT_MOVE;
        const t_size old_index = m_button_list_view->get_selected_item_single(); // meh

        uih::ListView::HitTestResult hi;

        POINT ptt = pt;
        ScreenToClient(m_button_list_view->get_wnd(), &ptt);

        m_button_list_view->hit_test_ex(ptt, hi);

        t_size new_index = pfc_infinite;

        if (hi.category == HitTestCategory::OnUnobscuredItem || hi.category == HitTestCategory::OnGroupHeader
            || hi.category == HitTestCategory::OnItemObscuredBelow || hi.category == HitTestCategory::NotOnItem)
            new_index = hi.insertion_index;

        if (new_index != pfc_infinite && new_index > old_index)
            --new_index;

        if (new_index != pfc_infinite && old_index != pfc_infinite && new_index != old_index
            && old_index < m_button_list_view->m_param.m_buttons.size()) {
            const size_t button_count = m_button_list_view->m_param.m_buttons.size();

            const int step = new_index > old_index ? 1 : -1;
            mmh::Permutation perm(button_count);

            for (t_size i = old_index; i != new_index; i += step)
                std::swap(perm[i], perm[i + step]);

            mmh::destructive_reorder(m_button_list_view->m_param.m_buttons, perm);

            // blaarrgg, designed in the dark ages
            m_button_list_view->m_param.m_selection = &m_button_list_view->m_param.m_buttons[new_index];

            const size_t first_index = (std::min)(old_index, new_index);
            const size_t last_index = (std::max)(old_index, new_index);
            m_button_list_view->m_param.refresh_buttons_list_items(first_index, last_index - first_index + 1);
            m_button_list_view->set_item_selected_single(new_index);
        }
    }
    m_DataObject.reset();
    m_button_list_view->remove_insert_mark();
    m_button_list_view->destroy_timer_scroll_up();
    m_button_list_view->destroy_timer_scroll_down();
    return S_OK;
}
ButtonsToolbar::ConfigParam::ButtonsList::ButtonsListDropTarget::ButtonsListDropTarget(
    ButtonsToolbar::ConfigParam::ButtonsList* p_blv)
    : drop_ref_count(0)
    , last_rmb(false)
    , m_button_list_view(p_blv)
{
    m_DropTargetHelper = wil::CoCreateInstanceNoThrow<IDropTargetHelper>(CLSID_DragDropHelper);
}

} // namespace cui::toolbars::buttons
