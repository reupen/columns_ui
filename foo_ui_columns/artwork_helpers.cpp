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
    const std::unordered_map<GUID, album_art_data_ptr>& p_content_previous,
    const std::unordered_map<GUID, pfc::list_t<pfc::string8>>& p_repositories, bool b_read_emptycover,
    t_size b_native_artwork_reader_mode, const metadb_handle_ptr& p_handle, const completion_notify_ptr& p_notify,
    std::shared_ptr<class ArtworkReaderManager> p_manager)
{
    m_requestIds = p_requestIds;
    m_content = p_content_previous;
    m_repositories = p_repositories;
    m_read_emptycover = b_read_emptycover;
    m_handle = p_handle;
    m_notify = p_notify;
    m_manager = std::move(p_manager);
    m_native_artwork_reader_mode = b_native_artwork_reader_mode;
}

const album_art_data_ptr& artwork_panel::ArtworkReader::get_emptycover() const
{
    return m_emptycover;
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
    m_emptycover.reset();
}

void artwork_panel::ArtworkReaderManager::initialise() {}

bool artwork_panel::ArtworkReaderManager::QueryEmptyCover(album_art_data_ptr& p_data)
{
    if (IsReady() && m_emptycover.is_valid()) {
        p_data = m_emptycover;
    }
    return p_data.is_valid();
}

bool artwork_panel::ArtworkReaderManager::Query(const GUID& p_what, album_art_data_ptr& p_data)
{
    if (IsReady() && m_current_reader->did_succeed()) {
        auto&& content = m_current_reader->get_content();
        auto content_iter = content.find(p_what);
        if (content_iter != content.end()) {
            p_data = content_iter->second;
            return true;
        }
    }
    return false;
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
            b_prev_valid ? ptr_prev->get_content() : std::unordered_map<GUID, album_art_data_ptr>(), m_repositories,
            !m_emptycover.is_valid(), cfg_fb2k_artwork_mode, p_handle, p_notify, shared_from_this());
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
    m_emptycover.reset();
}

void artwork_panel::ArtworkReaderManager::ResetRepository()
{
    abort_current_task();
    m_repositories.clear();
    m_emptycover.reset();
}

void artwork_panel::ArtworkReaderManager::SetScript(const GUID& p_what, const pfc::list_t<pfc::string8>& script)
{
    abort_current_task();
    m_repositories.insert_or_assign(p_what, script);
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
        if (m_current_reader->get_emptycover().is_valid())
            m_emptycover = m_current_reader->get_emptycover();
        m_current_reader->run_notification_thisthread(state);
        // m_current_reader.release();
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
        formatter << "Album Art loading failure: " << e.what();
        ret = -1;
    }
    // send this first so thread gets closed first
    ArtworkReaderNotification::g_run(m_manager, b_aborted, ret, this);
    /*if (!b_aborted)
    {
    if (m_notify.is_valid())
    m_notify->on_completion_async(m_succeeded ? ret : 1);
    }*/
    return ret;
}

bool artwork_panel::g_get_album_art_extractor_interface(service_ptr_t<album_art_extractor>& out, const char* path)
{
    service_enum_t<album_art_extractor> e;
    album_art_extractor::ptr ptr;
    pfc::string_extension ext(path);
    while (e.next(ptr)) {
        if (ptr->is_our_path(path, ext)) {
            out = ptr;
            return true;
        }
    }
    return false;
}

album_art_extractor_instance_ptr artwork_panel::g_get_album_art_extractor_instance(
    const char* path, abort_callback& p_abort)
{
    album_art_extractor::ptr api;
    if (artwork_panel::g_get_album_art_extractor_interface(api, path)) {
        return api->open(nullptr, path, p_abort);
    }
    throw exception_album_art_not_found();
}
unsigned artwork_panel::ArtworkReader::read_artwork(abort_callback& p_abort)
{
    TRACK_CALL_TEXT("artwork_reader_v2_t::read_artwork");
    std::unordered_map<GUID, album_art_data_ptr> content_previous = m_content;
    m_content.clear();

    bool b_opened = false;
    bool b_extracter_attempted = false;
    album_art_extractor_instance_ptr p_extractor;
    album_art_extractor_instance_v2::ptr artwork_api_v2;
    static_api_ptr_t<album_art_manager_v2> p_album_art_manager_v2;

    for (auto&& artwork_id : m_requestIds) {
        bool b_found = false;
        try {
            auto repo_iter = m_repositories.find(artwork_id);
            if (repo_iter != m_repositories.end()) {
                auto& to = repo_iter->second;
                t_size count = to.get_count();
                for (t_size i = 0; i < count && !b_found; i++) {
                    pfc::string8 path;
                    if (m_handle->format_title_legacy(nullptr, path, to[i], nullptr)) {
                        const char* image_extensions[] = {"bmp", "gif", "jpeg", "jpg", "png", "webp"};

                        t_size i, count = tabsize(image_extensions);

                        bool b_absolute = path.find_first(':') != pfc_infinite || path.get_ptr()[0] == '\\';

                        pfc::string8 realPath, test;
                        if (b_absolute)
                            realPath = path;
                        else
                            realPath << pfc::string_directory(m_handle->get_path()) << "\\" << path;

                        bool b_search
                            = realPath.find_first('*') != pfc_infinite || realPath.find_first('?') != pfc_infinite;
                        bool b_search_matched = false;

                        if (b_search) {
                            const char* pMainPath = realPath;
                            if (!stricmp_utf8_partial(pMainPath, "file://"))
                                pMainPath += 7;

                            puFindFile pSearcher;
                            {
                                pfc::string_formatter formatter;
                                pSearcher = uFindFirstFile(formatter << pMainPath << ".*");
                            }

                            pfc::string8 searchPath = realPath;
                            realPath.reset();
                            if (pSearcher) {
                                do {
                                    const char* pResult = pSearcher->GetFileName();
                                    for (i = 0; i < count; i++) {
                                        if (!stricmp_utf8(pfc::string_extension(pResult), image_extensions[i])) {
                                            realPath << pfc::string_directory(searchPath) << "\\" << pResult;
                                            b_search_matched = true;
                                            break;
                                        }
                                    }
                                } while (!b_search_matched && pSearcher->FindNext());
                                delete pSearcher;
                            }
                        }

                        if (!b_search || b_search_matched) {
                            {
                                file::ptr file;
                                if (b_search) {
                                    pfc::string8 canPath;
                                    filesystem::g_get_canonical_path(realPath, canPath);
                                    if (!filesystem::g_is_remote_or_unrecognized(canPath))
                                        filesystem::g_open(file, canPath, filesystem::open_mode_read, p_abort);
                                } else {
                                    for (i = 0; i < count; i++) {
                                        pfc::string8 canPath;
                                        try {
                                            pfc::string_formatter formatter;
                                            filesystem::g_get_canonical_path(
                                                formatter << realPath << "." << image_extensions[i], canPath);

                                            if (!filesystem::g_is_remote_or_unrecognized(canPath)) {
                                                filesystem::g_open(file, canPath, filesystem::open_mode_read, p_abort);
                                                break;
                                            }
                                        } catch (exception_io const&) {
                                            // console::formatter() << ex.what();
                                        }
                                    }
                                }
                                if (file.is_valid()) {
                                    service_ptr_t<album_art_data_impl> ptr = new service_impl_t<album_art_data_impl>;
                                    ptr->from_stream(
                                        file.get_ptr(), gsl::narrow<t_size>(file->get_size_ex(p_abort)), p_abort);
                                    b_found = true;
                                    album_art_data_ptr data = ptr;
                                    m_content.insert_or_assign(artwork_id, data);
                                }
                            }
                        }
                    }
                }
            }
        } catch (const exception_aborted&) {
            throw;
        } catch (pfc::exception const&) {
        }

        if (!b_found && m_native_artwork_reader_mode == fb2k_artwork_embedded_and_external) {
            {
                if (!b_opened) {
                    pfc::list_t<GUID> guids;
                    guids.add_items_fromptr(m_requestIds.data(), m_requestIds.size());
                    artwork_api_v2 = p_album_art_manager_v2->open(
                        pfc::list_single_ref_t<metadb_handle_ptr>(m_handle), guids, p_abort);
                    b_opened = true;
                }
                try {
                    album_art_data_ptr data = artwork_api_v2->query(artwork_id, p_abort);
                    m_content.insert_or_assign(artwork_id, data);
                    b_found = true;
                } catch (const exception_aborted&) {
                    throw;
                } catch (exception_io_not_found const&) {
                } catch (exception_io const& e) {
                    console::formatter formatter;
                    formatter << "Requested Album Art entry could not be retrieved: " << e.what();
                }
            }
        } else if (!b_found && m_native_artwork_reader_mode == fb2k_artwork_embedded) {
            {
                try {
                    if (!b_extracter_attempted) {
                        b_extracter_attempted = true;
                        p_extractor = g_get_album_art_extractor_instance(m_handle->get_path(), p_abort);
                    }
                    if (p_extractor.is_valid()) {
                        album_art_data_ptr data = p_extractor->query(artwork_id, p_abort);
                        m_content.insert_or_assign(artwork_id, data);
                        b_found = true;
                    }
                } catch (const exception_aborted&) {
                    throw;
                } catch (exception_io_not_found const&) {
                } catch (exception_io const& e) {
                    console::formatter formatter;
                    formatter << "Requested Album Art entry could not be retrieved: " << e.what();
                }
            }
        }
    }

    if (m_read_emptycover) {
        try {
            auto p_extractor = p_album_art_manager_v2->open_stub(p_abort);
            // FIXME: We are always using the front no cover image
            m_emptycover = p_extractor->query(album_art_ids::cover_front, p_abort);
        } catch (const exception_aborted&) {
            throw;
        } catch (exception_io_not_found const&) {
        } catch (exception_io const& e) {
            console::formatter formatter;
            formatter << "Requested Album Art entry could not be retrieved: " << e.what();
        }
        if (!m_emptycover.is_valid() && pvt::g_get_default_nocover_bitmap_data(m_emptycover, p_abort)) {
        }
    }
    return isContentEqual(m_content, content_previous) ? 0 : 1;
}
