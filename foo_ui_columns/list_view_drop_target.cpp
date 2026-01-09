#include "pch.h"

#include "list_view_drop_target.h"

namespace cui::utils {

namespace {

CLIPFORMAT clip_format()
{
    static auto clip_format = static_cast<CLIPFORMAT>(RegisterClipboardFormat(L"CUIListViewHWND"));
    return clip_format;
}

bool should_drop(const uih::ListView::HitTestResult& hit_result)
{
    return hit_result.category == uih::ListView::HitTestCategory::OnUnobscuredItem
        || hit_result.category == uih::ListView::HitTestCategory::OnGroupHeader
        || hit_result.category == uih::ListView::HitTestCategory::OnItemObscuredBelow
        || hit_result.category == uih::ListView::HitTestCategory::NotOnItem;
}

} // namespace

wil::com_ptr<IDataObject> create_simple_list_view_data_object(HWND wnd)
{
    wil::com_ptr<IDataObject> data_object = new CDataObject;
    uih::ole::set_blob(data_object.get(), clip_format(), &wnd, sizeof(wnd));
    return data_object;
}

HRESULT STDMETHODCALLTYPE SimpleListViewDropTarget::QueryInterface(REFIID riid, LPVOID FAR* ppvObject)
{
    if (ppvObject == nullptr)
        return E_INVALIDARG;

    *ppvObject = nullptr;

    if (riid == __uuidof(IUnknown))
        *ppvObject = static_cast<IUnknown*>(this);
    else if (riid == __uuidof(IDropTarget))
        *ppvObject = static_cast<IDropTarget*>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE SimpleListViewDropTarget::AddRef()
{
    return ++m_ref_count;
}

ULONG STDMETHODCALLTYPE SimpleListViewDropTarget::Release()
{
    const auto new_ref_count = --m_ref_count;

    if (new_ref_count == 0)
        delete this;

    return new_ref_count;
}

bool SimpleListViewDropTarget::check_data_object(IDataObject* data_object) const
{
    FORMATETC fe{};
    fe.cfFormat = clip_format();
    fe.dwAspect = DVASPECT_CONTENT;
    fe.lindex = -1;
    fe.tymed = TYMED_HGLOBAL;

    wil::unique_stg_medium stg_medium;

    if (SUCCEEDED(data_object->GetData(&fe, &stg_medium))) {
        const auto drag_wnd = static_cast<const HWND*>(GlobalLock(stg_medium.hGlobal));

        if (drag_wnd && *drag_wnd == m_list_view->get_wnd())
            return true;
    }
    return false;
}

HRESULT STDMETHODCALLTYPE SimpleListViewDropTarget::DragEnter(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    m_data_object = pDataObj;
    m_last_rmb = (grfKeyState & MK_RBUTTON) != 0;
    POINT pt = {ptl.x, ptl.y};

    if (m_drop_target_helper)
        LOG_IF_FAILED(m_drop_target_helper->DragEnter(m_list_view->get_wnd(), pDataObj, &pt, *pdwEffect));

    *pdwEffect = DROPEFFECT_NONE;

    if (check_data_object(pDataObj)) {
        uih::ole::set_drop_description(m_data_object.get(), DROPIMAGE_MOVE, "Move", "");
        *pdwEffect = DROPEFFECT_MOVE;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE SimpleListViewDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    POINT screen_pt{ptl.x, ptl.y};
    if (m_drop_target_helper)
        m_drop_target_helper->DragOver(&screen_pt, *pdwEffect);

    m_last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

    *pdwEffect = DROPEFFECT_NONE;

    if (check_data_object(m_data_object.get())) {
        *pdwEffect = DROPEFFECT_MOVE;
        uih::ole::set_drop_description(m_data_object.get(), DROPIMAGE_MOVE, "Move", "");
    }

    if (m_list_view->get_wnd()) {
        uih::ListView::HitTestResult hit_result{};

        POINT client_pt{screen_pt};
        ScreenToClient(m_list_view->get_wnd(), &client_pt);

        RECT rc_items = m_list_view->get_items_rect();

        rc_items.top += m_list_view->get_item_height();
        rc_items.bottom -= m_list_view->get_item_height();

        if (client_pt.y < rc_items.top && client_pt.y < rc_items.bottom) {
            m_list_view->create_timer_scroll_up();
        } else
            m_list_view->destroy_timer_scroll_up();

        if (client_pt.y >= rc_items.top && client_pt.y >= rc_items.bottom) {
            m_list_view->create_timer_scroll_down();
        } else
            m_list_view->destroy_timer_scroll_down();

        m_list_view->hit_test_ex(client_pt, hit_result);

        if (should_drop(hit_result))
            m_list_view->set_insert_mark(hit_result.insertion_index);
        else
            m_list_view->remove_insert_mark();
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE SimpleListViewDropTarget::DragLeave()
{
    if (m_drop_target_helper)
        m_drop_target_helper->DragLeave();

    uih::ole::set_drop_description(m_data_object.get(), DROPIMAGE_INVALID, "", "");
    m_list_view->remove_insert_mark();
    m_list_view->destroy_timer_scroll_up();
    m_list_view->destroy_timer_scroll_down();
    m_data_object.reset();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE SimpleListViewDropTarget::Drop(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    POINT screen_pt{ptl.x, ptl.y};

    if (m_drop_target_helper)
        m_drop_target_helper->Drop(pDataObj, &screen_pt, *pdwEffect);

    *pdwEffect = DROPEFFECT_NONE;
    if (check_data_object(pDataObj /*, pdwEffect*/)) {
        *pdwEffect = DROPEFFECT_MOVE;
        const auto old_index = fbh::as_optional(m_list_view->get_selected_item_single());

        uih::ListView::HitTestResult hit_result{};

        POINT client_pt{screen_pt};
        ScreenToClient(m_list_view->get_wnd(), &client_pt);

        m_list_view->hit_test_ex(client_pt, hit_result);

        std::optional<size_t> new_index;

        if (should_drop(hit_result))
            new_index = hit_result.insertion_index;

        if (new_index && new_index > old_index)
            --*new_index;

        const auto item_count = m_list_view->get_item_count();

        if (new_index && old_index && new_index != old_index && old_index < item_count) {
            const int step = new_index > old_index ? 1 : -1;
            mmh::Permutation perm(item_count);

            for (size_t index = *old_index; index != new_index; index += step)
                std::swap(perm[index], perm[index + step]);

            m_on_order_changed(perm, *old_index, *new_index);
            m_list_view->set_item_selected_single(*new_index);
            m_list_view->ensure_visible(*new_index);
        }
    }

    m_data_object.reset();
    m_list_view->remove_insert_mark();
    m_list_view->destroy_timer_scroll_up();
    m_list_view->destroy_timer_scroll_down();
    return S_OK;
}

} // namespace cui::utils
