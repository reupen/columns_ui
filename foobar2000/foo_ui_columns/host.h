#if 0

#ifndef _COLUMNS_HOST_H_
#define _COLUMNS_HOST_H_

#include "foo_ui_columns.h"

enum orientation_type {HORZ=0, VERT=1,};

enum {MSG_AUTOHIDE_END = WM_USER+3};
#define HOST_AUTOHIDE_TIMER_ID  672

enum 
{
	TYPE_EXTENSION,
	TYPE_EXTENSION_LIST,
};

class object_ptr : public pfc::refcounted_object_ptr_t<class object>
{
public:
	class extension * query_extension() const;
	class extension_list * query_extension_list() const;
	inline object_ptr(object * p_ptr = 0) : pfc::refcounted_object_ptr_t<object>(p_ptr) {};
};

struct size_limits
{
	unsigned min_height;
	unsigned max_height;
	unsigned min_width;
	unsigned max_width;
};

class object : public pfc::refcounted_object_root
{
public:
	unsigned orientation;
	unsigned height;
	object_ptr p_obj_parent;
	bool locked;
	bool hidden;

	RECT rc_area;

	virtual unsigned get_type() const = 0;

	virtual void copy (object & out)=0;
	virtual void write(stream_writer * out, abort_callback & p_abort)=0;
	virtual void read(stream_reader*t, abort_callback & p_abort)=0;
	virtual void destroy()=0;
	virtual void show_window()=0;

	virtual void set_area(int x, int y, int width, int height);

	virtual HDWP on_size(HDWP dwp) = 0;
	void on_size();
	virtual unsigned get_child_count()=0;

	virtual bool refresh(HWND wnd)=0;
	//virtual void setup_tree(HWND wnd_tree, HTREEITEM ti_parent)=0;

	virtual bool on_size_limit_change(HWND wnd, unsigned flags)=0;
	virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height, bool & rv)=0;
	virtual bool is_visible(HWND wnd, bool & rv)=0;
	virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility, bool & rv)=0;
	virtual bool set_window_visibility(HWND wnd, bool visibility, bool & rv)=0;
	virtual bool relinquish_ownership(HWND wnd)=0;

	virtual bool focus_playlist_window()=0;
	virtual bool have_extension(const GUID & p_guid)=0;

	virtual void get_size_limits( size_limits & p_out) = 0;

	virtual bool find_by_divider_pt(POINT & pt, object_ptr & p_out, object_ptr & p_obj_next=object_ptr())
	{
		return false;
	}

	void on_root_size()
	{
		object_ptr p_par = p_obj_parent;
		if (p_par.is_valid())
		{
			while (p_par->p_obj_parent.is_valid())
				p_par = p_par->p_obj_parent;
			p_par->on_size();
		}
	}

	object(object * p_obj = 0, unsigned h = 100, unsigned or = VERT) : height(h), p_obj_parent(p_obj),  locked(false),hidden(false),orientation(or)
	{
		rc_area.left=0;
		rc_area.top=0;
		rc_area.right=0;
		rc_area.bottom=0;
	};
};

extern GUID null_guid;

class extension_ptr : public  pfc::refcounted_object_ptr_t<class extension>
{
public:
	inline extension_ptr(const object_ptr & p_source) : pfc::refcounted_object_ptr_t<extension>(p_source.query_extension()){};
	inline const extension_ptr & operator=(object_ptr & p_ptr) {copy(p_ptr.query_extension()); return *this;}
};

class extension : public object
{
public:
	GUID guid;
	HWND wnd_panel;
	HWND wnd_host;
	ui_extension::window_ptr p_ext;
	pfc::array_t<t_uint8> config;
	unsigned min_height, max_height, min_width, max_width;
	bool show_caption;
//	bool b_need_show;

	extension(object *p_obj = 0, GUID id = null_guid, unsigned h = 100) : guid(id), wnd_host(0), wnd_panel(0), p_ext(0), /*height(h), */min_height(0), max_height(0), show_caption(true),
		min_width(0), max_width(0),object(p_obj, h)/*,b_need_show(false)*/{};

	virtual void write(stream_writer * out, abort_callback & p_abort);
	virtual void read(stream_reader*t, abort_callback & p_abort);
	void copy(object & out);

	void on_wm_size(int width, int height);

	inline void g_on_wm_size()
	{
		RECT rc;
		GetClientRect(wnd_host, &rc);
		on_wm_size(rc.right, rc.bottom);
	}

	virtual void get_size_limits( size_limits & p_out) ;

	using object::on_size;

	virtual unsigned get_type() const
	{
		return TYPE_EXTENSION;
	}
	virtual const GUID & get_guid() const
	{
		return guid;
	}
	void destroy();

	virtual bool refresh(HWND wnd);

	virtual unsigned get_child_count();

	virtual HDWP on_size(HDWP dwp);

	//virtual void setup_tree(HWND wnd_tree, HTREEITEM ti_parent);

	LRESULT WINAPI on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

	virtual bool on_size_limit_change(HWND wnd, unsigned flags);

	virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height, bool & rv);

	virtual bool is_visible(HWND wnd, bool & rv);

	virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility, bool & rv);

	virtual bool set_window_visibility(HWND wnd, bool visibility, bool & rv);

	virtual bool relinquish_ownership(HWND wnd);

	virtual bool focus_playlist_window();

	virtual bool have_extension(const GUID & p_guid);

	virtual void show_window();
};

class extension_list_ptr : public pfc::refcounted_object_ptr_t<class extension_list>
{
public:
	inline extension_list_ptr(const object_ptr & p_source = object_ptr()) : pfc::refcounted_object_ptr_t<extension_list>(p_source.query_extension_list()) {};
	inline const extension_list_ptr & operator=(object_ptr & p_ptr) {copy(p_ptr.query_extension_list()); return *this;}
};

template <class T>
class refcounted_ptr_list_t : public pfc::list_t<pfc::refcounted_object_ptr_t<T> >
{
public:
	inline static void g_swap(refcounted_ptr_list_t<T> & item1, refcounted_ptr_list_t<T> & item2)
	{
		pfc::swap_t(
			*(pfc::list_t<pfc::refcounted_object_ptr_t<T> >*)&item1,
			*(pfc::list_t<pfc::refcounted_object_ptr_t<T> >*)&item2
			);
	}
/*	unsigned find_item(object * item) const//returns index of first occurance, -1 if not found
	{
		unsigned n,max = get_count();
		for(n=0;n<max;n++)
			if (get_item(n)==item) return n;
		return infinite;
	}*/
};

class extension_list : public object//, public pfc::ptr_list_t<object_ptr>
{
public:
	pfc::list_t<object_ptr> m_objects;
	bool autohide;

	void set_list(extension_list & in);
	bool move_up(unsigned idx);
	bool move_down(unsigned idx);
	extension * find_by_wnd(HWND wnd);

	using object::on_size;
		
	virtual bool refresh(HWND wnd);
	void copy(object & out);
	virtual void get_size_limits( size_limits & p_out) ;

	virtual unsigned get_type() const
	{
		return TYPE_EXTENSION_LIST;
	}
	virtual const GUID & get_guid() const
	{
		return null_guid;
	}
	virtual unsigned get_child_count()
	{
		unsigned n, count = m_objects.get_count(), rv=0;
		for (n=0; n<count; n++) rv += m_objects.get_item(n)->get_child_count();
		return rv;
	}
	virtual void destroy()
	{
		unsigned n, count = m_objects.get_count();
		for (n=0; n<count; n++) m_objects.get_item(n)->destroy();
	}
//	void delete_all()
//	{
//		unsigned num = get_count();
//		for (;num;num--) delete_by_idx(0);
//	}
//	void delete_by_idx(unsigned n)
//	{
//		delete_by_idx(n);
//	}
//	void delete_item(object * p_obj)
//	{
//		unsigned idx = find_item(p_obj);
//		if (idx != infinite)
//			delete_by_idx(idx);
//}
	extension_list(object * p_obj = 0, unsigned or = VERT, unsigned h =100, bool ah = false) : object(p_obj, h, or),autohide(ah) {};
	virtual HDWP on_size(HDWP dwp)
	{
		return on_size(dwp, false);
	}

//	unsigned find_item(object * item) const//returns index of first occurance, -1 if not found
//	{
//		unsigned n,max = get_count();
//		for(n=0;n<max;n++)
//			if (*get_item(n)==item) return n;
//		return infinite;
//	}
	HDWP on_size(HDWP dwp, bool set = false);

	void write(stream_writer * out, abort_callback & p_abort);
	void read(stream_reader*t, abort_callback & p_abort);

	//virtual void setup_tree(HWND wnd_tree, HTREEITEM ti_parent);

	virtual bool on_size_limit_change(HWND wnd, unsigned flags);

	virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height, bool & rv);

	virtual bool is_visible(HWND wnd, bool & rv);

	virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility, bool & rv);

	virtual bool set_window_visibility(HWND wnd, bool visibility, bool & rv);

	virtual bool relinquish_ownership(HWND wnd);

	virtual bool focus_playlist_window();

	virtual bool have_extension(const GUID & p_guid);

	bool find_by_divider_pt(POINT & pt, object_ptr & p_out, object_ptr & p_obj_next=object_ptr());

	virtual void show_window();

struct min_max_info
{
	unsigned min_height;
	unsigned max_height;
	unsigned height;
};

	int override_size(unsigned idx, int delta);

	~extension_list()
	{
//		delete_all();
	};

};



class cfg_host_t : public cfg_var
{
private:
public:
	pfc::array_t<t_uint8> val;
	
	virtual void get_data_raw(stream_writer * out, abort_callback & p_abort);

	virtual void set_data_raw(stream_reader *, unsigned p_sizehint, abort_callback & p_abort);
	void reset();

	void save();

	explicit inline cfg_host_t(const GUID & p_guid) : cfg_var(p_guid) {reset();}
};


class host_window : public ui_helpers::container_window
{
	static bool class_registered;
	static bool panel_host_class_registered;

	enum {IDM_CLOSE=1, IDM_MOVE_UP, IDM_MOVE_DOWN, IDM_LOCK, IDM_CAPTION, IDM_BASE};
	unsigned menu_ext_base;

	bool is_dragging;
	object_ptr p_obj_dragging;
	unsigned last_position;
	
public:
	static const TCHAR * panel_host_class_name;
	object_ptr p_obj_base;
	HWND wnd_host;

	virtual class_data & get_class_data()const 
	{
		__implement_get_class_data_ex(_T("foo_ui_columns_ui_extension_host_window"), _T(""), false, 0, WS_CHILD | WS_CLIPSIBLINGS| WS_CLIPCHILDREN | WS_TABSTOP , WS_EX_CONTROLPARENT, CS_DBLCLKS);
	}
	
	static LRESULT WINAPI panel_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static void unregister_class();
	static void on_size(HWND wnd,int width,int height);
	
	inline void on_size()
	{
		if (p_obj_base.is_valid())
		{
			RECT rc;
			GetClientRect(wnd_host, &rc);
			HDWP dwp = BeginDeferWindowPos(p_obj_base->get_child_count());
			p_obj_base->set_area(0, 0, rc.right, rc.bottom);
			dwp = p_obj_base->on_size(dwp);
			EndDeferWindowPos(dwp);
		}
	}

	host_window();

	LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	
	void get_config(stream_writer*out, abort_callback & p_abort)
	{
		if (p_obj_base.is_valid())
			p_obj_base->write(out, p_abort);
	}
	void set_config(stream_reader*t, abort_callback & p_abort);

	~host_window();

	bool focus_playlist_window();
	
	void refresh(bool b_size = true);

	virtual void on_size_limit_change(HWND wnd, unsigned flags);

	virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height);

	virtual bool is_visible(HWND wnd)const;

	virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility)const;

	virtual bool set_window_visibility(HWND wnd, bool visibility);

	virtual void relinquish_ownership(HWND wnd);

	void show_window();

};

extern host_window * g_host_window;

extern cfg_host_t cfg_host;

BOOL uDrawPanelTitle(HDC dc, const RECT * rc_clip, const char * text, int len, bool vert, bool world);

#endif
#endif