/**
* Columns UI foobar2000 component
* 
* \author musicmusic
*/

#include "stdafx.h"

#include "status_pane.h"
#include "layout.h"

#include "main_window.h"

rebar_window * g_rebar_window = nullptr;
layout_window g_layout_window;
cui::main_window_t cui::g_main_window;
user_interface::HookProc_t main_window::g_hookproc;
status_pane g_status_pane;
mmh::comptr_t<ITaskbarList3> main_window::g_ITaskbarList3;

HIMAGELIST  g_imagelist = nullptr;

HWND g_main_window=nullptr,
	g_tooltip=nullptr,
	g_rebar=nullptr,
	g_status=nullptr;

bool ui_initialising=false,
	g_minimised = false
	;

bool g_playing = false;

HICON g_icon=nullptr;

pfc::string8
	statusbartext,
	windowtext;

HFONT g_font=nullptr;

bool remember_window_pos()
{
	return config_object::g_get_data_bool_simple(standard_config_objects::bool_remember_window_positions,false);
}



void action_remove_track(bool on_item, unsigned idx)
{
	if (on_item) 
	{
		static_api_ptr_t<playlist_manager> api;
		api->activeplaylist_undo_backup();
		api->activeplaylist_remove_items(bit_array_one(idx));
	}
}

void action_add_to_queue(bool on_item, unsigned idx)
{
	if (on_item)
	{
		static_api_ptr_t<playlist_manager> api;
		unsigned active = api->get_active_playlist();
		if (active != -1)
			api->queue_add_item_playlist(active, idx);
	}
}

void action_none(bool on_on_item, unsigned idx)
{
}

pma playlist_mclick_actions::g_pma_actions[] =
{
	{"(None)",0,action_none},
	{"Remove track from playlist",1,action_remove_track},
	{"Add to playback queue",2,action_add_to_queue},
};

unsigned playlist_mclick_actions::get_count()
{
	return tabsize(g_pma_actions);
}

unsigned playlist_mclick_actions::id_to_idx(unsigned id)
{
	unsigned n, count = tabsize(g_pma_actions);
	for (n=0;n<count;n++)
	{
		if (g_pma_actions[n].id == id) return n;
	}
	return 0;
}

bool process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist, bool keyb)
{
	static_api_ptr_t<keyboard_shortcut_manager_v2> keyboard_api;

	if (msg == WM_SYSKEYDOWN)
	{
		if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp)) 
		{
			return true;
		}
	}
	else if (msg == WM_KEYDOWN)
	{
		if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp)) 
		{
			return true;
		}
		if (wp == VK_TAB)
		{
			uie::window::g_on_tab(GetFocus());
		}
	}
	return false;
}


void set_main_window_text(const char * ptr)
{
	if (ptr)
	{
		if (strcmp(windowtext,ptr)) uSetWindowText(g_main_window,ptr);
		windowtext = ptr;
	}
}

bool g_icon_created = false;

void destroy_systray_icon()
{
	if (g_icon_created)
	{
		uShellNotifyIcon(NIM_DELETE, g_main_window, 1, MSG_NOTICATION_ICON, nullptr, nullptr);
		g_icon_created = false;
	}
}

void create_systray_icon()
{
	uShellNotifyIcon(g_icon_created ? NIM_MODIFY : NIM_ADD, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, "foobar2000"/*core_version_info::g_get_version_string()*/);
	/* There was some misbehaviour with the newer messages. So we don't use them. */
	//	if (!g_icon_created)
	//		uih::ShellNotifyIcon(NIM_SETVERSION, g_main_window, 1, NOTIFYICON_VERSION, MSG_NOTICATION_ICON, g_icon, "foobar2000"/*core_version_info::g_get_version_string()*/);
	g_icon_created = true;
}

void create_icon_handle()
{
	const unsigned cx = GetSystemMetrics(SM_CXSMICON);
	const unsigned cy = GetSystemMetrics(SM_CYSMICON);
	if (g_icon) { DestroyIcon(g_icon); g_icon = nullptr; }
	if (cfg_custom_icon)
		g_icon = (HICON)uLoadImage(core_api::get_my_instance(), cfg_tray_icon_path, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
	if (!g_icon)
		g_icon = static_api_ptr_t<ui_control>()->load_main_icon(cx, cy);
}



class playlist_callback_single_columns : public playlist_callback_single_static
{
public:

	void on_items_added(unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection) override//inside any of these methods, you can call IPlaylist APIs to get exact info about what happened (but only methods that read playlist state, not those that modify it)
	{
		if (g_main_window) 
		{
			status_bar::set_part_sizes(status_bar::t_part_length);
		}
	}
	void on_items_reordered(const unsigned * order,unsigned count) override{};//changes selection too; doesnt actually change set of items that are selected or item having focus, just changes their order
	void FB2KAPI on_items_removing(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count) override{};//called before actually removing them
	void FB2KAPI on_items_removed(const bit_array & p_mask,unsigned p_old_count,unsigned p_new_count) override
	{
		if (g_main_window) 
		{
			status_bar::set_part_sizes(status_bar::t_part_length);
		}
	};
	void on_items_selection_change(const bit_array & affected,const bit_array & state) override
	{
		if (g_main_window) 
		{
			status_bar::set_part_sizes(status_bar::t_part_length);
		}
	}
	void on_item_focus_change(unsigned from,unsigned to) override{};//focus may be -1 when no item has focus; reminder: focus may also change on other callbacks
	void FB2KAPI on_items_modified(const bit_array & p_mask) override{;}
	void FB2KAPI on_items_modified_fromplayback(const bit_array & p_mask,play_control::t_display_level p_level) override{};
	void on_items_replaced(const bit_array & p_mask,const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & p_data) override{};
	void on_item_ensure_visible(unsigned idx) override{};

	void on_playlist_switch() override 
	{
		if (g_main_window) 
		{
			status_bar::set_part_sizes(status_bar::t_parts_all);
		}
	};
	void on_playlist_renamed(const char * p_new_name,unsigned p_new_name_len) override {};
	void on_playlist_locked(bool p_locked) override 
	{
		if (g_main_window)
			if (g_status && main_window::config_get_status_show_lock())
				status_bar::set_part_sizes(status_bar::t_parts_all);
	};

	void on_default_format_changed() override {};
	void on_playback_order_changed(unsigned p_new_index) override {};

	unsigned get_flags() override {return playlist_callback_single::flag_all;}

};
static service_factory_single_t<playlist_callback_single_columns> asdf2;


void g_split_string_by_crlf(const char * text, pfc::string_list_impl & p_out)
{
	const char * ptr = text;
	while (*ptr)
	{
		const char * start = ptr;
		t_size counter = 0;
		while (*ptr && *ptr != '\r' && *ptr != '\n') 
		{
			ptr++;
		}

		p_out.add_item(pfc::string8(start, ptr-start));

		if (*ptr == '\r') ptr++;
		if (*ptr == '\n') ptr++;
	}
}


void make_ui()
{
	ui_initialising = true;
	
	RECT rc;
	GetWindowRect(g_main_window, &rc);
	
	g_layout_window.create(g_main_window);
	
	create_rebar();
	create_status();
	if (settings::show_status_pane) g_status_pane.create(g_main_window);
	
	g_layout_window.set_focus();
	ui_initialising = false;
}

void size_windows()
{
	if (!/*g_minimised*/IsIconic(g_main_window) && !ui_initialising)
	{		
		RECT rc_main_client;
		GetClientRect(g_main_window, &rc_main_client);
		
		HDWP dwp = BeginDeferWindowPos(7);
		if (dwp)
		{
			
			int status_height = 0;
			if (g_status) 
			{

				//SendMessage(g_status, WM_SETREDRAW, FALSE, 0);
				SendMessage(g_status,WM_SIZE,0,0);
				RECT rc_status;
				GetWindowRect(g_status, &rc_status);
				
				status_height += rc_status.bottom-rc_status.top;
				
				//dwp = DeferWindowPos(dwp, g_status, 0, 0, rc_main_client.bottom-status_height, rc_main_client.right-rc_main_client.left, status_height, SWP_NOZORDER|SWP_NOREDRAW);
				
			}
			if (g_status_pane.get_wnd()) 
			{
				int cy = g_status_pane.get_ideal_height();
				RedrawWindow(g_status_pane.get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
				dwp = DeferWindowPos(dwp, g_status_pane.get_wnd(), nullptr, 0, rc_main_client.bottom-status_height-cy, rc_main_client.right-rc_main_client.left, cy, SWP_NOZORDER);
				status_height += cy;
			}
			int rebar_height=0;
			
			if (g_rebar) 
			{
				RECT rc_rebar;
				GetWindowRect(g_rebar, &rc_rebar);
				rebar_height = rc_rebar.bottom-rc_rebar.top;
			}
			if (g_layout_window.get_wnd())
				dwp = DeferWindowPos(dwp, g_layout_window.get_wnd(), nullptr, 0, rebar_height, rc_main_client.right-rc_main_client.left, rc_main_client.bottom-rc_main_client.top-rebar_height-status_height, SWP_NOZORDER);
			if (g_rebar) 
			{
				RedrawWindow(g_rebar, nullptr, nullptr, RDW_INVALIDATE);
				dwp = DeferWindowPos(dwp, g_rebar, nullptr, 0, 0, rc_main_client.right-rc_main_client.left, rebar_height, SWP_NOZORDER);
			}
			
			EndDeferWindowPos(dwp);

			if (g_status)
			{
				status_bar::set_part_sizes(status_bar::t_parts_none);
			}
			
			
		}
	}
}

class rename_param
{
public:
	modal_dialog_scope m_scope;
	pfc::string8 * m_text;
};

static BOOL CALLBACK RenameProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(wnd,DWLP_USER,lp);
		{
			rename_param * ptr = (rename_param *)lp;
			ptr->m_scope.initialize(FindOwningPopup(wnd));
			pfc::string_formatter formatter;
			formatter << R"(Rename playlist: ")" << *ptr->m_text << R"(")";
			uSetWindowText(wnd, formatter);
			uSetDlgItemText(wnd,IDC_EDIT,ptr->m_text->get_ptr());
		}
		return 1;
	case WM_COMMAND:
		switch(wp)
		{
		case IDOK:
			{
				rename_param * ptr = (rename_param *)GetWindowLong(wnd,DWLP_USER);
				uGetDlgItemText(wnd,IDC_EDIT,*ptr->m_text);
				EndDialog(wnd,1);
			}
			break;
		case IDCANCEL:
			EndDialog(wnd,0);
			break;
		}
		break;
		case WM_CLOSE:
			EndDialog(wnd,0);
			break;
	}
	return 0;
}

bool g_rename_dialog(pfc::string8 * text,HWND parent)
{
	rename_param param;
	param.m_text = text;
	return !!uDialogBox(IDD_RENAME_PLAYLIST,parent,RenameProc,(LPARAM)(&param));
}

void g_rename_playlist(unsigned idx, HWND wnd_parent)
{
	static_api_ptr_t<playlist_manager> playlist_api;
	pfc::string8 temp;
	if (playlist_api->playlist_get_name(idx,temp))
	{
		if (g_rename_dialog(&temp, wnd_parent))
		{//fucko: dialogobx has a messgeloop, someone might have called switcher api funcs in the meanwhile
//			idx = ((HWND)wp == g_tab) ? idx : SendMessage(g_plist,LB_GETCURSEL,0,0);
			unsigned num = playlist_api->get_playlist_count();
			if (idx<num)
			{
				playlist_api->playlist_rename(idx,temp,temp.length());
			}
		}
	}
}


bool g_get_resource_data (INT_PTR id, pfc::array_t<t_uint8> & p_out)
{
	bool ret = false;
	HRSRC rsrc = FindResource(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NOCOVER), L"PNG");
	HGLOBAL handle = LoadResource(core_api::get_my_instance(), rsrc);
	DWORD size = SizeofResource(core_api::get_my_instance(), rsrc);
	LPVOID ptr = LockResource(handle);
	if (ptr && size)
	{
		p_out.append_fromptr((t_uint8*)ptr, size);
		ret = true;
	}
	FreeResource(handle);
	return ret;
}



/** FUCKO: ITaskbarList3::ThumbBarUpdateButtons calls SendMessageTimeout without SMTO_BLOCK flag */
void g_update_taskbar_buttons_delayed(bool b_init)
{
	if (g_main_window)
		PostMessage(g_main_window, MSG_UPDATE_THUMBBAR, b_init, NULL);
}

void g_update_taskbar_buttons_now(bool b_init)
{
	if (main_window::g_ITaskbarList3.is_valid())
	{
		static_api_ptr_t<playback_control> play_api;

		bool b_is_playing = play_api->is_playing();
		bool b_is_paused = play_api->is_paused();

		const WCHAR * ttips[6] = {L"Stop", L"Previous", (b_is_playing && !b_is_paused ? L"Pause" : L"Play"), L"Next", L"Random"};
		INT_PTR bitmap_indices[] = {0, 1, (b_is_playing && !b_is_paused ? 2 : 3), 4, 5};

		THUMBBUTTON tb[tabsize(bitmap_indices)];
		memset(&tb, 0, sizeof(tb));

		size_t i;
		for (i=0; i<tabsize(bitmap_indices); i++)
		{
			tb[i].dwMask = THB_BITMAP|THB_TOOLTIP/*|THB_FLAGS*/;
			tb[i].iId = taskbar_buttons::ID_FIRST + i;
			tb[i].iBitmap =bitmap_indices[i];
			wcscpy_s(tb[i].szTip, tabsize(tb[i].szTip), ttips[i]);
			//if (tb[i].iId == ID_STOP && !b_is_playing)
			//	tb[i].dwFlags |= THBF_DISABLED;
		}

		if (b_init)
			main_window::g_ITaskbarList3->ThumbBarAddButtons(g_main_window, tabsize(tb), tb);
		else
			main_window::g_ITaskbarList3->ThumbBarUpdateButtons(g_main_window, tabsize(tb), tb);
	}
}




void update_titlebar()
{
	metadb_handle_ptr track;
	static_api_ptr_t<play_control> play_api;
	play_api->get_now_playing(track);
	if (track.is_valid())
	{
		pfc::string8 title;
		service_ptr_t<titleformat_object> to_wtitle;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(to_wtitle, main_window::config_main_window_title_script.get());
		play_api->playback_format_title_ex(track, nullptr, title, to_wtitle, nullptr, play_control::display_level_all);
		set_main_window_text(title);
		track.release();
	}
	else 
	{
		set_main_window_text("foobar2000"/*core_version_info::g_get_version_string()*/);
	}
	
	
}

void update_systray(bool balloon, int btitle, bool force_balloon)
{
	if (g_icon_created)
	{
		metadb_handle_ptr track;
		static_api_ptr_t<play_control> play_api;
		play_api->get_now_playing(track);
		pfc::string8 sys, title;

		if (track.is_valid())
		{
			
			service_ptr_t<titleformat_object> to_systray;
			static_api_ptr_t<titleformat_compiler>()->compile_safe(to_systray, main_window::config_notification_icon_script.get());
			play_api->playback_format_title_ex(track, nullptr, title, to_systray, nullptr, play_control::display_level_titles);
			
			track.release();
			
		}
		else
		{
			title = "foobar2000";//core_version_info::g_get_version_string();
		}

		uFixAmpersandChars(title,sys);
		
		if (balloon && (cfg_balloon||force_balloon))
		{
			uShellNotifyIconEx(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys, "", "");
			uShellNotifyIconEx(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys, (btitle == 0 ? "Now playing:" : (btitle == 1 ? "Unpaused:" : "Paused:")), title);
		}
		else
		uShellNotifyIcon(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys);
		
	}

}


class control_impl : public columns_ui::control
{
public:
	bool get_string(const GUID & p_guid, pfc::string_base & p_out) const override
	{
		if (p_guid == columns_ui::strings::guid_global_variables)
		{
			p_out = cfg_globalstring;
			return true;
		}
		return false;
	}
};

service_factory_single_t<control_impl> g_control_impl;