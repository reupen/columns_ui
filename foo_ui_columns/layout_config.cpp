#include "stdafx.h"
#include "layout.h"

cfg_layout_t::cfg_layout_t(const GUID & p_guid) : cfg_var(p_guid), m_active(0)//, m_initialised(false)
{
};

void cfg_layout_t::preset::write(stream_writer * out, abort_callback & p_abort)
{
    out->write_lendian_t(m_guid, p_abort);
    out->write_string(m_name.get_ptr(), p_abort);
    out->write_lendian_t(m_val.get_size(), p_abort);
    out->write(m_val.get_ptr(), m_val.get_size(), p_abort);

}
void cfg_layout_t::preset::read(stream_reader * stream, abort_callback & p_abort)
{
    stream->read_lendian_t(m_guid, p_abort);
    pfc::string8 temp;
    stream->read_string(temp, p_abort);
    m_name = temp;
    unsigned size;
    stream->read_lendian_t(size, p_abort);
    m_val.set_size(size);
    stream->read(m_val.get_ptr(), m_val.get_size(), p_abort);
}

void cfg_layout_t::preset::get(uie::splitter_item_ptr & p_out)
{
    p_out = new uie::splitter_item_simple_t;
    p_out->set_panel_guid(m_guid);
    p_out->set_panel_config_from_ptr(m_val.get_ptr(), m_val.get_size());
}

void cfg_layout_t::preset::set(const uie::splitter_item_t * item)
{
    m_guid = item->get_panel_guid();
    item->get_panel_config_to_array(m_val, true);
}

void cfg_layout_t::get_preset(t_size index, uie::splitter_item_ptr & p_out)
{
    if (index == m_active && g_layout_window.get_wnd())
    {
        g_layout_window.get_child(p_out);
    }
    else if (index < m_presets.get_count())
    {
        m_presets[index].get(p_out);
    }
}

void cfg_layout_t::set_preset(t_size index, const uie::splitter_item_t * item)
{
    if (index == m_active && g_layout_window.get_wnd())
    {
        g_layout_window.set_child(item);
    }
    else if (index < m_presets.get_count()) //else??
    {
        m_presets[index].set(item);
    }
}

t_size cfg_layout_t::add_preset(const preset & item)
{
    return m_presets.add_item(item);
}
t_size cfg_layout_t::add_preset(const char * p_name, t_size len)
{
    preset temp;
    temp.m_name.set_string(p_name, len);
    temp.m_guid = columns_ui::panels::guid_playlist_view_v2;
    return m_presets.add_item(temp);
}
void cfg_layout_t::save_active_preset()
{
    if (m_active < m_presets.get_count() && g_layout_window.get_wnd())
    {
        uie::splitter_item_ptr ptr;
        g_layout_window.get_child(ptr);
        m_presets[m_active].set(ptr.get_ptr());
    }
}


void cfg_layout_t::set_active_preset(t_size index)
{
    if (index < m_presets.get_count() && m_active != index)
    {
        m_active = index;
        uie::splitter_item_ptr item;
        m_presets[index].get(item);
        if (g_layout_window.get_wnd())
            g_layout_window.set_child(item.get_ptr());
    }
}

t_size cfg_layout_t::delete_preset(t_size index)
{
    if (index < m_presets.get_count())
    {
        if (index == m_active)
            m_active = pfc_infinite;
        else if (index < m_active)
            m_active--;
        m_presets.remove_by_idx(index);
    }
    return m_presets.get_count();
}

void cfg_layout_t::set_presets(const pfc::list_base_const_t<preset> & presets, t_size active)
{
    if (presets.get_count())
    {
        m_presets.remove_all();
        m_active = pfc_infinite;
        m_presets.add_items(presets);
        set_active_preset(active);
    }
}

void layout_window::g_get_default_presets(pfc::list_t<cfg_layout_t::preset> & p_out)
{
    abort_callback_dummy abortCallbackDummy;
    {
        uie::window_ptr wnd, wnd2, wnd3;
        service_ptr_t<uie::splitter_window> splitter, splitter2, splitter3;
        if (uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd) && uie::window::create_by_guid(columns_ui::panels::guid_vertical_splitter, wnd2))
        {
            if (wnd->service_query_t(splitter) && wnd2->service_query_t(splitter2))
            {
                uie::splitter_item_simple_t item, item2;
                item.set_panel_guid(columns_ui::panels::guid_playlist_switcher);
                item2.set_panel_guid(columns_ui::panels::guid_vertical_splitter);
                splitter2->add_panel(&item);
                stream_writer_memblock conf1;
                splitter2->get_config(&conf1, abortCallbackDummy);
                item2.set_panel_config_from_ptr(conf1.m_data.get_ptr(), conf1.m_data.get_size());

                t_size index = splitter->add_panel(&item2);
                splitter->set_config_item_t(index, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                splitter->set_config_item_t(index, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                splitter->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);

                item.set_panel_guid(columns_ui::panels::guid_playlist_view_v2);
                t_size index_playlist = splitter->add_panel(&item);
                splitter->set_config_item_t(index_playlist, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                cfg_layout_t::preset preset_default;
                preset_default.m_name = "NG playlist + Playlist switcher";
                preset_default.m_guid = columns_ui::panels::guid_horizontal_splitter;
                stream_writer_memblock_ref conf(preset_default.m_val, true);
                splitter->get_config(&conf, abortCallbackDummy);

                p_out.add_item(preset_default);
            }
        }
    }
    {
        uie::window_ptr wnd, wnd2, wnd3, wnd_filter_splitter;
        service_ptr_t<uie::splitter_window> splitter, splitter2, splitter3, splitter_filter;
        if (uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd) && uie::window::create_by_guid(columns_ui::panels::guid_vertical_splitter, wnd2) && uie::window::create_by_guid(columns_ui::panels::guid_vertical_splitter, wnd3)
            && uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd_filter_splitter))
        {
            if (wnd->service_query_t(splitter) && wnd2->service_query_t(splitter2) && wnd3->service_query_t(splitter3)
                && wnd_filter_splitter->service_query_t(splitter_filter))
            {
                uie::splitter_item_simple_t item, item2, item3, item_filter, item_filter_splitter, item_artwork;
                item.set_panel_guid(columns_ui::panels::guid_playlist_switcher);
                item2.set_panel_guid(columns_ui::panels::guid_vertical_splitter);
                item3.set_panel_guid(columns_ui::panels::guid_vertical_splitter);
                item_filter_splitter.set_panel_guid(columns_ui::panels::guid_horizontal_splitter);
                item_filter.set_panel_guid(columns_ui::panels::guid_filter);
                item_artwork.set_panel_guid(columns_ui::panels::guid_artwork_view);;
                splitter2->add_panel(&item);
                stream_writer_memblock conf1, conf2, conf3;
                splitter2->get_config(&conf1, abortCallbackDummy);
                item2.set_panel_config_from_ptr(conf1.m_data.get_ptr(), conf1.m_data.get_size());

                t_size index = splitter->add_panel(&item2);
                splitter->set_config_item_t(index, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                splitter->set_config_item_t(index, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                
                splitter->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);

                t_size index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                {
                    splitter_filter->get_config(&conf3, abortCallbackDummy);
                    item_filter_splitter.set_panel_config_from_ptr(conf3.m_data.get_ptr(), conf3.m_data.get_size());
                    t_size indexfs = splitter3->add_panel(&item_filter_splitter);
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                    bool temp = true;
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                    
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);
                }

                item.set_panel_guid(columns_ui::panels::guid_playlist_view_v2);
                t_size index_playlist = splitter3->add_panel(&item);
                splitter3->set_config_item_t(index_playlist, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                splitter3->get_config(&conf2, abortCallbackDummy);
                item3.set_panel_config_from_ptr(conf2.m_data.get_ptr(), conf2.m_data.get_size());

                t_size index_splitter2 = splitter->add_panel(&item3);
                splitter->set_config_item_t(index_splitter2, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                cfg_layout_t::preset preset_default;
                preset_default.m_name = "NG playlist + Playlist switcher + Filters";
                preset_default.m_guid = columns_ui::panels::guid_horizontal_splitter;
                stream_writer_memblock_ref conf(preset_default.m_val, true);
                splitter->get_config(&conf, abortCallbackDummy);

                p_out.add_item(preset_default);

                index = splitter2->add_panel(&item_artwork);
                splitter2->set_config_item_t(index, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                splitter2->set_config_item_t(index, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                splitter2->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);
                conf1.m_data.set_size(0);
                splitter2->get_config(&conf1, abortCallbackDummy);
                item2.set_panel_config_from_ptr(conf1.m_data.get_ptr(), conf1.m_data.get_size());
                splitter->replace_panel(0, &item2);
                splitter->set_config_item_t(0, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                splitter->set_config_item_t(0, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                splitter->set_config_item_t(0, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);

                preset_default.m_name = "NG playlist + Playlist switcher + Artwork + Filters";
                preset_default.m_val.set_size(0);
                splitter->get_config(&conf, abortCallbackDummy);
                t_size index_preset = p_out.add_item(preset_default);

                splitter3->remove_panel(0);
                conf2.m_data.set_size(0);
                splitter3->get_config(&conf2, abortCallbackDummy);
                item3.set_panel_config_from_ptr(conf2.m_data.get_ptr(), conf2.m_data.get_size());

                splitter->replace_panel(index_splitter2, &item3);
                splitter->set_config_item_t(index_splitter2, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                preset_default.m_name = "NG playlist + Playlist switcher + Artwork";
                splitter->get_config_to_array(preset_default.m_val, abortCallbackDummy, true);
                p_out.insert_item(preset_default, index_preset);
            }
        }
    }
    {
        uie::window_ptr wnd, wnd2, wnd3, wnd_filter_splitter, wnd_bottom_splitter;
        service_ptr_t<uie::splitter_window> splitter, splitter2, splitter3, splitter_filter, splitter_bottom;
        if (uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd) && uie::window::create_by_guid(columns_ui::panels::guid_vertical_splitter, wnd2) && uie::window::create_by_guid(columns_ui::panels::guid_vertical_splitter, wnd3)
            && uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd_filter_splitter)
            && uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd_bottom_splitter))
        {
            if (wnd->service_query_t(splitter) && wnd2->service_query_t(splitter2) && wnd3->service_query_t(splitter3)
                && wnd_filter_splitter->service_query_t(splitter_filter)
                && wnd_bottom_splitter->service_query_t(splitter_bottom))
            {
                uie::splitter_item_simple_t item, item2, item3, item_filter, item_filter_splitter, item_artwork, item_bottom_splitter, item_item_details;
                item.set_panel_guid(columns_ui::panels::guid_playlist_switcher);
                item2.set_panel_guid(columns_ui::panels::guid_vertical_splitter);
                item3.set_panel_guid(columns_ui::panels::guid_vertical_splitter);
                item_filter_splitter.set_panel_guid(columns_ui::panels::guid_horizontal_splitter);
                item_bottom_splitter.set_panel_guid(columns_ui::panels::guid_horizontal_splitter);
                item_filter.set_panel_guid(columns_ui::panels::guid_filter);
                item_artwork.set_panel_guid(columns_ui::panels::guid_artwork_view);
                item_item_details.set_panel_guid(columns_ui::panels::guid_item_details);
                splitter2->add_panel(&item);
                stream_writer_memblock conf1, conf2, conf3, conf4;
                splitter2->get_config(&conf1, abortCallbackDummy);
                item2.set_panel_config_from_ptr(conf1.m_data.get_ptr(), conf1.m_data.get_size());

                t_size index = splitter->add_panel(&item2);
                splitter->set_config_item_t(index, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                splitter->set_config_item_t(index, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                splitter->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);

                t_size index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                {
                    splitter_filter->get_config(&conf3, abortCallbackDummy);
                    item_filter_splitter.set_panel_config_from_ptr(conf3.m_data.get_ptr(), conf3.m_data.get_size());
                }

                item.set_panel_guid(columns_ui::panels::guid_playlist_view_v2);
                t_size index_playlist = splitter3->add_panel(&item);
                splitter3->set_config_item_t(index_playlist, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                {
                    t_size index = splitter_bottom->add_panel(&item_item_details);
                    splitter_bottom->set_config_item_t(index, uie::splitter_window::bool_show_caption, bool(false), abortCallbackDummy);
                    index = splitter_bottom->add_panel(&item_artwork);
                    splitter_bottom->set_config_item_t(index, uie::splitter_window::bool_show_caption, bool(false), abortCallbackDummy);
                    splitter_bottom->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(125), abortCallbackDummy);
                    splitter_bottom->set_config_item_t(index, uie::splitter_window::bool_locked, bool(true), abortCallbackDummy);

                    splitter_bottom->get_config(&conf4, abortCallbackDummy);
                    item_bottom_splitter.set_panel_config_from_ptr(conf4.m_data.get_ptr(), conf4.m_data.get_size());
                    index = splitter3->add_panel(&item_bottom_splitter);
                    splitter3->set_config_item_t(index, uie::splitter_window::bool_show_caption, bool(false), abortCallbackDummy);
                    splitter3->set_config_item_t(index, uie::splitter_window::bool_locked, bool(true), abortCallbackDummy);
                    splitter3->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(125), abortCallbackDummy);
                }

                splitter3->get_config(&conf2, abortCallbackDummy);
                item3.set_panel_config_from_ptr(conf2.m_data.get_ptr(), conf2.m_data.get_size());

                t_size index_splitter2 = splitter->add_panel(&item3);
                splitter->set_config_item_t(index_splitter2, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                cfg_layout_t::preset preset_default;
                preset_default.m_name = "NG playlist + Playlist switcher + Item details + Artwork";
                preset_default.m_guid = columns_ui::panels::guid_horizontal_splitter;
                stream_writer_memblock_ref conf(preset_default.m_val, true);
                splitter->get_config(&conf, abortCallbackDummy);

                p_out.add_item(preset_default);

                {
                    t_size indexfs = 0;
                    splitter3->insert_panel(0, &item_filter_splitter);
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);
                }


                conf2.m_data.set_size(0);
                splitter3->get_config(&conf2, abortCallbackDummy);
                item3.set_panel_config_from_ptr(conf2.m_data.get_ptr(), conf2.m_data.get_size());

                //t_size index_splitter2 = 
                splitter->replace_panel(1, &item3);
                splitter->set_config_item_t(1, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                cfg_layout_t::preset preset_default2;
                preset_default2.m_name = "NG playlist + Playlist switcher + Filters + Item details + Artwork";
                preset_default2.m_guid = columns_ui::panels::guid_horizontal_splitter;
                stream_writer_memblock_ref conft(preset_default2.m_val, true);
                splitter->get_config(&conft, abortCallbackDummy);

                p_out.add_item(preset_default2);
            }
        }
    }
    {
        uie::window_ptr wnd;
        service_ptr_t<uie::splitter_window> splitter;
        if (uie::window::create_by_guid(columns_ui::panels::guid_playlist_tabs, wnd))
        {
            if (wnd->service_query_t(splitter))
            {
                uie::splitter_item_simple_t item;
                item.set_panel_guid(columns_ui::panels::guid_playlist_view_v2);
                splitter->add_panel(&item);

                cfg_layout_t::preset preset_default;
                preset_default.m_name = "NG playlist + Playlist tabs";
                preset_default.m_guid = columns_ui::panels::guid_playlist_tabs;
                stream_writer_memblock_ref conf(preset_default.m_val, true);
                splitter->get_config(&conf, abortCallbackDummy);

                //m_active = pfc_infinite;

                p_out.add_item(preset_default);
            }
        }
    }
    {
        uie::window_ptr wnd, wnd2, wnd3, wnd_filter_splitter, wnd_bottom_splitter;
        service_ptr_t<uie::splitter_window> splitter, splitter_tabs, splitter3, splitter_filter, splitter_bottom;
        if (uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd) && uie::window::create_by_guid(columns_ui::panels::guid_playlist_tabs, wnd2) && uie::window::create_by_guid(columns_ui::panels::guid_vertical_splitter, wnd3)
            && uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd_filter_splitter)
            && uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd_bottom_splitter))
        {
            if (wnd->service_query_t(splitter) && wnd2->service_query_t(splitter_tabs) && wnd3->service_query_t(splitter3)
                && wnd_filter_splitter->service_query_t(splitter_filter)
                && wnd_bottom_splitter->service_query_t(splitter_bottom))
            {
                uie::splitter_item_simple_t item_tabs, item, item3, item_filter, item_filter_splitter, item_artwork, item_bottom_splitter, item_item_details;
                item_tabs.set_panel_guid(columns_ui::panels::guid_playlist_tabs);

                item3.set_panel_guid(columns_ui::panels::guid_vertical_splitter);
                item_filter_splitter.set_panel_guid(columns_ui::panels::guid_horizontal_splitter);
                item_bottom_splitter.set_panel_guid(columns_ui::panels::guid_horizontal_splitter);
                item_filter.set_panel_guid(columns_ui::panels::guid_filter);
                item_artwork.set_panel_guid(columns_ui::panels::guid_artwork_view);
                item_item_details.set_panel_guid(columns_ui::panels::guid_item_details);
                bool val = true;

                stream_writer_memblock conf1, conf2, conf3, conf4;

                val = false;
                t_uint32 sz = 175;

                t_size index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, val, abortCallbackDummy);
                index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, val, abortCallbackDummy);
                index_filter = splitter_filter->add_panel(&item_filter);
                splitter_filter->set_config_item_t(index_filter, uie::splitter_window::bool_show_caption, val, abortCallbackDummy);
                {
                    splitter_filter->get_config(&conf3, abortCallbackDummy);
                    item_filter_splitter.set_panel_config_from_ptr(conf3.m_data.get_ptr(), conf3.m_data.get_size());
                    t_size indexfs = splitter3->add_panel(&item_filter_splitter);
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::bool_show_caption, val, abortCallbackDummy);
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                    splitter3->set_config_item_t(indexfs, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);
                }

                item.set_panel_guid(columns_ui::panels::guid_playlist_view_v2);
                t_size index_playlist = splitter_tabs->add_panel(&item);
                splitter_tabs->get_config(&conf1, abortCallbackDummy);
                item_tabs.set_panel_config_from_ptr(conf1.m_data.get_ptr(), conf1.m_data.get_size());
                t_size index_tabs = splitter3->add_panel(&item_tabs);
                splitter3->set_config_item_t(index_tabs, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);


                splitter3->get_config(&conf2, abortCallbackDummy);
                item3.set_panel_config_from_ptr(conf2.m_data.get_ptr(), conf2.m_data.get_size());

                t_size index_splitter2 = splitter->add_panel(&item3);
                splitter->set_config_item_t(index_splitter2, uie::splitter_window::bool_show_caption, val, abortCallbackDummy);

                cfg_layout_t::preset preset_default;
                preset_default.m_name = "NG playlist + Playlist tabs + Filters";
                preset_default.m_guid = columns_ui::panels::guid_horizontal_splitter;
                stream_writer_memblock_ref conf(preset_default.m_val, true);
                splitter->get_config(&conf, abortCallbackDummy);

                p_out.add_item(preset_default);

                {
                    t_size index = splitter_bottom->add_panel(&item_item_details);
                    splitter_bottom->set_config_item_t(index, uie::splitter_window::bool_show_caption, bool(false), abortCallbackDummy);
                    index = splitter_bottom->add_panel(&item_artwork);
                    splitter_bottom->set_config_item_t(index, uie::splitter_window::bool_show_caption, bool(false), abortCallbackDummy);
                    splitter_bottom->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(125), abortCallbackDummy);
                    splitter_bottom->set_config_item_t(index, uie::splitter_window::bool_locked, bool(true), abortCallbackDummy);

                    splitter_bottom->get_config(&conf4, abortCallbackDummy);
                    item_bottom_splitter.set_panel_config_from_ptr(conf4.m_data.get_ptr(), conf4.m_data.get_size());
                    index = splitter3->add_panel(&item_bottom_splitter);
                    splitter3->set_config_item_t(index, uie::splitter_window::bool_show_caption, bool(false), abortCallbackDummy);
                    splitter3->set_config_item_t(index, uie::splitter_window::bool_locked, bool(true), abortCallbackDummy);
                    splitter3->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(125), abortCallbackDummy);
                }

                conf2.m_data.set_size(0);
                splitter3->get_config(&conf2, abortCallbackDummy);
                item3.set_panel_config_from_ptr(conf2.m_data.get_ptr(), conf2.m_data.get_size());

                splitter->replace_panel(index_splitter2, &item3);
                splitter->set_config_item_t(index_splitter2, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                preset_default.m_name = "NG playlist + Playlist tabs + Filters + Item details + Artwork";
                preset_default.m_guid = columns_ui::panels::guid_horizontal_splitter;

                splitter->get_config_to_array(preset_default.m_val, abortCallbackDummy, true);


                t_size preset_tabs_all = p_out.add_item(preset_default);


                splitter3->remove_panel(0);
                conf2.m_data.set_size(0);
                splitter3->get_config(&conf2, abortCallbackDummy);
                item3.set_panel_config_from_ptr(conf2.m_data.get_ptr(), conf2.m_data.get_size());

                splitter->replace_panel(index_splitter2, &item3);
                splitter->set_config_item_t(index_splitter2, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                preset_default.m_name = "NG playlist + Playlist tabs + Item details + Artwork";
                preset_default.m_guid = columns_ui::panels::guid_horizontal_splitter;

                splitter->get_config_to_array(preset_default.m_val, abortCallbackDummy, true);


                p_out.insert_item(preset_default, preset_tabs_all);
            }
        }
    }
}

void cfg_layout_t::reset_presets()
{
    if (core_api::are_services_available())
    {
        abort_callback_dummy abortCallbackDummy;
        m_presets.remove_all();
        uie::window_ptr wnd, wnd2;
        service_ptr_t<uie::splitter_window> splitter, splitter2;
        if (uie::window::create_by_guid(columns_ui::panels::guid_horizontal_splitter, wnd) && uie::window::create_by_guid(columns_ui::panels::guid_vertical_splitter, wnd2))
        {
            if (wnd->service_query_t(splitter) && wnd2->service_query_t(splitter2))
            {
                uie::splitter_item_simple_t item, item2;
                item.set_panel_guid(columns_ui::panels::guid_playlist_switcher);
                item2.set_panel_guid(columns_ui::panels::guid_vertical_splitter);
                splitter2->add_panel(&item);
                stream_writer_memblock conf1;
                splitter2->get_config(&conf1, abortCallbackDummy);
                item2.set_panel_config_from_ptr(conf1.m_data.get_ptr(), conf1.m_data.get_size());

                t_size index = splitter->add_panel(&item2);
                splitter->set_config_item_t(index, uie::splitter_window::bool_locked, true, abortCallbackDummy);
                splitter->set_config_item_t(index, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);
                splitter->set_config_item_t(index, uie::splitter_window::size_and_dpi, uie::size_and_dpi(175), abortCallbackDummy);

                item.set_panel_guid(columns_ui::panels::guid_playlist_view_v2);
                t_size index_playlist = splitter->add_panel(&item);
                splitter->set_config_item_t(index_playlist, uie::splitter_window::bool_show_caption, false, abortCallbackDummy);

                preset preset_default;
                preset_default.m_name = "Default";
                preset_default.m_guid = columns_ui::panels::guid_horizontal_splitter;
                stream_writer_memblock_ref conf(preset_default.m_val, true);
                splitter->get_config(&conf, abortCallbackDummy);

                m_active = m_presets.add_item(preset_default);
            }
        }
        if (m_active < m_presets.get_count() && g_layout_window.get_wnd())
        {
            uie::splitter_item_ptr item;
            m_presets[m_active].get(item);
            g_layout_window.set_child(item.get_ptr());
        }
    }

}

void cfg_layout_t::get_preset_name(t_size index, pfc::string_base & p_out)
{
    if (index < m_presets.get_count())
    {
        p_out = m_presets[index].m_name;
    }
}
void cfg_layout_t::set_preset_name(t_size index, const char * ptr, t_size len)
{
    if (index < m_presets.get_count())
    {
        m_presets[index].m_name.set_string(ptr, len);
    }
}

const pfc::list_base_const_t<cfg_layout_t::preset> & cfg_layout_t::get_presets() const { return m_presets; }

void cfg_layout_t::get_data_raw(stream_writer * out, abort_callback & p_abort)
{
    out->write_lendian_t(t_uint32(stream_version_current), p_abort);
    out->write_lendian_t(m_active, p_abort);
    unsigned n, count = m_presets.get_count();
    out->write_lendian_t(count, p_abort);
    for (n = 0; n < count; n++)
    {
        if (n != m_active || !g_layout_window.get_wnd())
            m_presets[n].write(out, p_abort);
        else
        {
            uie::splitter_item_ptr item;
            g_layout_window.get_child(item);
            out->write_lendian_t(item->get_panel_guid(), p_abort);
            stream_writer_memblock conf;
            item->get_panel_config(&conf);
            out->write_string(m_presets[n].m_name.get_ptr(), p_abort);
            out->write_lendian_t(conf.m_data.get_size(), p_abort);
            out->write(conf.m_data.get_ptr(), conf.m_data.get_size(), p_abort);
        }
    }

}

void cfg_layout_t::set_data_raw(stream_reader * p_reader, unsigned p_sizehint, abort_callback & p_abort)
{
    t_uint32 version;
    p_reader->read_lendian_t(version, p_abort);
    if (version <= stream_version_current)
    {
        m_presets.remove_all();
        p_reader->read_lendian_t(m_active, p_abort);
        unsigned n, count;
        p_reader->read_lendian_t(count, p_abort);
        for (n = 0; n < count; n++)
        {
            preset temp;
            temp.read(p_reader, p_abort);
            m_presets.add_item(temp);
        }

        if (m_active < m_presets.get_count())
        {
            uie::splitter_item_simple_t item;
            item.set_panel_guid(m_presets[m_active].m_guid);
            item.set_panel_config_from_ptr(m_presets[m_active].m_val.get_ptr(), m_presets[m_active].m_val.get_size());
            g_layout_window.set_child(&item);
        }
        //m_initialised = m_presets.get_count() > 0;
    }
}

void cfg_layout_t::get_active_preset_for_use(uie::splitter_item_ptr & p_out)
{
    if (!m_presets.get_count())
        reset_presets();
    if (m_presets.get_count())
    {
        if (m_active >= m_presets.get_count())
            m_active = 0;
        m_presets[m_active].get(p_out);
    }
}

