#include "ui_extension.h"

#if(WINVER >= 0x0500)
#define _GetParent(wnd) \
	GetAncestor(wnd, GA_PARENT)
#else
#define _GetParent(wnd) \
	GetParent(wnd)
#endif

// {5E283800-E682-4120-A1A8-D9CDADDC8956}
const GUID ui_extension::window_host::class_guid = 
{ 0x5e283800, 0xe682, 0x4120, { 0xa1, 0xa8, 0xd9, 0xcd, 0xad, 0xdc, 0x89, 0x56 } };

// {0503EC86-FCBB-4643-8274-1A6A135A404D}
const GUID ui_extension::window_host_ex::class_guid = 
{ 0x503ec86, 0xfcbb, 0x4643, { 0x82, 0x74, 0x1a, 0x6a, 0x13, 0x5a, 0x40, 0x4d } };

// {A2A21B5F-6280-47bf-856F-C5F2C129C3EA}
const GUID ui_extension::menu_window::class_guid = 
{ 0xa2a21b5f, 0x6280, 0x47bf, { 0x85, 0x6f, 0xc5, 0xf2, 0xc1, 0x29, 0xc3, 0xea } };

// {F84C1030-4407-496c-B09A-14E5EC5CA1C3}
const GUID ui_extension::window::class_guid =
{ 0xf84c1030, 0x4407, 0x496c, { 0xb0, 0x9a, 0x14, 0xe5, 0xec, 0x5c, 0xa1, 0xc3 } };

// {0627C2F4-3D09-4c02-A186-D6C2A11D7AFC}
const GUID ui_extension::splitter_window::class_guid = 
{ 0x627c2f4, 0x3d09, 0x4c02, { 0xa1, 0x86, 0xd6, 0xc2, 0xa1, 0x1d, 0x7a, 0xfc } };

// {B2F8E8D9-3302-481e-B615-39D70ADF818E}
const GUID ui_extension::splitter_window_v2::class_guid = 
{ 0xb2f8e8d9, 0x3302, 0x481e, { 0xb6, 0x15, 0x39, 0xd7, 0xa, 0xdf, 0x81, 0x8e } };

// {47D142FB-27E3-4a51-9817-10E2A1480E7D}
const GUID ui_extension::visualisation::class_guid = 
{ 0x47d142fb, 0x27e3, 0x4a51, { 0x98, 0x17, 0x10, 0xe2, 0xa1, 0x48, 0xe, 0x7d } };

// {E0472FC9-8B4A-4e50-8995-314BDA3DB5A2}
const GUID ui_extension::visualisation_host::class_guid = 
{ 0xe0472fc9, 0x8b4a, 0x4e50, { 0x89, 0x95, 0x31, 0x4b, 0xda, 0x3d, 0xb5, 0xa2 } };

// {EF05DE6B-D65E-47b7-9A45-7310DBE523E0}
const GUID ui_extension::button::class_guid = 
{ 0xef05de6b, 0xd65e, 0x47b7, { 0x9a, 0x45, 0x73, 0x10, 0xdb, 0xe5, 0x23, 0xe0 } };

// {87FC6DE2-26E7-40ec-8CC6-4D24198B1ED5}
const GUID ui_extension::button_v2::class_guid = 
{ 0x87fc6de2, 0x26e7, 0x40ec, { 0x8c, 0xc6, 0x4d, 0x24, 0x19, 0x8b, 0x1e, 0xd5 } };

// {C27A5B38-97C4-491a-9B61-597BB84837EF}
const GUID uie::custom_button::class_guid = 
{ 0xc27a5b38, 0x97c4, 0x491a, { 0x9b, 0x61, 0x59, 0x7b, 0xb8, 0x48, 0x37, 0xef } };

// {6AB8127D-F3E2-4ef6-A515-D54F66FDB7AC}
const GUID uie::menu_button = 
{ 0x6ab8127d, 0xf3e2, 0x4ef6, { 0xa5, 0x15, 0xd5, 0x4f, 0x66, 0xfd, 0xb7, 0xac } };

// {E67BC90B-40A8-4f54-A1C9-169A14E639D0}
const GUID uie::playlist_window::class_guid = 
{ 0xe67bc90b, 0x40a8, 0x4f54, { 0xa1, 0xc9, 0x16, 0x9a, 0x14, 0xe6, 0x39, 0xd0 } };

// {9E2EDA65-A107-4a46-833E-5A8383C10DF0}
const GUID uie::window_host_with_control::class_guid = 
{ 0x9e2eda65, 0xa107, 0x4a46, { 0x83, 0x3e, 0x5a, 0x83, 0x83, 0xc1, 0xd, 0xf0 } };

HWND uFindParentPopup(HWND wnd_child)
{
	HWND wnd_temp = _GetParent(wnd_child);
	
	while (wnd_temp && (GetWindowLong(wnd_temp, GWL_EXSTYLE) & WS_EX_CONTROLPARENT))
	{
		if (GetWindowLong(wnd_temp, GWL_STYLE) & WS_POPUP) break;
		else wnd_temp = _GetParent(wnd_temp);
	}
	return wnd_temp;
}

HWND ui_extension::window::g_on_tab(HWND wnd_focus)
{
	HWND rv = 0;
	
	HWND wnd_temp = GetAncestor(wnd_focus, GA_ROOT);/*_GetParent(wnd_focus);
	
	while (wnd_temp && GetWindowLong(wnd_temp, GWL_EXSTYLE) & WS_EX_CONTROLPARENT)
	{
		if (GetWindowLong(wnd_temp, GWL_STYLE) & WS_POPUP) break;
		else wnd_temp = _GetParent(wnd_temp);
	}*/
	
	if (wnd_temp)
	{
		HWND wnd_next = GetNextDlgTabItem(wnd_temp, wnd_focus, (GetKeyState(VK_SHIFT) & KF_UP) ? TRUE :  FALSE);
		if (wnd_next && wnd_next != wnd_focus) 
		{
			unsigned flags = uSendMessage(wnd_next, WM_GETDLGCODE, 0, 0);
			if (flags & DLGC_HASSETSEL) uSendMessage(wnd_next, EM_SETSEL, 0, -1);
			SetFocus(wnd_next);
			
			rv = wnd_next;
		}
	}
	return rv;
};

void ui_extension::window_info_list_simple::get_name_by_guid (const GUID & in, pfc::string_base & out)
{
	unsigned n, count = get_count();
	for (n=0; n<count; n++)
	{
		if (get_item(n).guid == in)
		{
			out = get_item(n).name;
			return;
		}
	}
}

//template <class T>
//int ui_extension_info_list_simple::sort_base<T>::ui_extension_list_sort_callback::compare(const T &n1,const T &n2)

void ui_extension::menu_hook_impl::fix_ampersand(const char * src,pfc::string_base & out)
{
	unsigned ptr = 0;
	while(src[ptr])
	{
		if (src[ptr]=='&')
		{
			out.add_string("&&");
			ptr++;
			while(src[ptr]=='&')
			{
				out.add_string("&&");
				ptr++;
			}
		}
		else out.add_byte(src[ptr++]);
	}
}

unsigned ui_extension::menu_hook_impl::flags_to_win32(unsigned flags)
{
	unsigned ret = 0;
	if (flags & menu_node_t::state_checked) ret |= MF_CHECKED;
	if (flags & menu_node_t::state_disabled) ret |= MF_DISABLED;
	if (flags & menu_node_t::state_greyed) ret |= MF_GRAYED;
	return ret;
}

void set_menu_item_radio(HMENU menu, UINT_PTR id)
{
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof (mii);
	mii.fMask = MIIM_FTYPE|MIIM_ID;
	mii.wID = id;
	GetMenuItemInfo(menu, id, FALSE, &mii);
	mii.fType |=MFT_RADIOCHECK;
	SetMenuItemInfo(menu, id, FALSE, &mii);
}
unsigned ui_extension::menu_hook_impl::win32_build_menu_recur(HMENU menu,ui_extension::menu_node_ptr parent,unsigned base_id,unsigned max_id)//menu item identifiers are base_id<=N<base_id+max_id (if theres too many items, they will be clipped)
{
	if (parent.is_valid() && parent->get_type()==menu_node_t::type_popup)
	{
		pfc::string8_fast_aggressive temp,name;
		temp.prealloc(32);
		name.prealloc(32);
		unsigned child_idx,child_num = parent->get_children_count();
		unsigned new_base = base_id;
		for(child_idx=0;child_idx<child_num;child_idx++)
		{
			menu_node_ptr child;
			parent->get_child(child_idx, child);
			if (child.is_valid())
			{
				unsigned displayflags=0;
				child->get_display_data(name, displayflags);
				if (strchr(name,'&')) {fix_ampersand(name,temp);name=temp;temp.reset();}
				menu_node_t::type_t type = child->get_type();
				if (type==menu_node_t::type_popup)
				{
					HMENU new_menu = CreatePopupMenu();
					uAppendMenu(menu,MF_STRING|MF_POPUP | flags_to_win32(displayflags),(UINT)new_menu,name);
					new_base = win32_build_menu_recur(new_menu,child,new_base,max_id);
				}
				else if (type==menu_node_t::type_separator)
				{
					uAppendMenu(menu,MF_SEPARATOR,0,0);
				}
				else if (type==menu_node_t::type_command)
				{
					unsigned id = new_base;
					if (id>=0 && (max_id<0 || id<max_id))
					{
						uAppendMenu(menu,MF_STRING | flags_to_win32(displayflags),new_base,name);
						if (displayflags & menu_node_t::state_radio)
							set_menu_item_radio(menu, new_base);
					}
					new_base++;
				}
			}
		}
		return new_base;
	}
	return base_id;
}
unsigned ui_extension::menu_hook_impl::execute_by_id_recur(ui_extension::menu_node_ptr parent,unsigned base_id,unsigned max_id,unsigned id_exec)//menu item identifiers are base_id<=N<base_id+max_id (if theres too many items, they will be clipped)
{
	if (parent.is_valid() && parent->get_type()==menu_node_t::type_popup)
	{
		unsigned child_idx,child_num = parent->get_children_count();
		unsigned new_base = base_id;
		for(child_idx=0;child_idx<child_num;child_idx++)
		{
			menu_node_ptr child;
			parent->get_child(child_idx, child);
			if (child.is_valid())
			{
				menu_node_t::type_t type = child->get_type();
				if (type==menu_node_t::type_popup)
				{
					new_base = execute_by_id_recur(child,new_base,max_id, id_exec);
				}
				else if (type==menu_node_t::type_separator)
				{
				}
				else if (type==menu_node_t::type_command)
				{
					unsigned id = new_base;
					if (id>=0 && (max_id<0 || id<max_id))
					{
						if (new_base == id_exec) child->execute();
					}
					new_base++;
				}
			}
		}
		return new_base;
	}
	return base_id;
}

void ui_extension::menu_hook_impl::add_node (const ui_extension::menu_node_ptr & p_node)
{
	m_nodes.add_item(p_node);
}
unsigned ui_extension::menu_hook_impl::get_children_count() const {return m_nodes.get_count();}
void ui_extension::menu_hook_impl::get_child(unsigned p_index, menu_node_ptr & p_out) const {p_out = m_nodes[p_index];}
uie::menu_node_t::type_t uie::menu_hook_impl::get_type() const {return type_popup;};

bool ui_extension::menu_hook_impl::get_display_data(pfc::string_base & p_out,unsigned & p_displayflags) const {return false;};
bool ui_extension::menu_hook_impl::get_description(pfc::string_base & p_out) const {return false;};
void ui_extension::menu_hook_impl::execute() {};

void ui_extension::menu_hook_impl::win32_build_menu(HMENU menu,unsigned base_id,unsigned max_id)
{
	m_base_id = base_id;
	m_max_id = max_id;
	win32_build_menu_recur(menu, this, base_id, max_id);
}
void ui_extension::menu_hook_impl::execute_by_id(unsigned id_exec)
{
	execute_by_id_recur(this, m_base_id, m_max_id, id_exec);
}

/**Stoled from menu_manager.cpp */
bool test_key(unsigned k)
{
	return (GetKeyState(k) & 0x8000) ? true : false;
}

#define F_SHIFT (HOTKEYF_SHIFT<<8)
#define F_CTRL (HOTKEYF_CONTROL<<8)
#define F_ALT (HOTKEYF_ALT<<8)
#define F_WIN (HOTKEYF_EXT<<8)

t_uint32 get_key_code(WPARAM wp) {
	t_uint32 code = (t_uint32)(wp & 0xFF);
	if (test_key(VK_CONTROL)) code|=F_CTRL;
	if (test_key(VK_SHIFT)) code|=F_SHIFT;
	if (test_key(VK_MENU)) code|=F_ALT;
	if (test_key(VK_LWIN) || test_key(VK_RWIN)) code|=F_WIN;
	return code;
}

bool uie::window::g_process_keydown_keyboard_shortcuts(WPARAM wp)
{
	return static_api_ptr_t<keyboard_shortcut_manager_v2>()->process_keydown_simple(get_key_code(wp));
}

namespace ui_extension
{

// {4673437D-1685-433f-A2CC-3864D609F4E2}
const GUID splitter_window::bool_show_caption = 
{ 0x4673437d, 0x1685, 0x433f, { 0xa2, 0xcc, 0x38, 0x64, 0xd6, 0x9, 0xf4, 0xe2 } };

// {35FA3514-8120-49e3-A56C-3EA1C8170A2E}
const GUID splitter_window::bool_hidden = 
{ 0x35fa3514, 0x8120, 0x49e3, { 0xa5, 0x6c, 0x3e, 0xa1, 0xc8, 0x17, 0xa, 0x2e } };

// {40C95DFE-E5E9-4f11-90EC-E741BE887DDD}
const GUID splitter_window::bool_autohide = 
{ 0x40c95dfe, 0xe5e9, 0x4f11, { 0x90, 0xec, 0xe7, 0x41, 0xbe, 0x88, 0x7d, 0xdd } };

// {3661A5E9-0FB4-4d2a-AC05-EF2F47D18AD9}
const GUID splitter_window::bool_locked = 
{ 0x3661a5e9, 0xfb4, 0x4d2a, { 0xac, 0x5, 0xef, 0x2f, 0x47, 0xd1, 0x8a, 0xd9 } };

// {709465DE-42CD-484d-BE8F-E737F01A6458}
const GUID splitter_window::uint32_orientation = 
{ 0x709465de, 0x42cd, 0x484d, { 0xbe, 0x8f, 0xe7, 0x37, 0xf0, 0x1a, 0x64, 0x58 } };

// {5CB327AB-34EB-409c-9B4E-10D0A3B04E8D}
const GUID splitter_window::uint32_size = 
{ 0x5cb327ab, 0x34eb, 0x409c, { 0x9b, 0x4e, 0x10, 0xd0, 0xa3, 0xb0, 0x4e, 0x8d } };

// {5CE8945E-BBB4-4308-99C1-DFA6D10F9004}
const GUID splitter_window::bool_show_toggle_area = 
{ 0x5ce8945e, 0xbbb4, 0x4308, { 0x99, 0xc1, 0xdf, 0xa6, 0xd1, 0xf, 0x90, 0x4 } };

// {71BC1FBC-EDD1-429c-B262-74C2F00AB3D3}
const GUID splitter_window::bool_use_custom_title = 
{ 0x71bc1fbc, 0xedd1, 0x429c, { 0xb2, 0x62, 0x74, 0xc2, 0xf0, 0xa, 0xb3, 0xd3 } };

// {3B4DEDA5-493D-4c5c-B52C-036DE4CF43D9}
const GUID splitter_window::string_custom_title = 
{ 0x3b4deda5, 0x493d, 0x4c5c, { 0xb5, 0x2c, 0x3, 0x6d, 0xe4, 0xcf, 0x43, 0xd9 } };


};

void stream_to_mem_block(stream_reader * p_source, pfc::array_t<t_uint8> & p_out,abort_callback & p_abort, unsigned p_sizehint, bool b_reset)
{
	if (b_reset)
		p_out.set_size(0);

	enum {min_buffer = 256};
	const unsigned buffer_size = max (min_buffer, p_sizehint);
	pfc::array_t<t_uint8> buffer;
	buffer.set_size(buffer_size);

	for(;;)
	{
		unsigned delta_done = 0;
		delta_done = p_source->read(buffer.get_ptr(),buffer_size,p_abort);
		p_out.append_fromptr(buffer.get_ptr(),delta_done);
		if (delta_done < buffer_size) break;
	}
}

void uie::splitter_item_t::set(const splitter_item_t & p_source)
{
	stream_writer_memblock temp;
	{
		set_panel_guid(p_source.get_panel_guid());

		p_source.get_panel_config(&temp);
		set_panel_config(&stream_reader_memblock_ref(temp.m_data.get_ptr(), temp.m_data.get_size()), temp.m_data.get_size());
	}
}

/**
* \mainpage					Columns UI SDK
*
* \section		intro_sec	Introduction
*
* The Columns UI SDK provides interfaces you can use to:
* - Create windows controlled by a host and embedded in the host's window
* - Provide information about command to be used as a toolbar button
*
* The uie namespace is synonymous with the ui_extension namespace.
*
* \section		install_sec	Installation
*
* You'll need:
* - <a href="http://msdn.microsoft.com/visualc/">Microsoft Visual C++ 2005</a>
* - <a href="http://foobar2000.org/">foobar2000 0.9 SDK</a>
*
* The latest <a href="http://www.microsoft.com/downloads/details.aspx?FamilyId=0BAF2B35-C656-4969-ACE8-E4C0C0716ADB&displaylang=en">Microsoft Platform SDK</a> may also be helpful.
*
* To install, extract the columns_ui-sdk.7z archive to the foobar2000 subdirectory of your foobar2000 SDK folder.
*
* \section		usage_sec	Usage
*
* Insert the columns_ui-sdk project into your solution, and add it as a dependency for your project.
* Then #include "columns_ui-sdk/ui_extension.h" in your project as needed.
*
* \section		panel_sec	Panel APIs
* \subsection	step1		APIs
*
* Clients should implement ui_extension::window.
* Specific sub-classes exist for
* - Menus: ui_extension::menu_window
* - Playlists: ui_extension::playlist_window
* - Splitter panels: ui_extension::splitter_window
* 
* Hosts should implement ui_extension::window_host.
* Hosts wishing to expose extnernal control methods can implement ui_extension::window_host_with_control instead.
* 
* \subsection	step2		Helpers
*
* The prefered method of implementing the window class is to derive from
* ui_extension::container_ui_extension.
* Single instance panels or dialog-based panels may wish to derive from
* ui_extension::window_base_t instead.
*
* Deriving directly from ui_extension::window is generally not needed.
*  
* \section		button_sec	Button APIs
* \subsection	ss_buttons	APIs
*
* The base class for buttons is ui_extension::button.
*
* If you wish to provide default bitmaps and additional information
* for your menu items, derive from ui_extension::menu_button.
* If you wish to implement a custom button not not based upon a menu item,
* derive from ui_extension::custom_button.
*
* \section		columns_sec	Standard windows
*
* The GUIDs for the standard panels may be found in the columns_ui::panels nampespace.
* The GUIDs for the standard toolbars may be found in the columns_ui::toolbars nampespace.
*
* You may use these GUIDs to create the standard windows in your own component;
* do not use them as GUIDs for your own windows.
*/ 
