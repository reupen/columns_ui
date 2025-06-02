#include "pch.h"
#include "ng_playlist/ng_playlist.h"
#include "artwork_reader.h"

namespace cui::artwork_panel {

namespace {

bool are_artwork_data_maps_equal(
    const std::unordered_map<GUID, album_art_data_ptr>& left, const std::unordered_map<GUID, album_art_data_ptr>& right)
{
    if (left.size() != right.size())
        return false;

    for (auto&& [artwork_id, left_data] : left) {
        const auto right_iter = right.find(artwork_id);

        if (right_iter == right.end())
            return false;

        if (!album_art_data::equals(*left_data, *right_iter->second))
            return false;
    }

    return true;
}

} // namespace

void ArtworkReader::notify_panel(bool artwork_changed)
{
    if (m_on_artwork_loaded)
        m_on_artwork_loaded(artwork_changed);

    m_on_artwork_loaded = nullptr;
}

void ArtworkReader::start(ArtworkReaderArgs args)
{
    m_thread = std::jthread([this, args{std::move(args)}] {
        TRACK_CALL_TEXT("cui::artwork_panel::ArtworkReader::thread");
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

        bool artwork_changed{};

        try {
            artwork_changed = read_artwork(args, m_abort);
            m_abort.check();
            m_status = ArtworkReaderStatus::Succeeded;
        } catch (const exception_aborted&) {
            artwork_changed = false;
            m_status = ArtworkReaderStatus::Aborted;
        } catch (const std::exception& e) {
            console::print("Artwork view – unhandled error reading artwork: ", e.what());
            m_status = ArtworkReaderStatus::Failed;
        }

        if (m_status != ArtworkReaderStatus::Succeeded)
            m_artwork_data.clear();

        fb2k::inMainThread([manager = args.manager, artwork_changed, this]() {
            manager->on_reader_completion(artwork_changed, this);
        });
    });
}

void ArtworkReader::wait()
{
    m_thread.reset();
}

bool ArtworkReader::is_running() const
{
    return m_thread && m_thread->joinable();
}

const std::unordered_map<GUID, album_art_data_ptr>& ArtworkReader::get_stub_images() const
{
    return m_stub_images;
}

void ArtworkReader::set_image(GUID artwork_type_id, album_art_data_ptr data)
{
    m_artwork_data.insert_or_assign(artwork_type_id, data);
}

const std::unordered_map<GUID, album_art_data_ptr>& ArtworkReader::get_artwork_data() const
{
    return m_artwork_data;
}

ArtworkReaderStatus ArtworkReader::status() const
{
    core_api::ensure_main_thread();
    return m_status;
}

void ArtworkReader::abort()
{
    m_abort.abort();
}

bool ArtworkReader::is_aborting() const
{
    return m_abort.is_aborting();
}

void ArtworkReaderManager::deinitialise()
{
    for (auto iter = m_aborting_readers.begin(); iter != m_aborting_readers.end();)
        iter = m_aborting_readers.erase(iter);

    if (m_current_reader)
        m_current_reader.reset();

    m_stub_images.clear();
}

album_art_data_ptr ArtworkReaderManager::get_stub_image(GUID artwork_type_id)
{
    if (!is_ready())
        return {};

    if (const auto content_iter = m_stub_images.find(artwork_type_id); content_iter != m_stub_images.end()) {
        return content_iter->second;
    }

    if (artwork_type_id == album_art_ids::cover_front)
        return {};

    if (const auto content_iter = m_stub_images.find(album_art_ids::cover_front); content_iter != m_stub_images.end()) {
        return content_iter->second;
    }

    return {};
}

album_art_data_ptr ArtworkReaderManager::get_image(const GUID& p_what) const
{
    if (!is_ready() || m_current_reader->status() != ArtworkReaderStatus::Succeeded)
        return {};

    if (p_what == album_art_ids::cover_front && m_current_reader && m_current_reader->is_from_playback()) {
        const auto api = now_playing_album_art_notify_manager::get();

        if (auto data = api->current(); data.is_valid()) {
            // Inject the artwork data into m_previous_artwork_data, so that the check
            // whether the artwork has changed when the next track is played (or selected)
            // functions correctly
            m_previous_artwork_data.insert_or_assign(album_art_ids::cover_front, data);
            return data;
        }
    }

    auto&& artwork_data = m_current_reader->get_artwork_data();
    const auto content_iter = artwork_data.find(p_what);
    if (content_iter != artwork_data.end())
        return content_iter->second;

    return {};
}

void ArtworkReaderManager::request(
    const metadb_handle_ptr& handle, OnArtworkLoadedCallback on_artwork_loaded, const bool is_from_playback)
{
    const auto read_stub_images = m_stub_images.empty();
    abort_current_task();

    m_current_reader = std::make_shared<ArtworkReader>(
        m_artwork_type_ids, m_previous_artwork_data, is_from_playback, on_artwork_loaded);
    m_current_reader->start({read_stub_images, handle, shared_from_this()});
}

bool ArtworkReaderManager::is_ready() const
{
    return m_current_reader && !m_current_reader->is_running();
}

void ArtworkReaderManager::reset()
{
    abort_current_task();
    m_current_reader.reset();
    m_stub_images.clear();
}

void ArtworkReaderManager::abort_current_task()
{
    if (!m_current_reader || !m_current_reader->is_running())
        return;

    m_current_reader->abort();
    m_aborting_readers.emplace_back(m_current_reader);
    m_current_reader.reset();
}

void ArtworkReaderManager::set_types(std::vector<GUID> types)
{
    m_artwork_type_ids = std::move(types);
}

void ArtworkReaderManager::on_reader_completion(bool artwork_changed, const ArtworkReader* ptr)
{
    if (m_current_reader && ptr == &*m_current_reader) {
        m_current_reader->wait();
        m_previous_artwork_data = m_current_reader->get_artwork_data();

        if (!m_current_reader->get_stub_images().empty())
            m_stub_images = m_current_reader->get_stub_images();

        m_current_reader->notify_panel(artwork_changed);
    } else {
        const auto iter = ranges::find_if(m_aborting_readers, [ptr](auto&& reader) { return &*reader == ptr; });
        if (iter != ranges::end(m_aborting_readers))
            m_aborting_readers.erase(iter);
    }
}

album_art_data_ptr query_artwork_data(
    GUID artwork_type_id, album_art_extractor_instance_v2::ptr extractor, abort_callback& aborter)
{
    try {
        album_art_data_ptr data = extractor->query(artwork_type_id, aborter);

        if (data->get_size() > 0)
            return data;
    } catch (const exception_aborted&) {
        throw;
    } catch (exception_album_art_not_found const&) {
    } catch (exception_io const& ex) {
        fbh::print_to_console("Artwork view – error loading artwork: ", ex.what());
    }

    return {};
}

bool ArtworkReader::read_artwork(const ArtworkReaderArgs& args, abort_callback& p_abort)
{
    TRACK_CALL_TEXT("read_artwork");
    m_artwork_data.clear();

    const auto p_album_art_manager_v2 = album_art_manager_v2::get();
    pfc::list_t<GUID> guids;
    guids.add_items_fromptr(m_artwork_type_ids.data(), m_artwork_type_ids.size());

    const auto artwork_api_v2 = p_album_art_manager_v2->open(pfc::list_single_ref_t(args.track), guids, p_abort);

    for (auto&& artwork_id : m_artwork_type_ids) {
        const auto data = query_artwork_data(artwork_id, artwork_api_v2, p_abort);

        if (data.is_valid())
            m_artwork_data.insert_or_assign(artwork_id, data);
    }

    if (args.read_stub_image) {
        const auto stub_extractor = p_album_art_manager_v2->open_stub(p_abort);

        for (auto&& artwork_id : m_artwork_type_ids) {
            const auto data = query_artwork_data(artwork_id, stub_extractor, p_abort);

            if (data.is_valid())
                m_stub_images.insert_or_assign(artwork_id, data);
        }

        if (m_stub_images.find(album_art_ids::cover_front) == m_stub_images.end()) {
            album_art_data_ptr data;
            panels::playlist_view::g_get_default_nocover_bitmap_data(data, p_abort);

            if (data.is_valid())
                m_stub_images.insert_or_assign(album_art_ids::cover_front, data);
        }
    }

    return !are_artwork_data_maps_equal(m_artwork_data, m_previous_artwork_data);
}

} // namespace cui::artwork_panel
