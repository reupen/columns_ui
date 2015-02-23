#ifndef _CUI_ARTWORK_H_
#define _CUI_ARTWORK_H_

namespace artwork_panel
{

	//extern cfg_string cfg_front, cfg_back, cfg_disc;//, cfg_icon;
	extern cfg_uint cfg_fb2k_artwork_mode;
	extern cfg_objList<pfc::string8> cfg_front_scripts, cfg_back_scripts, cfg_disc_scripts, cfg_artist_scripts;

	class artwork_panel_t :
		public uie::container_ui_extension_t<>/*, public now_playing_album_art_receiver*/,
		public play_callback,
		public playlist_callback_single,
		public ui_selection_callback
	{
	public:
		class completion_notify_forwarder : public completion_notify
		{
		public:
			void on_completion(unsigned p_code);
			completion_notify_forwarder(artwork_panel_t * p_this);;
		private:
			service_ptr_t<artwork_panel_t> m_this;
		};
		virtual const GUID & get_extension_guid() const;
		virtual void get_name(pfc::string_base & out)const;
		virtual void get_category(pfc::string_base & out)const;
		unsigned get_type() const;

		static void g_on_edge_style_change();

#if 0
		virtual void on_data(const char * p_path);
		virtual void on_stopped();
#else
		void on_playback_new_track(metadb_handle_ptr p_track);
		void on_playback_stop(play_control::t_stop_reason p_reason);

		void on_playback_starting(play_control::t_track_command p_command, bool p_paused) {}
		void on_playback_seek(double p_time) {}
		void on_playback_pause(bool p_state) {}
		void on_playback_edited(metadb_handle_ptr p_track) {}
		void on_playback_dynamic_info(const file_info & p_info) {}
		void on_playback_dynamic_info_track(const file_info & p_info) {}
		void on_playback_time(double p_time) {}
		void on_volume_change(float p_new_val) {}
#endif

		enum {
			playlist_callback_flags =
			playlist_callback_single::flag_on_items_selection_change
			| playlist_callback_single::flag_on_playlist_switch
		};
		void on_playlist_switch();
		void on_item_focus_change(t_size p_from, t_size p_to) {};

		void on_items_added(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data, const bit_array & p_selection) {}
		void on_items_reordered(const t_size * p_order, t_size p_count) {}
		void on_items_removing(const bit_array & p_mask, t_size p_old_count, t_size p_new_count) {}
		void on_items_removed(const bit_array & p_mask, t_size p_old_count, t_size p_new_count) {}
		void on_items_selection_change(const bit_array & p_affected, const bit_array & p_state);
		void on_items_modified(const bit_array & p_mask) {}
		void on_items_modified_fromplayback(const bit_array & p_mask, play_control::t_display_level p_level) {}
		void on_items_replaced(const bit_array & p_mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & p_data) {}
		void on_item_ensure_visible(t_size p_idx) {}

		void on_playlist_renamed(const char * p_new_name, t_size p_new_name_len) {}
		void on_playlist_locked(bool p_locked) {}

		void on_default_format_changed() {}
		void on_playback_order_changed(t_size p_new_index) {}

		void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr> & p_selection);


		void on_completion(unsigned p_code);

		static void g_on_colours_change();
		static void g_on_repository_change();
		void on_repository_change();

		void force_reload_artwork();

		artwork_panel_t();

	private:
		virtual class_data & get_class_data() const;

		class menu_node_track_mode : public ui_extension::menu_node_command_t
		{
			service_ptr_t<artwork_panel_t> p_this;
			t_size m_source;
		public:
			static const char * get_name(t_size source);
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
			virtual bool get_description(pfc::string_base & p_out)const;
			virtual void execute();
			menu_node_track_mode(artwork_panel_t * p_wnd, t_size p_value);;
		};

		class menu_node_artwork_type : public ui_extension::menu_node_command_t
		{
			service_ptr_t<artwork_panel_t> p_this;
			t_size m_type;
		public:
			static const char * get_name(t_size source);
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
			virtual bool get_description(pfc::string_base & p_out)const;
			virtual void execute();
			menu_node_artwork_type(artwork_panel_t * p_wnd, t_size p_value);;
		};

		class menu_node_source_popup : public ui_extension::menu_node_popup_t
		{
			pfc::list_t<ui_extension::menu_node_ptr> m_items;
		public:
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
			virtual unsigned get_children_count()const;
			virtual void get_child(unsigned p_index, uie::menu_node_ptr & p_out)const;
			menu_node_source_popup(artwork_panel_t * p_wnd);;
		};

		class menu_node_type_popup : public ui_extension::menu_node_popup_t
		{
			pfc::list_t<ui_extension::menu_node_ptr> m_items;
		public:
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
			virtual unsigned get_children_count()const;
			virtual void get_child(unsigned p_index, uie::menu_node_ptr & p_out)const;
			menu_node_type_popup(artwork_panel_t * p_wnd);;
		};
		class menu_node_preserve_aspect_ratio : public ui_extension::menu_node_command_t
		{
			service_ptr_t<artwork_panel_t> p_this;
		public:
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
			virtual bool get_description(pfc::string_base & p_out)const;
			virtual void execute();
			menu_node_preserve_aspect_ratio(artwork_panel_t * p_wnd);;
		};

		class menu_node_options : public ui_extension::menu_node_command_t
		{
		public:
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
			virtual bool get_description(pfc::string_base & p_out)const;
			virtual void execute();
		};
		class menu_node_lock_type : public ui_extension::menu_node_command_t
		{
			service_ptr_t<artwork_panel_t> p_this;
		public:
			virtual bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags)const;
			virtual bool get_description(pfc::string_base & p_out)const;
			virtual void execute();
			menu_node_lock_type(artwork_panel_t * p_wnd);;
		};

		virtual void get_menu_items(ui_extension::menu_hook_t & p_hook);
		enum { current_stream_version = 3 };
		virtual void set_config(stream_reader * p_reader, t_size size, abort_callback & p_abort);
		virtual void get_config(stream_writer * p_writer, abort_callback & p_abort)const;

		virtual LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
		void refresh_cached_bitmap();
		void flush_cached_bitmap();
		bool refresh_image(t_size index);
		void show_emptycover();
		void flush_image();

		ULONG_PTR m_gdiplus_instance;
		bool m_gdiplus_initialised;

		//pfc::rcptr_t<CCustomAlbumArtLoader> m_artwork_loader;
		pfc::refcounted_object_ptr_t<artwork_reader_manager_t> m_artwork_loader;
		//now_playing_album_art_manager m_nowplaying_artwork_loader;
		pfc::rcptr_t<Gdiplus::Bitmap> m_image;
		gdi_object_t<HBITMAP>::ptr_t m_bitmap;
		t_size m_position;
		t_size m_track_mode;
		bool m_preserve_aspect_ratio, m_lock_type;
		metadb_handle_list m_selection_handles;

		static pfc::ptr_list_t<artwork_panel_t> g_windows;
	};


}

#endif //_CUI_ARTWORK_H_