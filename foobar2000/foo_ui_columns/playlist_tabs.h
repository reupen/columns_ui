#ifndef _COLUMNS_UI_TABS_H_
#define _COLUMNS_UI_TABS_H_

#include "foo_ui_columns.h"
#include "callback.h"
#include "extern.h"

void g_on_autohide_tabs_change();
void g_on_multiline_tabs_change();
void g_on_tabs_font_change();


class playlists_tabs_extension : public uie::container_ui_extension_t<ui_helpers::container_window, uie::splitter_window_v2>, public playlist_callback
{
public:
	//enum { MSG_EDIT_PANEL =  WM_USER+2};
	class window_host_impl : public ui_extension::window_host
	{
	public:

		virtual unsigned get_supported_types()const;

		virtual unsigned is_resize_supported(HWND wnd)const;

		virtual bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height);
		virtual bool is_visible(HWND wnd)const;
		virtual bool is_visibility_modifiable(HWND wnd, bool desired_visibility)const;
		virtual bool set_window_visibility(HWND wnd, bool visibility);

		virtual bool get_keyboard_shortcuts_enabled()const;

		virtual bool get_show_shortcuts()const;

		virtual void on_size_limit_change(HWND wnd, unsigned flags);;

		virtual const GUID & get_host_guid()const;

		virtual bool override_status_text_create(service_ptr_t<ui_status_text_override> & p_out);

		virtual void relinquish_ownership(HWND wnd);;
		void set_this(playlists_tabs_extension * ptr);
	private:
		service_ptr_t<playlists_tabs_extension> m_this;

	};
private:

	service_ptr_t<ui_status_text_override> m_status_override;

	WNDPROC tabproc;

	bool m_dragging;
	unsigned m_dragging_idx;
	RECT m_dragging_rect;

	bool m_playlist_switched;
	bool m_switch_timer;
	unsigned m_switch_playlist;
	bool initialised;

	t_int32 m_mousewheel_delta;
	UINT_PTR ID_CUSTOM_BASE;

	service_ptr_t<contextmenu_manager> p_manager;

	virtual class_data & get_class_data()const;

public:
	static pfc::ptr_list_t<playlists_tabs_extension> list_wnd;

	HWND wnd_tabs;
	LRESULT WINAPI hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT WINAPI main_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	playlists_tabs_extension();

	~playlists_tabs_extension();

	class playlists_tabs_drop_target : public IDropTarget
	{
		bool m_last_rmb;
		long drop_ref_count;
		POINTL last_over;
		service_ptr_t<playlists_tabs_extension> p_list;
		pfc::com_ptr_t<IDataObject> m_DataObject;
		mmh::comptr_t<IDropTargetHelper> m_DropTargetHelper;
	public:
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR *ppvObject);
		virtual ULONG STDMETHODCALLTYPE   AddRef();
		virtual ULONG STDMETHODCALLTYPE   Release();
		virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
		virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
		virtual HRESULT STDMETHODCALLTYPE DragLeave(void);
		virtual HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
		playlists_tabs_drop_target(playlists_tabs_extension * p_wnd);
	};

	virtual void FB2KAPI on_items_removing(unsigned p_playlist, const bit_array & p_mask, unsigned p_old_count, unsigned p_new_count);;//called before actually removing them
	virtual void FB2KAPI on_items_removed(unsigned p_playlist, const bit_array & p_mask, unsigned p_old_count, unsigned p_new_count);;

	void on_playlist_activate(unsigned p_old, unsigned p_new);

	void on_playlists_reorder(const unsigned * p_order, unsigned p_count);
	void on_playlist_created(unsigned p_index, const char * p_name, unsigned p_name_len);
	void on_playlists_removed(const bit_array & p_mask, unsigned p_old_count, unsigned p_new_count);
	void on_playlist_renamed(unsigned p_index, const char * p_new_name, unsigned p_new_name_len);

	void on_items_added(unsigned int, unsigned int, const pfc::list_base_const_t<metadb_handle_ptr> &, const bit_array &);;
	void on_items_reordered(unsigned int, const unsigned int *, unsigned int);;
	void on_items_selection_change(unsigned int, const bit_array &, const bit_array &);;
	void on_item_focus_change(unsigned int, unsigned int, unsigned int);;
	void on_items_modified(unsigned int, const bit_array &);;
	void on_items_modified_fromplayback(unsigned int, const bit_array &, play_control::t_display_level);;
	void on_items_replaced(unsigned int, const bit_array &, const pfc::list_base_const_t<t_on_items_replaced_entry> &);;
	void on_item_ensure_visible(unsigned int, unsigned int);;
	void on_playlists_removing(const bit_array &, unsigned int, unsigned int);;
	void on_default_format_changed(void);;
	void on_playback_order_changed(unsigned int);;
	void on_playlist_locked(unsigned int, bool);;

	void kill_switch_timer();

	void switch_to_playlist_delayed2(unsigned idx);

	static const GUID extension_guid;

	virtual const GUID & get_extension_guid() const;

	virtual void get_name(pfc::string_base & out)const;
	virtual void get_category(pfc::string_base & out)const;
	virtual bool get_short_name(pfc::string_base & out)const;

	void set_styles(bool visible = true);

	void reset_size_limits();

	void on_size();

	void adjust_rect(bool b_larger, RECT * rc);

	void on_size(unsigned cx, unsigned cy);


	virtual unsigned get_type() const;;
	static void on_font_change();
	bool create_tabs();

	void create_child();
	void destroy_child();

	virtual bool is_point_ours(HWND wnd_point, const POINT & pt_screen, pfc::list_base_t<uie::window::ptr> & p_hierarchy);
	virtual void get_supported_panels(const pfc::list_base_const_t<uie::window::ptr> & p_windows, bit_array_var & p_mask_unsupported);

	virtual void insert_panel(unsigned index, const uie::splitter_item_t *  p_item);;
	virtual void remove_panel(unsigned index);;
	virtual void replace_panel(unsigned index, const uie::splitter_item_t * p_item);;
	virtual unsigned get_panel_count()const;;
	virtual unsigned get_maximum_panel_count()const;;
	virtual uie::splitter_item_t * get_panel(unsigned index)const;;

	virtual void import_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort);
	virtual void export_config(stream_writer * p_writer, abort_callback & p_abort) const;

	virtual void set_config(stream_reader * config, t_size p_size, abort_callback & p_abort);;

	virtual void get_config(stream_writer * out, abort_callback & p_abort) const;;

	virtual bool have_config_popup(unsigned p_index) const;

	virtual bool show_config_popup(unsigned p_index, HWND wnd_parent);

	void on_child_position_change();

private:
	static HFONT g_font;

	GUID m_child_guid;
	pfc::array_t<t_uint8> m_child_data;
	service_ptr_t<window_host_impl> m_host;
	ui_extension::window_ptr m_child;
	HWND m_child_wnd;
	HWND m_host_wnd;

	unsigned m_child_top;

	MINMAXINFO mmi;
};

#endif