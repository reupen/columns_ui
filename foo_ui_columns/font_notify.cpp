#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"

#include "font_notify.h"

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
