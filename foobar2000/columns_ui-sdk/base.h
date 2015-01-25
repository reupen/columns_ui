#ifndef _COLUMNS_SDK_BASE_H_
#define _COLUMNS_SDK_BASE_H_

namespace ui_extension
{
	/** \brief Base class for ui_extension::window and ui_extension::visualisation classes */
	class NOVTABLE extension_base : public service_base
	{
	public:

		/**
		* \brief Get unique ID of extension.
		*
		* This GUID is used to identify a specific extension.
		*
		* \return extension GUID
		*/
		virtual const GUID & get_extension_guid() const = 0;

		/**
		* \brief Get a user-readable name of the extension.
		*
		* \warning
		* Do not use the name to identify extensions; use extension GUIDs instead.
		*
		* \param [out] out receives the name of the extension, e.g. "Spectrum analyser"
		*
		* \see get_extension_guid
		*/
		virtual void get_name(pfc::string_base & out) const =0; 

		/**
		* \brief Set instance configuration data.
		*
		* \remarks 
		* - Only called before enabling/window creation.
		* - Must not be used by single instance extensions.
		* - You should also make sure you deal with the case of an empty stream
		* 
		* \throw Throws pfc::exception on failure
		*
		* \param [in]	p_reader		Pointer to configuration data stream
		* \param [in]	p_size			Size of data in stream
		* \param [in]	p_abort			Signals abort of operation
		*/
		virtual void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort){;};

		/**
		* \brief Get instance configuration data.
		* \remarks
		* Must not be used by single instance extensions.
		*
		* \note  Consider compatibility with future versions of you own component when deciding
		*		 upon a data format. You may wish to change what is written by this function 
		*		 in the future. If you prepare for this in advance, you won't have to take 
		*		 measures such as changing your extension GUID to avoid incompatibility.
		* 
		* \throw Throws pfc::exception on failure
		*
		* \param [out]	p_writer		Pointer to stream receiving configuration data
		* \param [in]	p_abort			Signals abort of operation
		*/
		virtual void get_config(stream_writer * p_writer, abort_callback & p_abort) const {};

		/**
		* \brief Set instance configuration data. This differs from set_config in that
		*		 the data will be of that returned by export_config
		*
		* \remarks 
		* - Only called before enabling/window creation.
		*
		* \note  The default implementation calls set_config for compatibility only. Be sure that
		*        you override if you need to.
		* 
		* \throw Throws pfc::exception on failure
		*
		* \param [in]	p_reader		Pointer to configuration data stream
		* \param [in]	p_size			Size of data in stream
		* \param [in]	p_abort			Signals abort of operation
		*/
		virtual void import_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort)
		{
			set_config(p_reader, p_size, p_abort);
		};

		/**
		* \brief Get instance configuration data. This differs from get_config, in that
		* 		 what is written is intended to be transferable between different foobar2000 
		*		 installations on different computers (i.e. self-contained).
		*
		* \note  The default implementation calls get_config for compatibility only. Be sure that
		*        you override if you need to.
		* 
		* \throw Throws pfc::exception on failure
		*
		* \param [out]	p_writer		Pointer to stream receiving configuration data
		* \param [in]	p_abort			Signals abort of operation
		*/
		virtual void export_config(stream_writer * p_writer, abort_callback & p_abort) const 
		{
			get_config(p_writer, p_abort);
		};

		/**
		* \brief Gets whether the extension has a modal configuration window
		*
		* The window is exposed through show_config_popup()
		*
		* \return true iff a configuration window is exposed through show_config_popup
		*/
		virtual bool have_config_popup() const {return false;}

		/**
		* \brief Displays a modal configuartion dialog
		*
		* \param [in]	wnd_parent	The window to use as the owner window for your configuration dialog
		*
		* \return false if the configuration did not change
		*/
		virtual bool show_config_popup(HWND wnd_parent) {return false;}

		/**
		* \brief Retrieve menu items to be displayed in the host menu
		*
		* \param [in]	p_hook		The interface you use to add your menu items
		*/
		virtual void get_menu_items (menu_hook_t & p_hook) {};

	};
};

#endif //_COLUMNS_SDK_BASE_H_