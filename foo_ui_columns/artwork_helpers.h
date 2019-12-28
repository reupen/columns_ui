#pragma once

namespace artwork_panel {

enum Fb2KArtworkMode {
    fb2k_artwork_disabled,
    fb2k_artwork_embedded,
    fb2k_artwork_embedded_and_external,
};

class ArtworkReader : public mmh::Thread {
public:
    bool is_aborting();
    void abort();

    // only called when thread closed
    bool did_succeed();
    const std::unordered_map<GUID, album_art_data_ptr>& get_content() const;
    const album_art_data_ptr& get_emptycover() const;

    ArtworkReader() = default;

    void initialise(const std::vector<GUID>& p_requestIds,
        const std::unordered_map<GUID, album_art_data_ptr>& p_content_previous,
        const std::unordered_map<GUID, pfc::list_t<pfc::string8>>& p_repositories, bool b_read_emptycover,
        t_size b_native_artwork_reader_mode, const metadb_handle_ptr& p_handle, const completion_notify_ptr& p_notify,
        std::shared_ptr<class ArtworkReaderManager> p_manager);
    void run_notification_thisthread(DWORD state);

protected:
    DWORD on_thread() override;

private:
    unsigned read_artwork(abort_callback& p_abort);
    bool isContentEqual(const std::unordered_map<GUID, album_art_data_ptr>& content1,
        const std::unordered_map<GUID, album_art_data_ptr>& content2);

    std::vector<GUID> m_requestIds;
    std::unordered_map<GUID, album_art_data_ptr> m_content;
    std::unordered_map<GUID, pfc::list_t<pfc::string8>> m_repositories;
    metadb_handle_ptr m_handle;
    completion_notify_ptr m_notify;
    bool m_succeeded{false};
    bool m_read_emptycover{true};
    album_art_data_ptr m_emptycover;
    t_size m_native_artwork_reader_mode{fb2k_artwork_embedded_and_external};
    abort_callback_impl m_abort;
    std::shared_ptr<class ArtworkReaderManager> m_manager;
};

class ArtworkReaderManager : public std::enable_shared_from_this<ArtworkReaderManager> {
public:
    void AddType(const GUID& p_what);
    void abort_current_task();
    void SetScript(const GUID& p_what, const pfc::list_t<pfc::string8>& script);

    void ResetRepository();

    void Reset();

    bool IsReady();

    //! Completion notify code is 1 when content has changed, 0 when content is the same as before the request (like,
    //! advanced to another track with the same album art data).
    void Request(const metadb_handle_ptr& p_handle, completion_notify_ptr p_notify = nullptr);

    bool Query(const GUID& p_what, album_art_data_ptr& p_data);

    bool QueryEmptyCover(album_art_data_ptr& p_data);

    void initialise();

    void deinitialise();

    void on_reader_completion(DWORD ret, const ArtworkReader* ptr);
    void on_reader_abort(const ArtworkReader* ptr);

private:
    bool find_aborting_reader(const ArtworkReader* ptr, t_size& index);
    pfc::list_t<std::shared_ptr<ArtworkReader>> m_aborting_readers;
    std::shared_ptr<ArtworkReader> m_current_reader;
    // album_art_manager_instance_ptr m_api;

    std::vector<GUID> m_requestIds;
    std::unordered_map<GUID, album_art_data_ptr> m_content;
    std::unordered_map<GUID, pfc::list_t<pfc::string8>> m_repositories;
    album_art_data_ptr m_emptycover;
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

bool g_get_album_art_extractor_interface(service_ptr_t<album_art_extractor>& out, const char* path);
album_art_extractor_instance_ptr g_get_album_art_extractor_instance(const char* path, abort_callback& p_abort);

}; // namespace artwork_panel
