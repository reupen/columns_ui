#pragma once

#include "filter.h"

namespace filter_panel {

	class filter_search_bar :
		public uie::container_uie_window_v2
	{
		class menu_node_show_clear_button : public ui_extension::menu_node_command_t
		{
			service_ptr_t<filter_search_bar> p_this;
		public:
			bool get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const override;
			bool get_description(pfc::string_base & p_out) const override;
			void execute() override;
			menu_node_show_clear_button(filter_search_bar * wnd) : p_this(wnd) {};
		};
	public:
		static bool g_activate();
		//static void g_on_orderedbysplitters_change();
		static bool g_filter_search_bar_has_stream(filter_search_bar const* p_seach_bar, const filter_panel_t::filter_stream_t * p_stream);

		template <class TStream>
		static void g_initialise_filter_stream(const TStream & p_stream)
		{
			for (t_size i = 0, count = g_active_instances.get_count(); i<count; i++)
			{

				if (!cfg_orderedbysplitters || g_filter_search_bar_has_stream(g_active_instances[i], p_stream.get_ptr()))
				{
					if (!g_active_instances[i]->m_active_search_string.is_empty())
					{
						p_stream->m_source_overriden = true;
						p_stream->m_source_handles = g_active_instances[i]->m_active_handles;
						break;
					}
				}
			}
		}


		void get_name(pfc::string_base & out) const override { out = "Filter search"; }

		const GUID & get_extension_guid() const override { return cui::toolbars::guid_filter_search_bar; }

		void get_category(pfc::string_base & out) const override { out = "Toolbars"; }

		unsigned get_type() const override { return uie::type_toolbar; }
		//virtual HBRUSH get_class_background() const {return HBRUSH(COLOR_WINDOW+1);}
		t_uint32 get_flags() const override { return flag_default_flags_plus_transparent_background; }

		filter_search_bar();

	private:
		const GUID & get_class_guid() override { return cui::toolbars::guid_filter_search_bar; }

		void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort) override;
		void get_config(stream_writer * p_writer, abort_callback & p_abort) const override;
		void get_menu_items(uie::menu_hook_t & p_hook) override;

		void on_show_clear_button_change();
		void on_search_editbox_change();
		void commit_search_results(const char * str, bool b_force_autosend = false, bool b_stream_update = false);

		void update_favourite_icon(const char * p_new = nullptr);

		LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
		void create_edit();
		void on_size(t_size cx, t_size cy) override;
		void activate();

		static LRESULT WINAPI g_on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
		LRESULT on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

		using uie::container_uie_window_v2::on_size;

		enum { id_edit = 668, id_toolbar };
		enum { TIMER_QUERY = 1001 };
		enum { idc_clear = 1001, idc_favourite = 1002, msg_favourite_selected = WM_USER + 2 };
		enum { config_version_current = 0 };

		HWND m_search_editbox, m_wnd_toolbar;
		gdi_object_t<HFONT>::ptr_t m_font;
		WNDPROC m_proc_search_edit;
		bool m_favourite_state, m_query_timer_active, m_show_clear_button;
		pfc::string8 m_active_search_string;
		metadb_handle_list m_active_handles;
		HWND m_wnd_last_focused;
		HIMAGELIST m_imagelist;

		int m_combo_cx, m_combo_cy, m_toolbar_cx, m_toolbar_cy;

		static pfc::ptr_list_t<filter_search_bar> g_active_instances;
	};

};
