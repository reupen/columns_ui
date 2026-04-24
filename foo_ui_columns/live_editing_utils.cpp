#include "pch.h"

#include "live_editing_utils.h"

namespace cui::layout {

namespace {

constexpr auto MSG_QUIT_THREAD = WM_APP;

std::vector<std::shared_ptr<LowLevelMouseHandler>> ll_mouse_handlers;
std::optional<std::jthread> ll_mouse_thread;
std::mutex ll_mouse_handlers_mutex;

class LowLevelMouseHookToken : public mmh::EventToken {
public:
    explicit LowLevelMouseHookToken(std::shared_ptr<LowLevelMouseHandler> this_handler)
        : m_this_handler(std::move(this_handler))
    {
    }

    ~LowLevelMouseHookToken() override
    {
        {
            std::scoped_lock _(ll_mouse_handlers_mutex);
            std::erase(ll_mouse_handlers, m_this_handler);

            if (!ll_mouse_handlers.empty())
                return;
        }

        if (!ll_mouse_thread)
            return;

        const auto thread_id = GetThreadId(ll_mouse_thread->native_handle());
        PostThreadMessage(thread_id, MSG_QUIT_THREAD, 0, 0);
        ll_mouse_thread.reset();
    }

private:
    std::shared_ptr<LowLevelMouseHandler> m_this_handler;
};

LRESULT CALLBACK handle_hooked_message(int code, WPARAM wp, LPARAM lp) noexcept
{
    if (code >= 0) {
        decltype(ll_mouse_handlers) handlers_copy;

        {
            std::scoped_lock _(ll_mouse_handlers_mutex);
            handlers_copy = ll_mouse_handlers;
        }

        for (const auto& handler : handlers_copy) {
            if ((*handler)(wp, *reinterpret_cast<LPMSLLHOOKSTRUCT>(lp)))
                return TRUE;
        }
    }

    return CallNextHookEx(nullptr, code, wp, lp);
}

} // namespace

mmh::EventToken::Ptr add_low_level_mouse_handler(LowLevelMouseHandler handler)
{
    auto event_handler_ptr = std::make_shared<LowLevelMouseHandler>(handler);

    {
        std::scoped_lock _(ll_mouse_handlers_mutex);
        ll_mouse_handlers.emplace_back(event_handler_ptr);
    }

    if (!ll_mouse_thread) {
        ll_mouse_thread.emplace([&]() {
            TRACK_CALL_TEXT("cui::layout::LLMouseHookThread");
            (void)mmh::set_thread_description(GetCurrentThread(), L"[Columns UI] Live editing mouse hook");

            const auto hook
                = SetWindowsHookEx(WH_MOUSE_LL, &handle_hooked_message, wil::GetModuleInstanceHandle(), NULL);
            MSG msg{};
            BOOL res{};

            while ((res = GetMessage(&msg, nullptr, 0, 0)) != 0) {
                if (res == -1)
                    uBugCheck();

                if (msg.hwnd == nullptr && msg.message == MSG_QUIT_THREAD) {
                    PostQuitMessage(0);
                } else {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            UnhookWindowsHookEx(hook);
        });
    }

    return std::make_unique<LowLevelMouseHookToken>(std::move(event_handler_ptr));
}

} // namespace cui::layout
