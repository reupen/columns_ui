#pragma once

namespace cui::utils {

wil::com_ptr<IDataObject> create_simple_list_view_data_object(HWND wnd);

class SimpleListViewDropTarget : public IDropTarget {
public:
    using OnOrderChanged = std::function<void(size_t new_index, size_t old_index)>;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    bool check_data_object(IDataObject* data_object) const;
    HRESULT STDMETHODCALLTYPE DragEnter(
        IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragLeave() override;
    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect) override;
    explicit SimpleListViewDropTarget(uih::ListView* list_view, OnOrderChanged on_order_changed)
        : m_list_view(list_view)
        , m_on_order_changed(std::move(on_order_changed))
        , m_drop_target_helper(wil::CoCreateInstanceNoThrow<IDropTargetHelper>(CLSID_DragDropHelper))
    {
    }

private:
    uih::ListView* m_list_view{};
    OnOrderChanged m_on_order_changed;
    bool m_last_rmb{};
    std::atomic<uint32_t> m_ref_count;
    wil::com_ptr<IDataObject> m_data_object;
    wil::com_ptr<IDropTargetHelper> m_drop_target_helper;
};

} // namespace cui::utils
