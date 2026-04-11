#include "pch.h"

#include "playlist_search.h"

namespace cui::playlist_search {

fbh::ConfigInt32 cfg_search_bar_mode({0x8b8e6b11, 0x180a, 0x4f69, {0xa9, 0xf9, 0x9d, 0xf1, 0xb1, 0xd4, 0xc4, 0x22}}, 0);
fbh::ConfigString cfg_search_bar_script(
    {0xf9892211, 0xc021, 0x4a5b, {0x8c, 0x95, 0x2a, 0x0c, 0x12, 0xc0, 0x05, 0x3f}}, "[%artist%] [%title%] [%album%]");
fbh::ConfigBool cfg_search_bar_ignore_symbols(
    {0x8dbd88a1, 0x9ec2, 0x4eb0, {0xad, 0x8c, 0xa2, 0x79, 0xbd, 0x75, 0x20, 0x4b}}, true);

namespace {

auto split_into_words(std::wstring_view text)
{
    return text | ranges::views::split_when([](auto&& character) { return std::iswspace(character); })
        | ranges::views::filter([](auto&& word) { return !ranges::empty(word); })
        | ranges::views::transform([](auto&& word) {
              const auto size = ranges::distance(word);
              return size > 0 ? std::wstring_view(&*ranges::begin(word), size) : std::wstring_view{};
          });
}

bool match_string(
    std::wstring_view full_string, std::wstring_view partial_string, bool ignore_symbols, bool starts_with)
{
    const auto match_index = FindNLSStringEx(LOCALE_NAME_USER_DEFAULT,
        (starts_with ? FIND_STARTSWITH : FIND_FROMSTART) | LINGUISTIC_IGNOREDIACRITIC | NORM_IGNORECASE
            | NORM_IGNOREWIDTH | NORM_LINGUISTIC_CASING | (ignore_symbols ? NORM_IGNORESYMBOLS : 0),
        full_string.data(), gsl::narrow<int>(full_string.size()), partial_string.data(),
        gsl::narrow<int>(partial_string.size()), nullptr, nullptr, nullptr, 0);

    return match_index >= 0;
}

} // namespace

void PlaylistSearch::init()
{
    if (m_are_results_current)
        return;

    titleformat_compiler::get()->compile(m_titleformat_object, cfg_search_bar_script);

    m_playlist_index = m_playlist_api->get_active_playlist();
    const auto count = m_playlist_api->playlist_get_item_count(m_playlist_index);
    m_tracks.prealloc(count);
    m_playlist_api->playlist_get_all_items(m_playlist_index, m_tracks);
    m_matches.set_size(count);
    m_matches.fill(true);

    const auto mode = static_cast<SearchMode>(cfg_search_bar_mode.get());

    if (mode == SearchMode::mode_match_words_beginning_formatted_title
        || mode == SearchMode::mode_match_words_anywhere_formatted_title) {
        m_formatted.resize(count);

        const auto metadb_v2_api = metadb_v2::tryGet();

        if (fb2k::isLowMemModeActive() && metadb_v2_api.is_valid()) {
            metadb_v2_api->queryMultiParallelEx_<std::string>(
                m_tracks, [this](size_t index, const metadb_v2::rec_t& rec, auto&& buffer) {
                    metadb_handle_v2::ptr track;
                    track &= m_tracks[index];

                    mmh::StringAdaptor adapted_string(buffer);
                    track->formatTitle_v2(rec, nullptr, adapted_string, m_titleformat_object, nullptr);
                    m_formatted[index] = mmh::to_utf16(buffer);
                });
        } else {
            concurrency::parallel_for(size_t{}, count, [this](auto&& n) {
                thread_local std::string buffer;
                mmh::StringAdaptor adapted_string(buffer);
                m_tracks[n]->format_title(nullptr, adapted_string, m_titleformat_object, nullptr);
                m_formatted[n] = mmh::to_utf16(buffer);
            });
        }
    } else {
        m_formatted.clear();
    }

    if (!m_playlist_callback)
        m_playlist_callback.emplace([&] { mark_results_stale(); });

    m_are_results_current = true;
    m_last_mode = mode;
}

void PlaylistSearch::reset()
{
    m_are_results_current = false;
    m_match_index = 0;
    m_match_count = 0;
    m_matches.set_size(0);
    m_tracks.remove_all();
    m_search_terms.clear();
    m_formatted.clear();
    m_playlist_callback.reset();
}

void PlaylistSearch::on_return() const
{
    if (!m_are_results_current)
        return;

    const auto focus_index = fbh::as_optional(m_playlist_api->playlist_get_focus_item(m_playlist_index));

    if (!focus_index)
        return;

    const auto is_ctrl_down = (GetKeyState(VK_CONTROL) & KF_UP) != 0;

    if (is_ctrl_down) {
        metadb_handle_ptr track;
        m_playlist_api->queue_add_item_playlist(m_playlist_index, *focus_index);
    } else {
        m_playlist_api->playlist_execute_default_action(m_playlist_index, *focus_index);
    }
}

void PlaylistSearch::run()
{
    assert(m_matches.size() == m_playlist_api->activeplaylist_get_item_count());

    const auto mode = m_last_mode;
    auto clear_selection = mode == SearchMode::mode_query;

    m_match_count = 0;
    std::optional<std::string> query_error;

    if (mode == SearchMode::mode_query && !m_search_terms.empty()) {
        const auto filter_api = search_filter_manager_v2::get();
        const auto string_utf8 = mmh::to_utf8(m_search_terms);
        search_filter_v2::ptr filter;

        try {
            filter = filter_api->create_ex(string_utf8.c_str(), new service_impl_t<completion_notify_dummy>(),
                search_filter_manager_v2::KFlagSuppressNotify);
            clear_selection = false;
        } catch (const pfc::exception& ex) {
            query_error = ex.what();
        }

        if (filter.is_valid())
            filter->test_multi(m_tracks, m_matches.get_ptr());
    }

    if (clear_selection || m_search_terms.empty()) {
        m_playlist_api->playlist_clear_selection(m_playlist_index);

        if (query_error)
            dispatch_results_statistics_change(Status::QueryError, {}, {}, *query_error);
        else
            dispatch_results_statistics_change(Status::NoQuery);
        return;
    }

    const auto is_words_match = mode == SearchMode::mode_match_words_beginning_formatted_title
        || mode == SearchMode::mode_match_words_anywhere_formatted_title;

    const auto terms = is_words_match ? split_into_words(m_search_terms) | ranges::to<std::vector<std::wstring_view>>()
                                      : std::vector<std::wstring_view>{};

    if (mode == SearchMode::mode_match_words_beginning_formatted_title) {
        concurrency::parallel_for(size_t{}, m_formatted.size(), [this, &terms](auto&& index) {
            if (!m_matches[index])
                return;

            const auto& target_string = m_formatted[index];
            auto target_words = split_into_words(target_string);

            const auto all_terms_match = ranges::all_of(terms, [&](auto&& term) {
                return ranges::any_of(target_words, [&](auto&& target_word) {
                    return match_string(target_word, term, cfg_search_bar_ignore_symbols, true);
                });
            });

            if (!all_terms_match || terms.empty())
                m_matches[index] = false;
        });
    } else if (mode == SearchMode::mode_match_words_anywhere_formatted_title) {
        concurrency::parallel_for(size_t{}, m_formatted.size(), [this, &terms](auto&& index) {
            if (!m_matches[index])
                return;

            const auto& target_string = m_formatted[index];
            const auto all_terms_match = ranges::all_of(terms,
                [&](auto&& term) { return match_string(target_string, term, cfg_search_bar_ignore_symbols, false); });

            if (!all_terms_match || terms.empty())
                m_matches[index] = false;
        });
    }

    m_match_count = std::ranges::count(m_matches, true);

    m_playlist_api->playlist_set_selection(
        m_playlist_index, bit_array_true(), pfc::bit_array_lambda([&](auto index) { return m_matches[index]; }));

    if (m_match_count == 0) {
        dispatch_results_statistics_change(Status::NoMatches, 0, m_match_count);
        return;
    }

    const auto existing_focus = fbh::as_optional(m_playlist_api->playlist_get_focus_item(m_playlist_index)).value_or(0);
    const auto total_items = m_matches.size();

    const auto target_focus = [&] {
        if (m_matches[existing_focus])
            return existing_focus;

        auto matches_views = ranges::views::iota(size_t{1}, total_items) | ranges::views::transform([&](auto index) {
            return index < (total_items - existing_focus) ? existing_focus + index
                                                          : index - (total_items - existing_focus);
        }) | ranges::views::filter([&](auto index) { return m_matches[index]; });

        return matches_views.front();
    }();

    m_ensure_visible_func(target_focus);
    m_playlist_api->playlist_set_focus_item(m_playlist_index, target_focus);

    m_match_index = std::ranges::count(m_matches | std::views::take(target_focus), true);
    dispatch_results_statistics_change(Status::Matches, m_match_index + 1, m_match_count);
}

void PlaylistSearch::dispatch_results_statistics_change(Status status, std::optional<size_t> match_index,
    std::optional<size_t> match_count, std::string_view query_error) const
{
    m_results_statistics_change_func(status, match_index, match_count, query_error);
}

void PlaylistSearch::refresh()
{
    init();
    run();
}

void PlaylistSearch::mark_results_stale()
{
    if (!m_are_results_current)
        return;

    m_are_results_current = false;

    if (!m_search_terms.empty())
        dispatch_results_statistics_change(Status::Stale);
}

void PlaylistSearch::handle_next_or_previous(NavigationType navigation_type)
{
    if (m_search_terms.empty())
        return;

    if (!m_are_results_current) {
        refresh();
        return;
    }

    const auto total_items = m_matches.size();

    if (m_match_count == 0 || total_items <= 1)
        return;

    const auto focus = m_playlist_api->playlist_get_focus_item(m_playlist_index);
    auto matches_views = ranges::views::iota(size_t{1}, total_items) | ranges::views::transform([&](auto index) {
        if (navigation_type == NavigationType::Next)
            return index < (total_items - focus) ? focus + index : index - (total_items - focus);

        return index < focus ? focus - index : (total_items - index + focus - 1);
    });

    for (const auto index : matches_views) {
        if (!m_matches[index])
            continue;

        m_ensure_visible_func(index);

        if (!m_playlist_api->playlist_is_item_selected(m_playlist_index, index))
            m_playlist_api->playlist_set_selection(m_playlist_index, pfc::bit_array_true(), pfc::bit_array_one(index));

        m_playlist_api->playlist_set_focus_item(m_playlist_index, index);

        if (navigation_type == NavigationType::Next)
            m_match_index = m_match_index + 1 == m_match_count ? 0 : m_match_index + 1;
        else
            m_match_index = m_match_index == 0 ? m_match_count - 1 : m_match_index - 1;

        dispatch_results_statistics_change(Status::Matches, m_match_index + 1, m_match_count);
        break;
    }
}

} // namespace cui::playlist_search
