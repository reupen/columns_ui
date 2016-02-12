#include "stdafx.h"

/** Creates toggle buttons from standard config objects */
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



/**Defines icons for buttons */
template <const GUID & MenutItemID, INT IconID>
class button_menu_item_with_bitmap : public uie::button_v2
{
	virtual const GUID & get_item_guid()const
	{
		return MenutItemID;
	}

	virtual HANDLE get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, unsigned cx_hint, unsigned cy_hint, unsigned & handle_type) const
	{
		HICON icon = (HICON)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IconID), IMAGE_ICON, cx_hint, cy_hint, NULL);
		handle_type = uie::button_v2::handle_type_icon;
		return (HANDLE)icon;
	}
};


uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_stop, IDI_STOP> > g_button_stop;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_open, IDI_OPEN> > g_button_open;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_play, IDI_PLAY> > g_button_play;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_pause, IDI_PAUSE> > g_button_pause;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_next, IDI_NEXT> > g_button_next;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_previous, IDI_PREV> > g_button_previous;
uie::button_factory< button_menu_item_with_bitmap<standard_commands::guid_main_random, IDI_RAND> > g_button_random;


class button_blank : public ui_extension::custom_button
{
	virtual const GUID & get_item_guid ()const
	{
		// {A8FE61BA-A055-4a53-A588-9DDA92ED7312}
		static const GUID guid = 
		{ 0xa8fe61ba, 0xa055, 0x4a53, { 0xa5, 0x88, 0x9d, 0xda, 0x92, 0xed, 0x73, 0x12 } };
		return guid;
	}
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
	{
		COLORMAP map;
		map.from = 0x0;
		map.to = cr_btntext;

		p_mask_type = ui_extension::MASK_BITMAP;
		HBITMAP bm = LoadMonoBitmap(IDB_BLANK, cr_btntext);
		if (bm)
			bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_BLANK), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_MONOCHROME); 
		return bm;
	}
	virtual void get_name(pfc::string_base & p_out) const
	{
		p_out = "Blanking button";
	}
	virtual unsigned get_button_state() const
	{
		return NULL;
	}
	void execute(const pfc::list_base_const_t<metadb_handle_ptr> & p_items) {};
	virtual uie::t_button_guid get_guid_type() const
	{
		return uie::BUTTON_GUID_BUTTON;
	}

};

ui_extension::button_factory<button_blank> g_blank;


#if 0
/** Prototype stop button that was disabled when not playing. Was abandoned. */
class button_stop : public ui_extension::button//, play_callback
{
	virtual const GUID & get_item_guid()const
	{
		return standard_commands::guid_main_stop;
	}
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
	{

		//COLORMAP map;
		//map.from = 0x0;
		//map.to = cr_btntext;

		HBITMAP bm = NULL;

		DLLVERSIONINFO2 dvi;
		if (SUCCEEDED(uih::GetComCtl32Version(dvi)) && dvi.info1.dwMajorVersion >= 6)
		{
			p_mask_type = uie::MASK_NONE;
			bm = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP32), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
		}
		else
		{
			p_mask_type = ui_extension::MASK_BITMAP;
			bm = //CreateMappedBitmap(core_api::get_my_instance(), IDB_STOP, 0, &map, 1);
				//(HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
				LoadMonoBitmap(IDB_STOP, cr_btntext);
			if (bm)
				bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_STOP), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		}

		return bm;
	}
#if 0
	virtual void register_callback(uie::button_callback & p_callback)
	{
		m_callback = &p_callback;
		static_api_ptr_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_all, false);
	};
	virtual void deregister_callback(uie::button_callback & p_callback)
	{
		assert(m_callback == &p_callback);
		m_callback = NULL;
		static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
	};
	virtual void FB2KAPI on_playback_starting(play_control::t_track_command p_command, bool p_paused){};
	virtual void FB2KAPI on_playback_new_track(metadb_handle_ptr p_track)
	{
		if (m_callback) m_callback->on_button_state_change(uie::BUTTON_STATE_DEFAULT);
	};
	virtual void FB2KAPI on_playback_stop(play_control::t_stop_reason p_reason)
	{
		if (m_callback) m_callback->on_button_state_change(NULL);
	};
	virtual void FB2KAPI on_playback_seek(double p_time){};
	virtual void FB2KAPI on_playback_pause(bool p_state){};
	virtual void FB2KAPI on_playback_edited(metadb_handle_ptr p_track){};
	virtual void FB2KAPI on_playback_dynamic_info(const file_info & p_info){};
	virtual void FB2KAPI on_playback_dynamic_info_track(const file_info & p_info){};
	virtual void FB2KAPI on_playback_time(double p_time){};
	virtual void FB2KAPI on_volume_change(float p_new_val){};
	uie::button_callback * m_callback;
public:
	button_stop() : m_callback(NULL) {};
#endif
};

ui_extension::button_factory<button_stop> g_stop;
#endif


#if 0
/** Prototype next button with drop-down list for next 10 tracks. Was abandoned. */
class button_next : public ui_extension::button
{
	virtual const GUID & get_item_guid()const
	{
		return standard_commands::guid_main_next;
	}
	virtual HBITMAP get_item_bitmap(unsigned command_state_index, COLORREF cr_btntext, uie::t_mask & p_mask_type, COLORREF & cr_mask, HBITMAP & bm_mask) const
	{
		COLORMAP map;
		map.from = 0x0;
		map.to = cr_btntext;

		HBITMAP bm = NULL;

		DLLVERSIONINFO2 dvi;
		if (SUCCEEDED(uih::GetComCtl32Version(dvi)) && dvi.info1.dwMajorVersion >= 6)
		{
			p_mask_type = uie::MASK_NONE;
			bm = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NEXT32), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
		}
		else
		{
			p_mask_type = ui_extension::MASK_BITMAP;
			bm = LoadMonoBitmap(IDB_NEXT, cr_btntext);//CreateMappedBitmap(core_api::get_my_instance(), IDB_NEXT, 0, &map, 1);
			if (bm)
				bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NEXT), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_MONOCHROME);
		}return bm;
	}
#if 0
	virtual void get_menu_items(uie::menu_hook_t & p_out)
	{
		class menu_node_play_item : public ui_extension::menu_node_leaf
		{
			unsigned m_index, m_playlist;
			service_ptr_t<playlist_manager> m_playlist_api;
			service_ptr_t<play_control> m_play_api;
		public:
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)
			{
				metadb_handle_ptr p_handle;
				m_playlist_api->playlist_get_item_handle(p_handle, m_playlist, m_index);
				if (p_handle.is_valid())
					p_handle->query_meta_field("TITLE", 0, p_out);
				p_displayflags = 0;
				return true;
			}
			virtual bool get_description(pfc::string_base & p_out)
			{
				return false;
			}
			virtual void execute()
			{
				m_playlist_api->playlist_set_playback_cursor(m_playlist, m_index);
				m_play_api->play_start(play_control::TRACK_COMMAND_SETTRACK);
			}
			menu_node_play_item(unsigned pl, unsigned p_index)
				: m_playlist(pl), m_index(p_index)
			{
				playlist_manager::g_get(m_playlist_api);
				play_control::g_get(m_play_api);
			};
		};
		unsigned cursor, playlist;
		static_api_ptr_t<playlist_manager> api;
		if (!api->get_playing_item_location(&playlist, &cursor))
		{
			playlist = api->get_active_playlist();
			cursor = api->playlist_get_playback_cursor(playlist);
		}
		if (config_object::g_get_data_bool_simple(standard_config_objects::bool_playback_follows_cursor, false))
		{
			cursor = api->activeplaylist_get_focus_item();
			playlist = api->get_active_playlist();
		}

		unsigned n, count = api->playlist_get_item_count(playlist);
		for (n = 0; n<10 && n + cursor<count; n++)
			p_out.add_node(uie::menu_node_ptr(new menu_node_play_item(playlist, cursor + n)));
	}
#endif
};

ui_extension::button_factory<button_next> g_next;
#endif
