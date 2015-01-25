#ifndef _VIS_EXTENSION_H_
#define _VIS_EXTENSION_H_

/**
* \file visualisation.h
* \brief Visualisation Renderer API
*/

namespace ui_extension
{

/**
 * \brief Interface for visualisation extension hosts.
 */
class NOVTABLE visualisation_host : public service_base
{
public:

	/**
	* \brief Interface to paint on a visualistion host
	*
	* \note Releasing the object ends the paint operation, frees the DC and updates the screen.
	*/
	class painter_t : public pfc::refcounted_object_root
	{
	public:
		virtual HDC get_device_context()const=0;
		virtual const RECT * get_area()const=0;
	};

	typedef pfc::refcounted_object_ptr_t<painter_t> painter_ptr;

	/**
	 * \brief Creates a painter_t object
	 */
	virtual void create_painter(painter_ptr & p_out)=0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(visualisation_host);
};

/**
 * \brief Service factory for vis extension hosts.
 * \par Usage example
 * \code
 * static vis_extension_host_factory< my_vis_extension_host > foo_host;
 * \endcode
 */
template<class T>
class visualisation_host_factory : public service_factory_t<T> {};

template<class T>
class visualization_host_factory : public visualisation_host_factory<T> {};

/**
 * \brief Interface for vis_extension service.
 * This service allows you to embed the default Columns UI visualisation, and any other
 * visualisations that implement it, into your own window.
 */
class NOVTABLE visualisation : public extension_base
{
public:

	/**
	 * \brief Enables the visualisation.
	 *
	 * \param [in]	p_host		Pointer to host to use for drawing operations
	 */
	virtual void enable(const visualisation_host_ptr & p_host)=0; 

	/**
	 * \brief Paints the standard background of your visualisation.
	 */
	virtual void paint_background(HDC dc, const RECT * rc_area)=0; 

	/**
	 * \brief Disables the visualisation.
	 */
	virtual void disable()=0; 

	/**
	 * \brief Create extension by GUID.
	 *
	 * \param[in] guid GUID of a vis_extension
	 */
	static inline void create_by_guid(const GUID & guid, visualisation_ptr & p_out)
	{
		service_enum_t<ui_extension::visualisation> e;
		visualisation_ptr ptr;

		
		if (e.first(ptr)) do {
			if (ptr->get_extension_guid() == guid)
			{
				p_out.copy(ptr);
				return;
				
			}
		} while(e.next(ptr));

	}

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(visualisation);
};

/**
 * \brief Service factory for vis extensions.
 * \par Usage example
 * \code
 * static vis_extension_factory< my_vis_extension > foo_vis;
 * \endcode
 */
template<class T>
class visualisation_factory : public service_factory_t<T> {};

//template<class T>
//class visualization_factory : public visualisation_factory<T>{};

}
#endif