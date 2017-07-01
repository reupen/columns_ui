#include "stdafx.h"
#include "seekbar.h"

//#define _WIN32_WINNT 0x500


#define ID_SEEK 2005

pfc::ptr_list_t<seek_bar_extension> seek_bar_extension::windows;

extern bool g_playing;

void seek_bar_extension::track_bar_host_impl::on_position_change(unsigned pos, bool b_tracking)
{
    if (!b_tracking || (GetKeyState(VK_SHIFT) & KF_UP))
        static_api_ptr_t<play_control>()->playback_seek(pos / 10.0);
}

void seek_bar_extension::track_bar_host_impl::get_tooltip_text(unsigned pos, track_bar_string& out)
{
    out = pfc::stringcvt::string_os_from_utf8(pfc::format_time_ex(pos / 10.0, 1));
};

void seek_bar_extension::update_seekbars(bool positions_only)
{
    unsigned n, count = windows.get_count();
    if (positions_only) {
        for (n = 0; n < count; n++)
            if (windows[n]->get_wnd())
                windows[n]->update_seek_pos();
    } else {
        for (n = 0; n < count; n++)
            if (windows[n]->get_wnd())
                windows[n]->update_seek();
    }
}

unsigned seek_bar_extension::g_seek_timer = 0;

void seek_bar_extension::update_seek_timer()
{
    if (windows.get_count() && g_playing) {
        if (!g_seek_timer) {
            g_seek_timer = SetTimer(nullptr, NULL, 150, (TIMERPROC)SeekTimerProc);
        }
    } else if (g_seek_timer) {
        KillTimer(nullptr, g_seek_timer);
        g_seek_timer = 0;
    }
}

void seek_bar_extension::update_seek_pos()
{
    if (wnd_seekbar == nullptr)
        return;

    static_api_ptr_t<play_control> play_api;

    if (play_api->is_playing() && play_api->playback_get_length()/* && play_api->playback_can_seek()*/) {
        double position = 0, length = 0;
        position = play_api->playback_get_position();
        length = play_api->playback_get_length();

        if (position < 0)
            position = 0;
        if (position > length)
            position = length;

        int pos_display = (int) (10.0 * position);
        m_child.set_position(pos_display);
    }
}


VOID CALLBACK seek_bar_extension::SeekTimerProc(HWND wnd, UINT msg, UINT event, DWORD time)
{
    if (windows.get_count() && g_playing)
        update_seekbars(true);
}

void seek_bar_extension::disable_seek()
{
    m_child.set_position(0);
    m_child.set_enabled(false);
}

void seek_bar_extension::update_seek()
{
    if (wnd_seekbar == nullptr)
        return;

    static_api_ptr_t<play_control> play_api;

    if (play_api->is_playing() && play_api->playback_get_length()/* && play_api->playback_can_seek()*/) {
        double position = 0, length = 0;
        position = play_api->playback_get_position();
        length = play_api->playback_get_length();

        if (length < 0)
            length = 0;
        if (position < 0)
            position = 0;
        if (position > length)
            position = length;

        m_child.set_range((int)(10 * length));//VC8

        int pos_display = (int) (10.0 * position);
        m_child.set_position(pos_display);

        if (play_api->playback_can_seek()/* && play_api->playback_get_length()*/)
            m_child.set_enabled(true);

    } else {
        disable_seek();
    }
}


seek_bar_extension::seek_bar_extension() : initialised(0), wnd_seekbar(nullptr) {};

seek_bar_extension::~seek_bar_extension()
{
    if (initialised) {
        windows.remove_item(this);
        initialised = false;
    }
}

LRESULT seek_bar_extension::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {

        case WM_CREATE:
            {
                windows.add_item(this);

                initialised = true;

                m_child.set_callback(&m_track_bar_host);
                m_child.set_show_tooltips(true);
                m_child.set_scroll_step(3);

                wnd_seekbar = m_child.create(wnd);

                if (wnd_seekbar) {
                    update_seek();

                    update_seek_timer();
                }

                seek_bar_extension::update_seek_timer();
                ShowWindow(wnd_seekbar, SW_SHOWNORMAL);
                break;
            }
        case WM_WINDOWPOSCHANGED:
            {
                LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
                if (!(lpwp->flags & SWP_NOSIZE)) {
                    SetWindowPos(wnd_seekbar, nullptr, 0, 0, lpwp->cx, lpwp->cy, SWP_NOZORDER);
                }
                break;
            }
        case WM_GETMINMAXINFO:
            {
                LPMINMAXINFO mmi = LPMINMAXINFO(lp);
                mmi->ptMinTrackSize.y = uih::ScaleDpiValue(21);

                return 0;
            }

        case WM_DESTROY:
            {
                if (initialised) {
                    m_child.destroy();
                    windows.remove_item(this);
                    initialised = false;
                }
                break;
            }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}


const GUID& seek_bar_extension::get_extension_guid() const
{
    // {678FE380-ABBB-4c72-A0B3-72E769671125}
    static const GUID extension_guid =
    {0x678fe380, 0xabbb, 0x4c72, {0xa0, 0xb3, 0x72, 0xe7, 0x69, 0x67, 0x11, 0x25}};
    return extension_guid;
}

void seek_bar_extension::get_name(pfc::string_base& out) const
{
    out.set_string("Seekbar");
}

void seek_bar_extension::get_category(pfc::string_base& out) const
{
    out.set_string("Toolbars");
}

unsigned seek_bar_extension::get_type() const
{
    return ui_extension::type_toolbar;
};


ui_extension::window_factory<seek_bar_extension> blue;
