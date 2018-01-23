#include "stdafx.h"
#include "playlist_view.h"

void playlist_view::on_size(unsigned cx, unsigned cy)
{
    if (wnd_header) {
        RECT rc_playlist, rc_header;
        GetClientRect(get_wnd(), &rc_playlist);
        GetRelativeRect(wnd_header, get_wnd(), &rc_header);
        int header_height = calculate_header_height();

        if (rc_header.left != 0 - horizontal_offset || rc_header.top != 0
            || rc_header.right - rc_header.left != rc_playlist.right - rc_playlist.left + horizontal_offset
            || rc_header.bottom - rc_header.top != header_height) {
            SendMessage(wnd_header, WM_SETREDRAW, FALSE, 0);
            SetWindowPos(wnd_header, nullptr, 0 - horizontal_offset, 0,
                rc_playlist.right - rc_playlist.left + horizontal_offset, header_height, SWP_NOZORDER);
            if (cfg_nohscroll)
                rebuild_header(false);
            SendMessage(wnd_header, WM_SETREDRAW, TRUE, 0);
            RedrawWindow(wnd_header, nullptr, nullptr, RDW_UPDATENOW | RDW_INVALIDATE);
            if (rc_header.bottom - rc_header.top != header_height)
                RedrawWindow(wnd_playlist, nullptr, nullptr, RDW_UPDATENOW | RDW_INVALIDATE);
        }
    }
    // if (m_searcher.get_wnd())
    //    SetWindowPos(m_searcher.get_wnd(), NULL, 0, HIWORD(lp)-20, LOWORD(lp), 20, SWP_NOZORDER);
    update_scrollbar();
}

void playlist_view::on_size()
{
    RECT rc;
    GetClientRect(get_wnd(), &rc);
    on_size(rc.right, rc.bottom);
}
