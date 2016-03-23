#include "stdafx.h"

void column_t::read(stream_reader * p_reader, abort_callback & p_abort)
{
	p_reader->read_string(name, p_abort);
	p_reader->read_string(spec, p_abort);
	p_reader->read_lendian_t(use_custom_colour, p_abort);
	p_reader->read_string(colour_spec, p_abort);
	p_reader->read_lendian_t(use_custom_sort, p_abort);
	p_reader->read_string(sort_spec, p_abort);
	p_reader->read_lendian_t(width.value, p_abort);
	p_reader->read_lendian_t(align, p_abort);
	p_reader->read_lendian_t(filter_type, p_abort);
	p_reader->read_string(filter, p_abort);
	p_reader->read_lendian_t(parts, p_abort);
	p_reader->read_lendian_t(show, p_abort);
	p_reader->read_string(edit_field, p_abort);
}

void column_t::write(stream_writer * out, abort_callback & p_abort)
{
	out->write_string(name.get_ptr(), p_abort);
	out->write_string(spec.get_ptr(), p_abort);
	out->write_lendian_t(use_custom_colour, p_abort);
	out->write_string(colour_spec, p_abort);
	out->write_lendian_t(use_custom_sort, p_abort);
	out->write_string(sort_spec, p_abort);
	out->write_lendian_t(width.value, p_abort);
	out->write_lendian_t(align, p_abort);
	out->write_lendian_t(filter_type, p_abort);
	out->write_string(filter, p_abort);
	out->write_lendian_t(parts, p_abort);
	out->write_lendian_t(show, p_abort);
	out->write_string(edit_field, p_abort);
}

bool column_list_t::move_up(t_size idx)
{
	unsigned count = get_count();
	if (idx > 0 && idx< count)
	{
		order_helper order(count);
		order.swap(idx, idx-1);
		reorder(order.get_ptr());
		return true;
	}
	return false;
}

bool column_list_t::move(t_size from, t_size to)
{
	unsigned count = get_count();
	unsigned n = from;
	unsigned idx = to;
	bool rv = false;
				
	order_helper order(count);

	if (n < idx)
	{
		while (n<idx && n < count)
		{
			order.swap(n,n+1);
			n++;
		}
	}
	else if (n > idx)
	{
		while (n>idx && n > 0)
		{
			order.swap(n,n-1);
			n--;
		}
	}
	if (n != from) 
	{
		reorder(order.get_ptr());
		rv = true;
	}
	return rv;
}

bool column_list_t::move_down(t_size idx)
{
	unsigned count = get_count();
	if (idx >= 0 && idx < (count-1))
	{
		order_helper order(count);
		order.swap(idx, idx+1);
		reorder(order.get_ptr());
		return true;
	}
	return false;
}
void column_t::get_to_display(titleformat_object::ptr & p_out)
{
	if (!to_display.is_valid()) 
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_display, spec);
	p_out = to_display;
}
void column_t::get_to_sort(titleformat_object::ptr & p_out)
{
	if (use_custom_sort)
	{
		if (!to_sort.is_valid()) 
			static_api_ptr_t<titleformat_compiler>()->compile_safe(to_sort, sort_spec);
		p_out = to_sort;
	}
	else
	{
		get_to_display(p_out);
	}
}
void column_t::get_to_colour(titleformat_object::ptr & p_out)
{
	if (use_custom_colour)
	{
		if (!to_colour.is_valid()) 
			static_api_ptr_t<titleformat_compiler>()->compile_safe(to_colour, colour_spec);
		p_out = to_colour;
	}
	else
	{
		if (!g_to_global_colour.is_valid()) 
			static_api_ptr_t<titleformat_compiler>()->compile_safe(g_to_global_colour, cfg_colour);
		p_out = g_to_global_colour;
	}
}

void cfg_columns_t::get_data_raw(stream_writer * out, abort_callback & p_abort)
{
	//if (!cfg_nohscroll) playlist_view::g_save_columns(); FIXME

	unsigned n;
	unsigned num = get_count();
	out->write_lendian_t(num, p_abort);
	for(n=0;n<num;n++)
	{
		get_item(n)->write(out, p_abort);
	}
}
void cfg_columns_t::set_data_raw(stream_reader * p_reader, unsigned p_sizehint, abort_callback & p_abort)
{
	remove_all();


	t_uint32 num;
	p_reader->read_lendian_t(num, p_abort);
	{
		for(;num;num--)
		{
			column_t::ptr item = new column_t;
			item->read(p_reader, p_abort);
			add_item(item);
		}
	}
}
void cfg_columns_t::reset() 
{
	remove_all();
	add_item(new column_t("Artist","[%artist%]",false,"",false,"",180, ALIGN_LEFT, FILTER_NONE, "", 180, true, "ARTIST"));
	add_item(new column_t("#","[%tracknumber%]",false,"",false,"",18, ALIGN_RIGHT, FILTER_NONE, "", 18, true, "TRACKNUMBER"));
	add_item(new column_t("Title","[%title%]",false,"",false,"",300, ALIGN_LEFT, FILTER_NONE, "", 300, true, "TITLE"));
	add_item(new column_t("Album","[%album%]",false,"",false,"",200, ALIGN_LEFT, FILTER_NONE, "", 200, true, "ALBUM"));
	add_item(new column_t("Date","[%date%]",false,"",false,"",60, ALIGN_LEFT, FILTER_NONE, "", 60, true, "DATE"));
	add_item(new column_t("Length","[%_time_elapsed% / ]%_length%",false,"",true,"$num(%_length_seconds%,6)",60, ALIGN_RIGHT, FILTER_NONE, "", 60, true, ""));
}

cfg_columns_t::cfg_columns_t(const GUID & p_guid) : cfg_var(p_guid) 
{
	reset();
}
