#ifndef _UI_WIN32_HELPERS_H_
#define _UI_WIN32_HELPERS_H_

namespace win32 {
	extern const RECT rect_null;
}

bool g_test_os_version(DWORD major, DWORD minor);
bool is_vista_or_newer();
void g_set_listview_window_explorer_theme(HWND wnd);
void g_set_treeview_window_explorer_theme(HWND wnd, bool b_reduce_indent = false);
void g_remove_treeview_window_explorer_theme(HWND wnd);
bool g_keyboard_cues_enabled();
//void g_hide_keyboard_cues(HWND wnd, bool b_focus, bool b_accel);

struct win32_keyboard_lparam
{
	/** Specifies the repeat count. The value is the number of times the keystroke is repeated as a result of the user's holding down the key. */
LPARAM repeat_count : 16;
	/** Specifies the scan code. The value depends on the OEM. */
LPARAM scan_code : 8;
	/** Specifies whether the key is an extended key, such as a function key or a key on the numeric keypad. The value is 1 if the key is an extended key; otherwise, it is 0. */
LPARAM extended_key : 1;
	/** Reserved. */
LPARAM reserved : 4;
	/** Specifies the context code. The value is 1 if the ALT key is down; otherwise, it is 0.*/
LPARAM context_code : 1;
	/** Specifies the previous key state. The value is 1 if the key is down before the message is sent; it is 0 if the key is up.*/
LPARAM previous_key_state : 1;
	/** Specifies the transition state. The value is 0 if the key is being pressed and 1 if it is being released. */
LPARAM transition_code : 1;
};

#define get_keyboard_lparam(lp) (*(win32_keyboard_lparam*)&lp)

template<typename t_handle, class t_release>
class handle_container_t
{
	typedef handle_container_t<t_handle, t_release> t_self;
public:
	void release()
	{
		if (is_valid())
		{
			t_release::release(m_handle);
			t_release::set_invalid(m_handle);
		}
	}
	t_handle set (t_handle value)
	{
		release();
		return (m_handle = value);
	}
	t_handle get () const
	{
		return m_handle;
	}
	operator t_handle () const {return get();}
	t_handle operator = (t_handle value)
	{
		return set(value);
	}
	t_self & operator = (t_self & value)
	{
		set(value.detach());
		return *this;
	}
	t_handle detach()
	{
		t_handle ret = m_handle;
		t_release::set_invalid(m_handle);
		return ret;
	}
	bool is_valid() const
	{
		return t_release::is_valid(m_handle);
	}

	handle_container_t() {t_release::set_invalid(m_handle);};
	handle_container_t(t_handle value) : m_handle(value) {};
	handle_container_t(t_self & value) {set(value.detach());}

	~handle_container_t() {release();}
private:
	t_handle m_handle;
};

template <typename t_gdi_type>
class gdi_object_t
{
public:
	class gdi_release_t
	{
	public:
		template<typename t_gdi_type>
		static void release(t_gdi_type handle)
		{
			DeleteObject((t_gdi_type)handle);
		};
		template<typename t_gdi_type>
		static bool is_valid(t_gdi_type handle)
		{
			return handle!=NULL;
		};
		template<typename t_gdi_type>
		static void set_invalid(t_gdi_type & handle)
		{
			handle = NULL;
		};
	};
	typedef handle_container_t<t_gdi_type, gdi_release_t> ptr_t;
};

namespace win32_helpers
{
	class __handle_release_t
	{
	public:
		static void release(HANDLE handle)
		{
			CloseHandle(handle);
		};
		static bool is_valid(HANDLE handle)
		{
			return handle!=INVALID_HANDLE_VALUE;
		};
		static void set_invalid(HANDLE & handle)
		{
			handle = INVALID_HANDLE_VALUE;
		};
	};
	typedef handle_container_t<HANDLE, __handle_release_t> handle_ptr_t;


	void format_date(t_filetimestamp time, std::basic_string<TCHAR> & str, bool b_convert_to_local = false);
	HRESULT get_comctl32_version(DLLVERSIONINFO2 & p_dvi, pfc::string_base * p_path_out = NULL);
}

int ListView_InsertColumnText(HWND wnd_lv, UINT index, const TCHAR * text, int cx);
LRESULT ListView_InsertItemText(HWND wnd_lv, UINT item, UINT subitem, const TCHAR * text, bool b_set = false, LPARAM lp =0, int image_index = I_IMAGENONE);
LRESULT ListView_InsertItemText(HWND wnd_lv, UINT item, UINT subitem, const char * text, bool b_set = false, LPARAM lp=0, int image_index = I_IMAGENONE);

HTREEITEM uTreeView_InsertItemSimple(HWND wnd_tree, const char * sz_text, LPARAM data, DWORD state = TVIS_EXPANDED, HTREEITEM ti_parent = TVI_ROOT, HTREEITEM ti_after = TVI_LAST, bool b_image=false, UINT image=NULL, UINT integral_height=1);
HTREEITEM uTreeView_InsertItemSimple(HWND wnd_tree, const WCHAR * sz_text, LPARAM data, DWORD state = TVIS_EXPANDED, HTREEITEM ti_parent = TVI_ROOT, HTREEITEM ti_after = TVI_LAST, bool b_image=false, UINT image=NULL, UINT integral_height=1);

t_size uTreeView_GetChildIndex(HWND wnd_tv, HTREEITEM ti);

namespace win32_helpers
{
	BOOL ShellNotifyIconSimple(DWORD dwMessage,HWND wnd,UINT id,UINT callbackmsg,HICON icon,
		const char * tip, const char * balloon_title=NULL, const char * balloon_msg=NULL);

	BOOL uShellNotifyIcon(DWORD action,HWND wnd,UINT id,UINT version,UINT callbackmsg,HICON icon,const char * tip);
	BOOL uShellNotifyIconEx(DWORD action,HWND wnd,UINT id,UINT callbackmsg,HICON icon,const char * tip,const char * balloon_title,const char * balloon_msg);
	int ComboBox_AddStringData(HWND wnd, const TCHAR * str, LPARAM data);
	void RegisterShellHookWindowHelper(HWND wnd);
	void DeregisterShellHookWindowHelper(HWND wnd);

	int combobox_find_item_by_data(HWND wnd, t_size id);

	int rebar_id_to_idx(HWND wnd, unsigned id);
	void rebar_show_all_bands(HWND wnd);
}

class disable_redrawing_t
{
	bool m_active, m_disable_invalidate, m_disable_update;
	HWND m_wnd;
public:
	void reenable_redrawing()
	{
		if (m_active)
		{
			SendMessage(m_wnd, WM_SETREDRAW, TRUE, 0);
			if (!m_disable_invalidate ||!m_disable_update)
				RedrawWindow(m_wnd, 0, 0, (m_disable_invalidate?NULL:RDW_INVALIDATE)|(m_disable_update?NULL:RDW_UPDATENOW));
			m_active=false;
		}
	}
	disable_redrawing_t(HWND wnd, bool b_disable_invalidate = false, bool b_disable_update = false)
		: m_wnd(wnd), m_active(false), m_disable_invalidate(b_disable_invalidate), m_disable_update(b_disable_update)
	{
		if (IsWindowVisible(m_wnd))
		{
			SendMessage(m_wnd, WM_SETREDRAW, FALSE, 0);
			m_active = true;
		}
	}
	~disable_redrawing_t() {reenable_redrawing();}

};

namespace ui_helpers
{
void innerWMPaintModernBackground (HWND wnd, HWND wnd_button);
}

namespace mmh { namespace ole {

CLIPFORMAT ClipboardFormatDropDescription();
HRESULT SetBlob(IDataObject *pdtobj, CLIPFORMAT cf, const void *pvBlob, UINT cbBlob);
HRESULT SetDropDescription(IDataObject *pdtobj, DROPIMAGETYPE dit, const char * msg, const char * insert);

	class IDropSource_Generic : public IDropSource
	{
		long refcount;
		DWORD m_initial_key_state;
		mmh::comptr_t<IDragSourceHelper> m_DragSourceHelper;
	public:
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void ** ppvObject);
		ULONG STDMETHODCALLTYPE AddRef();
		ULONG STDMETHODCALLTYPE Release();
		HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState);
		HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);
		IDropSource_Generic(HWND wnd, IDataObject * pDataObj, DWORD initial_key_state, bool b_allowdropdescriptiontext = false); //careful, some fb2k versions have broken IDataObject
	};

}
}
#endif //_UI_WIN32_HELPERS_H_