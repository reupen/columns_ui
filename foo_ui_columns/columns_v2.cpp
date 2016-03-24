#include "stdafx.h"

void column_t::read(stream_reader * reader, ColumnStreamVersion streamVersion, abort_callback & abortCallback)
{
	reader->read_string(name, abortCallback);
	reader->read_string(spec, abortCallback);
	reader->read_lendian_t(use_custom_colour, abortCallback);
	reader->read_string(colour_spec, abortCallback);
	reader->read_lendian_t(use_custom_sort, abortCallback);
	reader->read_string(sort_spec, abortCallback);
	reader->read_lendian_t(width.value, abortCallback);
	reader->read_lendian_t(align, abortCallback);
	reader->read_lendian_t(filter_type, abortCallback);
	reader->read_string(filter, abortCallback);
	reader->read_lendian_t(parts, abortCallback);
	reader->read_lendian_t(show, abortCallback);
	reader->read_string(edit_field, abortCallback);
	if (streamVersion >= ColumnStreamVersion::streamVersion1) {
		reader->read_lendian_t(width.dpi, abortCallback);
	} else {
		width.dpi = uih::GetSystemDpiCached().cx;
	}
}

void column_t::write(stream_writer * out, ColumnStreamVersion streamVersion, abort_callback & abortCallback) const
{
	out->write_string(name.get_ptr(), abortCallback);
	out->write_string(spec.get_ptr(), abortCallback);
	out->write_lendian_t(use_custom_colour, abortCallback);
	out->write_string(colour_spec, abortCallback);
	out->write_lendian_t(use_custom_sort, abortCallback);
	out->write_string(sort_spec, abortCallback);
	out->write_lendian_t(width.value, abortCallback);
	out->write_lendian_t(align, abortCallback);
	out->write_lendian_t(filter_type, abortCallback);
	out->write_string(filter, abortCallback);
	out->write_lendian_t(parts, abortCallback);
	out->write_lendian_t(show, abortCallback);
	out->write_string(edit_field, abortCallback);
	if (streamVersion >= ColumnStreamVersion::streamVersion1) {
		out->write_lendian_t(width.dpi, abortCallback);
	}
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
	if (m_StreamVersion >= ColumnStreamVersion::streamVersion1)
		out->write_lendian_t(m_StreamVersion, p_abort);

	out->write_lendian_t(num, p_abort);
	for(n=0;n<num;n++)
	{
		get_item(n)->write(out, m_StreamVersion, p_abort);
	}
}
void cfg_columns_t::set_data_raw(stream_reader * p_reader, unsigned p_sizehint, abort_callback & p_abort)
{
	remove_all();

	ColumnStreamVersion streamVersion = ColumnStreamVersion::streamVersion0;
	if (m_StreamVersion >= ColumnStreamVersion::streamVersion1)
		p_reader->read_lendian_t(streamVersion, p_abort);

	t_uint32 num;
	p_reader->read_lendian_t(num, p_abort);
	{
		for(;num;num--)
		{
			column_t::ptr item = new column_t;
			item->read(p_reader, streamVersion, p_abort);
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

cfg_columns_t::cfg_columns_t(const GUID & p_guid, ColumnStreamVersion streamVersion) : cfg_var(p_guid), m_StreamVersion(streamVersion)
{
	reset();
}
