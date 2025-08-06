#pragma once

namespace cui {

using GenericEventHandler = std::function<void()>;

struct EventToken {
    using Ptr = std::unique_ptr<EventToken>;

    virtual ~EventToken() = default;
};

template <class Handler = GenericEventHandler>
struct GenericEventToken : EventToken {
    explicit GenericEventToken(
        std::vector<std::shared_ptr<Handler>>& all_handlers, std::shared_ptr<Handler> this_handler)
        : m_all_handlers(all_handlers)
        , m_this_handler(std::move(this_handler))
    {
    }
    ~GenericEventToken() override { std::erase(m_all_handlers, m_this_handler); }

private:
    std::vector<std::shared_ptr<Handler>>& m_all_handlers;
    std::shared_ptr<Handler> m_this_handler;
};

template <class Handler = GenericEventHandler>
EventToken::Ptr make_event_token(std::vector<std::shared_ptr<Handler>>& all_handlers, Handler new_handler)
{
    auto event_handler_ptr = std::make_shared<GenericEventHandler>(std::move(new_handler));
    all_handlers.emplace_back(event_handler_ptr);
    return std::make_unique<GenericEventToken<Handler>>(all_handlers, std::move(event_handler_ptr));
}

} // namespace cui
