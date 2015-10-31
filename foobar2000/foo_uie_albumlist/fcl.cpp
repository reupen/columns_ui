#include "stdafx.h"

// {3E3F5B9F-5599-417e-A867-AC690EF01A8A}
static const GUID g_guid_fcl_group_album_list_views =
{ 0x3e3f5b9f, 0x5599, 0x417e,{ 0xa8, 0x67, 0xac, 0x69, 0xe, 0xf0, 0x1a, 0x8a } };

// {F45D4B85-FA35-4abd-A04B-345D06C39E38}
static const GUID g_guid_fcl_dataset_album_list_views =
{ 0xf45d4b85, 0xfa35, 0x4abd,{ 0xa0, 0x4b, 0x34, 0x5d, 0x6, 0xc3, 0x9e, 0x38 } };

// {E93DF451-6196-48fd-A67B-F9056705336A}
static const GUID g_guid_fcl_dataset_album_list_appearance =
{ 0xe93df451, 0x6196, 0x48fd,{ 0xa6, 0x7b, 0xf9, 0x5, 0x67, 0x5, 0x33, 0x6a } };

namespace {
	cui::fcl::group_impl_factory g_fclgroup(g_guid_fcl_group_album_list_views, "Album List Views", "Album List view title format scripts", cui::fcl::groups::title_scripts);
}

class album_list_fcl_views : public cui::fcl::dataset
{
	enum { stream_version = 0 };
	enum { view_name, view_script };
	virtual void get_name(pfc::string_base & p_out)const { p_out = "Album List Views"; }
	virtual const GUID & get_guid()const { return g_guid_fcl_dataset_album_list_views; }
	virtual const GUID & get_group()const { return g_guid_fcl_group_album_list_views; }
	virtual void get_data(stream_writer * p_writer, t_uint32 type, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort)const
	{
		fcl::writer w(p_writer, p_abort);
		w.write_raw((t_size)stream_version);
		t_size i, count = cfg_view_list.get_count();
		w.write_raw(count);
		for (i = 0; i<count; i++)
		{
			w.write_raw(t_size(2));
			w.write_item(view_name, cfg_view_list.get_name(i));
			w.write_item(view_script, cfg_view_list.get_value(i));
		}
	}
	virtual void set_data(stream_reader * p_reader, t_size size, t_uint32 type, cui::fcl::t_import_feedback & feedback, abort_callback & p_abort)
	{
		t_size version, count, i;
		fcl::reader r(p_reader, size, p_abort);
		r.read_item(version);
		if (version <= stream_version)
		{
			cfg_view_list.remove_all();
			r.read_item(count);
			for (i = 0; i<count; i++)
			{
				t_size elems, id, size;
				r.read_item(elems);
				pfc::string8 name, script;
				while (elems)
				{
					r.read_item(id);
					r.read_item(size);
					switch (id)
					{
					case view_name:
						r.read_item(name, size);
						break;
					case view_script:
						r.read_item(script, size);
						break;
					default:
						r.skip(size);
						break;
					}
					--elems;
				}
				cfg_view_list.add_item(name, script);
			}
		}
		if (g_config_general.is_active())
			g_config_general.refresh_views();
		album_list_window::g_refresh_all();
	}
};

class album_list_fcl_appearance : public cui::fcl::dataset
{
	enum { stream_version = 0 };
	enum {
		id_sub_item_counts, id_sub_item_indices, id_horizontal_scrollbar, id_root_node,
		id_use_item_padding, id_item_padding, id_use_indentation, id_indentation, id_edge_style
	};
	virtual void get_name(pfc::string_base & p_out)const { p_out = "Album List appearance settings"; }
	virtual const GUID & get_guid()const { return g_guid_fcl_dataset_album_list_appearance; }
	virtual const GUID & get_group()const { return cui::fcl::groups::colours_and_fonts; }
	virtual void get_data(stream_writer * p_writer, t_uint32 type, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort)const
	{
		fcl::writer w(p_writer, p_abort);
		w.write_raw(t_size(stream_version));
		w.write_item(id_sub_item_counts, cfg_show_numbers);
		w.write_item(id_sub_item_indices, cfg_show_numbers2);
		w.write_item(id_horizontal_scrollbar, cfg_hscroll);
		w.write_item(id_root_node, cfg_show_root);
		w.write_item(id_use_item_padding, cfg_custom_item_height);
		w.write_item(id_item_padding, cfg_item_height);
		w.write_item(id_use_indentation, cfg_use_custom_indent);
		w.write_item(id_indentation, cfg_indent);
		w.write_item(id_edge_style, cfg_frame);
	}
	virtual void set_data(stream_reader * p_reader, t_size size, t_uint32 type, cui::fcl::t_import_feedback & feedback, abort_callback & p_abort)
	{
		t_size version, count, i;
		fcl::reader r(p_reader, size, p_abort);
		r.read_item(version);
		if (version <= stream_version)
		{
			while (r.get_remaining())
			{
				t_size id, size;
				r.read_item(id);
				r.read_item(size);
				switch (id)
				{
				case id_sub_item_counts:
					r.read_item(cfg_show_numbers);
					break;
				case id_sub_item_indices:
					r.read_item(cfg_show_numbers2);
					break;
				case id_horizontal_scrollbar:
					r.read_item(cfg_hscroll);
					break;
				case id_root_node:
					r.read_item(cfg_show_root);
					break;
				case id_use_item_padding:
					r.read_item(cfg_custom_item_height);
					break;
				case id_item_padding:
					r.read_item(cfg_item_height);
					break;
				case id_use_indentation:
					r.read_item(cfg_use_custom_indent);
					break;
				case id_indentation:
					r.read_item(cfg_indent);
					break;
				case id_edge_style:
					r.read_item(cfg_frame);
					break;
				default:
					r.skip(size);
					break;
				}
			}
			album_list_window::update_all_item_heights();
			album_list_window::update_all_indents();
			album_list_window::update_all_window_frames();
		}
	}
};

cui::fcl::dataset_factory<album_list_fcl_views> g_album_list_fcl_views;
cui::fcl::dataset_factory<album_list_fcl_appearance> g_album_list_fcl_appearance;
