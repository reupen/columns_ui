#include "pch.h"

#include "fb2k_callbacks.h"

namespace cui::fb2k_utils {

namespace {

class MetadbIoCallbackImpl : public metadb_io_callback_dynamic_impl_base {
public:
    explicit MetadbIoCallbackImpl(MetadbIoCallbackFunc callback) : m_callback(std::move(callback)) {}
    void on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook) noexcept override
    {
        m_callback(p_items_sorted, p_fromhook);
    }

private:
    MetadbIoCallbackFunc m_callback;
};

class MetadbIoCallbackToken : public EventToken {
public:
    explicit MetadbIoCallbackToken(MetadbIoCallbackFunc callback)
        : m_callback_wrapper(std::make_unique<MetadbIoCallbackImpl>(std::move(callback)))
    {
    }

private:
    std::unique_ptr<MetadbIoCallbackImpl> m_callback_wrapper;
};

} // namespace

EventToken::Ptr add_metadb_io_callback(MetadbIoCallbackFunc callback)
{
    return std::make_unique<MetadbIoCallbackToken>(std::move(callback));
}

} // namespace cui::fb2k_utils
