#include "pch.h"
#include "playlist_switcher_title_formatting.h"

#include "metadb_helpers.h"
#include "tf_field_provider.h"
#include "tf_splitter_hook.h"
#include "tf_text_format.h"

namespace cui {

namespace {

class LazyFieldCalculator {
public:
    explicit LazyFieldCalculator(size_t playlist_index) : m_playlist_index(playlist_index) {}

    t_filesize total_file_size()
    {
        if (!m_total_file_size)
            m_total_file_size = metadb_handle_list_helper::calc_total_size(tracks(), true);

        return *m_total_file_size;
    }

    double total_duration()
    {
        if (!m_total_duration)
            m_total_duration = helpers::calculate_tracks_total_length(tracks());

        return *m_total_duration;
    }

private:
    const metadb_handle_list_t<pfc::alloc_fast_aggressive>& tracks()
    {
        if (!m_tracks) {
            m_tracks = metadb_handle_list_t<pfc::alloc_fast_aggressive>{};
            const auto size = m_playlist_api->playlist_get_item_count(m_playlist_index);
            m_tracks->prealloc(size);
            m_playlist_api->playlist_get_all_items(m_playlist_index, *m_tracks);
        }
        return *m_tracks;
    }

    size_t m_playlist_index{};
    std::optional<double> m_total_duration;
    std::optional<t_filesize> m_total_file_size;
    std::optional<metadb_handle_list_t<pfc::alloc_fast_aggressive>> m_tracks;
    const playlist_manager_v3::ptr m_playlist_api = playlist_manager_v3::get();
};

} // namespace

pfc::string8 format_playlist_title(
    size_t index, const titleformat_object::ptr& tf_object, std::optional<float> default_font_size)
{
    auto playlist_api = playlist_manager_v3::get();
    auto playback_api = playback_control::get();

    pfc::string8 name;
    playlist_api->playlist_get_name(index, name);

    if (!tf_object.is_valid())
        return name;

    auto size = playlist_api->playlist_get_item_count(index);
    auto is_locked = playlist_api->playlist_lock_is_present(index);
    auto is_active = playlist_api->get_active_playlist() == index;
    auto is_playing = playback_api->is_playing() && playlist_api->get_playing_playlist() == index;

    LazyFieldCalculator lazy_fields{index};

    auto file_size_getter = [&lazy_fields]() { return mmh::format_file_size(lazy_fields.total_file_size()); };

    auto file_size_raw_getter
        = [&lazy_fields]() { return std::string(pfc::format_uint(lazy_fields.total_file_size())); };

    auto length_getter = [&lazy_fields]() { return std::string(pfc::format_time_ex(lazy_fields.total_duration(), 0)); };

    tf::FieldProviderTitleformatHook::FieldMap field_map{{"title", name}, {"size", std::string(pfc::format_uint(size))},
        {"is_locked", is_locked}, {"is_active", is_active}, {"is_playing", is_playing}, {"filesize", file_size_getter},
        {"filesize_raw", file_size_raw_getter}, {"length", length_getter}};

    if (is_locked) {
        pfc::string8 lock_name;
        playlist_api->playlist_lock_query_name(index, lock_name);
        field_map["lock_name"sv] = std::string(lock_name);
    }

    tf::FieldProviderTitleformatHook field_provider_hook(field_map);
    tf::TextFormatTitleformatHook text_format_tf_hook(default_font_size.value_or(0.f));
    tf::SplitterTitleformatHook combined_hook(&field_provider_hook, &text_format_tf_hook);

    pfc::string8 title;
    tf_object->run(&combined_hook, title, nullptr);

    return title;
}

} // namespace cui
