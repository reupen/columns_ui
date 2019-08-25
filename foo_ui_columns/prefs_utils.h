#ifndef _COLOUMNS_PREFS_H_
#define _COLOUMNS_PREFS_H_

void populate_menu_combo(HWND wnd, unsigned ID, unsigned ID_DESC, const MenuItemIdentifier& p_item,
    const std::vector<MenuItemInfo>& p_cache, bool insert_none);
void on_menu_combo_change(HWND wnd, LPARAM lp, class ConfigMenuItem& cfg_menu_store,
    const std::vector<MenuItemInfo>& p_cache, unsigned ID_DESC);

namespace cui::prefs {

class EditControlSelectAllHook {
public:
    void attach(HWND wnd)
    {
        m_edit_proc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(wnd, GWLP_WNDPROC, reinterpret_cast<LPARAM>(s_on_hooked_message)));
        SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
    }

private:
    static LRESULT WINAPI s_on_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        auto p_data = reinterpret_cast<EditControlSelectAllHook*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
        return p_data ? p_data->on_hooked_message(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
    }

    LRESULT WINAPI on_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_CHAR:
            if (!(HIWORD(lp) & KF_REPEAT) && wp == 1 && (GetKeyState(VK_CONTROL) & KF_UP)) {
                Edit_SetSel(wnd, 0, -1);
                return 0;
            }
            break;
        }
        return CallWindowProc(m_edit_proc, wnd, msg, wp, lp);
    }

    WNDPROC m_edit_proc{};
};

HFONT create_default_ui_font(unsigned point_size);
HFONT create_default_title_font();

} // namespace cui::prefs

#endif
