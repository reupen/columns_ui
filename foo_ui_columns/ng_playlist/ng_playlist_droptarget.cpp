#include "../stdafx.h"
#include "ng_playlist.h"

namespace pvt {
HRESULT STDMETHODCALLTYPE PlaylistViewDropTarget::QueryInterface(REFIID riid, LPVOID FAR* ppvObject)
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

ULONG STDMETHODCALLTYPE PlaylistViewDropTarget::AddRef()
{
    return InterlockedIncrement(&drop_ref_count);
}

ULONG STDMETHODCALLTYPE PlaylistViewDropTarget::Release()
{
    LONG rv = InterlockedDecrement(&drop_ref_count);
    if (!rv) {
        delete this;
    }
    return rv;
}

HRESULT STDMETHODCALLTYPE PlaylistViewDropTarget::DragEnter(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    m_DataObject = pDataObj;
    last_rmb = ((grfKeyState & MK_RBUTTON) != 0);

    POINT pt = {ptl.x, ptl.y};
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragEnter(p_playlist->get_wnd(), pDataObj, &pt, *pdwEffect);

    bool uid_handled = ui_drop_item_callback::g_is_accepted_type(pDataObj, pdwEffect);

    m_is_accepted_type
        = uid_handled || static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check(pDataObj);
    if (!uid_handled) {
        if (m_is_accepted_type) {
            *pdwEffect
                = p_playlist->m_dragging && p_playlist->m_DataObject == pDataObj && (0 == (grfKeyState & MK_CONTROL))
                ? DROPEFFECT_MOVE
                : DROPEFFECT_COPY;
            UpdateDropDescription(m_DataObject.get(), *pdwEffect);
        } else {
            *pdwEffect = DROPEFFECT_NONE;
            uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_INVALID, "", "");
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PlaylistViewDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    POINT pt = {ptl.x, ptl.y};
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragOver(&pt, *pdwEffect);

    last_rmb = ((grfKeyState & MK_RBUTTON) != 0);
    POINT pti;
    pti.y = ptl.y;
    pti.x = ptl.x;

    if (!m_is_accepted_type) {
        *pdwEffect = DROPEFFECT_NONE;
        uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_INVALID, "", "");
        return S_OK;
    }

    if (ui_drop_item_callback::g_is_accepted_type(m_DataObject.get(), pdwEffect))
        return S_OK;

    *pdwEffect = p_playlist->m_dragging && p_playlist->m_DataObject == m_DataObject && (0 == (grfKeyState & MK_CONTROL))
        ? DROPEFFECT_MOVE
        : DROPEFFECT_COPY;
    UpdateDropDescription(m_DataObject.get(), *pdwEffect);

    if (p_playlist->get_wnd()) {
        uih::ListView::HitTestResult hi;

        {
            POINT ptt = pti;
            ScreenToClient(p_playlist->get_wnd(), &ptt);

            RECT rc_items = p_playlist->get_items_rect();

            rc_items.top += p_playlist->get_item_height();
            rc_items.bottom -= p_playlist->get_item_height();

            if (ptt.y < rc_items.top && ptt.y < rc_items.bottom) {
                p_playlist->create_timer_scroll_up();
            } else
                p_playlist->destroy_timer_scroll_up();

            if (ptt.y >= rc_items.top && ptt.y >= rc_items.bottom) {
                p_playlist->create_timer_scroll_down();
            } else
                p_playlist->destroy_timer_scroll_down();

            if (*pdwEffect == DROPEFFECT_COPY && cfg_drop_at_end)
                p_playlist->set_insert_mark(p_playlist->get_item_count());
            else {
                p_playlist->hit_test_ex(ptt, hi);
                if (hi.category == uih::ListView::HitTestCategory::OnUnobscuredItem
                    || hi.category == uih::ListView::HitTestCategory::OnGroupHeader
                    || hi.category == uih::ListView::HitTestCategory::OnItemObscuredBelow
                    || hi.category == uih::ListView::HitTestCategory::NotOnItem)
                    p_playlist->set_insert_mark(hi.insertion_index);
                else
                    p_playlist->remove_insert_mark();
            }
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PlaylistViewDropTarget::DragLeave()
{
    if (m_DropTargetHelper)
        m_DropTargetHelper->DragLeave();
    uih::ole::set_drop_description(m_DataObject.get(), DROPIMAGE_INVALID, "", "");
    p_playlist->remove_insert_mark();
    p_playlist->destroy_timer_scroll_up();
    p_playlist->destroy_timer_scroll_down();
    m_DataObject.reset();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PlaylistViewDropTarget::Drop(
    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
    POINT pt = {ptl.x, ptl.y};
    if (m_DropTargetHelper)
        m_DropTargetHelper->Drop(pDataObj, &pt, *pdwEffect);

    *pdwEffect = p_playlist->m_dragging && p_playlist->m_DataObject == pDataObj && (0 == (grfKeyState & MK_CONTROL))
        ? DROPEFFECT_MOVE
        : DROPEFFECT_COPY;
    m_DataObject.reset();
    p_playlist->destroy_timer_scroll_up();
    p_playlist->destroy_timer_scroll_down();

    if (!m_is_accepted_type) {
        *pdwEffect = DROPEFFECT_NONE;
        p_playlist->remove_insert_mark();
        return S_OK;
    }

    POINT pti;
    pti.y = ptl.y;
    pti.x = ptl.x;
    if (p_playlist->get_wnd()) {
        bool process = !ui_drop_item_callback::g_on_drop(pDataObj);

        /*if (process && last_rmb)
        {
        process = false;
        enum {ID_DROP = 1, ID_CANCEL };

        HMENU menu = CreatePopupMenu();

        uAppendMenu(menu,(MF_STRING),ID_DROP,"&Add files here");
        uAppendMenu(menu,MF_SEPARATOR,0,0);
        uAppendMenu(menu,MF_STRING,ID_CANCEL,"&Cancel");

        int cmd =
        TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,p_playlist->wnd_playlist,0);
        DestroyMenu(menu);

        if (cmd)
        {
        switch(cmd)
        {
        case ID_DROP:
        process = true;
        break;
        }
        }
        }*/

        if (process) {
            metadb_handle_list data;

            // console::info(static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check_if_native(pDataObj)?"native":"not
            // very native?");

            static_api_ptr_t<ole_interaction> ole_api;
            bool b_isNative
                = static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check_if_native(pDataObj);

            dropped_files_data_impl dropped_data;
            // static_api_ptr_t<playlist_incoming_item_filter_v3>()->process_dropped_files(pDataObj, data,
            // true,p_playlist->get_wnd());
            ole_api->parse_dataobject(pDataObj, dropped_data);

            static_api_ptr_t<playlist_manager> playlist_api;

            t_size idx = playlist_api->activeplaylist_get_item_count();
            uih::ListView::HitTestResult hi;

            {
                POINT ptt = pti;
                ScreenToClient(p_playlist->get_wnd(), &ptt);
                // p_playlist->hit_test(ptt, idx);
                p_playlist->hit_test_ex(ptt, hi);
            }

            if (hi.category == uih::ListView::HitTestCategory::OnUnobscuredItem
                || hi.category == uih::ListView::HitTestCategory::OnGroupHeader
                || hi.category == uih::ListView::HitTestCategory::OnItemObscuredBelow
                || hi.category == uih::ListView::HitTestCategory::OnItemObscuredAbove
                || hi.category == uih::ListView::HitTestCategory::NotOnItem) {
                idx = hi.insertion_index;
            }

            if (*pdwEffect == DROPEFFECT_COPY && cfg_drop_at_end)
                idx = playlist_api->activeplaylist_get_item_count();

            if (p_playlist->m_dragging) {
                dropped_data.to_handles(data, false, p_playlist->get_wnd());
                pfc::list_t<bool> selection;
                std::vector<size_t> permutation_move;

                t_size active_playlist = playlist_api->get_active_playlist();

                if (p_playlist->m_dragging) {
                    selection.set_size(playlist_api->playlist_get_item_count(p_playlist->m_dragging_initial_playlist));
                    pfc::bit_array_var_table bitsel(selection.get_ptr(), selection.get_count());
                    playlist_api->playlist_get_selection_mask(p_playlist->m_dragging_initial_playlist, bitsel);
                    permutation_move.resize(selection.get_count());
                    ranges::iota(permutation_move, size_t{});

                    if (p_playlist->m_dragging_initial_playlist == active_playlist) {
                        if (*pdwEffect == DROPEFFECT_MOVE) {
                            t_size count = selection.get_count();
                            size_t counter{};
                            for (t_size i = 0; i < count; i++) {
                                if (selection[i]) {
                                    permutation_move.insert(permutation_move.begin() + idx + counter, i);
                                    ++counter;
                                }
                            }

                            selection.insert_items_repeat(false, counter, idx);

                            for (size_t i{selection.get_size()}; i; --i) {
                                const auto index = i - 1;
                                if (selection[index]) {
                                    permutation_move.erase(permutation_move.begin() + index);
                                }
                            }
                        } else {
                            selection.insert_items_repeat(false, data.get_count(), idx);
                        }
                    } else if (*pdwEffect == DROPEFFECT_MOVE)
                        playlist_api->playlist_undo_backup(p_playlist->m_dragging_initial_playlist);
                }

                playlist_api->activeplaylist_undo_backup();
                bool b_redraw = p_playlist->disable_redrawing();

                if (p_playlist->m_dragging && p_playlist->m_dragging_initial_playlist == active_playlist
                    && *pdwEffect == DROPEFFECT_MOVE) {
                    playlist_api->activeplaylist_reorder_items(permutation_move.data(), permutation_move.size());
                } else {
                    playlist_api->activeplaylist_clear_selection();
                    t_size index_insert = playlist_api->activeplaylist_insert_items(idx, data, pfc::bit_array_true());
                    playlist_api->activeplaylist_set_focus_item(index_insert);
                    if (p_playlist->m_dragging && *pdwEffect == DROPEFFECT_MOVE) {
                        playlist_api->playlist_remove_items(p_playlist->m_dragging_initial_playlist,
                            pfc::bit_array_table(selection.get_ptr(), selection.get_count()));
                    }
                }
                p_playlist->remove_insert_mark();

                SetFocus(p_playlist->get_wnd());
                if (b_redraw)
                    p_playlist->enable_redrawing();
            } else {
                class DelayedDropTargetProcesser : public process_locations_notify {
                public:
                    playlist_position_reference_tracker m_insertIndexTracker;
                    service_ptr_t<PlaylistView> p_playlist;

                    void on_completion(const pfc::list_base_const_t<metadb_handle_ptr>& p_items) override
                    {
                        static_api_ptr_t<playlist_manager> playlist_api;
                        if (m_insertIndexTracker.m_playlist != pfc_infinite && p_items.get_count()) {
                            if (m_insertIndexTracker.m_item == pfc_infinite)
                                m_insertIndexTracker.m_item
                                    = playlist_api->playlist_get_item_count(m_insertIndexTracker.m_playlist);
                            playlist_api->playlist_undo_backup(m_insertIndexTracker.m_playlist);
                            bool b_redraw = p_playlist->disable_redrawing();
                            playlist_api->playlist_clear_selection(m_insertIndexTracker.m_playlist);
                            t_size index_insert = playlist_api->playlist_insert_items(m_insertIndexTracker.m_playlist,
                                m_insertIndexTracker.m_item, p_items, pfc::bit_array_true());
                            playlist_api->playlist_set_focus_item(
                                m_insertIndexTracker.m_playlist, m_insertIndexTracker.m_item);
                            if (b_redraw)
                                p_playlist->enable_redrawing();
                        }
                    }
                    void on_aborted() override {}
                };

                service_ptr_t<DelayedDropTargetProcesser> ptr
                    = new service_impl_t<DelayedDropTargetProcesser>;
                ptr->p_playlist = p_playlist;
                ptr->m_insertIndexTracker.m_playlist = playlist_api->get_active_playlist();
                ptr->m_insertIndexTracker.m_item = idx;
                dropped_data.to_handles_async(true, p_playlist->get_wnd(), ptr);
                p_playlist->remove_insert_mark();
                SetFocus(p_playlist->get_wnd());
            }
        }
    }

    return S_OK;
}
PlaylistViewDropTarget::PlaylistViewDropTarget(PlaylistView* playlist)
    : drop_ref_count(0), last_rmb(false), m_is_accepted_type(false), p_playlist(playlist)
{
    m_DropTargetHelper = wil::CoCreateInstanceNoThrow<IDropTargetHelper>(CLSID_DragDropHelper);
}
HRESULT PlaylistViewDropTarget::UpdateDropDescription(IDataObject* pDataObj, DWORD pdwEffect)
{
    static_api_ptr_t<playlist_manager> playlist_api;
    DROPIMAGETYPE dit = DROPIMAGE_INVALID;
    const char* message = nullptr;
    pfc::string8 insertText;

    if (pdwEffect == DROPEFFECT_MOVE) {
        dit = DROPIMAGE_MOVE;
        if (p_playlist->m_dragging_initial_playlist != playlist_api->get_active_playlist()) {
            message = "Move to %1";
            playlist_api->activeplaylist_get_name(insertText);

        } else {
            message = "Move here";
        }
    } else // always set to DROPEFFECT_MOVE or DROPEFFECT_COPY
    {
        dit = DROPIMAGE_COPY;
        message = "Add to %1";
        playlist_api->activeplaylist_get_name(insertText);
    }
    return uih::ole::set_drop_description(pDataObj, dit, message, insertText);
}
} // namespace pvt
