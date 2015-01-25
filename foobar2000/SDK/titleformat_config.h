// DEPRECATED - do not use
class NOVTABLE titleformat_config : public service_base
{
public:
	virtual GUID get_guid() = 0;
	virtual const char * get_name() = 0;
	virtual void get_data(pfc::string_base & p_out) = 0;
	virtual void set_data(const char * p_string,unsigned p_string_length) = 0;
	virtual void reset() = 0;
	virtual bool compile(service_ptr_t<titleformat_object> & p_out) = 0;
	virtual double get_order_priority() = 0;



	static bool g_find(const GUID & p_guid,service_ptr_t<titleformat_config> & p_out);
	static bool g_get_data(const GUID & p_guid,pfc::string_base & p_out);
	static bool g_compile(const GUID & p_guid,service_ptr_t<titleformat_object> & p_out);


	static const GUID config_playlist,config_copy,config_statusbar,config_systray,config_windowtitle;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(titleformat_config);
};

class titleformat_config_callback : public service_base
{
public:
	virtual void on_change(const GUID & p_guid,const char * p_name,const char * p_value,t_size p_value_length) = 0;

	static void g_on_change(const GUID & p_guid,const char * p_name,const char * p_value,t_size p_value_length);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(titleformat_config_callback);
};

class titleformat_config_impl : public titleformat_config, private cfg_var
{
public:
	GUID get_guid();
	const char * get_name();
	void get_data(pfc::string_base & p_out);
	void set_data(const char * p_string,unsigned p_string_length);
	void reset();
	bool compile(service_ptr_t<titleformat_object> & p_out);
	double get_order_priority();
	titleformat_config_impl(const GUID &,const char * p_name,const char * p_initvalue,double p_order);
private:

	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort);
	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort);

	pfc::string8 m_name,m_value,m_value_default;
	service_ptr_t<titleformat_object> m_instance;
	bool m_compilation_failed;
	GUID m_guid;
	double m_order;
	critical_section m_sync;
};

typedef service_factory_single_transparent_t<titleformat_config_impl> titleformat_config_factory;
