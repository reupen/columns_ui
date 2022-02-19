#pragma once

class GetMsgHook : public uih::MessageHook {
    bool on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp) override;

public:
    void register_hook();
    void deregister_hook();
};
