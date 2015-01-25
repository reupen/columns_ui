#ifndef _FOOBAR2000_MASSTAG_ACTION_H_
#define _FOOBAR2000_MASSTAG_ACTION_H_

//DEPRECATED - masstagger component no longer included with the installer - do not reimplement.
class NOVTABLE masstagger_action : public service_base
{
public:
	/**
	 * Get name to display on list of available actions.
	 * \param p_out store name in here
	 */
	virtual void get_name(pfc::string_base & p_out)=0;

	/**
	 * Initialize and show configuration with default values, if appropriate.
	 * \param p_parent handle to parent window
	 * \returns true iff succesful
	 */
	virtual bool initialize(HWND p_parent) = 0;

	/**
	 * Initialize and get configuration from parameter.
	 * \param p_data a zero-terminated string containing previously stored configuration data.
	 * \returns true iff succesful
	 */
	virtual bool initialize_fromconfig(const char * p_data)=0;

	/**
	 * Show configuration with current values.
	 * \param parent handle to parent window
	 * \returns true if display string needs to be updated.
	 */
	virtual bool configure(HWND p_parent) = 0;

	/**
	 * Get name to display on list of configured actions.
	 * You should include value of settings, if possible.
	 * \param p_name store name in here
	 */
	virtual void get_display_string(pfc::string_base & p_name) = 0;

	/**
	 * Get current settings as zero-terminated string.
	 * \param p_data store settings in here
	 */
	virtual void get_config(pfc::string_base & p_data) = 0;

	/**
	 * Apply action on file info.
	 * \param p_location location of current item
	 * \param p_info file info of current item, contains modifications from previous actions
	 * \param p_index zero-based index of current item in list of processed items
	 * \param p_count number of processed items
	 */
	virtual void run(const playable_location & p_location, file_info * p_info, t_size p_index, t_size p_count) = 0;

	/**
	 * Get GUID.
	 * Used for identification when storing scripts.
	 * \returns your GUID
	 */
	virtual const GUID & get_guid()=0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(masstagger_action);
};

DECLARE_CLASS_GUID(masstagger_action,0x49da1f6c, 0x6744, 0x47f3, 0xbf, 0x93, 0x67, 0x1c, 0x37, 0x79, 0xff, 0xcd);

template<typename T>
class masstagger_action_factory_t : public service_factory_t<T> {};

#endif //_FOOBAR2000_MASSTAG_ACTION_H_
