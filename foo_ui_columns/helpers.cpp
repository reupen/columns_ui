#include "stdafx.h"

#include "dark_mode.h"

void g_ui_selection_manager_register_callback_no_now_playing_fallback(ui_selection_callback* p_callback)
{
    if (static_api_test_t<ui_selection_manager_v2>())
        static_api_ptr_t<ui_selection_manager_v2>()->register_callback(
            p_callback, ui_selection_manager_v2::flag_no_now_playing);
    else
        static_api_ptr_t<ui_selection_manager>()->register_callback(p_callback);
}

bool g_ui_selection_manager_is_now_playing_fallback()
{
    if (static_api_test_t<ui_selection_manager_v2>())
        return false;
    return static_api_ptr_t<ui_selection_manager>()->get_selection_type() == contextmenu_item::caller_now_playing;
}

void g_compare_file_with_bytes(
    const service_ptr_t<file>& p1, const pfc::array_t<t_uint8>& p2, bool& b_same, abort_callback& p_abort)
{
    try {
        b_same = false;
        t_filesize bytes = p1->get_size(p_abort);

        if (bytes == p2.get_size()) {
            enum { BUFSIZE = 1024 * 1024 };
            auto size = (unsigned)(BUFSIZE < bytes ? BUFSIZE : bytes);
            pfc::array_t<t_uint8> temp, temp2;
            temp.set_size(size);
            temp2.set_size(size);

            t_filesize done = 0;
            while (done < bytes) {
                if (p_abort.is_aborting())
                    throw exception_aborted();

                t_int64 delta64 = bytes - done;
                if (delta64 > BUFSIZE)
                    delta64 = BUFSIZE;
                auto delta = (unsigned)delta64;

                unsigned io_bytes_done = p1->read(temp.get_ptr(), delta, p_abort);

                if (io_bytes_done <= 0)
                    break;

                if (io_bytes_done != delta)
                    throw exception_io();

                if (memcmp(temp.get_ptr(), (char*)p2.get_ptr() + done, io_bytes_done) != 0)
                    return;

                done += delta;
            }
            b_same = true;
        }
    } catch (const exception_io&) {
    }
}

HBITMAP LoadMonoBitmap(INT_PTR uid, COLORREF cr_btntext)
{
    HBITMAP rv = nullptr;
    HRSRC rs = FindResource(core_api::get_my_instance(), MAKEINTRESOURCE(uid), RT_BITMAP);
    HGLOBAL glb = LoadResource(core_api::get_my_instance(), rs);
    void* p_res = LockResource(glb);
    auto* p_bih = (LPBITMAPINFO)p_res;
    if (p_bih) {
        unsigned num_colours = p_bih->bmiHeader.biClrUsed;
        if (!num_colours && p_bih->bmiHeader.biBitCount <= 8)
            num_colours = 1 << p_bih->bmiHeader.biBitCount;

        pfc::array_t<t_uint8> bmi;
        bmi.append_fromptr((t_uint8*)p_bih, p_bih->bmiHeader.biSize + sizeof(RGBQUAD) * num_colours);

        auto* lpbi = (LPBITMAPINFO)bmi.get_ptr();

        if (num_colours == 2) {
            lpbi->bmiColors[0].rgbRed = LOBYTE(LOWORD(cr_btntext));
            lpbi->bmiColors[0].rgbGreen = HIBYTE(LOWORD(cr_btntext));
            lpbi->bmiColors[0].rgbBlue = LOBYTE(HIWORD(cr_btntext));
            lpbi->bmiColors[1].rgbRed = 0xFF;
            lpbi->bmiColors[1].rgbGreen = 0xFF;
            lpbi->bmiColors[1].rgbBlue = 0xFF;
        }

        //        BITMAPINFOHEADER bmh = lpbi->bmiHeader;

        void* p_bits = &p_bih->bmiColors[num_colours];

        HDC dc = GetDC(nullptr);
        rv = CreateDIBitmap(dc, &lpbi->bmiHeader, CBM_INIT, p_bits, lpbi, DIB_RGB_COLORS);
        ReleaseDC(nullptr, dc);
    }
    return rv;
}

BOOL uDrawPanelTitle(HDC dc, const RECT* rc_clip, const char* text, int len, bool vert, bool is_dark)
{
    COLORREF cr_fore = cui::dark::get_system_colour(COLOR_MENUTEXT, is_dark);

    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, cr_fore);

        SIZE sz;
        uGetTextExtentPoint32(dc, text, len, &sz);
        int extra = vert ? rc_clip->bottom - sz.cy : (rc_clip->bottom - rc_clip->top - sz.cy - 1) / 2;
        /*
        if (world)
        {
        SetGraphicsMode(dc, GM_ADVANCED);
        XFORM xf;
        xf.eM11 = 0;
        xf.eM21 = 1;
        xf.eDx = 0;
        xf.eM12 = -1;
        xf.eM22 = 0;
        xf.eDy = rc_clip->right;
        SetWorldTransform(dc, &xf);
        }
        */
        //        HFONT old = SelectFont(dc, fnt_menu);

        uExtTextOut(dc, 5 + rc_clip->left, extra, ETO_CLIPPED, rc_clip, text, len, nullptr);
        //        SelectFont(dc, old);

        return TRUE;
    }
    return FALSE;
}

namespace cui::helpers {

struct EnumChildWindowsData {
    std::vector<HWND>& children;
    std::function<bool(HWND)> filter;
};

static BOOL WINAPI enum_child_windows_proc(HWND wnd, LPARAM lp)
{
    auto data = reinterpret_cast<EnumChildWindowsData*>(lp);
    if (!data->filter || data->filter(wnd))
        data->children.emplace_back(wnd);
    return TRUE;
}

std::vector<HWND> get_child_windows(HWND wnd, std::function<bool(HWND)> filter)
{
    std::vector<HWND> children;

    EnumChildWindowsData data{children, std::move(filter)};
    EnumChildWindows(wnd, enum_child_windows_proc, reinterpret_cast<LPARAM>(&data));

    return children;
}

pfc::string8 get_last_win32_error_message()
{
    pfc::string8 error_message;
    if (!uGetLastErrorMessage(error_message))
        error_message = "Unknown error";
    return error_message;
}

bool open_web_page(HWND wnd, const wchar_t* url)
{
    const auto process = ShellExecute(wnd, nullptr, url, nullptr, nullptr, SW_SHOWNORMAL);
    const bool succeeded = reinterpret_cast<int>(process) > 32;
    if (!succeeded) {
        fbh::show_info_box(wnd, "Error opening web page",
            "Columns UI was unable to open the web page using your default browser.", OIC_ERROR);
    }
    return succeeded;
}

void clip_minmaxinfo(MINMAXINFO& mmi)
{
    mmi.ptMinTrackSize.x = min(mmi.ptMinTrackSize.x, MAXSHORT);
    mmi.ptMinTrackSize.y = min(mmi.ptMinTrackSize.y, MAXSHORT);
    mmi.ptMaxTrackSize.y = min(mmi.ptMaxTrackSize.y, MAXSHORT);
    mmi.ptMaxTrackSize.x = min(mmi.ptMaxTrackSize.x, MAXSHORT);
}

} // namespace cui::helpers
