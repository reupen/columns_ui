#include "stdafx.h"

#include "colours_manager_data.h"

void ColoursManagerData::g_on_common_bool_changed(t_size mask)
{
    t_size count = m_callbacks.get_count();
    for (t_size i = 0; i < count; i++)
        m_callbacks[i]->on_bool_changed(mask);
}

void ColoursManagerData::g_on_common_colour_changed(t_size mask)
{
    t_size count = m_callbacks.get_count();
    for (t_size i = 0; i < count; i++)
        m_callbacks[i]->on_colour_changed(mask);
}

void ColoursManagerData::deregister_common_callback(cui::colours::common_callback* p_callback)
{
    m_callbacks.remove_item(p_callback);
}

void ColoursManagerData::register_common_callback(cui::colours::common_callback* p_callback)
{
    m_callbacks.add_item(p_callback);
}

ColoursManagerData::ColoursManagerData() : cfg_var(g_cfg_guid)
{
    m_global_entry = new Entry(true);
}

void ColoursManagerData::find_by_guid(const GUID& p_guid, entry_ptr_t& p_out)
{
    if (p_guid == pfc::guid_null) {
        p_out = m_global_entry;
        return;
    }
    t_size count = m_entries.get_count();
    for (t_size i = 0; i < count; i++) {
        if (m_entries[i]->guid == p_guid) {
            p_out = m_entries[i];
            return;
        }
    }
    p_out = new Entry;
    p_out->guid = p_guid;
    m_entries.add_item(p_out);
}

void ColoursManagerData::set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort)
{
    t_uint32 version;
    p_stream->read_lendian_t(version, p_abort);
    if (version <= cfg_version) {
        m_global_entry->read(version, p_stream, p_abort);
        t_size count;
        p_stream->read_lendian_t(count, p_abort);
        m_entries.remove_all();
        for (t_size i = 0; i < count; i++) {
            entry_ptr_t ptr = new Entry;
            ptr->read(version, p_stream, p_abort);
            m_entries.add_item(ptr);
        }
    }
}

void ColoursManagerData::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
{
    pfc::list_t<GUID> clients;
    {
        service_enum_t<cui::colours::client> clientenum;
        cui::colours::client::ptr ptr;
        while (clientenum.next(ptr))
            clients.add_item(ptr->get_client_guid());
    }

    pfc::array_t<bool> mask;
    t_size i;
    t_size count = m_entries.get_count();
    t_size counter = 0;
    mask.set_count(count);
    for (i = 0; i < count; i++)
        if (mask[i] = clients.have_item(m_entries[i]->guid))
            counter++;

    p_stream->write_lendian_t((t_uint32)cfg_version, p_abort);
    m_global_entry->write(p_stream, p_abort);
    p_stream->write_lendian_t(counter, p_abort);
    for (i = 0; i < count; i++)
        if (mask[i])
            m_entries[i]->write(p_stream, p_abort);
}

ColoursManagerData::Entry::Entry(bool b_global /*= false*/)
    : colour_mode(b_global ? cui::colours::colour_mode_themed : cui::colours::colour_mode_global)
{
    reset_colors();
}

void ColoursManagerData::Entry::reset_colors()
{
    text = cui::colours::g_get_system_color(cui::colours::colour_text);
    selection_text = cui::colours::g_get_system_color(cui::colours::colour_selection_text);
    inactive_selection_text = cui::colours::g_get_system_color(cui::colours::colour_inactive_selection_text);
    background = cui::colours::g_get_system_color(cui::colours::colour_background);
    selection_background = cui::colours::g_get_system_color(cui::colours::colour_selection_background);
    inactive_selection_background
        = cui::colours::g_get_system_color(cui::colours::colour_inactive_selection_background);
    active_item_frame = cui::colours::g_get_system_color(cui::colours::colour_active_item_frame);
    group_foreground = NULL;
    group_background = NULL;
    use_custom_active_item_frame = false;
}

void ColoursManagerData::Entry::read(t_uint32 version, stream_reader* p_stream, abort_callback& p_abort)
{
    p_stream->read_lendian_t(guid, p_abort);
    p_stream->read_lendian_t((t_uint32&)colour_mode, p_abort);
    p_stream->read_lendian_t(text, p_abort);
    p_stream->read_lendian_t(selection_text, p_abort);
    p_stream->read_lendian_t(inactive_selection_text, p_abort);
    p_stream->read_lendian_t(background, p_abort);
    p_stream->read_lendian_t(selection_background, p_abort);
    p_stream->read_lendian_t(inactive_selection_background, p_abort);
    p_stream->read_lendian_t(active_item_frame, p_abort);
    p_stream->read_lendian_t(use_custom_active_item_frame, p_abort);
}

void ColoursManagerData::Entry::import(
    stream_reader* p_reader, t_size stream_size, t_uint32 type, abort_callback& p_abort)
{
    fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
    t_uint32 element_id;
    t_uint32 element_size;

    while (reader.get_remaining()) {
        reader.read_item(element_id);
        reader.read_item(element_size);

        switch (element_id) {
        case identifier_guid:
            reader.read_item(guid);
            break;
        case identifier_mode:
            reader.read_item((t_uint32&)colour_mode);
            break;
        case identifier_text:
            reader.read_item(text);
            break;
        case identifier_selection_text:
            reader.read_item(selection_text);
            break;
        case identifier_inactive_selection_text:
            reader.read_item(inactive_selection_text);
            break;
        case identifier_background:
            reader.read_item(background);
            break;
        case identifier_selection_background:
            reader.read_item(selection_background);
            break;
        case identifier_inactive_selection_background:
            reader.read_item(inactive_selection_background);
            break;
        case identifier_custom_active_item_frame:
            reader.read_item(active_item_frame);
            break;
        case identifier_use_custom_active_item_frame:
            reader.read_item(use_custom_active_item_frame);
            break;
        default:
            reader.skip(element_size);
            break;
        }
    }
}

void ColoursManagerData::Entry::_export(stream_writer* p_stream, abort_callback& p_abort)
{
    fbh::fcl::Writer out(p_stream, p_abort);
    out.write_item(identifier_guid, guid);
    out.write_item(identifier_mode, (t_uint32)colour_mode);
    if (colour_mode == cui::colours::colour_mode_custom) {
        out.write_item(identifier_text, text);
        out.write_item(identifier_selection_text, selection_text);
        out.write_item(identifier_inactive_selection_text, inactive_selection_text);
        out.write_item(identifier_background, background);
        out.write_item(identifier_selection_background, selection_background);
        out.write_item(identifier_inactive_selection_background, inactive_selection_background);
    }
    out.write_item(identifier_use_custom_active_item_frame, use_custom_active_item_frame);
    if (use_custom_active_item_frame)
        out.write_item(identifier_custom_active_item_frame, active_item_frame);
}

void ColoursManagerData::Entry::write(stream_writer* p_stream, abort_callback& p_abort)
{
    p_stream->write_lendian_t(guid, p_abort);
    p_stream->write_lendian_t((t_uint32)colour_mode, p_abort);
    p_stream->write_lendian_t(text, p_abort);
    p_stream->write_lendian_t(selection_text, p_abort);
    p_stream->write_lendian_t(inactive_selection_text, p_abort);
    p_stream->write_lendian_t(background, p_abort);
    p_stream->write_lendian_t(selection_background, p_abort);
    p_stream->write_lendian_t(inactive_selection_background, p_abort);
    p_stream->write_lendian_t(active_item_frame, p_abort);
    p_stream->write_lendian_t(use_custom_active_item_frame, p_abort);
}
