#pragma once

/*!
 * \file mw_drop_target.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Class used for handling drag and drop operations on the main window (drop target only)
 */

class drop_handler_interface : public IDropTarget
{
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR *ppvObject) override;
    ULONG STDMETHODCALLTYPE   AddRef() override;
    ULONG STDMETHODCALLTYPE   Release() override;

    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;


    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;

    HRESULT STDMETHODCALLTYPE DragLeave() override;

    HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;
    drop_handler_interface();

private:
    static bool check_window_allowed(HWND wnd);

    long drop_ref_count;
    POINTL last_over;
    mmh::comptr_t<IDropTargetHelper> m_DropTargetHelper;
    mmh::comptr_t<IDataObject> m_DataObject;
};
