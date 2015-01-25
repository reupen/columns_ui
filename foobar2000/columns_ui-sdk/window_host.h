#ifndef _UI_EXTENSION_HOST_H_
#define _UI_EXTENSION_HOST_H_

/**
* \file window.h
* \brief Window Host API
*/

namespace ui_extension{

/**
* \brief Interface for window_host service.
*
* This interface is to be implemented by panel hosts.
*
* \remark The host may not be dialog managed.
*
* \remark Hosts must forward the following messages to hosted windows:
* - WM_SETTINGCHANGE
* - WM_SYSCOLORCHANGE
*/
class NOVTABLE window_host : public service_base
{
public:

	/**
	 * \brief Get the unique ID of the host.
	 *
	 * This GUID is used to identify a specific host.
	 *
	 * \return host GUID
	 */
	virtual const GUID & get_host_guid()const=0;

	/**
	 * \brief Notify host about changed size limits of a hosted extension.
	 *
	 * \pre May only be called by a hosted UI extension.
	 *
	 * \param[in] wnd window handle of the extension's window
	 * \param[in] flags a combination of SLC_* flags indicating which size limits changed
	 *
	 * \see ui_extension::size_limit_flag_t
	 */
	virtual void on_size_limit_change(HWND wnd, unsigned flags)=0;

	/**
	* \brief Called by panels hosted by this host to find out whether the host supports resizing
	* 
	* \param[in] wnd   handle to the window to test
	* \return   combination of uie::size_height and uie::size_width to indicate whether
	*           the width or height can be modified
	*
	* \pre May only be called by a hosted UI extension.
	*
	* \see ui_extension::resize_flag_t
	*/
	virtual unsigned is_resize_supported(HWND wnd)const=0;

	/**
	* \brief Called by ui extension hosted by this host to resize your window.
	* 
	* Implementers: If you cannot fully meet the request, do not attempt to partially fulfil it.
	* For example, if a request is made to modify both the width and height but you can only modify
	* one if those.
	* 
	* \param[in] wnd   handle to the window to test
	* \return   combination of uie::size_height and uie::size_width to indicate whether
	*           the width or height is being modified
	*
	* \see ui_extension::resize_flag_t
	*/
	virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height)=0;

	/**
	 * \brief Instantiates ui_status_text_override service, that can be used to display status messages.
	 *
	 * Implementers: if you wish to display status bar text in the main window,
	 * simply use ui_control::override_status_text_create. Hybrid panel-hosts can forward the call
	 * to their host.
	 * If alternatively you wish to display the text in your own status area,
	 * you are responsible for implementing ui_status_text_override. Be sure to obey certain conventions:
	 * - Releasing the ui_status_text_override object should restore the text if revert_text has not been
	 *   called.
	 *
	 * \pre May only be called by a hosted UI extension.
	 *
	 * \param[out] p_out   receives new ui_status_text_override instance.
	 * \return true on success, false on failure (out of memory / no GUI loaded / etc)
	 */
	virtual bool override_status_text_create(service_ptr_t<ui_status_text_override> & p_out) = 0;

	/**
	 * \brief Query if keyboard shortcuts should be processed.
	 *
	 * Use this to determine, if keyboard shortcuts should be processed.
	 * Do not process them, if this method returns false.
	 * Shortcuts can be processed using the keyboard_shortcut_manager service
	 * from the foobar2000 SDK.
	 *
	 * Keyboard shortcuts would not be processed, for example, if the panel
	 * is hosted in a popup window. In this case the method returns false.
	 *
	 * If the method does return true, whether you process keyboard shortcuts
	 * will depend on the type of functionality your control offers. For example,
	 * in a edit control you may wish not to process keyboard shortcuts.
	 * 
	 * The user must be able to navigate using the tab key. If VK_TAB is not
	 * processed by the keyboard_shortcut_manager and the TAB press is not being
	 * handled by the dialog manager, you should use g_on_tab() to change to the
	 * next control.
	 *
	 * \par Usage example
	 * \code
	 * case WM_KEYDOWN:
	 *     if (p_host->get_keyboardshortcuts_enabled() && static_api_ptr_t<keyboard_shortcut_manager>()->on_keydown_xxxx(wp)) break;
	 *     else if (wp == VK_TAB) window::g_on_tab(wnd);
	 * 	   break;
	 * \endcode
	 *
	 * \pre May only be called by a hosted UI extension.
	 *
	 * \return whether keyboard shortcuts should be processed
	 */
	virtual bool get_keyboard_shortcuts_enabled()const
	{
		return true;
	}

	/**
	 * \brief Query if extension window is visible.
	 *
	 * An extension that is not visible does not imply that its window has been hidden using ShowWindow
	 *
	 * \param[in] wnd   handle to the window to test
	 * \pre May only be called by a hosted UI extension.
	 *
	 * \return whether window is visible.
	 */
	virtual bool is_visible(HWND wnd)const=0;

	/**
	 * \brief Query if extension window can be hidden or shown.
	 *
	 * \param[in] wnd   handle to the window to test
	 * \param[in] desired_visibility   whether you want the window to be visible
	 * \pre May only be called by a hosted UI extension.
	 *
	 * \return whether the required visiblility can be set.
	 */
	virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility)const=0;

	/**
	 * \brief Hides or shows extension window.
	 *
	 * \param[in] wnd   handle to the window to test
	 * \param[in] visibility   whether you want the window to be visible
	 * \pre May only be called by a hosted UI extension.
	 *
	 * \return whether the required visiblility was be set.
	 */
	virtual bool set_window_visibility(HWND wnd, bool visibility)=0;

	/**
	 * \brief Relinquish ownership of a UI extension instance.
	 *
	 * Call this to remove control of an extension window from the host.
	 * The host will not destroy the window as a result of this call.
	 * However, the window may be destroyed, if the host destroys the
	 * containing winow, so be sure to call <code>SetParent</code> first.
	 * 
	 * Reasons for calling this method include: another host tries to take
	 * ownership of an existing extension instance, the window should be
	 * destroyed/closed, or the window is to be turned into a popup dialog.
	 *
	 * \pre May only be called by a hosted UI extension.
	 *
	 * \param[in] wnd window handle of the extension's window
	 *
	 * \see window::create_or_transfer_window
	 */
	virtual void relinquish_ownership(HWND wnd)=0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(window_host);
};

/** 
 *  \brief Sub-class of window_host, providing methods for retrieving information about children panels 
 *
 *  In addition to the methods exposed through the window_host interface, 
 *  this interface provides information about children panels that are in the same container/splitter.
 *  Note that these do not necessarily have the same window_host instance.
 */
class window_host_ex : public window_host
{
public:
	/**
	 * \brief Retreives list of children panels.
	 *
	 * \param[in]	p_out 			receives the list of panels
	 */
	virtual void get_children(pfc::list_base_t<uie::window_ptr> & p_out)=0;

	/**
	 * \brief Retreives deinitialising date.
	 *
	 * \return			 			deinitialising state
	 */
	//virtual bool is_deinitialising()=0;

	FB2K_MAKE_SERVICE_INTERFACE(window_host_ex, window_host);
};

/** 
 *  \brief Sub-class of window_host, providing methods for external control
 *
 *  In addition to the methods exposed through the window_host interface, 
 *  this interface provides information about the host and its state as well
 *  as methods to manage hosted extensions.
 */
class NOVTABLE window_host_with_control : public window_host
{
public:

	/**
	 * \brief Get supported UI extension types.
	 *
	 * \return a combination of window_flag::TYPE_* flags to indicate recommended types for the host
	 * \see window_flag::window_type
	 */
	virtual unsigned get_supported_types() const=0; 

	/**
	 * \brief Insert new instance of a UI extension.
	 *
	 * Creates an instance of the specified extension and inserts it into the host's
	 * client area. Single-instance extensions should removed themselves from the
	 * old host, if any.
	 *
	 * \pre May only be called, if is_available() returned true.
	 *
	 * \param[in] guid unique ID of the UI extension to be inserted
	 * \param[in] height desired height of the new panel
	 * \param[in] width desired width of the new panel
	 *
	 * \see is_available, window::init_or_take_ownership
	 */
	virtual void insert_extension (const GUID & guid, unsigned height, unsigned width)=0;

	/**
	 * \brief Insert existing instance of a UI extension.
	 *
	 * Inserts the given UI extension instance into the host's client area.
	 *
	 * \pre May only be called, if is_available() returned true.
	 *
	 * \param[in] p_ext pointer to the UI extension instance to be inserted
	 * \param[in] height desired height of the new panel
	 * \param[in] width desired width of the new panel
	 *
	 * \see is_available, window::init_or_take_ownership
	 */
	virtual void insert_extension (window_ptr & p_ext, unsigned height, unsigned width)=0; //insert existing instance (or new if it hasnt been initialised)

	/**
	 * \brief Get the name of the host.
	 *
	 * Get a user-readable name of the host.
	 *
	 * \warning
	 * Do not use the name to identify hosts; use host GUIDs instead.
	 *
	 * \param[out] out receives the name of the host, e.g. "My UI/Sidebar"
	 *
	 * \see get_host_guid
	 */
	virtual void get_name(pfc::string_base & out) const =0;

	/**
	 * \brief Get availability of the host.
	 *
	 * \return true if it is possible to insert a UI extension into the host.
	 *
	 * \see insert_extension(const GUID &, unsigned, unsigned), insert_extension(window *, unsigned, unsigned)
	 */
	virtual bool is_available() const=0;

	FB2K_MAKE_SERVICE_INTERFACE(window_host_with_control, window_host);

};

/**
 * \brief Service factory for window hosts.
 * \par Usage example
 * \code
 * static window_host_factory< my_window_host > foo_host;
 * \endcode
 */
template<class T>
class window_host_factory : public service_factory_t<T> {};

/**
 * \brief Service factory for window hosts.
 * \par Usage example
 * \code
 * static window_host_factory< my_window_host > foo_host;
 * \endcode
 * The static instance of <code>my_window_host</code> can be accessed
 * as <code>foo_host.get_static_instance()</code>.
 */
template<class T>
class window_host_factory_single : public service_factory_single_t<T> 
{
public:
	inline operator uie::window_host_ptr () {return uie::window_host_ptr(&get_my_instance());}
};

/**
 * \brief Service factory for window hosts.
 * \par Usage example
 * \code
 * static window_host_factory_transparent< my_window_host > foo_host2;
 * \endcode
 * The static instance of <code>my_window_host</code> can be accessed
 * as <code>foo_host2</code>.
 */
template<class T>
class window_host_factory_transparent_single : public service_factory_single_transparent_t<T> {};

}

#endif