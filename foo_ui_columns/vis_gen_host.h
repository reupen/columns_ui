#ifndef _COLUMNS_VIS_GEN_HOST_H_
#define _COLUMNS_VIS_GEN_HOST_H_

#if 1
class window_visualisation : public ui_extension::container_ui_extension
{
	static TCHAR * class_name;
	bool initialised;
	GUID m_guid;
	pfc::array_t<t_uint8> m_data;
	service_ptr_t<class window_visualisation_interface> m_interface;
	uie::visualisation_ptr p_vis;
	unsigned m_frame;
	HBITMAP bm_display;
	RECT rc_client;
	HWND m_wnd;

public:
	HBITMAP get_bitmap() const {return bm_display;}
	const RECT * get_rect_client() {return &rc_client;}
	static pfc::ptr_list_t<window_visualisation> list_vis;

	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	virtual class_data & get_class_data()const 
	{
		DWORD flags = WS_EX_CONTROLPARENT;
		if (m_frame == 1) flags |= WS_EX_CLIENTEDGE;
		if (m_frame == 2) flags |= WS_EX_STATICEDGE;
		
		__implement_get_class_data_ex(class_name, _T(""), false, 0, WS_CHILD|WS_CLIPCHILDREN, flags, 0);
	}

	window_visualisation();

	~window_visualisation();

	void flush_bitmap();
	void make_bitmap(HDC hdc=0);

	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;

	void set_frame_style(unsigned p_type);
	unsigned get_frame_style() const {return m_frame;}
	void get_vis_ptr(uie::visualisation_ptr & p_out){p_out = p_vis;}

	virtual unsigned get_type  () const{return ui_extension::type_toolbar|ui_extension::type_panel;};

	void set_vis_data(const void * p_data, unsigned p_size)
	{
		m_data.set_size(0);
		m_data.append_fromptr((t_uint8*)p_data, p_size);
	}
	void get_vis_data(pfc::array_t<t_uint8> & p_out) const
	{
		if (p_vis.is_valid())
		{
			p_out.set_size(0);
			abort_callback_dummy abort;
			p_vis->get_config_to_array(p_out, abort);
		}
	}
	friend class window_visualisation_interface;

	//override me

	// (leave default definitions in place for these three funcs)
	//virtual void get_menu_items (ui_extension::menu_hook_t & p_hook);
	//virtual void set_config ( stream_reader * config);
	//virtual void get_config( stream_writer * data);
	//virtual bool have_config_popup(){return true;}
	//virtual bool show_config_popup(HWND wnd_parent);

	virtual const GUID & get_visualisation_guid()const = 0;

};
#endif
#endif
