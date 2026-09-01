#include "pch.h"
#include "layout.h"
#include "main_window.h"

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
    preset_default.name.set_string(preset.name.data(), preset.name.size());
    preset_default.window_id = preset.node.guid;
    stream_writer_memblock_ref conf(preset_default.window_config, true);
    window->get_config(&conf, aborter);
    return preset_default;
}

} // namespace cui::default_presets

ConfigLayout::ConfigLayout(const GUID& p_guid) : cfg_var(p_guid), m_active(0) //, m_initialised(false)
{
}

void ConfigLayout::Preset::get(uie::splitter_item_ptr& p_out) const
{
    p_out = new uie::splitter_item_simple_t;
    p_out->set_panel_guid(window_id);
    p_out->set_panel_config_from_ptr(window_config.get_ptr(), window_config.get_size());
}

void ConfigLayout::Preset::set(const uie::splitter_item_t* item)
{
    window_id = item->get_panel_guid();
    item->get_panel_config_to_array(window_config, true);
}

void ConfigLayout::get_preset(size_t index, uie::splitter_item_ptr& p_out)
{
    if (index == m_active && g_layout_window.get_wnd()) {
        p_out = g_layout_window.get_child();
    } else if (index < m_presets.size()) {
        m_presets[index].get(p_out);
    }
}

void ConfigLayout::set_preset(size_t index, const uie::splitter_item_t* item)
{
    if (index == m_active && g_layout_window.get_wnd()) {
        g_layout_window.set_child(item);
        cui::main_window.update_window();
    } else if (index < m_presets.size()) {
        m_presets[index].set(item);
    }
}

size_t ConfigLayout::add_preset(const Preset& item)
{
    m_presets.emplace_back(item);
    return m_presets.size() - 1;
}

size_t ConfigLayout::add_preset(std::string_view name, bool remember_window_placement)
{
    Preset temp;
    temp.name.set_string(name.data(), name.size());
    temp.remember_window_placement = remember_window_placement;
    temp.window_id = cui::panels::guid_playlist_view_v2;
    m_presets.emplace_back(std::move(temp));
    return m_presets.size() - 1;
}

void ConfigLayout::save_active_preset()
{
    if (m_active < m_presets.size() && g_layout_window.get_wnd()) {
        const auto ptr = g_layout_window.get_child();
        m_presets[m_active].set(ptr.get_ptr());
    }
}

void ConfigLayout::set_active_preset(size_t index)
{
    if (index >= m_presets.size() || m_active == index)
        return;

    cui::main_window.save_window_placement();

    m_active = index;

    if (!cui::main_window.get_wnd())
        return;

    const auto& preset = m_presets[index];

    if (g_layout_window.get_wnd()) {
        uie::splitter_item_ptr item;
        preset.get(item);
        g_layout_window.set_child(item.get_ptr());
    }

    if (preset.remember_window_placement) {
        if (preset.placement_and_dpi) {
            cui::main_window.override_window_placement(*preset.placement_and_dpi);
        } else {
            cui::main_window.restore_window_placement();
            cui::main_window.mark_window_placement_overridden();
        }
    } else {
        cui::main_window.restore_window_placement();
    }

    cui::main_window.update_window();
}

size_t ConfigLayout::delete_preset(size_t index)
{
    if (index < m_presets.size()) {
        if (index == m_active)
            m_active = pfc_infinite;
        else if (index < m_active)
            m_active--;

        m_presets.erase(m_presets.begin() + index);
    }
    return m_presets.size();
}

void ConfigLayout::set_presets(std::vector<Preset> presets, size_t active)
{
    if (presets.size() == 0)
        return;

    m_active = pfc_infinite;
    m_presets = std::move(presets);
    set_active_preset(active);
}

void LayoutWindow::g_get_default_presets(std::vector<ConfigLayout::Preset>& p_out)
{
    for (auto&& preset : cui::default_presets::quick_setup_presets)
        p_out.emplace_back(preset_to_config_preset(preset));
}

void ConfigLayout::reset_presets()
{
    if (core_api::are_services_available()) {
        const auto preset = preset_to_config_preset(cui::default_presets::default_preset);
        m_presets.clear();
        m_presets.emplace_back(preset);
        m_active = m_presets.size() - 1;

        if (g_layout_window.get_wnd()) {
            uie::splitter_item_ptr item;
            m_presets[m_active].get(item);
            g_layout_window.set_child(item.get_ptr());
        }

        cui::main_window.restore_window_placement();
    }
}

void ConfigLayout::get_preset_name(size_t index, pfc::string_base& p_out)
{
    if (index < m_presets.size()) {
        p_out = m_presets[index].name;
    }
}
void ConfigLayout::set_preset_name(size_t index, const char* ptr, size_t len)
{
    if (index < m_presets.size()) {
        m_presets[index].name.set_string(ptr, len);
    }
}

bool ConfigLayout::get_remember_window_placement(size_t index) const
{
    return index < m_presets.size() && m_presets[index].remember_window_placement;
}

void ConfigLayout::set_remember_window_placement(size_t index, bool value)
{
    if (index >= m_presets.size())
        return;

    auto& preset = m_presets[index];

    if (preset.remember_window_placement == value)
        return;

    preset.remember_window_placement = value;

    if (!value)
        preset.placement_and_dpi.reset();

    if (index == m_active) {
        if (value) {
            cui::main_window.mark_window_placement_overridden();
        } else {
            cui::main_window.restore_window_placement();
        }
    }
}

void ConfigLayout::set_active_window_placement(cui::config::WindowPlacementAndDpi placement_and_dpi)
{
    if (m_active < m_presets.size())
        m_presets[m_active].placement_and_dpi = placement_and_dpi;
}

std::optional<WINDOWPLACEMENT> ConfigLayout::get_active_window_placement() const
{
    if (m_active >= m_presets.size())
        return {};

    const auto& preset = m_presets[m_active];

    if (preset.remember_window_placement && preset.placement_and_dpi)
        return preset.placement_and_dpi->get_adjusted_placement();

    return {};
}

bool ConfigLayout::is_remember_window_placement_active() const
{
    return cfg_layout.get_remember_window_placement(m_active);
}

const std::vector<ConfigLayout::Preset>& ConfigLayout::get_presets() const
{
    return m_presets;
}

void ConfigLayout::get_data_raw(stream_writer* out, abort_callback& p_abort)
{
    if (is_remember_window_placement_active())
        cui::main_window.save_window_placement();

    out->write_lendian_t(static_cast<uint32_t>(stream_version_current), p_abort);
    out->write_lendian_t(gsl::narrow<uint32_t>(m_active), p_abort);
    const auto preset_count = gsl::narrow<uint32_t>(m_presets.size());
    out->write_lendian_t(preset_count, p_abort);

    save_active_preset();

    for (const auto& preset : m_presets) {
        out->write_lendian_t(preset.window_id, p_abort);
        out->write_string(preset.name.get_ptr(), p_abort);
        out->write_lendian_t(gsl::narrow<uint32_t>(preset.window_config.get_size()), p_abort);
        out->write(preset.window_config.get_ptr(), preset.window_config.get_size(), p_abort);
    }

    for (const auto& preset : m_presets) {
        stream_writer_memblock extra_writer;
        extra_writer.write_lendian_t(preset.remember_window_placement, p_abort);
        const auto has_placement = preset.remember_window_placement && preset.placement_and_dpi;
        extra_writer.write_lendian_t(has_placement, p_abort);
        if (has_placement)
            cui::config::write_window_placement_and_dpi(extra_writer, *preset.placement_and_dpi, p_abort);

        out->write_lendian_t(gsl::narrow<uint32_t>(extra_writer.m_data.get_size()), p_abort);
        out->write(extra_writer.m_data.get_ptr(), extra_writer.m_data.get_size(), p_abort);
    }
}

void ConfigLayout::set_data_raw(stream_reader* base_reader, size_t p_sizehint, abort_callback& p_abort)
{
    stream_reader_limited_ref reader(base_reader, p_sizehint);

    const auto version = reader.read_lendian_t<uint32_t>(p_abort);

    if (version > stream_version_current)
        return;

    std::vector<Preset> presets;
    m_active = reader.read_lendian_t<uint32_t>(p_abort);

    const auto preset_count = reader.read_lendian_t<uint32_t>(p_abort);

    for (auto _ : ranges::views::iota(0u, preset_count)) {
        Preset preset;
        reader.read_lendian_t(preset.window_id, p_abort);
        preset.name = reader.read_string(p_abort);

        const auto size = reader.read_lendian_t<uint32_t>(p_abort);
        preset.window_config.set_size(size);
        reader.read(preset.window_config.get_ptr(), preset.window_config.get_size(), p_abort);

        presets.push_back(std::move(preset));
    }

    if (reader.get_remaining() > 0) {
        for (auto& preset : presets) {
            const auto extra_size = reader.read_lendian_t<uint32_t>(p_abort);
            stream_reader_limited_ref extra_reader(&reader, extra_size);
            extra_reader.read_lendian_t(preset.remember_window_placement, p_abort);
            const auto has_placement = extra_reader.read_lendian_t<bool>(p_abort);

            if (has_placement)
                preset.placement_and_dpi = cui::config::read_window_placement_and_dpi(extra_reader, p_abort);

            extra_reader.flush_remaining(p_abort);
        }
    }

    m_presets = std::move(presets);

    if (m_active < m_presets.size()) {
        uie::splitter_item_simple_t item;
        item.set_panel_guid(m_presets[m_active].window_id);
        item.set_panel_config_from_ptr(
            m_presets[m_active].window_config.get_ptr(), m_presets[m_active].window_config.get_size());
        g_layout_window.set_child(&item);
    }
}

void ConfigLayout::get_active_preset_for_use(uie::splitter_item_ptr& p_out)
{
    if (!m_presets.size())
        reset_presets();
    if (m_presets.size()) {
        if (m_active >= m_presets.size())
            m_active = 0;
        m_presets[m_active].get(p_out);
    }
}
