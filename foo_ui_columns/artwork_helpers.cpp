#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"
#include "artwork.h"
#include "artwork_helpers.h"

void artwork_panel::ArtworkReader::run_notification_thisthread(DWORD state)
{
    if (m_notify.is_valid())
        m_notify->on_completion(state);
    m_notify.release();
}

void artwork_panel::ArtworkReader::initialise(const std::vector<GUID>& p_requestIds,
    const std::unordered_map<GUID, album_art_data_ptr>& p_content_previous, bool read_stub_image,
    const metadb_handle_ptr& p_handle, const completion_notify_ptr& p_notify,
    std::shared_ptr<class ArtworkReaderManager> p_manager)
{
    m_requestIds = p_requestIds;
    m_content = p_content_previous;
    m_read_stub_image = read_stub_image;
    m_handle = p_handle;
    m_notify = p_notify;
    m_manager = std::move(p_manager);
}

const std::unordered_map<GUID, album_art_data_ptr>& artwork_panel::ArtworkReader::get_stub_images() const
{
    return m_stub_images;
}

const std::unordered_map<GUID, album_art_data_ptr>& artwork_panel::ArtworkReader::get_content() const
{
    return m_content;
}

bool artwork_panel::ArtworkReader::did_succeed()
{
    return m_succeeded;
}

void artwork_panel::ArtworkReader::abort()
{
    m_abort.abort();
}

bool artwork_panel::ArtworkReader::is_aborting()
{
    return m_abort.is_aborting();
}

void artwork_panel::ArtworkReaderManager::deinitialise()
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

album_art_data_ptr artwork_panel::ArtworkReaderManager::get_stub_image(GUID artwork_type_id)
{
    if (!IsReady())
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

album_art_data_ptr artwork_panel::ArtworkReaderManager::get_image(const GUID& p_what)
{
    if (!IsReady() || !m_current_reader->did_succeed())
        return {};

    auto&& content = m_current_reader->get_content();
    const auto content_iter = content.find(p_what);
    if (content_iter != content.end())
        return content_iter->second;

    return {};
}

void artwork_panel::ArtworkReaderManager::Request(
    const metadb_handle_ptr& p_handle, completion_notify_ptr p_notify /*= NULL*/)
{
    std::shared_ptr<ArtworkReader> ptr_prev = m_current_reader;
    bool b_prev_valid = ptr_prev && !ptr_prev->is_thread_open() && ptr_prev->did_succeed();
    abort_current_task();
    {
        m_current_reader = std::make_shared<ArtworkReader>();
        m_current_reader->initialise(m_requestIds,
            b_prev_valid ? ptr_prev->get_content() : std::unordered_map<GUID, album_art_data_ptr>(),
            m_stub_images.empty(), p_handle, p_notify, shared_from_this());
        m_current_reader->set_priority(THREAD_PRIORITY_BELOW_NORMAL);
        m_current_reader->create_thread();
    }
}

bool artwork_panel::ArtworkReaderManager::IsReady()
{
    return m_current_reader && !m_current_reader->is_thread_open();
}

void artwork_panel::ArtworkReaderManager::Reset()
{
    abort_current_task();
    m_current_reader.reset();
    m_stub_images.clear();
}

void artwork_panel::ArtworkReaderManager::abort_current_task()
{
    if (m_current_reader) {
        if (m_current_reader->is_thread_open()) {
            m_current_reader->abort();
            m_aborting_readers.emplace_back(m_current_reader);
            m_current_reader.reset();
        }
    }
}

void artwork_panel::ArtworkReaderManager::AddType(const GUID& p_what)
{
    m_requestIds.emplace_back(p_what);
}

void artwork_panel::ArtworkReaderNotification::g_run(
    std::shared_ptr<ArtworkReaderManager> p_manager, bool p_aborted, DWORD ret, const ArtworkReader* p_reader)
{
    service_ptr_t<ArtworkReaderNotification> ptr = new service_impl_t<ArtworkReaderNotification>;
    ptr->m_aborted = p_aborted;
    ptr->m_reader = p_reader;
    ptr->m_manager = std::move(p_manager);
    ptr->m_ret = ret;

    static_api_ptr_t<main_thread_callback_manager>()->add_callback(ptr.get_ptr());
}

void artwork_panel::ArtworkReaderNotification::callback_run()
{
    if (m_aborted)
        m_manager->on_reader_abort(m_reader);
    else
        m_manager->on_reader_completion(m_ret, m_reader);
}

void artwork_panel::ArtworkReaderManager::on_reader_completion(DWORD state, const ArtworkReader* ptr)
{
    if (m_current_reader && ptr == &*m_current_reader) {
        m_current_reader->wait_for_and_release_thread();
        if (!m_current_reader->get_stub_images().empty())
            m_stub_images = m_current_reader->get_stub_images();
        m_current_reader->run_notification_thisthread(state);
    } else {
        auto iter = ranges::find_if(m_aborting_readers, [ptr](auto&& reader) { return &*reader == ptr; });
        if (iter != ranges::end(m_aborting_readers)) {
            (*iter)->wait_for_and_release_thread();
            m_aborting_readers.erase(iter);
        }
    }
}
void artwork_panel::ArtworkReaderManager::on_reader_abort(const ArtworkReader* ptr)
{
    on_reader_completion(ERROR_PROCESS_ABORTED, ptr);
}

bool artwork_panel::ArtworkReader::isContentEqual(const std::unordered_map<GUID, album_art_data_ptr>& content1,
    const std::unordered_map<GUID, album_art_data_ptr>& content2)
{
    for (auto&& artwork_id : m_requestIds) {
        const auto content1_iter = content1.find(artwork_id);
        const auto content2_iter = content2.find(artwork_id);

        const bool found1 = content1_iter != content1.end();
        const bool found2 = content2_iter != content2.end();

        if (found1 != found2)
            return false;

        if (!found1 || !album_art_data::equals(*content1_iter->second, *content2_iter->second))
            return false;
    }

    return true;
}

DWORD artwork_panel::ArtworkReader::on_thread()
{
    TRACK_CALL_TEXT("artwork_reader_v2_t::on_thread");

    bool b_aborted = false;
    DWORD ret = -1;
    try {
        ret = read_artwork(m_abort);
        m_abort.check();
        m_succeeded = true;
    } catch (const exception_aborted&) {
        m_content.clear();
        b_aborted = true;
        ret = ERROR_PROCESS_ABORTED;
    } catch (pfc::exception const& e) {
        m_content.clear();
        console::formatter formatter;
        formatter << u8"Artwork view – unhandled error reading artwork: " << e.what();
        ret = -1;
    }
    ArtworkReaderNotification::g_run(m_manager, b_aborted, ret, this);
    return ret;
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
        fbh::print_to_console(u8"Artwork view – error loading artwork: ", ex.what());
    }

    return {};
}

unsigned artwork_panel::ArtworkReader::read_artwork(abort_callback& p_abort)
{
    TRACK_CALL_TEXT("artwork_reader_v2_t::read_artwork");
    std::unordered_map<GUID, album_art_data_ptr> content_previous = m_content;
    m_content.clear();

    static_api_ptr_t<album_art_manager_v2> p_album_art_manager_v2;

    pfc::list_t<GUID> guids;
    guids.add_items_fromptr(m_requestIds.data(), m_requestIds.size());
    const auto artwork_api_v2
        = p_album_art_manager_v2->open(pfc::list_single_ref_t<metadb_handle_ptr>(m_handle), guids, p_abort);

    for (auto&& artwork_id : m_requestIds) {
        const auto data = query_artwork_data(artwork_id, artwork_api_v2, p_abort);

        if (data.is_valid())
            m_content.insert_or_assign(artwork_id, data);
    }

    if (m_read_stub_image) {
        const auto stub_extractor = p_album_art_manager_v2->open_stub(p_abort);

        for (auto&& artwork_id : m_requestIds) {
            const auto data = query_artwork_data(artwork_id, stub_extractor, p_abort);

            if (data.is_valid())
                m_stub_images.insert_or_assign(artwork_id, data);
        }

        if (m_stub_images.find(album_art_ids::cover_front) == m_stub_images.end()) {
            album_art_data_ptr data;
            pvt::g_get_default_nocover_bitmap_data(data, p_abort);

            if (data.is_valid())
                m_stub_images.insert_or_assign(album_art_ids::cover_front, data);
        }
    }
    return isContentEqual(m_content, content_previous) ? 0 : 1;
}
