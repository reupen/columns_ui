#pragma once

namespace cui::artwork_panel {

using OnArtworkLoadedCallback = std::function<void(bool)>;

struct ArtworkReaderArgs {
    bool read_stub_image{};
    metadb_handle_ptr track;
    std::shared_ptr<class ArtworkReaderManager> manager;
};

enum class ArtworkReaderStatus {
    Pending,
    Succeeded,
    Failed,
    Aborted,
};

class ArtworkReader {
public:
    bool is_aborting() const;
    bool is_from_playback() const { return m_is_from_playback; }
    void abort();

    ArtworkReaderStatus status() const;
    const std::unordered_map<GUID, album_art_data_ptr>& get_artwork_data() const;
    const std::unordered_map<GUID, album_art_data_ptr>& get_stub_images() const;
    void set_image(GUID artwork_type_id, album_art_data_ptr data);

    ArtworkReader(std::vector<GUID> artwork_type_ids,
        std::unordered_map<GUID, album_art_data_ptr> previous_artwork_data, bool is_from_playback,
        OnArtworkLoadedCallback on_artwork_loaded)
        : m_artwork_type_ids(std::move(artwork_type_ids))
        , m_previous_artwork_data{std::move(previous_artwork_data)}
        , m_is_from_playback(is_from_playback)
        , m_on_artwork_loaded(std::move(on_artwork_loaded))
    {
    }

    ~ArtworkReader()
    {
        if (m_thread) {
            m_abort.abort();
            m_thread.reset();
        }
    }

    void notify_panel(bool artwork_changed);

    void start(ArtworkReaderArgs args);
    void wait();
    bool is_running() const;

private:
    bool read_artwork(const ArtworkReaderArgs& args, abort_callback& p_abort);

    std::vector<GUID> m_artwork_type_ids;
    std::unordered_map<GUID, album_art_data_ptr> m_previous_artwork_data;
    std::unordered_map<GUID, album_art_data_ptr> m_artwork_data;
    std::unordered_map<GUID, album_art_data_ptr> m_stub_images;
    ArtworkReaderStatus m_status{ArtworkReaderStatus::Pending};
    bool m_is_from_playback{};
    OnArtworkLoadedCallback m_on_artwork_loaded;
    abort_callback_impl m_abort;
    std::optional<std::jthread> m_thread;
};

class ArtworkReaderManager : public std::enable_shared_from_this<ArtworkReaderManager> {
public:
    void set_types(std::vector<GUID> types);

    void request(
        const metadb_handle_ptr& handle, OnArtworkLoadedCallback on_artwork_loaded, bool is_from_playback = false);
    bool is_ready() const;
    ArtworkReaderStatus status() const;
    void reset();
    void abort_current_task();

    album_art_data_ptr get_image(const GUID& p_what) const;
    album_art_data_ptr get_stub_image(GUID artwork_type_id);

    void deinitialise();

    void on_reader_completion(bool artwork_changed, const ArtworkReader* ptr);

private:
    std::vector<std::shared_ptr<ArtworkReader>> m_aborting_readers;
    std::shared_ptr<ArtworkReader> m_current_reader;

    std::vector<GUID> m_artwork_type_ids;
    mutable std::unordered_map<GUID, album_art_data_ptr> m_previous_artwork_data;
    std::unordered_map<GUID, album_art_data_ptr> m_stub_images;
};

} // namespace cui::artwork_panel
