struct t_dsp_chain_entry {
	service_ptr_t<dsp> m_dsp;
	dsp_preset_impl m_preset;
	bool m_recycle_flag;
};
typedef pfc::chain_list_v2_t<t_dsp_chain_entry> t_dsp_chain;

class dsp_manager {
public:
	dsp_manager() : m_config_changed(false) {}

	void set_config( const dsp_chain_config & p_data );
	double run(dsp_chunk_list * p_list,const metadb_handle_ptr & p_cur_file,unsigned p_flags,abort_callback & p_abort);
	void flush();
	void close();

	bool is_active();

private:
	t_dsp_chain m_chain;
	dsp_chain_config_impl m_config;
	bool m_config_changed;
	
	void dsp_run(t_dsp_chain::const_iterator p_iter,dsp_chunk_list * list,const metadb_handle_ptr & cur_file,unsigned flags,double & latency,abort_callback&);

	dsp_manager(const dsp_manager &) {throw pfc::exception_not_implemented();}
	const dsp_manager & operator=(const dsp_manager&) {throw pfc::exception_not_implemented();}
};


class dsp_config_manager : public service_base
{
public:
	virtual void get_core_settings(dsp_chain_config & p_out) = 0;
	virtual void set_core_settings(const dsp_chain_config & p_data) = 0;
	
	virtual bool configure_popup(dsp_chain_config & p_data,HWND p_parent,const char * p_title) = 0;
	
	virtual HWND configure_embedded(const dsp_chain_config & p_initdata,HWND p_parent,unsigned p_id,bool p_from_modal) = 0;
	virtual void configure_embedded_retrieve(HWND wnd,dsp_chain_config & p_data) = 0;
	virtual void configure_embedded_change(HWND wnd,const dsp_chain_config & p_data) = 0;


	void core_enable_dsp(const dsp_preset & preset);
	void core_disable_dsp(const GUID & id);
	bool core_query_dsp(const GUID & id, dsp_preset & out);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(dsp_config_manager);
};

class NOVTABLE dsp_config_callback : public service_base
{
public:
	virtual void on_core_settings_change(const dsp_chain_config & p_newdata) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(dsp_config_callback);
};
