#include "stdafx.h"
#include "layout.h"

namespace cui::default_presets {

struct Node {
    GUID guid{};
    std::initializer_list<Node> children = std::initializer_list<Node>{};
    bool locked{};
    uie::size_and_dpi size{175};
};

struct Preset {
    std::string_view name;
    Node node;
};

// clang-format off
const Preset default_preset = {"Default",
    {panels::guid_horizontal_splitter, {
        {panels::guid_vertical_splitter, {
            {panels::guid_playlist_switcher},
        }, true},
        {panels::guid_vertical_splitter, {
            {panels::guid_playlist_view_v2},
        }},
    }}
};

const std::array<Preset, 10> quick_setup_presets = {{
    {"Playlist switcher",
        {panels::guid_horizontal_splitter, {
            {panels::guid_vertical_splitter, {
                {panels::guid_playlist_switcher},
            }, true},
            {panels::guid_vertical_splitter, {
                {panels::guid_playlist_view_v2},
            }},
        }}
    },
    {"Playlist switcher + Filters",
        {panels::guid_horizontal_splitter, {
            {panels::guid_vertical_splitter, {
                {panels::guid_playlist_switcher},
            }, true},
            {panels::guid_vertical_splitter,{
                {panels::guid_horizontal_splitter, {
                    {panels::guid_filter},
                    {panels::guid_filter},
                    {panels::guid_filter},
                }, true},
                {panels::guid_playlist_view_v2},
            }},
        }}
    },
    {"Playlist switcher + Artwork",
        {panels::guid_horizontal_splitter, {
            {panels::guid_vertical_splitter, {
                {panels::guid_playlist_switcher},
                {panels::guid_artwork_view, {}, true}
            }, true},
            {panels::guid_vertical_splitter, {
                {panels::guid_playlist_view_v2},
            }},
        }}
    },
    {"Playlist switcher + Artwork + Filters",
        {panels::guid_horizontal_splitter, {
            {panels::guid_vertical_splitter, {
                {panels::guid_playlist_switcher},
                {panels::guid_artwork_view, {}, true}
            }, true},
            {panels::guid_vertical_splitter, {
                {panels::guid_horizontal_splitter, {
                    {panels::guid_filter},
                    {panels::guid_filter},
                    {panels::guid_filter},
                }, true},
                {panels::guid_playlist_view_v2},
            }},
        }}
    },
    {"Playlist switcher + Item details + Artwork ",
        {panels::guid_horizontal_splitter, {
            {panels::guid_vertical_splitter, {{panels::guid_playlist_switcher}}, true},
            {panels::guid_vertical_splitter, {
                {panels::guid_playlist_view_v2},
                {panels::guid_horizontal_splitter, {
                    {panels::guid_item_details},
                    {panels::guid_artwork_view, {}, true, 125},
                }, true, 125},
            }},
        }}
    },
    {"Playlist switcher + Filters + Item details + Artwork ",
        {panels::guid_horizontal_splitter, {
            {panels::guid_vertical_splitter, {{panels::guid_playlist_switcher}}, true},
            {panels::guid_vertical_splitter, {
                {panels::guid_horizontal_splitter, {
                    {panels::guid_filter},
                    {panels::guid_filter},
                    {panels::guid_filter},
                }, true},
                {panels::guid_playlist_view_v2},
                {panels::guid_horizontal_splitter, {
                    {panels::guid_item_details},
                    {panels::guid_artwork_view, {}, true, 125},
                }, true, 125},
            }},
        }}
    },
    {"Playlist tabs",
        {panels::guid_vertical_splitter, {
            {panels::guid_playlist_tabs, {
                {panels::guid_playlist_view_v2},
            }},
        }}
    },
    {"Playlist tabs + Filters",
        {panels::guid_vertical_splitter, {
            {panels::guid_horizontal_splitter, {
                {panels::guid_filter},
                {panels::guid_filter},
                {panels::guid_filter},
            }, true},
            {panels::guid_playlist_tabs, {
                {panels::guid_playlist_view_v2},
            }},
        }}
    },
    {"Playlist tabs + Item details + Artwork",
        {panels::guid_vertical_splitter, {
            {panels::guid_playlist_tabs, {
                {panels::guid_playlist_view_v2},
            }},
            {panels::guid_horizontal_splitter, {
                {panels::guid_item_details},
                {panels::guid_artwork_view, {}, true, 125},
            }, true, 125},
        }}
    },
    {"Playlist tabs + Filters + Item details + Artwork",
        {panels::guid_vertical_splitter, {
            {panels::guid_horizontal_splitter, {
                {panels::guid_filter},
                {panels::guid_filter},
                {panels::guid_filter},
            }, true},
            {panels::guid_playlist_tabs, {
                {panels::guid_playlist_view_v2},
            }},
            {panels::guid_horizontal_splitter, {
                {panels::guid_item_details},
                {panels::guid_artwork_view, {}, true, 125},
            }, true, 125},
        }}
    },
}};
// clang-format on

uie::window::ptr node_to_window(Node node)
{
    abort_callback_dummy aborter;
    uie::window::ptr window;
    uie::window::create_by_guid(node.guid, window);

    if (node.children.size() > 0) {
        uie::splitter_window::ptr splitter;

        if (!window->service_query_t(splitter))
            uBugCheck();

        for (auto&& child : node.children) {
            const auto child_window = node_to_window(child);

            stream_writer_memblock conf;
            child_window->get_config(&conf, aborter);

            uie::splitter_item_full_v3_impl_t item;
            item.set_panel_guid(child.guid);
            item.m_locked = child.locked;
            item.m_show_caption = false;
            item.m_size_v2 = child.size.size;
            item.m_size_v2_dpi = child.size.dpi;
            item.set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());

            splitter->add_panel(&item);
        }
    }
    return window;
}

ConfigLayout::Preset preset_to_config_preset(Preset preset)
{
    const auto window = node_to_window(preset.node);
    abort_callback_dummy aborter;

    ConfigLayout::Preset preset_default;
    preset_default.m_name.set_string(preset.name.data(), preset.name.size());
    preset_default.m_guid = preset.node.guid;
    stream_writer_memblock_ref conf(preset_default.m_val, true);
    window->get_config(&conf, aborter);
    return preset_default;
}

} // namespace cui::default_presets

ConfigLayout::ConfigLayout(const GUID& p_guid)
    : cfg_var(p_guid)
    , m_active(0) //, m_initialised(false)
    {};

void ConfigLayout::Preset::write(stream_writer* out, abort_callback& p_abort)
{
    out->write_lendian_t(m_guid, p_abort);
    out->write_string(m_name.get_ptr(), p_abort);
    out->write_lendian_t(m_val.get_size(), p_abort);
    out->write(m_val.get_ptr(), m_val.get_size(), p_abort);
}
void ConfigLayout::Preset::read(stream_reader* stream, abort_callback& p_abort)
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

void ConfigLayout::Preset::get(uie::splitter_item_ptr& p_out)
{
    p_out = new uie::splitter_item_simple_t;
    p_out->set_panel_guid(m_guid);
    p_out->set_panel_config_from_ptr(m_val.get_ptr(), m_val.get_size());
}

void ConfigLayout::Preset::set(const uie::splitter_item_t* item)
{
    m_guid = item->get_panel_guid();
    item->get_panel_config_to_array(m_val, true);
}

void ConfigLayout::get_preset(t_size index, uie::splitter_item_ptr& p_out)
{
    if (index == m_active && g_layout_window.get_wnd()) {
        g_layout_window.get_child(p_out);
    } else if (index < m_presets.get_count()) {
        m_presets[index].get(p_out);
    }
}

void ConfigLayout::set_preset(t_size index, const uie::splitter_item_t* item)
{
    if (index == m_active && g_layout_window.get_wnd()) {
        g_layout_window.set_child(item);
    } else if (index < m_presets.get_count()) // else??
    {
        m_presets[index].set(item);
    }
}

t_size ConfigLayout::add_preset(const Preset& item)
{
    return m_presets.add_item(item);
}
t_size ConfigLayout::add_preset(const char* p_name, t_size len)
{
    Preset temp;
    temp.m_name.set_string(p_name, len);
    temp.m_guid = cui::panels::guid_playlist_view_v2;
    return m_presets.add_item(temp);
}
void ConfigLayout::save_active_preset()
{
    if (m_active < m_presets.get_count() && g_layout_window.get_wnd()) {
        uie::splitter_item_ptr ptr;
        g_layout_window.get_child(ptr);
        m_presets[m_active].set(ptr.get_ptr());
    }
}

void ConfigLayout::set_active_preset(t_size index)
{
    if (index < m_presets.get_count() && m_active != index) {
        m_active = index;
        uie::splitter_item_ptr item;
        m_presets[index].get(item);
        if (g_layout_window.get_wnd())
            g_layout_window.set_child(item.get_ptr());
    }
}

t_size ConfigLayout::delete_preset(t_size index)
{
    if (index < m_presets.get_count()) {
        if (index == m_active)
            m_active = pfc_infinite;
        else if (index < m_active)
            m_active--;
        m_presets.remove_by_idx(index);
    }
    return m_presets.get_count();
}

void ConfigLayout::set_presets(const pfc::list_base_const_t<Preset>& presets, t_size active)
{
    if (presets.get_count()) {
        m_presets.remove_all();
        m_active = pfc_infinite;
        m_presets.add_items(presets);
        set_active_preset(active);
    }
}

void LayoutWindow::g_get_default_presets(pfc::list_t<ConfigLayout::Preset>& p_out)
{
    for (auto&& preset : cui::default_presets::quick_setup_presets)
        p_out.add_item(preset_to_config_preset(preset));
}

void ConfigLayout::reset_presets()
{
    if (core_api::are_services_available()) {
        const auto preset = preset_to_config_preset(cui::default_presets::default_preset);
        m_presets.remove_all();
        m_active = m_presets.add_item(preset);

        if (g_layout_window.get_wnd()) {
            uie::splitter_item_ptr item;
            m_presets[m_active].get(item);
            g_layout_window.set_child(item.get_ptr());
        }
    }
}

void ConfigLayout::get_preset_name(t_size index, pfc::string_base& p_out)
{
    if (index < m_presets.get_count()) {
        p_out = m_presets[index].m_name;
    }
}
void ConfigLayout::set_preset_name(t_size index, const char* ptr, t_size len)
{
    if (index < m_presets.get_count()) {
        m_presets[index].m_name.set_string(ptr, len);
    }
}

const pfc::list_base_const_t<ConfigLayout::Preset>& ConfigLayout::get_presets() const
{
    return m_presets;
}

void ConfigLayout::get_data_raw(stream_writer* out, abort_callback& p_abort)
{
    out->write_lendian_t(t_uint32(stream_version_current), p_abort);
    out->write_lendian_t(m_active, p_abort);
    unsigned count = m_presets.get_count();
    out->write_lendian_t(count, p_abort);
    for (unsigned n = 0; n < count; n++) {
        if (n != m_active || !g_layout_window.get_wnd())
            m_presets[n].write(out, p_abort);
        else {
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

void ConfigLayout::set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort)
{
    t_uint32 version;
    p_reader->read_lendian_t(version, p_abort);
    if (version <= stream_version_current) {
        m_presets.remove_all();
        p_reader->read_lendian_t(m_active, p_abort);
        unsigned count;
        p_reader->read_lendian_t(count, p_abort);
        for (unsigned n = 0; n < count; n++) {
            Preset temp;
            temp.read(p_reader, p_abort);
            m_presets.add_item(temp);
        }

        if (m_active < m_presets.get_count()) {
            uie::splitter_item_simple_t item;
            item.set_panel_guid(m_presets[m_active].m_guid);
            item.set_panel_config_from_ptr(m_presets[m_active].m_val.get_ptr(), m_presets[m_active].m_val.get_size());
            g_layout_window.set_child(&item);
        }
        // m_initialised = m_presets.get_count() > 0;
    }
}

void ConfigLayout::get_active_preset_for_use(uie::splitter_item_ptr& p_out)
{
    if (!m_presets.get_count())
        reset_presets();
    if (m_presets.get_count()) {
        if (m_active >= m_presets.get_count())
            m_active = 0;
        m_presets[m_active].get(p_out);
    }
}
