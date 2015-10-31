#pragma once

typedef pfc::refcounted_object_ptr_t<class node> node_ptr;

class node : public pfc::refcounted_object_root
{
private:
	string_simple value;
	//node * parent;
	list_t<node_ptr> children;
	metadb_handle_list entries;
	bool m_sorted, b_bydir;
	class album_list_window * p_dbe;

	static int sortproc(const pfc::string_simple_t<WCHAR> & n1, const pfc::string_simple_t<WCHAR> & n2)
	{
		return StrCmpLogicalW(n1, n2);
	}
public:
	HTREEITEM m_ti;
	bool m_label_dirty;

	void sort_children();

	void sort_entries();//for contextmenu

	const metadb_handle_list & get_entries() const { return entries; }

	void create_new_playlist();

	void send_to_playlist(bool replace);

	void remove_from_db() const;

	void remove_dead() const
	{
		static_api_ptr_t<library_manager> api;
		api->check_dead_entries(entries);
	}

	inline const char * get_sort_data() const { return value; }

	node(const char * p_value, unsigned p_value_len, class album_list_window * dbe);
	inline void set_bydir(bool p) { b_bydir = p; }

	~node()
	{
		entries.remove_all();
		children.remove_all();
	}

	const char * get_val()
	{
		return value.is_empty() ? "All music" : (const char*)value;
	}

	void add_entry(const metadb_handle_ptr & p_entry)
	{
		entries.add_item(p_entry);
	}

	void remove_entries(pfc::bit_array & mask);

	void set_data(const list_base_const_t<metadb_handle_ptr> & p_data, bool b_keep_existing);

	static int g_compare_name(const node_ptr & p1, const wchar_t * str)
	{
		return StrCmpLogicalW(uT(p1->value), str);
	}

	node_ptr find_or_add_child(const char * p_value, unsigned p_value_len, bool b_find, bool & b_new);



	node_ptr add_child_v2(const char * p_value, unsigned p_value_len);
	node_ptr add_child_v2(const char * p_value)
	{
		return add_child_v2(p_value, strlen(p_value));
	}

	void reset() { children.remove_all(); entries.remove_all(); m_sorted = false; }

	void mark_all_labels_dirty();

	void purge_empty_children(HWND wnd);

	const list_t<node_ptr> & get_children() const { return children; }
	list_t<node_ptr> & get_children() { return children; }

	unsigned get_num_children() const
	{
		return children.get_count();
	}
	unsigned get_num_entries() const
	{
		return entries.get_count();
	}

};
