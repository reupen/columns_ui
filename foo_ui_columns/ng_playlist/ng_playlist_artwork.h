#pragma once

namespace cui::panels::playlist_view {

using OnArtworkLoadedCallback = std::function<void(const class ArtworkReader* reader)>;

enum class ArtworkReaderStatus {
    Pending,
    Succeeded,
    Failed,
    Aborted,
};

class ArtworkReader {
public:
    using Ptr = std::shared_ptr<ArtworkReader>;

    ArtworkReader(metadb_handle_ptr track, int width, int height, COLORREF background_colour, bool show_reflection,
        OnArtworkLoadedCallback callback, std::shared_ptr<class ArtworkReaderManager> manager)
        : m_handle(std::move(track))
        , m_width(width)
        , m_height(height)
        , m_background_colour(background_colour)
        , m_show_reflection(show_reflection)
        , m_callback(std::move(callback))
        , m_manager(std::move(manager))
    {
    }

    ~ArtworkReader()
    {
        if (m_thread) {
            m_abort.abort();
            m_thread.reset();
        }
    }

    bool is_running() const
    {
        core_api::ensure_main_thread();
        return m_thread && m_thread->joinable();
    }

    bool is_aborting() const
    {
        core_api::ensure_main_thread();
        return m_abort.is_aborting();
    }

    void abort()
    {
        core_api::ensure_main_thread();
        m_abort.abort();
    }

    ArtworkReaderStatus status() const
    {
        core_api::ensure_main_thread();
        return m_status;
    }

    const std::unordered_map<GUID, wil::shared_hbitmap>& get_content() const
    {
        core_api::ensure_main_thread();
        return m_bitmaps;
    }

    void send_completion_notification()
    {
        core_api::ensure_main_thread();
        m_thread.reset();
        m_callback(this);
    }

    void start();

    void wait()
    {
        core_api::ensure_main_thread();
        m_thread.reset();
    }

private:
    unsigned read_artwork(abort_callback& p_abort);

    std::unordered_map<GUID, wil::shared_hbitmap> m_bitmaps;
    metadb_handle_ptr m_handle;
    int m_width{0};
    int m_height{0};
    COLORREF m_background_colour{RGB(255, 255, 255)};
    bool m_show_reflection{false};
    OnArtworkLoadedCallback m_callback;
    std::shared_ptr<class ArtworkReaderManager> m_manager;
    abort_callback_impl m_abort;
    ArtworkReaderStatus m_status{ArtworkReaderStatus::Pending};
    std::optional<std::jthread> m_thread;
};

class ArtworkReaderManager : public std::enable_shared_from_this<ArtworkReaderManager> {
public:
    void abort_current_tasks()
    {
        m_pending_readers.clear();

        for (auto&& reader : m_current_readers) {
            if (!reader->is_running())
                continue;

            reader->abort();
            m_aborting_readers.emplace_back(std::move(reader));
        }

        m_current_readers.clear();
    }

    void reset() { abort_current_tasks(); }

    void request(const metadb_handle_ptr& p_handle, ArtworkReader::Ptr& p_out, int cx, int cy, COLORREF cr_back,
        bool b_reflection, OnArtworkLoadedCallback p_notify);

    void flush_pending()
    {
        if (m_current_readers.size() >= max_readers || m_pending_readers.empty())
            return;

        auto reader = *m_pending_readers.rbegin();
        m_pending_readers.pop_back();
        reader->start();
        m_current_readers.emplace_back(std::move(reader));
    }

    void deinitialise()
    {
        m_pending_readers.clear();
        m_aborting_readers.clear();
        m_current_readers.clear();
        m_nocover_bitmap.reset();
    }

    void on_reader_done(const ArtworkReader* ptr);

    ArtworkReaderManager() = default;

    // Called from reader worker thread
    wil::shared_hbitmap request_nocover_image(
        int cx, int cy, COLORREF cr_back, bool b_reflection, abort_callback& p_abort);

    void flush_nocover()
    {
        std::scoped_lock lock(m_nocover_mutex);
        m_nocover_bitmap.reset();
    }

private:
    static constexpr size_t max_readers{4};

    std::vector<ArtworkReader::Ptr> m_aborting_readers;
    std::vector<ArtworkReader::Ptr> m_current_readers;
    std::vector<ArtworkReader::Ptr> m_pending_readers;

    std::mutex m_nocover_mutex;
    wil::shared_hbitmap m_nocover_bitmap;
    size_t m_nocover_cx{};
    size_t m_nocover_cy{};
};

} // namespace cui::panels::playlist_view
