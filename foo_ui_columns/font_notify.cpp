#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"

#include "status_pane.h"
#include "font_notify.h"
#include "status_bar.h"

HFONT g_status_font = nullptr;
HFONT g_tab_font = nullptr;
HFONT g_plist_font = nullptr;

void font_cleanup()
{
    if (g_status_font) {
        DeleteObject(g_status_font);
        g_status_font = nullptr;
    }
    if (g_tab_font) {
        DeleteObject(g_tab_font);
        g_tab_font = nullptr;
    }
}

void on_show_toolbars_change()
{
    if (cui::main_window.get_wnd()) {
        create_rebar();
        if (g_rebar) {
            ShowWindow(g_rebar, SW_SHOWNORMAL);
            UpdateWindow(g_rebar);
        }
        cui::main_window.resize_child_windows();
    }
}

void on_show_status_change()
{
    {
        if (cui::main_window.get_wnd()) {
            create_status();
            if (g_status) {
                ShowWindow(g_status, SW_SHOWNORMAL);
                UpdateWindow(g_status);
            }
            cui::main_window.resize_child_windows();
        }
    }
}

void on_show_status_pane_change()
{
    {
        if (cui::main_window.get_wnd()) {
            if (settings::show_status_pane != (g_status_pane.get_wnd() != nullptr)) {
                if (settings::show_status_pane) {
                    g_status_pane.create(cui::main_window.get_wnd());
                    ShowWindow(g_status_pane.get_wnd(), SW_SHOWNORMAL);
                } else
                    g_status_pane.destroy();
                cui::main_window.resize_child_windows();
            }
        }
    }
}

// {B9D5EA18-5827-40be-A896-302A71BCAA9C}
static const GUID font_client_status_guid
    = {0xb9d5ea18, 0x5827, 0x40be, {0xa8, 0x96, 0x30, 0x2a, 0x71, 0xbc, 0xaa, 0x9c}};

class font_client_status : public cui::fonts::client {
public:
    const GUID& get_client_guid() const override { return font_client_status_guid; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Status bar"; }

    cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_labels; }

    void on_font_changed() const override { on_status_font_change(); }
};

font_client_status::factory<font_client_status> g_font_client_status;

void on_status_font_change()
{
    {
        if (g_status_font != nullptr) {
            if (g_status)
                SendMessage(g_status, WM_SETFONT, (WPARAM)0, MAKELPARAM(0, 0));
            DeleteObject(g_status_font);
            g_status_font = nullptr;
        }

        if (g_status) {
            g_status_font = static_api_ptr_t<cui::fonts::manager>()->get_font(font_client_status_guid);
            SendMessage(g_status, WM_SETFONT, (WPARAM)g_status_font, MAKELPARAM(1, 0));
            status_bar::set_part_sizes(status_bar::t_parts_all);
            cui::main_window.resize_child_windows();
        }
    }
}

#if 0
void on_tab_font_change()
{
    {
        if (g_tab_font!=0)
        {
            if (g_tab) SendMessage(g_tab,WM_SETFONT,(WPARAM)0,MAKELPARAM(0,0));
            DeleteObject(g_tab_font);
            g_tab_font=0;
        }
        
        if (g_tab) 
        {
            g_tab_font = CreateFontIndirect(&(LOGFONT)cfg_tab_font);
            SendMessage(g_tab,WM_SETFONT,(WPARAM)g_tab_font,MAKELPARAM(1,0));
            size_windows();
            
        }
    }
}
#endif

int CALLBACK FontSizesProc(const LOGFONT* plf, const TEXTMETRIC* ptm, DWORD FontType, LPARAM lp);

struct fontsizeinfo {
    bool up;
    INT size;
    INT new_size;
    INT caps;
    bool changed;
};

void g_get_font_size_next_step(LOGFONT& p_lf, bool up)
{
    LOGFONT lf = p_lf;

    {
        HDC dc = GetDC(nullptr);

        fontsizeinfo size{};

        size.up = up;
        size.caps = GetDeviceCaps(dc, LOGPIXELSY);
        size.size = -MulDiv(lf.lfHeight, 72, size.caps);
        size.new_size = up ? size.size : 0;
        size.changed = false;

        EnumFontFamiliesEx(dc, &lf, FontSizesProc, reinterpret_cast<LPARAM>(&size), 0);
        if (size.changed)
            lf.lfHeight = -MulDiv(size.new_size, size.caps, 72);

        ReleaseDC(nullptr, dc);

        if (lf.lfHeight)
            p_lf = lf;
    }
}

void set_font_size(bool up)
{
    LOGFONT lf_cp, lf_ng;
    static_api_ptr_t<cui::fonts::manager> api;
    api->get_font(pvt::g_guid_items_font, lf_ng);

    g_get_font_size_next_step(lf_cp, up);
    g_get_font_size_next_step(lf_ng, up);

    api->set_font(pvt::g_guid_items_font, lf_ng);
}

int CALLBACK FontSizesProc(const LOGFONT* plf, const TEXTMETRIC* ptm, DWORD FontType, LPARAM lp)
{
    auto* fn = (fontsizeinfo*)lp;

    if (FontType != TRUETYPE_FONTTYPE) {
        //   int  logsize    = plf->lfHeight;  //ptm->tmHeight - ptm->tmInternalLeading;
        int pointsize = abs(MulDiv(plf->lfHeight, 72, ((fontsizeinfo*)lp)->caps));

        if (fn->up) {
            if (pointsize > fn->size && (!(fn->new_size > fn->size) || pointsize < fn->new_size)) {
                fn->new_size = pointsize;
                fn->changed = true;
            }
        } else {
            if (pointsize < fn->size && pointsize > fn->new_size) {
                fn->new_size = pointsize;
                fn->changed = true;
            }
        }
        return 1;
    }
    if (fn->size > 1 && !fn->up)
        fn->new_size = fn->size - 1;
    else if (fn->size < MAXLONG && fn->up)
        fn->new_size = fn->size + 1;
    fn->changed = true;
    return 0;
}

int fontsizecommonproc(int pointsize, DWORD FontType, fontsizeinfo* fn)
{
    if (FontType != TRUETYPE_FONTTYPE) {
        if (fn->up) {
            if (pointsize > fn->size && (!(fn->new_size > fn->size) || pointsize < fn->new_size)) {
                fn->new_size = pointsize;
            }
        } else {
            if (pointsize < fn->size && pointsize > fn->new_size) {
                fn->new_size = pointsize;
            }
        }
        return 1;
    }
    fn->new_size = fn->size + (fn->up ? 1 : -1);
    return 0;
}
