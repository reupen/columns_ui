#include "pch.h"

#include "dark_mode.h"
#include "dark_mode_dialog.h"
#include "resource_utils.h"

void g_ui_selection_manager_register_callback_no_now_playing_fallback(ui_selection_callback* p_callback)
{
    if (static_api_test_t<ui_selection_manager_v2>())
        ui_selection_manager_v2::get()->register_callback(p_callback, ui_selection_manager_v2::flag_no_now_playing);
    else
        ui_selection_manager::get()->register_callback(p_callback);
}

bool g_ui_selection_manager_is_now_playing_fallback()
{
    if (static_api_test_t<ui_selection_manager_v2>())
        return false;
    return ui_selection_manager::get()->get_selection_type() == contextmenu_item::caller_now_playing;
}

void g_compare_file_with_bytes(
    const service_ptr_t<file>& p1, const pfc::array_t<uint8_t>& p2, bool& b_same, abort_callback& p_abort)
{
    try {
        b_same = false;
        t_filesize bytes = p1->get_size(p_abort);

        if (bytes == p2.get_size()) {
            enum {
                BUFSIZE = 1024 * 1024
            };
            auto size = (unsigned)(BUFSIZE < bytes ? BUFSIZE : bytes);
            pfc::array_t<uint8_t> temp, temp2;
            temp.set_size(size);
            temp2.set_size(size);

            t_filesize done = 0;
            while (done < bytes) {
                if (p_abort.is_aborting())
                    throw exception_aborted();

                int64_t delta64 = bytes - done;
                if (delta64 > BUFSIZE)
                    delta64 = BUFSIZE;
                auto delta = (unsigned)delta64;

                const auto io_bytes_done = p1->read(temp.get_ptr(), delta, p_abort);

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

HBITMAP LoadMonoBitmap(WORD uid, COLORREF cr_btntext)
{
    HBITMAP rv = nullptr;
    const auto [data, _] = cui::resource_utils::get_resource_data(uid, RT_BITMAP);
    auto* p_bih = (LPBITMAPINFO)data;
    if (p_bih) {
        unsigned num_colours = p_bih->bmiHeader.biClrUsed;
        if (!num_colours && p_bih->bmiHeader.biBitCount <= 8)
            num_colours = 1 << p_bih->bmiHeader.biBitCount;

        pfc::array_t<uint8_t> bmi;
        bmi.append_fromptr((uint8_t*)p_bih, p_bih->bmiHeader.biSize + sizeof(RGBQUAD) * num_colours);

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

BOOL uDrawPanelTitle(HDC dc, const RECT* rc_clip, const char* text, int len, bool is_font_vertical, bool is_dark)
{
    const COLORREF cr_fore = get_colour(cui::dark::ColourID::PanelCaptionText, is_dark);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, cr_fore);

    SIZE sz{};
    uGetTextExtentPoint32(dc, text, len, &sz);

    const auto rect_text_top = is_font_vertical ? rc_clip->left : rc_clip->top;
    const auto rect_text_bottom = is_font_vertical ? rc_clip->right : rc_clip->bottom;

    const auto text_top_offset = rect_text_top + (rect_text_bottom - rect_text_top - sz.cy - 1) / 2;
    const auto x = is_font_vertical ? text_top_offset : 4_spx;
    const auto y = is_font_vertical ? rc_clip->bottom - 5_spx : text_top_offset;

    return uExtTextOut(dc, rc_clip->left + x, rc_clip->top + y, ETO_CLIPPED, rc_clip, text, len, nullptr);
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
    const bool succeeded = reinterpret_cast<INT_PTR>(process) > 32;
    if (!succeeded) {
        dark::modeless_info_box(wnd, "Error opening web page",
            "Columns UI was unable to open the web page using your default browser.", uih::InfoBoxType::Error);
    }
    return succeeded;
}

void clip_minmaxinfo(MINMAXINFO& mmi)
{
    mmi.ptMinTrackSize.x = std::min(mmi.ptMinTrackSize.x, static_cast<long>(MAXSHORT));
    mmi.ptMinTrackSize.y = std::min(mmi.ptMinTrackSize.y, static_cast<long>(MAXSHORT));
    mmi.ptMaxTrackSize.y = std::min(mmi.ptMaxTrackSize.y, static_cast<long>(MAXSHORT));
    mmi.ptMaxTrackSize.x = std::min(mmi.ptMaxTrackSize.x, static_cast<long>(MAXSHORT));
}

namespace {

bool is_tab_control(HWND wnd)
{
    std::array<wchar_t, 256> class_name{};
    RealGetWindowClass(wnd, class_name.data(), gsl::narrow<UINT>(class_name.size()));
    return wcsncmp(class_name.data(), WC_TABCONTROL, class_name.size()) == 0;
}

HWND find_parent_tab_control(HWND child, HWND root)
{
    if (is_tab_control(child))
        return child;

    HWND wnd = child;

    while (wnd != root) {
        HWND next = wnd;
        while ((next = GetWindow(next, GW_HWNDNEXT))) {
            if (is_tab_control(next)) {
                return next;
            }
        }

        wnd = GetAncestor(wnd, GA_PARENT);
    }

    return nullptr;
}

} // namespace

void handle_tabs_ctrl_tab(MSG* msg, HWND wnd_container, HWND wnd_tabs)
{
    if (msg->message != WM_KEYDOWN || msg->wParam != VK_TAB)
        return;

    if (msg->hwnd != wnd_container && !IsChild(wnd_container, msg->hwnd))
        return;

    if ((GetKeyState(VK_CONTROL) & 0x8000) == 0)
        return;

    const auto closest_tab_control = find_parent_tab_control(msg->hwnd, wnd_container);

    if (closest_tab_control && closest_tab_control != wnd_tabs)
        return;

    // Don't bother with sending and checking WM_GETDLGCODE – too many false positives

    msg->message = WM_NULL;
    msg->lParam = 0;
    msg->wParam = 0;

    const auto index = TabCtrl_GetCurSel(wnd_tabs);
    const auto count = TabCtrl_GetItemCount(wnd_tabs);

    if (count == 0)
        return;

    const auto new_index = [=] {
        if (GetKeyState(VK_SHIFT) & 0x8000)
            return index <= 0 ? count - 1 : index - 1;

        return index < 0 || index + 1 == count ? 0 : index + 1;
    }();

    if (index != new_index)
        TabCtrl_SetCurFocus(wnd_tabs, new_index);
}

} // namespace cui::helpers
