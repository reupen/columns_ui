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
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR *ppvObject);
	virtual ULONG STDMETHODCALLTYPE   AddRef();
	virtual ULONG STDMETHODCALLTYPE   Release();

	virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);


	virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void);

	virtual HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	drop_handler_interface();

private:
	static bool check_window_allowed(HWND wnd);

	long drop_ref_count;
	POINTL last_over;
	mmh::comptr_t<IDropTargetHelper> m_DropTargetHelper;
	mmh::comptr_t<IDataObject> m_DataObject;
};
