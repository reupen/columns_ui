#include "foo_ui_columns.h"

#define __DEFINE_MENU_BUTTON(_namespace, _guid_command, _guid_config) \
	namespace _namespace { \
class menu_command_button_t : public uie::button \
{ \
	virtual const GUID & get_item_guid() const {return _guid_command;} \
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const {return NULL;} \
	virtual unsigned get_button_state() const \
	{ \
		return config_object::g_get_data_bool_simple(_guid_config,false) \
			? uie::BUTTON_STATE_PRESSED|uie::BUTTON_STATE_ENABLED : uie::BUTTON_STATE_DEFAULT; \
	} \
	virtual void register_callback(uie::button_callback & p_callback) {m_callbacks.add_item(&p_callback);}; \
	virtual void deregister_callback(uie::button_callback & p_callback) {m_callbacks.remove_item(&p_callback);}; \
	pfc::ptr_list_t<uie::button_callback> m_callbacks; \
	static pfc::ptr_list_t<menu_command_button_t> m_buttons; \
public: \
	static void g_on_state_change(bool b_enabled) \
	{ \
		t_size i, ic=m_buttons.get_count(), j, jc; \
		for (i=0; i<ic; i++) \
		{ \
			jc = m_buttons[i]->m_callbacks.get_count(); \
			for (j=0; j<jc; j++) \
			{ \
				m_buttons[i]->m_callbacks[j]->on_button_state_change(b_enabled ?uie::BUTTON_STATE_PRESSED|uie::BUTTON_STATE_ENABLED : uie::BUTTON_STATE_DEFAULT); \
			} \
		} \
	} \
	menu_command_button_t() {m_buttons.add_item(this);} \
	~menu_command_button_t() {m_buttons.remove_item(this);} \
}; \
	pfc::ptr_list_t<menu_command_button_t> menu_command_button_t::m_buttons; \
uie::button_factory<menu_command_button_t> g_menu_command_button;  \
class config_object_notify_impl : public config_object_notify \
{ \
public: \
	virtual t_size get_watched_object_count() {return 1;} \
	virtual GUID get_watched_object(t_size p_index) {return _guid_config;} \
	virtual void on_watched_object_changed(const service_ptr_t<config_object> & p_object)  \
	{ bool val = false; try {p_object->get_data_bool(val); menu_command_button_t::g_on_state_change(val); } catch (exception_io const &) {}; } \
}; \
	service_factory_t<config_object_notify_impl> g_config_object_notify_impl; \
}


__DEFINE_MENU_BUTTON(a, standard_commands::guid_main_stop_after_current, standard_config_objects::bool_playlist_stop_after_current);
__DEFINE_MENU_BUTTON(b, standard_commands::guid_main_playback_follows_cursor, standard_config_objects::bool_playback_follows_cursor);
__DEFINE_MENU_BUTTON(c, standard_commands::guid_main_cursor_follows_playback, standard_config_objects::bool_cursor_follows_playback);
__DEFINE_MENU_BUTTON(d, standard_commands::guid_main_always_on_top, standard_config_objects::bool_ui_always_on_top);

#if 0
class stop_after_current_button_t : public uie::button
{
	virtual const GUID & get_item_guid() const
	{
		return standard_commands::guid_main_stop_after_current;
	}
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
	{
		return NULL;
	}
	virtual unsigned get_button_state() const
	{
		return config_object::g_get_data_bool_simple(standard_config_objects::bool_playlist_stop_after_current,false)
			? uie::BUTTON_STATE_PRESSED|uie::BUTTON_STATE_ENABLED : uie::BUTTON_STATE_DEFAULT;
	}
	virtual void register_callback(uie::button_callback & p_callback)
	{
		m_callbacks.add_item(&p_callback);
	};
	virtual void deregister_callback(uie::button_callback & p_callback)
	{
		m_callbacks.remove_item(&p_callback);
	};
	pfc::ptr_list_t<uie::button_callback> m_callbacks;
	static pfc::ptr_list_t<live_layout_editing_button_t> m_buttons;
public:
	static void g_on_state_change(bool b_enabled)
	{
		t_size i, ic=m_buttons.get_count(), j, jc;
		for (i=0; i<ic; i++)
		{
			jc = m_buttons[i]->m_callbacks.get_count();
			for (j=0; j<jc; j++)
			{
				m_buttons[i]->m_callbacks[j]->on_button_state_change(b_enabled ?uie::BUTTON_STATE_PRESSED|uie::BUTTON_STATE_ENABLED : uie::BUTTON_STATE_DEFAULT);
			}
		}
	}
	stop_after_current_button_t() {m_buttons.add_item(this);}
	~stop_after_current_button_t() {m_buttons.remove_item(this);}
};

class stop_after_current_button_t : public uie::button
{
	virtual const GUID & get_item_guid() const
	{
		return standard_commands::guid_main_stop_after_current;
	}
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
	{
		return NULL;
	}
	virtual unsigned get_button_state() const
	{
		return config_object::g_get_data_bool_simple(standard_config_objects::bool_playlist_stop_after_current,false)
			? uie::BUTTON_STATE_PRESSED|uie::BUTTON_STATE_ENABLED : uie::BUTTON_STATE_DEFAULT;
	}
	virtual void register_callback(uie::button_callback & p_callback)
	{
		m_callbacks.add_item(&p_callback);
	};
	virtual void deregister_callback(uie::button_callback & p_callback)
	{
		m_callbacks.remove_item(&p_callback);
	};
	pfc::ptr_list_t<uie::button_callback> m_callbacks;
	static pfc::ptr_list_t<live_layout_editing_button_t> m_buttons;
public:
	static void g_on_state_change(bool b_enabled)
	{
		t_size i, ic=m_buttons.get_count(), j, jc;
		for (i=0; i<ic; i++)
		{
			jc = m_buttons[i]->m_callbacks.get_count();
			for (j=0; j<jc; j++)
			{
				m_buttons[i]->m_callbacks[j]->on_button_state_change(b_enabled ?uie::BUTTON_STATE_PRESSED|uie::BUTTON_STATE_ENABLED : uie::BUTTON_STATE_DEFAULT);
			}
		}
	}
	stop_after_current_button_t() {m_buttons.add_item(this);}
	~stop_after_current_button_t() {m_buttons.remove_item(this);}
};

namespace {
uie::button_factory<stop_after_current_button_t> g_stop_after_current_button_t;
};
#endif