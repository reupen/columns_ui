#include "stdafx.h"
#include "playlist_switcher_title_formatting.h"
#include "title_formatting.h"

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
            m_total_duration = tracks().calc_total_duration();

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

pfc::string8 format_playlist_title(size_t index)
{
    auto playlist_api = playlist_manager_v3::get();
    auto playback_api = playback_control::get();

    pfc::string8 name;
    playlist_api->playlist_get_name(index, name);

    if (!cfg_playlist_switcher_use_tagz)
        return name;

    auto size = playlist_api->playlist_get_item_count(index);
    auto is_locked = playlist_api->playlist_lock_is_present(index);
    auto is_active = playlist_api->get_active_playlist() == index;
    auto is_playing = playback_api->is_playing() && playlist_api->get_playing_playlist() == index;

    LazyFieldCalculator lazy_fields{index};

    auto file_size_getter
        = [&lazy_fields]() { return std::string(mmh::FileSizeFormatter(lazy_fields.total_file_size())); };

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

    pfc::string8 title;
    service_ptr_t<titleformat_object> to_temp;
    titleformat_compiler::get()->compile_safe(to_temp, cfg_playlist_switcher_tagz);

    to_temp->run(&field_provider_hook, title, nullptr);

    return title;
}
} // namespace cui
