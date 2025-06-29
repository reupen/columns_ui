#include "pch.h"
#include "item_properties.h"

#include "file_info_utils.h"

namespace cui::panels::item_properties {

// {8F6069CD-2E36-4ead-B171-93F3DFF0073A}
static const GUID g_guid_selection_properties
    = {0x8f6069cd, 0x2e36, 0x4ead, {0xb1, 0x71, 0x93, 0xf3, 0xdf, 0xf0, 0x7, 0x3a}};

// {02570710-E204-4077-AEAE-5A00D6A2AC08}
static const GUID g_guid_selection_properties_tracking_mode
    = {0x2570710, 0xe204, 0x4077, {0xae, 0xae, 0x5a, 0x0, 0xd6, 0xa2, 0xac, 0x8}};

// {1D921F23-1708-451a-A02E-C6657F7C4386}
static const GUID g_guid_selection_poperties_edge_style
    = {0x1d921f23, 0x1708, 0x451a, {0xa0, 0x2e, 0xc6, 0x65, 0x7f, 0x7c, 0x43, 0x86}};

// {C314F956-71AD-4d75-8749-5882508F6904}
static const GUID g_guid_selection_poperties_info_sections
    = {0xc314f956, 0x71ad, 0x4d75, {0x87, 0x49, 0x58, 0x82, 0x50, 0x8f, 0x69, 0x4}};

// {9AE7CE9F-7DA8-4115-A895-8D519A039325}
static const GUID g_guid_selection_poperties_show_column_titles
    = {0x9ae7ce9f, 0x7da8, 0x4115, {0xa8, 0x95, 0x8d, 0x51, 0x9a, 0x3, 0x93, 0x25}};

// {B84886A5-4510-42d0-837A-33E55360C24E}
static const GUID g_guid_selection_poperties_show_group_titles
    = {0xb84886a5, 0x4510, 0x42d0, {0x83, 0x7a, 0x33, 0xe5, 0x53, 0x60, 0xc2, 0x4e}};

cfg_uint cfg_selection_properties_tracking_mode(g_guid_selection_properties_tracking_mode, 0);
cfg_uint cfg_selection_properties_edge_style(g_guid_selection_poperties_edge_style, 0);
cfg_uint cfg_selection_properties_info_sections(g_guid_selection_poperties_info_sections, 1 + 2 + 4);
cfg_bool cfg_selection_poperties_show_column_titles(g_guid_selection_poperties_show_column_titles, true);
cfg_bool cfg_selection_poperties_show_group_titles(g_guid_selection_poperties_show_group_titles, true);

std::vector<ItemProperties*> ItemProperties::s_windows;

namespace {
colours::client::factory<ItemPropertiesColoursClient> g_appearance_client_impl;
}

void ItemProperties::s_redraw_all()
{
    for (auto& window : s_windows)
        window->invalidate_all();
}

void ItemProperties::s_on_dark_mode_status_change()
{
    const auto is_dark = colours::is_dark_mode_active();
    for (auto&& window : s_windows)
        window->set_use_dark_mode(is_dark);
}

void ItemProperties::s_on_app_activate(bool b_activated)
{
    for (auto& window : s_windows)
        window->on_app_activate(b_activated);
}

void ItemProperties::on_app_activate(bool b_activated)
{
    if (b_activated) {
        if (GetFocus() != get_wnd())
            register_callback();
    } else {
        deregister_callback();
    }
}

const GUID& ItemProperties::get_extension_guid() const
{
    return g_guid_selection_properties;
}
void ItemProperties::get_name(pfc::string_base& p_out) const
{
    p_out = "Item properties";
}
void ItemProperties::get_category(pfc::string_base& p_out) const
{
    p_out = "Panels";
}
unsigned ItemProperties::get_type() const
{
    return uie::type_panel;
}

void ItemProperties::set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    if (!p_size)
        return;

    const auto version = p_reader->read_lendian_t<uint32_t>(p_abort);

    if (version > config_version_current)
        return;

    uint32_t field_count;
    p_reader->read_lendian_t(field_count, p_abort);
    m_fields.remove_all();
    m_fields.set_count(field_count);

    for (uint32_t i = 0; i < field_count; i++) {
        p_reader->read_string(m_fields[i].m_name_friendly, p_abort);
        p_reader->read_string(m_fields[i].m_name, p_abort);
    }
    p_reader->read_lendian_t(m_tracking_mode, p_abort);
    p_reader->read_lendian_t(m_autosizing_columns, p_abort);
    p_reader->read_lendian_t(m_column_name_width.value, p_abort);
    m_column_name_width.dpi = uih::get_system_dpi_cached().cx;
    p_reader->read_lendian_t(m_column_field_width.value, p_abort);
    m_column_field_width.dpi = uih::get_system_dpi_cached().cx;

    if (version < 3)
        return;

    p_reader->read_lendian_t(m_edge_style, p_abort);

    if (version < 4)
        return;

    p_reader->read_lendian_t(m_info_sections_mask, p_abort);
    p_reader->read_lendian_t(m_show_column_titles, p_abort);
    p_reader->read_lendian_t(m_show_group_titles, p_abort);

    if (version < 5)
        return;

    p_reader->read_lendian_t(m_column_name_width.dpi, p_abort);
    p_reader->read_lendian_t(m_column_field_width.dpi, p_abort);
}

void ItemProperties::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(static_cast<uint32_t>(config_version_current), p_abort);
    size_t count = m_fields.get_count();
    p_writer->write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
    for (size_t i = 0; i < count; i++) {
        p_writer->write_string(m_fields[i].m_name_friendly, p_abort);
        p_writer->write_string(m_fields[i].m_name, p_abort);
    }
    p_writer->write_lendian_t(m_tracking_mode, p_abort);
    p_writer->write_lendian_t(m_autosizing_columns, p_abort);
    p_writer->write_lendian_t(m_column_name_width.value, p_abort);
    p_writer->write_lendian_t(m_column_field_width.value, p_abort);
    p_writer->write_lendian_t(m_edge_style, p_abort);

    p_writer->write_lendian_t(m_info_sections_mask, p_abort);
    p_writer->write_lendian_t(m_show_column_titles, p_abort);
    p_writer->write_lendian_t(m_show_group_titles, p_abort);

    p_writer->write_lendian_t(m_column_name_width.dpi, p_abort);
    p_writer->write_lendian_t(m_column_field_width.dpi, p_abort);
}

void ItemProperties::notify_on_initialisation()
{
    set_use_dark_mode(colours::is_dark_mode_active());
    set_autosize(m_autosizing_columns);

    recreate_items_text_format();
    recreate_header_text_format();
    recreate_group_text_format();

    set_edge_style(m_edge_style);
    set_show_header(m_show_column_titles);
    set_group_level_indentation_enabled(false);
}
void ItemProperties::notify_on_create()
{
    set_columns({{"Field", m_column_name_width, 0}, {"Value", m_column_field_width, 1}});
    set_group_count(m_show_group_titles ? 1 : 0);

    register_callback();
    play_callback_manager::get()->register_callback(this, flag_on_playback_stop | flag_on_playback_new_track, true);
    metadb_io_v3::get()->register_callback(this);
    refresh_contents();

    if (s_windows.empty())
        s_create_message_window();

    s_windows.push_back(this);
}
void ItemProperties::notify_on_destroy()
{
    std::erase(s_windows, this);
    if (s_windows.empty())
        s_destroy_message_window();

    play_callback_manager::get()->unregister_callback(this);
    metadb_io_v3::get()->unregister_callback(this);
    deregister_callback();
    m_handles.remove_all();
    m_selection_handles.remove_all();
    m_edit_handles.remove_all();
    m_selection_holder.release();
    m_library_autocomplete_v1.reset();
    m_library_autocomplete_v2.reset();
}

void ItemProperties::notify_on_set_focus(HWND wnd_lost)
{
    deregister_callback();
    m_selection_holder = ui_selection_manager::get()->acquire();
    m_selection_holder->set_selection(m_handles);
}
void ItemProperties::notify_on_kill_focus(HWND wnd_receiving)
{
    m_selection_holder.release();
    register_callback();
}

void ItemProperties::register_callback()
{
    if (!m_callback_registered)
        g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
    m_callback_registered = true;
}
void ItemProperties::deregister_callback()
{
    if (m_callback_registered)
        ui_selection_manager::get()->unregister_callback(this);
    m_callback_registered = false;
}

class MetadataFieldValueAggregator {
public:
    bool process_file_info(const char* field, const file_info* info)
    {
        if (m_truncated)
            return false;

        const size_t field_index = info->meta_find(field);

        if (field_index == (std::numeric_limits<size_t>::max)()) {
            m_some_values_missing = true;
            return true;
        }

        const size_t value_count = info->meta_enum_value_count(field_index);

        for (auto value_index : ranges::views::iota(size_t{0}, value_count)) {
            const auto value = info->meta_enum_value(field_index, value_index);
            add_value(value);
        }
        return !m_truncated;
    }

    std::vector<std::string> m_values;
    bool m_truncated{false};
    bool m_some_values_missing{};

private:
    void add_value(const char* p_value)
    {
        if (m_values.size() >= max_values)
            return;

        auto has_value = [p_value](auto&& value) { return !stricmp_utf8(p_value, value.c_str()); };

        if (ranges::any_of(m_values, has_value))
            return;

        m_values.emplace_back(p_value);

        if (m_values.size() == max_values)
            m_truncated = true;
    }

    static constexpr size_t max_values = 16;
};

class TrackProperty {
public:
    using Self = TrackProperty;

    pfc::string8 m_name;
    pfc::string8 m_value;
    double m_sortpriority{0};

    static int s_compare(Self const& a, Self const& b)
    {
        int ret = pfc::compare_t(a.m_sortpriority, b.m_sortpriority);
        if (!ret)
            ret = StrCmpLogicalW(
                pfc::stringcvt::string_wide_from_utf8(a.m_name), pfc::stringcvt::string_wide_from_utf8(b.m_name));
        return ret;
    }
};

class TrackPropertyCallback : public track_property_callback_v2 {
public:
    TrackPropertyCallback(const std::vector<std::string>& fields, bool include_unknown_sections)
        : m_fields(fields)
        , m_include_unknown_sections(include_unknown_sections)
    {
        m_known_sections.resize(g_info_sections.size());
    }

    void set_property(const char* p_group, double p_sortpriority, const char* p_name, const char* p_value) override
    {
        if (!is_group_wanted(p_group))
            return;

        auto section = ranges::find_if(g_info_sections, [p_group](auto&& section) {
            return !section.is_unknown_section_default && !stricmp_utf8(p_group, section.name);
        });

        if (section != g_info_sections.end()) {
            auto index = std::distance(g_info_sections.begin(), section);
            m_known_sections[index].push_back({p_name, p_value, p_sortpriority});
        } else {
            m_unknown_sections[p_group].push_back({p_name, p_value, p_sortpriority});
        }
    }

    bool is_group_wanted(const char* p_group) override
    {
        auto is_known_section = ranges::any_of(g_info_sections, [p_group](auto&& section) {
            return !section.is_unknown_section_default && !stricmp_utf8(p_group, section.name);
        });

        if (is_known_section)
            return ranges::any_of(
                m_fields, [p_group](auto&& section) { return !stricmp_utf8(p_group, section.c_str()); });

        return m_include_unknown_sections;
    }

    auto to_sorted_vector()
    {
        std::vector<std::tuple<std::string, concurrency::concurrent_vector<TrackProperty>>> all_sections;
        std::vector<std::tuple<std::string, concurrency::concurrent_vector<TrackProperty>>> unknown_sections;

        for (auto&& [index, values] : ranges::views::enumerate(m_known_sections)) {
            if (values.empty())
                continue;

            ranges::sort(values, TrackProperty::s_compare);

            all_sections.emplace_back(g_info_sections[index].name, std::move(values));
        }

        for (auto&& [name, values] : m_unknown_sections) {
            ranges::sort(values, TrackProperty::s_compare);

            unknown_sections.emplace_back(name, std::move(values));
        }

        ranges::sort(unknown_sections, [](auto&& left, auto&& right) {
            auto&& [left_name, _left_values] = left;
            auto&& [right_name, _right_values] = right;
            return stricmp_utf8(left_name.data(), right_name.data()) < 0;
        });

        ranges::push_back(all_sections, unknown_sections);

        return all_sections;
    }

private:
    std::vector<concurrency::concurrent_vector<TrackProperty>> m_known_sections;
    concurrency::concurrent_unordered_map<std::string, concurrency::concurrent_vector<TrackProperty>>
        m_unknown_sections;
    const std::vector<std::string>& m_fields;
    bool m_include_unknown_sections{};
};

class TrackPropertyInfoSourceProviderV3 : public track_property_provider_v3_info_source {
public:
    TrackPropertyInfoSourceProviderV3(
        const std::vector<metadb_v2_rec_t>& recs, const std::vector<metadb_info_container::ptr>& info_refs)
        : m_recs(recs)
        , m_info_refs(info_refs)
    {
    }

    metadb_info_container::ptr get_info(size_t index) override
    {
        auto info_ref = m_recs.empty() ? m_info_refs[index] : m_recs[index].info;

        if (!info_ref.is_valid())
            return m_dummy_info_ref;

        return info_ref;
    }

private:
    metadb_info_container::ptr m_dummy_info_ref{fb2k::service_new<metadb_info_container_const_impl>()};
    const std::vector<metadb_v2_rec_t>& m_recs;
    const std::vector<metadb_info_container::ptr>& m_info_refs;
};

class TrackPropertyInfoSourceProviderV5 : public track_property_provider_v5_info_source {
public:
    explicit TrackPropertyInfoSourceProviderV5(const std::vector<metadb_v2_rec_t>& recs) : m_recs(recs) {}
    metadb_v2_rec_t get_info(size_t index) override { return m_recs[index]; }

private:
    const std::vector<metadb_v2_rec_t>& m_recs;
};

decltype(std::declval<TrackPropertyCallback>().to_sorted_vector()) get_track_properties(
    const std::vector<metadb_v2_rec_t>& recs, const std::vector<metadb_info_container::ptr>& info_refs,
    const std::vector<std::string>& info_sections, bool include_unknown_sections, const metadb_handle_list& tracks)
{
    if (!tracks.get_count())
        return {};

    TrackPropertyCallback props(info_sections, include_unknown_sections);

    std::vector<track_property_provider::ptr> main_thread_providers;
    std::vector<track_property_provider_v4::ptr> thread_safe_providers;
    service_enum_t<track_property_provider> provider_enumerator;

    track_property_provider::ptr enumerator_value;

    while (provider_enumerator.next(enumerator_value)) {
        track_property_provider_v4::ptr provider_v4;

        if (enumerator_value->service_query_t(provider_v4))
            thread_safe_providers.emplace_back(std::move(provider_v4));
        else
            main_thread_providers.emplace_back(std::move(enumerator_value));
    }

    TrackPropertyInfoSourceProviderV3 info_source_v3(recs, info_refs);
    TrackPropertyInfoSourceProviderV5 info_source_v5(recs);

    for (auto&& provider : main_thread_providers) {
        if (track_property_provider_v3::ptr provider_v3; provider->service_query_t(provider_v3)) {
            provider_v3->enumerate_properties_v3(tracks, info_source_v3, props);
        } else if (track_property_provider_v2::ptr provider_v2; provider->service_query_t(provider_v2)) {
            provider_v2->enumerate_properties_v2(tracks, props);
        } else {
            provider->enumerate_properties(tracks, props);
        }
    }

    concurrency::parallel_for(size_t{0}, thread_safe_providers.size(),
        [&thread_safe_providers, &props, &info_source_v3, &info_source_v5, &tracks, has_recs{!recs.empty()}](
            auto&& index) {
            auto&& provider = thread_safe_providers[index];

            track_property_provider_v5::ptr provider_v5;
            if (has_recs && (provider_v5 &= provider)) {
                provider_v5->enumerate_properties_v5(tracks, info_source_v5, props, fb2k::noAbort);
                return;
            }

            provider->enumerate_properties_v4(tracks, info_source_v3, props, fb2k::noAbort);
        });

    return props.to_sorted_vector();
}

void ItemProperties::refresh_contents()
{
    bool b_redraw = disable_redrawing();

    size_t field_count = m_fields.get_count();

    std::vector<MetadataFieldValueAggregator> metadata_aggregators;
    metadata_aggregators.resize(field_count);

    pfc::list_t<InsertItem> items;
    size_t count = m_handles.get_count();

    std::vector<metadb_v2_rec_t> recs;
    std::vector<metadb_info_container::ptr> info_refs;

    const auto metadb_v2_api = metadb_v2::tryGet();

    const auto has_metadb_v2 = metadb_v2_api.is_valid();

    if (has_metadb_v2) {
        recs.resize(count);

        metadb_v2_api->queryMultiParallel_(
            m_handles, [&recs](size_t index, const metadb_v2_rec_t& rec) { recs[index] = rec; });
    } else {
        info_refs.resize(count);

        for (size_t i{}; i < count; i++)
            info_refs[i] = m_handles[i]->get_info_ref();
    }

    file_info_const_impl dummy_file_info;

    concurrency::parallel_for(size_t{0}, field_count,
        [&metadata_aggregators, &info_refs, &recs, &dummy_file_info, has_metadb_v2, this](auto&& field_index) {
            auto& metadata_aggregator = metadata_aggregators[field_index];

            for (size_t i = 0; i < m_handles.get_count(); i++) {
                auto& info_ref = has_metadb_v2 ? recs[i].info : info_refs[i];
                auto& info = info_ref.is_valid() ? info_ref->info() : dummy_file_info;

                if (!metadata_aggregator.process_file_info(m_fields[field_index].m_name, &info))
                    break;
            }
        });

    for (size_t i{}; i < field_count; i++) {
        auto& field = m_fields[i];
        auto& aggregator = metadata_aggregators[i];

        InsertItem item(2, 1);
        pfc::string8 temp;
        item.m_subitems[0] = field.m_name_friendly;
        temp.reset();

        size_t count_values = aggregator.m_values.size();

        for (size_t j = 0; j < count_values; j++) {
            auto&& value = aggregator.m_values[j];

            if (value.length() > 0)
                temp << value.c_str();
            else
                temp << "(blank)";

            if (j + 1 != count_values)
                temp << "; ";
        }

        if (aggregator.m_truncated)
            temp << "; …";
        else if (count_values > 0 && aggregator.m_some_values_missing)
            temp << "; (not set)";

        item.m_subitems[1] = temp;
        item.m_groups[0] = "Metadata";
        items.add_item(item);
    }

    bool include_unknown_sections{};

    std::vector<std::string> info_sections;
    for (auto&& info_section : g_info_sections) {
        if (m_info_sections_mask & (1 << (info_section.id))) {
            if (info_section.is_unknown_section_default)
                include_unknown_sections = true;
            else
                info_sections.emplace_back(info_section.name);
        }
    }

    auto track_properties = get_track_properties(recs, info_refs, info_sections, include_unknown_sections, m_handles);

    for (auto&& [section, values] : track_properties) {
        for (auto&& value : values) {
            InsertItem item(2, 1);
            item.m_subitems[0] = value.m_name;
            item.m_subitems[1] = value.m_value;
            item.m_groups[0] = section.c_str();
            items.add_item(item);
        }
    }

    size_t old_count = get_item_count();
    size_t new_count = items.get_count();

    if (new_count && old_count) {
        pfc::list_t<InsertItem> items_replace;
        items_replace.add_items_fromptr(items.get_ptr(), std::min(new_count, old_count));
        replace_items(0, items_replace);
    }

    if (new_count > old_count) {
        insert_items(old_count, items.get_count() - old_count, items.get_ptr() + old_count);
    } else if (new_count < old_count) {
        remove_items(bit_array_range(new_count, old_count - new_count));
    }

    if (b_redraw)
        enable_redrawing();
}

void ItemProperties::on_playback_new_track(metadb_handle_ptr p_track) noexcept
{
    if (m_tracking_mode == track_nowplaying || m_tracking_mode == track_automatic) {
        m_handles.remove_all();
        m_handles.add_item(p_track);
        refresh_contents();
    }
}

void ItemProperties::on_playback_stop(play_control::t_stop_reason p_reason) noexcept
{
    if (p_reason != play_control::stop_reason_starting_another && p_reason != play_control::stop_reason_shutting_down) {
        if (m_tracking_mode == track_nowplaying || m_tracking_mode == track_automatic) {
            if (m_tracking_mode == track_automatic)
                m_handles = m_selection_handles;
            else
                m_handles.remove_all();
            refresh_contents();
        }
    }
}

void ItemProperties::on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook) noexcept
{
    for (auto&& track : m_handles) {
        if (size_t index{};
            p_items_sorted.bsearch_t(pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, track, index)) {
            refresh_contents();
            break;
        }
    }
}

bool ItemProperties::check_process_on_selection_changed()
{
    HWND wnd_focus = GetFocus();
    if (wnd_focus == nullptr)
        return false;

    DWORD processid = NULL;
    GetWindowThreadProcessId(wnd_focus, &processid);
    return processid == GetCurrentProcessId();
}

void ItemProperties::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) noexcept
{
    if (check_process_on_selection_changed()) {
        if (g_ui_selection_manager_is_now_playing_fallback())
            m_selection_handles.remove_all();
        else
            m_selection_handles = p_selection;

        if (m_tracking_mode == track_nowplaying
            || (m_tracking_mode == track_automatic && play_control::get()->is_playing()))
            return;

        m_handles = m_selection_handles;
        refresh_contents();
    }
}

void ItemProperties::on_tracking_mode_change()
{
    m_handles.remove_all();
    if (m_tracking_mode == track_selection
        || (m_tracking_mode == track_automatic && !play_control::get()->is_playing())) {
        m_handles = m_selection_handles;
    } else if (m_tracking_mode == track_nowplaying
        || (m_tracking_mode == track_automatic && play_control::get()->is_playing())) {
        metadb_handle_ptr item;
        if (playback_control::get()->get_now_playing(item))
            m_handles.add_item(item);
    }
    refresh_contents();
}

class ItemsFontClientItemProperties : public fonts::client {
public:
    const GUID& get_client_guid() const override { return items_font_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item properties: Items"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { ItemProperties::s_on_font_items_change(); }
};

class HeaderFontClientItemProperties : public fonts::client {
public:
    const GUID& get_client_guid() const override { return header_font_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item properties: Column titles"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { ItemProperties::s_on_font_header_change(); }
};

class GroupClientItemProperties : public fonts::client {
public:
    const GUID& get_client_guid() const override { return group_font_id; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item properties: Group titles"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { ItemProperties::s_on_font_groups_change(); }
};
void ItemProperties::s_on_font_items_change()
{
    for (auto& window : s_windows)
        window->recreate_items_text_format();
}

void ItemProperties::s_on_font_groups_change()
{
    for (auto& window : s_windows)
        window->recreate_group_text_format();
}

void ItemProperties::s_on_font_header_change()
{
    for (auto& window : s_windows)
        window->recreate_header_text_format();
}

ItemProperties::ItemProperties()
    : m_tracking_mode(cfg_selection_properties_tracking_mode)
    , m_info_sections_mask(cfg_selection_properties_info_sections)
    , m_show_column_titles(cfg_selection_poperties_show_column_titles)
    , m_show_group_titles(cfg_selection_poperties_show_group_titles)
    , m_edge_style(cfg_selection_properties_edge_style)
    , m_edit_column(pfc_infinite)
    , m_edit_index(pfc_infinite)
{
    m_fields.add_item(Field("Artist", "ARTIST"));
    m_fields.add_item(Field("Title", "TITLE"));
    m_fields.add_item(Field("Album", "ALBUM"));
    m_fields.add_item(Field("Date", "DATE"));
    m_fields.add_item(Field("Genre", "GENRE"));
    m_fields.add_item(Field("Composer", "COMPOSER"));
    m_fields.add_item(Field("Performer", "PERFORMER"));
    m_fields.add_item(Field("Album artist", "ALBUM ARTIST"));
    m_fields.add_item(Field("Track number", "TRACKNUMBER"));
    m_fields.add_item(Field("Total tracks", "TOTALTRACKS"));
    m_fields.add_item(Field("Disc number", "DISCNUMBER"));
    m_fields.add_item(Field("Total discs", "TOTALDISCS"));
    m_fields.add_item(Field("Comment", "COMMENT"));
}

void ItemProperties::s_create_message_window()
{
    uie::container_window_v3_config config(L"{9D0A0408-59AC-4a96-A3EF-FF26B7B7C118}", false);
    config.window_styles = 0;
    config.extended_window_styles = 0;

    s_message_window = std::make_unique<uie::container_window_v3>(
        config, [](auto&& wnd, auto&& msg, auto&& wp, auto&& lp) -> LRESULT {
            if (msg == WM_ACTIVATEAPP)
                s_on_app_activate(wp != 0);
            return DefWindowProc(wnd, msg, wp, lp);
        });
    s_message_window->create(nullptr);
}

void ItemProperties::s_destroy_message_window()
{
    s_message_window->destroy();
    s_message_window.reset();
}

void ItemProperties::notify_save_inline_edit(const char* value)
{
    auto _ = gsl::finally([this] {
        m_edit_column = pfc_infinite;
        m_edit_index = pfc_infinite;
        m_edit_field.reset();
        m_edit_handles.remove_all();
    });

    auto values = helpers::split_meta_value(value);
    const auto filter
        = fb2k::service_new<helpers::SingleFieldFileInfoFilter>(m_edit_field.get_ptr(), std::move(values));

    metadb_io_v2::get()->update_info_async(m_edit_handles, filter, GetAncestor(get_wnd(), GA_ROOT),
        metadb_io_v2::op_flag_no_errors | metadb_io_v2::op_flag_background | metadb_io_v2::op_flag_delay_ui, nullptr);
}

bool ItemProperties::notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
    pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries)
{
    const size_t indices_count = indices.get_count();

    if (m_handles.get_count() == 0 || column != 1 || indices_count != 1 || indices[0] >= m_fields.get_count())
        return false;

    m_edit_index = indices[0];
    m_edit_column = column;
    m_edit_field = m_fields[m_edit_index].m_name;
    m_edit_handles = m_handles;

    if (m_library_autocomplete_v2.is_empty() && m_library_autocomplete_v1.is_empty()) {
        if (!library_meta_autocomplete_v2::tryGet(m_library_autocomplete_v2)) {
            m_library_autocomplete_v1 = library_meta_autocomplete::get();
        }
    }

    p_flags |= inline_edit_autocomplete;
    pfc::com_ptr_t<IUnknown> local_autocomplete_entries;

    if (m_library_autocomplete_v2.is_valid())
        m_library_autocomplete_v2->get_value_list_async(m_edit_field, local_autocomplete_entries);
    else
        m_library_autocomplete_v1->get_value_list(m_edit_field, local_autocomplete_entries);

    autocomplete_entries.attach(local_autocomplete_entries.detach());
    helpers::EditMetadataFieldValueAggregator aggregator;

    for (const auto& track : m_edit_handles) {
        const auto info_container = track->get_info_ref();
        aggregator.process_file_info(m_edit_field, &info_container->info());
    }

    const auto joined = mmh::join(aggregator.m_values, "; "s);

    if (aggregator.m_mixed_values) {
        p_text = "«mixed values» ";
        p_text += joined.c_str();

        if (aggregator.m_truncated)
            p_text += "; …";
    } else {
        p_text = joined.c_str();
    }

    return true;
}

void ItemProperties::s_print_field(const char* field, const file_info& p_info, pfc::string_base& p_out)
{
    size_t meta_index = p_info.meta_find(field);
    if (meta_index != pfc_infinite) {
        size_t count = p_info.meta_enum_value_count(meta_index);
        for (size_t i = 0; i < count; i++)
            p_out << p_info.meta_enum_value(meta_index, i) << (i + 1 < count ? "; " : "");
    }
}

bool ItemProperties::notify_before_create_inline_edit(
    const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse)
{
    return m_handles.get_count() && column == 1 && indices.get_count() == 1 && indices[0] < m_fields.get_count();
}

void ItemProperties::notify_on_column_size_change(size_t index, int new_width)
{
    if (index == 0)
        m_column_name_width = new_width;
    else
        m_column_field_width = new_width;
}

bool ItemProperties::notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp)
{
    uie::window_ptr p_this = this;
    bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
    return ret;
}

bool ItemProperties::notify_on_keyboard_keydown_copy()
{
    copy_selected_items_as_text(1);
    return true;
}

void ItemProperties::get_menu_items(ui_extension::menu_hook_t& p_hook)
{
    ui_extension::menu_node_ptr p_node = new MenuNodeSourcePopup(this);
    p_hook.add_node(p_node);
    p_node = new ModeNodeAutosize(this);
    p_hook.add_node(p_node);
    p_node = new uie::menu_node_configure(this);
    p_hook.add_node(p_node);
}

bool ItemProperties::show_config_popup(HWND wnd_parent)
{
    ItemPropertiesConfig dialog(
        m_fields, m_edge_style, m_info_sections_mask, m_show_column_titles, m_show_group_titles);
    if (dialog.run_modal(wnd_parent)) {
        m_fields = dialog.m_fields;
        if (get_wnd()) {
            m_info_sections_mask = dialog.m_info_sections_mask;
            cfg_selection_properties_info_sections = dialog.m_info_sections_mask;

            m_show_column_titles = dialog.m_show_columns;
            cfg_selection_poperties_show_column_titles = m_show_column_titles;
            set_show_header(m_show_column_titles);

            if (m_show_group_titles != dialog.m_show_groups) {
                m_show_group_titles = dialog.m_show_groups;
                cfg_selection_poperties_show_group_titles = m_show_group_titles;

                remove_items(bit_array_true());
                set_group_count(m_show_group_titles ? 1 : 0);
            }

            refresh_contents();
            m_edge_style = dialog.m_edge_style;
            cfg_selection_properties_edge_style = m_edge_style;
            set_edge_style(m_edge_style);
        }
        return true;
    }
    return false;
}

bool ItemProperties::have_config_popup() const
{
    return true;
}

namespace {
ItemsFontClientItemProperties::factory<ItemsFontClientItemProperties> g_font_client_selection_properties;
HeaderFontClientItemProperties::factory<HeaderFontClientItemProperties> g_font_header_client_selection_properties;
GroupClientItemProperties::factory<GroupClientItemProperties> g_font_group_client_selection_properties;
} // namespace
uie::window_factory<ItemProperties> g_selection_properties;

ItemProperties::MenuNodeSourcePopup::MenuNodeSourcePopup(ItemProperties* p_wnd)
{
    m_items.add_item(new MenuNodeTrackMode(p_wnd, 2));
    m_items.add_item(new MenuNodeTrackMode(p_wnd, 0));
    // m_items.add_item(new uie::menu_node_separator_t());
    // m_items.add_item(new menu_node_track_mode(p_wnd, 2));
    m_items.add_item(new MenuNodeTrackMode(p_wnd, 1));
}

void ItemProperties::MenuNodeSourcePopup::get_child(size_t p_index, uie::menu_node_ptr& p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

size_t ItemProperties::MenuNodeSourcePopup::get_children_count() const
{
    return m_items.get_count();
}

bool ItemProperties::MenuNodeSourcePopup::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Tracking mode";
    p_displayflags = 0;
    return true;
}

ItemProperties::ModeNodeAutosize::ModeNodeAutosize(ItemProperties* p_wnd) : p_this(p_wnd) {}

void ItemProperties::ModeNodeAutosize::execute()
{
    p_this->m_autosizing_columns = !p_this->m_autosizing_columns;
    p_this->set_autosize(p_this->m_autosizing_columns);
}

bool ItemProperties::ModeNodeAutosize::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ItemProperties::ModeNodeAutosize::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Auto-sizing columns";
    p_displayflags = (p_this->m_autosizing_columns) ? state_checked : 0;
    return true;
}

ItemProperties::MenuNodeTrackMode::MenuNodeTrackMode(ItemProperties* p_wnd, uint32_t p_value)
    : p_this(p_wnd)
    , m_source(p_value)
{
}

void ItemProperties::MenuNodeTrackMode::execute()
{
    p_this->m_tracking_mode = m_source;
    cfg_selection_properties_tracking_mode = m_source;
    p_this->on_tracking_mode_change();
}

bool ItemProperties::MenuNodeTrackMode::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ItemProperties::MenuNodeTrackMode::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = get_name(m_source);
    p_displayflags = (m_source == p_this->m_tracking_mode) ? state_radiochecked : 0;
    return true;
}

const char* ItemProperties::MenuNodeTrackMode::get_name(uint32_t source)
{
    if (source == track_nowplaying)
        return "Playing item";
    if (source == track_selection)
        return "Current selection";
    if (source == track_automatic)
        return "Automatic";
    return "";
}

void ItemPropertiesColoursClient::on_colour_changed(uint32_t mask) const
{
    ItemProperties::s_redraw_all();
}

void ItemPropertiesColoursClient::on_bool_changed(uint32_t mask) const
{
    if (mask & colours::bool_flag_dark_mode_enabled)
        ItemProperties::s_on_dark_mode_status_change();
}

} // namespace cui::panels::item_properties
