#include "pch.h"
#include "ng_playlist/ng_playlist.h"
#include "artwork_helpers.h"

namespace cui::artwork_panel {

void ArtworkReader::notify_panel(bool artwork_changed)
{
    if (m_on_artwork_read)
        (*m_on_artwork_read)(artwork_changed);

    m_on_artwork_read.reset();
}

void ArtworkReader::initialise(const std::vector<GUID>& artwork_type_ids,
    const std::unordered_map<GUID, album_art_data_ptr>& p_content_previous, bool read_stub_image,
    const metadb_handle_ptr& p_handle, bool is_from_playback, OnArtworkReadCallback on_artwork_read,
    std::shared_ptr<class ArtworkReaderManager> p_manager)
{
    m_artwork_type_ids = artwork_type_ids;
    m_content = p_content_previous;
    m_read_stub_image = read_stub_image;
    m_handle = p_handle;
    m_is_from_playback = is_from_playback;
    m_on_artwork_read = std::move(on_artwork_read);
    m_manager = std::move(p_manager);
}

const std::unordered_map<GUID, album_art_data_ptr>& ArtworkReader::get_stub_images() const
{
    return m_stub_images;
}

void ArtworkReader::set_image(GUID artwork_type_id, album_art_data_ptr data)
{
    m_content.insert_or_assign(artwork_type_id, data);
}

const std::unordered_map<GUID, album_art_data_ptr>& ArtworkReader::get_content() const
{
    return m_content;
}

bool ArtworkReader::did_succeed()
{
    return m_succeeded;
}

void ArtworkReader::abort()
{
    m_abort.abort();
}

bool ArtworkReader::is_aborting()
{
    return m_abort.is_aborting();
}

void ArtworkReaderManager::deinitialise()
{
    for (auto iter = m_aborting_readers.begin(); iter != m_aborting_readers.end();) {
        (*iter)->wait_for_and_release_thread();
        iter = m_aborting_readers.erase(iter);
    }
    if (m_current_reader) {
        m_current_reader->wait_for_and_release_thread();
        m_current_reader.reset();
    }
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

album_art_data_ptr ArtworkReaderManager::get_image(const GUID& p_what)
{
    if (!is_ready() || !m_current_reader->did_succeed())
        return {};

    if (p_what == album_art_ids::cover_front && m_current_reader && m_current_reader->is_from_playback()) {
        const auto api = now_playing_album_art_notify_manager::get();

        if (auto data = api->current(); data.is_valid()) {
            // Inject the artwork data into the reader, so that the check whether the artwork
            // has changed when the next track is played (or selected) functions correctly
            m_current_reader->set_image(album_art_ids::cover_front, data);
            return data;
        }
    }

    auto&& content = m_current_reader->get_content();
    const auto content_iter = content.find(p_what);
    if (content_iter != content.end())
        return content_iter->second;

    return {};
}

void ArtworkReaderManager::request(
    const metadb_handle_ptr& handle, OnArtworkReadCallback on_artwork_read, const bool is_from_playback)
{
    std::shared_ptr<ArtworkReader> ptr_prev = m_current_reader;
    const bool is_prev_valid = ptr_prev && !ptr_prev->is_thread_open() && ptr_prev->did_succeed();
    const auto read_stub_images = m_stub_images.empty();
    abort_current_task();

    m_current_reader = std::make_shared<ArtworkReader>();
    m_current_reader->initialise(m_artwork_type_ids,
        is_prev_valid ? ptr_prev->get_content() : std::unordered_map<GUID, album_art_data_ptr>(), read_stub_images,
        handle, is_from_playback, on_artwork_read, shared_from_this());
    m_current_reader->set_priority(THREAD_PRIORITY_BELOW_NORMAL);
    m_current_reader->create_thread();
}

bool ArtworkReaderManager::is_ready()
{
    return m_current_reader && !m_current_reader->is_thread_open();
}

void ArtworkReaderManager::reset()
{
    abort_current_task();
    m_current_reader.reset();
    m_stub_images.clear();
}

void ArtworkReaderManager::abort_current_task()
{
    if (m_current_reader) {
        if (m_current_reader->is_thread_open()) {
            m_current_reader->abort();
            m_aborting_readers.emplace_back(m_current_reader);
            m_current_reader.reset();
        }
    }
}

void ArtworkReaderManager::set_types(std::vector<GUID> types)
{
    m_artwork_type_ids = types;
}

void ArtworkReaderManager::on_reader_completion(bool artwork_changed, const ArtworkReader* ptr)
{
    if (m_current_reader && ptr == &*m_current_reader) {
        m_current_reader->wait_for_and_release_thread();
        if (!m_current_reader->get_stub_images().empty())
            m_stub_images = m_current_reader->get_stub_images();
        m_current_reader->notify_panel(artwork_changed);
    } else {
        auto iter = ranges::find_if(m_aborting_readers, [ptr](auto&& reader) { return &*reader == ptr; });
        if (iter != ranges::end(m_aborting_readers)) {
            (*iter)->wait_for_and_release_thread();
            m_aborting_readers.erase(iter);
        }
    }
}

bool ArtworkReader::are_contents_equal(const std::unordered_map<GUID, album_art_data_ptr>& content1,
    const std::unordered_map<GUID, album_art_data_ptr>& content2)
{
    for (auto&& artwork_id : m_artwork_type_ids) {
        const auto content1_iter = content1.find(artwork_id);
        const auto content2_iter = content2.find(artwork_id);

        const bool found1 = content1_iter != content1.end();
        const bool found2 = content2_iter != content2.end();

        if (found1 != found2)
            return false;

        if (found1 && !album_art_data::equals(*content1_iter->second, *content2_iter->second))
            return false;
    }

    return true;
}

DWORD ArtworkReader::on_thread()
{
    TRACK_CALL_TEXT("artwork_panel::ArtworkReader::on_thread");

    bool artwork_changed{};

    try {
        artwork_changed = read_artwork(m_abort);
        m_abort.check();
        m_succeeded = true;
    } catch (const exception_aborted&) {
        m_content.clear();
        artwork_changed = false;
    } catch (const std::exception& e) {
        m_content.clear();
        console::print(u8"Artwork view – unhandled error reading artwork: "_pcc, e.what());
    }

    fb2k::inMainThread(
        [manager = m_manager, artwork_changed, this]() { m_manager->on_reader_completion(artwork_changed, this); });

    return 0;
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
        fbh::print_to_console(u8"Artwork view – error loading artwork: "_pcc, ex.what());
    }

    return {};
}

bool ArtworkReader::read_artwork(abort_callback& p_abort)
{
    TRACK_CALL_TEXT("artwork_reader_v2_t::read_artwork");
    std::unordered_map<GUID, album_art_data_ptr> content_previous = m_content;
    m_content.clear();

    const auto p_album_art_manager_v2 = album_art_manager_v2::get();
    pfc::list_t<GUID> guids;
    guids.add_items_fromptr(m_artwork_type_ids.data(), m_artwork_type_ids.size());

    const auto artwork_api_v2
        = p_album_art_manager_v2->open(pfc::list_single_ref_t<metadb_handle_ptr>(m_handle), guids, p_abort);

    for (auto&& artwork_id : m_artwork_type_ids) {
        const auto data = query_artwork_data(artwork_id, artwork_api_v2, p_abort);

        if (data.is_valid())
            m_content.insert_or_assign(artwork_id, data);
    }

    if (m_read_stub_image) {
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

    return !are_contents_equal(m_content, content_previous);
}

} // namespace cui::artwork_panel
