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
    void abort_task(size_t index)
    {
        {
            if (m_current_readers[index]->is_running()) {
                m_current_readers[index]->abort();
                m_aborting_readers.add_item(m_current_readers[index]);
            }
            m_current_readers.remove_by_idx(index);
        }
    }
    void abort_current_tasks()
    {
        m_pending_readers.remove_all();
        size_t i = m_current_readers.get_count();
        for (; i; i--)
            abort_task(i - 1);
    }

    void reset() { abort_current_tasks(); }

    enum {
        max_readers = 4
    };

    void request(const metadb_handle_ptr& p_handle, std::shared_ptr<ArtworkReader>& p_out, int cx, int cy,
        COLORREF cr_back, bool b_reflection, OnArtworkLoadedCallback p_notify);

    void flush_pending()
    {
        size_t count = m_current_readers.get_count();
        size_t count_pending = m_pending_readers.get_count();
        if (count < max_readers) {
            if (count_pending) {
                std::shared_ptr<ArtworkReader> p_reader = m_pending_readers[count_pending - 1];
                m_pending_readers.remove_by_idx(count_pending - 1);
                p_reader->start();
                m_current_readers.add_item(p_reader);
            }
        }
    }

    void deinitialise()
    {
        m_pending_readers.remove_all();

        size_t i = m_aborting_readers.get_count();
        for (; i; i--) {
            m_aborting_readers.remove_by_idx(i - 1);
        }
        i = m_current_readers.get_count();
        for (; i; i--) {
            m_current_readers.remove_by_idx(i - 1);
        }

        {
            insync(m_nocover_sync);
            m_nocover_bitmap.reset();
        }
    }

    void on_reader_done(const ArtworkReader* ptr);

    ArtworkReaderManager() = default;

    wil::shared_hbitmap request_nocover_image(
        int cx, int cy, COLORREF cr_back, bool b_reflection, abort_callback& p_abort);
    void flush_nocover() { m_nocover_bitmap.reset(); }

private:
    bool find_aborting_reader(const ArtworkReader* ptr, size_t& index)
    {
        size_t count = m_aborting_readers.get_count();
        for (size_t i = 0; i < count; i++)
            if (&*m_aborting_readers[i] == ptr) {
                index = i;
                return true;
            }
        return false;
    }
    bool find_current_reader(const ArtworkReader* ptr, size_t& index)
    {
        size_t count = m_current_readers.get_count();
        for (size_t i = 0; i < count; i++)
            if (&*m_current_readers[i] == ptr) {
                index = i;
                return true;
            }
        return false;
    }
    pfc::list_t<std::shared_ptr<ArtworkReader>> m_aborting_readers;
    pfc::list_t<std::shared_ptr<ArtworkReader>> m_current_readers;
    pfc::list_t<std::shared_ptr<ArtworkReader>> m_pending_readers;

    critical_section m_nocover_sync;
    wil::shared_hbitmap m_nocover_bitmap;
    size_t m_nocover_cx{0}, m_nocover_cy{0};
};

} // namespace cui::panels::playlist_view
