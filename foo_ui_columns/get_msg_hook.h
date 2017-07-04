#pragma once

/*!
 * \file get_msg_hook.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Class used for hooking message loops
 */

class get_msg_hook_t : public uih::MessageHook
{
    bool on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp) override;
public:
    void register_hook();
    void deregister_hook();
};

