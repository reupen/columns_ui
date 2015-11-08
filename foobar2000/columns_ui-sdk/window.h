#ifndef _UI_EXTENSION_EXTENSION_H_
#define _UI_EXTENSION_EXTENSION_H_

/**
* \file window.h
* \brief Window API
*/

namespace ui_helpers
{
	/** \brief Simple class used to hold window positions */
	class window_position_t
	{
	public:
		int x;
		int y;
		unsigned cx;
		unsigned cy;
		window_position_t(int i_x, int i_y, unsigned u_cx, unsigned u_cy)
			: x(i_x), y(i_y), cx (u_cx), cy(u_cy)
		{};
		window_position_t(const RECT & rc)
			: x(rc.left), y(rc.top), cx (rc.right-rc.left), cy(rc.bottom-rc.top)
		{};
		window_position_t(){};
		window_position_t(HWND wnd_relative, int i_x, int i_y, unsigned u_cx, unsigned u_cy)
		{
			RECT rc;
			GetClientRect(wnd_relative, &rc);
			MapWindowPoints(wnd_relative, HWND_DESKTOP, (LPPOINT)&rc, 2);
			x = rc.left;// + (RECT_CX(rc)-cx)/2;
			y = rc.top;// + (RECT_CY(rc)-cy)/2;
			cx = u_cx;
			cy = u_cy;
		};

		void convert_to_rect(RECT & p_out) const
		{
			p_out.left = x;
			p_out.top = y;
			p_out.right = x + cx;
			p_out.bottom = y + cy;
		}
		void set_from_rect(const RECT & rc)
		{
			x = (rc.left); y = (rc.top); cx = (rc.right-rc.left); cy = (rc.bottom-rc.top);
		}
	};

	/** The window position {0,0,0,0} */
	extern const window_position_t window_position_null;
};

namespace ui_extension
{

	/** \brief Class describing the size limits of a window */
	struct size_limit_t
	{
		/** \brief The minimum height of the window */
		unsigned min_height;
		/** \brief The maximum height of the window */
		unsigned max_height;
		/** \brief The minimum width of the window */
		unsigned min_width;
		/** \brief The maximum width of the window */
		unsigned max_width;
		/**
		* \brief Default constructor
		* \remark Initialises max_height, max_width with MAXLONG because WIN32 API takes windows sizes as signed 
		*/
		size_limit_t()
			: min_height(0), max_height(MAXLONG), min_width(0), max_width(MAXLONG)
		{};
	};

	/**
	* \brief Interface for window service.
	*/
	class NOVTABLE window : public extension_base
	{
	public:

		/**
		* \brief Gets whether the panel is single instance or not.
		*
		* \note Do not explicitly override. The service factory implements this method.
		*/
		virtual const bool get_is_single_instance() const = 0;

		/**
		* \brief Gets the category of the extension.
		* 
		* Categories you may use are "Toolbars", "Panels", "Splitters", "Playlist views" and "Visualisations"
		*
		* \param [out]	out		receives the category of the panel, utf-8 encoded
		*/
		virtual void get_category(pfc::string_base & out) const =0;

		/**
		* \brief Gets the short, presumably more user-friendly than the name returned by get_name, name of the panel.
		*
		* \param [out]	out		receives the short name of the extension, e.g. "Order" instead
		*						of "Playback order", or "Playlists" instead of "Playlist switcher"
		* 
		* \return				true if the extension has a short name
		*/
		virtual bool get_short_name(pfc::string_base & out) const {return false;}; //short/friendly name, e.g. order vs. playback order, playlists vs. playlist switcher

		/**
		* \brief Gets the description of the extension.
		*
		* \param [out]	out		receives the description of the extension,
		*						e.g. "Drop-down list for displaying and changing the current playback order"
		* 
		* \return true if the extension has a description
		*/
		virtual bool get_description(pfc::string_base & out) const {return false;}; //e.g. "Drop-down list to display the current playback order and to allow you to select a new playback order"

		/**
		* \brief Gets the type of the extension.
		*
		* \return				a combination of ui_extension::type_* flags
		*
		* \see ui_extension::window_type_t
		*/
		virtual unsigned get_type() const =0;

		/**
		* \brief Gets whther the panel prefers to be created in multiple instances.
		*
		* For example, a spacer panel.
		*
		* \return				true iff the panel prefers to be created in multiple instances
		*/
		virtual bool get_prefer_multiple_instances() const {return false;}

		/**
		* \brief Get availability of the extension.
		*
		* This method is called before create_or_transfer() to test, if this call will be legal.
		* If this instance is already hosted, it should check whether the given host's GUID equals its
		* current host's GUID, and should return <code>false</code>, if it does. This is mostly important 
		* for single instance extensions.
		*
		* Extensions that support multiple instances can generally return <code>true</code>.
		*
		* \return whether this instance can be created in or moved to the given host
		*/
		virtual bool is_available(const window_host_ptr & p_host) const =0;

		/**
		* \brief Create or transfer extension window.
		*
		* Create your window here.
		*
		* In the case of single instance panels, if your window is already created, you must
		* (in the same order):
		* - Hide your window. i.e: \code ShowWindow(wnd, SW_HIDE) \endcode
		* - Set the parent window to to wnd_parent. I.e. \code SetParent(get_wnd(), wnd_parent) \endcode
		* - Move your window to the new window position. I.e.:
		*   \code SetWindowPos(get_wnd(), NULL, p_position.x, p_position.y, p_position.cx, p_position.cy, SWP_NOZORDER); \endcode
		* - Call relinquish_ownership() on your current host.
		*
		* Other rules you should follow are:
		* - Ensure you are using the correct window styles. The window MUST have the WS_CHILD
		*   window style. It MUST NOT have the WS_POPUP, WS_CAPTION styles.
		* - The window must be created hidden.
		* - Use WS_EX_CONTROLPARENT if you have child windows that receive keyboard input,
		*   and you want them to be included in tab operations in the host window.
		* - Do not directly create a common control as your window. You must create
		*   a window to contain any common controls, and any other controls that
		*   communicate to the parent window via WM_COMMAND and WM_NOTIFY window messages.
		* - Under NO CIRCUMSTANCES may you subclass the host window.
		* - If you are not hosting any panels yourself, you may dialog manage your window if you wish.
		* - The window MUST have a dialog item ID of 0.
		*
		* \pre May only be called if is_available() returned true.
		*
		* \param [in]	wnd_parent		Handle to the window to use as the parent for your window
		* \param [in]	p_host			Pointer to the host that creates the extension.
		*								This parameter may not be NULL.
		* \param [in]	p_position		Initial position of the window
		* \return						Window handle of the panel window
		*/
		virtual HWND create_or_transfer_window(HWND wnd_parent, const window_host_ptr & p_host, const ui_helpers::window_position_t & p_position = ui_helpers::window_position_null)=0; 

		/**
		* \brief Destroys the extension window.
		*/
		virtual void destroy_window()=0; 

		/**
		* \brief Gets extension window handle.
		*
		* \pre May only be called on hosted extensions.
		*
		* \return						Window handle of the extension window
		*/
		virtual HWND get_wnd() const =0;

		/**
		* \brief Gets size limits of the window.
		* 
		* Override if you like, or just handle WM_GETMINMAXINFO.
		*
		* \note	This function is reserved for future use. Handle WM_GETMINMAXINFO
		*		for now instead.
		*
		* \param [out]	p_out			Receives the size limits of the window.
		*/
		virtual void get_size_limits(size_limit_t & p_out) const
		{
			MINMAXINFO mmi;
			memset(&mmi, 0, sizeof(MINMAXINFO));
			mmi.ptMaxTrackSize.x = MAXLONG;
			mmi.ptMaxTrackSize.y = MAXLONG;
			SendMessage(get_wnd(), WM_GETMINMAXINFO, 0, (LPARAM)&mmi);

			p_out.min_height = mmi.ptMinTrackSize.y;
			p_out.min_width = mmi.ptMinTrackSize.x;
			p_out.max_width = mmi.ptMaxTrackSize.x;
			p_out.max_height = mmi.ptMaxTrackSize.y;
		}

		/**
		* \brief Creates extension by GUID.
		*
		* \param [in]	guid			GUID of a ui_extension
		* \param [out]	p_out			Receives a pointer to the window.
		*
		* \return						true if the window was found and instantiated.
		* 								You may assume that if the method returns true,
		*								p_out is a valid pointer.
		*/
		static inline bool create_by_guid(const GUID & guid, window_ptr & p_out) 
		{
			service_enum_t<window> e;
			service_ptr_t<window> ptr;


			if (e.first(ptr)) do {
				if (ptr->get_extension_guid() == guid)
				{
					p_out.copy(ptr);
					return true;

				}
			} while(e.next(ptr));
			return false;
		}

		/**
		* \brief Helper function. Activates next or previous window.
		* 
		* \param [in]	wnd_focus		Window you want the next or previous window handle respective to.
		* \return						The handle to the window that was activated, or NULL if none was.
		*/
		static HWND g_on_tab(HWND wnd_focus);

		/**
		* \brief	Helper function. Processes keyboard shortcuts using keyboard_shortcut_manager_v2::process_keydown_simple().
		*			Requires foobar2000 >= 0.9.5.
		* 
		* \param [in]	wp				Key down message WPARAM value.
		* \return						If a shortcut was executed.
		*/
		static bool g_process_keydown_keyboard_shortcuts(WPARAM wp);
		
		FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(window);
	};

	/**
	* \brief Subclass of ui_extension::window, specifically for menubars.
	*/
	class NOVTABLE menu_window : public window
	{
	public:

		/**
		* \brief Called by host when a menu accelerator is pressed
		*
		* Called by host in its WM_MENUCHAR handler to notify extension that a menu was requested to be opened. 
		* You should check whether the accelerator key pressed is one of yours.
		*
		* \pre May only be called on hosted extensions.
		*
		* \param[in] chr character that was pressed
		* \return whether you claimed the accelerator key, and showed/will show your menu
		*/
		virtual bool on_menuchar(unsigned short chr)=0; 

		/**
		* \brief Called by host to indicate you should focus your menu.
		*
		* \pre May only be called on hosted extensions.
		*/
		virtual void set_focus()=0;

		/**
		* \brief Retrieve whether the menu has the keyboard focus..
		*
		* \pre May only be called on hosted extensions.
		* \return whether your menu has keyboard focus
		*/
		virtual bool is_menu_focused()const=0;

		/**
		* \brief	Indicates that you should underline menu access keys in your menu
		*
		* \remark	Applicable only if your menu underlines menu access keys only when 
		* 			activated by the keyboard. This is typically determined by the
		*			SPI_GETKEYBOARDCUES system parameter.
		*
		* \remark	Do not change the state within this function call. Use PostMessage.
		*
		* \par		Implementation example
		* \code
		* PostMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR , UISF_HIDEACCEL), 0);
		* \endcode
		*/
		virtual void show_accelerators()=0;

		/**
		* \brief	Indicates that you should stop underlining menu access keys in your menu
		*
		* \remark	Applicable only if your menu underlines menu access keys only when 
		* 			activated by the keyboard. This is typically determined by the
		*			SPI_GETKEYBOARDCUES system parameter.
		*
		* \remark	Do not change the state within this function call. Use PostMessage.
		*
		* \par		Implementation example
		* \code
		* BOOL b_showkeyboardcues = TRUE; 
		* SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &b_showkeyboardcues, 0);
		* PostMessage(wnd_menu, WM_UPDATEUISTATE, MAKEWPARAM(b_showkeyboardcues ? UIS_CLEAR : UIS_SET , UISF_HIDEACCEL), 0);
		* \endcode
		*/
		virtual void hide_accelerators()=0;

		FB2K_MAKE_SERVICE_INTERFACE(menu_window, window);
	};

	/**
	* \brief Subclass of ui_extension::window for playlist views
	*/
	class NOVTABLE playlist_window : public window
	{
	public:

		/**
		* \brief Called by host to indicate you should focus your window.
		*
		* \pre May only be called on hosted extensions.
		*/
		virtual void set_focus()=0;

		FB2K_MAKE_SERVICE_INTERFACE(playlist_window, window);
	};

	/** \brief Standard implementation of ui_extension::menu_node_command_t, for an "Options" menu item */
	class menu_node_configure : public ui_extension::menu_node_command_t
	{
		window_ptr p_this;
		pfc::string8 m_title;
	public:
		virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags) const
		{
			p_out = m_title;
			p_displayflags= 0;
			return true;
		}
		virtual bool get_description(pfc::string_base & p_out) const
		{
			return false;
		}
		virtual void execute()
		{
			p_this->show_config_popup(p_this->get_wnd());
		}
		menu_node_configure(window * wnd, const char * p_title = "Options") : p_this(wnd), m_title(p_title) {};
	};

	template<class T>
	class window_factory;

	template<class T>
	class window_factory_single;

	template<class T>
	class window_factory_transparent_single;

}

template<class T, bool B>
class window_implementation : public T
{
	virtual const bool get_is_single_instance() const{return B;}
protected:
	window_implementation() {};

	friend ui_extension::window_factory<T>;
	friend ui_extension::window_factory_single<T>;
	friend ui_extension::window_factory_transparent_single<T>;
};

namespace ui_extension{

	/**
	* \brief Service factory for multiple instance windows.
	* \par Usage example
	* \code
	* static window_factory< my_ui_extension > foo_extension;
	* \endcode
	*/
	template<class T>
	class window_factory : public service_factory_base_t<window>
	{
	public:
		window_factory() : service_factory_base_t<window>()
		{
		}

		~window_factory()
		{
		}

		virtual void instance_create(service_ptr_t<service_base> & p_out)
		{
			service_impl_t<::window_implementation<T, false> > * item = new service_impl_t<::window_implementation<T, false> >;
			p_out = (service_base*)(window*)(::window_implementation<T, false>*)item;
		}
	};

	/**
	* \brief Service factory for single instance windows.
	* \par Usage example
	* \code
	* static window_factory_single< my_window > foo_extension2;
	* \endcode
	* The static instance of <code>my_window</code> can be accessed
	* as <code>foo_extension2.get_static_instance()</code>.
	*/
	template<class T>
	class window_factory_single : public service_factory_base_t<window>
	{
		service_impl_single_t<window_implementation<T, true> > g_instance;
	public:
		window_factory_single() : service_factory_base_t<window>() {}

		~window_factory_single() {}

		virtual void instance_create(service_ptr_t<service_base> & p_out)
		{
			p_out = (service_base*)(window*)(window_implementation<T, true>*)&g_instance;
		}

		inline T& get_static_instance() {return (T&)g_instance;}
	};

	/**
	* \brief Service factory for single instance windows.
	* \par Usage example
	* \code
	* static window_factory_single_transparent< my_window > foo_extension3;
	* \endcode
	* The static instance of <code>my_window</code> can be accessed
	* as <code>foo_extension3</code>.
	*/
	template<class T>
	class window_factory_transparent_single : public service_factory_base_t<window>, public service_impl_single_t<window_implementation<T, true> >
	{	
	public:
		window_factory_transparent_single() : service_factory_base_t<window>() {}

		virtual void instance_create(service_ptr_t<service_base> & p_out)
		{
			p_out = (service_base*)(window*)(window_implementation<T, true>*)this;
		}

		inline T& get_static_instance() const {return *(T*)this;}
	};


	/**
	* \brief Helper class to hold information about ui_extension services
	*/
	class window_info_simple
	{
	public:
		GUID guid;
		pfc::string8 name;
		pfc::string8 category;
		bool prefer_multiple_instances;
		unsigned type;
		window_info_simple() : guid(pfc::guid_null), prefer_multiple_instances(false), type(NULL)
		{};
	};

	/**
	* \brief Helper class to hold information about many ui_extension services
	*/
	class window_info_list_simple: public pfc::list_t<window_info_simple>
	{
	public:
		/**
		* \brief window_info_simple comparison function
		*/
		static int compare(const window_info_simple &n1,const window_info_simple &n2)
		{
			int rv = 0;
			rv =  uStringCompare(n1.category,n2.category);
			if (!rv) rv = uStringCompare(n1.name,n2.name);
			return rv;
		}
		/**
		* \brief Helper function to get the name of a ui_extension by its GUID.
		* \pre You must populate the list first
		*/
		void get_name_by_guid (const GUID & in, pfc::string_base & out);
		/**
		* \brief Helper function to sort the extensions, first by category, then by name.
		*/
		inline void sort()
		{
			pfc::list_t<window_info_simple>::sort_t(compare);
		}
	};

}
#endif
