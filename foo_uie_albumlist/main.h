#pragma once

#define IDC_FILTER 1001
#define EDIT_TIMER_ID 2001

#define USE_TIMER

extern const char * directory_structure_view_name;

class IDropSource_albumlist : public IDropSource
{
	long refcount;
	HWND wnd;
public:
	IDropSource_albumlist() : refcount(0) {}
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

	virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);

	virtual HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);
	IDropSource_albumlist(HWND wnd_dbe) : wnd(wnd_dbe) {};

};

class album_list_window : public ui_extension::container_ui_extension, public library_callback_dynamic
{
	static const char * class_name;
	bool initialised;
	bool m_populated;
	WNDPROC treeproc;

	bool dragging, clicked;
	DWORD clickpoint;
	int indent_default;

	bool
		m_filter,
		m_timer;

protected:
	static ptr_list_t<album_list_window> list_wnd;
	HWND wnd_tv, wnd_edit;
	node_ptr p_selection;

	friend class node;

public:
	void on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
	void on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
	void on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);

	string8 view;

	bool is_bydir()
	{
		return !stricmp_utf8(view, directory_structure_view_name);
	}

	const char * get_hierarchy()
	{
		unsigned idx = cfg_view_list.find_item(view);
		if (idx != (unsigned)(-1)) return cfg_view_list.get_value(idx);
		return "N/A";
	}

	node_ptr m_root;

	LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	void create_or_destroy_filter();
	void create_filter();
	void destroy_filter();
	void create_tree();
	void destroy_tree();
	void on_task_completion(t_uint32 task, t_uint32 code);
	void on_size(unsigned cx, unsigned cy);
	void on_size();

	LRESULT WINAPI on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT WINAPI hook_proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

	album_list_window();

	void refresh_tree();
	void refresh_tree_internal();
	void refresh_tree_internal_add_tracks(metadb_handle_list & p_tracks);
	void refresh_tree_internal_remove_tracks(metadb_handle_list & p_tracks);
	void update_all_labels();
	void update_colours();
	void update_item_height();
	void on_view_script_change(const char * p_view_before, const char * p_view);
	static void update_all_colours();
	//static void update_all_heights();
	static void update_all_item_heights();
	static void update_all_indents();
	static void g_update_all_labels();
	static void g_update_all_showhscroll();
	static void g_update_all_fonts();
	static void g_refresh_all();
	static void g_on_view_script_change(const char * p_view_before, const char * p_view);

	~album_list_window();

	static const GUID extension_guid;

	virtual const GUID & get_extension_guid() const
	{
		return extension_guid;
	}


	virtual void get_name(string_base & out)const;
	virtual void get_category(string_base & out)const;

	virtual void set_config(stream_reader * p_reader, t_size size, abort_callback & p_abort);
	virtual void get_config(stream_writer * p_writer, abort_callback & p_abort)const;

	unsigned get_type() const { return ui_extension::type_panel; }

	virtual class_data & get_class_data()const
	{
		__implement_get_class_data(_T("{606E9CDD-45EE-4c3b-9FD5-49381CEBE8AE}"), false);
	}

	static void update_all_window_frames();

	class menu_node_settings : public ui_extension::menu_node_command_t
	{
	public:
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
		{
			p_out = "Settings";
			p_displayflags = 0;
			return true;
		}
		virtual bool get_description(pfc::string_base & p_out) const
		{
			return false;
		}
		virtual void execute()
		{
			static_api_ptr_t<ui_control>()->show_preferences(g_guid_preferences_album_list_panel);
		}
		menu_node_settings() {};
	};
	class menu_node_filter : public ui_extension::menu_node_command_t
	{
		service_ptr_t<album_list_window> p_this;
	public:
		virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
		{
			p_out = "Filter";
			p_displayflags = p_this->m_filter ? uie::menu_node_t::state_checked : 0;
			return true;
		}
		virtual bool get_description(pfc::string_base & p_out) const
		{
			return false;
		}
		virtual void execute()
		{
			p_this->m_filter = !p_this->m_filter;
			p_this->create_or_destroy_filter();
		}
		menu_node_filter(album_list_window * p_wnd) : p_this(p_wnd) {};
	};

	class menu_node_view : public ui_extension::menu_node_command_t
	{
		service_ptr_t<album_list_window> p_this;
		string_simple view;
	public:
		virtual bool get_display_data(string_base & p_out, unsigned & p_displayflags)const
		{
			p_out = view;
			p_displayflags = (!stricmp_utf8(view, p_this->view) ? ui_extension::menu_node_t::state_checked : 0);
			return true;
		}
		virtual bool get_description(string_base & p_out)const
		{
			return false;
		}
		virtual void execute()
		{
			p_this->view = view;
			p_this->refresh_tree();
		}
		menu_node_view(album_list_window * p_wnd, const char * p_value) : p_this(p_wnd), view(p_value) {};
	};

	class menu_node_select_view : public ui_extension::menu_node_popup_t
	{
		list_t<ui_extension::menu_node_ptr> m_items;
	public:
		virtual bool get_display_data(string_base & p_out, unsigned & p_displayflags)const
		{
			p_out = "View";
			p_displayflags = 0;
			return true;
		}
		virtual unsigned get_children_count()const { return m_items.get_count(); }
		virtual void get_child(unsigned p_index, uie::menu_node_ptr & p_out)const { p_out = m_items[p_index].get_ptr(); }
		menu_node_select_view(album_list_window * p_wnd)
		{
			unsigned n, m = cfg_view_list.get_count();
			string8_fastalloc temp;
			temp.prealloc(32);

			m_items.add_item(new menu_node_view(p_wnd, directory_structure_view_name));

			for (n = 0; n<m; n++)
			{
				m_items.add_item(new menu_node_view(p_wnd, cfg_view_list.get_name(n)));
			};
		};
	};

	virtual void get_menu_items(ui_extension::menu_hook_t & p_hook)
	{
		p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_select_view(this)));
		p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_filter(this)));
		p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_settings()));
	}

	friend class font_notify;
private:
	static HFONT g_font;
	search_filter::ptr m_filter_ptr;
	ui_selection_holder::ptr m_selection_holder;
};

void TreeView_CollapseOtherNodes(HWND wnd, HTREEITEM ti);
void do_playlist(node_ptr const & src, bool replace, bool b_new = false);
void do_autosend_playlist(node_ptr const & src, string_base & view, bool b_play = false);
void setup_tree(HWND list, HTREEITEM parent, node_ptr ptr, t_size level, t_size idx, t_size max_idx, metadb_handle_list_t<pfc::alloc_fast_aggressive> & entries, HTREEITEM ti_after = TVI_LAST /*, bool b_sort,const service_ptr_t<titleformat_object> & p_sort_script*/);
