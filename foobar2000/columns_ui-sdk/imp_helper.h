#if !defined(UI_EXTENSION_HELPER_H)
#define UI_EXTENSION_HELPER_H

/**
 * \file imp_helper.h
 * \brief User interface extension API helper
 * \author Holger Stenger, musicmusic
 */

namespace ui_extension{

	template< window_type_t t_window_type, typename t_base = window >
	class NOVTABLE window_base_t : public t_base
	{
	protected:
		HWND m_wnd;
		window_host_ptr m_host;

	public:
		window_base_t()
			: m_wnd(NULL)
		{};

		virtual unsigned get_type() const {return t_window_type;}

		virtual bool is_available(window_host * p_host) const
		{
			if (get_is_single_instance()) {
				return !p_host.is_valid() || p_host->get_host_guid() != m_host->get_host_guid();
			} else {
				return true;
			}
		}

		virtual HWND create_or_transfer_window(HWND wnd_parent, const window_host_ptr & p_host)
		{
			if (m_wnd == NULL) {
				// Create new window.

				// Store host.
				host = p_host;

				// create window
				m_wnd = create_window(wnd_parent);
			} else {
				// Transfer existing window to new host.
				// Possibly extend this to handle free-floating <-> hosted transitions.

				// Set new parent window.
				SetParent(m_wnd, wnd_parent);

				// Tell old host to let us go. We need to do this after using SetParent()!
				m_host->relinquish_ownership(m_wnd);

				// Store new host.
				m_host = p_host;
			}

			// ensure the window is not visible
			ShowWindow(wnd, SW_HIDE);

			return wnd;
		}

		virtual HWND get_wnd() const {return m_wnd;}

		virtual void destroy_window()
		{
			if (m_wnd != NULL) {
				DestroyWindow(m_wnd);
				m_wnd = NULL;
			}
			m_host.release();
		}

	protected:
		// Override these in subclasses.

		virtual HWND create_window(HWND wnd_parent) = 0;

		virtual void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort){};
		virtual void get_config(stream_writer * p_writer, abort_callback & p_abort) const {};

	};

}

#endif /* !defined(UI_EXTENSION_HELPER_H) */
