#pragma once

/*!
 * \file get_msg_hook.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Class used for hooking message loops
 */

class get_msg_hook_t : public message_hook_manager::message_hook
{
	bool on_hooked_message(message_hook_manager::t_message_hook_type p_type, int code, WPARAM wp, LPARAM lp) override;
public:
	void register_hook();
	void deregister_hook();
};

