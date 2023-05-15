#include "pch.h"

#ifdef QUICKFIND_ENABLED
#include "playlist_search.h"

#define cfg_default_search "%artist% - %title%"
#define cfg_default_search_mode 0

quickfind_window::quickfind_window()
    : wnd_edit(0)
    , m_is_running(false)
    , m_initialised(false)
    , m_editproc(0)
    , m_pattern(cfg_default_search)
    , height(0)
    , wnd_prev(0)
    , m_mode(cfg_default_search_mode)
{
}

quickfind_window::~quickfind_window() {}

void quickfind_window::on_size(unsigned cx, unsigned cy)
{
    SetWindowPos(wnd_edit, 0, 0, 0, cx, height, SWP_NOZORDER);
}
void quickfind_window::on_size()
{
    RECT rc;
    GetWindowRect(get_wnd(), &rc);
    on_size(wil::rect_width(rc), wil::rect_height(rc));
}

LRESULT quickfind_window::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        m_initialised = true;

        modeless_dialog_manager::g_add(wnd);

        long flags = 0;
        if (cfg_frame == 1)
            flags |= WS_EX_CLIENTEDGE;
        else if (cfg_frame == 2)
            flags |= WS_EX_STATICEDGE;

        m_search.set_pattern(m_pattern);
        m_search.set_mode(m_mode);

        wnd_edit = CreateWindowEx(flags, WC_EDIT, _T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0,
            0, wnd, HMENU(IDC_TREE), core_api::get_my_instance(), NULL);

        if (wnd_edit) {
            m_font = CreateFontIndirect(&cfg_font.get_value());
            SendMessage(wnd_edit, WM_SETFONT, (WPARAM)m_font.get(), MAKELPARAM(FALSE, 0));

            SetWindowLongPtr(wnd_edit, GWLP_USERDATA, (LPARAM)(this));
            m_editproc = (WNDPROC)SetWindowLongPtr(wnd_edit, GWLP_WNDPROC, (LPARAM)(hook_proc));
        }

        height = uGetFontHeight(m_font) + 2;
        SetFocus(wnd_edit);

        SendMessage(wnd_edit, EM_SETSEL, 0, -1);
        uGetWindowText text(wnd_edit);
        m_search.init();
        if (text.length())
            m_search.set_string(text);
        on_size();
    } break;
    case WM_GETMINMAXINFO: {
        LPMINMAXINFO mmi = LPMINMAXINFO(lp);
        mmi->ptMinTrackSize.y = height;
        mmi->ptMaxTrackSize.y = height;
        return 0;
    }
    case WM_SIZE:
        on_size(LOWORD(lp), HIWORD(lp));
        break;
    case WM_COMMAND:
        switch (wp) {
        case IDOK:
            if (m_search.on_key(VK_RETURN))
                return 0;
            break;
        case IDCANCEL: {
            SetFocus(wnd_prev);
        }
            return 0;
        }
        break;
    case WM_DESTROY:
        wnd_edit = 0;
        if (m_initialised) {
            m_initialised = false;
            modeless_dialog_manager::g_remove(wnd);
        }
        m_font.release();
        break;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

LRESULT WINAPI quickfind_window::hook_proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    quickfind_window* p_this;
    LRESULT rv;

    p_this = reinterpret_cast<quickfind_window*>(GetWindowLongPtr(wnd, GWLP_USERDATA));

    rv = p_this ? p_this->on_hook(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);

    return rv;
}

LRESULT WINAPI quickfind_window::on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_KEYDOWN:
        if (wp == VK_TAB) {
            ui_extension::window::g_on_tab(wnd);
        } else if (m_search.on_key(wp))
            return 0;
        else if (wp == VK_DELETE) {
            LRESULT ret = CallWindowProc(m_editproc, wnd, msg, wp, lp);
            m_search.set_string(uGetWindowText(wnd));
            return ret;
        }
        /*
        else if (wp == VK_ESCAPE)
        {
            SetFocus(wnd_prev);
            return 0;
        }*/
        break;
    case WM_SYSKEYDOWN:
        break;
    case WM_CHAR: {
        bool ctrl_down = 0 != (GetKeyState(VK_CONTROL) & KF_UP);
        if (wp == VK_RETURN || (wp == 0xa && ctrl_down) || wp == VK_ESCAPE)
            return 0;
        else if (!ctrl_down) {
            // assert (wp != VK_DELETE);
            unsigned start, end;
            SendMessage(wnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
            if (wp == VK_BACK || start != end || end != SendMessage(wnd, WM_GETTEXTLENGTH, 0, 0)) {
                LRESULT ret = CallWindowProc(m_editproc, wnd, msg, wp, lp);
                m_search.set_string(uGetWindowText(wnd));
                return ret;
            }
            m_search.add_char(wp);
        }
    } break;
    case WM_SETFOCUS:
        wnd_prev = (HWND)wp;
        break;
    case WM_KILLFOCUS: {
        m_search.reset();
        destroy();
    } break;
    }
    return CallWindowProc(m_editproc, wnd, msg, wp, lp);
}

#endif
