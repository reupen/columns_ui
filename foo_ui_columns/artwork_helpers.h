#pragma once

namespace cui::artwork_panel {

class ArtworkReader : public mmh::Thread {
public:
    bool is_aborting();
    bool is_from_playback() const { return m_is_from_playback; }
    void abort();

    // only called when thread closed
    bool did_succeed();
    const std::unordered_map<GUID, album_art_data_ptr>& get_content() const;
    const std::unordered_map<GUID, album_art_data_ptr>& get_stub_images() const;
    void set_image(GUID artwork_type_id, album_art_data_ptr data);

    ArtworkReader() = default;

    void initialise(const std::vector<GUID>& artwork_type_ids,
        const std::unordered_map<GUID, album_art_data_ptr>& p_content_previous, bool read_stub_image,
        const metadb_handle_ptr& p_handle, bool is_from_playback, const completion_notify_ptr& p_notify,
        std::shared_ptr<class ArtworkReaderManager> p_manager);
    void run_notification_thisthread(DWORD state);

protected:
    DWORD on_thread() override;

private:
    unsigned read_artwork(abort_callback& p_abort);
    bool are_contents_equal(const std::unordered_map<GUID, album_art_data_ptr>& content1,
        const std::unordered_map<GUID, album_art_data_ptr>& content2);

    std::vector<GUID> m_artwork_type_ids;
    std::unordered_map<GUID, album_art_data_ptr> m_content;
    std::unordered_map<GUID, album_art_data_ptr> m_stub_images;
    metadb_handle_ptr m_handle;
    completion_notify_ptr m_notify;
    bool m_succeeded{false};
    bool m_read_stub_image{true};
    bool m_is_from_playback{};
    abort_callback_impl m_abort;
    std::shared_ptr<class ArtworkReaderManager> m_manager;
};

class ArtworkReaderManager : public std::enable_shared_from_this<ArtworkReaderManager> {
public:
    void set_types(std::vector<GUID> types);

    void request(const metadb_handle_ptr& handle, completion_notify_ptr notify, bool is_from_playback = false);
    bool is_ready();
    void reset();
    void abort_current_task();

    album_art_data_ptr get_image(const GUID& p_what);
    album_art_data_ptr get_stub_image(GUID artwork_type_id);

    void deinitialise();

    void on_reader_completion(DWORD state, const ArtworkReader* ptr);
    void on_reader_abort(const ArtworkReader* ptr);

private:
    std::vector<std::shared_ptr<ArtworkReader>> m_aborting_readers;
    std::shared_ptr<ArtworkReader> m_current_reader;

    std::vector<GUID> m_artwork_type_ids;
    std::unordered_map<GUID, album_art_data_ptr> m_content;
    std::unordered_map<GUID, album_art_data_ptr> m_stub_images;
};

class ArtworkReaderNotification : public main_thread_callback {
public:
    void callback_run() override;

    static void g_run(
        std::shared_ptr<ArtworkReaderManager> p_manager, bool p_aborted, DWORD ret, const ArtworkReader* p_reader);

    bool m_aborted;
    DWORD m_ret;
    const ArtworkReader* m_reader;
    std::shared_ptr<ArtworkReaderManager> m_manager;
};

} // namespace cui::artwork_panel
