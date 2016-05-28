#include "stdafx.h"
#include "volume.h"

class volume_panel_class_name
{
public:
	static const TCHAR * const get_class_name()
	{
		return _T("volume_toolbar");
	}
	static bool get_show_caption() {return false;}
	static COLORREF get_background_colour() {return -1;}
};

class volume_control_panel : public volume_control_t<false, false, volume_panel_class_name, uie::container_ui_extension_t<> >
{
	const GUID & get_extension_guid() const override
	{
		// {B3259290-CB68-4d37-B0F1-8094862A9524}
		static const GUID ret = 
		{ 0xb3259290, 0xcb68, 0x4d37, { 0xb0, 0xf1, 0x80, 0x94, 0x86, 0x2a, 0x95, 0x24 } };
		return ret;
	};

	void get_name(pfc::string_base & out)const override
	{
		out = "Volume";
	}
	void get_category(pfc::string_base & out)const override
	{
		out = "Toolbars";
	}

	unsigned get_type  () const override
	{
		return uie::type_toolbar;
	}
};

uie::window_factory<volume_control_panel> g_volume_panel;