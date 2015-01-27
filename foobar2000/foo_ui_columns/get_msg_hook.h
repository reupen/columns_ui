#pragma once

class get_msg_hook_t : public message_hook_manager::message_hook
{
	virtual bool on_hooked_message(message_hook_manager::t_message_hook_type p_type, int code, WPARAM wp, LPARAM lp);
public:
	void register_hook();
	void deregister_hook();
};

