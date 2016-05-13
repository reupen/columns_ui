#include "stdafx.h"
#include "splitter.h"


splitter_window_impl::panel::panel() : m_hidden(false), m_guid(pfc::guid_null), m_locked(false), m_wnd(nullptr),
m_wnd_child(nullptr), m_show_caption(true), m_caption_orientation(NULL),
m_autohide(false), m_container(this), m_size(150), m_show_toggle_area(false),
m_use_custom_title(false)
{

}

void splitter_window_impl::panel::destroy()
{
	if (m_child.is_valid())
	{
		//			pal.m_child_data.set_size(0);
		//			stream_writer_memblock_ref blah(pal.m_child_data);
		//			pal.m_child->get_config(&blah);
		m_child->destroy_window();
		m_wnd_child = nullptr;
		DestroyWindow(m_wnd);
		m_wnd = nullptr;
		m_child.release();
	}
	if (m_container.get_wnd())
		m_container.destroy();
}

void splitter_window_impl::panel::on_size(unsigned cx, unsigned cy)
{
	unsigned caption_size = m_show_caption ? g_get_caption_size() : 0;

	//get_orientation()
	unsigned x = m_caption_orientation == vertical ? caption_size : 0;
	unsigned y = m_caption_orientation == vertical ? 0 : caption_size;

	if (m_show_toggle_area && !m_autohide) x++;

	if (m_wnd_child)
		SetWindowPos(m_wnd_child, nullptr, x, y, cx - x, cy - y, SWP_NOZORDER);
	if (caption_size /*&& (m_caption_orientation == vertical || (m_container.m_uxtheme.is_valid() && m_container.m_theme))*/)
	{
		int caption_cx = min(m_caption_orientation == vertical ? caption_size : (cx), MAXLONG);
		int caption_cy = min(m_caption_orientation == vertical ? cy : caption_size, MAXLONG);

		RECT rc_caption = { 0, 0, caption_cx, caption_cy };
		RedrawWindow(m_wnd, &rc_caption, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void splitter_window_impl::panel::on_size()
{
	RECT rc;
	if (GetClientRect(m_wnd, &rc))
	{
		on_size(rc.right, rc.bottom);
	}
}

void splitter_window_impl::panel::set_hidden(bool val)
{
	m_hidden = val;
	if (m_container.m_this.is_valid())
	{
		m_container.m_this->get_host()->on_size_limit_change(m_container.m_this->get_wnd(), uie::size_limit_all);
		m_container.m_this->on_size_changed();
	}
}

void splitter_window_impl::panel::read(stream_reader*t, abort_callback & p_abort)
{
	t->read_lendian_t(m_guid, p_abort);
	t->read_lendian_t(m_caption_orientation, p_abort);
	t->read_lendian_t(m_locked, p_abort);
	t->read_lendian_t(m_hidden, p_abort);
	t->read_lendian_t(m_show_caption, p_abort);
	t->read_lendian_t(m_autohide, p_abort);
	if (m_autohide) m_hidden = true;
	uint32_t size;
	t->read_lendian_t(size, p_abort);
	m_size = size;
	//console::formatter() << "read panel, size: " << m_size;
	t->read_lendian_t(m_show_toggle_area, p_abort);
	unsigned data_size;
	t->read_lendian_t(data_size, p_abort);
	m_child_data.set_size(data_size);
	t->read(m_child_data.get_ptr(), data_size, p_abort);
	t->read_lendian_t(m_use_custom_title, p_abort);
	t->read_string(m_custom_title, p_abort);
}

void splitter_window_impl::panel::import(stream_reader*t, abort_callback & p_abort)
{
	t->read_lendian_t(m_guid, p_abort);
	t->read_lendian_t(m_caption_orientation, p_abort);
	t->read_lendian_t(m_locked, p_abort);
	t->read_lendian_t(m_hidden, p_abort);
	t->read_lendian_t(m_show_caption, p_abort);
	t->read_lendian_t(m_autohide, p_abort);
	if (m_autohide) m_hidden = true;
	uint32_t size;
	t->read_lendian_t(size, p_abort);
	m_size = size;
	//console::formatter() << "read panel, size: " << m_size;
	t->read_lendian_t(m_show_toggle_area, p_abort);
	unsigned data_size;
	t->read_lendian_t(data_size, p_abort);
	pfc::array_t<t_uint8> data;
	data.set_size(data_size);
	t->read(data.get_ptr(), data_size, p_abort);
	t->read_lendian_t(m_use_custom_title, p_abort);
	t->read_string(m_custom_title, p_abort);

	if (uie::window::create_by_guid(m_guid, m_child))
	{
		try {
			m_child->import_config_from_ptr(data.get_ptr(), data.get_size(), p_abort);
		}
		catch (const exception_io &) {};
		m_child->get_config_to_array(m_child_data, p_abort, true);
	}
	//else
	//	throw pfc::exception_not_implemented();
}

void splitter_window_impl::panel::read_extra(stream_reader* reader, abort_callback & p_abort)
{
	reader->read_lendian_t(m_size.value, p_abort);
	reader->read_lendian_t(m_size.dpi, p_abort);
}

void splitter_window_impl::panel::write_extra(stream_writer* writer, abort_callback & p_abort)
{
	writer->write_lendian_t(m_size.value, p_abort);
	writer->write_lendian_t(m_size.dpi, p_abort);
}

void splitter_window_impl::panel::write(stream_writer * out, abort_callback & p_abort)
{
	if (m_child.is_valid())
	{
		m_child->get_config_to_array(m_child_data, p_abort, true);
	}
	out->write_lendian_t(m_guid, p_abort);
	out->write_lendian_t(m_caption_orientation, p_abort);
	out->write_lendian_t(m_locked, p_abort);
	out->write_lendian_t(m_hidden, p_abort);
	out->write_lendian_t(m_show_caption, p_abort);
	out->write_lendian_t(m_autohide, p_abort);
	out->write_lendian_t(m_size.getScaledValue(), p_abort);
	out->write_lendian_t(m_show_toggle_area, p_abort);
	out->write_lendian_t(m_child_data.get_size(), p_abort);
	out->write(m_child_data.get_ptr(), m_child_data.get_size(), p_abort);
	out->write_lendian_t(m_use_custom_title, p_abort);
	out->write_string(m_custom_title, p_abort);
}

void splitter_window_impl::panel::_export(stream_writer * out, abort_callback & p_abort)
{
	stream_writer_memblock child_exported_data;
	uie::window_ptr ptr = m_child;
	if (!ptr.is_valid())
	{
		if (!uie::window::create_by_guid(m_guid, ptr))
			throw cui::fcl::exception_missing_panel();
		try {
			ptr->set_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size(), p_abort);
		}
		catch (const exception_io &) {};
	}
			{
				ptr->export_config(&child_exported_data, p_abort);
			}
			out->write_lendian_t(m_guid, p_abort);
			out->write_lendian_t(m_caption_orientation, p_abort);
			out->write_lendian_t(m_locked, p_abort);
			out->write_lendian_t(m_hidden, p_abort);
			out->write_lendian_t(m_show_caption, p_abort);
			out->write_lendian_t(m_autohide, p_abort);
			out->write_lendian_t(m_size.getScaledValue(), p_abort);
			out->write_lendian_t(m_show_toggle_area, p_abort);
			out->write_lendian_t(child_exported_data.m_data.get_size(), p_abort);
			out->write(child_exported_data.m_data.get_ptr(), child_exported_data.m_data.get_size(), p_abort);
			out->write_lendian_t(m_use_custom_title, p_abort);
			out->write_string(m_custom_title, p_abort);
}

void splitter_window_impl::panel::set_from_splitter_item(const uie::splitter_item_t * p_source)
{
	if (m_wnd) destroy();
	const uie::splitter_item_full_t * ptr = nullptr;
	if (p_source->query(ptr)) {
		m_autohide = ptr->m_autohide;
		m_caption_orientation = ptr->m_caption_orientation;
		m_locked = ptr->m_locked;
		m_hidden = ptr->m_hidden;
		m_show_caption = ptr->m_show_caption;
		m_size = ptr->m_size;
		m_show_toggle_area = ptr->m_show_toggle_area;
		m_use_custom_title = ptr->m_custom_title;
		ptr->get_title(m_custom_title);
	}
	const uie::splitter_item_full_v2_t * splitter_item_v2 = nullptr;
	if (p_source->query(splitter_item_v2)) {
		m_size.value = splitter_item_v2->m_size_v2;
		m_size.dpi = splitter_item_v2->m_size_v2_dpi;
	}
	m_guid = p_source->get_panel_guid();
	p_source->get_panel_config_to_array(m_child_data, true);
}

uie::splitter_item_full_v2_t * splitter_window_impl::panel::create_splitter_item(bool b_set_ptr /*= true*/)
{
	auto  ret = new uie::splitter_item_full_v2_impl_t;
	ret->m_autohide = m_autohide;
	ret->m_caption_orientation = m_caption_orientation;
	ret->m_locked = m_locked;
	ret->m_hidden = m_hidden;
	ret->m_show_caption = m_show_caption;
	ret->m_size = m_size;
	ret->set_panel_guid(m_guid);
	ret->set_panel_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size());
	ret->m_show_toggle_area = m_show_toggle_area;
	ret->m_custom_title = m_use_custom_title;
	ret->set_title(m_custom_title, m_custom_title.length());
	ret->m_size_v2 = m_size.value;
	ret->m_size_v2_dpi = m_size.dpi;

	if (b_set_ptr)
		ret->set_window_ptr(m_child);
	return ret;
}

bool splitter_window_impl::panel_list::find_by_wnd_child(HWND wnd, unsigned & p_out)
{
	unsigned n, count = get_count();
	for (n = 0; n < count; n++)
	{
		if (get_item(n)->m_wnd_child == wnd)
		{
			p_out = n;
			return true;
		}
	}
	return false;
}

bool splitter_window_impl::panel_list::find_by_wnd(HWND wnd, unsigned & p_out)
{
	unsigned n, count = get_count();
	for (n = 0; n < count; n++)
	{
		if (get_item(n)->m_wnd == wnd)
		{
			p_out = n;
			return true;
		}
	}
	return false;
}
