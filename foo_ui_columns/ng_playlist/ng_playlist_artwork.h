#pragma once

namespace cui::panels::playlist_view {

using OnArtworkLoadedCallback = std::function<void(const class ArtworkReader* reader)>;

enum class ArtworkReaderStatus {
    Pending,
    Succeeded,
    Failed,
    Aborted,
};

struct ArtworkRenderingContext {
    using Ptr = std::shared_ptr<ArtworkRenderingContext>;

    wil::com_ptr<ID3D11Device> d3d_device;
    wil::com_ptr<ID2D1Factory1> d2d_factory;
    wil::com_ptr<ID2D1Device> d2d_device;
    wil::com_ptr<ID2D1DeviceContext> d2d_device_context;
    wil::com_ptr<IWICImagingFactory> wic_factory;
    std::unordered_map<std::wstring, wil::com_ptr<ID2D1ColorContext>> colour_contexts;

    static Ptr s_create(unsigned width, unsigned height);

    void enlarge(unsigned width, unsigned height) const;
};

class ArtworkReader {
public:
    using Ptr = std::shared_ptr<ArtworkReader>;

    ArtworkReader(metadb_handle_ptr track, HMONITOR monitor, int width, int height, bool show_reflection,
        OnArtworkLoadedCallback callback, std::shared_ptr<class ArtworkReaderManager> manager)
        : m_handle(std::move(track))
        , m_monitor(monitor)
        , m_width(width)
        , m_height(height)
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

    void start(ArtworkRenderingContext::Ptr context);

    void wait()
    {
        core_api::ensure_main_thread();
        m_thread.reset();
    }

private:
    static constexpr GUID artwork_type_id = album_art_ids::cover_front;

    album_art_data_ptr read_artwork(abort_callback& p_abort);
    void render_artwork(const ArtworkRenderingContext::Ptr& context, const std::wstring& display_profile_name,
        album_art_data_ptr data, abort_callback& p_abort);

    std::unordered_map<GUID, wil::shared_hbitmap> m_bitmaps;
    metadb_handle_ptr m_handle;
    HMONITOR m_monitor{};
    int m_width{};
    int m_height{};
    bool m_show_reflection{};
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

    void request(const metadb_handle_ptr& p_handle, HMONITOR monitor, int cx, int cy, bool b_reflection,
        OnArtworkLoadedCallback p_notify);

    void start_pending_tasks()
    {
        if (m_current_readers.size() >= max_readers || m_pending_readers.empty())
            return;

        auto reader = *m_pending_readers.rbegin();
        m_pending_readers.pop_back();

        auto context = get_d2d_device_context();
        reader->start(std::move(context));
        m_current_readers.emplace_back(std::move(reader));
    }

    void deinitialise()
    {
        m_pending_readers.clear();
        m_aborting_readers.clear();
        m_current_readers.clear();
        m_nocover_bitmap.reset();
    }

    void on_reader_done(ArtworkRenderingContext::Ptr context, const ArtworkReader* ptr);

    ArtworkReaderManager() = default;

    // Called from reader worker thread
    wil::shared_hbitmap request_nocover_image(const ArtworkRenderingContext::Ptr& context,
        const std::wstring& display_profile_name, int cx, int cy, bool b_reflection, abort_callback& p_abort);

    void flush_nocover()
    {
        std::scoped_lock lock(m_nocover_mutex);
        m_nocover_bitmap.reset();
    }

private:
    static constexpr size_t max_readers{4};

    std::shared_ptr<ArtworkRenderingContext> get_d2d_device_context();

    wil::com_ptr<ID2D1Factory1> m_d2d_factory;
    std::vector<std::shared_ptr<ArtworkRenderingContext>> m_rendering_contexts;

    std::vector<ArtworkReader::Ptr> m_aborting_readers;
    std::vector<ArtworkReader::Ptr> m_current_readers;
    std::vector<ArtworkReader::Ptr> m_pending_readers;

    std::mutex m_nocover_mutex;
    wil::shared_hbitmap m_nocover_bitmap;
    size_t m_nocover_cx{};
    size_t m_nocover_cy{};
};

bool get_default_artwork_placeholder_data(album_art_data_ptr& p_out, abort_callback& p_abort);

} // namespace cui::panels::playlist_view
