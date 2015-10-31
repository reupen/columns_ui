#pragma once

struct create_guid : public GUID
{
	create_guid(t_uint32 p_data1, t_uint16 p_data2, t_uint16 p_data3, t_uint8 p_data41, t_uint8 p_data42, t_uint8 p_data43, t_uint8 p_data44, t_uint8 p_data45, t_uint8 p_data46, t_uint8 p_data47, t_uint8 p_data48)
	{
		Data1 = p_data1;
		Data2 = p_data2;
		Data3 = p_data3;
		Data4[0] = p_data41;
		Data4[1] = p_data42;
		Data4[2] = p_data43;
		Data4[3] = p_data44;
		Data4[4] = p_data45;
		Data4[5] = p_data46;
		Data4[6] = p_data47;
		Data4[7] = p_data48;
	}
};

class cfg_view_list_t : public cfg_var
{
private:
	struct entry
	{
		string8 name;
		string8 value;
		entry(const char * p_name, const char * p_value) : name(p_name), value(p_value) {}
		entry() {}
	};
	list_t<entry> data;
public:

	~cfg_view_list_t() {}
	void get_data_raw(stream_writer * out, abort_callback & p_abort);
	virtual void set_data_raw(stream_reader * r, unsigned psize, abort_callback & p_abort);

	virtual void reset();

	const char * get_name(unsigned idx) const
	{
		if (idx >= 0 && idx<data.get_count()) return data[idx].name;
		else return 0;
	}

	const char * get_value(unsigned idx) const
	{
		if (idx >= 0 && idx<data.get_count()) return data[idx].value;
		else return 0;
	}

	unsigned find_item(const char * name) const
	{
		unsigned n, m = data.get_count();
		for (n = 0; n<m; n++) if (!stricmp_utf8(data[n].name, name)) return n;
		return (unsigned)(-1);
	}

	unsigned add_item(const char * name, const char * value)
	{
		return data.add_item(entry(name, value));
	}

	void remove_item(unsigned idx)
	{
		data.remove_by_idx(idx);
	}

	void remove_all()
	{
		data.remove_all();
	}

	void modify_item(unsigned idx, const char * name, const char * value)
	{
		entry & p_entry = data[idx];
		p_entry.name = name;
		p_entry.value = value;
	}

	unsigned get_count() const { return data.get_count(); }

	void swap(unsigned idx1, unsigned idx2) { data.swap_items(idx1, idx2); }

	cfg_view_list_t(const GUID & p_guid) : cfg_var(p_guid)
	{
		reset();
	}

	void format_display(unsigned n, string_base & out) const
	{
		out = get_name(n);
		out += " : ";
		out += get_value(n);
	}
};


extern const GUID g_guid_album_list_font,
	g_guid_album_list_colours;

extern cfg_view_list_t cfg_view_list;
extern create_guid g_guid_preferences_album_list_panel;
extern cfg_bool cfg_themed;
extern cfg_int cfg_populate,
	cfg_autosend,
	cfg_picmixer,
	cfg_add_items_use_core_sort,
	cfg_add_items_select,
	cfg_show_numbers,
	cfg_show_numbers2,
	cfg_dblclk,
	cfg_keyb,
	cfg_middle,
	cfg_frame,
	cfg_hscroll,
	cfg_show_root,
	cfg_autoplay,
	cfg_use_custom_indent,
	cfg_indent,
	cfg_item_height,
	cfg_custom_item_height;
extern cfg_string cfg_playlist_name;

