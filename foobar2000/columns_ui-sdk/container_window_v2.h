#pragma once

namespace ui_helpers 
{

	/** \brief Implements a window that serves either as an empty container for either other windows, or as a custom control */
	template <typename TBase>
	class container_window_v2_t : public TBase
	{
	private:
		container_window_v2_t(const container_window_v2_t<TBase> & p_source) {};

	public:
		enum 
		{
			flag_forward_system_settings_change = (1<<0),
			flag_forward_system_colours_change = (1<<1),
			flag_forward_system_time_change = (1<<2),
			flag_transparent_background = (1<<3),
			flag_default_flags = flag_forward_system_settings_change|flag_forward_system_colours_change|flag_forward_system_time_change,
			flag_default_flags_plus_transparent_background = flag_default_flags|flag_transparent_background,

			style_child_default = WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
			ex_style_child_default = WS_EX_CONTROLPARENT,

			style_popup_default = WS_SYSMENU | WS_POPUP | WS_CLIPSIBLINGS| WS_CLIPCHILDREN  | WS_CAPTION | WS_THICKFRAME,
			ex_style_popup_default = WS_EX_DLGMODALFRAME,
		};

		virtual t_uint32 get_flags() const {return flag_default_flags;}
		virtual t_uint32 get_styles() const {return style_child_default;}
		virtual t_uint32 get_ex_styles() const {return ex_style_child_default;}

		virtual t_uint32 get_class_styles() const {return CS_DBLCLKS;}
		virtual LPWSTR get_class_cursor() const {return IDC_ARROW;}
		virtual HBRUSH get_class_background() const {return NULL;}
		virtual const GUID & get_class_guid() = 0;
		virtual const char * get_window_title() {return "";};
		virtual t_uint32 get_class_extra_wnd_bytes() const {return 0;}

		virtual void on_size(t_size cx, t_size cy) {};
		void on_size();

		container_window_v2_t() : m_wnd(NULL), m_autounreg_disabled(false) {};

		HWND create(HWND wnd_parent, LPVOID create_param = 0, const ui_helpers::window_position_t & p_window_position = ui_helpers::window_position_null);

		HWND create_in_dialog_units(HWND wnd_dialog, const ui_helpers::window_position_t & p_window_position, LPVOID create_param = NULL);

		void destroy() {m_autounreg_disabled = true; GUID class_guid = get_class_guid(); DestroyWindow(m_wnd); g_window_class_manager.class_deref(class_guid);}

		static LRESULT WINAPI g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

		HWND get_wnd() const {return m_wnd;};

		LRESULT __on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

		//override me
		//you won't get called for WM_ERASEBKGRND if you specify want_transparent_background
		virtual LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)=0;

	private:
		HWND m_wnd;
		bool m_autounreg_disabled;
	};

	class print_guid_wide
	{
	public:
		print_guid_wide(const GUID & p_guid);
		inline operator const wchar_t * () const {return m_data;}
		inline const wchar_t * get_ptr() {return m_data;}
	private:
		wchar_t m_data[64];
	};

	class window_class : public pfc::refcounted_object_root
	{
	public:
		typedef pfc::refcounted_object_ptr_t<window_class> ptr;
		static int g_compare_by_value(const ptr & p1, const GUID & p2) {return pfc::compare_t(p1->m_guid, p2);}

		window_class(const GUID & p_guid, WNDPROC wndproc, t_uint32 class_styles, LPWSTR cursor, HBRUSH br_back, t_uint32 extra_wnd_bytes) : m_guid(p_guid), m_refcount(1) 
		{
			print_guid_wide wstr(m_guid);

			WNDCLASS  wc;
			memset(&wc,0,sizeof(WNDCLASS));
			wc.lpfnWndProc    = wndproc;
			wc.hInstance      = core_api::get_my_instance();
			wc.hCursor        = LoadCursor(NULL, cursor);
			wc.hbrBackground  = br_back;
			wc.lpszClassName  = wstr;
			wc.style = class_styles;
			wc.cbWndExtra  = extra_wnd_bytes;

			m_registered = (RegisterClass(&wc) != 0);
		};

		bool release()
		{
			if (--m_refcount == 0)
			{
				UnregisterClass(print_guid_wide(m_guid), core_api::get_my_instance());
				return true;
			}
			return false;
		}

		t_size m_refcount;
		GUID m_guid;
		bool m_registered;
	};

	class window_class_manager 
	{
	public:
		class window_class_release_delayed : public main_thread_callback
		{
		public:
			window_class_release_delayed(GUID const & p_guid) : m_guid(p_guid) {};
			virtual void callback_run();
			GUID m_guid;
		};

		bool class_addref (const GUID & p_guid, WNDPROC wndproc, t_uint32 class_styles, LPWSTR cursor, HBRUSH bk_back, t_uint32 extra_wnd_bytes)
		{
			t_size index = NULL;
			if (m_classes.bsearch_t(window_class::g_compare_by_value, p_guid, index))
				++m_classes[index]->m_refcount;
			else
			{
				m_classes.insert_item(new window_class(p_guid, wndproc, class_styles, cursor, bk_back, extra_wnd_bytes), index);
			}
			return m_classes[index]->m_registered;
		};
		void class_deref (const GUID & p_guid)
		{
			t_size index = NULL;
			if (m_classes.bsearch_t(window_class::g_compare_by_value, p_guid, index))
			{
				if (m_classes[index]->release())
					m_classes.remove_by_idx(index);
			}
		}
		/** Will not work if window message loop isn't on the main thread */
		void class_deref_delayed (const GUID & p_guid)
		{
			core_api::ensure_main_thread();
			if (core_api::are_services_available()) //useful check ??
				static_api_ptr_t<main_thread_callback_manager>()->add_callback(new service_impl_t<window_class_release_delayed>(p_guid));
		};
	private:
		pfc::list_t<window_class::ptr> m_classes;
	};

	extern window_class_manager g_window_class_manager;

	template <typename TBase>
	void container_window_v2_t<TBase>::on_size()
	{
		RECT rc;
		GetClientRect(get_wnd(), &rc);
		on_size(RECT_CX(rc), RECT_CY(rc));
	}

	template <typename TBase>
	HWND container_window_v2_t<TBase>::create(HWND wnd_parent, LPVOID create_param, const ui_helpers::window_position_t & p_window_position)
	{
		if (m_wnd) return NULL;

		const GUID ourGUID = get_class_guid();

		if (g_window_class_manager.class_addref(ourGUID, (WNDPROC)g_on_message, get_class_styles(), get_class_cursor(), get_class_background(), get_class_extra_wnd_bytes()))
		{
			LPVOID createparams[2] = {this, create_param};
			m_wnd = CreateWindowEx(get_ex_styles(), print_guid_wide(get_class_guid()), pfc::stringcvt::string_wide_from_utf8(get_window_title()),
				get_styles(), p_window_position.x, p_window_position.y, p_window_position.cx, p_window_position.cy,
				wnd_parent, 0, core_api::get_my_instance(), &createparams);
		}

		if (m_wnd == NULL) //we may be deleted in this case !!
			g_window_class_manager.class_deref(ourGUID);

		return m_wnd;
	}

	template <typename TBase>
	HWND container_window_v2_t<TBase>::create_in_dialog_units(HWND wnd_dialog, const ui_helpers::window_position_t & p_window_position, LPVOID create_param)
	{
		RECT rc;
		p_window_position.convert_to_rect(rc);
		MapDialogRect(wnd_dialog, &rc);
		return create(wnd_dialog, create_param, ui_helpers::window_position_t(rc));
	}

	template <typename TBase>
	LRESULT WINAPI container_window_v2_t<TBase>::g_on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		container_window_v2_t<TBase> * p_this = NULL;

		if(msg == WM_NCCREATE)
		{
			LPVOID * create_params = reinterpret_cast<LPVOID *>(((CREATESTRUCT *)(lp))->lpCreateParams);
			p_this = reinterpret_cast<container_window_v2_t<TBase>*>(create_params[0]); //retrieve pointer to class
			SetWindowLongPtr(wnd, GWL_USERDATA, (LPARAM)p_this);//store it for future use

		}
		else
			p_this = reinterpret_cast<container_window_v2_t<TBase>*>(GetWindowLongPtr(wnd,GWL_USERDATA));//if isnt wm_nccreate, retrieve pointer to class

		if (msg == WM_NCDESTROY)
			SetWindowLongPtr(wnd, GWL_USERDATA, (LPARAM)NULL);

		return p_this ? p_this->__on_message(wnd, msg, wp, lp) : uDefWindowProc(wnd, msg, wp, lp);
	}


	template <typename TBase>
	LRESULT container_window_v2_t<TBase>::__on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch (msg)
		{
		case WM_NCCREATE:
			m_wnd = wnd;
			break;
		case WM_NCDESTROY:
			if (!m_autounreg_disabled)
				g_window_class_manager.class_deref_delayed(get_class_guid());
			m_autounreg_disabled = false;
			m_wnd = NULL;
			break;
		case WM_SETTINGCHANGE:
			if (get_flags() & flag_forward_system_settings_change)
				win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
			break;
		case WM_SYSCOLORCHANGE:
			if (get_flags() & flag_forward_system_colours_change)
				win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
			break;
		case WM_TIMECHANGE:
			if (get_flags() & flag_forward_system_time_change)
				win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
			break;
		case WM_WINDOWPOSCHANGED:
			{
				LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
				if (!(lpwp->flags & SWP_NOSIZE) || (lpwp->flags & SWP_FRAMECHANGED))
					on_size();
			}
			break;
		};


		if (get_flags() & flag_transparent_background)
		{
			if (msg == WM_ERASEBKGND || (msg == WM_PRINTCLIENT && (lp & PRF_ERASEBKGND)))
			{			
				HDC dc = (HDC)wp;
				BOOL b_ret = TRUE;

				HWND wnd_parent = GetParent(wnd);
				POINT pt = {0, 0}, pt_old = {0,0};
				MapWindowPoints(wnd, wnd_parent, &pt, 1);
				OffsetWindowOrgEx(dc, pt.x, pt.y, &pt_old);
				if (msg == WM_PRINTCLIENT)
				{
					SendMessage(wnd_parent, WM_PRINTCLIENT, wp, PRF_ERASEBKGND);
				}
				else
				{
#if 0
					SendMessage(wnd_parent, WM_PRINTCLIENT,wp, PRF_ERASEBKGND);
#else
					b_ret = SendMessage(wnd_parent, WM_ERASEBKGND,wp, 0);
#endif
				}
				SetWindowOrgEx(dc, pt_old.x, pt_old.y, 0);

				return b_ret;
			}
			else if (msg==WM_WINDOWPOSCHANGED)
			{
				LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
				if (!(lpwp->flags & SWP_NOSIZE) || !(lpwp->flags & SWP_NOMOVE) || (lpwp->flags & SWP_FRAMECHANGED))
				{
					RedrawWindow(wnd, 0, 0, RDW_ERASE|RDW_INVALIDATE|RDW_ALLCHILDREN);
				}
			}
		}

		return on_message(wnd, msg, wp, lp);
	}

	class emptyClass {};
	typedef container_window_v2_t<emptyClass> container_window_v2;
	typedef container_window_v2_t<pfc::refcounted_object_root> popup_container_window;
}

namespace ui_extension{

	/** \brief Wraps ui_helpers::container_window into a panel */
	template <class W = ui_helpers::container_window_v2_t<window> >
	class container_uie_window_v2_t : public W
	{
		window_host_ptr m_host;

	public:
		HWND create_or_transfer_window(HWND parent, const window_host_ptr & host, const ui_helpers::window_position_t & p_position)
		{
			if (W::get_wnd())
			{
				ShowWindow(W::get_wnd(), SW_HIDE);
				SetParent(W::get_wnd(), parent);
				m_host->relinquish_ownership(W::get_wnd());
				m_host = host;

				SetWindowPos(get_wnd(), NULL, p_position.x, p_position.y, p_position.cx, p_position.cy, SWP_NOZORDER);
			}
			else
			{
				m_host = host; //store interface to host
				create(parent, get_create_param(), p_position);
			}

			return W::get_wnd();
		}
		virtual void destroy_window() {destroy();m_host.release();}

		virtual bool is_available(const window_host_ptr & p)const {return true;}
		const window_host_ptr & get_host() const {return m_host;}
		virtual HWND get_wnd()const{return W::get_wnd();}

		//override me
		virtual void set_config(stream_reader * p_reader){};
		virtual LPVOID get_create_param() {return this;} //lpCreateParams in CREATESTRUCT struct in WM_NCCREATE/WM_CREATE is a pointer to an array of LPVOIDs. This is the second LPVOID in the array.
	};

	typedef container_uie_window_v2_t<> container_uie_window_v2;

};
