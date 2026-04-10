#pragma once
#include "fb2k_callbacks.h"

namespace cui::playlist_search {

extern fbh::ConfigInt32 cfg_search_bar_mode;
extern fbh::ConfigString cfg_search_bar_script;
extern fbh::ConfigBool cfg_search_bar_ignore_symbols;

enum class SearchMode : uint32_t {
    mode_match_words_beginning_formatted_title = 0,
    mode_match_words_anywhere_formatted_title = 1,
    mode_query = 2,
};

class PlaylistSearchPlaylistCallback : public playlist_callback_single_impl_base {
public:
    PlaylistSearchPlaylistCallback(std::function<void()> on_playlist_changed)
        : playlist_callback_single_impl_base(
              flag_on_items_added | flag_on_items_removed | flag_on_items_replaced | flag_on_playlist_switch)
        , m_on_playlist_changed(std::move(on_playlist_changed))
    {
        m_metadb_io_change_token
            = fb2k_utils::add_metadb_io_callback([this](metadb_handle_list_cref tracks, bool from_hook) {
                  if (from_hook)
                      return;

                  bool any_modified{};

                  playlist_manager::get()->activeplaylist_enum_items(
                      [&tracks, &any_modified](size_t index, const metadb_handle_ptr& track, bool _selected) {
                          size_t modified_index;
                          any_modified = tracks.bsearch_t(
                              pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, track, modified_index);

                          return any_modified;
                      },
                      bit_array_true());

                  if (any_modified)
                      m_on_playlist_changed();
              });
    }

protected:
    void on_items_added(t_size p_base, metadb_handle_list_cref p_data, const bit_array& p_selection) override
    {
        m_on_playlist_changed();
    }
    void on_items_removed(const bit_array& p_mask, t_size p_old_count, t_size p_new_count) override
    {
        m_on_playlist_changed();
    }
    void on_items_replaced(const bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override
    {
        m_on_playlist_changed();
    }
    void on_playlist_switch() override { m_on_playlist_changed(); }

private:
    mmh::EventToken::Ptr m_metadb_io_change_token;
    std::function<void()> m_on_playlist_changed;
};

enum Status {
    NoMatches,
    Matches,
    Stale,
    NoQuery,
    QueryError,
};

class PlaylistSearch {
public:
    using EnsureVisibleFunc = std::function<void(size_t index)>;
    using ResultsStatisticsChange = std::function<void(Status status, std::optional<size_t> match_index,
        std::optional<size_t> match_count, std::string_view query_error)>;

    static void s_mark_results_stale()
    {
        for (auto&& instance : s_instances)
            instance->mark_results_stale();
    }

    PlaylistSearch() { s_instances.push_back(this); }
    ~PlaylistSearch() { std::erase(s_instances, this); }

    void on_ensure_visible(EnsureVisibleFunc func) { m_ensure_visible_func = std::move(func); }
    void on_results_statistics_change(ResultsStatisticsChange func)
    {
        m_results_statistics_change_func = std::move(func);
    }

    void reset();

    void add_char(unsigned c)
    {
        m_string.push_back(c);
        refresh();
    }

    void set_string(std::wstring text)
    {
        if (m_running)
            m_matches.fill(true);

        m_string = std::move(text);
        refresh();
    }

    void on_previous() { handle_next_or_previous(NavigationType::Previous); }
    void on_next() { handle_next_or_previous(NavigationType::Next); }
    void on_return() const;

private:
    void init();
    void run();
    void refresh();
    bool refresh_if_stale();
    void mark_results_stale();

    void dispatch_results_statistics_change(Status status, std::optional<size_t> match_index = {},
        std::optional<size_t> match_count = {}, std::string_view query_error = {}) const;

    enum class NavigationType {
        Previous,
        Next,
    };

    void handle_next_or_previous(NavigationType navigation_type);

    inline static std::vector<PlaylistSearch*> s_instances;

    bool m_running{};
    metadb_handle_list_t<pfc::alloc_fast_aggressive> m_tracks;
    service_ptr_t<titleformat_object> m_titleformat_object;
    pfc::array_t<bool> m_matches;
    size_t m_match_index{};
    size_t m_match_count{};
    std::wstring m_string;
    static_api_ptr_t<playlist_manager> m_playlist_api;
    size_t m_playlist_index{};
    SearchMode m_last_mode{};
    std::vector<std::wstring> m_formatted;
    std::optional<PlaylistSearchPlaylistCallback> m_playlist_callback;
    bool m_are_results_stale{};
    EnsureVisibleFunc m_ensure_visible_func;
    ResultsStatisticsChange m_results_statistics_change_func;
};

} // namespace cui::playlist_search
