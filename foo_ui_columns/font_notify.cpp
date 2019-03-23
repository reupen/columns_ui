#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"

#include "font_notify.h"
#include "font_utils.h"

void set_font_size(bool up)
{
    LOGFONT lf_ng;
    static_api_ptr_t<cui::fonts::manager> api;
    api->get_font(pvt::g_guid_items_font, lf_ng);

    cui::fonts::get_next_font_size_step(lf_ng, up);

    api->set_font(pvt::g_guid_items_font, lf_ng);
}
