#pragma once

namespace cui::helpers {

class FullFileInfoRequest : public std::enable_shared_from_this<FullFileInfoRequest> {
public:
    using CallbackFunction = void(std::shared_ptr<FullFileInfoRequest>);
    FullFileInfoRequest(metadb_handle_ptr track, std::function<CallbackFunction> callback)
        : m_track(std::move(track)), m_callback(std::move(callback))
    {
    }
    void queue();
    void abort();
    bool is_ready() const;
    void wait() const;
    std::shared_ptr<file_info_impl> get();
    std::shared_ptr<file_info_impl> get_safe(const char* requester_name);

private:
    std::shared_ptr<file_info_impl> run();
    std::shared_ptr<file_info_impl> read_info();

    metadb_handle_ptr m_track;
    std::function<CallbackFunction> m_callback;
    std::future<std::shared_ptr<file_info_impl>> m_future;
    abort_callback_impl m_aborter;
};

} // namespace cui::helpers
