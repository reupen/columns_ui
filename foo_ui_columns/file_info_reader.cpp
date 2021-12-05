#include "stdafx.h"
#include "file_info_reader.h"

// See https://hydrogenaud.io/index.php/topic,101928.0.html

namespace cui::helpers {

std::shared_ptr<file_info_impl> FullFileInfoRequest::run()
{
    auto _ = gsl::finally([this] {
        fb2k::inMainThread([callback = m_callback, self = shared_from_this()]() mutable { callback(std::move(self)); });
    });
    return read_info();
}

std::shared_ptr<file_info_impl> FullFileInfoRequest::read_info()
{
    m_aborter.check();
    input_info_reader::ptr reader;
    input_entry::g_open_for_info_read(reader, nullptr, m_track->get_path(), m_aborter);
    auto info = std::make_shared<file_info_impl>();
    reader->get_info(m_track->get_subsong_index(), *info, m_aborter);
    return info;
}

void FullFileInfoRequest::queue()
{
    m_future = std::async(std::launch::async, [self = shared_from_this()] { return self->run(); });
}

void FullFileInfoRequest::abort()
{
    m_aborter.abort();
}

bool FullFileInfoRequest::is_ready() const
{
    return !m_future.valid() || m_future.wait_for(0ns) == std::future_status::ready;
}

void FullFileInfoRequest::wait() const
{
    if (m_future.valid())
        m_future.wait();
}

std::shared_ptr<file_info_impl> FullFileInfoRequest::get()
{
    return m_future.get();
}

std::shared_ptr<file_info_impl> FullFileInfoRequest::get_safe(const char* requester_name)
{
    console::formatter formatter;
    try {
        return get();
    } catch (const exception_aborted&) {
    } catch (const std::exception& ex) {
        pfc::string8 display_path;
        filesystem::g_get_display_path(m_track->get_path(), display_path);
        const char* error_message = ex.what();
        if (error_message == nullptr) {
            error_message = "Unknown error";
        }
        formatter << requester_name << ": Error reading file info for track \"" << display_path
                  << "\": " << error_message;
    } catch (...) {
        pfc::string8 display_path;
        filesystem::g_get_display_path(m_track->get_path(), display_path);
        formatter << requester_name << ": Unknown error reading file info for track \"" << display_path << "\"";
    }
    return nullptr;
}

} // namespace cui::helpers
