#pragma once

#include "playlist_manager_utils.h"
#include "playlist_switcher.h"
#include "list_view_panel.h"

class playlist_switcher_t : 
	public uie::container_ui_extension_t<t_list_view_panel<appearance_client_ps_impl>, uie::window>,
	private playlist_callback,
	private play_callback
{
	enum {ID_PLAY=1, ID_SWITCH, ID_REMOVE, ID_RENAME, ID_NEW, ID_SAVE, ID_SAVE_ALL, ID_LOAD, ID_UP, ID_DOWN, ID_CUT, ID_COPY, ID_PASTE, ID_AUTOPLAYLIST, ID_RECYCLER_CLEAR , ID_RECYCLER_BASE};

	enum {TIMER_SWITCH = TIMER_BASE};

	class IDropSource_t : public IDropSource
	{
		long refcount;
		service_ptr_t<playlist_switcher_t> m_window;
		DWORD m_initial_key_state;
	public:
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void ** ppvObject);
		ULONG STDMETHODCALLTYPE AddRef();
		ULONG STDMETHODCALLTYPE Release();
		HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState);
		HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);
		IDropSource_t(playlist_switcher_t * p_window, DWORD initial_key_state);
	};

	class IDropTarget_t : public IDropTarget
	{
	public:
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR *ppvObject);
		ULONG STDMETHODCALLTYPE   AddRef();
		ULONG STDMETHODCALLTYPE   Release();
		HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
		HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
		HRESULT STDMETHODCALLTYPE DragLeave( void);
		HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

		IDropTarget_t(playlist_switcher_t * p_window);
	private:
		long drop_ref_count;
		bool m_last_rmb;
		bool m_is_playlists;
		bool m_is_accepted_type;
		service_ptr_t<playlist_switcher_t> m_window;
		pfc::com_ptr_t<IDataObject> m_DataObject;
		service_ptr_t<ole_interaction_v2> m_ole_api;
		service_ptr_t<playlist_manager_v4> m_playlist_api;
		mmh::comptr_t<IDropTargetHelper> m_DropTargetHelper;
	};

public:
	static void g_on_font_items_change();
	static void g_on_edgestyle_change();
	static void g_on_vertical_item_padding_change();
	static void g_redraw_all();
	static void g_refresh_all_items();

	void destroy_switch_timer()
	{
		if (m_switch_timer_active)
		{
			KillTimer(get_wnd(), TIMER_SWITCH);
			m_switch_timer_active = false;
			m_switch_playlist.release();
		}
	}
	void set_switch_timer (t_size index)
	{
		if (!m_switch_timer_active || !m_switch_playlist.is_valid() || m_switch_playlist->m_playlist != index)
		{
			if (index != m_playlist_api->get_active_playlist())
			{
				destroy_switch_timer();
				m_switch_playlist = pfc::rcnew_t<playlist_position_reference_tracker>(false);
				m_switch_playlist->m_playlist = index;
				SetTimer(get_wnd(), TIMER_SWITCH, cfg_autoswitch_delay, NULL);
				m_switch_timer_active = true;
			}
		}
	}

	bool notify_on_timer(UINT_PTR timerid)
	{
		if (timerid == TIMER_SWITCH)
		{
			t_size index = m_switch_playlist.is_valid() ? m_switch_playlist->m_playlist : pfc_infinite;
			destroy_switch_timer();
			if (index != pfc_infinite)
				m_playlist_api->set_active_playlist(index);
			return true;
		}
		return false;
	}

	void get_insert_items (t_size base, t_size count, pfc::list_t<t_list_view::t_item_insert> & p_out);
	void refresh_all_items();
	void refresh_items (t_size base, t_size count, bool b_update = true);
	void add_items (t_size base, t_size count);
	void refresh_columns();

	void notify_on_initialisation();
	void notify_on_create();
	void notify_on_destroy();
	
	void move_selection (int delta);

	bool notify_before_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, bool b_source_mouse) ;
	bool notify_create_inline_edit(const pfc::list_base_const_t<t_size> & indices, unsigned column, pfc::string_base & p_text, t_size & p_flags, mmh::comptr_t<IUnknown> & pAutocompleteEntries) ;
	void notify_save_inline_edit(const char * value) ;

	virtual const char * get_drag_unit_singular() const override { return "playlist"; }
	virtual const char * get_drag_unit_plural() const override { return "playlists"; }

	bool do_drag_drop(WPARAM wp);

	bool notify_on_contextmenu(const POINT & pt);

	bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp, bool & b_processed)
	{
		uie::window_ptr p_this = this;
		bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
		b_processed = ret;
		return ret;
	};

	bool notify_on_middleclick(bool on_item, t_size index)
	{
		if (cfg_mclick && on_item && index < m_playlist_api->get_playlist_count())
		{
			m_playlist_api->remove_playlist_switch(index);
			return true;
		}
		return false;
	}

	bool notify_on_doubleleftclick_nowhere()
	{
		m_playlist_api->set_active_playlist(m_playlist_api->create_playlist("Untitled",pfc_infinite,m_playlist_api->get_playlist_count()));
		return true;
	}

	virtual bool notify_on_keyboard_keydown_remove()
	{
		t_size index = m_playlist_api->get_active_playlist();
		if (index < m_playlist_api->get_playlist_count())
		{
			pfc::string8 name;
			m_playlist_api->playlist_get_name(index, name);
			pfc::string_formatter formatter;
			if (uMessageBox(get_wnd(), formatter << "Are you sure you want to delete the \"" << name << "\" playlist?","Delete Playlist", MB_YESNO) == IDYES)
				m_playlist_api->remove_playlist_switch(index);
		}
		return true;
	}

	virtual bool notify_on_keyboard_keydown_cut()
	{
		t_size index = m_playlist_api->get_active_playlist();
		if (index != pfc_infinite) 
			playlist_manager_utils::cut(pfc::list_single_ref_t<t_size>(index));
		return true;
	}
	virtual bool notify_on_keyboard_keydown_copy()
	{
		t_size index = m_playlist_api->get_active_playlist();
		if (index != pfc_infinite) 
			playlist_manager_utils::copy(pfc::list_single_ref_t<t_size>(index));
		return true;
	}
	virtual bool notify_on_keyboard_keydown_paste()
	{
		t_size index = m_playlist_api->get_active_playlist();
		if (index == pfc_infinite) 
			index = m_playlist_api->get_playlist_count();
		else index++;
		playlist_manager_utils::paste(get_wnd(), index);
		return true;
	}

	void execute_default_action (t_size index, t_size column, bool b_keyboard, bool b_ctrl) 
	{
		if (m_playlist_api->playlist_get_item_count(index))
		{
			m_playlist_api->set_playing_playlist(index);
			static_api_ptr_t<play_control>()->start();
		}
	}
	void notify_on_selection_change(const bit_array & p_affected,const bit_array & p_status, notification_source_t p_notification_source)
	{
		if (p_notification_source != notification_source_rmb)
		{

			t_size numSelected = get_selection_count(2);

			if (numSelected == 1)
			{
				bit_array_bittable mask(get_item_count());
				get_selection_state(mask);
				t_size index=0, count=get_item_count();
				while (index < count)
				{
					if (mask[index]) break;
					index++;
				}
				m_playlist_api->set_active_playlist(index);
			}
			else// if (numSelected == 0)
				m_playlist_api->set_active_playlist(pfc_infinite);

		}
	}

	virtual void notify_on_set_focus(HWND wnd_lost)
	{
		m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
		m_selection_holder->set_playlist_tracking();
	}
	virtual void notify_on_kill_focus(HWND wnd_receiving)
	{
		m_selection_holder.release();
	}


	t_size get_playing_playlist()
	{
		return m_playback_api->is_playing() ? m_playlist_api->get_playing_playlist() : pfc_infinite;
	}

	void on_playing_playlist_change(unsigned p_playing_playlist)
	{
		t_size previous_playing = m_playing_playlist;
		m_playing_playlist = p_playing_playlist;
		if (previous_playing != pfc_infinite && previous_playing < get_item_count())
			refresh_items(previous_playing, 1);
		if (p_playing_playlist != previous_playing && p_playing_playlist != pfc_infinite && p_playing_playlist < get_item_count())
			refresh_items(p_playing_playlist, 1);
	}

	void refresh_playing_playlist()
	{
		m_playing_playlist = get_playing_playlist();
	}

	void on_items_added(t_size p_playlist,t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection);
	void on_items_reordered(t_size p_playlist,const t_size * p_order,t_size p_count) {};
	void on_items_removing(t_size p_playlist,const bit_array & p_mask,t_size p_old_count,t_size p_new_count){};
	void on_items_removed(t_size p_playlist,const bit_array & p_mask,t_size p_old_count,t_size p_new_count);
	void on_items_selection_change(t_size p_playlist,const bit_array & p_affected,const bit_array & p_state){}
	void on_item_focus_change(t_size p_playlist,t_size p_from,t_size p_to){}

	void on_items_modified(t_size p_playlist,const bit_array & p_mask);
	void on_items_modified_fromplayback(t_size p_playlist,const bit_array & p_mask,play_control::t_display_level p_level){};

	void on_items_replaced(t_size p_playlist,const bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data);

	void on_item_ensure_visible(t_size p_playlist,t_size p_idx) {};

	void on_playlist_activate(t_size p_old,t_size p_new) ;
	void on_playlist_created(t_size p_index,const char * p_name,t_size p_name_len) ;
	void on_playlists_reorder(const t_size * p_order,t_size p_count) ;
	void on_playlists_removing(const bit_array & p_mask,t_size p_old_count,t_size p_new_count) {};
	void on_playlists_removed(const bit_array & p_mask,t_size p_old_count,t_size p_new_count);
	void on_playlist_renamed(t_size p_index,const char * p_new_name,t_size p_new_name_len) ;

	void on_default_format_changed() {};
	void on_playback_order_changed(t_size p_new_index) {};
	void on_playlist_locked(t_size p_playlist,bool p_locked) ;


	void on_playback_starting(play_control::t_track_command p_command,bool p_paused) ;
	void on_playback_new_track(metadb_handle_ptr p_track) ;
	void on_playback_stop(play_control::t_stop_reason p_reason) ;
	void on_playback_seek(double p_time) {};
	void on_playback_pause(bool p_state) {};
	void on_playback_edited(metadb_handle_ptr p_track) {};
	void on_playback_dynamic_info(const file_info & p_info) {};
	void on_playback_dynamic_info_track(const file_info & p_info) {};
	void on_playback_time(double p_time) {};
	void on_volume_change(float p_new_val) {};


	const GUID & get_extension_guid() const
	{
		return cui::panels::guid_playlist_switcher;
	}
	void get_name(pfc::string_base & out)const
	{
		out = "Playlist switcher";
	}
	void get_category(pfc::string_base & out)const
	{
		out = "Panels";
	}
	bool get_short_name(pfc::string_base & out)const
	{
		out.set_string("Playlists");
		return true;
	}
	unsigned get_type () const
	{
		return uie::type_panel;
	}

	playlist_switcher_t() : m_playing_playlist(pfc_infinite), m_contextmenu_manager_base(NULL), 
		m_dragging(false), m_switch_timer_active(false) {};


private:
	contextmenu_manager::ptr m_contextmenu_manager;
	UINT_PTR m_contextmenu_manager_base;
	ui_status_text_override::ptr m_status_text_override;
	ui_selection_holder::ptr m_selection_holder;

	bool m_switch_timer_active;
	pfc::rcptr_t<playlist_position_reference_tracker> m_switch_playlist;

	bool m_dragging;
	pfc::com_ptr_t<IDataObject> m_DataObject;

	pfc::rcptr_t<playlist_position_reference_tracker> m_edit_playlist;
	t_size m_playing_playlist;
	service_ptr_t<playlist_manager_v3> m_playlist_api;
	service_ptr_t<playback_control> m_playback_api;

	static const GUID g_guid_font;
	static pfc::ptr_list_t<playlist_switcher_t> g_windows;
};
