#pragma once

#include "fcl.h"
#include "layout.h"
#include "font_notify.h"

/*!
 * \file menu_items.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Main menu commands
 */
namespace mainmenu_groups_columns {
	extern const GUID view_columns_part;
	extern const GUID view_playlist_popup;
	extern const GUID playlist_font_part;
	extern const GUID playlist_misc_part;
	extern const GUID view_layout_commands;
	extern const GUID view_layout_presets;
}

class NOVTABLE mainmenu_command_t {
public:
	virtual const GUID& get_guid() const =0;
	virtual void get_name(pfc::string_base& p_out) const = 0;
	virtual bool get_description(pfc::string_base& p_out) const = 0;

	virtual bool get_display(pfc::string_base& p_text, t_uint32& p_flags) const
	{
		p_flags = 0;
		get_name(p_text);
		return g_main_window != 0;
	}

	virtual void execute(service_ptr_t<service_base> p_callback) const = 0;
};

class mainmenu_decrease_fontsize_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Decrease font size";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Decreases columns playlist font to previous available point size.";
		return true;
	}

	void execute(service_ptr_t<service_base> p_callback) const override
	{
		set_font_size(false);
		//on_playlist_font_change();
		//pvt::ng_playlist_view_t::g_on_font_change();
	}
};

class mainmenu_increase_fontsize_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Increase font size";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Increases columns playlist font to next available point size.";
		return true;
	}

	void execute(service_ptr_t<service_base> p_callback) const override
	{
		set_font_size(true);
		//on_playlist_font_change();
		//pvt::ng_playlist_view_t::g_on_font_change();
	}
};

//#define DEFINE_MAIN_MEMU_ITEM_CHECK(mainmenu_command_t

class mainmenu_show_statusbar_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Show status bar";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Shows or hides the Columns UI status bar.";
		return true;
	}

	bool get_display(pfc::string_base& p_text, t_uint32& p_flags) const override
	{
		p_flags = cfg_status != 0 ? mainmenu_commands::flag_checked : 0;
		get_name(p_text);
		return (g_main_window != 0);
	}

	void execute(service_ptr_t<service_base> p_callback) const override
	{
		cfg_status = (cfg_status == 0);
		on_show_status_change();
	}
};

class mainmenu_show_statuspane_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Show status pane";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Shows or hides the Columns UI status pane.";
		return true;
	}

	bool get_display(pfc::string_base& p_text, t_uint32& p_flags) const override
	{
		p_flags = settings::show_status_pane ? mainmenu_commands::flag_checked : 0;
		get_name(p_text);
		return (g_main_window != 0);
	}

	void execute(service_ptr_t<service_base> p_callback) const override
	{
		settings::show_status_pane = !settings::show_status_pane;
		on_show_status_pane_change();
	}
};

class mainmenu_show_toolbars_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Show toolbars";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Shows or hides the Columns UI toolbars.";
		return true;
	}

	bool get_display(pfc::string_base& p_text, t_uint32& p_flags) const override
	{
		p_flags = cfg_toolbars != 0 ? mainmenu_commands::flag_checked : 0;
		get_name(p_text);
		return (g_main_window != 0);
	}

	void execute(service_ptr_t<service_base> p_callback) const override
	{
		cfg_toolbars = (cfg_toolbars == 0);
		on_show_toolbars_change();
	}
};

class mainmenu_activate_now_playing_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Activate now playing";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Activates now playing item.";
		return true;
	}

	void execute(service_ptr_t<service_base> p_callback) const override
	{
		static_api_ptr_t<playlist_manager>()->highlight_playing_item();
	}
};

class mainmenu_export_layout_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Export layout";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Exports the active layout.";
		return true;
	}

	bool get_display(pfc::string_base& p_text, t_uint32& p_flags) const override
	{
		p_flags = 0;
		get_name(p_text);
		return (g_main_window != 0);
	}

	void execute(service_ptr_t<service_base> p_callback) const override
	{
		g_export_layout(core_api::get_main_window());
	}
};

class mainmenu_import_layout_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Import layout";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Imports a layout.";
		return true;
	}

	bool get_display(pfc::string_base& p_text, t_uint32& p_flags) const override
	{
		p_flags = 0;
		get_name(p_text);
		return (g_main_window != 0);
	}

	void execute(service_ptr_t<service_base> p_callback) const override
	{
		g_import_layout(core_api::get_main_window());
	}
};

class mainmenu_layout_live_edit_t : public mainmenu_command_t {
public:
	static const GUID g_guid;

	const GUID& get_guid() const override
	{
		return g_guid;
	}

	void get_name(pfc::string_base& p_out) const override
	{
		p_out = "Live editing";
	};

	bool get_description(pfc::string_base& p_out) const override
	{
		p_out = "Enables live editing of layout.";
		return true;
	}

	bool get_display(pfc::string_base& p_text, t_uint32& p_flags) const override
	{
		p_flags = 0 | (g_layout_window.get_layout_editing_active() ? mainmenu_commands::flag_checked : 0);
		get_name(p_text);
		return (g_main_window != 0);
	}

	void execute(service_ptr_t<service_base> p_callback) const override;
};

#if 0
class mainmenu_activate_t : public mainmenu_command_t
{
public:
	static const GUID g_guid;

	virtual const GUID & get_guid() const {return g_guid;}
	virtual void get_name(pfc::string_base & p_out)const {p_out = "Show or hide foobar2000";};
	virtual bool get_description(pfc::string_base & p_out) const {p_out = "Shows or hides foobar2000."; return true;}
	virtual bool get_display(pfc::string_base & p_text,t_uint32 & p_flags) const 
	{
		p_flags = 0;
		bool b_state = static_api_ptr_t<ui_control>()->is_visible();
		if (b_state)
			p_text = "Hide foobar2000";
		else
			p_text = "Show foobar2000";
		return (g_main_window != 0);
	}
	virtual void execute(service_ptr_t<service_base> p_callback) const
	{
		if (static_api_ptr_t<ui_control>()->is_visible())
			static_api_ptr_t<ui_control>()->hide();
		else
			static_api_ptr_t<ui_control>()->activate();
	}
};
#endif
