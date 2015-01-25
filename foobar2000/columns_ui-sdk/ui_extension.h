#ifndef _UI_EXTENSION_H_
#define _UI_EXTENSION_H_

#define UI_EXTENSION_VERSION "6.4"

#define uie ui_extension

#include "../SDK/foobar2000.h"

/**
* \file ui_extension.h
* \brief User interface extension API
* \author musicmusic
* \author Holger Stenger (original doxygen comments)
* \version 6.3
*/

//ripped from stream_reader::read_string_raw
void stream_to_mem_block(stream_reader * p_source, pfc::array_t<t_uint8> & p_out,abort_callback & p_abort, unsigned p_sizehint = 0, bool b_reset = false);

class stream_writer_memblock_ref : public stream_writer
{
public:
	stream_writer_memblock_ref(pfc::array_t<t_uint8> & p_data, bool b_reset = false) : m_data(p_data) {if (b_reset) m_data.set_size(0);};
	void write(const void * p_buffer,t_size p_bytes,abort_callback & p_abort)
	{
		m_data.append_fromptr((t_uint8*)p_buffer, p_bytes);
	}
private:
	pfc::array_t<t_uint8> & m_data;
};

class stream_writer_memblock : public stream_writer
{
public:
	stream_writer_memblock() {};
	void write(const void * p_buffer,t_size p_bytes,abort_callback & p_abort)
	{
		m_data.append_fromptr((t_uint8*)p_buffer, p_bytes);
	}
	pfc::array_t<t_uint8> m_data;
};

 /**
 * \brief Namespace for UI Extension services
 */
namespace ui_extension
{

	 /**
	 * \brief Flags indicating the type of the UI extension.
	 *
	 * \remarks Combine multiple flags using bitwise or,
	 * if an extension supports more than one type.
	 *
	 * \see window::get_type, window_host_with_control::get_supported_types
	 */
	enum window_type_t
	{
		/** The extension is a sidebar panel. */
		type_panel		= (1 << 0),
		/** The extension is a toolbar panel. */
		type_toolbar	= (1 << 1),
		/** The extension is a playlist panel. */
		type_playlist	= (1 << 2),
		/** The extension is a layout panel. */
		type_layout		= (1 << 3),
		/** The extension is a splitter panel. */
		type_splitter	= (1 << 4),
	};
		
		
	/**
	 * \brief Flags indicating a change of a size limit.
	 *
	 * Combine multiple flags using bitwise or,
	 * if more than one size limit changed.
	 *
	 * \see window_host::on_size_limit_change
	 */
	enum size_limit_flag_t
	{
		/** The minimum width changed. */
		size_limit_minimum_width	= (1 << 0),
		/** The maximum width changed. */
		size_limit_maximum_width	= (1 << 1),
		/** The minimum height changed. */
		size_limit_minimum_height	= (1 << 2),
		/** The maximum height changed. */
		size_limit_maximum_height	= (1 << 3),
		/** All size limits changed. */
		size_limit_all				= size_limit_minimum_width|size_limit_maximum_width|size_limit_minimum_height|size_limit_maximum_height,
	};
		
	/**
	 * \brief Flags indicating whether the size and height parameters are valid.
	 *
	 * Combine multiple flags using bitwise or.
	 *
	 * \see ui_extension::window_host::on_size_limit_change
	 */
	enum resize_flag_t
	{
		/** The width parameter is valid. */
		size_width		= (1 << 0),
		/** The height parameter is valid. */
		size_height		= (1 << 1),
	};

	class visualisation;
	class visualisation_host;

	typedef visualisation visualization;

	typedef service_ptr_t<class visualisation> visualisation_ptr;
	typedef service_ptr_t<class visualisation_host> visualisation_host_ptr;

	typedef visualisation_ptr visualization_ptr;
	typedef visualisation_host_ptr visualization_host_ptr;

	typedef service_ptr_t<class window> window_ptr;
	typedef service_ptr_t<class window_host> window_host_ptr;

	typedef service_ptr_t<class splitter_window> splitter_window_ptr;
	typedef service_ptr_t<class splitter_window_v2> splitter_window_v2_ptr;
	typedef service_ptr_t<class menu_window> menu_window_ptr;
	typedef service_ptr_t<class playlist_window> playlist_window_ptr;

	typedef pfc::refcounted_object_ptr_t<class menu_node_t> menu_node_ptr;
}

#include "menu.h"
#include "window_host.h"
#include "base.h"
#include "window.h"
#include "win32_helpers.h"
#include "window_helper.h"
#include "container_window_v2.h"
#include "splitter.h"
#include "visualisation.h"
#include "imp_helper.h"
#include "buttons.h"
#include "columns_ui.h"
#include "columns_ui_appearance.h"

#endif