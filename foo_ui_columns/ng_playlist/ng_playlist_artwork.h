#pragma once

namespace cui::panels::playlist_view {
class BaseArtworkCompletionNotify {
public:
    using ptr_t = std::shared_ptr<BaseArtworkCompletionNotify>;

    virtual ~BaseArtworkCompletionNotify() = default;
    virtual void on_completion(const std::shared_ptr<class ArtworkReader>& p_reader) = 0;

private:
};

class ArtworkReader : public mmh::Thread {
public:
    bool is_aborting() { return m_abort.is_aborting(); }
    void abort() { m_abort.abort(); }

    // only called when thread closed
    bool did_succeed() { return m_succeeded; }
    bool is_ready() { return !is_thread_open(); }
    const std::unordered_map<GUID, wil::shared_hbitmap>& get_content() const { return m_bitmaps; }

    void initialise(const metadb_handle_ptr& p_handle, int cx, int cy, COLORREF cr_back, bool b_reflection,
        BaseArtworkCompletionNotify::ptr_t p_notify, std::shared_ptr<class ArtworkReaderManager> p_manager)
    {
        m_handle = p_handle;
        m_notify = std::move(p_notify);
        m_cx = cx;
        m_cy = cy;
        m_reflection = b_reflection;
        m_back = cr_back;
        m_manager = std::move(p_manager);
    }
    void send_completion_notification(const std::shared_ptr<ArtworkReader>& p_this)
    {
        if (m_notify) {
            m_notify->on_completion(p_this);
        }
    }

protected:
    DWORD on_thread() override;

private:
    unsigned read_artwork(abort_callback& p_abort);

    std::unordered_map<GUID, wil::shared_hbitmap> m_bitmaps;
    int m_cx{0};
    int m_cy{0};
    COLORREF m_back{RGB(255, 255, 255)};
    bool m_reflection{false};
    metadb_handle_ptr m_handle;
    BaseArtworkCompletionNotify::ptr_t m_notify;
    bool m_succeeded{false};
    abort_callback_impl m_abort;
    std::shared_ptr<class ArtworkReaderManager> m_manager;
};

class ArtworkReaderManager : public std::enable_shared_from_this<ArtworkReaderManager> {
public:
    void abort_task(size_t index)
    {
        {
            if (m_current_readers[index]->is_thread_open()) {
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
        COLORREF cr_back, bool b_reflection, BaseArtworkCompletionNotify::ptr_t p_notify);

    void flush_pending()
    {
        size_t count = m_current_readers.get_count();
        size_t count_pending = m_pending_readers.get_count();
        if (count < max_readers) {
            if (count_pending) {
                std::shared_ptr<ArtworkReader> p_reader = m_pending_readers[count_pending - 1];
                m_pending_readers.remove_by_idx(count_pending - 1);
                p_reader->set_priority(THREAD_PRIORITY_BELOW_NORMAL);
                p_reader->create_thread();
                m_current_readers.add_item(p_reader);
            }
        }
    }

    void initialise() {}

    void deinitialise()
    {
        m_pending_readers.remove_all();

        size_t i = m_aborting_readers.get_count();
        for (; i; i--) {
            m_aborting_readers[i - 1]->wait_for_and_release_thread();
            m_aborting_readers.remove_by_idx(i - 1);
        }
        i = m_current_readers.get_count();
        for (; i; i--) {
            m_current_readers[i - 1]->wait_for_and_release_thread();
            m_current_readers.remove_by_idx(i - 1);
        }

        {
            insync(m_nocover_sync);
            m_nocover_bitmap.reset();
        }
    }

    void on_reader_completion(const ArtworkReader* ptr);
    void on_reader_abort(const ArtworkReader* ptr);

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
