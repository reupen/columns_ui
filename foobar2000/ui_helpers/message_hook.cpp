#include "stdafx.h"

namespace message_hook_manager
{
	class hook_list
	{
	public:
		pfc::ptr_list_t<message_hook> m_hooks;
		HHOOK m_hook;
		hook_list() 
			: m_hook(NULL)
		{};
	};

	hook_list g_hooks[hook_type_count];

	LRESULT CALLBACK g_on_hooked_message(t_message_hook_type p_type, int code, WPARAM wp, LPARAM lp)
	{
		
		bool b_call_next = true;

		if (code >= 0)
		{
			t_size n, count = g_hooks[p_type].m_hooks.get_count();

			for (n=0; n<count; n++)
			{
				if ((p_type != type_get_message || wp==PM_REMOVE) && g_hooks[p_type].m_hooks[n]->on_hooked_message(p_type, code, wp, lp))
				{
					//if (p_type != type_get_message)
					//{
						b_call_next = false;
						break;
					//}
				}
			}
		}
		return b_call_next ? CallNextHookEx(NULL, code, wp, lp) : p_type != type_get_message ? TRUE : 0;
	}

	LRESULT CALLBACK g_keyboard_proc(int code, WPARAM wp, LPARAM lp)
	{
		return g_on_hooked_message(type_keyboard, code, wp, lp);
	}

	LRESULT CALLBACK g_getmsg_proc(int code, WPARAM wp, LPARAM lp)
	{
		return g_on_hooked_message(type_get_message, code, wp, lp);
	}

	LRESULT CALLBACK g_message_proc(int code, WPARAM wp, LPARAM lp)
	{
		return g_on_hooked_message(type_message_filter, code, wp, lp);
	}

	LRESULT CALLBACK g_mouse_proc(int code, WPARAM wp, LPARAM lp)
	{
		return g_on_hooked_message(type_mouse, code, wp, lp);
	}

	LRESULT CALLBACK g_mouse_low_level_proc(int code, WPARAM wp, LPARAM lp)
	{
		return g_on_hooked_message(type_mouse_low_level, code, wp, lp);
	}

	const HOOKPROC g_hook_procs[] = {g_keyboard_proc, g_getmsg_proc, g_message_proc, g_mouse_proc, g_mouse_low_level_proc};
	const int g_hook_ids[] = {WH_KEYBOARD, WH_GETMESSAGE, WH_MSGFILTER, WH_MOUSE, WH_MOUSE_LL};

	void register_hook(t_message_hook_type p_type, message_hook * p_hook)
	{
		if (g_hooks[p_type].m_hooks.add_item(p_hook) == 0)
		{
			g_hooks[p_type].m_hook = SetWindowsHookEx(g_hook_ids[p_type], g_hook_procs[p_type], p_type != type_mouse_low_level ? NULL : core_api::get_my_instance(), p_type != type_mouse_low_level ? GetCurrentThreadId() : NULL);
		}
	}
	void deregister_hook(t_message_hook_type p_type, message_hook * p_hook)
	{
		g_hooks[p_type].m_hooks.remove_item(p_hook);
		if (g_hooks[p_type].m_hooks.get_count() == 0)
		{
			UnhookWindowsHookEx(g_hooks[p_type].m_hook);
			g_hooks[p_type].m_hook = NULL;
		}
	}

};
