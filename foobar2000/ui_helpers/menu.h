#pragma once

namespace mmh
{
	namespace ui
	{
		class menu
		{
		public:
			enum flags
			{
				flag_checked=(1<<0),
				flag_radiochecked=(1<<1),
				flag_default=(1<<2)
			};

			menu() 
			{
				m_handle = CreatePopupMenu();
			}
			~menu()
			{
				if (m_handle)
				{
					DestroyMenu(m_handle);
					m_handle = NULL;
				}
			}

			size_t size()
			{
				return GetMenuItemCount(m_handle);
			}
			HMENU detach() { HMENU ret = m_handle; m_handle = NULL; return ret; }
			void insert_command(size_t index, const wchar_t * text, size_t id, size_t flags = NULL)
			{
				MENUITEMINFO mii;
				memset(&mii, 0, sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_ID|MIIM_STRING|MIIM_STATE|MIIM_FTYPE;
				mii.dwTypeData = const_cast<wchar_t*>(text);
				mii.cch = wcslen(text);
				mii.fType = MFT_STRING;
				mii.fState = MFS_ENABLED;
				if (flags & flag_radiochecked) mii.fType |= MFT_RADIOCHECK;
				if (flags & (flag_checked|flag_radiochecked)) mii.fState |= MFS_CHECKED;
				if (flags & (flag_default)) mii.fState |= MFS_DEFAULT;
				mii.wID = id;
				InsertMenuItem(m_handle, index, TRUE, &mii);
			}
			void insert_submenu(size_t index, const wchar_t * text, HMENU submenu, size_t flags = NULL)
			{
				MENUITEMINFO mii;
				memset(&mii, 0, sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_SUBMENU|MIIM_STRING|MIIM_FTYPE;
				mii.dwTypeData = const_cast<wchar_t*>(text);
				mii.cch = wcslen(text);
				mii.fType = MFT_STRING;
				mii.fState = MFS_ENABLED;
				mii.hSubMenu = submenu;
				InsertMenuItem(m_handle, index, TRUE, &mii);
			}
			void insert_separator(size_t index, size_t flags = NULL)
			{
				MENUITEMINFO mii;
				memset(&mii, 0, sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_FTYPE;
				mii.fType = MFT_SEPARATOR;
				InsertMenuItem(m_handle, index, TRUE, &mii);
			}
			void append_command(const wchar_t * text, size_t id, size_t flags = NULL)
			{
				insert_command(size(), text, id, flags);
			}
			void append_submenu(const wchar_t * text, HMENU submenu, size_t flags = NULL)
			{
				insert_submenu(size(), text, submenu, flags);
			}
			void append_separator(size_t flags = NULL)
			{
				insert_separator(size(), flags);
			}
			size_t run(HWND wnd, const POINT & pt)
			{
				return TrackPopupMenu(m_handle,TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,wnd,0);
			}
		private:
			HMENU m_handle;
		};
	}
}